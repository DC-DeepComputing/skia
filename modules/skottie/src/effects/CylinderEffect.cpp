/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTPin.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/effects/Effects.h"
#include "modules/sksg/include/SkSGNode.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <utility>
#include <vector>

namespace skjson {
class ArrayValue;
}
namespace sksg {
class InvalidationController;
}

namespace skottie::internal {
namespace {

// This shader maps its child shader onto a cylinder.  To simplify things, we set it up such that:
//
//   - the cylinder is centered at origin and has r == 1
//   - the eye is positioned at (0,0,eye_z), where eye_z is chosen to visually match AE
//   - the POI for a given pixel is on the z = 0 plane (x,y,0)
//   - we're only rendering inside the projected circle, which guarantees a quadratic solution
//
// Effect stages:
//
//   1) ray-cast to find the cylinder intersection (selectable front/back solution)
//   2) calculate the surface normal
//   3) rotate the intersection point
//   4) UV-map the cylinder from the rotated point
//   5) scale uv to source size and sample
//   6) apply lighting model
//
// Note: the current implementation uses two passes for two-side ("full") rendering, on the
//       assumption that in practice most textures are opaque and two-side mode is infrequent;
//       if this proves to be problematic, we could expand the implementation to blend both sides
//       in one pass.
//
static constexpr char gCylinderSkSL[] =
    "uniform shader child;"

    "uniform half3x3 rot_matrix;"
    "uniform half2 child_scale;"
    "uniform half side_select;"

    // apply_light()"
    "%s"

    "half3 to_cylinder(half3 EYE) {"
        "half eye_z2 = EYE.z*EYE.z;"

        "half a = EYE.x*EYE.x + eye_z2,"
             "b = -2*eye_z2,"
             "c = eye_z2 - 1,"
             "t = (-b + side_select*sqrt(b*b - 4*a*c))/(2*a);"

        "return half3(0, 0, -EYE.z) + EYE*t;"
    "}"

    "half4 main(float2 xy) {"
        "half3 EYE = half3(xy, -5.5),"
              "POS = to_cylinder(EYE),"
                "N = normalize(half3(POS.x, 0, POS.z)),"
             "R_POS = rot_matrix*POS;"

        "half kRPI = 1/3.1415927;"

        "half2 UV = half2("
            "0.5 + kRPI * 0.5 * atan(R_POS.x, R_POS.z),"
            "0.5 + R_POS.y * 0.5"
        ");"

        "return apply_light(EYE, N, child.eval(UV*child_scale));"
    "}"
;

// CC Cylinder uses a Phong-like lighting model:
//
//   - "ambient" controls the intensity of the texture color
//   - "diffuse" controls a multiplicative mix of texture and light color
//   - "specular" controls a light color specular component
//   - "roughness" is the specular exponent reciprocal
//   - "light intensity" modulates the diffuse and specular components (but not ambient)
//   - "light height" and "light direction" specify the light source position in spherical coords
//
// Implementation-wise, light intensity/height/direction are all combined into l_vec.
// For efficiency, we fall back to a stripped-down shader (ambient-only) when the diffuse & specular
// components are not used.
//
// TODO: "metal" and "reflective" parameters are ignored.
static constexpr char gBasicLightSkSL[] =
    "uniform half l_coeff_ambient;"

    "half4 apply_light(half3 EYE, half3 N, half4 c) {"
        "c.rgb *= l_coeff_ambient;"
        "return c;"
    "}"
;

static constexpr char gFancyLightSkSL[] =
    "uniform half3 l_vec;"
    "uniform half3 l_color;"
    "uniform half l_coeff_ambient;"
    "uniform half l_coeff_diffuse;"
    "uniform half l_coeff_specular;"
    "uniform half l_specular_exp;"

    "half4 apply_light(half3 EYE, half3 N, half4 c) {"
        "half3 LR = reflect(-l_vec*side_select, N);"
        "half s_base = max(dot(normalize(EYE), LR), 0),"

        "a = l_coeff_ambient,"
        "d = l_coeff_diffuse * max(dot(l_vec, N), 0),"
        "s = l_coeff_specular * saturate(pow(s_base, l_specular_exp));"

        "c.rgb = (a + d*l_color)*c.rgb + s*l_color*c.a;"

        "return c;"
    "}"
;

static sk_sp<SkRuntimeEffect> cylinder_fancylight_effect() {
    static const SkRuntimeEffect* effect =
            SkRuntimeEffect::MakeForShader(SkStringPrintf(gCylinderSkSL, gFancyLightSkSL), {})
                    .effect.release();
    if (0 && !effect) {
        printf("!!! %s\n",
               SkRuntimeEffect::MakeForShader(SkStringPrintf(gCylinderSkSL, gFancyLightSkSL), {})
                       .errorText.c_str());
    }
    SkASSERT(effect);

    return sk_ref_sp(effect);
}

static sk_sp<SkRuntimeEffect> cylinder_basiclight_effect() {
    static const SkRuntimeEffect* effect =
            SkRuntimeEffect::MakeForShader(SkStringPrintf(gCylinderSkSL, gBasicLightSkSL), {})
                    .effect.release();
    SkASSERT(effect);

    return sk_ref_sp(effect);
}

class CylinderNode final : public sksg::CustomRenderNode {
public:
    CylinderNode(sk_sp<RenderNode> child, const SkSize& child_size)
        : INHERITED({std::move(child)})
        , fChildSize(child_size) {}

    enum class RenderSide {
        kFull,
        kOutside,
        kInside,
    };

    SG_ATTRIBUTE(Center  , SkPoint   , fCenter)
    SG_ATTRIBUTE(Radius  , float     , fRadius)
    SG_ATTRIBUTE(Rotation, SkM44     , fRot   )
    SG_ATTRIBUTE(Side    , RenderSide, fSide  )

    SG_ATTRIBUTE(LightVec     , SkV3 , fLightVec     )
    SG_ATTRIBUTE(LightColor   , SkV3 , fLightColor   )
    SG_ATTRIBUTE(AmbientLight , float, fAmbientLight )
    SG_ATTRIBUTE(DiffuseLight , float, fDiffuseLight )
    SG_ATTRIBUTE(SpecularLight, float, fSpecularLight)
    SG_ATTRIBUTE(SpecularExp  , float, fSpecularExp  )

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

    sk_sp<SkShader> buildEffectShader(float selector) {
        const auto has_fancy_light =
                fLightVec.length() > 0 && (fDiffuseLight > 0 || fSpecularLight > 0);

        SkRuntimeShaderBuilder builder(has_fancy_light
                                           ? cylinder_fancylight_effect()
                                           : cylinder_basiclight_effect());

        builder.child  ("child")       = this->contentShader();
        builder.uniform("child_scale") = fChildSize;
        builder.uniform("side_select") = selector;
        builder.uniform("rot_matrix")  = std::array<float,9>{
            fRot.rc(0,0), fRot.rc(0,1), fRot.rc(0,2),
            fRot.rc(1,0), fRot.rc(1,1), fRot.rc(1,2),
            fRot.rc(2,0), fRot.rc(2,1), fRot.rc(2,2),
        };

        builder.uniform("l_coeff_ambient")  = fAmbientLight;

        if (has_fancy_light) {
            builder.uniform("l_vec")            = fLightVec * -selector;
            builder.uniform("l_color")          = fLightColor;
            builder.uniform("l_coeff_diffuse")  = fDiffuseLight;
            builder.uniform("l_coeff_specular") = fSpecularLight;
            builder.uniform("l_specular_exp")   = fSpecularExp;
        }

        const auto lm = SkMatrix::Translate(fCenter.fX, fCenter.fY) *
                        SkMatrix::Scale(fRadius, fRadius);

        return builder.makeShader(&lm);
    }

    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        fCylinderShader.reset();
        if (fSide != RenderSide::kOutside) {
            fCylinderShader = this->buildEffectShader(1);
        }
        if (fSide != RenderSide::kInside) {
            auto outside = this->buildEffectShader(-1);
            fCylinderShader = fCylinderShader
                    ? SkShaders::Blend(SkBlendMode::kSrcOver,
                                       std::move(fCylinderShader),
                                       std::move(outside))
                    : std::move(outside);
        }
        SkASSERT(fCylinderShader);

        return SkRect::MakeLTRB(fCenter.fX - fRadius,
                                fCenter.fY - fRadius,
                                fCenter.fX + fRadius,
                                fCenter.fY + fRadius);
    }

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
        if (fRadius <= 0) {
            return;
        }

        SkPaint cylinder_paint;
        cylinder_paint.setAntiAlias(true);
        cylinder_paint.setShader(fCylinderShader);

        canvas->drawCircle(fCenter, fRadius, cylinder_paint);
    }

    const RenderNode* onNodeAt(const SkPoint&) const override { return nullptr; } // no hit-testing

    const SkSize fChildSize;

    // Cached shaders
    sk_sp<SkShader> fCylinderShader;
    sk_sp<SkShader> fContentShader;

    // Effect controls.
    SkM44      fRot;
    SkPoint    fCenter = {0,0};
    float      fRadius = 0;
    RenderSide fSide   = RenderSide::kFull;

    SkV3       fLightVec      = {0,0,1},
               fLightColor    = {1,1,1};
    float      fAmbientLight  = 1,
               fDiffuseLight  = 0,
               fSpecularLight = 0,
               fSpecularExp   = 0;

    using INHERITED = sksg::CustomRenderNode;
};

class CylinderAdapter final : public DiscardableAdapterBase<CylinderAdapter, CylinderNode> {
public:
    CylinderAdapter(const skjson::ArrayValue& jprops,
                  const AnimationBuilder* abuilder,
                  sk_sp<CylinderNode> node)
        : INHERITED(std::move(node))
    {
        enum : size_t {
            kRadius_Index =  0,
            kPositionX_Index =  2,
            kPositionY_Index =  3,
            kRotX_Index =  7,
            kRotY_Index =  8,
            kRotZ_Index =  9,
            kRotOrder_Index = 10,
            kRender_Index = 12,
            kLightIntensity_Index = 14,
            kLightColor_Index = 15,
            kLightHeight_Index = 16,
            kLightDirection_Index = 17,
            kAmbient_Index = 20,
            kDiffuse_Index = 21,
            kSpecular_Index = 22,
            kRoughness_Index = 23,
        };

        EffectBinder(jprops, *abuilder, this)
            .bind(kRadius_Index, fRadius)
            .bind(kPositionX_Index, fPositionX)
            .bind(kPositionY_Index, fPositionY)
            .bind(kRotX_Index, fRotX)
            .bind(kRotY_Index, fRotY)
            .bind(kRotZ_Index, fRotZ)
            .bind(kRotOrder_Index, fRotOrder)
            .bind(kRender_Index, fRender)
            .bind(kLightIntensity_Index, fLightIntensity)
            .bind(kLightColor_Index, fLightColor)
            .bind(kLightHeight_Index, fLightHeight)
            .bind(kLightDirection_Index, fLightDirection)
            .bind(kAmbient_Index, fAmbient)
            .bind(kDiffuse_Index, fDiffuse)
            .bind(kSpecular_Index, fSpecular)
            .bind(kRoughness_Index, fRoughness);
    }

private:
    void onSync() override {
        const auto side = [](ScalarValue s) {
            switch (SkScalarRoundToInt(s)) {
                case 1:  return CylinderNode::RenderSide::kFull;
                case 2:  return CylinderNode::RenderSide::kOutside;
                case 3:
                default: return CylinderNode::RenderSide::kInside;
            }
            SkUNREACHABLE;
        };

        const auto rotation = [](ScalarValue order,
                                 ScalarValue x, ScalarValue y, ScalarValue z) {
            const SkM44 rx = SkM44::Rotate({1,0,0}, SkDegreesToRadians( x)),
                        ry = SkM44::Rotate({0,1,0}, SkDegreesToRadians( y)),
                        rz = SkM44::Rotate({0,0,1}, SkDegreesToRadians(-z));

            switch (SkScalarRoundToInt(order)) {
                case 1: return rx * ry * rz;
                case 2: return rx * rz * ry;
                case 3: return ry * rx * rz;
                case 4: return ry * rz * rx;
                case 5: return rz * rx * ry;
                case 6:
               default: return rz * ry * rx;
            }
            SkUNREACHABLE;
        };

        const auto light_vec = [](float height, float direction) {
            float z = std::sin(height * SK_ScalarPI / 2),
                  r = std::sqrt(1 - z*z),
                  x = std::cos(direction) * r,
                  y = std::sin(direction) * r;

            return SkV3{x,y,z};
        };

        const auto& cyl = this->node();

        cyl->setCenter({fPositionX, fPositionY});
        cyl->setRadius(fRadius);
        cyl->setSide(side(fRender));
        cyl->setRotation(rotation(fRotOrder, fRotX, fRotY, fRotZ));

        cyl->setAmbientLight (SkTPin(fAmbient * 0.01f, 0.0f, 2.0f));

        const auto intensity = SkTPin(fLightIntensity * 0.01f,  0.0f, 10.0f);
        cyl->setDiffuseLight (SkTPin(fDiffuse * 0.01f, 0.0f, 1.0f) * intensity);
        cyl->setSpecularLight(SkTPin(fSpecular* 0.01f, 0.0f, 1.0f) * intensity);

        cyl->setLightVec(light_vec(
            SkTPin(fLightHeight    * 0.01f, -1.0f,  1.0f),
            SkDegreesToRadians(fLightDirection - 90)
        ));

        const auto lc = static_cast<SkColor4f>(fLightColor);
        cyl->setLightColor({lc.fR, lc.fG, lc.fB});

        cyl->setSpecularExp(1/SkTPin(fRoughness, 0.001f, 0.5f));
    }

    ScalarValue fRadius   = 0,
                fPositionX = 0,
                fPositionY = 0,
                fRotX     = 0,
                fRotY     = 0,
                fRotZ     = 0,
                fRotOrder = 1,
                fRender   = 1;

    ColorValue  fLightColor;
    ScalarValue fLightIntensity =   0,
                fLightHeight    =   0,
                fLightDirection =   0,
                fAmbient        = 100,
                fDiffuse        =   0,
                fSpecular       =   0,
                fRoughness      =   0.5f;

    using INHERITED = DiscardableAdapterBase<CylinderAdapter, CylinderNode>;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachCylinderEffect(
        const skjson::ArrayValue& jprops, sk_sp<sksg::RenderNode> layer) const {
    auto cylinder = sk_make_sp<CylinderNode>(std::move(layer), fLayerSize);

    return fBuilder->attachDiscardableAdapter<CylinderAdapter>(jprops, fBuilder, std::move(cylinder));
}

} // namespace skottie::internal
