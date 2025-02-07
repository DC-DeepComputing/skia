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

using namespace::skgpu::graphite;

namespace {

// draw a circle centered at "center" w/ inner and outer radii in "radii"
static const char* str1 =
        "uniform float2 center;"
        "uniform float2 radii;"
        "half4 main(float2 xy) {"
        "float len = length(xy - center);"
        "half value = len < radii.x ? 0.0 : (len > radii.y ? 0.0 : 1.0);"
        "return half4(value);"
        "}";

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_runtime_shader() {
    SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForShader(SkString(str1));
    SkASSERT(result.effect);

    static const float kUniforms[4] = { 50.0f, 50.0f, 40.0f, 50.0f };

    sk_sp<SkData> uniforms = SkData::MakeWithCopy(kUniforms, sizeof(kUniforms));

    sk_sp<SkShader> s = result.effect->makeShader(std::move(uniforms), /* children= */ {});
    sk_sp<PrecompileShader> o = PrecompileRuntimeEffects::MakePrecompileShader(result.effect);
    return { std::move(s), std::move(o) };
}

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


// This tests adds some user-defined stably-keyed runtime effects and then verifies that
// everything behaves as expected. For the purposes of this test, "behaves as expected" means:
//    sdfs;jfdskfj
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(UserdefinedStableKeyTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNever) {

    std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();

    // TODO: create a pipelinecallbackhandler here

    precompileContext->heyBuddy();

    std::unique_ptr<Recorder> recorder =
            context->makeRecorder(ToolUtils::CreateTestingRecorderOptions());
    REPORTER_ASSERT(reporter, recorder);


    auto [s, o] = create_runtime_shader();

    flibble_flabble(context, recorder.get());

}

#endif // SK_GRAPHITE
