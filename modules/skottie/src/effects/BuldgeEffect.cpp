/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/core/SkPictureRecorder.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkNx.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "src/utils/SkJSON.h"

namespace skottie::internal {

#ifdef SK_ENABLE_SKSL

namespace {

static constexpr char gBulgeDisplacementSkSL[] = R"(
    uniform shader iImage;

    uniform float2 iCenter;
    uniform float2 iRadius;
    uniform float  iHeight;

    uniform float  iSelector;

    float2 displace_sph(float2 v) {
        float h = sqrt(iHeight),
              r = (1 + h*h)/2/h,
              arc_ratio = asin(length(v)/r)/asin(1/r);

        return normalize(v)*arc_ratio - v;
    }

    float2 displace_exp(float2 v) {
        float h = iHeight*iHeight*iHeight*1.3;
        return v*pow(dot(v,v), h) - v;
    }

    half2 displace(float2 v) {
        float t = dot(v, v);
        if (t >= 1) {
            return v;
        }

        float2 d = displace_sph(v) + displace_exp(v);

        return v + (d * iSelector);
    }

    half4 main(float2 xy) {
        float2 scale  = iRadius,
               center = iCenter;

        xy = (xy - center)/scale;

        xy = displace(xy);

        xy = xy*scale + center;

        return iImage.eval(xy);
    }
)";

class BulgeNode final : public sksg::CustomRenderNode {
public:
    explicit BulgeNode(sk_sp<RenderNode> child, const SkSize& child_size)
        : INHERITED({std::move(child)})
        , fChildSize(child_size) {}

    SG_ATTRIBUTE(Center  , SkPoint   , fCenter)
    SG_ATTRIBUTE(Radius  , SkV2      , fRadius)
    SG_ATTRIBUTE(Height  , float     , fHeight)
    SG_ATTRIBUTE(Selector, float     , fSelector)

private:
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

    sk_sp<SkShader> buildEffectShader() {
        auto result =
            SkRuntimeEffect::MakeForShader(SkString(gBulgeDisplacementSkSL), {});

        if (!result.effect) {
            printf("!!! %s\n", result.errorText.c_str());
        }

        SkRuntimeShaderBuilder builder(result.effect);
        builder.uniform("iCenter")   = fCenter;
        builder.uniform("iRadius")   = fRadius;
        builder.uniform("iHeight")   = fHeight;
        builder.uniform("iSelector") = fSelector;

        builder.child("iImage") = this->contentShader();

        return builder.makeShader(&SkMatrix::I(), false);
    }

    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        const auto& child = this->children()[0];
        fEffectShader = buildEffectShader();
        return child->revalidate(ic, ctm);
    }

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
        if (fHeight == 0) {
            return;
        }
        const auto& bounds = this->bounds();
        const auto local_ctx = ScopedRenderContext(canvas, ctx)
                .setIsolation(bounds, canvas->getTotalMatrix(), true);

        canvas->saveLayer(&bounds, nullptr);
        this->children()[0]->render(canvas, local_ctx);

        SkPaint effect_paint;
        effect_paint.setShader(fEffectShader);
        effect_paint.setBlendMode(SkBlendMode::kSrcOver);

        canvas->drawPaint(effect_paint);
    }

    const RenderNode* onNodeAt(const SkPoint&) const override { return nullptr; } // no hit-testing

    sk_sp<SkShader> fEffectShader;
    sk_sp<SkShader> fContentShader;
    const SkSize fChildSize;

    SkPoint fCenter = {0,0};
    SkV2    fRadius = {0,0};
    float   fHeight = 0;
    float   fSelector = 1;

    using INHERITED = sksg::CustomRenderNode;
};

class BulgeEffectAdapter final : public DiscardableAdapterBase<BulgeEffectAdapter,
                                                             BulgeNode> {
public:
    BulgeEffectAdapter(const skjson::ArrayValue& jprops,
                      const AnimationBuilder& abuilder,
                      sk_sp<BulgeNode> node)
        : INHERITED(std::move(node))
    {
        enum : size_t {
            kHorizontalRadius_Index = 0,
            kVerticalRadius_Index = 1,
            kBulgeCenter_Index = 2,
            kBulgeHeight_Index = 3,
            // kTaper_Index = 4,
            // kAA_Index = 5,
            // kPinning_Index = 6,
        };
        EffectBinder(jprops, abuilder, this).bind(kHorizontalRadius_Index, fHorizontalRadius)
                                            .bind(kVerticalRadius_Index, fVerticalRadius)
                                            .bind(kBulgeCenter_Index, fCenter)
                                            .bind(kBulgeHeight_Index, fBulgeHeight);
    }

private:
    void onSync() override {
        auto n = this->node();
        n->setCenter({fCenter.x, fCenter.y});
        n->setRadius({fHorizontalRadius, fVerticalRadius});
        n->setHeight(std::abs(fBulgeHeight)/4);
        n->setSelector((fBulgeHeight > 0) ? 1 : -1);

    }

    Vec2Value fCenter;
    ScalarValue fHorizontalRadius,
                fVerticalRadius,
                fBulgeHeight;
    using INHERITED = DiscardableAdapterBase<BulgeEffectAdapter, BulgeNode>;
};

} // namespace

#endif  // SK_ENABLE_SKSL

sk_sp<sksg::RenderNode> EffectBuilder::attachBulgeEffect(const skjson::ArrayValue& jprops,
                                                             sk_sp<sksg::RenderNode> layer) const {
#ifdef SK_ENABLE_SKSL
    auto shaderNode = sk_make_sp<BulgeNode>(std::move(layer), fLayerSize);
    return fBuilder->attachDiscardableAdapter<BulgeEffectAdapter>(jprops, *fBuilder, shaderNode);
#else
    return layer;
#endif
}

} // namespace skottie::internal
