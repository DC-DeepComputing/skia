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
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/PrecompileInternal.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/precompile/PaintOptionsPriv.h"
#include "tools/graphite/UniqueKeyUtils.h"

#include <array>

using namespace::skgpu::graphite;

namespace {

void run_test(Context* context,
              const PaintOptions& paintOptions,
              DrawTypeFlags drawType,
              DepthStencilFlags dsFlags,
              SkColorType dstCT,
              bool requiresMSAA,
              const char* expectedString,
              skiatest::Reporter* reporter) {
    context->priv().globalCache()->resetGraphicsPipelines();

    int before = context->priv().globalCache()->numGraphicsPipelines();
    SkASSERT(before == 0);
    Precompile(context, paintOptions, drawType, dsFlags, dstCT, requiresMSAA);
//    int after = context->priv().globalCache()->numGraphicsPipelines();

    {
        const RendererProvider* rendererProvider = context->priv().rendererProvider();
        const ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

        std::vector<skgpu::UniqueKey> afterKeys;

        UniqueKeyUtils::FetchUniqueKeys(context->priv().globalCache(), &afterKeys);

        for (size_t i = 0; i < afterKeys.size(); ++i) {
            const skgpu::UniqueKey& afterKey = afterKeys[i];

            GraphicsPipelineDesc originalPipelineDesc;
            RenderPassDesc originalRenderPassDesc;
            UniqueKeyUtils::ExtractKeyDescs(context, afterKey,
                                            &originalPipelineDesc,
                                            &originalRenderPassDesc);

            SkDebugf("------- key %zu:\n", i);
            UniqueKeyUtils::DumpDescs(rendererProvider, dict,
                                      originalPipelineDesc,
                                      originalRenderPassDesc);
        }
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
        "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + TessellateWedgesRenderStep[winding] + (empty)",
        "RP(color: Dawn(f=23,s=1), resolve: Mock(s=1),      ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + AnalyticRRectRenderStep + SolidColor SrcOver",
        "RP(color: Dawn(f=23,s=1), resolve: Mock(s=1),      ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[non-aa-fill] + LocalMatrix [ Compose [ HardwareImage fData(size: 0) ColorSpaceTransform ] ] SrcOver",
        "RP(color: Dawn(f=23,s=1), resolve: Mock(s=1),      ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[non-aa-fill] + SolidColor SrcOver",
        "RP(color: Dawn(f=23,s=1), resolve: Mock(s=1),      ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage fData(size: 0) ColorSpaceTransform ] ] Src",
        "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + TessellateWedgesRenderStep[evenodd] + (empty)",
        "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + TessellateWedgesRenderStep[convex] + SolidColor SrcOver",
        "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[non-aa-fill] + SolidColor SrcOver",
        "RP(color: Dawn(f=23,s=1), resolve: Mock(s=1),      ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + AnalyticBlurRenderStep + Compose [ SolidColor Blend [ SolidColor Passthrough BlendModeBlender ] ] SrcOver",
        "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + TessellateStrokeRenderStep + SolidColor SrcOver",
/**/    "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + BitmapTextRenderStep[mask] + LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransform ] ] SrcOver",
        "RP(color: Dawn(f=23,s=1), resolve: Mock(s=1),      ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[non-aa-fill] + SolidColor Src",
        "RP(color: Dawn(f=23,s=1), resolve: Mock(s=1),      ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + CoverBoundsRenderStep[non-aa-fill] + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransform ] ] Dither ] SrcOver",
        "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[non-aa-fill] + SolidColor Src",
        "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage fData(size: 0) ColorSpaceTransform ] ] SrcOver",
        "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + AnalyticRRectRenderStep + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransform ] ] Dither ] SrcOver",
        "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[non-aa-fill] + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransform ] ] Dither ] SrcOver",
        "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ Image fData(size: 0) ColorSpaceTransform ] ] SrcOver",
/**/    "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + BitmapTextRenderStep[mask] + SolidColor SrcOver",
        "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + CoverBoundsRenderStep[non-aa-fill] + LocalMatrix [ Compose [ HardwareImage fData(size: 0) ColorSpaceTransform ] ] SrcOver",
    };

    for (size_t i = 0; i < std::size(kCases); ++i) {
        PaintOptions paintOptions;
        DrawTypeFlags drawTypeFlags = DrawTypeFlags::kSimpleShape;
        DepthStencilFlags dsFlags = DepthStencilFlags::kNone;
        SkColorType dstCT = kRGBA_8888_SkColorType;
        bool requiresMSAA = false;

        switch (i) {
            case 10:
                paintOptions.setShaders({ PrecompileShaders::LinearGradient() });
                paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
                drawTypeFlags = DrawTypeFlags::kBitmapText_Mask;
                dsFlags = DepthStencilFlags::kDepthStencil;
                dstCT = kBGRA_8888_SkColorType;
                requiresMSAA = true;
                break;
//            case 18:
//                paintOptions.setBlendModes(SkBlendMode::kSrcOver);
//                drawTypeFlags = DrawTypeFlags::kBitmapText_Mask;
//                break;
            default:
                continue;
        }

        run_test(context, paintOptions, drawTypeFlags, dsFlags, dstCT, requiresMSAA, kCases[i], reporter);
    }
}

#endif // SK_GRAPHITE
