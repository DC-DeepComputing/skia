/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/core/SkColorSpace.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileBlender.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/PrecompileInternal.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/precompile/PaintOptionsPriv.h"
#include "tools/graphite/UniqueKeyUtils.h"

#include <array>

using namespace::skgpu::graphite;

struct Blob {
    DepthStencilFlags fDSFlags = DepthStencilFlags::kNone;
    SkColorType fDstCT = kRGBA_8888_SkColorType;
    bool fRequiresMSAA = false;
};

namespace {

// "SolidColor SrcOver"
PaintOptions solid_srcover() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions image_srcover() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Image() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions image_src() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Image() });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

// "RP(color: Dawn(f=23,s=1), resolve: Mock(s=1), ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba)"
// Single sampled BGRA w/ just depth
Blob blob1() {
    return { DepthStencilFlags::kDepth, kBGRA_8888_SkColorType, /* requiresMSAA= */ false };
}

void run_test(Context* context,
              skiatest::Reporter* reporter,
              const char* expectedString, size_t caseID,
              const PaintOptions& paintOptions,
              DrawTypeFlags drawType,
              const Blob& blob,
              unsigned int allowedOvergeneration) {

    context->priv().globalCache()->resetGraphicsPipelines();

    Precompile(context, paintOptions, drawType, blob.fDSFlags, blob.fDstCT, blob.fRequiresMSAA);

    std::vector<std::string> generated;

    {
        const RendererProvider* rendererProvider = context->priv().rendererProvider();
        const ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

        std::vector<skgpu::UniqueKey> generatedKeys;

        UniqueKeyUtils::FetchUniqueKeys(context->priv().globalCache(), &generatedKeys);

        for (const skgpu::UniqueKey& key : generatedKeys) {
            GraphicsPipelineDesc pipelineDesc;
            RenderPassDesc renderPassDesc;
            UniqueKeyUtils::ExtractKeyDescs(context, key, &pipelineDesc, &renderPassDesc);

            const RenderStep* renderStep = rendererProvider->lookup(pipelineDesc.renderStepID());
            generated.push_back(GetPipelineLabel(dict, renderPassDesc, renderStep,
                                                 pipelineDesc.paintParamsID()));
        }
    }

    REPORTER_ASSERT(reporter, generated.size() <= allowedOvergeneration,
                    "case %zu overgenerated - %zu > %d\n",
                    caseID, generated.size(), allowedOvergeneration);

    const size_t len = strlen(expectedString);

    bool foundIt = false;
    for (size_t i = 0; i < generated.size(); ++i) {
        // The generated strings have trailing whitespace
        if (!strncmp(expectedString, generated[i].c_str(), len)) {
            foundIt = true;
            break;
        }
    }

    if (foundIt) {
        return;
    }

    SkDebugf("Expected string:\n%s\n%s in %zu strings:\n",
             expectedString,
             foundIt ? "found" : "NOT found",
             generated.size());

    for (size_t i = 0; i < generated.size(); ++i) {
        SkDebugf("%zu: %s\n", i, generated[i].c_str());
    }
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ChromeTest, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    SkDebugf("----- %s\n", BackendApiToStr(context->backend()));

    // RGBA8Unorm = 18
    // BGRA8Unorm = 23
    // Depth16Unorm = 39
    // Depth24PlusStencil8 = 41

    const char* kCases[] = {
        // Wikipedia 2018
        /*  0 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + TessellateWedgesRenderStep[winding] + (empty)",
        /*  -1 */ "RP(color: Dawn(f=23,s=1), resolve: Mock(s=1), ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + SolidColor SrcOver",
        /*  -2 */ "RP(color: Dawn(f=23,s=1), resolve: Mock(s=1), ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[non-aa-fill] + LocalMatrix [ Compose [ HardwareImage fData(size: 0) ColorSpaceTransform ] ] SrcOver",
        /*  3 */ "RP(color: Dawn(f=23,s=1), resolve: Mock(s=1), ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[non-aa-fill] + SolidColor SrcOver",
        /*  -4 */ "RP(color: Dawn(f=23,s=1), resolve: Mock(s=1), ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage fData(size: 0) ColorSpaceTransform ] ] Src",
        /*  5 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + TessellateWedgesRenderStep[evenodd] + (empty)",
        /*  6 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + TessellateWedgesRenderStep[convex] + SolidColor SrcOver",
        /*  7 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[non-aa-fill] + SolidColor SrcOver",
        /*  8 */ "RP(color: Dawn(f=23,s=1), resolve: Mock(s=1), ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + AnalyticBlurRenderStep + Compose [ SolidColor Blend [ SolidColor Passthrough BlendModeBlender ] ] SrcOver",
        /*  9 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + TessellateStrokeRenderStep + SolidColor SrcOver",
        /* -10 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + BitmapTextRenderStep[mask] + LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransform ] ] SrcOver",
        /* -11 */ "RP(color: Dawn(f=23,s=1), resolve: Mock(s=1), ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[non-aa-fill] + SolidColor Src",
        /* -12 */ "RP(color: Dawn(f=23,s=1), resolve: Mock(s=1), ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[non-aa-fill] + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransform ] ] Dither ] SrcOver",
        /* -13 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[non-aa-fill] + SolidColor Src",
        /* -14 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage fData(size: 0) ColorSpaceTransform ] ] SrcOver",
        /* -15 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransform ] ] Dither ] SrcOver",
        /* -16 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[non-aa-fill] + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransform ] ] Dither ] SrcOver",
        /* 17 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ Image fData(size: 0) ColorSpaceTransform ] ] SrcOver",
        /* -18 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + BitmapTextRenderStep[mask] + SolidColor SrcOver",
        /* -19 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[non-aa-fill] + LocalMatrix [ Compose [ HardwareImage fData(size: 0) ColorSpaceTransform ] ] SrcOver",
    };

    for (size_t i = 0; i < std::size(kCases); ++i) {
        PaintOptions paintOptions;
        DrawTypeFlags drawTypeFlags = DrawTypeFlags::kSimpleShape;
        Blob blob;
        unsigned int allowedOvergeneration = 0;

        switch (i) {


            case 1:
                paintOptions = solid_srcover();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                blob = blob1();
                allowedOvergeneration = 15;  // all from RenderSteps
                break;
            case 2:
                paintOptions = image_srcover();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                blob = blob1();
                allowedOvergeneration = 60; // !!!! - a lot for just a non-aa image rect draw
                break;
            case 4:
                paintOptions = image_src();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                blob = blob1();
                // This is a lot for a kSrc image draw:
                allowedOvergeneration = 60; // 3x of this are the paint combos,
                                            // the rest (20x) are the RenderSteps!!
                break;
#if 0
            case 10:
                paintOptions.setShaders({ PrecompileShaders::LinearGradient() });
                paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
                drawTypeFlags = DrawTypeFlags::kBitmapText_Mask;
                dsFlags = DepthStencilFlags::kDepthStencil;
                dstCT = kBGRA_8888_SkColorType;
                requiresMSAA = true;
                allowedOvergeneration = 3; // from the 3 internal gradient alternatives
                break;
            case 11: // only differs from 13 by MSAA
                paintOptions.setBlendModes({ SkBlendMode::kSrc });
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                dsFlags = DepthStencilFlags::kDepth;
                dstCT = kBGRA_8888_SkColorType;
                requiresMSAA = false;
                allowedOvergeneration = 15; // a lot for a rectangle clear - all RenderSteps
                break;
            case 12:
                paintOptions.setShaders({ PrecompileShaders::LinearGradient() });
                paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
                paintOptions.setDither(/* dither= */ true);
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                dsFlags = DepthStencilFlags::kDepth;
                dstCT = kBGRA_8888_SkColorType;
                requiresMSAA = false;
                allowedOvergeneration = 33; // 3x from gradient, 11x from RenderSteps
                break;
            case 13: // only differs from 11 by MSAA
                paintOptions.setBlendModes({ SkBlendMode::kSrc });
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                dsFlags = DepthStencilFlags::kDepthStencil;
                dstCT = kBGRA_8888_SkColorType;
                requiresMSAA = true;
                allowedOvergeneration = 15; // a lot for a rectangle clear - all RenderSteps
                break;
            case 14: // these two are paired in an AA rect draw
            case 19:
                paintOptions.setShaders({ PrecompileShaders::Image() });
                paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                dsFlags = DepthStencilFlags::kDepthStencil;
                dstCT = kBGRA_8888_SkColorType;
                requiresMSAA = true;
                allowedOvergeneration = 60;
                break;
            case 15: // 15 & 16 are combined pair
            case 16:
                paintOptions.setShaders({ PrecompileShaders::LinearGradient() });
                paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
                paintOptions.setDither(/* dither= */ true);
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                dsFlags = DepthStencilFlags::kDepthStencil;
                dstCT = kBGRA_8888_SkColorType;
                requiresMSAA = true;
                allowedOvergeneration = 33; // 3x from gradient, 11x from RenderSteps
                break;
            case 18:
                paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
                drawTypeFlags = DrawTypeFlags::kBitmapText_Mask;
                dsFlags = DepthStencilFlags::kDepthStencil;
                dstCT = kBGRA_8888_SkColorType;
                requiresMSAA = true;
                allowedOvergeneration = 1;
                break;
            /* case 19 is paired with case 14 */
#endif
            default:
                continue;
        }

        SkDebugf("running case %zu\n", i);
        run_test(context, reporter,
                 kCases[i], i,
                 paintOptions, drawTypeFlags, blob, allowedOvergeneration);
    }
}

#endif // SK_GRAPHITE
