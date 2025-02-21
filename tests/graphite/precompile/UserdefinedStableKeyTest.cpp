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
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileBlender.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/PrecompileContextPriv.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "tools/graphite/ContextFactory.h"
#include "tools/graphite/GraphiteToolUtils.h"
#include "tools/graphite/UniqueKeyUtils.h"
#include "tools/graphite/precompile/PipelineCallbackHandler.h"
#include "tools/graphite/precompile/PrecompileEffectFactories.h"

using namespace::skgpu::graphite;
using namespace skiatest::graphite;
using namespace skiatools::graphite;

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

// c.f. GraphitePrecompileTestingSink::resetAndRecreatePipelines
void reset_and_recreate_pipelines_with_serialized_keys(skiatest::Reporter* reporter,
                                                       PipelineCallBackHandler* handler,
                                                       PrecompileContext* precompileContext) {
    GlobalCache* globalCache = precompileContext->priv().globalCache();

    std::vector<skgpu::UniqueKey> origKeys;
    UniqueKeyUtils::FetchUniqueKeys(precompileContext, &origKeys);
    globalCache->resetGraphicsPipelines();

    std::vector<sk_sp<SkData>> androidStyleKeys;
    handler->retrieve(&androidStyleKeys);
    handler->reset();

    // We expect one serialized key - chock full of user-defined stable keys
    REPORTER_ASSERT(reporter, origKeys.size() == 1);
    REPORTER_ASSERT(reporter, androidStyleKeys.size() == 1);

    REPORTER_ASSERT(reporter, globalCache->numGraphicsPipelines() == 0);
    REPORTER_ASSERT(reporter, handler->numKeys() == 0);

    // Use the serialized keys to regenerate the Pipelines
    for (sk_sp<SkData>& d : androidStyleKeys) {
        bool result = precompileContext->precompile(d);
        SkAssertResult(result);
    }

    REPORTER_ASSERT(reporter, globalCache->numGraphicsPipelines() == 1);
    REPORTER_ASSERT(reporter, handler->numKeys() == 1);

    {
        std::vector<skgpu::UniqueKey> recreatedKeys;
        UniqueKeyUtils::FetchUniqueKeys(precompileContext, &recreatedKeys);

        REPORTER_ASSERT(reporter, recreatedKeys.size() == 1);
        REPORTER_ASSERT(reporter, origKeys[0] == recreatedKeys[0]);
    }
    {
        std::vector<sk_sp<SkData>> recreatedAndroidStyleKeys;
        handler->retrieve(&recreatedAndroidStyleKeys);

        REPORTER_ASSERT(reporter, recreatedAndroidStyleKeys.size() == 1);
        REPORTER_ASSERT(reporter, androidStyleKeys[0]->equals(recreatedAndroidStyleKeys[0].get()));
    }
}

void reset_and_recreate_pipelines_with_normal_api(skiatest::Reporter* reporter,
                                                  ShaderCodeDictionary* shaderCodeDictionary,
                                                  PipelineCallBackHandler* handler,
                                                  PrecompileContext* precompileContext,
                                                  const PaintOptions& paintOptions) {
    GlobalCache* globalCache = precompileContext->priv().globalCache();

    std::vector<skgpu::UniqueKey> origKeys;
    UniqueKeyUtils::FetchUniqueKeys(precompileContext, &origKeys);
    globalCache->resetGraphicsPipelines();

    std::vector<sk_sp<SkData>> androidStyleKeys;
    handler->retrieve(&androidStyleKeys);
    handler->reset();

    // We expect one serialized key - full of user-defined stable keys
    REPORTER_ASSERT(reporter, origKeys.size() == 1);
    REPORTER_ASSERT(reporter, androidStyleKeys.size() == 1);

    REPORTER_ASSERT(reporter, globalCache->numGraphicsPipelines() == 0);
    REPORTER_ASSERT(reporter, handler->numKeys() == 0);

    RenderPassProperties renderPassProps;
    renderPassProps.fDSFlags = DepthStencilFlags::kDepth;

    Precompile(precompileContext,
               paintOptions,
               DrawTypeFlags::kSimpleShape,
               { renderPassProps });

    {
        std::vector<skgpu::UniqueKey> recreatedKeys;
        UniqueKeyUtils::FetchUniqueKeys(precompileContext, &recreatedKeys);

        bool foundIt = false;
        for (auto k : recreatedKeys) {
            if (origKeys[0] == k) {
                foundIt = true;
                break;
            }
        }
        REPORTER_ASSERT(reporter, foundIt);
    }
    {
        std::vector<sk_sp<SkData>> recreatedAndroidStyleKeys;
        handler->retrieve(&recreatedAndroidStyleKeys);

        bool foundIt = false;
        for (auto k : recreatedAndroidStyleKeys) {
            if (androidStyleKeys[0]->equals(k.get())) {
                foundIt = true;
                break;
            }
        }
        REPORTER_ASSERT(reporter, foundIt);
    }
}

// The blessed shader will have a user-defined stable key
// The cursed shader has the same SkSL as the blessed one but no stable key
std::pair<sk_sp<SkShader>, sk_sp<SkShader>> make_annulus_shader_pair() {
    SkRuntimeEffect* blessed = PrecompileFactories::GetAnnulusShaderEffect();

    SkRuntimeEffect* cursed = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                                  PrecompileFactories::GetAnnulusShaderCode());


    static const float kUniforms[4] = { 50.0f, 50.0f, 40.0f, 50.0f };

    sk_sp<SkData> uniforms = SkData::MakeWithCopy(kUniforms, sizeof(kUniforms));

    return { blessed->makeShader(uniforms, /* children= */ {}),
             cursed->makeShader(uniforms, /* children= */ {}) };
}

// Draw once with a registered runtime effect, reset, and then re-draw w/ an un-registered
// runtime effect that uses the same SkSL.
void reset_and_test_sksl_reuse(skiatest::Reporter* reporter,
                               Context* context,
                               Recorder* recorder,
                               PipelineCallBackHandler* handler,
                               PrecompileContext* precompileContext) {
    GlobalCache* globalCache = precompileContext->priv().globalCache();

    auto [blessedShader, cursedShader] = make_annulus_shader_pair();

    // The blessed paint is the static one from the PrecompileFactories which has been
    // registered as a user-defined known runtime effect.
    SkPaint blessedPaint;
    blessedPaint.setShader(std::move(blessedShader));

    // The cursed paint uses the same SkSL as the blessed version but uses a wholly separate
    // runtime effect (which is not registered).
    SkPaint cursedPaint;
    cursedPaint.setShader(std::move(cursedShader));

    globalCache->resetGraphicsPipelines();
    handler->reset();

    draw_with_normal_api(context, recorder, blessedPaint);

    std::vector<skgpu::UniqueKey> origKeys;
    UniqueKeyUtils::FetchUniqueKeys(precompileContext, &origKeys);
    globalCache->resetGraphicsPipelines();
    REPORTER_ASSERT(reporter, origKeys.size() == 1);

    std::vector<sk_sp<SkData>> serializedKeys;
    handler->retrieve(&serializedKeys);
    handler->reset();
    REPORTER_ASSERT(reporter, serializedKeys.size() == 1);

    draw_with_normal_api(context, recorder, cursedPaint);

    {
        std::vector<skgpu::UniqueKey> recreatedKeys;
        UniqueKeyUtils::FetchUniqueKeys(precompileContext, &recreatedKeys);

        REPORTER_ASSERT(reporter, recreatedKeys.size() == 1);
        REPORTER_ASSERT(reporter, origKeys[0] == recreatedKeys[0]);
    }
    {
        std::vector<sk_sp<SkData>> recreatedSerializedKeys;
        handler->retrieve(&recreatedSerializedKeys);

        // The un-registered runtime effect should've been mapped back to the registered one
        // and successfully serialized.
        REPORTER_ASSERT(reporter, recreatedSerializedKeys.size() == 1);
        REPORTER_ASSERT(reporter, serializedKeys[0]->equals(recreatedSerializedKeys[0].get()));
    }
}

static const int kNumUserDefinedStableKeys = 7;

// This test adds some user-defined stably-keyed runtime effects and then verifies that
// everything behaves as expected. For the purposes of this test, "behaves as expected" means:
//    1) the user-defined stably keyed runtime effects appear as such in the ShaderCodeDictionary
//    2) the user-defined stable keys appear in the serialized Pipeline
//       keys (i.e., from the PipelineCallBackHandler)
//    3) said keys correctly (re)generate the desired pipelines
//    4) the normal (non-serialized-key) Precompile API generates the same keys.
//    5) if the blessed stable runtime-effects aren't used, the free-range runtime-effects are
//       mapped back to the stable runtime-effects (clients really shouldn't do this though!)
void run_test(skiatest::Reporter* reporter,
              Context* context,
              PipelineCallBackHandler* handler) {
    using namespace skiatest::graphite;

    std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();
    GlobalCache* globalCache = precompileContext->priv().globalCache();
    ShaderCodeDictionary* shaderCodeDictionary = context->priv().shaderCodeDictionary();

    REPORTER_ASSERT(reporter, !globalCache->numGraphicsPipelines());
    // The next two lines check #1 above
    REPORTER_ASSERT(reporter, !shaderCodeDictionary->numUserDefinedRuntimeEffects());
    REPORTER_ASSERT(reporter, shaderCodeDictionary->numUserDefinedKnownRuntimeEffects() ==
                              kNumUserDefinedStableKeys);

    std::unique_ptr<Recorder> recorder =
            context->makeRecorder(ToolUtils::CreateTestingRecorderOptions());
    REPORTER_ASSERT(reporter, recorder);

    auto [paint, paintOptions] = create_paint_and_options();

    // This block verifies #2 and #3 above (esp. the reset_and_recreate_pipelines method)
    {
        draw_with_normal_api(context, recorder.get(), paint);

        // None of the user-defined stable runtime effects should've been transmuted to un-stable
        REPORTER_ASSERT(reporter, !shaderCodeDictionary->numUserDefinedRuntimeEffects());

        reset_and_recreate_pipelines_with_serialized_keys(reporter, handler,
                                                          precompileContext.get());

        // None of the user-defined stable runtime effects should've been transmuted to un-stable
        REPORTER_ASSERT(reporter, !shaderCodeDictionary->numUserDefinedRuntimeEffects());

        int numBeforeSecondDraw = globalCache->numGraphicsPipelines();

        draw_with_normal_api(context, recorder.get(), paint);

        REPORTER_ASSERT(reporter, numBeforeSecondDraw == globalCache->numGraphicsPipelines());

        // None of the user-defined stable runtime effects should've been transmuted to un-stable
        REPORTER_ASSERT(reporter, !shaderCodeDictionary->numUserDefinedRuntimeEffects());
    }

    // This method tests out #4 above
    reset_and_recreate_pipelines_with_normal_api(reporter,
                                                 shaderCodeDictionary,
                                                 handler,
                                                 precompileContext.get(),
                                                 paintOptions);


    // This tests out #5 above
    reset_and_test_sksl_reuse(reporter, context, recorder.get(), handler, precompileContext.get());
}

} // anonymous namespace

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(UserDefinedStableKeyTest,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           testContext,
                                           origOptions,
                                           /* opt_filter= */ nullptr,
                                           /* cond= */ true,
                                           CtsEnforcement::kNever) {
    if (context->backend() != skgpu::BackendApi::kMetal &&
        context->backend() != skgpu::BackendApi::kVulkan) {
        // Only Native Metal and Vulkan configs currently support Pipeline key serialization
        return;
    }

    std::unique_ptr<PipelineCallBackHandler> pipelineHandler(new PipelineCallBackHandler);

    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fPipelineCallbackContext = pipelineHandler.get();
    newOptions.fContextOptions.fPipelineCallback = PipelineCallBackHandler::CallBack;

    // We're going to use all these runtime effects via the normal API
    // (c.f. create_paint_and_options)
    sk_sp<SkRuntimeEffect> userDefinedKnownRuntimeEffects[kNumUserDefinedStableKeys] = {
        sk_ref_sp(PrecompileFactories::GetAnnulusShaderEffect()),

        sk_ref_sp(PrecompileFactories::GetSrcBlenderEffect()),
        sk_ref_sp(PrecompileFactories::GetDstBlenderEffect()),
        sk_ref_sp(PrecompileFactories::GetComboBlenderEffect()),

        sk_ref_sp(PrecompileFactories::GetDoubleColorFilterEffect()),
        sk_ref_sp(PrecompileFactories::GetHalfColorFilterEffect()),
        sk_ref_sp(PrecompileFactories::GetComboColorFilterEffect()),
    };

    for (auto e: userDefinedKnownRuntimeEffects) {
        // The PrecompileFactories runtime effects are static so prior runs may have already
        // set their StableKeys. Reset the StableKeys so all our expectations/asserts will be met.
        SkRuntimeEffectPriv::ResetStableKey(e.get(), 0);
    }

    newOptions.fContextOptions.fUserDefinedKnownRuntimeEffects = { userDefinedKnownRuntimeEffects };

    ContextFactory workaroundFactory(newOptions);
    ContextInfo ctxInfo = workaroundFactory.getContextInfo(testContext->contextType());
    run_test(reporter,
             ctxInfo.fContext,
             pipelineHandler.get());
}


// Test that the ShaderCodeDictionary can deduplicate the user-defined known runtime effect list
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(UserDefinedStableKeyTest_Duplicates,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           testContext,
                                           origOptions,
                                           /* opt_filter= */ nullptr,
                                           /* cond= */ true,
                                           CtsEnforcement::kNever) {
    if (context->backend() != skgpu::BackendApi::kMetal &&
        context->backend() != skgpu::BackendApi::kVulkan) {
        // Only Native Metal and Vulkan configs currently support Pipeline key serialization
        return;
    }

    std::unique_ptr<PipelineCallBackHandler> pipelineHandler(new PipelineCallBackHandler);

    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fPipelineCallbackContext = pipelineHandler.get();
    newOptions.fContextOptions.fPipelineCallback = PipelineCallBackHandler::CallBack;

    sk_sp<SkRuntimeEffect> userDefinedKnownRuntimeEffects[] = {
            sk_ref_sp(PrecompileFactories::GetAnnulusShaderEffect()),
            sk_ref_sp(PrecompileFactories::GetAnnulusShaderEffect()),
    };

    for (auto e: userDefinedKnownRuntimeEffects) {
        // The PrecompileFactories runtime effects are static so prior runs may have already
        // set their StableKeys. Reset the StableKeys so all our expectations/asserts will be met.
        SkRuntimeEffectPriv::ResetStableKey(e.get(), 0);
    }

    newOptions.fContextOptions.fUserDefinedKnownRuntimeEffects = { userDefinedKnownRuntimeEffects };

    ContextFactory workaroundFactory(newOptions);
    ContextInfo ctxInfo = workaroundFactory.getContextInfo(testContext->contextType());

    ShaderCodeDictionary* shaderCodeDictionary = ctxInfo.fContext->priv().shaderCodeDictionary();

    REPORTER_ASSERT(reporter, shaderCodeDictionary->numUserDefinedKnownRuntimeEffects() == 1);
}

// Test that the ShaderCodeDictionary can handle nullptrs in the
// user-defined known runtime effect list
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(UserDefinedStableKeyTest_Nullptrs,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           testContext,
                                           origOptions,
                                           /* opt_filter= */ nullptr,
                                           /* cond= */ true,
                                           CtsEnforcement::kNever) {
    if (context->backend() != skgpu::BackendApi::kMetal &&
        context->backend() != skgpu::BackendApi::kVulkan) {
        // Only Native Metal and Vulkan configs currently support Pipeline key serialization
        return;
    }

    std::unique_ptr<PipelineCallBackHandler> pipelineHandler(new PipelineCallBackHandler);

    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fPipelineCallbackContext = pipelineHandler.get();
    newOptions.fContextOptions.fPipelineCallback = PipelineCallBackHandler::CallBack;

    sk_sp<SkRuntimeEffect> userDefinedKnownRuntimeEffects[] = {
            sk_ref_sp(PrecompileFactories::GetAnnulusShaderEffect()),
            nullptr,
            sk_ref_sp(PrecompileFactories::GetSrcBlenderEffect()),
    };

    for (auto e : userDefinedKnownRuntimeEffects) {
        // The PrecompileFactories runtime effects are static so prior runs may have already
        // set their StableKeys. Reset the StableKeys so all our expectations/asserts will be met.
        if (e) {
            SkRuntimeEffectPriv::ResetStableKey(e.get(), 0);
        }
    }

    newOptions.fContextOptions.fUserDefinedKnownRuntimeEffects = { userDefinedKnownRuntimeEffects };

    ContextFactory workaroundFactory(newOptions);
    ContextInfo ctxInfo = workaroundFactory.getContextInfo(testContext->contextType());

    ShaderCodeDictionary* shaderCodeDictionary = ctxInfo.fContext->priv().shaderCodeDictionary();

    REPORTER_ASSERT(reporter, shaderCodeDictionary->numUserDefinedKnownRuntimeEffects() == 2);
}

// Test that the ShaderCodeDictionary can handle excess user-defined known runtime effects
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(UserDefinedStableKeyTest_Overflow,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           testContext,
                                           origOptions,
                                           /* opt_filter= */ nullptr,
                                           /* cond= */ true,
                                           CtsEnforcement::kNever) {
    if (context->backend() != skgpu::BackendApi::kMetal &&
        context->backend() != skgpu::BackendApi::kVulkan) {
        // Only Native Metal and Vulkan configs currently support Pipeline key serialization
        return;
    }

    std::unique_ptr<PipelineCallBackHandler> pipelineHandler(new PipelineCallBackHandler);

    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fPipelineCallbackContext = pipelineHandler.get();
    newOptions.fContextOptions.fPipelineCallback = PipelineCallBackHandler::CallBack;

    std::vector<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects;

    for (int i = 0; i < 2*SkKnownRuntimeEffects::kUserDefinedKnownRuntimeEffectsReservedCnt; ++i) {
        SkString sksl;
        sksl.printf("half4 main(float2 xy) { return half4(%d/255.0, %d/255.0, %d/255.0, 1.0); }",
                    i, i, i);
        userDefinedKnownRuntimeEffects.push_back(SkRuntimeEffect::MakeForShader(sksl).effect);
    }

    newOptions.fContextOptions.fUserDefinedKnownRuntimeEffects = { userDefinedKnownRuntimeEffects };

    ContextFactory workaroundFactory(newOptions);
    ContextInfo ctxInfo = workaroundFactory.getContextInfo(testContext->contextType());

    ShaderCodeDictionary* shaderCodeDictionary = ctxInfo.fContext->priv().shaderCodeDictionary();

    REPORTER_ASSERT(reporter, shaderCodeDictionary->numUserDefinedKnownRuntimeEffects() ==
                              SkKnownRuntimeEffects::kUserDefinedKnownRuntimeEffectsReservedCnt);
}

#endif // SK_GRAPHITE
