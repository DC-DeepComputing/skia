/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkKnownRuntimeEffects.h"

#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkBlurEngine.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/effects/imagefilters/SkMatrixConvolutionImageFilter.h"

namespace SkKnownRuntimeEffects {

namespace {

SkRuntimeEffect::Options get_options(StableKey stableKey) {
    SkRuntimeEffect::Options options;
    SkRuntimeEffectPriv::SetStableKeyOnOptions(&options, static_cast<uint32_t>(stableKey));
    SkRuntimeEffectPriv::AllowPrivateAccess(&options);
    return options;
}

SkRuntimeEffect* make_effect(
        SkRuntimeEffect::Result (*make)(SkString, const SkRuntimeEffect::Options&),
        StableKey stableKey,
        const char* code) {
    return SkMakeRuntimeEffect(make, code, get_options(stableKey));
}

SkRuntimeEffect* make_blur_1D_shader(int kernelWidth, StableKey stableKey) {
    SkASSERT(kernelWidth <= SkShaderBlurAlgorithm::kMaxSamples);
    // The SkSL structure performs two kernel taps; if the kernel has an odd width the last
    // sample will be skipped with the current loop limit calculation.
    SkASSERT(kernelWidth % 2 == 0);
    return make_effect(SkRuntimeEffect::MakeForShader, stableKey,
            SkStringPrintf(
                    // The coefficients are always stored for the max radius to keep the
                    // uniform block consistent across all effects.
                    "const int kMaxUniformKernelSize = %d / 2;"
                    // But we generate an exact loop over the kernel size. Note that this
                    // program can be used for kernels smaller than the constructed max as long
                    // as the kernel weights for excess entries are set to 0.
                    "const int kMaxLoopLimit = %d / 2;"

                    "uniform half4 offsetsAndKernel[kMaxUniformKernelSize];"
                    "uniform half2 dir;"

                    "uniform shader child;"

                    "half4 main(float2 coord) {"
                        "half4 sum = half4(0);"
                        "for (int i = 0; i < kMaxLoopLimit; ++i) {"
                            "half4 s = offsetsAndKernel[i];"
                            "sum += s.y * child.eval(coord + s.x*dir);"
                            "sum += s.w * child.eval(coord + s.z*dir);"
                        "}"
                        "return sum;"
                    "}", SkShaderBlurAlgorithm::kMaxSamples, kernelWidth).c_str());
}

SkRuntimeEffect* make_blur_2D_shader(int maxKernelSize, StableKey stableKey) {
    SkASSERT(maxKernelSize <= SkShaderBlurAlgorithm::kMaxSamples);
    SkASSERT(maxKernelSize % 4 == 0);
    return make_effect(SkRuntimeEffect::MakeForShader, stableKey,
            SkStringPrintf(
                    // The coefficients are always stored for the max radius to keep the
                    // uniform block consistent across all effects.
                    "const int kMaxUniformKernelSize = %d / 4;"
                    "const int kMaxUniformOffsetsSize = 2*kMaxUniformKernelSize;"
                    // But we generate an exact loop over the kernel size. Note that this
                    // program can be used for kernels smaller than the constructed max as long
                    // as the kernel weights for excess entries are set to 0.
                    "const int kMaxLoopLimit = %d / 4;"

                    // Pack scalar coefficients into half4 for better packing on std140, and
                    // upload offsets to avoid having to transform the 1D index into a 2D coord
                    "uniform half4 kernel[kMaxUniformKernelSize];"
                    "uniform half4 offsets[kMaxUniformOffsetsSize];"

                    "uniform shader child;"

                    "half4 main(float2 coord) {"
                        "half4 sum = half4(0);"

                        "for (int i = 0; i < kMaxLoopLimit; ++i) {"
                            "half4 k = kernel[i];"
                            "half4 o = offsets[2*i];"
                            "sum += k.x * child.eval(coord + o.xy);"
                            "sum += k.y * child.eval(coord + o.zw);"
                            "o = offsets[2*i + 1];"
                            "sum += k.z * child.eval(coord + o.xy);"
                            "sum += k.w * child.eval(coord + o.zw);"
                        "}"
                        "return sum;"
                    "}", SkShaderBlurAlgorithm::kMaxSamples, maxKernelSize).c_str());
}

#define MAKE_BLUR_SHADER_FN(N) \
SkRuntimeEffect* make_1DBlur##N() { return make_blur_1D_shader(N, StableKey::k1DBlur##N); } \
SkRuntimeEffect* make_2DBlur##N() { return make_blur_2D_shader(N, StableKey::k2DBlur##N); }
MAKE_BLUR_SHADER_FN(4)
MAKE_BLUR_SHADER_FN(8)
MAKE_BLUR_SHADER_FN(12)
MAKE_BLUR_SHADER_FN(16)
MAKE_BLUR_SHADER_FN(20)
MAKE_BLUR_SHADER_FN(28)
#undef MAKE_BLUR_SHADER_FN

SkRuntimeEffect* make_Blend() {
    return make_effect(SkRuntimeEffect::MakeForShader, StableKey::kBlend,
        "uniform shader s, d;"
        "uniform blender b;"
        "half4 main(float2 xy) {"
            "return b.eval(s.eval(xy), d.eval(xy));"
        "}");
}

SkRuntimeEffect* make_Lerp() {
    return make_effect(SkRuntimeEffect::MakeForColorFilter, StableKey::kLerp,
        "uniform colorFilter cf0;"
        "uniform colorFilter cf1;"
        "uniform half weight;"

        "half4 main(half4 color) {"
            "return mix(cf0.eval(color), cf1.eval(color), weight);"
        "}");
}

// There are three shader variants:
//   1.   A smaller kernel version that stores the matrix in uniforms and iterates in 1D.
//   2+3. A larger kernel version that stores the matrix in a 1D textureThe texture version has
//        small and large variants w/ the actual kernel size uploaded as a uniform.
SkRuntimeEffect* make_matrix_conv_shader(int maxTextureKernelSize, StableKey stableKey) {
    // While the uniforms and kernel access are different, pieces of the algorithm are common and
    // defined statically for re-use in the two shaders:
    static const char* kHeaderAndBeginLoopSkSL =
        "uniform int2 size;"
        "uniform int2 offset;"
        "uniform half2 gainAndBias;"
        "uniform int convolveAlpha;" // FIXME not a full  int? Put in a half3 w/ gainAndBias?

        "uniform shader child;"

        "half4 main(float2 coord) {"
            "half4 sum = half4(0);"
            "half origAlpha = 0;"
            "int2 kernelPos = int2(0);"
            "for (int i = 0; i < kMaxKernelSize; ++i) {"
                "if (kernelPos.y >= size.y) { break; }";

    // Used in the inner loop to accumulate convolution sum and increment the kernel position
    static const char* kAccumulateAndIncrementSkSL =
                "half4 c = child.eval(coord + half2(kernelPos) - half2(offset));"
                "if (convolveAlpha == 0) {"
                    // When not convolving alpha, remember the original alpha for actual sample
                    // coord, and perform accumulation on unpremul colors.
                    "if (kernelPos == offset) {"
                        "origAlpha = c.a;"
                    "}"
                    "c = unpremul(c);"
                "}"
                "sum += c*k;"
                "kernelPos.x += 1;"
                "if (kernelPos.x >= size.x) {"
                    "kernelPos.x = 0;"
                    "kernelPos.y += 1;"
                "}";

    // Closes the loop and calculates final color
    static const char* kCloseLoopAndFooterSkSL =
            "}"
            "half4 color = sum*gainAndBias.x + gainAndBias.y;"
            "if (convolveAlpha == 0) {"
                // Reset the alpha to the original and convert to premul RGB
                "color = half4(color.rgb*origAlpha, origAlpha);"
            "} else {"
                // Ensure convolved alpha is within [0, 1]
                "color.a = saturate(color.a);"
            "}"
            // Make RGB valid premul w/ respect to the alpha (either original or convolved)
            "color.rgb = clamp(color.rgb, 0, color.a);"
            "return color;"
        "}";

    if (maxTextureKernelSize <= 0) {
        return make_effect(SkRuntimeEffect::MakeForShader, stableKey,
                SkStringPrintf("const int kMaxKernelSize = %d / 4;"
                                "uniform half4 kernel[kMaxKernelSize];"
                                "%s" // kHeaderAndBeginLoopSkSL
                                        "half4 k4 = kernel[i];"
                                        "for (int j = 0; j < 4; ++j) {"
                                            "if (kernelPos.y >= size.y) { break; }"
                                            "half k = k4[j];"
                                            "%s" // kAccumulateAndIncrementSkSL
                                        "}"
                                "%s", // kCloseLoopAndFooterSkSL
                                MatrixConvolutionImageFilter::kMaxUniformKernelSize,
                                kHeaderAndBeginLoopSkSL,
                                kAccumulateAndIncrementSkSL,
                                kCloseLoopAndFooterSkSL).c_str());
    } else {
        return make_effect(SkRuntimeEffect::MakeForShader, stableKey,
                SkStringPrintf("const int kMaxKernelSize = %d;"
                                "uniform shader kernel;"
                                "uniform half2 innerGainAndBias;"
                                "%s" // kHeaderAndBeginLoopSkSL
                                        "half k = kernel.eval(half2(half(i) + 0.5, 0.5)).a;"
                                        "k = k * innerGainAndBias.x + innerGainAndBias.y;"
                                        "%s" // kAccumulateAndIncrementSkSL
                                "%s", // kCloseLoopAndFooterSkSL
                                maxTextureKernelSize,
                                kHeaderAndBeginLoopSkSL,
                                kAccumulateAndIncrementSkSL,
                                kCloseLoopAndFooterSkSL).c_str());
    }
}

SkRuntimeEffect* make_MatrixConvUniforms() {
    return make_matrix_conv_shader(/*maxTextureKernelSize=*/-1,
                                   StableKey::kMatrixConvUniforms);
}
SkRuntimeEffect* make_MatrixConvTexSm() {
    return make_matrix_conv_shader(MatrixConvolutionImageFilter::kSmallKernelSize,
                                   StableKey::kMatrixConvTexSm);
}
SkRuntimeEffect* make_MatrixConvTexLg() {
    return make_matrix_conv_shader(MatrixConvolutionImageFilter::kLargeKernelSize,
                                   StableKey::kMatrixConvTexLg);
}

SkRuntimeEffect* make_Decal() {
    return make_effect(SkRuntimeEffect::MakeForShader, StableKey::kDecal,
        "uniform shader image;"
        "uniform float4 decalBounds;"

        "half4 main(float2 coord) {"
            "return sk_decal(image, coord, decalBounds);"
        "}");
}

SkRuntimeEffect* make_Displacement() {
    // NOTE: This uses dot product selection to work on all GLES2 hardware (enforced by
    // public runtime effect restrictions). Otherwise, this would use a "uniform ivec2"
    // and component indexing to convert the displacement color into a vector.
    return make_effect(SkRuntimeEffect::MakeForShader, StableKey::kDisplacement,
        "uniform shader displMap;"
        "uniform shader colorMap;"
        "uniform half2 scale;"
        "uniform half4 xSelect;" // Only one of RGBA will be 1, the rest are 0
        "uniform half4 ySelect;"

        "half4 main(float2 coord) {"
            "return sk_displacement(displMap, colorMap, coord, scale, xSelect, ySelect);"
        "}");
}

SkRuntimeEffect* make_Lighting() {
    return make_effect(SkRuntimeEffect::MakeForShader, StableKey::kLighting,
        "uniform shader normalMap;"

        // Packs surface depth, shininess, material type (0 == diffuse) and light type
        // (< 0 = distant, 0 = point, > 0 = spot)
        "uniform half4 materialAndLightType;"

        "uniform half4 lightPosAndSpotFalloff;" // (x,y,z) are lightPos, w is spot falloff
                                                // exponent
        "uniform half4 lightDirAndSpotCutoff;" // (x,y,z) are lightDir,
                                               // w is spot cos(cutoffAngle)
        "uniform half3 lightColor;" // Material's k has already been multiplied in

        "half4 main(float2 coord) {"
            "return sk_lighting(normalMap, coord,"
                                /*depth=*/"materialAndLightType.x,"
                                /*shininess=*/"materialAndLightType.y,"
                                /*materialType=*/"materialAndLightType.z,"
                                /*lightType=*/"materialAndLightType.w,"
                                /*lightPos=*/"lightPosAndSpotFalloff.xyz,"
                                /*spotFalloff=*/"lightPosAndSpotFalloff.w,"
                                /*lightDir=*/"lightDirAndSpotCutoff.xyz,"
                                /*cosCutoffAngle=*/"lightDirAndSpotCutoff.w,"
                                "lightColor);"
        "}");
}

SkRuntimeEffect* make_LinearMorphology() {
    return make_effect(SkRuntimeEffect::MakeForShader, StableKey::kLinearMorphology,
        "uniform shader child;"
        "uniform half2 offset;"
        "uniform half flip;" // -1 converts the max() calls to min()
        "uniform int radius;"

        "half4 main(float2 coord) {"
            "return sk_linear_morphology(child, coord, offset, flip, radius);"
        "}");
}

SkRuntimeEffect* make_SparseMorphology() {
    return make_effect(SkRuntimeEffect::MakeForShader, StableKey::kSparseMorphology,
        "uniform shader child;"
        "uniform half2 offset;"
        "uniform half flip;"

        "half4 main(float2 coord) {"
            "return sk_sparse_morphology(child, coord, offset, flip);"
        "}");
}

SkRuntimeEffect* make_Magnifier() {
return make_effect(SkRuntimeEffect::MakeForShader, StableKey::kMagnifier,
        "uniform shader src;"
        "uniform float4 lensBounds;"
        "uniform float4 zoomXform;"
        "uniform float2 invInset;"

        "half4 main(float2 coord) {"
            "return sk_magnifier(src, coord, lensBounds, zoomXform, invInset);"
        "}");
}

SkRuntimeEffect* make_Normal() {
    return make_effect(SkRuntimeEffect::MakeForShader, StableKey::kNormal,
        "uniform shader alphaMap;"
        "uniform float4 edgeBounds;"
        "uniform half negSurfaceDepth;"

        "half4 main(float2 coord) {"
           "return sk_normal(alphaMap, coord, edgeBounds, negSurfaceDepth);"
        "}");
}

SkRuntimeEffect* make_Arithmetic() {
    return make_effect(SkRuntimeEffect::MakeForBlender, StableKey::kArithmetic,
        "uniform half4 k;"
        "uniform half pmClamp;"

        "half4 main(half4 src, half4 dst) {"
            "return sk_arithmetic_blend(src, dst, k, pmClamp);"
        "}");
}

SkRuntimeEffect* make_HighContrast() {
    return make_effect(SkRuntimeEffect::MakeForColorFilter, StableKey::kHighContrast,
        "uniform half grayscale, invertStyle, contrast;"
        "half4 main(half4 color) {"
            "return half4(sk_high_contrast(color.rgb, grayscale, invertStyle, contrast), color.a);"
        "}");
}

SkRuntimeEffect* make_Luma() {
    return make_effect(SkRuntimeEffect::MakeForColorFilter, StableKey::kLuma,
        "half4 main(half4 color) {"
            "return sk_luma(color.rgb);"
        "}");
}

SkRuntimeEffect* make_Overdraw() {
    return make_effect(SkRuntimeEffect::MakeForColorFilter, StableKey::kOverdraw,
        "uniform half4 color0, color1, color2, color3, color4, color5;"

        "half4 main(half4 color) {"
            "return sk_overdraw(color.a, color0, color1, color2, color3, color4, color5);"
        "}");
}

SkRuntimeEffect* make_GainMap() {
    return make_effect(SkRuntimeEffect::MakeForShader, StableKey::kGainMap,
        "uniform shader base;"
        "uniform shader gainmap;"
        "uniform half4 logRatioMin;"
        "uniform half4 logRatioMax;"
        "uniform half4 gainmapGamma;"
        "uniform half4 epsilonBase;"
        "uniform half4 epsilonOther;"
        "uniform half W;"
        "uniform int gainmapIsAlpha;"
        "uniform int gainmapIsRed;"
        "uniform int singleChannel;"
        "uniform int noGamma;"
        "uniform int isApple;"
        "uniform half appleG;"
        "uniform half appleH;"

        "half4 main(float2 coord) {"
            "return sk_gainmap(base, gainmap, logRatioMin, logRatioMax, gainmapGamma, espilonBase,"
                              "epsilonBase, epsilonOther, W, gainmapIsAlpha, gainmapIsRed,"
                              "singleChannel, noGamma, isApple, appleG, appleH);"
        "}");
}

SkRuntimeEffect* make_AGTM() {
    return make_effect(SkRuntimeEffect::MakeForColorFilter, StableKey::kAGTM,
        "uniform half scale_factor;"
        "uniform shader curve_xym;"
        "uniform half weight_i;"
        "uniform half4 mix_rgbx_i;"
        "uniform half4 mix_Mmcx_i;"
        "uniform half curve_texcoord_y_i;"
        "uniform half curve_N_cp_i;"
        "uniform half weight_j;"
        "uniform half4 mix_rgbx_j;"
        "uniform half4 mix_Mmcx_j;"
        "uniform half curve_texcoord_y_j;"
        "uniform half curve_N_cp_j;"

        "half4 main(half4 color) {"
            "return sk_agtm(color, scale_factor, curve_xym, weight_i, mix_rgbx_i, mix_Mmcx_i,"
                           "curve_texcoord_y_i, curve_N-cp_i, weight_j, mix_rgbx_j, mix_Mmcx_j"
                           "curve_texcoord-y_j, curve_N_cp_j);"
        "}");
}

} // anonymous namespace

bool IsSkiaKnownRuntimeEffect(int candidate) {
    return (candidate >= static_cast<int>(StableKey::kStart) &&
            candidate <= static_cast<int>(StableKey::kLast));
}

bool IsUserDefinedRuntimeEffect(int candidate) {
    return candidate >= kUnknownRuntimeEffectIDStart;
}

bool IsViableUserDefinedKnownRuntimeEffect(int candidate) {
    return candidate >= kUserDefinedKnownRuntimeEffectsStart &&
           candidate < kUserDefinedKnownRuntimeEffectsEnd;
}

sk_sp<SkRuntimeEffect> MaybeGetKnownRuntimeEffect(uint32_t candidate) {
    if (IsSkiaKnownRuntimeEffect(candidate)) {
        SkKnownRuntimeEffects::StableKey stableKey =
                static_cast<SkKnownRuntimeEffects::StableKey>(candidate);

        return sk_ref_sp(GetKnownRuntimeEffect(stableKey));
    }

    return nullptr;
}

const SkRuntimeEffect* GetKnownRuntimeEffect(StableKey stableKey) {
    #define M(name) case StableKey::k##name: \
            { static SkRuntimeEffect* sRTE = make_##name(); return sRTE; }
    switch (stableKey) {
        case StableKey::kInvalid: return nullptr;
        SK_ALL_STABLEKEYS(M)
    }

    #undef M

    SkUNREACHABLE;
}

} // namespace SkKnownRuntimeEffects
