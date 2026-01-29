/*
* Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE) && defined(SK_VULKAN)

namespace {

static PipelineLabel kPixel8Cases[] = {
    { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid linear F rgba cf1lf1) ] ColorSpaceTransformPremul ] ] SrcOver" },
    { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid linear F rgba cf1lf1) ] ColorSpaceTransformPremul ] ] SrcOver" },
    { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 601+narrow mid mid linear F rgba cf1lf1) ] ColorSpaceTransformPremul ] ] SrcOver" },
    { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 601+narrow mid mid linear F rgba cf1lf1) ] ColorSpaceTransformPremul ] ] SrcOver" },

    { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 601+narrow mid mid linear F rgba cf1lf1) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
    { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid linear F rgba cf1lf1) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
    { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid linear F rgba cf1lf1) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
    { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos linear F rgba cf1lf1) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver" },
    { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose [ Compose [ RE_LinearEffect_BT2020_HLG__UNKNOWN__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos linear F rgba cf1lf1) ] ColorSpaceTransform ] ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver" },
    { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ RE_LinearEffect_BT2020_HLG__UNKNOWN__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos linear F rgba cf1lf1) ] ColorSpaceTransform ] ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver" },
    { -1, "RP((RGBA16F+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_MouriMap_CrossTalkAndChunk16x16Effect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos linear F rgba cf1lf1) ] ColorSpaceTransform ] ] ] Src" },
    { -1, "RP((RGBA16F+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos linear F rgba cf1lf1) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] SrcOver" },

    { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+full mid mid linear F rgba cf1lf1) ] ColorSpaceTransformPremul ] ] SrcOver" },
    { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+full mid mid linear F rgba cf1lf1) ] Passthrough ] ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
    { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+full mid mid linear F rgba cf1lf1) ] Passthrough ] ] ] SrcOver" },
    { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ RE_LinearEffect_BT2020_HLG__UNKNOWN__false__UNKNOWN__Shader [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos linear F rgba cf1lf1) ] ColorSpaceTransform ] ] ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver" },

    { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ BlendCompose [ Compose [ RE_LinearEffect_BT2020_HLG__UNKNOWN__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos linear F rgba cf1lf1) ] ColorSpaceTransform ] ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
};


void VisitAndroidPrecompileSettings_Pixel8(
         skgpu::graphite::PrecompileContext* precompileContext,
         RuntimeEffectManager& effectManager,
         const std::function<void(skgpu::graphite::PrecompileContext*,
                                  const PrecompileSettings&,
                                  int index)>& func) {

    const skgpu::VulkanYcbcrConversionInfo kPixel8YCbCrs[] = {
        ycbcr_info(238,
                   VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
                   VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
                   VK_CHROMA_LOCATION_MIDPOINT,
                   VK_FILTER_LINEAR,
                   /* samplerFilterMustMatchChromaFilter= */ true,
                   /* supportsLinearFilter= */ true),
        ycbcr_info(238,
                   VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
                   VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
                   VK_CHROMA_LOCATION_MIDPOINT,
                   VK_FILTER_LINEAR,
                   /* samplerFilterMustMatchChromaFilter= */ true,
                   /* supportsLinearFilter= */ true),
        ycbcr_info(238,
                   VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601,
                   VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
                   VK_CHROMA_LOCATION_MIDPOINT,
                   VK_FILTER_LINEAR,
                   /* samplerFilterMustMatchChromaFilter= */ true,
                   /* supportsLinearFilter= */ true),
        ycbcr_info(247,
                   VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020,
                   VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
                   VK_CHROMA_LOCATION_COSITED_EVEN,
                   VK_FILTER_LINEAR,
                   /* samplerFilterMustMatchChromaFilter= */ true,
                   /* supportsLinearFilter= */ true),
    };
    const shaders::LinearEffect kPixel8LinearEffects[] {
        { ui::Dataspace::BT2020_HLG,
          ui::Dataspace::UNKNOWN, false,
          ui::Dataspace::UNKNOWN,
          shaders::LinearEffect::Shader },
    };

    std::vector<PrecompileSettings> precompileCases;
    combine(effectManager, {kPixel8YCbCrs}, {kPixel8LinearEffects}, precompileCases);

    for (size_t i = 0; i < std::size(precompileCases); ++i) {
        func(precompileContext, precompileCases[i], i);
    }
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_VULKAN_CONTEXT(AndroidYCbCrPrecompileTest, reporter, context,
                                     CtsEnforcement::kNever) {
    test(reporter, context, kPixel8Cases, VisitAndroidPrecompileSettings_Pixel8);
}

#endif
