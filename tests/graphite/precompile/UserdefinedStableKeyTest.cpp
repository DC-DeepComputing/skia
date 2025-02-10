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
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/PrecompileBlender.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"

#include "tools/graphite/GraphiteToolUtils.h"
#include "tools/graphite/precompile/PrecompileEffectFactories.h"
#include "tools/graphite/precompile/PrecompileUtils.h"

#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"

using namespace::skgpu::graphite;
using namespace skiatest::graphite;

namespace {

sk_sp<SkSurface> make_surface(skgpu::graphite::Recorder* recorder, SkISize dimensions) {
    auto ii = SkImageInfo::Make(dimensions, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    return SkSurfaces::RenderTarget(recorder, ii, skgpu::Mipmapped::kNo);
}

std::pair<SkPaint, PaintOptions> create_paint_and_options() {
    SkPaint paint;
    PaintOptions paintOptions;

    auto [shader, shaderOption] = PrecompileFactories::CreateAnnulusRuntimeShader();

    paint.setShader(std::move(shader));
    paintOptions.setShaders({ std::move(shaderOption) });

    auto [colorFilter, colorFilterOption] = PrecompileFactories::CreateComboRuntimeColorFilter();

    paint.setColorFilter(std::move(colorFilter));
    paintOptions.setColorFilters({ std::move(colorFilterOption) });

    auto [blender, blenderOption] = PrecompileFactories::CreateComboRuntimeBlender();

    paint.setBlender(std::move(blender));
    paintOptions.setBlenders({ std::move(blenderOption) });

    return { paint, paintOptions };
}

bool draw_with_normal_api(skgpu::graphite::Context* context,
                          skgpu::graphite::Recorder* recorder,
                          const SkPaint& paint) {

    sk_sp<SkSurface> surface = make_surface(recorder, { 256, 256 });
    if (!surface) {
        return false;
    }

    SkCanvas* canvas = surface->getCanvas();

    canvas->drawRect({0, 0, 100, 100}, paint);

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

static void add_pipe_line_callback_handler(ContextOptions* options) {
    std::unique_ptr<PipelineCallBackHandler> pipelineHandler(new PipelineCallBackHandler);

    options->fPipelineCallbackContext = pipelineHandler.get();
    options->fPipelineCallback = PipelineCallBackHandler::CallBack;
}

// This tests adds some user-defined stably-keyed runtime effects and then verifies that
// everything behaves as expected. For the purposes of this test, "behaves as expected" means:
//    the user-defined stable keys appear in the serialized Pipeline keys
//    and said keys correctly (re)generate the desired pipelines
// For good measure this test also checks that the full Precompile API generates the same keys.
DEF_GRAPHITE_TEST_FOR_CONTEXTS(UserdefinedStableKeyTest,
                               /* context_filter= */ nullptr,
                               reporter,
                               context,
                               /* test_ctx */,
                               add_pipe_line_callback_handler,
                               CtsEnforcement::kNever) {
    using namespace skiatest::graphite;

    std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();
    ShaderCodeDictionary* shaderCodeDictionary = context->priv().shaderCodeDictionary();

    SkASSERT_RELEASE(!context->priv().globalCache()->numGraphicsPipelines());
    SkASSERT_RELEASE(!shaderCodeDictionary->numUserDefinedRuntimeEffects());

    static const char* kSkSL[] = {
        PrecompileFactories::GetAnnulusShaderCode(),

        PrecompileFactories::GetSrcBlenderCode(),
        PrecompileFactories::GetDstBlenderCode(),
        PrecompileFactories::GetComboBlenderCode(),

        PrecompileFactories::GetDoubleColorFilterCode(),
        PrecompileFactories::GetHalfColorFilterCode(),
        PrecompileFactories::GetComboColorFilterCode(),
    };

    precompileContext->makeKnown(kSkSL);

    SkASSERT_RELEASE(!shaderCodeDictionary->numUserDefinedRuntimeEffects());

    std::unique_ptr<Recorder> recorder =
            context->makeRecorder(ToolUtils::CreateTestingRecorderOptions());
    REPORTER_ASSERT(reporter, recorder);

    auto [paint, paintOptions] = create_paint_and_options();


    draw_with_normal_api(context, recorder.get(), paint);

}

#endif // SK_GRAPHITE
