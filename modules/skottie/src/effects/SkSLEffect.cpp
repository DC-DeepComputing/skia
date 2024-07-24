/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMalloc.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/effects/Effects.h"
#include "modules/sksg/include/SkSGColorFilter.h"
#include "modules/sksg/include/SkSGNode.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "src/utils/SkJSON.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

class SkMatrix;
struct SkPoint;

namespace sksg {
class InvalidationController;
}

namespace skottie::internal {

// https://g-issues.chromium.org/issues/40064011
#if defined(SK_ENABLE_SKOTTIE_SKSLEFFECT)

class SkSLShaderNode final : public sksg::CustomRenderNode {
public:
    explicit SkSLShaderNode(sk_sp<RenderNode> child, const SkSize& child_size)
        : INHERITED({std::move(child)})
        , fChildSize(child_size) {}

    sk_sp<SkShader> contentShader() {
        if (!fContentShader || this->hasChildrenInval()) {
            const auto& child = this->children()[0];
            child->revalidate(nullptr, SkMatrix::I());

            SkPictureRecorder recorder;
            child->render(recorder.beginRecording(SkRect::MakeSize(fChildSize)));

            fContentShader = recorder.finishRecordingAsPicture()
                    ->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkFilterMode::kLinear,
                                 nullptr, nullptr);
        }

        return fContentShader;
    }

    SG_ATTRIBUTE(Shader, sk_sp<SkShader>, fEffectShader)
private:
    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        const auto& child = this->children()[0];
        return child->revalidate(ic, ctm);
    }

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
        const auto& bounds = this->bounds();
        const auto local_ctx = ScopedRenderContext(canvas, ctx)
                .setIsolation(bounds, canvas->getTotalMatrix(), true);

        canvas->saveLayer(&bounds, nullptr);
        this->children()[0]->render(canvas, local_ctx);

        SkPaint effect_paint;
        effect_paint.setShader(fEffectShader);
        effect_paint.setBlendMode(SkBlendMode::kSrcIn);

        canvas->drawPaint(effect_paint);
    }

    const RenderNode* onNodeAt(const SkPoint&) const override { return nullptr; } // no hit-testing

    sk_sp<SkShader> fEffectShader;
    sk_sp<SkShader> fContentShader;
    const SkSize fChildSize;

    using INHERITED = sksg::CustomRenderNode;
};

class SkSLEffectBase {
public:
    SkSLEffectBase(const skjson::ArrayValue& jprops,
                   const AnimationBuilder& abuilder)
    {
        if (jprops.size() < 1) {
            return;
        }
        const skjson::ObjectValue* jSkSL = jprops[kSkSL_index];
        if (!jSkSL) {
            return;
        }
        const skjson::StringValue* jShader = (*jSkSL)["sh"];
        if (!jShader) {
            return;
        }
        SkString shader = SkString(jShader->begin(), jShader->size());
        // Here we declare implicit u_layer to capture the underlying layer content.
        // Do not declare this in the custom SkSL.
        shader.prepend(std::string_view("uniform shader u_layer;"));
        auto result = SkRuntimeEffect::MakeForShader(shader, {});
        if (!result.effect) {
            abuilder.log(Logger::Level::kError, nullptr, "Failed to parse SkSL shader: %s",
               result.errorText.c_str());
            return;
        }
        fEffect = std::move(result.effect);
    }
protected:
    enum : size_t {
        kSkSL_index = 0,
        kFirstUniform_index = 1,
    };

    void bindUniforms(const skjson::ArrayValue& jprops,
                      const AnimationBuilder& abuilder,
                      AnimatablePropertyContainer * const &container) {
        // construct dynamic uniform list from jprops, skip SkSL property
        for (size_t i = kFirstUniform_index; i < jprops.size(); i++) {
            const skjson::ObjectValue* jprop = jprops[i];
            if (!jprop) { continue; }
            const skjson::StringValue* uniformName = (*jprop)["nm"];
            if (!uniformName) { continue; }
            int type = ParseDefault<int>((*jprop)["ty"], -1);
            if (type == 0) { /* uniforms */
                auto uniformTuple = std::make_tuple(SkString(uniformName->begin(),
                                                            uniformName->size()),
                                                    std::make_unique<VectorValue>());
                fUniforms.push_back(std::move(uniformTuple));
                container->bind(abuilder, (*jprop)["v"], std::get<1>(fUniforms.back()).get());
            } else if (type == 1) { /* images */
                const skjson::ObjectValue* jimage = (*jprop)["v"];

                const skjson::StringValue* name = (*jimage)["p"];
                const skjson::StringValue* path = (*jimage)["u"];
                const skjson::StringValue* id   = (*jimage)["id"];
                auto asset = abuilder.fResourceProvider->loadImageAsset(path->begin(), name->begin(), id->begin());
                if (asset) {
                    // Child shaders are sampled in the same order in which they are declared in the
                    // SkSL. This has to happen because we don't use the SkRuntimeEffect builder.
                    // Note: We bypassed the builder to have animatable uniforms. If possible to get
                    // animatable uniforms while still using the builder, we should switch.
                    auto frameData = asset->getFrameData(0);
                    SkSamplingOptions sampling(SkFilterMode::kLinear);
                    fChildren.push_back(frameData.image->makeShader(sampling));
                } else {
                    SkDebugf("cannot find asset for custom shader effect: %s\n", name->begin());
                }
            }
        }
    }

    sk_sp<SkData> buildUniformData() const {
        auto uniformData = SkData::MakeUninitialized(fEffect->uniformSize());
        SkASSERT(uniformData);
        sk_bzero(uniformData->writable_data(), uniformData->size());
        for (const auto& uniform : fUniforms) {
            const auto& name = std::get<0>(uniform);
            const auto& data = std::get<1>(uniform);
            auto metadata = fEffect->findUniform(name.c_str());
            if (metadata && metadata->count == static_cast<int>(data->size())) {
                auto dst = reinterpret_cast<uint8_t*>(uniformData->writable_data()) + metadata->offset;
                memcpy(reinterpret_cast<void*>(dst), data->data(), data->size() * sizeof(float));
            } else {
                SkDebugf("cannot set malformed uniform: %s\n", name.c_str());
            }
        }
        return uniformData;
    }
    sk_sp<SkRuntimeEffect> fEffect;
    std::vector<std::tuple<SkString, std::unique_ptr<VectorValue>>> fUniforms;
    std::vector<sk_sp<SkShader>> fChildren{nullptr}; // fChildren[0] is the layer content child
};

class SkSLShaderAdapter final : public DiscardableAdapterBase<SkSLShaderAdapter,
                                                              SkSLShaderNode>,
                                public SkSLEffectBase {
public:
    SkSLShaderAdapter(const skjson::ArrayValue& jprops,
                      const AnimationBuilder& abuilder,
                      sk_sp<SkSLShaderNode> node)
        : DiscardableAdapterBase<SkSLShaderAdapter, SkSLShaderNode>(std::move(node))
        , SkSLEffectBase(jprops, abuilder)
    {
        this->bindUniforms(jprops, abuilder, this);
    }

private:
    void onSync() override {
        if (!fEffect) {
            return;
        }
        fChildren.at(0) = this->node()->contentShader();
        sk_sp<SkShader> shader =
                fEffect->makeShader(buildUniformData(), fChildren.data(), fChildren.size());
        this->node()->setShader(std::move(shader));
    }
};

class SkSLColorFilterAdapter final : public DiscardableAdapterBase<SkSLColorFilterAdapter,
                                                             sksg::ExternalColorFilter>
                                   , public SkSLEffectBase {
public:
    SkSLColorFilterAdapter(const skjson::ArrayValue& jprops,
                      const AnimationBuilder& abuilder,
                      sk_sp<sksg::ExternalColorFilter> node)
        : DiscardableAdapterBase<SkSLColorFilterAdapter, sksg::ExternalColorFilter>(std::move(node))
        , SkSLEffectBase(jprops, abuilder)
    {
        this->bindUniforms(jprops, abuilder, this);
    }

private:
    void onSync() override {
        if (!fEffect) {
            return;
        }
        auto cf = fEffect->makeColorFilter(buildUniformData());
        this->node()->setColorFilter(std::move(cf));
    }
};

#endif  // SK_ENABLE_SKOTTIE_SKSLEFFECT

sk_sp<sksg::RenderNode> EffectBuilder::attachSkSLShader(const skjson::ArrayValue& jprops,
                                                        sk_sp<sksg::RenderNode> layer) const {
#if defined(SK_ENABLE_SKOTTIE_SKSLEFFECT)
    auto shaderNode = sk_make_sp<SkSLShaderNode>(std::move(layer), fLayerSize);
    return fBuilder->attachDiscardableAdapter<SkSLShaderAdapter>(jprops, *fBuilder,
                                                                 std::move(shaderNode));
#else
    return layer;
#endif
}

sk_sp<sksg::RenderNode> EffectBuilder::attachSkSLColorFilter(const skjson::ArrayValue& jprops,
                                                             sk_sp<sksg::RenderNode> layer) const {
#if defined(SK_ENABLE_SKOTTIE_SKSLEFFECT)
    auto cfNode = sksg::ExternalColorFilter::Make(std::move(layer));
    return fBuilder->attachDiscardableAdapter<SkSLColorFilterAdapter>(jprops, *fBuilder,
                                                                      std::move(cfNode));
#else
    return layer;
#endif
}

} // namespace skottie::internal
