/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if 1 //defined(SK_GRAPHITE) && defined(SK_VULKAN) && defined(SK_BUILD_FOR_ANDROID)

#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "include/gpu/graphite/vk/precompile/VulkanPrecompileShader.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "tests/graphite/precompile/AndroidRuntimeEffectManager.h"
#include "tests/graphite/precompile/PrecompileTestUtils.h"
using namespace skgpu::graphite;
using namespace PrecompileTestUtils;

namespace {

// Used in lieu of SkEnumBitMask to avoid adding casts when copying in precompile cases.
static constexpr DrawTypeFlags operator|(DrawTypeFlags a, DrawTypeFlags b) {
    return static_cast<DrawTypeFlags>(static_cast<std::underlying_type<DrawTypeFlags>::type>(a) |
                                      static_cast<std::underlying_type<DrawTypeFlags>::type>(b));
}

constexpr DrawTypeFlags kRRectAndNonAARect =
        static_cast<DrawTypeFlags>(DrawTypeFlags::kAnalyticRRect |
                                   DrawTypeFlags::kNonAAFillRect);

static const SkColorInfo kRGBA8Premul(kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);

static const skgpu::graphite::RenderPassProperties kRGBA_1_D_SRGB {
    skgpu::graphite::DepthStencilFlags::kDepth,
    kRGBA_8888_SkColorType,
    SkColorSpace::MakeSRGB(),
    /* fRequiresMSAA= */ false
};
static const skgpu::graphite::RenderPassProperties kRGBA_4_DS_SRGB {
    skgpu::graphite::DepthStencilFlags::kDepthStencil,
    kRGBA_8888_SkColorType,
    SkColorSpace::MakeSRGB(),
    /* fRequiresMSAA= */ true
};
static const skgpu::graphite::RenderPassProperties kRGBA16F_1_D_SRGB {
    skgpu::graphite::DepthStencilFlags::kDepth,
    kRGBA_F16_SkColorType,
    SkColorSpace::MakeSRGB(),
    /* fRequiresMSAA= */ false
};

static const SkColorInfo kRGBA8PremulHLG(kRGBA_8888_SkColorType,
                                         kPremul_SkAlphaType,
                                         SkColorSpace::MakeRGB(SkNamedTransferFn::kHLG,
                                                               SkNamedGamut::kRec2020));

static const SkColorInfo kRGBA8PremulDisplayP3(kRGBA_8888_SkColorType,
                                               kPremul_SkAlphaType,
                                               SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                                     SkNamedGamut::kDisplayP3));

static const SkColorInfo kRGBA8PremulPQ(kRGBA_8888_SkColorType,
                                        kPremul_SkAlphaType,
                                        SkColorSpace::MakeRGB(SkNamedTransferFn::kPQ,
                                                              SkNamedGamut::kRec2020));

static const SkColorInfo kRGBA8Premul2020(kRGBA_8888_SkColorType,
                                          kPremul_SkAlphaType,
                                          SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020_12bit,
                                                                SkNamedGamut::kRec2020));

static PipelineLabel kPixel8Cases[] = {
/*0*/{ -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid linear F rgba cf1lf1) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*1*/{ -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid linear F rgba cf1lf1) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*2*/{ -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 601+narrow mid mid linear F rgba cf1lf1) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*3*/{ -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 601+narrow mid mid linear F rgba cf1lf1) ] ColorSpaceTransformPremul ] ] SrcOver" },

/*4*/{ -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 601+narrow mid mid linear F rgba cf1lf1) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*5*/{ -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid linear F rgba cf1lf1) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*6*/{ -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid linear F rgba cf1lf1) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*7*/{ -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos linear F rgba cf1lf1) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver" },
/*8*/{ -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose [ Compose [ RE_LinearEffect_BT2020_HLG__UNKNOWN__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos linear F rgba cf1lf1) ] ColorSpaceTransform ] ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver" },
/*9*/{ -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ RE_LinearEffect_BT2020_HLG__UNKNOWN__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos linear F rgba cf1lf1) ] ColorSpaceTransform ] ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver" },
/*10*/{ -1, "RP((RGBA16F+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_MouriMap_CrossTalkAndChunk16x16Effect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos linear F rgba cf1lf1) ] ColorSpaceTransform ] ] ] Src" },
/*11*/{ -1, "RP((RGBA16F+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos linear F rgba cf1lf1) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] SrcOver" },

/*12*/{ -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+full mid mid linear F rgba cf1lf1) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*13*/{ -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+full mid mid linear F rgba cf1lf1) ] Passthrough ] ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*14*/{ -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+full mid mid linear F rgba cf1lf1) ] Passthrough ] ] ] SrcOver" },
/*15*/{ -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ RE_LinearEffect_BT2020_HLG__UNKNOWN__false__UNKNOWN__Shader [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos linear F rgba cf1lf1) ] ColorSpaceTransform ] ] ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver" },

/*16*/{ -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ BlendCompose [ Compose [ RE_LinearEffect_BT2020_HLG__UNKNOWN__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos linear F rgba cf1lf1) ] ColorSpaceTransform ] ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
};

static PipelineLabel kPixel10Cases[] = {
/*0*/{ -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x769 709+narrow mid mid nearest F rgba cf0lf1) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*1*/{ -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x769 709+narrow mid mid nearest F rgba cf0lf1) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*2*/  // haven't seen yet
/*3*/  // haven't seen yet
/*4*/  // haven't seen yet
/*5*/  // haven't seen yet
/*6*/  // haven't seen yet
/*7*/{ -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ Compose [ RE_LutEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x54 2020+narrow mid mid nearest F rgba cf0lf1) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver" },
/*8*/{ -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ Compose [ RE_LutEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x54 2020+narrow mid mid nearest F rgba cf0lf1) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver" },
/*9*/{ -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ Compose [ RE_LutEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x54 2020+narrow mid mid nearest F rgba cf0lf1) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver" },
/*10*/  // haven't seen yet
/*11*/{ -1, "RP((RGBA16F+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ Compose [ RE_LutEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x54 2020+narrow mid mid nearest F rgba cf0lf1) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] SrcOver" },
/*12*/  // haven't seen yet
//??
/*14*/{ -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x769 709+narrow mid mid nearest F rgba cf0lf1) ] Passthrough ] ] ] SrcOver" },
/*15*/  // haven't seen yet
/*16*/  // haven't seen yet

/*7'*/ // haven't seen yet
/*8'*/{ -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ Compose [ RE_LutEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x54 2020+full mid mid nearest F rgba cf0lf1) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver" },
/*9'*/  // haven't seen yet
/*10'*/ // haven't seen yet
/*11'*/ // haven't seen yet
/*15'*/ // haven't seen yet
/*16'*/ // haven't seen yet

/*17*/ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x769 709+narrow mid mid nearest F rgba cf0lf1) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },

/*18*/ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x769 709+narrow mid mid nearest F rgba cf0lf1) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*19*/ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x769 709+narrow mid mid nearest F rgba cf0lf1) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver" },

/*20*/ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ Compose [ RE_LutEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x54 2020+narrow mid mid nearest F rgba cf0lf1) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] MatrixColorFilter ] Dither ] SrcOver" },

};

inline HdrRenderType getHdrRenderType(ui::Dataspace dataspace,
                                      float hdrSdrRatio = 1.f) {
#define HAL_DATASPACE_TRANSFER_MASK 130023424   // (31 << TRANSFER_SHIFT)
#define HAL_DATASPACE_RANGE_MASK 939524096      // (7 << RANGE_SHIFT)
#define HAL_DATASPACE_RANGE_EXTENDED 402653184  // (3 << RANGE_SHIFT)
#define HAL_DATASPACE_TRANSFER_HLG 33554432     // (8 << TRANSFER_SHIFT)

    const auto transfer = static_cast<uint32_t>(dataspace) & HAL_DATASPACE_TRANSFER_MASK;
    const auto range = static_cast<uint32_t>(dataspace) & HAL_DATASPACE_RANGE_MASK;

    if (transfer == HAL_DATASPACE_TRANSFER_HLG) {
        return HdrRenderType::GENERIC_HDR;
    }

    // Extended range layer with an hdr/sdr ratio of > 1.01f can "self-promote" to HDR.
    if (range == HAL_DATASPACE_RANGE_EXTENDED && hdrSdrRatio > 1.01f) {
        return HdrRenderType::DISPLAY_HDR;
    }

    return HdrRenderType::SDR;
}

sk_sp<PrecompileShader> vulkan_ycbcr_image_shader(const skgpu::VulkanYcbcrConversionInfo& ycbcrInfo,
                                                  const SkColorInfo& ci) {
    return PrecompileShaders::VulkanYCbCrImage(ycbcrInfo,
                                               PrecompileShaders::ImageShaderFlags::kExcludeCubic,
                                               { &ci, 1 },
                                               {});
}

sk_sp<PrecompileShader> wrap_in_linear_effect(RuntimeEffectManager& effectManager,
                                              const shaders::LinearEffect& linearEffect,
                                              sk_sp<PrecompileShader> child,
                                              sk_sp<SkColorSpace> outCS) {
    sk_sp<SkRuntimeEffect> runtimeEffect =
            effectManager.getOrCreateLinearRuntimeEffect(linearEffect);
    sk_sp<PrecompileShader> linearEffectShader = PrecompileRuntimeEffects::MakePrecompileShader(
        std::move(runtimeEffect),
        {{ {{ std::move(child) }} }});
    if (outCS) {
        linearEffectShader = linearEffectShader->makeWithWorkingColorSpace(nullptr, outCS);
    }

    return linearEffectShader;
}

sk_sp<PrecompileShader> wrap_in_MouriMapToneMap(RuntimeEffectManager& effectManager,
                                                sk_sp<PrecompileShader> img) {

    SkColorInfo luxCI { kRGBA_F16_SkColorType,
                        kPremul_SkAlphaType,
                        SkColorSpace::MakeSRGBLinear() };
    sk_sp<PrecompileShader> lux = PrecompileShaders::Image(
        PrecompileShaders::ImageShaderFlags::kExcludeCubic,
        { &luxCI, 1 },
        {});

    sk_sp<PrecompileShader> toneMap = PrecompileRuntimeEffects::MakePrecompileShader(
            effectManager.getKnownRuntimeEffect(
                    RuntimeEffectManager::KnownId::kMouriMap_TonemapEffect),
            {{ {{ std::move(img) }}, {{ std::move(lux) }} }});
    sk_sp<PrecompileShader> inLinear =
            toneMap->makeWithWorkingColorSpace(luxCI.refColorSpace(), nullptr);

    return inLinear;
}

sk_sp<PrecompileShader> wrap_in_MouriMapCrossTalk(RuntimeEffectManager& effectManager,
                                                  sk_sp<PrecompileShader> child) {
    return PrecompileRuntimeEffects::MakePrecompileShader(
            effectManager.getKnownRuntimeEffect(
                    RuntimeEffectManager::KnownId::kMouriMap_CrossTalkAndChunk16x16Effect),
            {{ {{ std::move(child) }} }});
}

sk_sp<PrecompileShader> wrap_in_edge_extension(RuntimeEffectManager& effectManager,
                                               sk_sp<PrecompileShader> child) {
    return PrecompileRuntimeEffects::MakePrecompileShader(
        effectManager.getKnownRuntimeEffect(RuntimeEffectManager::KnownId::kEdgeExtensionEffect),
        {{ {{ std::move(child) }} }});
}


sk_sp<PrecompileShader>wrap_in_lut(RuntimeEffectManager& effectManager,
                                   sk_sp<PrecompileShader> child) {
    sk_sp<SkRuntimeEffect> lutEffect = effectManager.getKnownRuntimeEffect(
            RuntimeEffectManager::KnownId::kLutEffect);

    const SkColorInfo kRGBA8PremulColorSpin(kRGBA_8888_SkColorType,
                                            kPremul_SkAlphaType,
                                            SkColorSpace::MakeSRGB()->makeColorSpin());

    sk_sp<PrecompileShader> lutImg = PrecompileShaders::Image(
        PrecompileShaders::ImageShaderFlags::kExcludeCubic,
        { kRGBA8PremulColorSpin },
        {});

    sk_sp<PrecompileShader> lutShader = PrecompileRuntimeEffects::MakePrecompileShader(
            std::move(lutEffect),
            {{ {{ std::move(child) }}, {{ std::move(lutImg) }} }});

    sk_sp<SkColorSpace> cs = SkColorSpace::MakeSRGB()->makeColorSpin();
    return lutShader->makeWithWorkingColorSpace(std::move(cs));
}

struct LayerSettings {
    ui::Dataspace sourceDataspace = ui::Dataspace::UNKNOWN;
    bool edgeExtensionEffect = false;
    bool luts = false;
};

struct DisplaySettings {
    enum TonemapStrategy {
        Other,
        Local
    };
    TonemapStrategy tonemapStrategy = Other;
};

struct RuntimeEffectShaderParameters {
    sk_sp<PrecompileShader> shader;
    LayerSettings layer;
    DisplaySettings display;
    bool requiresLinearEffect;
    const ui::Dataspace outputDataSpace;
    const ui::Dataspace fakeOutputDataspace;
};

// Precompile correlate to SkiaRenderEngine::createRuntimeEffectShader. It has
// been simplified for just YCbCr draws.
sk_sp<PrecompileShader> create_runtime_effect_shader(
        RuntimeEffectManager& effectManager,
        const RuntimeEffectShaderParameters& parameters,
        bool squashCS) {

    bool wrapped = false;
    sk_sp<PrecompileShader> shader = parameters.shader;
    if (parameters.layer.edgeExtensionEffect) {
        shader = wrap_in_edge_extension(effectManager, std::move(shader));
        wrapped = true;
    }

    if (parameters.layer.luts) {
        shader = wrap_in_lut(effectManager, std::move(shader));
        wrapped = true;
    }

    if (parameters.requiresLinearEffect) {
        const HdrRenderType hdrType = getHdrRenderType(parameters.layer.sourceDataspace);
//                                                       parameters.layerDimmingRatio);

        const auto usingLocalTonemap =
            parameters.display.tonemapStrategy == DisplaySettings::TonemapStrategy::Local &&
            hdrType != HdrRenderType::SDR &&
            !wrapped &&
            (hdrType != HdrRenderType::DISPLAY_HDR);
//                 || parameters.display.targetHdrSdrRatio < parameters.layerDimmingRatio);

        SkDebugf("hdrType %d %d %d == %d\n", (int) parameters.display.tonemapStrategy, (int) hdrType, wrapped, usingLocalTonemap);

        if (usingLocalTonemap) {
            shader = wrap_in_MouriMapToneMap(effectManager, std::move(shader));
        }

        shaders::LinearEffect linearEffect {
            .inputDataspace = parameters.layer.sourceDataspace,
            .outputDataspace = parameters.outputDataSpace,
            .undoPremultipliedAlpha = false,
            .fakeOutputDataspace = parameters.fakeOutputDataspace};

        if (squashCS) { //(usingLocalTonemap) {// || parameters.layer.luts) {
            linearEffect.inputDataspace = ui::Dataspace::UNKNOWN;
            linearEffect.outputDataspace = ui::Dataspace::SRGB;
            linearEffect.fakeOutputDataspace = ui::Dataspace::UNKNOWN;
        }

        shader = wrap_in_linear_effect(effectManager,
                                       linearEffect,
                                       std::move(shader),
                                       SkColorSpace::MakeSRGBLinear());
    }

    return shader;
}

// Basic external format YCbCr wrapped in a RE_LinearEffect
void linear_effect_hdr(RuntimeEffectManager& effectManager,
                       sk_sp<PrecompileShader> hdrImg,
                       const shaders::LinearEffect& linearEffect,
                       bool luts,
                       bool withMatrixColorFilter,
                       std::vector<PrecompileSettings>& result) {

    RuntimeEffectShaderParameters parameters{
        hdrImg,
        {.sourceDataspace = linearEffect.inputDataspace,
         .edgeExtensionEffect = false,
         .luts = luts},
        {.tonemapStrategy = DisplaySettings::TonemapStrategy::Other},
        /* requiresLinearEffect= */ true,
        linearEffect.outputDataspace,
        linearEffect.fakeOutputDataspace
    };
#if 1
    sk_sp<PrecompileShader> shader = create_runtime_effect_shader(effectManager, parameters, false);
#else
    sk_sp<PrecompileShader> shader = wrap_in_linear_effect(
        effectManager,
        linearEffect,
        hdrImg,
        SkColorSpace::MakeSRGBLinear());
#endif

    PaintOptions paintOptions;
    paintOptions.setShaders({{ std::move(shader) }});
    paintOptions.setDither(true);

    result.push_back({ paintOptions,
                       kRRectAndNonAARect,
                       kRGBA_1_D_SRGB });

    // This is the only MSAA Pipeline
    paintOptions.setPaintColorIsOpaque(false);
    result.push_back({ paintOptions,
                       DrawTypeFlags::kNonAAFillRect,
                       kRGBA_4_DS_SRGB });

    if (withMatrixColorFilter) {
        paintOptions.setPaintColorIsOpaque(true);
        paintOptions.setColorFilters({ PrecompileColorFilters::Matrix() });
        result.push_back({ paintOptions,
                           kRRectAndNonAARect,
                           kRGBA_1_D_SRGB });
    }
}

// RE_EdgeExtensionEffect wrapped in a RE_LinearEffect
void linear_effect_edge_extension_hdr(RuntimeEffectManager& effectManager,
                                      sk_sp<PrecompileShader> hdrImg,
                                      const shaders::LinearEffect& linearEffect,
                                      std::vector<PrecompileSettings>& result) {
    RuntimeEffectShaderParameters parameters{
        hdrImg,
        {.sourceDataspace = linearEffect.inputDataspace,
         .edgeExtensionEffect = true,
         .luts = false},
        {.tonemapStrategy = DisplaySettings::TonemapStrategy::Other},
        /* requiresLinearEffect= */ true,
        linearEffect.outputDataspace,
        linearEffect.fakeOutputDataspace
    };
#if 1
    sk_sp<PrecompileShader> shader = create_runtime_effect_shader(effectManager, parameters, false);
#else
    sk_sp<PrecompileShader> edgeEffect = wrap_in_edge_extension(effectManager, hdrImg);
    sk_sp<PrecompileShader> shader = wrap_in_linear_effect(
        effectManager,
        linearEffect,
        std::move(edgeEffect),
        SkColorSpace::MakeSRGBLinear());
#endif

    PaintOptions paintOptions;
    paintOptions.setShaders( {{ std::move(shader) }});
    paintOptions.setDither(true);

    result.push_back({ paintOptions,
                       DrawTypeFlags::kNonAAFillRect,
                       kRGBA_1_D_SRGB });
}

// RE_MouriMap_TonemapEffect wrapped in a RE_LinearEffect
void linear_effect_mourimap_tonemap_hdr(RuntimeEffectManager& effectManager,
                                        sk_sp<PrecompileShader> hdrImg,
                                        const shaders::LinearEffect& linearEffect,
                                        bool luts,
                                        std::vector<PrecompileSettings>& result) {

    RuntimeEffectShaderParameters parameters{
        hdrImg,
        {.sourceDataspace = linearEffect.inputDataspace,
         .edgeExtensionEffect = false,
         .luts = luts},
        {.tonemapStrategy = DisplaySettings::TonemapStrategy::Local},
        /* requiresLinearEffect= */ true,
        linearEffect.outputDataspace,
        linearEffect.fakeOutputDataspace
    };

#if 1
    sk_sp<PrecompileShader> shader = create_runtime_effect_shader(effectManager, parameters, true);
#else
    static const shaders::LinearEffect kUnkSRGBfalseUnk {
        ui::Dataspace::UNKNOWN,
        ui::Dataspace::SRGB,
        false,
        ui::Dataspace::UNKNOWN,
        shaders::LinearEffect::Shader };

    sk_sp<PrecompileShader> wrapped = wrap_in_MouriMapToneMap(effectManager, hdrImg);
    sk_sp<PrecompileShader> shader = wrap_in_linear_effect(
        effectManager,
        kUnkSRGBfalseUnk,
        std::move(wrapped),
        SkColorSpace::MakeSRGBLinear());
#endif

    PaintOptions paintOptions;
    paintOptions.setShaders( {{ std::move(shader) }});
    result.push_back({ paintOptions,
                       DrawTypeFlags::kNonAAFillRect,
                       kRGBA16F_1_D_SRGB });

    paintOptions.setDither(true);
    result.push_back({ paintOptions,
                       DrawTypeFlags::kNonAAFillRect,
                       kRGBA_1_D_SRGB });
}

// RE_MouriMap_CrossTalkAndChunk16x16Effect [ ... ]
// This is a utility option created for the RE_MouriMap_TonemapEffect. Technically,
// we should also create an 8x8 effect.
void mourimap_crosstalk_and_chunk_hdr(RuntimeEffectManager& effectManager,
                                      sk_sp<PrecompileShader> hdrImg,
                                      std::vector<PrecompileSettings>& result) {

    sk_sp<PrecompileShader> wrapped = wrap_in_MouriMapCrossTalk(effectManager, hdrImg);

    PaintOptions paintOptions;
    paintOptions.setShaders( {{ std::move(wrapped) }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrc ));
    result.push_back({ paintOptions,
                       DrawTypeFlags::kNonAAFillRect,
                       kRGBA16F_1_D_SRGB });
}

// Basic SDR YCbCr draws
void basic_sdr(sk_sp<PrecompileShader> sdrImg,
               std::vector<PrecompileSettings>& result,
               bool withMatrixColorFilter) {
    PaintOptions paintOptions;

    paintOptions.setShaders({{ sdrImg }});
    result.push_back({ paintOptions,
                       kRRectAndNonAARect,
                       kRGBA_1_D_SRGB });

    // TODO: see if this clipped rect draw can be replaced with an rrect draw
    result.push_back({ paintOptions,
                       DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
                       kRGBA_1_D_SRGB });

    paintOptions.setPaintColorIsOpaque(false);
    result.push_back({ paintOptions,
                       kRRectAndNonAARect,
                       kRGBA_1_D_SRGB });

    if (withMatrixColorFilter) {
        paintOptions.setPaintColorIsOpaque(true);
        paintOptions.setColorFilters({ PrecompileColorFilters::Matrix() });
        result.push_back({ paintOptions,
                           DrawTypeFlags::kNonAAFillRect,
                           kRGBA_1_D_SRGB });

        paintOptions.setDither(true);
        result.push_back({ paintOptions,
                           DrawTypeFlags::kNonAAFillRect,
                           kRGBA_1_D_SRGB });
    }
}

// RE_EdgeExtensionEffect wrapping and SDR YCbCr image
void edge_extension_sdr(RuntimeEffectManager& effectManager,
                        sk_sp<PrecompileShader> sdrImg,
                        std::vector<PrecompileSettings>& result) {
    RuntimeEffectShaderParameters parameters{
        sdrImg,
        {.sourceDataspace = ui::Dataspace::SRGB,
         .edgeExtensionEffect = true,
         .luts = false},
        {.tonemapStrategy = DisplaySettings::TonemapStrategy::Other},
        /* requiresLinearEffect= */ false,
        ui::Dataspace::UNKNOWN,
        ui::Dataspace::UNKNOWN
    };

#if 1
    sk_sp<PrecompileShader> shader = create_runtime_effect_shader(effectManager, parameters, true);
#else
    sk_sp<PrecompileShader> shader = wrap_in_edge_extension(effectManager, sdrImg);
#endif

    PaintOptions paintOptions;
    paintOptions.setShaders({{ std::move(shader) }});
    result.push_back({ paintOptions,
                       DrawTypeFlags::kNonAAFillRect,
                       kRGBA_1_D_SRGB });

    paintOptions.setPaintColorIsOpaque(false);
    result.push_back({ paintOptions,
                       DrawTypeFlags::kNonAAFillRect,
                       kRGBA_1_D_SRGB });
}

// This method takes a set of YCbCr infos and merges them a set of PaintOptions that Android
// would use for them. The main split is between HDR/SDR handling. HDR handling is much more
// complicated with tone mapping, etc.
void ycbcr_merge(RuntimeEffectManager& effectManager,
                 SkSpan<const skgpu::VulkanYcbcrConversionInfo> ycbcrs,
                 SkSpan<const shaders::LinearEffect> linearEffects,
                 bool luts,
                 bool withMatrixColorFilter,
                 std::vector<PrecompileSettings>& result) {

    for (const auto& ycbcr : ycbcrs) {
        if (ycbcr.model() == VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020) {
            for (const auto& linearEffect : linearEffects) {
                // Use toSkColorSpace(ui::Dataspace dataspace) for this
                const SkColorInfo* ci;
                if (linearEffect.inputDataspace == ui::Dataspace::BT2020_ITU_PQ) {
                    ci = &kRGBA8PremulPQ;
                } else if (linearEffect.inputDataspace == ui::Dataspace::BT2020_HLG) {
                    ci = &kRGBA8PremulHLG;
                } else if (linearEffect.inputDataspace == ui::Dataspace::DISPLAY_P3) {
                    ci = &kRGBA8PremulDisplayP3;
                } else if (linearEffect.inputDataspace == static_cast<ui::Dataspace>(0x188a0000)) {
                    // TODO: This is incorrect and should be kRGBA8PremulDisplayP3. Fix how
                    // colorspaces are handled so this can be fixed. The issue is that Android
                    // doesn't associate the colorSpace w/ the image but keeps it separate to
                    // externally manipulate it.
                    ci = &kRGBA8PremulHLG;    // kRGBA8PremulDisplayP3
                } else {
                    ci = &kRGBA8Premul2020;
                }

                sk_sp<PrecompileShader> hdrImg = vulkan_ycbcr_image_shader(ycbcr, *ci);

                linear_effect_hdr(effectManager, hdrImg, linearEffect, luts,
                                  withMatrixColorFilter, result);
                linear_effect_edge_extension_hdr(effectManager, hdrImg, linearEffect, result);
                linear_effect_mourimap_tonemap_hdr(effectManager, hdrImg, linearEffect, luts, result);
                mourimap_crosstalk_and_chunk_hdr(effectManager, hdrImg, result);
            }
        } else {
            sk_sp<PrecompileShader> sdrImg = vulkan_ycbcr_image_shader(ycbcr, kRGBA8Premul);

            basic_sdr(sdrImg, result, withMatrixColorFilter);
            edge_extension_sdr(effectManager, sdrImg, result);
        }
    }
}


#if 0
void flibble(const char* label, ui::Dataspace dataspace) {
#define HAL_DATASPACE_STANDARD_MASK 4128768                   // (63 << STANDARD_SHIFT)
#define HAL_DATASPACE_TRANSFER_MASK 130023424       // (31 << TRANSFER_SHIFT)
#define HAL_DATASPACE_RANGE_MASK 939524096      // (7 << RANGE_SHIFT)

    const uint32_t standard = ((uint32_t)dataspace) & HAL_DATASPACE_STANDARD_MASK;
    const uint32_t transfer = ((uint32_t)dataspace) & HAL_DATASPACE_TRANSFER_MASK;
    const uint32_t range = ((uint32_t)dataspace) & HAL_DATASPACE_RANGE_MASK;

    SkDebugf("%s: standard %d xfer %d range %d\n", label, standard, transfer, range);
}
#endif

void VisitAndroidPrecompileSettings_Pixel8(
         skgpu::graphite::PrecompileContext* precompileContext,
         RuntimeEffectManager& effectManager,
         const std::function<void(skgpu::graphite::PrecompileContext*,
                                  const PrecompileSettings&,
                                  int index)>& func) {
    static const VkComponentMapping kDefaultComponents = { VK_COMPONENT_SWIZZLE_IDENTITY,
                                                           VK_COMPONENT_SWIZZLE_IDENTITY,
                                                           VK_COMPONENT_SWIZZLE_IDENTITY,
                                                           VK_COMPONENT_SWIZZLE_IDENTITY };

    const skgpu::VulkanYcbcrConversionInfo kPixel8YCbCrs[] = {
        { 238,
          VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
          VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
          VK_CHROMA_LOCATION_MIDPOINT,
          VK_CHROMA_LOCATION_MIDPOINT,
          VK_FILTER_LINEAR,
          /*forceExplicitReconstruction=*/false,
          kDefaultComponents,
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT
        },
        { 238,
          VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
          VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
          VK_CHROMA_LOCATION_MIDPOINT,
          VK_CHROMA_LOCATION_MIDPOINT,
          VK_FILTER_LINEAR,
          /*forceExplicitReconstruction=*/false,
          kDefaultComponents,
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT
        },
        { 238,
          VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601,
          VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
          VK_CHROMA_LOCATION_MIDPOINT,
          VK_CHROMA_LOCATION_MIDPOINT,
          VK_FILTER_LINEAR,
          /*forceExplicitReconstruction=*/false,
          kDefaultComponents,
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT
        },
        { 247,
          VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020,
          VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
          VK_CHROMA_LOCATION_COSITED_EVEN,
          VK_CHROMA_LOCATION_COSITED_EVEN,
          VK_FILTER_LINEAR,
          /*forceExplicitReconstruction=*/false,
          kDefaultComponents,
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT
        },
    };
    // This specifies the color space information to be used with Rec.2020 YCbCr images.
    // TODO: we will need to be able to get this from Android
    const shaders::LinearEffect kPixel8LinearEffects[] {
        { ui::Dataspace::BT2020_HLG,
          ui::Dataspace::UNKNOWN,
          false,
          ui::Dataspace::UNKNOWN },
    };

//    in: xfer 33554432 range 134217728  HAL_DATASPACE_TRANSFER_HLG HAL_DATASPACE_RANGE_FULL
//    out: xfer 0 range 0
//    fake: xfer 0 range 0

//    flibble("in", kPixel8LinearEffects[0].inputDataspace);
//    flibble("out", kPixel8LinearEffects[0].outputDataspace);
//    flibble("fake", kPixel8LinearEffects[0].fakeOutputDataspace);

    std::vector<PrecompileSettings> precompileCases;
    ycbcr_merge(effectManager, {kPixel8YCbCrs}, {kPixel8LinearEffects},
                /* luts= */ false, /* withMatrixColorFilter= */ false, precompileCases);

    for (size_t i = 0; i < std::size(precompileCases); ++i) {
        func(precompileContext, precompileCases[i], i);
    }
}

void VisitAndroidPrecompileSettings_Pixel10(
         skgpu::graphite::PrecompileContext* precompileContext,
         RuntimeEffectManager& effectManager,
         const std::function<void(skgpu::graphite::PrecompileContext*,
                                  const PrecompileSettings&,
                                  int index)>& func) {
    static const VkComponentMapping kDefaultComponents = { VK_COMPONENT_SWIZZLE_IDENTITY,
                                                           VK_COMPONENT_SWIZZLE_IDENTITY,
                                                           VK_COMPONENT_SWIZZLE_IDENTITY,
                                                           VK_COMPONENT_SWIZZLE_IDENTITY };

    const skgpu::VulkanYcbcrConversionInfo kPixel8YCbCrs[] = {
        { 769,
          VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
          VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
          VK_CHROMA_LOCATION_MIDPOINT,
          VK_CHROMA_LOCATION_MIDPOINT,
          VK_FILTER_NEAREST,
          /*forceExplicitReconstruction=*/false,
          kDefaultComponents,
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT
        },
        { 54,
          VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020,
          VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
          VK_CHROMA_LOCATION_MIDPOINT,
          VK_CHROMA_LOCATION_MIDPOINT,
          VK_FILTER_NEAREST,
          /*forceExplicitReconstruction=*/false,
          kDefaultComponents,
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT
        },
        { 54,
          VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020,
          VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
          VK_CHROMA_LOCATION_MIDPOINT,
          VK_CHROMA_LOCATION_MIDPOINT,
          VK_FILTER_NEAREST,
          /*forceExplicitReconstruction=*/false,
          kDefaultComponents,
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT
        },
    };
    // This specifies the color space information to be used with Rec.2020 YCbCr images.
    // TODO: we will need to be able to get this from Android
    const shaders::LinearEffect kPixel10LinearEffects[] {
        { static_cast<ui::Dataspace>(0x188a0000),
          ui::Dataspace::DISPLAY_P3,
          false,
          static_cast<ui::Dataspace>(0x90a0000) },
    };


//    in: standard 655360 xfer 8388608 range 402653184    HAL_DATASPACE_STANDARD_DCI_P3 HAL_DATASPACE_TRANSFER_SRGB     HAL_DATASPACE_RANGE_EXTENDED
//    out: standard 655360 xfer 8388608 range 134217728   HAL_DATASPACE_STANDARD_DCI_P3 HAL_DATASPACE_TRANSFER_SRGB     HAL_DATASPACE_RANGE_FULL
//    fake: standard 655360 xfer 16777216 range 134217728 HAL_DATASPACE_STANDARD_DCI_P3 HAL_DATASPACE_TRANSFER_GAMMA2_2 HAL_DATASPACE_RANGE_FULL

//    flibble("in", kPixel10LinearEffects[0].inputDataspace);
//    flibble("out", kPixel10LinearEffects[0].outputDataspace);
//    flibble("fake", kPixel10LinearEffects[0].fakeOutputDataspace);

    std::vector<PrecompileSettings> precompileCases;
    ycbcr_merge(effectManager, {kPixel8YCbCrs}, {kPixel10LinearEffects},
                /* luts= */ true, /* withMatrixColorFilter= */ true, precompileCases);

    for (size_t i = 0; i < std::size(precompileCases); ++i) {
        func(precompileContext, precompileCases[i], i);
    }
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_VULKAN_CONTEXT(AndroidYCbCrPrecompileTest_P8, reporter, context,
                                     CtsEnforcement::kNever) {
    PrecompileTest(reporter, context, kPixel8Cases, VisitAndroidPrecompileSettings_Pixel8,
                   /* checkPaintOptionCoverage= */ false,
                   /* checkPipelineLabelCoverage= */ true);
}

DEF_GRAPHITE_TEST_FOR_VULKAN_CONTEXT(AndroidYCbCrPrecompileTest_P10, reporter, context,
                                     CtsEnforcement::kNever) {
    PrecompileTest(reporter, context, kPixel10Cases, VisitAndroidPrecompileSettings_Pixel10,
                   /* checkPaintOptionCoverage= */ false,
                   /* checkPipelineLabelCoverage= */ true);
}

#endif
