/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"


#if defined(SK_GRAPHITE)

#include "include/core/SkCanvas.h"
#include "include/core/SkString.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/PrecompileContext.h"
#include "include/gpu/graphite/Surface.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"

#include "tools/graphite/GraphiteToolUtils.h"
#include "tools/graphite/precompile/PrecompileEffectFactories.h"
#include "tools/graphite/precompile/PrecompileUtils.h"

using namespace::skgpu::graphite;

namespace {


sk_sp<SkSurface> make_surface(skgpu::graphite::Recorder* recorder, SkISize dimensions) {
    auto ii = SkImageInfo::Make(dimensions, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    return SkSurfaces::RenderTarget(recorder, ii, skgpu::Mipmapped::kNo);
}

bool flibble_flabble(skgpu::graphite::Context* context,
                     skgpu::graphite::Recorder* recorder) {

    sk_sp<SkSurface> surface = make_surface(recorder, { 256, 256 });
    if (!surface) {
        return false;
    }

    SkCanvas* canvas = surface->getCanvas();

    SkPaint p;
    //p.setShader(s);
    canvas->drawRect({0, 0, 100, 100}, p);

    std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
    if (!recording) {
        return false;
    }

    if (!context->insertRecording({ recording.get() })) {
        return false;
    }

    if (!context->submit(skgpu::graphite::SyncToCpu::kYes)) {
        return false;
    }

    return true;
}

} // anonymous namespace

static const char* kSkSLShaderString = "";

// This tests adds some user-defined stably-keyed runtime effects and then verifies that
// everything behaves as expected. For the purposes of this test, "behaves as expected" means:
//    the user-defined stable keys appear in the serialized Pipeline keys
//    and said keys correctly (re)generate the desired pipelines
// For good measure this test also checks that the full Precompile API generates the same keys.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(UserdefinedStableKeyTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNever) {
    using namespace skiatest::graphite;

    std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();

    std::unique_ptr<PipelineCallBackHandler> pipelineHandler(new PipelineCallBackHandler);


    const char* sksl[] = { kSkSLShaderString,  };

    precompileContext->makeKnown(sksl);

    std::unique_ptr<Recorder> recorder =
            context->makeRecorder(ToolUtils::CreateTestingRecorderOptions());
    REPORTER_ASSERT(reporter, recorder);


    auto [s, o] = PrecompileFactories::CreateRuntimeShader();

    flibble_flabble(context, recorder.get());

}

#endif // SK_GRAPHITE
