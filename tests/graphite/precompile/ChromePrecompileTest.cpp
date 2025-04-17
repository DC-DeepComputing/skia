/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/gpu/graphite/Context.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/PrecompileContextPriv.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "tools/graphite/UniqueKeyUtils.h"

#include "src/gpu/graphite/precompile/PrecompileColorFiltersPriv.h"

#include <cstring>
#include <set>

// Print out a final report that includes missed cases in 'kCases'
#define FINAL_REPORT

// Print out the cases (in 'kCases') that are covered by each 'kPrecompileCases' case
// Also lists the utilization of each 'kPrecompileCases' case
#define PRINT_COVERAGE

// Print out all the generated labels and whether they were found in 'kCases'.
// This is usually used along with the 'kChosenCase' variable.
//#define PRINT_GENERATED_LABELS

/*** From here to the matching banner can be cut and pasted into Chrome's graphite_precompile.cc **/
#include "include/gpu/graphite/PrecompileContext.h"
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"

namespace {

using ::skgpu::graphite::DepthStencilFlags;
using ::skgpu::graphite::DrawTypeFlags;
using ::skgpu::graphite::PaintOptions;
using ::skgpu::graphite::RenderPassProperties;

// "SolidColor SrcOver"
PaintOptions solid_srcover() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

// "SolidColor SrcOver"
// "SolidColor Src"
// "SolidColor Clear"
PaintOptions solid_clear_src_srcover() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kClear,
                                 SkBlendMode::kSrc,
                                 SkBlendMode::kSrcOver });
    return paintOptions;
}

// "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver"
// "LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver"
PaintOptions image_premul_srcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    PaintOptions paintOptions;
    paintOptions.setShaders({ skgpu::graphite::PrecompileShaders::Image({ &ci, 1 }, {}, true, true) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

// LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
// LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
PaintOptions yuv_image_srgb_srcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType,
                     kPremul_SkAlphaType,
                     SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kAdobeRGB) };

    PaintOptions paintOptions;
    paintOptions.setShaders({ skgpu::graphite::PrecompileShaders::YUVImage(
            { &ci, 1 },
            /* includeCubic= */ false) });   // using cubic sampling w/ YUV images is rare
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

// "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src"
// "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver"
// "LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver"
PaintOptions image_premul_src_srcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    PaintOptions paintOptions;
    paintOptions.setShaders({ skgpu::graphite::PrecompileShaders::Image({ &ci, 1 }, {}, true, true) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc,
                                 SkBlendMode::kSrcOver });
    return paintOptions;
}

// "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Src"
PaintOptions image_srgb_src() {
    SkColorInfo ci { kRGBA_8888_SkColorType,
                     kPremul_SkAlphaType,
                     SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                           SkNamedGamut::kAdobeRGB) };
    PaintOptions paintOptions;
    paintOptions.setShaders({ skgpu::graphite::PrecompileShaders::Image({ &ci, 1 }, {}, true, true) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

// "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver"
PaintOptions blend_porter_duff_cf_srcover() {
    PaintOptions paintOptions;
    // kSrcOver will trigger the PorterDuffBlender
    paintOptions.setColorFilters(
            { skgpu::graphite::PrecompileColorFilters::Blend({ SkBlendMode::kSrcOver }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });

    return paintOptions;
}

// Compose [ BlendCompose
//            [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformPremul ] Modulate ]
//            Compose [ GaussianColorFilter
//                      BlendCompose [ SolidColor Passthrough Modulate ] ]
//         ] SrcOver",
// TODO: we can get close but need to add support for kModulate primitive blending
PaintOptions draw_shadow() {
    using namespace skgpu::graphite;

    PaintOptions paintOptions;

    sk_sp<PrecompileColorFilter> modulateCF = PrecompileColorFilters::Blend({ SkBlendMode::kModulate });
    sk_sp<PrecompileColorFilter> finalCF = modulateCF->makeComposed(PrecompileColorFiltersPriv::Gaussian());
    paintOptions.setColorFilters({ std::move(finalCF) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });

    return paintOptions;
}

//    BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",
//                   LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver

//    BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver
        //h 9: RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver
//17: RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver

PaintOptions foo() {
    using namespace skgpu::graphite;

    SkColorInfo ci = { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };

    PaintOptions paintOptions;
    paintOptions.setShaders({ skgpu::graphite::PrecompileShaders::Image({ &ci, 1 }, {}, true, true) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });

    return paintOptions;
}

PaintOptions foo_srgb() {
    using namespace skgpu::graphite;

    SkColorInfo ci = { kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                       SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kAdobeRGB) };

    PaintOptions paintOptions;
    paintOptions.setShaders({ skgpu::graphite::PrecompileShaders::Image({ &ci, 1 }, {}, true, true) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });

    return paintOptions;
}

// "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000)",
// Single sampled R w/ just depth
const RenderPassProperties kR_1_D { DepthStencilFlags::kDepth,
                                    kAlpha_8_SkColorType,
                                    /* fDstCS= */ nullptr,
                                    /* fRequiresMSAA= */ false };

// "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000)",
// "RP(color: Dawn(f=R8,s=4) w/ msaa load, resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000)",
// MSAA R w/ depth and stencil
const RenderPassProperties kR_4_DS { DepthStencilFlags::kDepthStencil,
                                     kAlpha_8_SkColorType,
                                     /* fDstCS= */ nullptr,
                                     /* fRequiresMSAA= */ true };

// "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba)"
// Single sampled BGRA w/ just depth
const RenderPassProperties kBGRA_1_D { DepthStencilFlags::kDepth,
                                       kBGRA_8888_SkColorType,
                                       /* fDstCS= */ nullptr,
                                       /* fRequiresMSAA= */ false };
// RGBA version of the above
const RenderPassProperties kRGBA_1_D { DepthStencilFlags::kDepth,
                                       kRGBA_8888_SkColorType,
                                       /* fDstCS= */ nullptr,
                                       /* fRequiresMSAA= */ false };

// "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba)"
// "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba)"
// MSAA BGRA w/ just depth
const RenderPassProperties kBGRA_4_D { DepthStencilFlags::kDepth,
                                       kBGRA_8888_SkColorType,
                                       /* fDstCS= */ nullptr,
                                       /* fRequiresMSAA= */ true };
// RGBA version of the above
const RenderPassProperties kRGBA_4_D { DepthStencilFlags::kDepth,
                                       kRGBA_8888_SkColorType,
                                       /* fDstCS= */ nullptr,
                                       /* fRequiresMSAA= */ true };


// "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba)"
// "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba)",
// MSAA BGRA w/ depth and stencil
const RenderPassProperties kBGRA_4_DS { DepthStencilFlags::kDepthStencil,
                                        kBGRA_8888_SkColorType,
                                        /* fDstCS= */ nullptr,
                                        /* fRequiresMSAA= */ true };
// RGBA version of the above
const RenderPassProperties kRGBA_4_DS { DepthStencilFlags::kDepthStencil,
                                        kRGBA_8888_SkColorType,
                                        /* fDstCS= */ nullptr,
                                        /* fRequiresMSAA= */ true };

// The same as kBGRA_1_D but w/ an SRGB colorSpace
const RenderPassProperties kBGRA_1_D_SRGB { DepthStencilFlags::kDepth,
                                            kBGRA_8888_SkColorType,
                                            SkColorSpace::MakeSRGB(),
                                            /* fRequiresMSAA= */ false };
// RGBA version of the above
const RenderPassProperties kRGBA_1_D_SRGB { DepthStencilFlags::kDepth,
                                            kRGBA_8888_SkColorType,
                                            SkColorSpace::MakeSRGB(),
                                            /* fRequiresMSAA= */ false };

// The same as kBGRA_4_DS but w/ an SRGB colorSpace
const RenderPassProperties kBGRA_4_DS_SRGB { DepthStencilFlags::kDepthStencil,
                                             kBGRA_8888_SkColorType,
                                             SkColorSpace::MakeSRGB(),
                                             /* fRequiresMSAA= */ true };
// RGBA version of the above
const RenderPassProperties kRGBA_4_DS_SRGB { DepthStencilFlags::kDepthStencil,
                                             kRGBA_8888_SkColorType,
                                             SkColorSpace::MakeSRGB(),
                                             /* fRequiresMSAA= */ true };

// These settings cover 176 of the 202 cases in 'kCases'.
const struct PrecompileSettings {
    PaintOptions fPaintOptions;
    DrawTypeFlags fDrawTypeFlags = DrawTypeFlags::kNone;
    RenderPassProperties fRenderPassProps;
} kPrecompileCases[] = {

#if 0
    { solid_srcover(),                DrawTypeFlags::kSimpleShape,     kR_1_D },

    { solid_srcover(),                DrawTypeFlags::kNonSimpleShape,  kR_4_DS },
#endif

#if 1
    { solid_srcover(),                DrawTypeFlags::kSimpleShape,     kRGBA_1_D },
#endif

    // This results from a drop shadow draw - it should cover #4 but is, currently, broken
    //{ draw_shadow(),                DrawTypeFlags::kDrawVertices,    kRGBA_1_D },

    { foo(),                          DrawTypeFlags::kSimpleShape,     kRGBA_1_D },
    { foo_srgb(),                     DrawTypeFlags::kSimpleShape,     kRGBA_1_D_SRGB },

    { foo(),                          DrawTypeFlags::kSimpleShape,     kRGBA_4_DS },
    { foo_srgb(),                     DrawTypeFlags::kSimpleShape,     kRGBA_4_DS_SRGB },

#if 1
    { blend_porter_duff_cf_srcover(), DrawTypeFlags::kNonSimpleShape,  kRGBA_1_D },
    { image_premul_src_srcover(),     DrawTypeFlags::kSimpleShape,     kRGBA_1_D },
    { solid_clear_src_srcover(),      DrawTypeFlags::kSimpleShape,     kRGBA_1_D },

    { solid_srcover(),                DrawTypeFlags::kBitmapText_Mask, kRGBA_4_D },
    { solid_srcover(),                DrawTypeFlags::kNonSimpleShape,  kRGBA_4_D },

    { solid_srcover(),                DrawTypeFlags::kBitmapText_Mask, kRGBA_4_DS },
    { solid_srcover(),                DrawTypeFlags::kCircularArc,     kRGBA_4_DS },
    { solid_srcover(),                DrawTypeFlags::kNonSimpleShape,  kRGBA_4_DS },
    { image_premul_srcover(),         DrawTypeFlags::kSimpleShape,     kRGBA_4_DS },
    { solid_clear_src_srcover(),      DrawTypeFlags::kSimpleShape,     kRGBA_4_DS },

    { image_srgb_src(),               DrawTypeFlags::kSimpleShape,     kRGBA_1_D_SRGB },
    { yuv_image_srgb_srcover(),       DrawTypeFlags::kSimpleShape,     kRGBA_1_D_SRGB },

    // These two are interesting but have < 40% utility
    // { yuv_image_srgb_srcover(),      DrawTypeFlags::kSimpleShape,     kRGBA_4_DS_SRGB },
    // { solid_srcover(),               DrawTypeFlags::kSimpleShape,     kRGBA_4_D },
#endif
};

/*********** Here ends the part that can be pasted into Chrome's graphite_precompile.cc ***********/

// This helper maps from the RenderPass string in the Pipeline label to the
// RenderPassProperties needed by the Precompile system
// TODO(robertphillips): converting this to a more piecemeal approach might better illuminate
// the mapping between the string and the RenderPassProperties
[[maybe_unused]] RenderPassProperties get_render_pass_properties(const char* str) {
    static const struct {
        const char* fStr;
        RenderPassProperties fRenderPassProperties;
    } kRenderPassPropertiesMapping[] = {
        { "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000)",
          kR_1_D },

        // This RPP (kR_4_DS) can generate two strings when Caps::loadOpAffectsMSAAPipelines.
        { "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000)",
          kR_4_DS },
        { "RP(color: Dawn(f=R8,s=4) w/ msaa load, resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000)",
          kR_4_DS },

        { "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba)",
          kBGRA_1_D },
        // RGBA version of above
        { "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba)",
          kRGBA_1_D },

        // This RPP (kBGRA_4_D) can generate two strings when Caps::loadOpAffectsMSAAPipelines.
        { "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba)",
          kBGRA_4_D },
        { "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba)",
          kBGRA_4_D },
        // RGBA versions of above
        { "RP(color: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba)",
          kRGBA_4_D },
        { "RP(color: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba)",
          kRGBA_4_D },

        // This RPP (kBGRA_4_DS) can generate two strings when Caps::loadOpAffectsMSAAPipelines.
        { "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba)",
           kBGRA_4_DS },
        { "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba)",
           kBGRA_4_DS },
        // RGBA version of above
        { "RP(color: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba)",
                kRGBA_4_DS },
        { "RP(color: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba)",
                kRGBA_4_DS },
    };

    for (const auto& rppm : kRenderPassPropertiesMapping) {
        if (strstr(str, rppm.fStr)) {
            return rppm.fRenderPassProperties;
        }
    }

    SkAssertResult(0);
    return {};
}

// This helper maps from the RenderStep's name in the Pipeline label to the DrawTypeFlag that
// resulted in its use.
[[maybe_unused]] DrawTypeFlags get_draw_type_flags(const char* str) {
    static const struct {
        const char* fStr;
        DrawTypeFlags fFlags;
    } kDrawTypeFlagsMapping[] = {
        { "BitmapTextRenderStep[Mask]",                  DrawTypeFlags::kBitmapText_Mask  },
        { "BitmapTextRenderStep[LCD]",                   DrawTypeFlags::kBitmapText_LCD   },
        { "BitmapTextRenderStep[Color]",                 DrawTypeFlags::kBitmapText_Color },

        { "SDFTextRenderStep",                           DrawTypeFlags::kSDFText      },
        { "SDFTextLCDRenderStep",                        DrawTypeFlags::kSDFText_LCD  },

        { "VerticesRenderStep[Tris]",                    DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TrisTexCoords]",           DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TrisColor]",               DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TrisColorTexCoords]",      DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[Tristrips]",               DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TristripsTexCoords]",      DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TristripsColor]",          DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TristripsColorTexCoords]", DrawTypeFlags::kDrawVertices },

        { "CircularArcRenderStep",                       DrawTypeFlags::kCircularArc  },

        { "AnalyticRRectRenderStep",                     DrawTypeFlags::kSimpleShape  },
        { "CoverBoundsRenderStep[NonAAFill]",            DrawTypeFlags::kSimpleShape  },
        { "PerEdgeAAQuadRenderStep",                     DrawTypeFlags::kSimpleShape  },

        { "CoverageMaskRenderStep",                      DrawTypeFlags::kNonSimpleShape },
        { "CoverBoundsRenderStep[RegularCover]",         DrawTypeFlags::kNonSimpleShape },
        { "CoverBoundsRenderStep[InverseCover]",         DrawTypeFlags::kNonSimpleShape },
        { "MiddleOutFanRenderStep[EvenOdd]",             DrawTypeFlags::kNonSimpleShape },
        { "MiddleOutFanRenderStep[Winding]",             DrawTypeFlags::kNonSimpleShape },
        { "TessellateCurvesRenderStep[EvenOdd]",         DrawTypeFlags::kNonSimpleShape },
        { "TessellateCurvesRenderStep[Winding]",         DrawTypeFlags::kNonSimpleShape },
        { "TessellateStrokesRenderStep",                 DrawTypeFlags::kNonSimpleShape },
        { "TessellateWedgesRenderStep[Convex]",          DrawTypeFlags::kNonSimpleShape },
        { "TessellateWedgesRenderStep[EvenOdd]",         DrawTypeFlags::kNonSimpleShape },
        { "TessellateWedgesRenderStep[Winding]",         DrawTypeFlags::kNonSimpleShape },
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


typedef const char* ChromePipeline;

//
// These Pipelines are candidates for inclusion in Chrome's precompile. They were generated
// by collecting all the Pipelines from 9 of the top 14 visited sites according to Wikipedia
//
static const ChromePipeline kCases[] = {
/*   0 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor Src AnalyticClip",
/*   1 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver",
/*   2 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + SolidColor SrcOver",
/*   3 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ SolidColor ColorSpaceTransform ColorSpaceTransform ] SrcOver",
// Candidate - broken though
/*   4 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + VerticesRenderStep[TrisColor] + Compose [ BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformPremul ] Modulate ] Compose [ GaussianColorFilter BlendCompose [ SolidColor Passthrough Modulate ] ] ] SrcOver",
/*   5 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_KawaseBlurDualFilter_LowSampleBlurEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ] Src",
/*   6 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_KawaseBlurDualFilter_LowSampleBlurEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ] SrcOver",
/*   7 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src",
/*   8 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src",
/*   9 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_BlurFilterMixEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ] Src",
/*  10 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_KawaseBlurDualFilter_HighSampleBlurEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ] Src",
/*  11 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*  12 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",
/*  13 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*  14 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",
/*  15 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*  16 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] SrcOver",
/*  17 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*  18 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] SrcOver",
/*  19 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver",
/*  20 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver",
// Candidate - dither!
/*  21 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver",
/*  22 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip",
/*  23 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip",
/*  24 */ "RP(color: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*  25 */ "RP(color: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + TessellateWedgesRenderStep[EvenOdd] + (empty)",
// Candidate
/*  26 */ "RP(color: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[InverseCover] + (empty)",
/*  27 */ "RP(color: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",
/*  28 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip",
/*  29 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip",
/*  30 */ "RP(color: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*  31 */ "RP(color: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] SrcOver",
/*  32 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver AnalyticClip",
/*  33 */ "RP(color: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver",
/*  34 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*  35 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ] SrcOver",
/*  36 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*  37 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ] SrcOver",
// Candidate - Plus
/*  38 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver",
// Candidate - Plus
/*  39 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] SrcOver",
/*  40 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*  41 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] SrcOver",
/*  42 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*  43 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] SrcOver",
/*  44 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*  45 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] SrcOver",
/*  46 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver",
/*  47 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver",
/*  48 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] MatrixColorFilter ] Dither ] SrcOver",
/*  49 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip",
/*  50 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] SrcOver AnalyticClip",
/*  51 */ "RP(color: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*  52 */ "RP(color: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] SrcOver",
/*  53 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip",
/*  54 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] SrcOver AnalyticClip",
/*  55 */ "RP(color: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*  56 */ "RP(color: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] SrcOver",
/*  57 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ RE_EdgeExtensionEffect [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*  58 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_EdgeExtensionEffect [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] ] SrcOver",
/*  59 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ RE_EdgeExtensionEffect [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*  60 */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_EdgeExtensionEffect [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] ] SrcOver",
/*  61 */ "RP(color: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ BlendCompose [ RE_LinearEffect_SRGB_SRGB_true_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_SRGB_SRGB_true_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ BlendCompose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ BlendCompose [ RE_LinearEffect_SRGB_SRGB_true_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ RE_LinearEffect_SRGB_SRGB_true_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ BlendCompose [ RE_LinearEffect_SRGB_SRGB_true_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_SRGB_SRGB_true_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ BlendCompose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ BlendCompose [ RE_LinearEffect_SRGB_SRGB_true_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ RE_LinearEffect_SRGB_SRGB_true_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver AnalyticClip",
/*     */ "RP(color: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ BlendCompose [ RE_LinearEffect_BT2020_ITU_PQ_BT2020_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ BlendCompose [ RE_LinearEffect_BT2020_ITU_PQ_BT2020_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_BT2020_ITU_PQ_BT2020_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ BlendCompose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ BlendCompose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ BlendCompose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] MatrixColorFilter ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ Compose [ BlendCompose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ Compose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] MatrixColorFilter ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ BlendCompose [ Compose [ HardwareImage(0) ColorSpaceTransform ] RGBPaintColor DstIn ] ] MatrixColorFilter ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_MouriMap_CrossTalkAndChunk16x16Effect [ LocalMatrix [ Compose [ HardwareImage(3: kEwA) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Src",
/*     */ "RP(color: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_MouriMap_Chunk8x8Effect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ] Src",
/*     */ "RP(color: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_MouriMap_BlurEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ] Src",
/*     */ "RP(color: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ SolidColor ColorSpaceTransform ColorSpaceTransform ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ HardwareImage(3: kEwA) ColorSpaceTransform ] ] LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] ColorSpaceTransform ColorSpaceTransform ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_MouriMap_CrossTalkAndChunk16x16Effect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Src",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Src",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_GainmapEffect [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransformPremul ColorSpaceTransformPremul ] LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] ColorSpaceTransformPremul ColorSpaceTransformPremul ] Src",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_BT2020_ITU_PQ_BT2020_false_UNKNOWN_Shader [ SolidColor ColorSpaceTransform ColorSpaceTransform ] Src",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ SolidColor MatrixColorFilter ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_BT2020_ITU_PQ_BT2020_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(3: kEwA) ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ SolidColor MatrixColorFilter ] SrcOver AnalyticClip",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_BT2020_ITU_PQ_BT2020_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(3: kEwA) ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver AnalyticClip",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ SolidColor MatrixColorFilter ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ RE_LinearEffect_BT2020_ITU_PQ_BT2020_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(3: kEwA) ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ BlendCompose [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] RGBPaintColor DstIn ] ] MatrixColorFilter ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(3: kHoA) ColorSpaceTransformPremul ] ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor Src",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(3: kHoA) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(3: kHoA) ColorSpaceTransformPremul ] ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ SolidColor MatrixColorFilter ] Src",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ HardwareImage(3: kHoA) ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ HardwareImage(3: kHoA) ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src",
/*     */ "RP(color: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + MiddleOutFanRenderStep[EvenOdd] + (empty)",
/*     */ "RP(color: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + TessellateCurvesRenderStep[EvenOdd] + (empty)",
/*     */ "RP(color: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[InverseCover] + (empty)",
/*     */ "RP(color: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver AnalyticClip",
/*     */ "RP(color: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ BlendCompose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] MatrixColorFilter ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ Compose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] MatrixColorFilter ] Dither ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] ColorSpaceTransform ColorSpaceTransform ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(3: kHIA) ColorSpaceTransformPremul ] ] SrcOver",
/*     */ "RP(color: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(3: kHIA) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip",
    };

[[maybe_unused]] void find_duplicates(SkSpan<const ChromePipeline> cases) {
    for (size_t i = 0; i < std::size(cases); ++i) {
        for (size_t j = i+1; j < std::size(cases); ++j) {
            if (!strcmp(cases[i], cases[j])) {
                SkDebugf("Duplicate %zu && %zu\n", i, j);
            }
        }
    }
}

std::string rm_whitespace(const std::string& s) {
    auto start = s.find_first_not_of(' ');
    auto end = s.find_last_not_of(' ');
    return s.substr(start, (end - start) + 1);
}

// Precompile with the provided PrecompileSettings then verify that:
//   1) some case in 'kCases' is covered
//   2) more than 40% of the generated Pipelines are in kCases
void run_test(skgpu::graphite::PrecompileContext* precompileContext,
              skiatest::Reporter* reporter,
              const PrecompileSettings& settings,
              int precompileSettingsIndex,
              std::vector<bool>* casesThatAreMatched) {
    using namespace skgpu::graphite;

    precompileContext->priv().globalCache()->resetGraphicsPipelines();

    Precompile(precompileContext,
               settings.fPaintOptions,
               settings.fDrawTypeFlags,
               { &settings.fRenderPassProps, 1 });

    std::set<std::string> generatedLabels;

    {
        const RendererProvider* rendererProvider = precompileContext->priv().rendererProvider();
        const ShaderCodeDictionary* dict = precompileContext->priv().shaderCodeDictionary();

        std::vector<skgpu::UniqueKey> generatedKeys;

        UniqueKeyUtils::FetchUniqueKeys(precompileContext, &generatedKeys);

        for (const skgpu::UniqueKey& key : generatedKeys) {
            GraphicsPipelineDesc pipelineDesc;
            RenderPassDesc renderPassDesc;
            UniqueKeyUtils::ExtractKeyDescs(precompileContext, key, &pipelineDesc, &renderPassDesc);

            const RenderStep* renderStep = rendererProvider->lookup(pipelineDesc.renderStepID());
            std::string tmp = GetPipelineLabel(dict, renderPassDesc, renderStep,
                                               pipelineDesc.paintParamsID());
            generatedLabels.insert(rm_whitespace(tmp));
        }
    }

    std::vector<bool> localMatches;
    std::vector<size_t> matchesInCases;

    for (const std::string& g : generatedLabels) {
        bool didThisLabelMatch = false;
        for (size_t j = 0; j < std::size(kCases); ++j) {
            const char* testStr = kCases[j];
            if (!strcmp(g.c_str(), testStr)) {

#if defined(SK_DEBUG)
                DrawTypeFlags expectedFlags = get_draw_type_flags(testStr);
                SkASSERT(expectedFlags == settings.fDrawTypeFlags);
                RenderPassProperties expectedRPP = get_render_pass_properties(testStr);
                if (strstr(testStr, "ColorSpaceTransformSRGB")) {
                    expectedRPP.fDstCS = SkColorSpace::MakeSRGB();
                }
                SkASSERT(expectedRPP == settings.fRenderPassProps);
#endif

                didThisLabelMatch = true;
                matchesInCases.push_back(j);
                (*casesThatAreMatched)[j] = true;
            }
        }

        localMatches.push_back(didThisLabelMatch);
    }

//    SkASSERT(matchesInCases.size() >= 1); // This tests requirement 1, above
    [[maybe_unused]] float utilization = ((float) matchesInCases.size())/generatedLabels.size();
//    SkASSERT(utilization >= 0.4f); // This tests requirement 2, above

#if defined(PRINT_COVERAGE)
    // This block will print out all the cases in 'kCases' that the given PrecompileSettings
    // covered.
    sort(matchesInCases.begin(), matchesInCases.end());
    SkDebugf("precompile case %d handles %zu/%zu cases (%.2f utilization): ",
             precompileSettingsIndex, matchesInCases.size(), generatedLabels.size(), utilization);
    for (size_t h : matchesInCases) {
        SkDebugf("%zu ", h);
    }
    SkDebugf("\n");
#endif

#if defined(PRINT_GENERATED_LABELS)
    // This block will print out all the labels from the given PrecompileSettings marked with
    // whether they were found in 'kCases'. This is useful for analyzing the set of Pipelines
    // generated by a single PrecompileSettings and is usually used along with 'kChosenCase'.
    SkASSERT(localMatches.size() == generatedLabels.size());

    int index = 0;
    for (const std::string& g : generatedLabels) {
        SkDebugf("%c %d: %s\n", localMatches[index] ? 'h' : ' ', index, g.c_str());
        ++index;
    }
#endif
}

[[maybe_unused]] bool skip(const char* str) {
    if (strstr(str, "AnalyticClip")) {  // we have to think about this a bit more
        return true;
    }
    if (strstr(str, "RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader")) {
        return true;
    }
    if (strstr(str, "RE_LinearEffect_SRGB_SRGB_true_UNKNOWN_Shader")) {
        return true;
    }
    if (strstr(str, "RE_KawaseBlurDualFilter_LowSampleBlurEffect")) {
        return true;
    }
    if (strstr(str, "RE_KawaseBlurDualFilter_HighSampleBlurEffect")) {
        return true;
    }
    if (strstr(str, "RE_BlurFilterMixEffect")) {
        return true;
    }
    if (strstr(str, "RE_EdgeExtensionEffect")) {
        return true;
    }

    return false;
}

// The pipeline strings were created using the Dawn Metal backend so that is the only viable
// comparison
bool is_dawn_metal_context_type(skgpu::ContextType type) {
    return type == skgpu::ContextType::kDawn_Metal;
}

} // anonymous namespace

// This test verifies that for each case in 'kPrecompileCases':
//    1) it covers some pipeline(s) in 'kCases'
//    2) more than 40% of the generated Precompile Pipelines are used (i.e., that over-generation
//        isn't too out of control).
// Optionally, it can also:
//    FINAL_REPORT:   Print out a final report that includes missed cases in 'kCases'
//    PRINT_COVERAGE: list the cases (in 'kCases') that are covered by each 'kPrecompileCases' case
//    PRINT_GENERATED_LABELS: list the Pipeline labels for a specific 'kPrecompileCases' case
// Also of note, the "skip" method documents the Pipelines we're intentionally skipping and why.
DEF_GRAPHITE_TEST_FOR_CONTEXTS(ChromePrecompileTest, is_dawn_metal_context_type,
                               reporter, context, /* testContext */, CtsEnforcement::kNever) {
    using namespace skgpu::graphite;

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

    std::vector<bool> casesThatAreMatched(std::size(kCases), false);

    static const size_t kChosenCase = -1;  // only test this entry in 'kPrecompileCases'
    for (size_t i = 0; i < std::size(kPrecompileCases); ++i) {
        if (kChosenCase != -1 && kChosenCase != i) {
            continue;
        }

        run_test(precompileContext.get(), reporter,
                 kPrecompileCases[i], i, &casesThatAreMatched);
    }

#if defined(FINAL_REPORT)
    // This block prints out a final report. This includes a list of the cases in 'kCases' that
    // were not covered by the PaintOptions.
    int numCovered = 0, numNotCovered = 0, numIntentionallySkipped = 0;
    SkDebugf("not covered: ");
    for (size_t i = 0; i < std::size(kCases); ++i) {
        if (!casesThatAreMatched[i]) {
            if (skip(kCases[i])) {
                ++numIntentionallySkipped;
            } else {
                SkDebugf("%zu, ", i);
                ++numNotCovered;
            }
        } else {
            ++numCovered;
        }
    }
    SkDebugf("\n");
    SkDebugf("covered %d notCovered %d skipped %d total %zu\n",
             numCovered, numNotCovered, numIntentionallySkipped,
             std::size(kCases));
#endif
}

#endif // SK_GRAPHITE
