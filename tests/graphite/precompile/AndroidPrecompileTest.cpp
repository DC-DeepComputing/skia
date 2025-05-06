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
// 100% (1/1) handles: 0
/*  0 */ { ImagePremulHWOnlySrcover(),         DrawTypeFlags::kNonAAFillRect,    kRGBA16F_1_D },
// 100% (2/2) handles: 23 45
/*  1 */ { ImagePremulHWOnlySrcover(),         kRRectAndNonAARect,               kRGBA_1_D },
// 50% (2/4) handles: 66 75 - due to the w/o msaa load variants not being used
/*  2 */ { ImagePremulHWOnlySrcover(),         kRRectAndNonAARect,               kRGBA_4_DS },


//-----------------
/*  3 */ { SolidSrcover(),                     kRRectAndNonAARect,               kRGBA_1_D },
//-----------------
/*  4 */ { SolidSrcover(),                     DrawTypeFlags::kNonAAFillRect,    kRGBA_4_DS },
//-----------------

//-----------------
/*  5 */ { ImagePremulClampNoCubicDstin(),     kQuadAndNonAARect,                kRGBA_1_D },
/*  6 */ { ImagePremulHWOnlyMatrixCFSrcover(), DrawTypeFlags::kNonAAFillRect,    kRGBA_1_D },
/*  7 */ { ImagePremulHWOnlyPorterDuffCFSrcover(), DrawTypeFlags::kPerEdgeAAQuad,kRGBA_1_D },
/*  8 */ { LinearGradSmSrcover(),              DrawTypeFlags::kNonAAFillRect,    kRGBA_1_D },
/*  9 */ { TransparentPaintImagePremulHWAndClampSrcover(),kQuadAndNonAARect,     kRGBA_1_D },
/* 10 */ { ImageHWOnlySRGBSrcover(),           kRRectAndNonAARect,               kRGBA_1_D_SRGB },
/* 11 */ { ImageSRGBNoCubicSrc(),              kQuadAndNonAARect,                kRGBA_1_D_SRGB },
/* 12 */ { YUVImageSRGBNoCubicSrcover(),       DrawTypeFlags::kSimpleShape,      kRGBA_1_D_SRGB },

//-----------------
/* 13 */ { ImagePremulHWOnlyDstin(),           DrawTypeFlags::kPerEdgeAAQuad,    kRGBA_4_D },
/* 14 */ { ImagePremulHWOnlySrcover(),         kQuadAndNonAARect,                kRGBA_4_D },
/* 15 */ { SolidSrcSrcover(),                  kRRectAndNonAARect,               kRGBA_4_D },
/* 16 */ { BlendPorterDuffCFSrcover(),         DrawTypeFlags::kNonAAFillRect,    kRGBA_4_DS },
/* 17 */ { ImagePremulHWOnlyDstin(),           DrawTypeFlags::kPerEdgeAAQuad,    kRGBA_4_DS },
/* 18 */ { ImagePremulHWOnlyMatrixCFSrcover(), DrawTypeFlags::kNonAAFillRect,    kRGBA_4_DS },

/* 32 */ { SolidSrcover(),                     DrawTypeFlags::kNonSimpleShape,   kRGBA_4_DS },
/* 33 */ { SolidSrcover(),                     DrawTypeFlags::kAnalyticRRect,    kRGBA_4_DS },
/* 34 */ { TransparentPaintImagePremulHWOnlySrcover(), DrawTypeFlags::kPerEdgeAAQuad, kRGBA_4_DS },
/* 36 */ { ImageHWOnlySRGBSrcover(),           DrawTypeFlags::kAnalyticRRect,    kRGBA_4_DS_SRGB },
/* 37 */ { YUVImageSRGBSrcover2(),             DrawTypeFlags::kSimpleShape,      kRGBA_4_DS_SRGB },
};

// Case 36 is the only case that solely covers Pipeline labels with the "w/ msaa load" sub-string.
#define MSAA_ONLY_CASE -1

//
// These Pipelines are candidates for inclusion in Chrome's precompile. They were generated
// by collecting all the Pipelines from 9 of the top 14 visited sites according to Wikipedia
//
static const PipelineLabel kCases[] = {
/*   0 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*   ? */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] SrcOver" },
/*   X */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*   X */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*   X */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ SolidColor ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*   X */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_BlurEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] Src" },
/*   X */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_Chunk8x8Effect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] Src" },
/*   X */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_CrossTalkAndChunk16x16Effect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Src" },
/*   X */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_CrossTalkAndChunk16x16Effect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Src" },
/*   ? */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  10-*/ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose [ BlendCompose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*  20-*/ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose [ SolidColor MatrixColorFilter ] SrcOver" },
/*  23 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + SolidColor SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  30-*/ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransform ] RGBPaintColor DstIn ] ] MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] RGBPaintColor DstIn ] ] MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*  40-*/ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ SolidColor MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ SolidColor MatrixColorFilter ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Src" },
/*  45 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  50-*/ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_BlurFilterMixEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] Src" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_GainmapEffect [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ColorSpaceTransformPremul ColorSpaceTransformPremul ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransform ] ] ColorSpaceTransformPremul ColorSpaceTransformPremul ] Src" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_KawaseBlurDualFilter_HighSampleBlurEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] Src" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_KawaseBlurDualFilter_LowSampleBlurEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] Src" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_KawaseBlurDualFilter_LowSampleBlurEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ SolidColor ColorSpaceTransform ColorSpaceTransform ] Src" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Src" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor Src" },
/*  60-*/ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Src" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + VerticesRenderStep[TrisColor] + Compose [ BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformPremul ] Modulate ] Compose [ GaussianColorFilter BlendCompose [ SolidColor Passthrough Modulate ] ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + AnalyticRRectRenderStep + BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + AnalyticRRectRenderStep + BlendCompose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  66 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + AnalyticRRectRenderStep + RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + CoverBoundsRenderStep[InverseCover] + (empty)" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  70-*/ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + CoverBoundsRenderStep[NonAAFill] + Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransform ] RGBPaintColor DstIn ] ] MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Src" },
/*  75 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + MiddleOutFanRenderStep[EvenOdd] + (empty)" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/*  80-*/ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + AnalyticRRectRenderStep + Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[InverseCover] + (empty)" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ SolidColor MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + MiddleOutFanRenderStep[EvenOdd] + (empty)" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + VerticesRenderStep[TrisColor] + Compose [ BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformPremul ] Modulate ] Compose [ GaussianColorFilter BlendCompose [ SolidColor Passthrough Modulate ] ] ] SrcOver" },
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
