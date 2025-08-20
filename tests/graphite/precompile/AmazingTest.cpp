/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/core/SkCanvas.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/PrecompileContext.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "tools/graphite/GraphiteTestContext.h"

using namespace::skgpu::graphite;

const RenderPassProperties kRGBA_1_D {
        DepthStencilFlags::kDepth,
        kRGBA_8888_SkColorType,
        /* fDstCS= */ nullptr,
        /* fRequiresMSAA= */ false
};

namespace {

void precompile_an_arc_draw(skiatest::Reporter* reporter,
                            PrecompileContext* precompileContext) {
    PaintOptions paintOptions;
    paintOptions.addBlendMode(SkBlendMode::kSrcOver);

    Precompile(precompileContext,
               paintOptions,
               DrawTypeFlags::kCircularArc,
               { kRGBA_1_D });
}

void draw_an_arc(skiatest::Reporter* reporter,
                 Context* context,
                 skiatest::graphite::GraphiteTestContext* testContext) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkImageInfo ii = SkImageInfo::Make(16, 16,
                                       kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);


    sk_sp<SkSurface> surf = SkSurfaces::RenderTarget(recorder.get(), ii, skgpu::Mipmapped::kNo);
    SkCanvas* canvas = surf->getCanvas();

    SkArc arc = SkArc::Make(SkRect::MakeWH(16, 16), 0, 270, SkArc::Type::kWedge);
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrcOver);

    canvas->drawArc(arc, paint);

    std::unique_ptr<Recording> recording = recorder->snap();
    if (!recording) {
        ERRORF(reporter, "Failed to make recording");
        return;
    }

    context->insertRecording({ recording.get() });
    testContext->syncedSubmit(context);
}

} // anonymous namespace

// The goal here is to (initially for Native Metal and then for Dawn Metal):
//   kick off a precompile
//       threaded mode: Compilinator starts the precompile on a thread
//            - this will complete at some arbitrary time
//       non-threaded:  Compilinator fully compiles the Pipeline
//   kick off the draw
//       threaded mode: Compilator starts the Pipeline compilation on a thread
//            - this will complete at some arbitrary time
//            - it will probably be discarded in favor of the precompiled version
//       non-threaded:  Compilinator should just find the precompiled version

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(AmazingTest,
                                                     reporter,
                                                     context,
                                                     testContext,
                                                     /* condition= */ true,
                                                     CtsEnforcement::kNever) {
    std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();

    precompile_an_arc_draw(reporter, precompileContext.get());

    draw_an_arc(reporter, context, testContext);

#if 0
    auto begin = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();

    auto deltaMS = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

    const auto kDuration = std::chrono::milliseconds(1);

    REPORTER_ASSERT(reporter, deltaMS < kDuration);

    precompileContext->purgePipelinesNotUsedInMs(deltaMS);
#endif
}

#endif // SK_GRAPHITE
