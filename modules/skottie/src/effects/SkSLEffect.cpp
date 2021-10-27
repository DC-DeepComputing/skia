/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/effects/SkRuntimeEffect.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGColorFilter.h"

namespace skottie::internal {

#ifdef SK_ENABLE_SKSL

namespace  {
class SkSLShaderNode final : public sksg::CustomRenderNode {
public:
    explicit SkSLShaderNode(sk_sp<RenderNode> child) : INHERITED({std::move(child)}) {}

    SG_ATTRIBUTE(Shader, sk_sp<SkShader>, fEffectShader)
private:
    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        const auto& child = this->children()[0];
        const auto bounds = child->revalidate(nullptr, SkMatrix::I());

        return bounds;
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

    using INHERITED = sksg::CustomRenderNode;
};

class SkSLEffectAdapter final : public DiscardableAdapterBase<SkSLEffectAdapter,
                                                             SkSLShaderNode> {
public:
    SkSLEffectAdapter(const skjson::ArrayValue& jprops,
                      const AnimationBuilder& abuilder,
                      sk_sp<SkSLShaderNode> node)
        : INHERITED(std::move(node))
    {
        enum : size_t {
            kShader_Index = 0,
            kUniforms_Index = 1
        };

        // construct dynamic uniform list from jprops
        std::vector<std::tuple<SkString, VectorValue>> uniforms;
        for (const skjson::ObjectValue* jprop : jprops) {
            const skjson::StringValue* uniformName = (*jprop)["nm"];
            VectorValue uniformValue;
            auto uniformTuple = std::make_tuple(uniformName->toString(), uniformValue);
            fUniforms.push_back(uniformTuple);
            this->bind(abuilder, (*jprop)["v"], uniformValue);
        }
    }

private:
    void onSync() override {
        this->node()->setShader(buildEffectShader());
    }

    sk_sp<SkShader> buildEffectShader() const {
        auto result = SkRuntimeEffect::MakeForShader(fSkSL, {});
        if (!result.effect) {
            printf("!!! %s\n", result.errorText.c_str());
        }
        SkRuntimeShaderBuilder builder(result.effect);

        for (auto uniform : fUniforms) {
            auto name = std::get<0>(uniform);
            VectorValue data = std::get<1>(uniform);
            // TODO: check under the hood impl. for setUniform(data, size) get rid of switch?
            switch(data.size()) {
                // TODO: support more uniform types
                case 1:
                    builder.uniform(name.c_str()) = data.at(0);
                    break;
                default:
                   printf("!!! %s\n", "uniform data type not supported");
            }
        }
        return builder.makeShader(&SkMatrix::I(), false);
    }

    SkString fSkSL;
    std::vector<std::tuple<SkString, VectorValue>> fUniforms;

    using INHERITED = DiscardableAdapterBase<SkSLEffectAdapter, SkSLShaderNode>;
};

} // namespace

#endif  // SK_ENABLE_SKSL

sk_sp<sksg::RenderNode> EffectBuilder::attachSkSLEffect(const skjson::ArrayValue& jprops,
                                                             sk_sp<sksg::RenderNode> layer) const {
#ifdef SK_ENABLE_SKSL
    auto shaderNode = sk_make_sp<SkSLShaderNode>(std::move(layer));
    return fBuilder->attachDiscardableAdapter<SkSLEffectAdapter>(jprops, *fBuilder, shaderNode);
#else
    return layer;
#endif
}

} // namespace skottie::internal
