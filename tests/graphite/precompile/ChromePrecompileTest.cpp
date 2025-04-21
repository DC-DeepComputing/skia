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

// LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] SrcOver" },
PaintOptions fancy() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ skgpu::graphite::PrecompileShaders::LinearGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

// Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
PaintOptions fancy2() {
    PaintOptions paintOptions;
#if 0
    paintOptions.setDither(true);
    paintOptions.setShaders({ skgpu::graphite::PrecompileShaders::LinearGradient(
            { SkGradientShader::Interpolation::InPremul::kNo,
              SkGradientShader::Interpolation::ColorSpace::kSRGB,
              SkGradientShader::Interpolation::HueMethod::kShorter }) });
#endif

    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}
// "SolidColor SrcOver"
// "SolidColor Src"
PaintOptions solid_clear_src_srcover() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kClear,
                                 SkBlendMode::kSrc,
                                 SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions solid_src_srcover() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kSrc, SkBlendMode::kSrcOver });
    return paintOptions;
}

// "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver"
// "LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver"
PaintOptions image_premul_srcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    PaintOptions paintOptions;
    paintOptions.setShaders({ skgpu::graphite::PrecompileShaders::Image({ &ci, 1 }) });
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
    paintOptions.setShaders({ skgpu::graphite::PrecompileShaders::Image({ &ci, 1 }) });
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
    paintOptions.setShaders({ skgpu::graphite::PrecompileShaders::Image({ &ci, 1 }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

// "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver"
[[maybe_unused]] PaintOptions blend_porter_duff_cf_srcover() {
    PaintOptions paintOptions;
    // kSrcOver will trigger the PorterDuffBlender
    paintOptions.setColorFilters(
            { skgpu::graphite::PrecompileColorFilters::Blend({ SkBlendMode::kSrcOver }) });
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

// "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba)"
// "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba)"
// MSAA BGRA w/ just depth
const RenderPassProperties kBGRA_4_D { DepthStencilFlags::kDepth,
                                       kBGRA_8888_SkColorType,
                                       /* fDstCS= */ nullptr,
                                       /* fRequiresMSAA= */ true };

// "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba)"
// "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba)",
// MSAA BGRA w/ depth and stencil
const RenderPassProperties kBGRA_4_DS { DepthStencilFlags::kDepthStencil,
                                        kBGRA_8888_SkColorType,
                                        /* fDstCS= */ nullptr,
                                        /* fRequiresMSAA= */ true };

// The same as kBGRA_1_D but w/ an SRGB colorSpace
const RenderPassProperties kBGRA_1_D_SRGB { DepthStencilFlags::kDepth,
                                            kBGRA_8888_SkColorType,
                                            SkColorSpace::MakeSRGB(),
                                            /* fRequiresMSAA= */ false };

// The same as kBGRA_4_DS but w/ an SRGB colorSpace
const RenderPassProperties kBGRA_4_DS_SRGB { DepthStencilFlags::kDepthStencil,
                                             kBGRA_8888_SkColorType,
                                             SkColorSpace::MakeSRGB(),
                                             /* fRequiresMSAA= */ true };

// These settings cover 176 of the 202 cases in 'kCases'.
const struct PrecompileSettings {
    PaintOptions fPaintOptions;
    DrawTypeFlags fDrawTypeFlags = DrawTypeFlags::kNone;
    RenderPassProperties fRenderPassProps;
} kPrecompileCases[] = {
        // 2/3 - over-generates the PerEdgeAAQuadRenderStep case
/*  0 */ { solid_srcover(),                DrawTypeFlags::kSimpleShape,     kR_1_D },

// Only hits 2/22:
//  h 12: RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver
//  h 20: RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + TessellateWedgesRenderStep[EvenOdd] + (empty)
///*  X */ { solid_srcover(),                DrawTypeFlags::kNonSimpleShape,  kR_4_DS },

// 100%
/*  1 */ { solid_srcover(),                DrawTypeFlags::kBitmapText_Mask, kBGRA_1_D },

// Only hits 1/11:
// h 2: RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverageMaskRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver
///*  X */ { blend_porter_duff_cf_srcover(), DrawTypeFlags::kNonSimpleShape,  kBGRA_1_D },

// 11/24 -- would be better (11/18) if we could remove Cubic sampling - removing AnalyticRRectRenderStep would also help
/*  2 */ { image_premul_src_srcover(),     DrawTypeFlags::kSimpleShape,     kBGRA_1_D },

// 4/6
/*  3 */ { solid_src_srcover(),            DrawTypeFlags::kSimpleShape,     kBGRA_1_D },

// 100% (2/2
/*  4 */ { solid_srcover(),                DrawTypeFlags::kBitmapText_Mask, kBGRA_4_D },

// 4/22
// h 18: RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + TessellateStrokesRenderStep + SolidColor SrcOver
// h 19: RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + TessellateWedgesRenderStep[Convex] + SolidColor SrcOver
// h 7: RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + TessellateStrokesRenderStep + SolidColor SrcOver
// h 8: RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + TessellateWedgesRenderStep[Convex] + SolidColor SrcOver
///*  X */ { solid_srcover(),                DrawTypeFlags::kNonSimpleShape,  kBGRA_4_D },

// 100% (2/2)
/*  5 */ { solid_srcover(),                DrawTypeFlags::kBitmapText_Mask, kBGRA_4_DS },

// doesn't cover anything
///*  X */ { solid_srcover(),                DrawTypeFlags::kCircularArc,     kBGRA_4_DS },

// 15/22
/*  6 */ { solid_srcover(),                DrawTypeFlags::kNonSimpleShape,  kBGRA_4_DS },

// 6/24 - overgenerates on AnalyticRRectRenderStep (8) and Cubic (6)
/*  7 */ { image_premul_srcover(),         DrawTypeFlags::kSimpleShape,     kBGRA_4_DS },

// 8/18 - overgenerates on PerEdgeAAQuadRenderStep (6)
/*  8 */ { solid_clear_src_srcover(),      DrawTypeFlags::kSimpleShape,     kBGRA_4_DS },

// 2/12 - overgenerate on AnalyticRRectRenderStep and anything but HardwareImage
/*  9 */ { image_srgb_src(),               DrawTypeFlags::kSimpleShape,     kBGRA_1_D_SRGB },

// 6/9
/* 10 */ { yuv_image_srgb_srcover(),       DrawTypeFlags::kSimpleShape,     kBGRA_1_D_SRGB },

// 2/6 overgenerates on LinearGradient8 (2) and LinearGradientBuffer (2)
/* 11 */ { fancy(),                        DrawTypeFlags::kBitmapText_Mask, kBGRA_4_DS },

// 10/18 overgenerates on HWYUVImage case (6)
/* 12 */ { yuv_image_srgb_srcover(),      DrawTypeFlags::kSimpleShape,     kBGRA_4_DS_SRGB },

// 4/6 overgenerates on PerEdgeAAQuadRenderStep (2)
/* 13 */ { solid_srcover(),               DrawTypeFlags::kSimpleShape,     kBGRA_4_D },

///* 12 */ { fancy2(),                       DrawTypeFlags::kSimpleShape,     kBGRA_1_D_SRGB },

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

        // This RPP (kBGRA_4_D) can generate two strings when Caps::loadOpAffectsMSAAPipelines.
        { "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba)",
          kBGRA_4_D },
        { "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba)",
          kBGRA_4_D },

        // This RPP (kBGRA_4_DS) can generate two strings when Caps::loadOpAffectsMSAAPipelines.
        { "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba)",
           kBGRA_4_DS },
        { "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba)",
           kBGRA_4_DS },
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


struct ChromePipeline {
    int fNumHits;         // the number of uses in the top 9 most visited web sites
    const char* fString;
};

//
// These Pipelines are candidates for inclusion in Chrome's precompile. They were generated
// by collecting all the Pipelines from 9 of the top 14 visited sites according to Wikipedia
//
static const ChromePipeline kCases[] = {
//--------
/*   0 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateWedgesRenderStep[Winding] + (empty)" },
/*   1 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateWedgesRenderStep[EvenOdd] + (empty)" },
/*   2 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateWedgesRenderStep[Convex] + SolidColor SrcOver" },
/*   3 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*   4 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver" },
/*   X */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver AnalyticClip" },
/*   6 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/*   7 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor Src" },
/*   8 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor Clear" },
/*   ? */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "   //------------------------
               "CoverBoundsRenderStep[InverseCover] + (empty)" },
/*  10 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "BitmapTextRenderStep[Mask] + SolidColor SrcOver" },
/*  11 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "AnalyticRRectRenderStep + SolidColor SrcOver" },
//--------
/*   ? */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "       //------------------------
               "TessellateWedgesRenderStep[Convex] + SolidColor SrcOver" },
/*   ? */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "      //------------------------
               "TessellateStrokesRenderStep + SolidColor SrcOver" },
/*   X */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
//--------
/*  15 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateWedgesRenderStep[Winding] + (empty)" },
/*  16 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  17 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver" },
/*   X */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver AnalyticClip" },
/*  19 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/*  20 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor Src" },
/*  21 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor Clear" },
/*  22 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*   ? */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "     //------------------------
               "CoverBoundsRenderStep[InverseCover] + (empty)" },
/*  24 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "BitmapTextRenderStep[Mask] + SolidColor SrcOver" },
/*  25 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "AnalyticRRectRenderStep + SolidColor SrcOver" },
//--------
/*  26 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Src" },
/*  27 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  28 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/*  29 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor Src" },
/*   X */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  31 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "BitmapTextRenderStep[Mask] + SolidColor SrcOver" },
/*  32 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "AnalyticRRectRenderStep + SolidColor SrcOver" },
/*   X */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "AnalyticBlurRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*   X */ { 7, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*   X */ { 7, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "AnalyticBlurRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*  36 */ { 7, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateWedgesRenderStep[EvenOdd] + (empty)" },
/*  37 */ { 7, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateWedgesRenderStep[Convex] + SolidColor SrcOver" },
/*   X */ { 7, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*   X */ { 7, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver AnalyticClip" },
/*  40 */ { 7, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Src" },
/*   X */ { 6, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*  42 */ { 6, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "BitmapTextRenderStep[Mask] + LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] SrcOver" },
/*  43 */ { 6, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  44 */ { 6, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] Src" },
/*   X */ { 6, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*   X */ { 6, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  47 */ { 6, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src" },
/*  48 */ { 6, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src" },
/*   ? */ { 6, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "                                  //---------------------------
               "CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*   X */ { 6, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "AnalyticBlurRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*   ? */ { 5, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "                 //-----------------
               "PerEdgeAAQuadRenderStep + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  52 */ { 5, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "BitmapTextRenderStep[Mask] + LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] SrcOver" },
/*   X */ { 5, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "AnalyticRRectRenderStep + SolidColor SrcOver AnalyticClip" },
/*  54 */ { 5, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/*  55 */ { 5, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*  ? */ { 5, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "                       //------------------------
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] DstIn" },
/*  57 */ { 5, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/*  58 */ { 5, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*   ? */ { 4, "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000) + "                              //------------------------
               "PerEdgeAAQuadRenderStep + LocalMatrix [ BlendCompose [ Compose [ HardwareImage(0) ColorSpaceTransform ] RGBPaintColor DstIn ] ] SrcOver" },
/*  60 */ { 4, "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/*   X */ { 4, "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000) + "
               "CoverBoundsRenderStep[NonAAFill] + KnownRuntimeEffect_1DBlur16 [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransform ] ] ] Src" },
/*  62 */ { 4, "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000) + "
               "AnalyticRRectRenderStep + SolidColor SrcOver" },
/*  63 */ { 4, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "                     //------------------------
               "TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/*   X */ { 4, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*  65 */ { 4, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*  66 */ { 4, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "MiddleOutFanRenderStep[EvenOdd] + (empty)" },
/*  67 */ { 4, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*  68 */ { 4, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  69 */ { 4, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "                       //------------------------
               "CoverBoundsRenderStep[InverseCover] + SolidColor SrcOver" },
/*   X */ { 4, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "AnalyticRRectRenderStep + SolidColor SrcOver AnalyticClip" },
/*  71 */ { 4, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "BitmapTextRenderStep[Mask] + SolidColor SrcOver" },
/*  72 */ { 4, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "AnalyticRRectRenderStep + SolidColor SrcOver" },
/*  73 */ { 4, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/*   X */ { 4, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*  75 */ { 4, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/*  76 */ { 4, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  77 */ { 4, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*   ? */ { 4, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "     //------------------------
               "PerEdgeAAQuadRenderStep + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  79 */ { 4, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "MiddleOutFanRenderStep[EvenOdd] + (empty)" },
/*  80 */ { 4, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/*  81 */ { 4, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*   X */ { 4, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "AnalyticBlurRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*   ? */ { 4, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "        //------------------------
               "TessellateWedgesRenderStep[Convex] + SolidColor SrcOver" },
/*  84 */ { 4, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "BitmapTextRenderStep[Mask] + SolidColor SrcOver" },
/*  85 */ { 4, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + SolidColor SrcOver" },
/*   X */ { 4, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*   ? */ { 4, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "                             //------------------------
               "PerEdgeAAQuadRenderStep + Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] KnownRuntimeEffect_Luma ] SrcOver" },
/*  88 */ { 4, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] Src" },
/*   X */ { 4, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + KnownRuntimeEffect_1DBlur16 [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] ] Src" },
/*   X */ { 4, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "AnalyticRRectRenderStep + SolidColor SrcOver AnalyticClip" },
/*   ? */ { 3, "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000) + "                //------------------------
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ BlendCompose [ Compose [ Image(0) ColorSpaceTransform ] RGBPaintColor DstIn ] ] Src" },
/*   X */ { 3, "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000) + "
               "CoverBoundsRenderStep[NonAAFill] + KnownRuntimeEffect_1DBlur16 [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] ] Src" },
/*   ? */ { 3, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "   //------------------------
               "TessellateWedgesRenderStep[Convex] + (empty)" },
/*  94 */ { 3, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateStrokesRenderStep + SolidColor SrcOver" },
/*   X */ { 3, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*  96 */ { 3, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/*   X */ { 3, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*   ? */ { 3, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "   //------------------------
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] DstIn" },
/*   X */ { 3, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver AnalyticClip" },
/*   ? */ { 3, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "   //------------------------
               "CoverBoundsRenderStep[RegularCover] + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/* 101 */ { 3, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/*   ? */ { 3, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "      //------------------------
               "CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*   X */ { 3, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "BitmapTextRenderStep[Mask] + SolidColor SrcOver AnalyticClip" },
/* 104 */ { 3, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "       //------------------------
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/*   ? */ { 3, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "      //------------------------
               "CoverBoundsRenderStep[NonAAFill] + SolidColor Src" },
/*   ? */ { 3, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "  //------------------------
               "TessellateWedgesRenderStep[Convex] + (empty)" },
/*   X */ { 3, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*   ? */ { 3, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "   //------------------------
               "CoverBoundsRenderStep[RegularCover] + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*   ? */ { 3, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "   //------------------------
               "CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*   X */ { 3, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverageMaskRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*   ? */ { 3, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "      //------------------------
               "BitmapTextRenderStep[Color] + BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformSRGB ] PorterDuffBlender ] SrcOver" },
/*   ? */ { 3, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "   //------------------------
               "AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/* 113 */ { 3, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/* 114 */ { 3, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "AnalyticRRectRenderStep + SolidColor SrcOver" },
/*   X */ { 3, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*     */ { 3, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*     */ { 3, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { 3, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { 3, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] Src" },
/*     */ { 3, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { 3, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "CoverageMaskRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*     */ { 3, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "BitmapTextRenderStep[Color] + BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformSRGB ] PorterDuffBlender ] SrcOver" },
/*     */ { 3, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { 2, "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ BlendCompose [ Compose [ HardwareImage(0) ColorSpaceTransform ] RGBPaintColor DstIn ] ] Src" },
/*   X */ { 2, "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000) + "
               "CoverBoundsRenderStep[NonAAFill] + KnownRuntimeEffect_1DBlur4 [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransform ] ] ] Src" },
/*   X */ { 2, "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000) + "
               "CoverBoundsRenderStep[NonAAFill] + KnownRuntimeEffect_1DBlur4 [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] ] Src" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateWedgesRenderStep[Convex] + SolidColor SrcOver AnalyticClip" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[RegularCover] + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverageMaskRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "BitmapTextRenderStep[Mask] + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "BitmapTextRenderStep[Color] + BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformSRGB ] PorterDuffBlender ] SrcOver" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "TessellateWedgesRenderStep[Convex] + (empty)" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] DstIn" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateStrokesRenderStep + SolidColor SrcOver" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver AnalyticClip" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[RegularCover] + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ LinearGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "BitmapTextRenderStep[Mask] + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver AnalyticClip" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor Src" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + BlendCompose [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] SrcOver" },
/*   X */ { 2, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + KnownRuntimeEffect_1DBlur16 [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ] Src" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*     */ { 2, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
               "AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + TessellateWedgesRenderStep[EvenOdd] + (empty)" },
/*     */ { 1, "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + TessellateStrokesRenderStep + SolidColor SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Multiply" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + BlendCompose [ LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + BlendCompose [ LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] Multiply" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[RegularCover] + Compose [ LocalMatrix [ Compose [ RadialGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[RegularCover] + Compose [ LocalMatrix [ Compose [ RadialGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[RegularCover] + Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[RegularCover] + Compose [ BlendCompose [ LocalMatrix [ Compose [ RadialGradient4 ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[RegularCover] + Compose [ BlendCompose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ LinearGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + BitmapTextRenderStep[Color] + BlendCompose [ BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformSRGB ] PorterDuffBlender ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ RadialGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ RadialGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ RadialGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + SolidColor SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + AnalyticBlurRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + TessellateWedgesRenderStep[Convex] + SolidColor SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + TessellateStrokesRenderStep + SolidColor SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + BlendCompose [ LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[RegularCover] + Compose [ LocalMatrix [ Compose [ RadialGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[RegularCover] + Compose [ LocalMatrix [ Compose [ RadialGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[RegularCover] + Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[RegularCover] + Compose [ BlendCompose [ LocalMatrix [ Compose [ RadialGradient4 ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[RegularCover] + Compose [ BlendCompose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ LinearGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverageMaskRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + BitmapTextRenderStep[Color] + BlendCompose [ BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformSRGB ] PorterDuffBlender ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ RadialGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ RadialGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ RadialGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + TessellateWedgesRenderStep[Convex] + (empty)" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + TessellateStrokesRenderStep + SolidColor SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ LinearGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ LinearGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + BitmapTextRenderStep[Color] + BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformSRGB ] PorterDuffBlender ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + SolidColor SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] DstIn" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] KnownRuntimeEffect_Luma ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor Src AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] DstIn" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + KnownRuntimeEffect_1DBlur8 [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] ] Src" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + KnownRuntimeEffect_1DBlur16 [ LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] ] Src" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + KnownRuntimeEffect_1DBlur12 [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] ] Src" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + KnownRuntimeEffect_1DBlur12 [ LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] ] Src" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ LinearGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ LinearGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformPremul ] ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] Compose [ MatrixColorFilter MatrixColorFilter ] ] Src" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ BlendCompose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] PorterDuffBlender ] Src" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] PorterDuffBlender ] Src" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverageMaskRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CircularArcRenderStep + SolidColor SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + BitmapTextRenderStep[Mask] + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + BitmapTextRenderStep[Mask] + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + BitmapTextRenderStep[Color] + BlendCompose [ BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformPremul ] PorterDuffBlender ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] SrcOver" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/*     */ { 1, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
    };

[[maybe_unused]] void find_duplicates(SkSpan<const ChromePipeline> cases) {
    for (size_t i = 0; i < std::size(cases); ++i) {
        for (size_t j = i+1; j < std::size(cases); ++j) {
            if (!strcmp(cases[i].fString, cases[j].fString)) {
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
            const char* testStr = kCases[j].fString;
            if (!strcmp(g.c_str(), testStr)) {

#if defined(SK_DEBUG)
                DrawTypeFlags expectedFlags = get_draw_type_flags(testStr);
                SkASSERT(expectedFlags == settings.fDrawTypeFlags);
                RenderPassProperties expectedRPP = get_render_pass_properties(testStr);
                if (strstr(testStr, "ColorSpaceTransformSRGB")) {
                    expectedRPP.fDstCS = SkColorSpace::MakeSRGB();
                }
//                SkASSERT(expectedRPP == settings.fRenderPassProps);
#endif

                didThisLabelMatch = true;
                matchesInCases.push_back(j);
                (*casesThatAreMatched)[j] = true;
            }
        }

        localMatches.push_back(didThisLabelMatch);
    }

//    REPORTER_ASSERT(reporter, matchesInCases.size() >= 1,   // This tests requirement 1, above
//                    "%d: num matches: %zu", precompileSettingsIndex, matchesInCases.size());
    float utilization = ((float) matchesInCases.size())/generatedLabels.size();
//    REPORTER_ASSERT(reporter, utilization >= 0.4f,         // This tests requirement 2, above
//                    "%d: utilization: %f", precompileSettingsIndex, utilization);

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
    if (strstr(str, "AnalyticBlurRenderStep")) { // currently internal only
        return true;
    }
    if (strstr(str, "KnownRuntimeEffect_1DBlur16")) {  // we have to revise how we do blurring
        return true;
    }
//    if (strstr(str, "LinearGradient4")) {  // this seems too specialized
//        return true;
//    }
    if (strstr(str, "KnownRuntimeEffect_Luma")) {  // this also seems too specialized
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
    std::vector<size_t> skipped;
    // This block prints out a final report. This includes a list of the cases in 'kCases' that
    // were not covered by the PaintOptions.
    int numCovered = 0, numNotCovered = 0, numIntentionallySkipped = 0;
    SkDebugf("not covered: ");
    for (size_t i = 0; i < std::size(kCases); ++i) {
        if (!casesThatAreMatched[i]) {
            if (skip(kCases[i].fString)) {
                skipped.push_back(i);
                ++numIntentionallySkipped;
            } else {
                SkDebugf("%zu, ", i);
                ++numNotCovered;
            }
        } else {
            ++numCovered;
        }
    }
    SkDebugf("\n skipped: ");
    for (auto f : skipped) {
        SkDebugf("%zu, ", f);
    }
    SkDebugf("\n");
    SkDebugf("covered %d notCovered %d skipped %d total %zu\n",
             numCovered, numNotCovered, numIntentionallySkipped,
             std::size(kCases));
#endif
}

#endif // SK_GRAPHITE
