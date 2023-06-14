/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkM44.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#incldue "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkCPUTypes.h"
#include "include/private/base/SkSpan_impl.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/imagefilters/SkCropImageFilter.h"

#include <cstdint>
#include <utility>

namespace {
// The 3D points/vectors used for lighting don't have a great analog for the rest of the image
// filtering system, and don't have any representation for ParameterSpace and LayerSpace. The
// SVG spec is also vague on how to handle managing them. For some principle of least-surprise,
// the X and Y coordinates are treated as ParameterSpace<SkPoint|Vector> and the Z will be
// scaled by the average of the X and Y scale factors when tranforming to layer space.
struct ZValue {
    ZValue() : fZ(0.f) {}
    ZValue(float z) : fZ(z) {}
    operator float() const { return fZ; }

    float fZ;
};
} // anonymous namespace

namespace skif {
template<>
class LayerSpace<ZValue> {
public:
    LayerSpace() = default;
    explicit LayerSpace(ZValue z) : fData(z) {}

    float val() const { return fData.fZ; }

    static LayerSpace<ZValue> Map(const Mapping& mapping, ParameterSpace<ZValue> z) {
        // skif::Mapping and the image filter pipeline and the SVG spec don't have a great model
        // for how the depth components of lights and the alpha map are to be transformed. We
        // extend the mapping model to have the mapped Z value be the average of the mapped 2D
        // vector (z,z). For uniform scaling transforms, this has the desirable behavior of
        // uniformly scaling the Z axis as well.
        skif::LayerSpace<skif::Vector> z2d = mapping.paramToLayer(
                skif::ParameterSpace<skif::Vector>({ZValue(z), ZValue(z)}));
        return LayerSpace<ZValue>(SkScalarAve(z2d.x(), z2d.y()));
    }

private:
    ZValue fData;
};
} // namespace skif

namespace {

struct Light {
    enum class Type {
        kDistant,
        kPoint,
        kSpot,
        kLast = kSpot
    };

    Type fType;
    SkColor fLightColor; // All lights

    // Location and direction are decomposed into typed XY and Z for how they are transformed from
    // parameter space to layer space.
    skif::ParameterSpace<SkPoint> fLocationXY; // Spotlight and point lights only
    skif::ParameterSpace<ZValue>  fLocationZ;  //  ""

    skif::ParameterSpace<skif::Vector> fDirectionXY; // Spotlight and distant lights only
    skif::ParameterSpace<ZValue>       fDirectionZ;  //  ""

    // Spotlight only (and unchanged by layer matrix)
    float fFalloffExponent;
    float fCosCutoffAngle;

    static Light Point(SkColor color, const SkPoint3& location) {
        return {Type::kPoint,
                color,
                skif::ParameterSpace<SkPoint>({location.fX, location.fY}),
                skif::ParameterSpace<ZValue>(location.fZ),
                /*directionXY=*/{},
                /*directionZ=*/{},
                /*falloffExponent=*/0.f,
                /*cutoffAngle=*/0.f};
    }

    static Light Distant(SkColor color, const SkPoint3& direction) {
        return {Type::kDistant,
                color,
                /*locationXY=*/{},
                /*locationZ=*/{},
                skif::ParameterSpace<skif::Vector>({direction.fX, direction.fY}),
                skif::ParameterSpace<ZValue>(direction.fZ),
                /*falloffExponent=*/0.f,
                /*cutoffAngle=*/0.f};
    }

    static Light Spot(SkColor color, const SkPoint3& location, const SkPoint3& direction,
                      float falloffExponent, float cosCutoffAngle) {
        return {Type::kSpot,
                color,
                skif::ParameterSpace<SkPoint>({location.fX, location.fY}),
                skif::ParameterSpace<ZValue>(location.fZ),
                skif::ParameterSpace<skif::Vector>({direction.fX, direction.fY}),
                skif::ParameterSpace<ZValue>(direction.fZ),
                falloffExponent,
                cosCutoffAngle};
    }
};

struct Material {
    enum class Type {
        kDiffuse,
        kSpecular,
        kLast = kSpecular
    };

    Type fType;
    // The base scale factor applied to alpha image to go from [0-1] to [0-depth] before computing
    // surface normals. Since this is a Z component, there's no direct relation with the Mapping's
    // layer matrix. This will be scaled manually by the average of the X and Y scale factors to
    // remain consistent with how lights are transformed.
    skif::ParameterSpace<ZValue> fSurfaceDepth;

    // Non-geometric
    float fK; // Reflectance coefficient
    float fShininess; // Specular only

    static Material Diffuse(float k, float surfaceDepth) {
        return {Type::kDiffuse, skif::ParameterSpace<ZValue>(surfaceDepth), k, 0.f};
    }

    static Material Specular(float k, float shininess, float surfaceDepth) {
        return {Type::kSpecular, skif::ParameterSpace<ZValue>(surfaceDepth), k, shininess};
    }
};

class SkLightingImageFilter final : public SkImageFilter_Base {
public:
    SkLightingImageFilter(const Light& light, const Material& material, sk_sp<SkImageFilter> input)
            : SkImageFilter_Base(&input, 1, nullptr)
            , fLight(light)
            , fMaterial(material) {}

    SkRect computeFastBounds(const SkRect& src) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterLightingImageFilterFlattenables();
    SK_FLATTENABLE_HOOKS(SkLightingImageFilter)
    static Light LegacyDeserializeLight(SkReadBuffer& buffer);
    static sk_sp<SkFlattenable> LegacyDiffuseCreateProc(SkReadBuffer& buffer);
    static sk_sp<SkFlattenable> LegacySpecularCreateProc(SkReadBuffer& buffer);

    bool onAffectsTransparentBlack() const override { return true; }

    skif::FilterResult onFilterImage(const skif::Context&) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    skif::LayerSpace<SkIRect> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    skif::LayerSpace<SkIRect> requiredInput(const skif::LayerSpace<SkIRect>& desiredOutput) const {
        // We request 1px of padding so that the visible normal map can do a regular Sobel kernel
        // eval. If there's a crop rect involved, the actual input image may not include the extra
        // data, in which case the shader will apply modified Sobel kernel weights for 8 edge
        // possibilities.
        skif::LayerSpace<SkIRect> requiredInput = desiredOutput;
        // The Sobel kernel is always applied in layer pixels
        requiredInput.outset(skif::LayerSpace<SkISize>({1, 1}));
        return requiredInput;
    }

    Light fLight;
    Material fMaterial;
};

// Creates a shader that performs a Sobel filter on the alpha channel of the input image, using
// 'edgeBounds' to decide how to modify the kernel weights.
sk_sp<SkShader> make_normal_shader(sk_sp<SkShader> alphaMap,
                                   const skif::LayerSpace<SkIRect>& edgeBounds,
                                   skif::LayerSpace<SkSize> surfaceXYScale,
                                   skif::LayerSpace<ZValue> surfaceDepth) {
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "uniform shader alphaMap;"
        "uniform float4 edgeBounds;"
        "uniform half2 surfaceScale;"

        "const half3x3 kTLx = half3x3( 0, 0, 0,   0,-2,-1,   0, 2, 1) * 2.0/3.0;"
        "const half3x3 kTLy = half3x3( 0, 0, 0,   0,-2, 2,   0,-1, 1) * 2.0/3.0;"
        "const half3x3  kLx = half3x3( 0, 0, 0,  -1,-2,-1,   1, 2, 1) * 1.0/2.0;"
        "const half3x3  kLy = half3x3( 0, 0, 0,  -2, 0, 2,  -1, 0, 1) * 1.0/3.0;"
        "const half3x3 kBLx = half3x3( 0, 0, 0,  -1,-2, 0,   1, 2, 0) * 2.0/3.0;"
        "const half3x3 kBLy = half3x3( 0, 0, 0,  -2, 2, 0,  -1, 1, 0) * 2.0/3.0;"

        "const half3x3  kTx = half3x3( 0,-2,-1,   0, 0, 0,   0, 2, 1) * 1.0/3.0;"
        "const half3x3  kTy = half3x3( 0,-1, 1,   0,-2, 2,   0,-1, 1) * 1.0/2.0;"
        "const half3x3  kCx = half3x3(-1,-2,-1,   0, 0, 0,   1, 2, 1) * 1.0/4.0;"
        "const half3x3  kCy = half3x3(-1, 0, 1,  -2, 0, 2,  -1, 0, 1) * 1.0/4.0;"
        "const half3x3  kBx = half3x3(-1,-2, 0,   0, 0, 0,   1, 2, 0) * 1.0/3.0;"
        "const half3x3  kBy = half3x3(-1, 1, 0,  -2, 2, 0,  -1, 1, 0) * 1.0/2.0;"

        "const half3x3 kTRx = half3x3( 0,-2,-1,   0, 2, 1,   0, 0, 0) * 2.0/3.0;"
        "const half3x3 kTRy = half3x3( 0,-1, 1,   0,-2, 2,   0, 0, 0) * 2.0/3.0;"
        "const half3x3  kRx = half3x3(-1,-2,-1,   1, 2, 1,   0, 0, 0) * 1.0/2.0;"
        "const half3x3  kRy = half3x3(-1, 0, 1,  -2, 0, 2,   0, 0, 0) * 1.0/3.0;"
        "const half3x3 kBRx = half3x3(-1,-2, 0,   1, 2, 0,   0, 0, 0) * 2.0/3.0;"
        "const half3x3 kBRy = half3x3(-1, 1, 0,  -2, 2, 0,   0, 0, 0) * 2.0/3.0;"

        // x and y actually hold the dz.
        //
        // so dz*surfaceDepth is the amount of change across a pixel.
        //
        //   so surface slope is dz*surfaceDepth, orthogonal to that would be (-dz*surfaceDepth, 1)
        //
        // but if are scaling everything uniformly, the parameter pixel is actually S pixels,
        // but the change across the pixel becomes dz*surfaceDepth*S
        //
        // so we have (-dz*surfaceDepth*S, 1/S)
        "half3 point_to_normal(half x, half y) {"
            "return normalize(half3(-surfaceScale * half2(x, y), 1));"
        "}"

        // TODO: remove weight if we can fold into the kernel constants
        "half sobel(half3x3 alphas, half3x3 k, half weight) {"
            "return /*weight * */ (dot(alphas[0], k[0]) + "
                             "dot(alphas[1], k[1]) + "
                             "dot(alphas[2], k[2]));"
        "}"

        "half3 normal(float2 coord, half3x3 alphas) {"
            "if (coord.x <= edgeBounds.L) {"
                "if (coord.y <= edgeBounds.T) {"
                    "return point_to_normal(sobel(alphas, kTLx, 2.0/3.0),"
                                           "sobel(alphas, kTLy, 2.0/3.0));"
                "} else if (coord.y >= edgeBounds.B) {"
                    "return point_to_normal(sobel(alphas, kBLx, 2.0/3.0),"
                                           "sobel(alphas, kBLy, 2.0/3.0));"
                "} else {"
                    "return point_to_normal(sobel(alphas,  kLx, 1.0/2.0),"
                                           "sobel(alphas,  kLy, 1.0/3.0));"
                "}"
            "} else if (coord.x >= edgeBounds.R) {"
                "if (coord.y <= edgeBounds.T) {"
                    "return point_to_normal(sobel(alphas, kTRx, 2.0/3.0),"
                                           "sobel(alphas, kTRy, 2.0/3.0));"
                "} else if (coord.y >= edgeBounds.B) {"
                    "return point_to_normal(sobel(alphas, kBRx, 2.0/3.0),"
                                           "sobel(alphas, kBRy, 2.0/3.0));"
                "} else {"
                    "return point_to_normal(sobel(alphas,  kRx, 1.0/2.0),"
                                           "sobel(alphas,  kRy, 1.0/3.0));"
                "}"
            "} else {"
                "if (coord.y <= edgeBounds.T) {"
                    "return point_to_normal(sobel(alphas,  kTx, 1.0/3.0),"
                                           "sobel(alphas,  kTy, 1.0/2.0));"
                "} else if (coord.y >= edgeBounds.B) {"
                    "return point_to_normal(sobel(alphas,  kBx, 1.0/3.0),"
                                           "sobel(alphas,  kBy, 1.0/2.0));"
                "} else {"
                    "return point_to_normal(sobel(alphas,  kCx, 1.0/4.0),"
                                           "sobel(alphas,  kCy, 1.0/4.0));"
                "}"
            "}"
        "}"

        "half4 main(float2 coord) {"
            "half mainAlpha = alphaMap.eval(coord).a;"
            "half3 alphaC0 = half3("
                "alphaMap.eval(coord + float2(-1,-1)).a,"
                "alphaMap.eval(coord + float2(-1, 0)).a,"
                "alphaMap.eval(coord + float2(-1, 1)).a);"
            "half3 alphaC1 = half3("
                "alphaMap.eval(coord + float2( 0,-1)).a,"
                "mainAlpha,"
                "alphaMap.eval(coord + float2( 0, 1)).a);"
            "half3 alphaC2 = half3("
                "alphaMap.eval(coord + float2( 1,-1)).a,"
                "alphaMap.eval(coord + float2( 1, 0)).a,"
                "alphaMap.eval(coord + float2( 1, 1)).a);"

            "return half4(normal(coord, half3x3(alphaC0, alphaC1, alphaC2)), mainAlpha);"
        "}");

    SkRuntimeShaderBuilder builder(sk_ref_sp(effect));
    builder.child("alphaMap") = std::move(alphaMap);
    builder.uniform("edgeBounds") = SkRect::Make(SkIRect(edgeBounds));
    builder.uniform("surfaceScale") = SkV2{surfaceDepth.val()*surfaceDepth.val() / surfaceXYScale.width(), surfaceDepth.val()*surfaceDepth.val() / surfaceXYScale.height()};

    return builder.makeShader();
}

// FIXME not actually light and material, but the transformed state.
sk_sp<SkShader> make_lighting_shader(sk_sp<SkShader> normalMap,
                                     Light::Type lightType,
                                     SkColor lightColor,
                                     skif::LayerSpace<SkPoint> locationXY,
                                     skif::LayerSpace<ZValue> locationZ,
                                     skif::LayerSpace<skif::Vector> directionXY,
                                     skif::LayerSpace<ZValue> directionZ,
                                     float falloffExponent,
                                     float cosCutoffAngle,
                                     Material::Type matType,
                                     skif::LayerSpace<ZValue> surfaceDepth,
                                     float k,
                                     float shininess) {
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        // Matches Light::Type
        "const int kDistant = 0;"
        "const int kPoint = 1;"
        "const int kSpot = 2;"

        // Matches Material::Type
        "const int kDiffuse = 0;"
        "const int kSpecular = 1;"

        "const half kConeAAThreshold = 0.016;"
        "const half kConeScale = 1.0 / kConeAAThreshold;"

        "uniform shader normalMap;"

        "uniform int lightType;"
        "uniform int matType;"

        "uniform float3 lightPos;"
        "uniform half3 lightDir;"
        "uniform half3 lightColor;" // Assumed to have material's k multiplied into it
        "uniform half falloffExponent;"
        "uniform half cosCutoffAngle;"

        "uniform half surfaceDepth;"
        "uniform half shininess;"

        // ordering of uniforms for better packing (adjust float3's to hold extra as float4?)

        "half3 surface_to_light(float3 coord) {"
            "if (lightType == kDistant) {"
                "return lightDir;"
            "} else {"
                // Spot and point have the same equation
                "return normalize(lightPos - coord);"
            "}"
        "}"

        "half spotlight_scale(half3 surfaceToLight) {"
            "half cosAngle = -dot(surfaceToLight, lightDir);"
            "if (cosAngle < cosCutoffAngle) {"
                "return 0.0;"
            "}"
            "half scale = pow(cosAngle, falloffExponent);"
            "if (cosAngle < cosCutoffAngle + kConeAAThreshold) {"
                "return scale * (cosAngle - cosCutoffAngle) * kConeScale;"
            "} else {"
                "return scale;"
            "}"
        "}"

        "half4 compute_lighting(half3 normal, half3 surfaceToLight) {"
            // Point and distant light color contributions are constant
            "half3 color = lightColor;"
            // Spot lights fade based on the angle away from its direction
            "if (lightType == kSpot) {"
                "color *= spotlight_scale(surfaceToLight);"
            "}"

            // Diffuse and specular reflections scale the light's "color" differently
            "if (matType == kDiffuse) {"
                "half coeff = dot(normal, surfaceToLight);"
                "color = saturate(coeff * color);"
                "return half4(color, 1.0);"
            "} else {"
                "half3 halfDir = normalize(surfaceToLight + half3(0, 0, 1));"
                "half coeff = pow(dot(normal, halfDir), shininess);"
                "color = saturate(coeff * color);"
                "return half4(color, max(max(color.r, color.g), color.b));"
            "}"
        "}"

        "half4 main(float2 coord) {"
            "half4 normalAndA = normalMap.eval(coord);"
            "half3 surfaceToLight = surface_to_light(float3(coord, normalAndA.a * surfaceDepth));"
            "return compute_lighting(normalAndA.xyz, surfaceToLight);"
            //"return half4(normalAndA.xyz*0.5 + 0.5, 1.0);"
        "}");

    SkRuntimeShaderBuilder builder(sk_ref_sp(effect));
    builder.child("normalMap") = std::move(normalMap);

    builder.uniform("lightType") = (int32_t) lightType;
    builder.uniform("matType") = (int32_t) matType;

    builder.uniform("lightPos") = SkV3{locationXY.x(), locationXY.y(), locationZ.val()};

    // Pre-normalize the light direction, but this can be (0,0,0) for point lights, which won't use
    // the uniform anyways. Avoid a division by 0 to keep ASAN happy or in the event that a spot/dir
    // light have bad user input.
    SkV3 dir{directionXY.x(), directionXY.y(), directionZ.val()};
    float invDirLen = dir.length();
    invDirLen = invDirLen ? 1.0f / invDirLen : 0.f;
    builder.uniform("lightDir") = (invDirLen * dir);

    const float colorScale = k / 255.f;
    builder.uniform("lightColor") = SkV3{SkColorGetR(lightColor) * colorScale,
                                         SkColorGetG(lightColor) * colorScale,
                                         SkColorGetB(lightColor) * colorScale};
    builder.uniform("falloffExponent") = falloffExponent;
    builder.uniform("cosCutoffAngle") = cosCutoffAngle;
    builder.uniform("surfaceDepth") = surfaceDepth.val();
    builder.uniform("shininess") = shininess;

    return builder.makeShader();
}

sk_sp<SkImageFilter> make_lighting(const Light& light,
                                   const Material& material,
                                   sk_sp<SkImageFilter> input,
                                   const SkImageFilters::CropRect& cropRect) {
    // According to the spec, ks and kd can be any non-negative number:
    // http://www.w3.org/TR/SVG/filters.html#feSpecularLightingElement
    if (!SkScalarIsFinite(material.fK) || material.fK < 0.f ||
        !SkScalarIsFinite(material.fShininess) ||
        !SkScalarIsFinite(ZValue(material.fSurfaceDepth))) {
        return nullptr;
    }

    // FIXME validate lighting vectors as non-finite

    sk_sp<SkImageFilter> filter(new SkLightingImageFilter(light, material, std::move(input)));
    if (cropRect) {
        filter = SkMakeCropImageFilter(*cropRect, std::move(filter));
    }
    return filter;
}

} // anonymous namespace

sk_sp<SkImageFilter> SkImageFilters::DistantLitDiffuse(
        const SkPoint3& direction, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return make_lighting(Light::Distant(lightColor, direction),
                         Material::Diffuse(kd, surfaceScale),
                         std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::PointLitDiffuse(
        const SkPoint3& location, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return make_lighting(Light::Point(lightColor, location),
                         Material::Diffuse(kd, surfaceScale),
                         std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::SpotLitDiffuse(
        const SkPoint3& location, const SkPoint3& target, SkScalar falloffExponent,
        SkScalar cutoffAngle, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    SkPoint3 dir = target - location;
    float cosCutoffAngle = SkScalarCos(SkDegreesToRadians(cutoffAngle));
    return make_lighting(Light::Spot(lightColor, location, dir, falloffExponent, cosCutoffAngle),
                         Material::Diffuse(kd, surfaceScale),
                         std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::DistantLitSpecular(
        const SkPoint3& direction, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return make_lighting(Light::Distant(lightColor, direction),
                         Material::Specular(ks, shininess, surfaceScale),
                         std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::PointLitSpecular(
        const SkPoint3& location, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return make_lighting(Light::Point(lightColor, location),
                         Material::Specular(ks, shininess, surfaceScale),
                         std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::SpotLitSpecular(
        const SkPoint3& location, const SkPoint3& target, SkScalar falloffExponent,
        SkScalar cutoffAngle, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    SkPoint3 dir = target - location;
    float cosCutoffAngle = SkScalarCos(SkDegreesToRadians(cutoffAngle));
    return make_lighting(Light::Spot(lightColor, location, dir, falloffExponent, cosCutoffAngle),
                         Material::Specular(ks, shininess, surfaceScale),
                         std::move(input), cropRect);
}

void SkRegisterLightingImageFilterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkLightingImageFilter);
    // TODO (michaelludwig): Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkDiffuseLightingImageFilter",
                            SkLightingImageFilter::LegacyDiffuseCreateProc);
    SkFlattenable::Register("SkSpecularLightingImageFilter",
                            SkLightingImageFilter::LegacySpecularCreateProc);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkLightingImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    Light light;
    light.fType = buffer.read32LE(Light::Type::kLast);
    light.fLightColor = buffer.readColor();

    SkPoint3 lightPos, lightDir;
    buffer.readPoint3(&lightPos);
    light.fLocationXY = skif::ParameterSpace<SkPoint>({lightPos.fX, lightPos.fY});
    light.fLocationZ = skif::ParameterSpace<ZValue>(lightPos.fZ);

    buffer.readPoint3(&lightDir);
    light.fDirectionXY = skif::ParameterSpace<skif::Vector>({lightDir.fX, lightDir.fY});
    light.fDirectionZ = skif::ParameterSpace<ZValue>(lightDir.fZ);

    light.fFalloffExponent = buffer.readScalar();
    light.fCosCutoffAngle = buffer.readScalar();

    Material material;
    material.fType = buffer.read32LE(Material::Type::kLast);
    material.fSurfaceDepth = skif::ParameterSpace<ZValue>(buffer.readScalar());
    material.fK = buffer.readScalar();
    material.fShininess = buffer.readScalar();

    return make_lighting(light, material, common.getInput(0), common.cropRect());
}

Light SkLightingImageFilter::LegacyDeserializeLight(SkReadBuffer& buffer) {
    // Light::Type has the same order as the legacy SkImageFilterLight::LightType enum
    Light::Type lightType = buffer.read32LE(Light::Type::kLast);
    if (!buffer.isValid()) {
        return {};
    }

    // Legacy lights stored just the RGB, but as floats (notably *not* normalized to [0-1])
    SkColor lightColor = SkColorSetARGB(/*a (ignored)=*/255,
                                        /*r=*/  (U8CPU) buffer.readScalar(),
                                        /*g=*/(U8CPU) buffer.readScalar(),
                                        /*b=*/ (U8CPU) buffer.readScalar());
    // Legacy lights only serialized fields specific to that type
    switch (lightType) {
        case Light::Type::kDistant: {
            SkPoint3 dir = {buffer.readScalar(), buffer.readScalar(), buffer.readScalar()};
            return Light::Distant(lightColor, dir);
        }
        case Light::Type::kPoint: {
            SkPoint3 loc = {buffer.readScalar(), buffer.readScalar(), buffer.readScalar()};
            return Light::Point(lightColor, loc);
        }
        case Light::Type::kSpot: {
            SkPoint3 loc = {buffer.readScalar(), buffer.readScalar(), buffer.readScalar()};
            SkPoint3 target = {buffer.readScalar(), buffer.readScalar(), buffer.readScalar()};
            float falloffExponent = buffer.readScalar();
            float cosOuterConeAngle = buffer.readScalar();
            buffer.readScalar(); // skip cosInnerConeAngle, derived from outer cone angle
            buffer.readScalar(); // skip coneScale, which is a constant
            buffer.readScalar(); // skip S, which is normalize(target - loc)
            buffer.readScalar(); //  ""
            buffer.readScalar(); //  ""
            return Light::Spot(lightColor, loc, target - loc, falloffExponent, cosOuterConeAngle);
        }
    }

    SkUNREACHABLE; // Validation by read32LE() should avoid this
}

sk_sp<SkFlattenable> SkLightingImageFilter::LegacyDiffuseCreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    Light light = LegacyDeserializeLight(buffer);

    // Legacy implementations used (scale/255) when filtering, but serialized (fScale*255) so the
    // buffer held the original unmodified surface scale. Presumably, removing the 255 factor from
    // fScale corrected the light colors being defined in terms of [0-255] (verify...)
    // Ganesh FPs used (fScale*255) so unclear why they are dividing...
    float surfaceScale = buffer.readScalar();
    float kd = buffer.readScalar();
    Material material = Material::Diffuse(kd, surfaceScale);

    return make_lighting(light, material, common.getInput(0), common.cropRect());
}

sk_sp<SkFlattenable> SkLightingImageFilter::LegacySpecularCreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    Light light = LegacyDeserializeLight(buffer);

    // Legacy implementations used (scale/255) when filtering, but serialized (fScale*255) so the
    // buffer held the original unmodified surface scale. Presumably, removing the 255 factor from
    // fScale corrected the light colors being defined in terms of [0-255] (verify...)
    float surfaceScale = buffer.readScalar();
    float ks = buffer.readScalar();
    float shininess = buffer.readScalar();
    Material material = Material::Specular(ks, shininess, surfaceScale);

    return make_lighting(light, material, common.getInput(0), common.cropRect());
}

void SkLightingImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);

    // Light
    buffer.writeInt((int) fLight.fType);
    buffer.writeColor(fLight.fLightColor);

    buffer.writePoint(SkPoint(fLight.fLocationXY));
    buffer.writeScalar(ZValue(fLight.fLocationZ));

    skif::Vector dirXY{fLight.fDirectionXY};
    buffer.writePoint(SkPoint{dirXY.fX, dirXY.fY});
    buffer.writeScalar(ZValue(fLight.fDirectionZ));
    buffer.writeScalar(fLight.fFalloffExponent);
    buffer.writeScalar(fLight.fCosCutoffAngle);

    // Material
    buffer.writeInt((int) fMaterial.fType);
    buffer.writeScalar(ZValue(fMaterial.fSurfaceDepth));
    buffer.writeScalar(fMaterial.fK);
    buffer.writeScalar(fMaterial.fShininess);
}

///////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkLightingImageFilter::onFilterImage(const skif::Context& ctx) const {
    using ShaderFlags = skif::FilterResult::ShaderFlags;

    auto mapZToLayer = [&ctx](skif::ParameterSpace<ZValue> z) {
        return skif::LayerSpace<ZValue>::Map(ctx.mapping(), z);
    };

    skif::LayerSpace<SkIRect> requiredInput = this->requiredInput(ctx.desiredOutput());

    // ZValue has a custom param->layer space mapping function that's not defined on skif::Mapping
    skif::LayerSpace<SkSize> surfaceXYScale = ctx.mapping().paramToLayer(
            skif::ParameterSpace<SkSize>({1.f, 1.f}));
    skif::LayerSpace<ZValue> surfaceDepth = mapZToLayer(fMaterial.fSurfaceDepth);

    skif::LayerSpace<SkPoint> lightLocationXY = ctx.mapping().paramToLayer(fLight.fLocationXY);
    skif::LayerSpace<ZValue> lightLocationZ = mapZToLayer(fLight.fLocationZ); // average of (z,z) mapped as a vector

    skif::LayerSpace<skif::Vector> lightDirXY = ctx.mapping().paramToLayer(fLight.fDirectionXY);
    skif::LayerSpace<ZValue> lightDirZ = mapZToLayer(fLight.fDirectionZ);

    skif::FilterResult childOutput = this->getChildOutput(0, ctx.withNewDesiredOutput(requiredInput));

    skif::FilterResult::Builder builder{ctx};
    // The normal map shader will use the bounds of the actual input image to use an edge-aware
    // Sobel filter. Samples beyond those bounds (e.g. transparent black) will not actually impact
    // the normal equation. If the full required input was provided (no up-graph cropping), then
    // the edge normals won't be visible; visible normals would be derived from the interior Sobel.
    // Thus we override the default sample bounds (final output) to be the layer bounds.
    builder.add(childOutput, childOutput.layerBounds());
    return builder.eval([&](SkSpan<sk_sp<SkShader>> input) {
        // TODO: Once shaders are deferred in FilterResult, it will likely make sense to have an
        // internal normal map filter that uses this shader, and then have the lighting effects as
        // a separate filter. It's common for multiple lights to use the same input (producing the
        // same normal map) before being merged together. With a separate normal image filter, its
        // output would be automatically cached, and the lighting equation shader would be deferred
        // to the merge's draw operation, making for a maximum of 2 renderpasses instead of N+1.
        sk_sp<SkShader> normals = make_normal_shader(std::move(input[0]), childOutput.layerBounds(), surfaceXYScale, surfaceDepth);
        return make_lighting_shader(std::move(normals), fLight.fType, fLight.fLightColor, lightLocationXY, lightLocationZ, lightDirXY, lightDirZ, fLight.fFalloffExponent, fLight.fCosCutoffAngle, fMaterial.fType, surfaceDepth, fMaterial.fK, fMaterial.fShininess);
    }, ShaderFlags::kNone);
}

skif::LayerSpace<SkIRect> SkLightingImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    skif::LayerSpace<SkIRect> requiredInput = this->requiredInput(desiredOutput);
    return this->getChildInputLayerBounds(0, mapping, requiredInput, contentBounds);
}

skif::LayerSpace<SkIRect> SkLightingImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // The lighting equation is defined on the entire plane, even if the input image that defines
    // the normal map is bounded. It just is evaluated at a constant normal vector, which can still
    // produce non-constant color since the direction to the eye and light change per pixel.
    return skif::LayerSpace<SkIRect>(SkRectPriv::MakeILarge());
}

SkRect SkLightingImageFilter::computeFastBounds(const SkRect& src) const {
    return SkRectPriv::MakeLargeS32();
}
