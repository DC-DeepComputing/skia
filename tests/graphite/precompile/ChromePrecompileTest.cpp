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
    paintOptions.setShaders({ skgpu::graphite::PrecompileShaders::Image({ &ci, 1 }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

// LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
// LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
// LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
// LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
PaintOptions yuv_image_srgb_srcover() {
        SkColorInfo ci { kRGBA_8888_SkColorType,
                         kPremul_SkAlphaType,
                         SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                               SkNamedGamut::kAdobeRGB) };
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
PaintOptions blend_porter_duff_cf_srcover() {
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
// MSAA R w/ depth and stencil
const RenderPassProperties kR_4_DS { DepthStencilFlags::kDepthStencil,
                                     kAlpha_8_SkColorType,
                                     /* fDstCS= */ nullptr,
                                     /* fRequiresMSAA= */ true };

// "RP(color: Dawn(f=R8,s=4) w/ msaa load, resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000)",
// MSAA R w/ depth and stencil (and a msaa load)
const RenderPassProperties kR_4_DS_wLoad { DepthStencilFlags::kDepthStencil,
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
// MSAA BGRA w/ just depth
const RenderPassProperties kBGRA_4_D { DepthStencilFlags::kDepth,
                                       kBGRA_8888_SkColorType,
                                       /* fDstCS= */ nullptr,
                                       /* fRequiresMSAA= */ true };

// "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba)"
// MSAA BGRA w/ just depth (and a msaa load)
const RenderPassProperties kBGRA_4_D_wLoad { DepthStencilFlags::kDepth,
                                             kBGRA_8888_SkColorType,
                                             /* fDstCS= */ nullptr,
                                             /* fRequiresMSAA= */ true };

// "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba)"
// MSAA BGRA w/ depth and stencil
const RenderPassProperties kBGRA_4_DS { DepthStencilFlags::kDepthStencil,
                                        kBGRA_8888_SkColorType,
                                        /* fDstCS= */ nullptr,
                                        /* fRequiresMSAA= */ true };

// "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba)",
// MSAA BGRA w/ depth and stencil (and a msaa load)
const RenderPassProperties kBGRA_4_DS_wLoad { DepthStencilFlags::kDepthStencil,
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

#if 0
case 0 handles: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21                     ------ 22 hits/ 22 generated -> 1.000000 efficacy
case 1 handles: 159                     ------ 1 hits/ 1 generated -> 1.000000 efficacy
case 2 handles: 117 118 119 120 121 122 138 139 140 157 158                     ------ 11 hits/ 11 generated -> 1.000000 efficacy
case 3 handles: 127 128 130 131 133 134 136 137 145 146 148 149 152 153 155 156 164 165 167 168 170 171 173 174                     ------ 24 hits/ 24 generated -> 1.000000 efficacy
case 4 handles: 123 124 125 141 142 143 160 161 162                     ------ 9 hits/ 9 generated -> 1.000000 efficacy
case 5 handles: 58 59 60 61 62 63 64 65 66 68 69 106 107 108 109 110 111 112 113 114 115 116                     ------ 22 hits/ 22 generated -> 1.000000 efficacy
case 6 handles: 50 98                     ------ 2 hits/ 2 generated -> 1.000000 efficacy
case 7 handles: 49 97                     ------ 2 hits/ 2 generated -> 1.000000 efficacy
case 8 handles: 22 23 24 25 26 27 35 36 37 46 48 70 71 72 73 74 75 83 84 85 94 96                     ------ 22 hits/ 22 generated -> 1.000000 efficacy
case 9 handles: 31 32 33 34 42 43 44 45 54 55 56 57 79 80 81 82 90 91 92 93 102 103 104 105                     ------ 24 hits/ 24 generated -> 1.000000 efficacy
case 10 handles: 28 29 30 39 40 41 51 52 53 76 77 78 87 88 89 99 100 101                     ------ 18 hits/ 18 generated -> 1.000000 efficacy
case 11 handles: 126 129 132 135 144 147 150 154 163 166 169 172                     ------ 12 hits/ 12 generated -> 1.000000 efficacy
case 12 handles: 177 178 179 180                     ------ 4 hits/ 6 generated -> 0.666667 efficacy
case 13 handles: 183 186                     ------ 2 hits/ 12 generated -> 0.166667 efficacy
case 14 handles: 198                     ------ 1 hits/ 2 generated -> 0.500000 efficacy
case 15 handles: 197                     ------ 1 hits/ 6 generated -> 0.166667 efficacy
case 16 handles: 190 192                     ------ 2 hits/ 3 generated -> 0.666667 efficacy
not covered: 47, 95, 199, 201,
covered 179 notCovered 4 skipped 19 total 202
#endif
const struct PrecompileSettings {
    PaintOptions fPaintOptions;
    DrawTypeFlags fDrawTypeFlags = DrawTypeFlags::kNone;
    RenderPassProperties fRenderPassProps;
} kPrecompileCases[] = {
    { solid_srcover(),                DrawTypeFlags::kNonSimpleShape,  kR_4_DS },        // 1.0

    { solid_srcover(),                DrawTypeFlags::kBitmapText_Mask, kBGRA_1_D },      // 1.0
    { blend_porter_duff_cf_srcover(), DrawTypeFlags::kNonSimpleShape,  kBGRA_1_D },      // 1.0
    { image_premul_src_srcover(),     DrawTypeFlags::kSimpleShape,     kBGRA_1_D },      // 1.0
    { solid_clear_src_srcover(),      DrawTypeFlags::kSimpleShape,     kBGRA_1_D },      // 1.0

    { solid_srcover(),                DrawTypeFlags::kNonSimpleShape,  kBGRA_4_D },      // 1.0

    { solid_srcover(),                DrawTypeFlags::kBitmapText_Mask, kBGRA_4_DS },     // 1.0
    { solid_srcover(),                DrawTypeFlags::kCircularArc,     kBGRA_4_DS },     // 1.0
    { solid_srcover(),                DrawTypeFlags::kNonSimpleShape,  kBGRA_4_DS },     // 1.0
    { image_premul_srcover(),         DrawTypeFlags::kSimpleShape,     kBGRA_4_DS },     // 1.0
    { solid_clear_src_srcover(),      DrawTypeFlags::kSimpleShape,     kBGRA_4_DS },     // 1.0

    { image_srgb_src(),               DrawTypeFlags::kSimpleShape,     kBGRA_1_D_SRGB }, // 1.0
    //----
    { yuv_image_srgb_srcover(),       DrawTypeFlags::kSimpleShape,     kBGRA_1_D_SRGB }, // 0.6667

//  { yuv_image_srgb_srcover(),       DrawTypeFlags::kSimpleShape,     kBGRA_4_DS_SRGB },// 0.1667

    { solid_srcover(),                DrawTypeFlags::kBitmapText_Mask, kBGRA_4_D },      // 0.5
//  { solid_srcover(),                DrawTypeFlags::kSimpleShape,     kBGRA_4_D },      // 0.1667

//  { solid_srcover(),                DrawTypeFlags::kSimpleShape,     kR_1_D },         // 0.6667
};

/*********** Here ends the part that can be pasted into Chrome's graphite_precompile.cc ***********/

// This helper maps from the RenderPass string in the Pipeline label to the
// RenderPassProperties needed by the Precompile system
// TODO(robertphillips): converting this to a more piecemeal approach might better illuminate
// the mapping between the string and the RenderPassProperties
RenderPassProperties get_render_pass_properties(const char* str) {
    static const struct {
        const char* fStr;
        RenderPassProperties fRenderPassProperties;
    } kRenderPassPropertiesMapping[] = {
        { "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000)",
          kR_1_D },
        { "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000)",
          kR_4_DS },
        { "RP(color: Dawn(f=R8,s=4) w/ msaa load, resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000)",
          kR_4_DS_wLoad },
        { "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba)",
          kBGRA_1_D },
        { "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba)",
          kBGRA_4_D },
        { "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba)",
          kBGRA_4_D_wLoad },
        { "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba)",
           kBGRA_4_DS },
        { "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba)",
           kBGRA_4_DS_wLoad },
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
DrawTypeFlags get_draw_type_flags(const char* str) {
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

struct Foo {
    int fNumHits;
    const char* fString;
};

std::string whatwhat(const std::string& foo) {
    auto start = foo.find_first_not_of(' ');
    auto end = foo.find_last_not_of(' ');
    return foo.substr(start, (end - start) + 1);
}

// Precompile with the provided paintOptions, drawType, and RenderPassSettings then verify that
// the expected string is in the generated set.
// Additionally, verify that overgeneration is within expected tolerances.
// If you add an additional RenderStep you may need to increase the tolerance values.
void run_test(skgpu::graphite::PrecompileContext* precompileContext,
              skiatest::Reporter* reporter,
              SkSpan<const Foo> cases,
              size_t caseID,
              const PrecompileSettings& settings,
              int expectedNumPipelines,
              std::vector<bool>& covered) {
    using namespace skgpu::graphite;

    const char* expectedString = cases[caseID].fString;

    precompileContext->priv().globalCache()->resetGraphicsPipelines();

    Precompile(precompileContext,
               settings.fPaintOptions,
               settings.fDrawTypeFlags,
               { &settings.fRenderPassProps, 1 });

    std::set<std::string> generatedS;

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
            generatedS.insert(whatwhat(tmp));
        }
    }

    std::vector<std::string> generatedV;
    for (auto s : generatedS) {
        generatedV.push_back(s);
    }

    bool correctGenerationAmt = (expectedNumPipelines == -1) ||
                                (int) generatedV.size() == expectedNumPipelines;
    REPORTER_ASSERT(reporter, correctGenerationAmt,
                    "case %zu generated unexpected amount - a: %zu != e: %u\n",
                    caseID, generatedV.size(), expectedNumPipelines);

    bool foundIt = false;
    for (size_t i = 0; i < generatedV.size(); ++i) {
        // The generated strings have trailing whitespace
        if (!strcmp(expectedString, generatedV[i].c_str())) {
            foundIt = true;
            break;
        }
    }

//    REPORTER_ASSERT(reporter, foundIt);
    std::vector<bool> enh(generatedV.size(), false);
    std::vector<size_t> hits;

    int numHits = 0;
    for (size_t i = 0; i < generatedV.size(); ++i) {
        for (size_t j = 0; j < cases.size(); ++j) {
            const char* testStr = cases[j].fString;
            if (!strcmp(generatedV[i].c_str(), testStr)) {
//                SkDebugf("%zu ", j);
                ++numHits;
                enh[i] = true;
                hits.push_back(j);
                covered[j] = true;
            }
        }
    }

    SkASSERT(numHits == (int) hits.size());
    sort(hits.begin(), hits.end());
    SkDebugf("case %zu handles: ", caseID);
    for (size_t h : hits) {
        SkDebugf("%zu ", h);
    }
    SkDebugf("                    ------ %d hits/ %zu generated -> %f efficacy\n",
             numHits, generatedV.size(), ((float) numHits)/generatedV.size());

#if 0
    if (foundIt && correctGenerationAmt) {
        return;
    }

    SkDebugf("Expected string:\n%s\n%s in %zu strings:\n",
             expectedString,
             foundIt ? "found" : "NOT found",
             generated.size());
#endif

#if 0
    for (size_t i = 0; i < generatedV.size(); ++i) {
        SkDebugf("%c %zu: %s\n", enh[i] ? 'h' : ' ', i, generatedV[i].c_str());
    }
#endif
}

// The pipeline strings were created using the Dawn Metal backend so that is the only viable
// comparison
bool is_dawn_metal_context_type(skgpu::ContextType type) {
    return type == skgpu::ContextType::kDawn_Metal;
}

bool skip(const char* str) {
    if (strstr(str, "AnalyticClip")) {
        return true;
    }
    if (strstr(str, "KnownRuntimeEffect_1DBlur16")) {
        return true;
    }
    if (strstr(str, "LinearGradient4")) {
        return true;
    }
    if (strstr(str, "KnownRuntimeEffect_Luma")) {
        return true;
    }

    return false;
}

} // anonymous namespace


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

    //
    // These Pipelines are candidates for inclusion in Chrome's precompile. They were generated
    // by collecting all the Pipelines from the following stories:
    //
    //    animometer_webgl_attrib_arrays, balls_javascript_canvas, canvas_05000_pixels_per_second,
    //    chip_tune, css_value_type_shadow, fill_shapes, ie_chalkboard, main_30fps_impl_60fps,
    //    new_tilings, transform_transitions_js_block, web_animations_staggered_infinite_iterations,
    //    wikipedia_2018
    //
    const char* kCasesOld[] = {
            //---- Single-sample - R8Unorm/Depth16Unorm --> kR_1_D
/* 0 */  "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "KnownRuntimeEffect_1DBlur12 [ LocalMatrix [ Compose [ Image(0) ColorSpaceTransform ] ] ] Src",

/* 1 */  "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "KnownRuntimeEffect_1DBlur8  [ LocalMatrix [ Compose [ Image(0) ColorSpaceTransform ] ] ] Src",

            //---- MSAA4 - R8Unorm/Depth24PlusStencil8 --> kR_4_DS
/* 2 */  "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
         "TessellateWedgesRenderStep[EvenOdd] + "
         "(empty)",

/* 3 */  "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
         "CoverBoundsRenderStep[RegularCover] + "
         "SolidColor SrcOver",

            //---- Single-sample - BGRA8Unorm/Depth16Unorm -> kBGRA_1_D
/* 4 */  "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "AnalyticBlurRenderStep + "
         "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver",

            // For now, we're going to pass on the AnalyticClip Pipelines
/* 5 */  "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "AnalyticBlurRenderStep + "
         "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip",

/* 6 */  "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "AnalyticRRectRenderStep + "
         "SolidColor SrcOver",

/* 7 */  "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "BitmapTextRenderStep[Mask] + "
         "SolidColor SrcOver",

/* 8 */  "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor SrcOver",

/* 9 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "SolidColor Src",

/* 10 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor Clear",

/* 11 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src",

            // For now, we're going to pass on the AnalyticClip Pipelines
/* 12 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip",

/* 13 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverageMaskRenderStep + "
         "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver",

            // This is the only AlphaOnlyPaintColor case so, we're going to pass for now
/* 14 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",

/* 15 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "SolidColor SrcOver",

/* 16 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src",

/* 17 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",

/* 18 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Src",

/* 19 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver",

            //---- MSAA4 - BGRA8Unorm/Depth16Unorm -> bgra_4_D
            // For now, we're going to pass on the AnalyticClip Pipelines
/* 20 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip",

/* 21 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "TessellateStrokesRenderStep + "
         "SolidColor SrcOver",

/* 22 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "TessellateWedgesRenderStep[Convex] + "
         "SolidColor SrcOver",

            //---- MSAA4 - BGRA8Unorm/Depth24PlusStencil8 --> kBGRA_4_DS
/* 23 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "AnalyticRRectRenderStep + "
         "SolidColor SrcOver",

/* 24 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "BitmapTextRenderStep[Mask] + "
         "SolidColor SrcOver",

            // This seems like a quirk of our test set. Is linear gradient text that common? Pass for now.
/* 25 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "BitmapTextRenderStep[Mask] + "
         "LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] SrcOver",

/* 26 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CircularArcRenderStep + "
         "SolidColor SrcOver",

/* 27 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[InverseCover] + "
         "(empty)",

/* 28 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[RegularCover] + "
         "SolidColor SrcOver",

/* 29 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[RegularCover] + "
         "(empty)",

/* 30 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor SrcOver",

/* 31 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor Clear",

/* 32 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",

/* 33 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",

/* 34 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "TessellateWedgesRenderStep[Winding] + "
         "(empty)",

/* 35 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "TessellateWedgesRenderStep[EvenOdd] + "
         "(empty)",

/* 36 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "TessellateWedgesRenderStep[Convex] + "
         "SolidColor SrcOver",

//----------------------------
// These are leftover Pipelines that were generated by the stories but are pretty infrequently used.
// Some of them get picked up in the preceding cases
            //---- Single-sample - BGRA8Unorm/Depth16Unorm -> kBGRA_1_D
/* 37 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] Dither ] SrcOver",

/* 38 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",

/* 39 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver",

/* 40 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] SrcOver",

/* 41 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Src",

/* 42 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver",

/* 43 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver",

            //---- MSAA4 - BGRA8Unorm/Depth16Unorm -> kBGRA_4_D
/* 44 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "AnalyticRRectRenderStep + "
         "SolidColor SrcOver",

/* 45 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "BitmapTextRenderStep[Mask] + "
         "SolidColor SrcOver",

/* 46 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor SrcOver",

/* 47 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor Src",

/* 48 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] Dither ] SrcOver",

/* 49 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver",

/* 50 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",

/* 51 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver",

            //---- MSAA4 - BGRA8Unorm/Depth24PlusStencil8 --> kBGRA_4_DS
/* 52 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "AnalyticRRectRenderStep + "
         "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] Dither ] SrcOver",

/* 53 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor Src",

/* 54 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] Dither ] SrcOver",

            // For now, we're going to pass on the AnalyticClip Pipelines
/* 55 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor SrcOver AnalyticClip",

/* 56 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[RegularCover] + "
         "Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformPremul ] ] Dither ] SrcOver",

/* 57 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "MiddleOutFanRenderStep[Winding] + "
         "(empty)",

/* 58 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "MiddleOutFanRenderStep[EvenOdd] + "
         "(empty)",

/* 59 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",

/* 60 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver",

/* 61 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "TessellateCurvesRenderStep[Winding] + "
         "(empty)",

/* 62 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "TessellateCurvesRenderStep[EvenOdd] + "
         "(empty)",

/* 63 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "TessellateWedgesRenderStep[Convex] + "
         "Compose [ LocalMatrix [ Compose [ LinearGradientBuffer ColorSpaceTransformPremul ] ] Dither ] SrcOver",

/* 64 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "TessellateWedgesRenderStep[Convex] + "
         "Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformPremul ] ] Dither ] SrcOver",

/* 65 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "TessellateStrokesRenderStep + "
         "SolidColor SrcOver",
    };


    const Foo kCases[] = {
          //--------
/*!  0 */ { 9, "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "TessellateWedgesRenderStep[Winding] + (empty)" },
/*!  1 */ { 9, "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "TessellateWedgesRenderStep[EvenOdd] + (empty)" },
/*!  2 */ { 9, "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "TessellateWedgesRenderStep[Convex] + SolidColor SrcOver" },
/*!  3 */ { 9, "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "TessellateStrokesRenderStep + SolidColor SrcOver" },
/*!  4 */ { 9, "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "TessellateCurvesRenderStep[Winding] + (empty)" },
/*!  5 */ { 9, "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/*!  6 */ { 9, "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "MiddleOutFanRenderStep[Winding] + (empty)" },
/*!  7 */ { 9, "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "MiddleOutFanRenderStep[EvenOdd] + (empty)" },
//          16: RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) +
//              CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver
/*!  8 */ { 9, "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver" },
/*!  9 */ { 9, "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "CoverBoundsRenderStep[InverseCover] + SolidColor SrcOver" },
/*! 10 */ { 9, "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "CoverageMaskRenderStep + SolidColor SrcOver" },
          //--------
/*! 11 */ { 9, "RP(color: Dawn(f=R8,s=4) w/ msaa load, resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "TessellateWedgesRenderStep[Winding] + (empty)" },
/*! 12 */ { 9, "RP(color: Dawn(f=R8,s=4) w/ msaa load, resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "TessellateWedgesRenderStep[EvenOdd] + (empty)" },
/*! 13 */ { 9, "RP(color: Dawn(f=R8,s=4) w/ msaa load, resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "TessellateWedgesRenderStep[Convex] + SolidColor SrcOver" },
/*! 14 */ { 9, "RP(color: Dawn(f=R8,s=4) w/ msaa load, resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "TessellateStrokesRenderStep + SolidColor SrcOver" },
/*! 15 */ { 9, "RP(color: Dawn(f=R8,s=4) w/ msaa load, resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "TessellateCurvesRenderStep[Winding] + (empty)" },
/*! 16 */ { 9, "RP(color: Dawn(f=R8,s=4) w/ msaa load, resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/*! 17 */ { 9, "RP(color: Dawn(f=R8,s=4) w/ msaa load, resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "MiddleOutFanRenderStep[Winding] + (empty)" },
/*! 18 */ { 9, "RP(color: Dawn(f=R8,s=4) w/ msaa load, resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "MiddleOutFanRenderStep[EvenOdd] + (empty)" },
//           5: RP(color: Dawn(f=R8,s=4) w/ msaa load, resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) +
//              CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver
/*! 19 */ { 9, "RP(color: Dawn(f=R8,s=4) w/ msaa load, resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver" },
/*! 20 */ { 9, "RP(color: Dawn(f=R8,s=4) w/ msaa load, resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "CoverBoundsRenderStep[InverseCover] + SolidColor SrcOver" },
/*! 21 */ { 9, "RP(color: Dawn(f=R8,s=4) w/ msaa load, resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
               "CoverageMaskRenderStep + SolidColor SrcOver" },
          //--------
/*  22 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateWedgesRenderStep[Winding] + (empty)" },
/*  23 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateWedgesRenderStep[EvenOdd] + (empty)" },
/*  24 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateWedgesRenderStep[Convex] + SolidColor SrcOver" },
/*  25 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateStrokesRenderStep + SolidColor SrcOver" },
/*  26 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateCurvesRenderStep[Winding] + (empty)" },
/*  27 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/*  28 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + SolidColor SrcOver" },
/*  29 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + SolidColor Src" },
/*  30 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + SolidColor Clear" },
/*  31 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  32 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  33 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  34 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  35 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "MiddleOutFanRenderStep[Winding] + (empty)" },
/*  36 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "MiddleOutFanRenderStep[EvenOdd] + (empty)" },
/*  37 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver" },
/*  38 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver AnalyticClip" },
/*  39 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/*  40 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor Src" },
/*  41 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor Clear" },
/*  42 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  43 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  44 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  45 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  46 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[InverseCover] + SolidColor SrcOver" },
          // Hmm - what even is this? A cover draw w/o a Shader?
/*  47 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[InverseCover] + (empty)" },
/*  48 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CoverageMaskRenderStep + SolidColor SrcOver" },
/*  49 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "CircularArcRenderStep + SolidColor SrcOver" },
/*  50 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "BitmapTextRenderStep[Mask] + SolidColor SrcOver" },
/*  51 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "AnalyticRRectRenderStep + SolidColor SrcOver" },
/*  52 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "AnalyticRRectRenderStep + SolidColor Src" },
/*  53 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "AnalyticRRectRenderStep + SolidColor Clear" },
/*  54 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  55 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  56 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  57 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  58 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "TessellateWedgesRenderStep[Winding] + (empty)" },
/*  59 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "TessellateWedgesRenderStep[EvenOdd] + (empty)" },
/*  60 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "TessellateWedgesRenderStep[Convex] + SolidColor SrcOver" },
/*  61 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "TessellateStrokesRenderStep + SolidColor SrcOver" },
/*  62 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "TessellateCurvesRenderStep[Winding] + (empty)" },
/*  63 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/*  64 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "MiddleOutFanRenderStep[Winding] + (empty)" },
/*  65 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "MiddleOutFanRenderStep[EvenOdd] + (empty)" },
/*  66 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver" },
/*  67 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  68 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "CoverBoundsRenderStep[InverseCover] + SolidColor SrcOver" },
/*  69 */ { 9, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
               "CoverageMaskRenderStep + SolidColor SrcOver" },
/*  70 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateWedgesRenderStep[Winding] + (empty)" },
/*  71 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateWedgesRenderStep[EvenOdd] + (empty)" },
/*  72 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateWedgesRenderStep[Convex] + SolidColor SrcOver" },
/*  73 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateStrokesRenderStep + SolidColor SrcOver" },
/*  74 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateCurvesRenderStep[Winding] + (empty)" },
/*  75 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/*  76 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + SolidColor SrcOver" },
/*  77 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + SolidColor Src" },
/*  78 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + SolidColor Clear" },
/*  79 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  80 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  81 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  82 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  83 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + MiddleOutFanRenderStep[Winding] + (empty)" },
/*  84 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + MiddleOutFanRenderStep[EvenOdd] + (empty)" },
/*  85 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver" },
/*  86 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver AnalyticClip" },
/*  87 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/*  88 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor Src" },
/*  89 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor Clear" },
/*  90 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  91 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  92 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
          // duplicate of 92
          //{ 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  93 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
          // duplicate of 92
          //{ 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  94 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[InverseCover] + SolidColor SrcOver" },
          // What even is this? A cover draw w/o a shader?
/*  95 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[InverseCover] + (empty)" },
/*  96 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverageMaskRenderStep + SolidColor SrcOver" },
/*  97 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CircularArcRenderStep + SolidColor SrcOver" },
/*  98 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + BitmapTextRenderStep[Mask] + SolidColor SrcOver" },
/*  99 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + SolidColor SrcOver" },
/* 100 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + SolidColor Src" },
/* 101 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + SolidColor Clear" },
/* 102 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 103 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 104 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 105 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 106 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + TessellateWedgesRenderStep[Winding] + (empty)" },
/* 107 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + TessellateWedgesRenderStep[EvenOdd] + (empty)" },
/* 108 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + TessellateWedgesRenderStep[Convex] + SolidColor SrcOver" },
/* 109 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + TessellateStrokesRenderStep + SolidColor SrcOver" },
/* 110 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + TessellateCurvesRenderStep[Winding] + (empty)" },
/* 111 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/* 112 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + MiddleOutFanRenderStep[Winding] + (empty)" },
/* 113 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + MiddleOutFanRenderStep[EvenOdd] + (empty)" },
/* 114 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver" },
/* 115 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[InverseCover] + SolidColor SrcOver" },
/* 116 */ { 9, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + CoverageMaskRenderStep + SolidColor SrcOver" },
/* 117 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + TessellateWedgesRenderStep[Winding] + (empty)" },
/* 118 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + TessellateWedgesRenderStep[EvenOdd] + (empty)" },
/* 119 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + TessellateWedgesRenderStep[Convex] + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/* 120 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + TessellateStrokesRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/* 121 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + TessellateCurvesRenderStep[Winding] + (empty)" },
/* 122 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/* 123 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + SolidColor SrcOver" },
/* 124 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + SolidColor Src" },
/* 125 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + SolidColor Clear" },
/* 126 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformSRGB ] ] Src" },
/* 127 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 128 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] Src" },
/* 129 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformSRGB ] ] Src" },
/* 130 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 131 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] Src" },
/* 132 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Src" },
/* 133 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 134 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src" },
/* 135 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformSRGB ] ] Src" },
/* 136 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 137 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] Src" },
/* 138 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + MiddleOutFanRenderStep[Winding] + (empty)" },
/* 139 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + MiddleOutFanRenderStep[EvenOdd] + (empty)" },
/* 140 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[RegularCover] + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/* 141 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/* 142 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor Src" },
/* 143 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor Clear" },
/* 144 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformSRGB ] ] Src" },
/* 145 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 146 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] Src" },
/* 147 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformSRGB ] ] Src" },
/* 148 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 149 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] Src" },
/* 150 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Src" },
/* 151 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/* 152 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 153 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src" },
/* 154 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformSRGB ] ] Src" },
/* 155 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 156 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] Src" },
/* 157 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[InverseCover] + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/* 158 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverageMaskRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/* 159 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + BitmapTextRenderStep[Mask] + SolidColor SrcOver" },
/* 160 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + SolidColor SrcOver" },
/* 161 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + SolidColor Src" },
/* 162 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + SolidColor Clear" },
/* 163 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformSRGB ] ] Src" },
/* 164 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 165 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] Src" },
/* 166 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformSRGB ] ] Src" },
/* 167 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 168 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] Src" },
/* 169 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Src" },
/* 170 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 171 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src" },
/* 172 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformSRGB ] ] Src" },
/* 173 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 174 */ { 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] Src" },
            // AnalyticBlurRenderStep is currently internal only
            //{ 9, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticBlurRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
            //{ 8, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticBlurRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
          // LinearGradient?
/* 175 */ { 8, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + BitmapTextRenderStep[Mask] + LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] SrcOver" },
/* 176 */ { 8, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/* 177 */ { 8, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/* 178 */ { 7, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/* 179 */ { 7, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/* 180 */ { 7, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
          // LinearGradient and a Dither?
/* 181 */ { 7, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
            // AnalyticBlurRenderStep is currently internal only
            //{ 7, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + AnalyticBlurRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/* 182 */ { 6, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/* 183 */ { 6, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/* 184 */ { 6, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/* 185 */ { 6, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/* 186 */ { 6, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/* 187 */ { 6, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/* 188 */ { 6, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/* 189 */ { 6, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver AnalyticClip" },
/* 190 */ { 5, "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
          // blurs need to wait
/* 191 */ { 5, "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000) + CoverBoundsRenderStep[NonAAFill] + KnownRuntimeEffect_1DBlur16 [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransform ] ] ] Src" },
/* 192 */ { 5, "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000) + AnalyticRRectRenderStep + SolidColor SrcOver" },
          // LinearGradient
/* 193 */ { 5, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/* 194 */ { 5, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverageMaskRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
          // LinearGradient
/* 195 */ { 5, "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + BitmapTextRenderStep[Mask] + LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] SrcOver" },
          // LinearGradient + Dither
/* 196 */ { 5, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
            // AnalyticBlurRenderStep is currently internal only
            //{ 5, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + AnalyticBlurRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/* 197 */ { 5, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/* 198 */ { 5, "RP(color: Dawn(f=BGRA8,s=4) w/ msaa load, resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + BitmapTextRenderStep[Mask] + SolidColor SrcOver" },
          // DstIn?
/* 199 */ { 5, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] DstIn" },
          // Luma
/* 200 */ { 5, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] KnownRuntimeEffect_Luma ] SrcOver" },
          // Alpha-only
/* 201 */ { 5, "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
    };

#if 0
    old 2 -> new 1
    old 3 -> new 8
    old 6 -> new 162
    old 7 -> new 161
    old 8 -> new 143
    old 9 -> new 144
    old 10 -> new 145
    old 11 -> new 155
    old 12 -> new 153
    old 13 -> new 160
    old 15 -> new 125
    old 16 -> new 136
    old 17 -> new 135
    old 18 -> new 134
    old 19 -> new 132
    old 20 -> new 67
    old 21 -> new 61
    old 22 -> new 60
    old 23 -> new 51
    old 24 -> new 50
    old 25 -> new 197
    old 26 -> new 49
    old 27 -> new 47
    old 28 -> new 37
    old 30 -> new 39
    old 31 -> new 41
    old 32 -> new 44
    old 33 -> new 33
    old 34 -> new 22
    old 35 -> new 23
    old 36 -> new 24
    old 38 -> new 154
    old 39 -> new 150
    old 41 -> new 152
    old 42 -> new 179
    old 43 -> new 181
    old 53 -> new 40
    old 55 -> new 38
    old 57 -> new 35
    old 58 -> new 36
    old 60 -> new 32
    old 61 -> new 26
    old 62 -> new 27
    old 65 -> new 25
    old 66 new 204 matches 44
#endif

#if 0
int matches = 0;
    for (size_t old = 0; old < std::size(kCasesOld); ++old) {
        for (size_t new1 = 0; new1 < std::size(kCases); ++new1) {
            if (!strcmp(kCasesOld[old], kCases[new1].fString)) {
                ++matches;
                SkDebugf("old %zu -> new %zu\n", old, new1);
            }
        }
    }
    SkDebugf("old %zu new %zu matches %d\n",
             std::size(kCasesOld),
             std::size(kCases),
             matches);
    exit(0);
#endif

    for (size_t i = 0; i < std::size(kCases); ++i) {
        for (size_t j = i+1; j < std::size(kCases); ++j) {
            if (!strcmp(kCases[i].fString, kCases[j].fString)) {
                SkDebugf("Duplicate %zu && %zu\n", i, j);
            }
        }
    }

    std::vector<bool> covered(std::size(kCases), false);

//    for (size_t i = 13; i < 17 /*std::size(kPrecompileCases)*/; ++i) {
    for (size_t i = 0; i < std::size(kPrecompileCases); ++i) {
        PrecompileSettings settings;

        run_test(precompileContext.get(), reporter,
                 { kCases, std::size(kCases) }, i,
                 kPrecompileCases[i], -1, covered);
    }

    int covered1 = 0, notCovered = 0, intentionallySkipped = 0;
    SkDebugf("not covered: ");
    for (size_t i = 0; i < std::size(kCases); ++i) {
        if (!covered[i]) {
            if (skip(kCases[i].fString)) {
                ++intentionallySkipped;
            } else {
                SkDebugf("%zu, ", i);
                ++notCovered;
            }
        } else {
            ++covered1;
        }
    }
    SkDebugf("\n");
    SkDebugf("covered %d notCovered %d skipped %d total %zu\n",
             covered1, notCovered, intentionallySkipped,
             std::size(kCases));

    return;
    for (size_t i = 0; i < std::size(kCases); ++i) {
        PrecompileSettings settings;

        // TODO(robertphillips): splitting kCases[i] into substrings (based on a " + " separator)
        // before passing to the helpers would make this prettier
        RenderPassProperties expectedRenderPassSettings = get_render_pass_properties(kCases[i].fString);
        DrawTypeFlags expectedDrawTypeFlags = get_draw_type_flags(kCases[i].fString);
        unsigned int expectedNumPipelines = 0;

        switch (i) {

            // 1 and 8 form a single-draw pair and are, thus, addressed by the same settings
            case 1: [[fallthrough]];   // TessellateWedgesRenderStep[EvenOdd]
            case 8:                    // kSrcOver - CoverBoundsRenderStep[RegularCover]
                // solid_srcover, kNonSimpleShape, kR_4_DS
                settings = kPrecompileCases[2];
                expectedNumPipelines = 11;   // This is pretty bad for just 2 Pipelines
                break;
#if 0
#if 0
            // Punting on the blur cases for now since they horribly over-generate
            case 0:             [[fallthrough]];
            case 1:
                renderPassSettings = kR_1_D;
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                paintOptions = blur_image();
                expectedNumPipelines = 11;
                break;
#endif

            // 2 and 3 form a single-draw pair and are, thus, addressed by the same settings
            case 2: [[fallthrough]];   // TessellateWedgesRenderStep[EvenOdd]
            case 3:                    // kSrcOver - CoverBoundsRenderStep[RegularCover]
                // solid_srcover, kNonSimpleShape, kR_4_DS
                settings = kPrecompileCases[2];
                expectedNumPipelines = 11;   // This is pretty bad for just 2 Pipelines
                break;

            // 4 and 13 share the same paintOptions
            case 4:  // AnalyticBlurRenderStep
#if 0
                // We need to handle AnalyticBlurs differently (b/403264070)
                // blend_porter_duff_color_filter_srcover(), kAnalyticBlur, kBGRA_1_D
                settings = kPrecompileCases[0];
                expectedNumPipelines = 1;
                break;
#else
                continue;
#endif

            case 13: // CoverageMaskRenderStep
                // this case could be greatly reduced if CoverageMaskRenderStep were split out
                // blend_porter_duff_color_filter_srcover(), kNonSimpleShape, kBGRA_1_D
                settings = kPrecompileCases[1];
                expectedNumPipelines = 11; // This is pretty bad for just 1 Pipeline
                break;

            // This is the only AlphaOnlyPaintColor case so, we're going to pass for now
            case 14:
                continue;

            // For now, we're passing on the AnalyticClip Pipelines
            case 5:  // same as 4 but with an AnalyticClip
            case 12:
            case 20:
            case 55:
                continue;

            // The two text pipelines (7 and 24) just differ in render pass settings (one MSAA, the
            // other not)
            case 7:
                // solid_srcover(), kBitmapText_Mask, kBGRA_1_D
                settings = kPrecompileCases[3];
                expectedNumPipelines = 1;
                break;
            case 24:
                // solid_srcover(), kBitmapText_Mask, kBGRA_4_DS
                settings = kPrecompileCases[4];
                expectedNumPipelines = 1;
                break;

            // 21 & 22 form a pair (since they both need kNonSimpleShape)
            case 21: // kSrcOver - TessellateStrokesRenderStep
            case 22: // kSrcOver - TessellateWedgesRenderStep[Convex]
                // solid_srcover(), kNonSimpleShape,  kBGRA_4_D
                settings = kPrecompileCases[5];
                expectedNumPipelines = 11;  // very bad for 2 pipelines
                break;

            // Skipping linear gradient text draw. It doesn't seem representative.
            case 25:
                continue;

            // TODO(robertphillips): these two cases aren't being generated!
            case 27:  // CoverBoundsRenderStep[InverseCover]
            case 29:  // CoverBoundsRenderStep[RegularCover]
                continue; // they should normally just fall through

            // the next 9 form a block (since they all need kNonSimpleShape)
            case 28: // kSrcOver - CoverBoundsRenderStep[RegularCover]
            case 34: // TessellateWedgesRenderStep[Winding]
            case 35: // TessellateWedgesRenderStep[EvenOdd]
            case 36: // kSrcOver - TessellateWedgesRenderStep[Convex]
            // related utility Pipelines
            case 57: // MiddleOutFanRenderStep[Winding]
            case 58: // MiddleOutFanRenderStep[EvenOdd]
            case 61: // TessellateCurvesRenderStep[Winding]
            case 62: // TessellateCurvesRenderStep[EvenOdd]
            case 65: // kSrcOver - TessellateStrokesRenderStep
                // solid_srcover(), kNonSimpleShape,  kBGRA_4_DS
                settings = kPrecompileCases[6];
                expectedNumPipelines = 11;  // not so bad for 9 pipelines
                break;

            // These are the same except for the blend mode
            // kSrcOver seems fine (since it gets 3 pipelines). kSrc and kClear seem like they
            // should be narrowed (since they're over-generating a lot)
            case 6:  // kSrcOver - AnalyticRRectRenderStep
            case 8:  // kSrcOver - CoverBoundsRenderStep[NonAAFill]
            case 9:  // kSrc     - CoverBoundsRenderStep[NonAAFill]
            case 10: // kClear   - CoverBoundsRenderStep[NonAAFill]
            case 15: // kSrcOver - PerEdgeAAQuadRenderStep
                // solid_clear_src_srcover(), kSimpleShape, kBGRA_1_D
                settings = kPrecompileCases[8];
                // This is 3 each for kClear, kSrc and kSrcOver:
                //     AnalyticRRectRenderStep
                //     CoverBoundsRenderStep[NonAAFill]
                //     PerEdgeAAQuadRenderStep
                expectedNumPipelines = 9; // This is pretty bad for 5 pipelines
                break;

            case 26: // kSrcOver - CircularArcRenderStep
                // solid_srcover(), kCircularArc, kBGRA_4_DS
                settings = kPrecompileCases[7];
                expectedNumPipelines = 1;
                break;

            case 23: // kSrcOver - AnalyticRRectRenderStep
            case 30: // kSrcOver - CoverBoundsRenderStep[NonAAFill]
            case 31: // kClear   - CoverBoundsRenderStep[NonAAFill]
            case 53: // kSrc     - CoverBoundsRenderStep[NonAAFill]
                //  solid_clear_src_srcover(), kSimpleShape, kBGRA_4_DS
                settings = kPrecompileCases[9];
                // This is 3 each for kClear, kSrc and kSrcOver:
                //     AnalyticRRectRenderStep
                //     CoverBoundsRenderStep[NonAAFill]
                //     PerEdgeAAQuadRenderStep
                expectedNumPipelines = 9; // This is pretty bad for 4 pipelines
                break;

            // For all the image paintOptions we could add the option to exclude cubics to
            // the public API
            case 11: // CoverBoundsRenderStep[NonAAFill] + HardwareImage(0) + kSrc
            case 16: // PerEdgeAAQuadRenderStep + HardwareImage(0) + kSrc
            case 17: // PerEdgeAAQuadRenderStep + HardwareImage(0) + kSrcOver
            case 19: // PerEdgeAAQuadRenderStep + Image(0) + kSrcOver
            case 38: // CoverBoundsRenderStep[NonAAFill] + HardwareImage(0) + kSrcOver
                // image_premul_src_srcover(), kSimpleShape, kBGRA_1_D
                settings = kPrecompileCases[10];
                expectedNumPipelines = 24; // a bad deal for 5 pipelines
                break;

            // same as except 16 except it has ColorSpaceTransformSRGB
            case 18: // PerEdgeAAQuadRenderStep + HardwareImage(0) + kSrc
                // image_srgb_src(), kSimpleShape, kBGRA_1_D_SRGB
                settings = kPrecompileCases[12];
                expectedRenderPassSettings.fDstCS = SkColorSpace::MakeSRGB();
                expectedNumPipelines = 12;  // a bad deal for 1 pipeline
                break;

            case 32: // CoverBoundsRenderStep[NonAAFill] + HardwareImage(0) + kSrcOver
            case 33: // PerEdgeAAQuadRenderStep + HardwareImage(0) + kSrcOver
            case 60: // PerEdgeAAQuadRenderStep + Image(0) + kSrcOver
                // image_premul_srcover(), kSimpleShape, kBGRA_4_DS
                settings = kPrecompileCases[11];
                expectedNumPipelines = 12; // a bad deal for 3 pipelines
                break;
#endif

            default:
                continue;
        }

        SkAssertResult(settings.fRenderPassProps == expectedRenderPassSettings);
        SkAssertResult(settings.fDrawTypeFlags == expectedDrawTypeFlags);

        if (settings.fRenderPassProps.fRequiresMSAA && caps->loadOpAffectsMSAAPipelines()) {
            expectedNumPipelines *= 2; // due to wgpu::LoadOp::ExpandResolveTexture
        }

        run_test(precompileContext.get(), reporter,
                 { kCases, std::size(kCases) }, i,
                 settings, expectedNumPipelines, covered);
    }
}

#endif // SK_GRAPHITE
