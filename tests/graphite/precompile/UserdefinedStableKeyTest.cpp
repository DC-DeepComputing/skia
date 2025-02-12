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
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/PrecompileContext.h"
#include "include/gpu/graphite/Surface.h"
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

#include "tools/graphite/ContextFactory.h"

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

static const int kFoo = 7;

// This tests adds some user-defined stably-keyed runtime effects and then verifies that
// everything behaves as expected. For the purposes of this test, "behaves as expected" means:
//    the user-defined stably keyed runtime effects are in the ShaderCodeDictionary
//    the user-defined stable keys appear in the serialized Pipeline keys (from the PipelineCallBackHandler)
//    and said keys correctly (re)generate the desired pipelines
// For good measure this test also checks that the normal (non-serialized-key) Precompile API
// generates the same keys.
//
// The magic that is occurring here results from ...
//
// TODO: can we look up duplicate SkSL at runtime so I we always get the stably keyed version!
void foo(skiatest::Reporter* reporter, Context* context) {
    using namespace skiatest::graphite;

    std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();
    ShaderCodeDictionary* shaderCodeDictionary = context->priv().shaderCodeDictionary();

    SkASSERT_RELEASE(!context->priv().globalCache()->numGraphicsPipelines());
    SkASSERT_RELEASE(!shaderCodeDictionary->numUserDefinedRuntimeEffects());
    SkASSERT_RELEASE(shaderCodeDictionary->numUserDefinedKnownRuntimeEffects() == kFoo);

    std::unique_ptr<Recorder> recorder =
            context->makeRecorder(ToolUtils::CreateTestingRecorderOptions());
    REPORTER_ASSERT(reporter, recorder);

    auto [paint, paintOptions] = create_paint_and_options();

    draw_with_normal_api(context, recorder.get(), paint);

}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(UserDefinedStableKeyTest,
                                                     reporter,
                                                     context,
                                                     testContext,
                                                     /* cond= */ true,
                                                     CtsEnforcement::kNever) {
    std::unique_ptr<PipelineCallBackHandler> pipelineHandler(new PipelineCallBackHandler);

    ContextOptions options;  // TODO: it would be cool if we could get the existing options passed in
    options.fPipelineCallbackContext = pipelineHandler.get();
    options.fPipelineCallback = PipelineCallBackHandler::CallBack;

    // We're going to use all these runtime effects via the normal API (
    // (c.f. create_paint_and_options)
    sk_sp<SkRuntimeEffect> stableEffects [kFoo] = {
        sk_ref_sp(PrecompileFactories::GetAnnulusShaderEffect()),

        sk_ref_sp(PrecompileFactories::GetSrcBlenderEffect()),
        sk_ref_sp(PrecompileFactories::GetDstBlenderEffect()),
        sk_ref_sp(PrecompileFactories::GetComboBlenderEffect()),

        sk_ref_sp(PrecompileFactories::GetDoubleColorFilterEffect()),
        sk_ref_sp(PrecompileFactories::GetHalfColorFilterEffect()),
        sk_ref_sp(PrecompileFactories::GetComboColorFilterEffect()),
    };

    options.fSpecial = { stableEffects };

    ContextFactory workaroundFactory({ options });
    ContextInfo ctxInfo = workaroundFactory.getContextInfo(testContext->contextType());
    foo(reporter, ctxInfo.fContext);
}

#endif // SK_GRAPHITE
