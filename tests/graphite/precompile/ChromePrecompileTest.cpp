/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/PrecompileContext.h"
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileImageFilter.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/PrecompileContextPriv.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "tools/graphite/UniqueKeyUtils.h"

using namespace::skgpu::graphite;

namespace {

// "KnownRuntimeEffect_1DBlur12 [ LocalMatrix [ Compose [ Image(0) ColorSpaceTransform ] ] ] Src",
// "KnownRuntimeEffect_1DBlur8  [ LocalMatrix [ Compose [ Image(0) ColorSpaceTransform ] ] ] Src",
PaintOptions blur_image() {
    PaintOptions paintOptions;
    paintOptions.setImageFilters({ PrecompileImageFilters::Blur(/* input= */ nullptr) });
    paintOptions.setShaders({ PrecompileShaders::Image() });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

// "SolidColor SrcOver"
PaintOptions solid_srcover() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

// "SolidColor Src"
PaintOptions solid_src() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

// "SolidColor Clear", "SolidColor Src" and "SolidColor SrcOver"
PaintOptions solid_clear_src_srcover() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kClear, SkBlendMode::kSrc, SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions image_premul_srcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Image({ &ci, 1 }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

//??
PaintOptions image_premul_src_srcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Image({ &ci, 1 }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc, SkBlendMode::kSrcOver });
    return paintOptions;
}

//!!!!!!!
PaintOptions image_srgb_src() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                     SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kAdobeRGB) };
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Image({ &ci, 1 }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc, /*SkBlendMode::kSrcOver*/ });
    return paintOptions;
}


// "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] Src"
PaintOptions image_src() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Image() });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

// "LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransform ] ] SrcOver"
PaintOptions lineargrad_srcover() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::LinearGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

// "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransform ] ] Dither ] SrcOver"
PaintOptions lineargrad_srcover_dithered() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::LinearGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setDither(/* dither= */ true);
    return paintOptions;
}

// "Compose [ SolidColor Blend [ SolidColor Passthrough BlendModeBlender ] ] SrcOver"
[[maybe_unused]] PaintOptions blend_color_filter_srcover() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setColorFilters({ PrecompileColorFilters::Blend() });
    return paintOptions;
}

// "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver
[[maybe_unused]] PaintOptions blend_porter_duff_color_filter_srcover() {
    PaintOptions paintOptions;
    // kSrcOver will trigger the PorterDuffBlender
    paintOptions.setColorFilters({ PrecompileColorFilters::Blend({ SkBlendMode::kSrcOver }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });

    return paintOptions;
}

// Single sampled R w/ just depth
static const RenderPassProperties kR_1_Depth { DepthStencilFlags::kDepth,
                                               kAlpha_8_SkColorType,
                                               /* fDstCS= */ nullptr,
                                               /* fRequiresMSAA= */ false };
// MSAA R w/ depth and stencil
static const RenderPassProperties kR_4_DepthStencil { DepthStencilFlags::kDepthStencil,
                                                      kAlpha_8_SkColorType,
                                                      /* fDstCS= */ nullptr,
                                                      /* fRequiresMSAA= */ true };
// Single sampled BGRA w/ just depth
static const RenderPassProperties kBGRA_1_Depth { DepthStencilFlags::kDepth,
                                                  kBGRA_8888_SkColorType,
                                                  /* fDstCS= */ nullptr,
                                                  /* fRequiresMSAA= */ false };
// MSAA BGRA w/ just depth
static const RenderPassProperties kBGRA_4_Depth { DepthStencilFlags::kDepth,
                                                  kBGRA_8888_SkColorType,
                                                  /* fDstCS= */ nullptr,
                                                  /* fRequiresMSAA= */ true };
// MSAA BGRA w/ depth and stencil
static const RenderPassProperties kBGRA_4_DepthStencil { DepthStencilFlags::kDepthStencil,
                                                         kBGRA_8888_SkColorType,
                                                         /* fDstCS= */ nullptr,
                                                         /* fRequiresMSAA= */ true };


RenderPassProperties get_render_pass_properties(const char* str) {
    const sk_sp<SkColorSpace> kNoDestCS = nullptr;
    const bool kWithMSAA = true;
    const bool kNoMSAA = false;

    static const struct {
        const char* fStr;
        RenderPassProperties fRenderPassProperties;
    } kRenderPassPropertiesMapping[] = {
        { "RP(color: Dawn(f=1,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: a000)",
          kR_1_Depth },
        { "RP(color: Dawn(f=1,s=4), resolve: Dawn(f=1,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: a000)",
          kR_4_DepthStencil},
        { "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba)",
          kBGRA_1_Depth },
        { "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba)",
          kBGRA_4_Depth },
        { "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba)",
           kBGRA_4_DepthStencil },
    };

    for (const auto& rppm : kRenderPassPropertiesMapping) {
        if (strstr(str, rppm.fStr)) {
            return rppm.fRenderPassProperties;
        }
    }

    SkAssertResult(0);
    return {};
}

// TODO: make sure all the known RenderSteps are accounted for here
DrawTypeFlags get_draw_type_flags(const char* str) {
    static const struct {
        const char* fStr;
        DrawTypeFlags fFlags;
    } kDrawTypeFlagsMapping[] = {
        { "BitmapTextRenderStep[Mask]",          DrawTypeFlags::kBitmapText_Mask },

        { "AnalyticBlurRenderStep",              DrawTypeFlags::kSimpleShape     }, // split out
        { "AnalyticRRectRenderStep",             DrawTypeFlags::kSimpleShape     },
        { "CircularArcRenderStep",               DrawTypeFlags::kSimpleShape     }, // split out
        { "CoverBoundsRenderStep[NonAAFill]",    DrawTypeFlags::kSimpleShape     },
        { "PerEdgeAAQuadRenderStep",             DrawTypeFlags::kSimpleShape     },

        { "CoverBoundsRenderStep[RegularCover]", DrawTypeFlags::kNonSimpleShape  },
        { "CoverBoundsRenderStep[InverseCover]", DrawTypeFlags::kNonSimpleShape  },
        { "CoverageMaskRenderStep",              DrawTypeFlags::kNonSimpleShape  },
        { "MiddleOutFanRenderStep[EvenOdd]",     DrawTypeFlags::kNonSimpleShape  },
        { "MiddleOutFanRenderStep[Winding]",     DrawTypeFlags::kNonSimpleShape  },
        { "TessellateCurvesRenderStep[EvenOdd]", DrawTypeFlags::kNonSimpleShape  },
        { "TessellateCurvesRenderStep[Winding]", DrawTypeFlags::kNonSimpleShape  } ,
        { "TessellateStrokesRenderStep",         DrawTypeFlags::kNonSimpleShape  },
        { "TessellateWedgesRenderStep[Convex]",  DrawTypeFlags::kNonSimpleShape  },
        { "TessellateWedgesRenderStep[Winding]", DrawTypeFlags::kNonSimpleShape  },
        { "TessellateWedgesRenderStep[EvenOdd]", DrawTypeFlags::kNonSimpleShape  },
    };

    for (const auto& dtfm : kDrawTypeFlagsMapping) {
        if (strstr(str, dtfm.fStr)) {
            SkAssertResult(dtfm.fFlags != DrawTypeFlags::kNone);
            return dtfm.fFlags;
        }
    }

    SkAssertResult(0);
    return DrawTypeFlags::kNone;
}


// Precompile with the provided paintOptions, drawType, and RenderPassSettings then verify that
// the expected string is in the generated set.
// Additionally, verify that overgeneration is within expected tolerances.
// If you add an additional RenderStep you may need to increase the tolerance values.
void run_test(PrecompileContext* precompileContext,
              skiatest::Reporter* reporter,
              SkSpan<const char*> cases,
              size_t caseID,
              const PaintOptions& paintOptions,
              DrawTypeFlags drawType,
              const RenderPassProperties& renderPassSettings,
              unsigned int allowedOvergeneration) {
    const char* expectedString = cases[caseID];

    const skgpu::graphite::Caps* caps = precompileContext->priv().caps();
    precompileContext->priv().globalCache()->resetGraphicsPipelines();

    Precompile(precompileContext, paintOptions, drawType, { &renderPassSettings, 1 });

    std::vector<skgpu::UniqueKey> generatedKeys;
    std::vector<std::string> generated;

    {
        const RendererProvider* rendererProvider = precompileContext->priv().rendererProvider();
        const ShaderCodeDictionary* dict = precompileContext->priv().shaderCodeDictionary();


//        SkDebugf("got %zu uniqueKeys\n", generatedKeys.size());
        UniqueKeyUtils::FetchUniqueKeys(precompileContext, &generatedKeys);

        for (const skgpu::UniqueKey& key : generatedKeys) {
            GraphicsPipelineDesc pipelineDesc;
            RenderPassDesc renderPassDesc;
            UniqueKeyUtils::ExtractKeyDescs(precompileContext, key, &pipelineDesc, &renderPassDesc);

            const RenderStep* renderStep = rendererProvider->lookup(pipelineDesc.renderStepID());
            generated.push_back(GetPipelineLabel(caps, dict, renderPassDesc, renderStep,
                                                 pipelineDesc.paintParamsID()));
        }
    }

    bool correctGenerationAmt = generated.size() == allowedOvergeneration;
    REPORTER_ASSERT(reporter, correctGenerationAmt,
                    "case %zu generated unexpected amount - a: %zu != e: %d\n",
                    caseID, generated.size(), allowedOvergeneration);

    const size_t len = strlen(expectedString);

    bool foundIt = false;
    for (size_t i = 0; i < generated.size(); ++i) {
        // The generated strings have trailing whitespace
        if (!strncmp(expectedString, generated[i].c_str(), len)) {
            foundIt = true;
        }
    }

    REPORTER_ASSERT(reporter, foundIt);

#if 0
    SkDebugf("-----------\n");
    for (size_t i = 0; i < cases.size(); ++i) {
        for (size_t j = 0; j < generated.size(); ++j) {
            if (strstr(generated[j].c_str(), cases[i]) || strstr(cases[i], generated[j].c_str())) {
                SkDebugf("case %zu similar to case %zu\n", caseID, i);
            }
        }
    }
#endif

#ifdef SK_DEBUG
    if (foundIt && correctGenerationAmt) {
        return;
    }

    SkDebugf("Expected string:\n%s\n%s in %zu strings:\n",
             expectedString,
             foundIt ? "found" : "NOT found",
             generated.size());

    for (size_t i = 0; i < generated.size(); ++i) {
        generatedKeys[i].dump("");
        SkDebugf("%zu: %s\n", i, generated[i].c_str());
    }
#endif
}

// The pipeline strings were created using the Dawn Metal backend so that is the only viable
// comparison
bool is_dawn_metal_context_type(skgpu::ContextType type) {
    return type == skgpu::ContextType::kDawn_Metal;
}

} // anonymous namespace


DEF_GRAPHITE_TEST_FOR_CONTEXTS(ChromePrecompileTest, is_dawn_metal_context_type,
                               reporter, context, /* testContext */, CtsEnforcement::kNever) {

    std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();
    const skgpu::graphite::Caps* caps = precompileContext->priv().caps();

    TextureInfo textureInfo = caps->getDefaultSampledTextureInfo(kBGRA_8888_SkColorType,
                                                                 skgpu::Mipmapped::kNo,
                                                                 skgpu::Protected::kNo,
                                                                 skgpu::Renderable::kYes);

    TextureInfo msaaTex = caps->getDefaultMSAATextureInfo(textureInfo, Discardable::kYes);

    if (msaaTex.numSamples() <= 1) {
        // The following pipelines rely on having MSAA
        return;
    }

#ifdef SK_ENABLE_VELLO_SHADERS
    if (caps->computeSupport()) {
        // The following pipelines rely on not utilizing Vello
        return;
    }
#endif

    // In the following, here is the Dawn mapping from surface type to ID
    //    R8Unorm = 1
    //    RGBA8Unorm = 18
    //    BGRA8Unorm = 23
    //    Stencil8 = 38
    //    Depth16Unorm = 39
    //    Depth24PlusStencil8 = 41

    //
    // These Pipelines are candidates for inclusion in Chrome's precompile. They were generated
    // by collecting all the Pipelines from the following stories:
    //
    //    animometer_webgl_attrib_arrays, balls_javascript_canvas, canvas_05000_pixels_per_second,
    //    chip_tune, css_value_type_shadow, fill_shapes, ie_chalkboard, main_30fps_impl_60fps,
    //    new_tilings, transform_transitions_js_block, web_animations_staggered_infinite_iterations,
    //    wikipedia_2018
    //
    const char* kCases[] = {
          //---- Single-sample - R8Unorm/Depth16Unorm --> kR_1_Depth
/* 0 */  "RP(color: Dawn(f=1,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: a000) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "KnownRuntimeEffect_1DBlur12 [ LocalMatrix [ Compose [ Image(0) ColorSpaceTransform ] ] ] Src",

/* 1 */  "RP(color: Dawn(f=1,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: a000) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "KnownRuntimeEffect_1DBlur8  [ LocalMatrix [ Compose [ Image(0) ColorSpaceTransform ] ] ] Src",

          //---- MSAA4 - R8Unorm/Depth24PlusStencil8 --> kR_4_DepthStencil
/*- 2 */  "RP(color: Dawn(f=1,s=4), resolve: Dawn(f=1,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: a000) + "
         "TessellateWedgesRenderStep[EvenOdd] + "
         "(empty)",

/*- 3 */  "RP(color: Dawn(f=1,s=4), resolve: Dawn(f=1,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: a000) + "
         "CoverBoundsRenderStep[RegularCover] + "
         "SolidColor SrcOver",

         //---- Single-sample - BGRA8Unorm/Depth16Unorm -> kBGRA_1_Depth
/*- 4 */  "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "AnalyticBlurRenderStep + "
         "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver",

          // For now, we're going to pass on the AnalyticClip Pipelines
/*x 5 */  "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "AnalyticBlurRenderStep + "
         "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip",

/*- 6 */  "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "AnalyticRRectRenderStep + "
         "SolidColor SrcOver",

/*- 7 */  "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "BitmapTextRenderStep[Mask] + "
         "SolidColor SrcOver",

/*- 8 */  "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor SrcOver",

/*- 9 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor Src",

/*- 10 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor Clear",

/*- 11 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src",

          // For now, we're going to pass on the AnalyticClip Pipelines
/*x 12 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip",

/*- 13 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "CoverageMaskRenderStep + "
         "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver",

         // This is the only AlphaOnlyPaintColor case so we're going to pass for now
/*x 14 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",

/*- 15 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "SolidColor SrcOver",

/*- 16 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src",

/*- 17 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",

/*- 18 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Src",

/*- 19 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver",

         //---- MSAA4 - BGRA8Unorm/Depth16Unorm -> bgra_4_depth
         // For now, we're going to pass on the AnalyticClip Pipelines
/*x 20 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip",

/*- 21 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + "
         "TessellateStrokesRenderStep + "
         "SolidColor SrcOver",

/*- 22 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + "
         "TessellateWedgesRenderStep[Convex] + "
         "SolidColor SrcOver",

         //---- MSAA4 - BGRA8Unorm/Depth24PlusStencil8 --> kBGRA_4_DepthStencil
/*- 23 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "AnalyticRRectRenderStep + "
         "SolidColor SrcOver",

/*- 24 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "BitmapTextRenderStep[Mask] + "
         "SolidColor SrcOver",

         // Ugg - this seems like a quirk of our test set. Is linear gradient text that common? Pass for now.
/*x 25 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "BitmapTextRenderStep[Mask] + "
         "LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] SrcOver",

/*- 26 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "CircularArcRenderStep + "
         "SolidColor SrcOver",

/* 27 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[InverseCover] + "
         "(empty)",

/*- 28 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[RegularCover] + "
         "SolidColor SrcOver",

/* 29 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[RegularCover] + "
         "(empty)",

/*- 30 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor SrcOver",

/*- 31 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor Clear",

/*- 32 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",

/*- 33 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",

/*- 34 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "TessellateWedgesRenderStep[Winding] + "
         "(empty)",

/*- 35 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "TessellateWedgesRenderStep[EvenOdd] + "
         "(empty)",

/*- 36 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "TessellateWedgesRenderStep[Convex] + "
         "SolidColor SrcOver",

//----------------------------
// These are leftover Pipelines that were generated by the stories but are pretty infrequently used.
          //---- Single-sample - BGRA8Unorm/Depth16Unorm -> bgra_1_depth
/* 37 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] Dither ] SrcOver",

/*- 38 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",

/* 39 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver",

/* 40 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] SrcOver",

/* 41 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Src",

/* 42 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver",

/* 43 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver",

         //---- MSAA4 - BGRA8Unorm/Depth16Unorm -> bgra_4_depth
/* 44 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + "
         "AnalyticRRectRenderStep + "
         "SolidColor SrcOver",

/* 45 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + "
         "BitmapTextRenderStep[Mask] + "
         "SolidColor SrcOver",

/* 46 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor SrcOver",

/* 47 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor Src",

/* 48 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] Dither ] SrcOver",

/* 49 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver",

/* 50 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",

/* 51 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver",

         //---- MSAA4 - BGRA8Unorm/Depth24PlusStencil8 --> kBGRA_4_DepthStencil
/* 52 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "AnalyticRRectRenderStep + "
         "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] Dither ] SrcOver",

/*- 53 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor Src",

/* 54 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] Dither ] SrcOver",

         // For now, we're going to pass on the AnalyticClip Pipelines
/* 55 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor SrcOver AnalyticClip",

/* 56 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[RegularCover] + "
         "Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformPremul ] ] Dither ] SrcOver",

/*- 57 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "MiddleOutFanRenderStep[Winding] + "
         "(empty)",

/*- 58 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "MiddleOutFanRenderStep[EvenOdd] + "
         "(empty)",

/* 59 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",

/*- 60 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver",

/*- 61 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "TessellateCurvesRenderStep[Winding] + "
         "(empty)",

/*- 62 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "TessellateCurvesRenderStep[EvenOdd] + "
         "(empty)",

/* 63 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "TessellateWedgesRenderStep[Convex] + "
         "Compose [ LocalMatrix [ Compose [ LinearGradientBuffer ColorSpaceTransformPremul ] ] Dither ] SrcOver",

/* 64 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "TessellateWedgesRenderStep[Convex] + "
         "Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformPremul ] ] Dither ] SrcOver",

/*- 65 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
         "TessellateStrokesRenderStep + "
         "SolidColor SrcOver",
    };

#if 1
    const char* kOldCases[] = {
        // Wikipedia 2018 - these are reordered from the spreadsheet
        /*  0 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "TessellateWedgesRenderStep[Winding] + "
                 "(empty)",
        /*  1 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "TessellateWedgesRenderStep[EvenOdd] + "
                 "(empty)",
        /*  2 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "CoverBoundsRenderStep[NonAAFill] + "
                 "SolidColor SrcOver",
        /*  3 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "CoverBoundsRenderStep[NonAAFill] + "
                 "SolidColor Src",
        /*  4 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "PerEdgeAAQuadRenderStep + "
                 "LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver",
        /*  5 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "PerEdgeAAQuadRenderStep + "
                 "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",
        /*  6 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "CoverBoundsRenderStep[NonAAFill] + "
                 "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",
        /*  7 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "AnalyticRRectRenderStep + "
                 "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] Dither ] SrcOver",
        /*  8 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "CoverBoundsRenderStep[NonAAFill] + "
                 "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] Dither ] SrcOver",
        /*  9 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "BitmapTextRenderStep[Mask] + "
                 "LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] SrcOver",
        /* 10 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "BitmapTextRenderStep[Mask] + "
                 "SolidColor SrcOver",
        /* 11 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
                 "AnalyticRRectRenderStep + "
                 "SolidColor SrcOver",
        /* 12 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
                 "CoverBoundsRenderStep[NonAAFill] + "
                 "SolidColor SrcOver",
        /* 13 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
                 "PerEdgeAAQuadRenderStep + "
                 "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src",
        /* 14 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
                 "CoverBoundsRenderStep[NonAAFill] + "
                 "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",
        /* 15 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + "
                 "TessellateWedgesRenderStep[Convex] + "
                 "SolidColor SrcOver",
        /* 16 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + "
                 "TessellateStrokesRenderStep + "
                 "SolidColor SrcOver",
        /* 17 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
                 "AnalyticBlurRenderStep + "
                 "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver",
        /* 18 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
                 "CoverBoundsRenderStep[NonAAFill] + "
                 "SolidColor Src",
        /* 19 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
                 "CoverBoundsRenderStep[NonAAFill] + "
                 "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] Dither ] SrcOver",
    };
#endif

#if 0
    mapping from old to new cases:
    0 -> 34
    1 -> 35
    2 -> 30
    3 -> 53
    4 -> 60
    5 -> 33
    6 -> 32
    7 -> 52
    8 -> 54
    9 -> 25
    10 -> 24
    11 -> 6
    12 -> 8
    13 -> 16
    14 -> 38
    15 -> 22
    16 -> 21
    17 -> 4
    18 -> 9
    19 -> 37


    for (size_t i = 0; i < std::size(kOldCases); ++i) {
        bool foundIt = false;
        for (size_t j = 0; j < std::size(kCases); ++j) {
            if (!strcmp(kOldCases[i], kCases[j])) {
                SkDebugf("%zu -> %zu\n", i, j);
                foundIt = true;
                break;
            }
        }

        if (!foundIt) {
            SkDebugf("oldcase %zu not found in new cases\n", i);
        }
    }

    exit(0);
#endif

    for (size_t i = 0; i < std::size(kCases); ++i) {
//        if (i > 36) {
//            // After this point the Pipelines become pretty rarely used.
//            continue;
//        }


        PaintOptions paintOptions;
        RenderPassProperties renderPassSettings;
        DrawTypeFlags drawTypeFlags = DrawTypeFlags::kNone;

        // TODO: split kCases[i] into substrings before passing to helpers
        RenderPassProperties expectedRenderPassSettings = get_render_pass_properties(kCases[i]);
        DrawTypeFlags expectedDrawTypeFlags = get_draw_type_flags(kCases[i]);
        unsigned int expectedNumPipelines = 0;

        switch (i) {
#if 0 // broken!
            case 0:             [[fallthrough]];
            case 1:
                paintOptions = blur_image();
                allowedOvergeneration = 11;
                break;
#endif

            // 2 and 3 form a single-draw pair and are, thus, addressed by the same settings
            case 2: [[fallthrough]];   // TessellateWedgesRenderStep[EvenOdd]
            case 3:                    // kSrcOver - CoverBoundsRenderStep[RegularCover]
                renderPassSettings = kR_4_DepthStencil;
                drawTypeFlags = DrawTypeFlags::kNonSimpleShape;
                paintOptions = solid_srcover();
                expectedNumPipelines = 11;   // This is pretty bad for just 2 Pipelines
                break;

            // 4 and 13 share the same paintOptions
            case 4:  // AnalyticBlurRenderStep
                // 4 could be reduced  to 1 Pipeline if AnalyticBlurRenderStep were split out
                renderPassSettings = kBGRA_1_Depth;
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                paintOptions = blend_porter_duff_color_filter_srcover();
                expectedNumPipelines = 5; // This is pretty bad for just 1 Pipeline
                break;

            case 13: // CoverageMaskRenderStep
                // 13 could be reduced to 1 Pipeline if CoverageMaskRenderStep were split out
                renderPassSettings = kBGRA_1_Depth;
                drawTypeFlags = DrawTypeFlags::kNonSimpleShape;
                paintOptions = blend_porter_duff_color_filter_srcover();
                expectedNumPipelines = 11; // This is pretty bad for just 1 Pipeline
                break;

#if 0 // revisit
            // For now, we're passing on the AnalyticClip Pipelines
            case 5:  // same as 4 but with an AnalyticClip
            case 12:
            case 20:
            case 55: /* dk */
                continue;
#endif

            // 7 and 24 just differ in render pass settings (one MSAA, the other not)
            case 7:
                renderPassSettings = kBGRA_1_Depth;
                drawTypeFlags = DrawTypeFlags::kBitmapText_Mask;
                paintOptions = solid_srcover();
                expectedNumPipelines = 1;
                break;
            case 24:
                renderPassSettings = kBGRA_4_DepthStencil;
                drawTypeFlags = DrawTypeFlags::kBitmapText_Mask;
                paintOptions = solid_srcover();
                expectedNumPipelines = 1;
                break;

            // 21 & 22 form a pair (since they both need kNonSimpleShape)
            case 21: // kSrcOver - TessellateStrokesRenderStep
            case 22: // kSrcOver - TessellateWedgesRenderStep[Convex]
                renderPassSettings = kBGRA_4_Depth;
                drawTypeFlags = DrawTypeFlags::kNonSimpleShape;
                paintOptions = solid_srcover();
                expectedNumPipelines = 11;  // very bad for 2 pipelines
                break;

            // the next 4 form a block (since they all need kNonSimpleShape)
            case 28: // kSrcOver - CoverBoundsRenderStep[RegularCover]
//??? - this isn't generating            case 27:  // CoverBoundsRenderStep[InverseCover]
//??? - this isn't generating            case 29:  // CoverBoundsRenderStep[RegularCover]
            case 34: // TessellateWedgesRenderStep[Winding]
            case 35: // TessellateWedgesRenderStep[EvenOdd]
            case 36: // kSrcOver - TessellateWedgesRenderStep[Convex]
            // related utility Pipelines
            case 57: // MiddleOutFanRenderStep[Winding]
            case 58: // MiddleOutFanRenderStep[EvenOdd]
            case 61: // TessellateCurvesRenderStep[Winding]
            case 62: // TessellateCurvesRenderStep[EvenOdd]
            case 65: // kSrcOver - TessellateStrokesRenderStep
                renderPassSettings = kBGRA_4_DepthStencil;
                drawTypeFlags = DrawTypeFlags::kNonSimpleShape;
                paintOptions = solid_srcover();
                expectedNumPipelines = 11;  // not so bad for 9 pipelines
                break;

            // These are the same except for the blend mode
            // kSrcOver seems fine. kSrc and kClear seem like they should be narrowed
            case 6:  // kSrcOver - AnalyticRRectRenderStep
            case 8:  // kSrcOver - CoverBoundsRenderStep[NonAAFill]
            case 9:  // kSrc     - CoverBoundsRenderStep[NonAAFill]
            case 10: // kClear   - CoverBoundsRenderStep[NonAAFill]
            case 15: // kSrcOver - PerEdgeAAQuadRenderStep
                renderPassSettings = kBGRA_1_Depth;
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                paintOptions = solid_clear_src_srcover();
                // This is 5 each for kClear, kSrc and kSrcOver:
                //     AnalyticBlurRenderStep              -- split out
                //     AnalyticRRectRenderStep
                //     CircularArcRenderStep               -- split out
                //     CoverBoundsRenderStep[NonAAFill]
                //     PerEdgeAAQuadRenderStep
                expectedNumPipelines = 15; // This is pretty bad for 5 pipelines
                break;

            case 23: // kSrcOver - AnalyticRRectRenderStep
            case 26: // kSrcOver - CircularArcRenderStep
            case 30: // kSrcOver - CoverBoundsRenderStep[NonAAFill]
            case 31: // kClear   - CoverBoundsRenderStep[NonAAFill]
            case 53: // kSrc     - CoverBoundsRenderStep[NonAAFill]
                renderPassSettings = kBGRA_4_DepthStencil;
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                paintOptions = solid_clear_src_srcover();
                // This is 5 each for kClear, kSrc and kSrcOver:
                //     AnalyticBlurRenderStep              -- split out
                //     AnalyticRRectRenderStep
                //     CircularArcRenderStep               -- split out
                //     CoverBoundsRenderStep[NonAAFill]
                //     PerEdgeAAQuadRenderStep
                expectedNumPipelines = 15; // This is pretty bad for 5 pipelines
                break;

            // For all the image paintOptions we could add the option to exclude cubics to
            // the public API
            case 11: // CoverBoundsRenderStep[NonAAFill] + HardwareImage(0) + kSrc
            case 16: // PerEdgeAAQuadRenderStep + HardwareImage(0) + kSrc
            case 17: // PerEdgeAAQuadRenderStep + HardwareImage(0) + kSrcOver
            case 19: // PerEdgeAAQuadRenderStep + Image(0) + kSrcOver
            case 38: // CoverBoundsRenderStep[NonAAFill] + HardwareImage(0) + kSrcOver
                renderPassSettings = kBGRA_1_Depth;
                paintOptions = image_premul_src_srcover();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                expectedNumPipelines = 30; // a bad deal for 5 pipelines
                break;

            // same as except 16 except it has ColorSpaceTransformSRGB
            case 18:
                expectedRenderPassSettings.fDstCS = SkColorSpace::MakeSRGB();
                renderPassSettings = kBGRA_1_Depth;
                renderPassSettings.fDstCS = SkColorSpace::MakeSRGB();
                paintOptions = image_srgb_src();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                expectedNumPipelines = 15;  // a bad deal for 1 pipeline
                break;

            //---
            case 32: // CoverBoundsRenderStep[NonAAFill] + HardwareImage(0) + kSrcOver
            case 33: // PerEdgeAAQuadRenderStep + HardwareImage(0) + kSrcOver
            case 60: // PerEdgeAAQuadRenderStep + Image(0) + kSrcOver
                renderPassSettings = kBGRA_4_DepthStencil;
                paintOptions = image_premul_srcover();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                expectedNumPipelines = 15; // a bad deal for 3 pipelines
                break;

            default:
                continue;
        }

#if 0
        switch (i) {
            case 0:            [[fallthrough]];
            case 1:
                paintOptions = solid_srcover();
                drawTypeFlags = DrawTypeFlags::kNonSimpleShape;
                allowedOvergeneration = 11;
                break;
            case 2:
                paintOptions = solid_srcover();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                allowedOvergeneration = 5;
                break;
            case 3: // only differs from 18 by MSAA and depth vs depth-stencil
                paintOptions = solid_src();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                allowedOvergeneration = 5; // a lot for a rectangle clear - all RenderSteps
                break;
            case 4: // 4 is part of an AA image rect draw that can't use HW tiling
            case 5: // 5 & 6 together make up an AA image rect draw w/ a filled center
            case 6:
                paintOptions = image_srcover();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                allowedOvergeneration = 60;
                break;
            case 7: // 7 & 8 are combined pair
            case 8:
                paintOptions = lineargrad_srcover_dithered();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                allowedOvergeneration = 15; // 3x from gradient, 12x from RenderSteps
                break;
            case 9:
                paintOptions = lineargrad_srcover();
                drawTypeFlags = DrawTypeFlags::kBitmapText_Mask;
                allowedOvergeneration = 3; // from the 3 internal gradient alternatives
                break;
            case 10:
                paintOptions = solid_srcover();
                drawTypeFlags = DrawTypeFlags::kBitmapText_Mask;
                allowedOvergeneration = 1;
                break;
            case 11: // 11 & 12 are a pair - an RRect draw w/ a non-aa-fill center
            case 12:
                paintOptions = solid_srcover();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                allowedOvergeneration = 5;  // all from RenderSteps
                break;
            case 13:
                paintOptions = image_src();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                // This is a lot for a kSrc image draw:
                allowedOvergeneration = 60; // 6x of this are the paint combos,
                                            // the rest are the RenderSteps!!
                break;
            case 14:
                paintOptions = image_srcover();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                allowedOvergeneration = 60; // !!!! - a lot for just a non-aa image rect draw
                break;
            case 15:
            case 16:
                paintOptions = solid_srcover();
                drawTypeFlags = DrawTypeFlags::kNonSimpleShape;
                allowedOvergeneration = 11;
                break;
            case 17:
                // After https://skia-review.googlesource.com/c/skia/+/887476 ([graphite] Split up
                // universal blend shader snippet) this case no longer exists/is reproducible.
                //
                //  paintOptions = blend_color_filter_srcover();
                //  drawTypeFlags = DrawTypeFlags::kSimpleShape;
                //  renderPassSettings = bgra_1_depth();
                //  allowedOvergeneration = 4;
                continue;
            case 18: // only differs from 3 by MSAA and depth vs depth-stencil
                paintOptions = solid_src();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                allowedOvergeneration = 5; // a lot for a rectangle clear - all RenderSteps
                break;
            case 19:
                paintOptions = lineargrad_srcover_dithered();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                allowedOvergeneration = 15; // 3x from gradient, rest from RenderSteps
                break;
            default:
                continue;
        }
#endif

        SkAssertResult(renderPassSettings == expectedRenderPassSettings);
        SkAssertResult(drawTypeFlags == expectedDrawTypeFlags);

        if (renderPassSettings.fRequiresMSAA && caps->loadOpAffectsMSAAPipelines()) {
            expectedNumPipelines *= 2; // due to wgpu::LoadOp::ExpandResolveTexture
        }

        run_test(precompileContext.get(), reporter,
                 { kCases, std::size(kCases) }, i,
                 paintOptions, drawTypeFlags, renderPassSettings, expectedNumPipelines);
    }
}

#endif // SK_GRAPHITE
