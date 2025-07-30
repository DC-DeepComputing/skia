/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/PrecompileContext.h"
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"

#if 0
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkTextBlob.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/graphite/UniqueKeyUtils.h"

#include <algorithm>
#include <random>
#include <thread>
#endif

using namespace::skgpu::graphite;

const RenderPassProperties kRGBA_1_D {
        DepthStencilFlags::kDepth,
        kRGBA_8888_SkColorType,
        /* fDstCS= */ nullptr,
        /* fRequiresMSAA= */ false
};

namespace {

void precompile(PrecompileContext* precompileContext) {
    PaintOptions paintOptions;
    paintOptions.addBlendMode(SkBlendMode::kSrcOver);

    Precompile(precompileContext,
               paintOptions,
               DrawTypeFlags::kCircularArc,
               { kRGBA_1_D });
}

} // anonymous namespace

// The goal here is to (initially for Native Metal and then for Dawn Metal):
//   kick off a precompile
//   kick off the draw
//   see the Compilinator start up the precompile compile
//   see the draw wait
//   see the Compilinator discard the normal compile

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(AmazingTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNever) {
    std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();

    auto begin = std::chrono::steady_clock::now();

    precompile(precompileContext.get());

    auto end = std::chrono::steady_clock::now();

    auto deltaMS = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

    const auto kDuration = std::chrono::milliseconds(1);

    REPORTER_ASSERT(reporter, deltaMS < kDuration);

    precompileContext->purgePipelinesNotUsedInMs(deltaMS);
}

#endif // SK_GRAPHITE
