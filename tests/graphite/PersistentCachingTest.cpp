/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurfaceProps.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Surface.h"
#include "tools/graphite/ContextFactory.h"
#include "tools/graphite/GraphiteMemoryCache.h"
#include "tools/graphite/TestOptions.h"

// Persistent Cache testing requires a working Context::canDetectNewVkPipelineCacheData
// implementation. This can only be reliably found on Android devices. VMs often claim to
// support the Vulkan feature but do not in actuality.
//#if defined(SK_BUILD_FOR_ANDROID)

using namespace skiatest::graphite;

namespace {

void draw(skgpu::graphite::Context* context, GraphiteTestContext* testContext) {

    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();
    if (!recorder) {
        return; // Result::Fatal("Could not create a recorder.");
    }

    {
        SkSurfaceProps props(0, kRGB_H_SkPixelGeometry);
        auto ii = SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

        sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii,
                                                            skgpu::Mipmapped::kNo, &props);
        if (!surface) {
            return; // Result::Fatal("Could not create a surface.");
        }

        SkPaint paint;
        surface->getCanvas()->drawRect({0, 0, 100, 100}, paint);
    }

    std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
    if (!recording) {
        return; // Result::Fatal("Could not create a recording.");
    }

    skgpu::graphite::InsertRecordingInfo info;
    info.fRecording = recording.get();
    if (!context->insertRecording(info)) {
        return; // Result::Fatal("Context::insertRecording failed.");
    }
    testContext->syncedSubmit(context);

    if (context->hasNewVkPipelineCacheData()) {
        context->storeVkPipelineCacheData();
    }
}

} // anonymous namespace

namespace skgpu::graphite {

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(PersistentCachingTest,
                                           skiatest::IsVulkanContextType,
                                           reporter,
                                           origContext,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNextRelease) {

    if (!origContext->canDetectNewVkPipelineCacheData()) {
        return;
    }

    sk_gpu_test::GraphiteMemoryCache memoryCache;

    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fPersistentCache = &memoryCache;

    skiatest::graphite::ContextFactory workaroundFactory(newOptions);
    ContextInfo ctxInfo = workaroundFactory.getContextInfo(origTestContext->contextType());

    Context* newContext = ctxInfo.fContext;
    GraphiteTestContext* newTestContext = ctxInfo.fTestContext;

    // Draw twice, once with a cold cache, and again with a warm cache. Verify that the
    // pipelines from the warmed cache are found and used - so no new pipelines
    // are created.
    memoryCache.reset();

    draw(newContext, newTestContext);

    SkDebugf("pass 1: misses %d stores %d\n",
             memoryCache.numCacheMisses(),
             memoryCache.numCacheStores());

    draw(newContext, newTestContext);

    SkDebugf("pass 2: misses %d stores %d\n",
             memoryCache.numCacheMisses(),
             memoryCache.numCacheStores());

    REPORTER_ASSERT(reporter, !memoryCache.numCacheMisses());
    REPORTER_ASSERT(reporter, !memoryCache.numCacheStores());
}

}  // namespace skgpu::graphite

//#endif // SK_BUILD_FOR_ANDROID
