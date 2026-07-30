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
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "tools/graphite/ContextFactory.h"
#include "tools/graphite/GraphiteTestContext.h"

using namespace skgpu::graphite;
using namespace skiatest::graphite;

const skgpu::graphite::RenderPassProperties kR_1_D {
    skgpu::graphite::DepthStencilFlags::kDepth,
    kAlpha_8_SkColorType,
    /* fDstCS= */ nullptr,
    /* fRequiresMSAA= */ false
};

const skgpu::graphite::RenderPassProperties kRGBA_1_D {
    skgpu::graphite::DepthStencilFlags::kDepth,
    kRGBA_8888_SkColorType,
    /* fDstCS= */ nullptr,
    /* fRequiresMSAA= */ false
};

const skgpu::graphite::RenderPassProperties kRGBA_1_D_SRGB {
    skgpu::graphite::DepthStencilFlags::kDepth,
    kRGBA_8888_SkColorType,
    SkColorSpace::MakeSRGB(),
    /* fRequiresMSAA= */ false
};

const skgpu::graphite::RenderPassProperties kRGBA_4_DS {
    skgpu::graphite::DepthStencilFlags::kDepthStencil,
    kRGBA_8888_SkColorType,
    /* fDstCS= */ nullptr,
    /* fRequiresMSAA= */ true
};

const skgpu::graphite::RenderPassProperties kRGBA_4_DS_SRGB {
    skgpu::graphite::DepthStencilFlags::kDepthStencil,
    kRGBA_8888_SkColorType,
    SkColorSpace::MakeSRGB(),
    /* fRequiresMSAA= */ true
};

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(PipelineManagerEarlyExitTest,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           /* origContext */,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {

    // Create a scoped Context so we can destroy it early
    {
        ContextFactory factory(origOptions);
        ContextInfo ctxInfo = factory.getContextInfo(origTestContext->contextType());

        std::unique_ptr<PrecompileContext> precompileContext =
                ctxInfo.fContext->makePrecompileContext();

        // Queue up an excess of precompiles
        PaintOptions paintOptions;
        paintOptions.setShaders({ PrecompileShaders::LinearGradient(
                                          PrecompileShaders::GradientShaderFlags::kAll) });

        Precompile(precompileContext.get(),
                   paintOptions,
                   static_cast<DrawTypeFlags>(DrawTypeFlags::kSimpleShape |
                                              DrawTypeFlags::kNonSimpleShape),
                   { kR_1_D, kRGBA_1_D, kRGBA_1_D_SRGB, kRGBA_4_DS, kRGBA_4_DS_SRGB });

        precompileContext = nullptr;
        // Destroying the Context here should trigger PipelineManager shutdown with work still
        // pending.
    }

    // Success is defined by not crashing or triggering any *SAN errors
}

#endif // SK_GRAPHITE
