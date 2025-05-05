/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/PrecompileContext.h"
#include "src/gpu/graphite/PrecompileContextPriv.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "tests/graphite/precompile/PrecompileTestUtils.h"

using namespace skgpu::graphite;
using namespace PrecompileTestUtils;

namespace {

// These settings cover 113 of the 255 cases in 'kCases'.
// They create 141 Pipelines so only modestly over-generate (28 Pipelines).
//
// The order here is:
//    First all the kBitmapText_Mask draws
//    Second  all the kBitmapText_Color draws
// Then we switch to being subdivided by the Render Pass Properties and sorted by
// the name of the PaintOptions creation function.
const PrecompileSettings kPrecompileCases[] = {
//-----------------
/*  0 */ { BlendPorterDuffCFSrcover(),         DrawTypeFlags::kBitmapText_Mask,  kBGRA_1_D },
/*  1 */ { SolidSrcover(),                     DrawTypeFlags::kBitmapText_Mask,  kBGRA_1_D },
//-----------------
/*  2 */ { SolidSrcover(),                     DrawTypeFlags::kBitmapText_Mask,  kBGRA_4_D },
//-----------------
/*  3 */ { SolidSrcover(),                     DrawTypeFlags::kBitmapText_Mask,  kBGRA_4_DS },
/*  4 */ { LinearGradSmSrcover(),              DrawTypeFlags::kBitmapText_Mask,  kBGRA_4_DS },
/*  5 */ { BlendPorterDuffCFSrcover(),         DrawTypeFlags::kBitmapText_Mask,  kBGRA_4_DS },

//-----------------
/*  6 */ { TransparentPaintSrcover(),          DrawTypeFlags::kBitmapText_Color, kBGRA_1_D },
/*  7 */ { SolidSrcover(),                     DrawTypeFlags::kBitmapText_Color, kBGRA_1_D_Adobe },
//-----------------
/*  8 */ { SolidSrcover(),                     DrawTypeFlags::kBitmapText_Color, kBGRA_4_DS_Adobe },

//-----------------
/*  9 */ { SolidSrcover(),                     kRRectAndNonAARect,               kR_1_D },
/* 10 */ { ImageAlphaHWOnlySrcover(),          DrawTypeFlags::kPerEdgeAAQuad,    kR_1_D },
/* 11 */ { ImageAlphaNoCubicSrc(),             DrawTypeFlags::kNonAAFillRect,    kR_1_D },

//-----------------
/* 12 */ { ImagePremulClampNoCubicDstin(),     kQuadAndNonAARect,                kBGRA_1_D },
/* 13 */ { ImagePremulHWOnlyMatrixCFSrcover(), DrawTypeFlags::kNonAAFillRect,    kBGRA_1_D },
/* 14 */ { ImagePremulHWOnlyPorterDuffCFSrcover(), DrawTypeFlags::kPerEdgeAAQuad,kBGRA_1_D },
/* 15 */ { ImagePremulNoCubicSrcover(),        DrawTypeFlags::kAnalyticRRect,    kBGRA_1_D },
/* 16 */ { ImagePremulNoCubicSrcSrcover(),     kQuadAndNonAARect,                kBGRA_1_D },
/* 17 */ { LinearGradSmSrcover(),              DrawTypeFlags::kNonAAFillRect,    kBGRA_1_D },
/* 18 */ { SolidSrcSrcover(),                  DrawTypeFlags::kSimpleShape,      kBGRA_1_D },
/* 19 */ { TransparentPaintImagePremulHWAndClampSrcover(),kQuadAndNonAARect,     kBGRA_1_D },
/* 20 */ { LinearGradSRGBSmMedSrcover(),       kRRectAndNonAARect,               kBGRA_1_D_Adobe },
/* 21 */ { ImageHWOnlySRGBSrcover(),           kRRectAndNonAARect,               kBGRA_1_D_SRGB },
/* 22 */ { ImageSRGBNoCubicSrc(),              kQuadAndNonAARect,                kBGRA_1_D_SRGB },
/* 23 */ { YUVImageSRGBNoCubicSrcover(),       DrawTypeFlags::kSimpleShape,      kBGRA_1_D_SRGB },

//-----------------
/* 24 */ { ImagePremulHWOnlyDstin(),           DrawTypeFlags::kPerEdgeAAQuad,    kBGRA_4_D },
/* 25 */ { ImagePremulHWOnlySrcover(),         kQuadAndNonAARect,                kBGRA_4_D },
/* 26 */ { SolidSrcSrcover(),                  kRRectAndNonAARect,               kBGRA_4_D },
/* 27 */ { BlendPorterDuffCFSrcover(),         DrawTypeFlags::kNonAAFillRect,    kBGRA_4_DS },
/* 28 */ { ImagePremulHWOnlyDstin(),           DrawTypeFlags::kPerEdgeAAQuad,    kBGRA_4_DS },
/* 29 */ { ImagePremulHWOnlyMatrixCFSrcover(), DrawTypeFlags::kNonAAFillRect,    kBGRA_4_DS },
/* 30 */ { ImagePremulNoCubicSrcover(),        kQuadAndNonAARect,                kBGRA_4_DS },
/* 31 */ { SolidClearSrcSrcover(),             DrawTypeFlags::kNonAAFillRect,    kBGRA_4_DS },
/* 32 */ { SolidSrcover(),                     DrawTypeFlags::kNonSimpleShape,   kBGRA_4_DS },
/* 33 */ { SolidSrcover(),                     DrawTypeFlags::kAnalyticRRect,    kBGRA_4_DS },
/* 34 */ { TransparentPaintImagePremulHWOnlySrcover(), DrawTypeFlags::kPerEdgeAAQuad, kBGRA_4_DS },
/* 35 */ { LinearGradSRGBSmMedSrcover(),       kRRectAndNonAARect,               kBGRA_4_DS_Adobe },
/* 36 */ { ImageHWOnlySRGBSrcover(),           DrawTypeFlags::kAnalyticRRect,    kBGRA_4_DS_SRGB },
/* 37 */ { YUVImageSRGBSrcover2(),             DrawTypeFlags::kSimpleShape,      kBGRA_4_DS_SRGB },
};

// Case 36 is the only case that solely covers Pipeline labels with the "w/ msaa load" sub-string.
#define MSAA_ONLY_CASE 36

//
// These Pipelines are candidates for inclusion in Chrome's precompile. They were generated
// by collecting all the Pipelines from 9 of the top 14 visited sites according to Wikipedia
//
static const PipelineLabel kCases[] = {
/*   0 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor Src AnalyticClip" },
/*   1 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/*   2 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + SolidColor SrcOver" },
/*   3 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ SolidColor ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*   4 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + VerticesRenderStep[TrisColor] + Compose [ BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformPremul ] Modulate ] Compose [ GaussianColorFilter BlendCompose [ SolidColor Passthrough Modulate ] ] ] SrcOver" },
/*   5 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_KawaseBlurDualFilter_LowSampleBlurEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ] Src" },
/*   6 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_KawaseBlurDualFilter_LowSampleBlurEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ] SrcOver" },
/*   7 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src" },
/*   8 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src" },
/*   9 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_BlurFilterMixEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ] Src" },
/*  10 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_KawaseBlurDualFilter_HighSampleBlurEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ] Src" },
/*  11 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  12 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  13 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  14 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  15 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  16 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] SrcOver" },
/*  17 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  18 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] SrcOver" },
/*  19 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*  20 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*  21 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*  22 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*  23 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  24 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  25 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + TessellateWedgesRenderStep[EvenOdd] + (empty)" },
/*  26 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[InverseCover] + (empty)" },
/*  27 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  28 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*  29 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*  30 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  31 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] SrcOver" },
/*  32 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver AnalyticClip" },
/*  33 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/*  34 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  35 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ] SrcOver" },
/*  36 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  37 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ] SrcOver" },
/*  38 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  39 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] SrcOver" },
/*  40 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  41 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] SrcOver" },
/*  42 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  43 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] SrcOver" },
/*  44 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  45 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] SrcOver" },
/*  46 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*  47 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*  48 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] MatrixColorFilter ] Dither ] SrcOver" },
/*  49 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*  50 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] SrcOver AnalyticClip" },
/*  51 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  52 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] SrcOver" },
/*  53 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*  54 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] SrcOver AnalyticClip" },
/*  55 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  56 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] SrcOver" },
/*  57 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ RE_EdgeExtensionEffect [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  58 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_EdgeExtensionEffect [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Plus ] ] SrcOver" },
/*  59 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ RE_EdgeExtensionEffect [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  60 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_EdgeExtensionEffect [ BlendCompose [ SolidColor LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Plus ] ] SrcOver" },
/*  61 */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ BlendCompose [ RE_LinearEffect_SRGB_SRGB_true_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_SRGB_SRGB_true_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ BlendCompose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ BlendCompose [ RE_LinearEffect_SRGB_SRGB_true_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ RE_LinearEffect_SRGB_SRGB_true_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ BlendCompose [ RE_LinearEffect_SRGB_SRGB_true_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_SRGB_SRGB_true_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ BlendCompose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ BlendCompose [ RE_LinearEffect_SRGB_SRGB_true_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ RE_LinearEffect_SRGB_SRGB_true_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver AnalyticClip" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4), resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ BlendCompose [ RE_LinearEffect_BT2020_ITU_PQ_BT2020_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ BlendCompose [ RE_LinearEffect_BT2020_ITU_PQ_BT2020_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_BT2020_ITU_PQ_BT2020_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ BlendCompose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ BlendCompose [ RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ BlendCompose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ Compose [ BlendCompose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ Compose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ BlendCompose [ Compose [ HardwareImage(0) ColorSpaceTransform ] RGBPaintColor DstIn ] ] MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_MouriMap_CrossTalkAndChunk16x16Effect [ LocalMatrix [ Compose [ HardwareImage(3: kEwA) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Src" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_MouriMap_Chunk8x8Effect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ] Src" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_MouriMap_BlurEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ] Src" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ SolidColor ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ HardwareImage(3: kEwA) ColorSpaceTransform ] ] LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_MouriMap_CrossTalkAndChunk16x16Effect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Src" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Src" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_GainmapEffect [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransformPremul ColorSpaceTransformPremul ] LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] ColorSpaceTransformPremul ColorSpaceTransformPremul ] Src" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_BT2020_ITU_PQ_BT2020_false_UNKNOWN_Shader [ SolidColor ColorSpaceTransform ColorSpaceTransform ] Src" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ SolidColor MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_BT2020_ITU_PQ_BT2020_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(3: kEwA) ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ SolidColor MatrixColorFilter ] SrcOver AnalyticClip" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_BT2020_ITU_PQ_BT2020_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(3: kEwA) ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver AnalyticClip" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ SolidColor MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ RE_LinearEffect_BT2020_ITU_PQ_BT2020_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(3: kEwA) ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ BlendCompose [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] RGBPaintColor DstIn ] ] MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(3: kHoA) ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor Src" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(3: kHoA) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(3: kHoA) ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ SolidColor MatrixColorFilter ] Src" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ HardwareImage(3: kHoA) ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ HardwareImage(3: kHoA) ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + MiddleOutFanRenderStep[EvenOdd] + (empty)" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[InverseCover] + (empty)" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver AnalyticClip" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + BlendCompose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=4) w/ msaa load, resolve: Dawn(f=RGBA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ BlendCompose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ Compose [ RE_LinearEffect_0x188a0000_DISPLAY_P3_false_0x90a0000_Shader [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA16F,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_UNKNOWN_SRGB_false_UNKNOWN_Shader [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(3: kHIA) ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { -1, "RP(color:r: Dawn(f=RGBA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(3: kHIA) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },

};

// The pipeline strings were created with Android Vulkan
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
DEF_GRAPHITE_TEST_FOR_CONTEXTS(AndroidPrecompileTest, is_dawn_metal_context_type,
                               reporter, context, /* testContext */, CtsEnforcement::kNever) {
    using namespace skgpu::graphite;

    std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();
    const skgpu::graphite::Caps* caps = precompileContext->priv().caps();

    TextureInfo textureInfo = caps->getDefaultSampledTextureInfo(kBGRA_8888_SkColorType,
                                                                 skgpu::Mipmapped::kNo,
                                                                 skgpu::Protected::kNo,
                                                                 skgpu::Renderable::kYes);

    const bool msaaSupported =
            caps->msaaRenderToSingleSampledSupport() ||
            caps->isSampleCountSupported(TextureInfoPriv::ViewFormat(textureInfo),
                                         caps->defaultMSAASamplesCount());

    if (!msaaSupported) {
        // The following pipelines rely on having MSAA
        return;
    }

#ifdef SK_ENABLE_VELLO_SHADERS
    if (caps->computeSupport()) {
        // The following pipelines rely on not utilizing Vello
        return;
    }
#endif

    PipelineLabelInfoCollector collector({ kCases });

    static const size_t kChosenCase = -1; // only test this entry in 'kPrecompileCases'
    for (size_t i = 0; i < std::size(kPrecompileCases); ++i) {
        if (kChosenCase != -1 && kChosenCase != i) {
            continue;
        }

        if (i == MSAA_ONLY_CASE && !caps->loadOpAffectsMSAAPipelines()) {
            // If "w/ msaa load" strings aren't being generated, cases that only handle Pipeline
            // labels with that sub-string will never be matched.
            continue;
        }

        RunTest(precompileContext.get(), reporter, { kPrecompileCases }, i,
                { kCases },
                &collector);
    }

#if defined(FINAL_REPORT)
    // This block prints out a final report. This includes a list of the cases in 'kCases' that
    // were not covered by the PaintOptions.

    collector.finalReport();
#endif
}

#endif // SK_GRAPHITE
