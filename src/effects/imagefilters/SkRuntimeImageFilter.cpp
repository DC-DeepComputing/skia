/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#ifdef SK_ENABLE_SKSL

#include "include/core/SkData.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkSpinlock.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkWriteBuffer.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

using namespace skia_private;

// NOTE: Not in an anonymous namespace so that SkRuntimeShaderBuilder can friend it.
class SkRuntimeImageFilter final : public SkImageFilter_Base {
public:
    SkRuntimeImageFilter(const SkRuntimeShaderBuilder& builder,
                         std::string_view childShaderNames[],
                         const sk_sp<SkImageFilter> inputs[],
                         int inputCount)
            : SkImageFilter_Base(inputs, inputCount, /*cropRect=*/nullptr)
            , fShaderBuilder(builder) {
        fChildShaderNames.reserve_back(inputCount);
        for (int i = 0; i < inputCount; i++) {
            fChildShaderNames.push_back(SkString(childShaderNames[i]));
        }
    }

    SkRect computeFastBounds(const SkRect& src) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterRuntimeImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkRuntimeImageFilter)

    bool onAffectsTransparentBlack() const override { return true; }
    // Currently there is no way for a client to specify the semantics of geometric uniforms that
    // should respond to the canvas matrix. Forcing translate-only is a hammer that lets the output
    // be correct at the expense of resolution when there's a lot of scaling. See skbug.com/13416.
    MatrixCapability onGetCTMCapability() const override { return MatrixCapability::kTranslate; }

    skif::FilterResult onFilterImage(const skif::Context&) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping&,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    skif::LayerSpace<SkIRect> onGetOutputLayerBounds(
            const skif::Mapping&,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    mutable SkSpinlock fShaderBuilderLock;
    mutable SkRuntimeShaderBuilder fShaderBuilder;
    STArray<1, SkString> fChildShaderNames;
};

sk_sp<SkImageFilter> SkImageFilters::RuntimeShader(const SkRuntimeShaderBuilder& builder,
                                                   std::string_view childShaderName,
                                                   sk_sp<SkImageFilter> input) {
    // If no childShaderName is provided, check to see if we can implicitly assign it to the only
    // child in the effect.
    if (childShaderName.empty()) {
        auto children = builder.effect()->children();
        if (children.size() != 1) {
            return nullptr;
        }
        childShaderName = children.front().name;
    }

    return SkImageFilters::RuntimeShader(builder, &childShaderName, &input, 1);
}

sk_sp<SkImageFilter> SkImageFilters::RuntimeShader(const SkRuntimeShaderBuilder& builder,
                                                   std::string_view childShaderNames[],
                                                   const sk_sp<SkImageFilter> inputs[],
                                                   int inputCount) {
    auto child_is_shader = [](const SkRuntimeEffect::Child* child) {
        return child && child->type == SkRuntimeEffect::ChildType::kShader;
    };

    for (int i = 0; i < inputCount; i++) {
        std::string_view name = childShaderNames[i];
        // All names must be non-empty, and present as a child shader in the effect:
        if (name.empty() || !child_is_shader(builder.effect()->findChild(name))) {
            return nullptr;
        }

        // We don't allow duplicates, either:
        for (int j = 0; j < i; j++) {
            if (name == childShaderNames[j]) {
                return nullptr;
            }
        }
    }

    return sk_sp<SkImageFilter>(new SkRuntimeImageFilter(builder, childShaderNames,
                                                         inputs, inputCount));
}

void SkRegisterRuntimeImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkRuntimeImageFilter);
}

sk_sp<SkFlattenable> SkRuntimeImageFilter::CreateProc(SkReadBuffer& buffer) {
    // We don't know how many inputs to expect yet. Passing -1 allows any number of children.
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, -1);
    if (common.cropRect()) {
        return nullptr;
    }

    // Read the SkSL string and convert it into a runtime effect
    SkString sksl;
    buffer.readString(&sksl);
    auto effect = SkMakeCachedRuntimeEffect(SkRuntimeEffect::MakeForShader, std::move(sksl));
    if (!buffer.validate(effect != nullptr)) {
        return nullptr;
    }

    // Read the uniform data and make sure it matches the size from the runtime effect
    sk_sp<SkData> uniforms = buffer.readByteArrayAsData();
    if (!buffer.validate(uniforms->size() == effect->uniformSize())) {
        return nullptr;
    }

    // Read the child shader names
    STArray<4, std::string_view> childShaderNames;
    STArray<4, SkString> childShaderNameStrings;
    childShaderNames.resize(common.inputCount());
    childShaderNameStrings.resize(common.inputCount());
    for (int i = 0; i < common.inputCount(); i++) {
        buffer.readString(&childShaderNameStrings[i]);
        childShaderNames[i] = childShaderNameStrings[i].c_str();
    }

    SkRuntimeShaderBuilder builder(std::move(effect), std::move(uniforms));

    // Populate the builder with the corresponding children
    for (const SkRuntimeEffect::Child& child : builder.effect()->children()) {
        std::string_view name = child.name;
        switch (child.type) {
            case SkRuntimeEffect::ChildType::kBlender: {
                builder.child(name) = buffer.readBlender();
                break;
            }
            case SkRuntimeEffect::ChildType::kColorFilter: {
                builder.child(name) = buffer.readColorFilter();
                break;
            }
            case SkRuntimeEffect::ChildType::kShader: {
                builder.child(name) = buffer.readShader();
                break;
            }
        }
    }

    if (!buffer.isValid()) {
        return nullptr;
    }

    return SkImageFilters::RuntimeShader(builder, childShaderNames.data(),
                                         common.inputs(), common.inputCount());
}

void SkRuntimeImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    fShaderBuilderLock.acquire();
    buffer.writeString(fShaderBuilder.effect()->source().c_str());
    buffer.writeDataAsByteArray(fShaderBuilder.uniforms().get());
    for (const SkString& name : fChildShaderNames) {
        buffer.writeString(name.c_str());
    }
    for (size_t x = 0; x < fShaderBuilder.children().size(); x++) {
        buffer.writeFlattenable(fShaderBuilder.children()[x].flattenable());
    }
    fShaderBuilderLock.release();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkRuntimeImageFilter::onFilterImage(const skif::Context& ctx) const {
    using ShaderFlags = skif::FilterResult::ShaderFlags;

    const int inputCount = this->countInputs();
    SkASSERT(inputCount == fChildShaderNames.size());

    skif::FilterResult::Builder builder{ctx};
    for (int i = 0; i < inputCount; ++i) {
        builder.add(this->getChildOutput(i, ctx));
    }
    return builder.eval([&](SkSpan<sk_sp<SkShader>> inputs) {
        // lock the mutation of the builder and creation of the shader so that the builder's state
        // is const and is safe for multi-threaded access.
        fShaderBuilderLock.acquire();
        for (int i = 0; i < inputCount; i++) {
            fShaderBuilder.child(fChildShaderNames[i].c_str()) = inputs[i];
        }
        sk_sp<SkShader> shader = fShaderBuilder.makeShader();

        // Remove the inputs from the builder to avoid unnecessarily prolonging the input shaders'
        // lifetimes.
        for (int i = 0; i < inputCount; i++) {
            fShaderBuilder.child(fChildShaderNames[i].c_str()) = nullptr;
        }
        fShaderBuilderLock.release();

        return shader;
    }, ShaderFlags::kSampleInParameterSpace | ShaderFlags::kNonLinearSampling);
}

skif::LayerSpace<SkIRect> SkRuntimeImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    return this->getChildInputLayerBounds(0, mapping, desiredOutput, contentBounds);
}

skif::LayerSpace<SkIRect> SkRuntimeImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // Pessimistically assume it can cover anything
    return skif::LayerSpace<SkIRect>(SkRectPriv::MakeILarge());
}

SkRect SkRuntimeImageFilter::computeFastBounds(const SkRect& src) const {
    return SkRectPriv::MakeLargeS32();
}

#endif  // SK_ENABLE_SKSL
