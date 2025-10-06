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
#include "tools/graphite/GraphiteMemoryPipelineStorage.h"
#include "tools/graphite/TestOptions.h"

// Persistent Cache testing requires a working Context::canDetectNewPipelineCacheData
// implementation. This can only be reliably found on Android devices. VMs (i.e., SwiftShader) often
// claim to support the Vulkan feature but do not actually do so.
//#if defined(SK_BUILD_FOR_ANDROID)

using namespace skiatest::graphite;

namespace {

bool draw(GraphiteTestContext* origTestContext,
          const TestOptions& origOptions,
          sk_gpu_test::GraphiteMemoryPipelineStorage* memoryPipelineStorage) {

    // Rebuild the Context with a PersistentCache
    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fPersistentPipelineStorage = memoryPipelineStorage;

    skiatest::graphite::ContextFactory workaroundFactory(newOptions);
    ContextInfo ctxInfo = workaroundFactory.getContextInfo(origTestContext->contextType());

    skgpu::graphite::Context* newContext = ctxInfo.fContext;
    GraphiteTestContext* newTestContext = ctxInfo.fTestContext;

    std::unique_ptr<skgpu::graphite::Recorder> recorder = newContext->makeRecorder();
    if (!recorder) {
        return false;
    }

    {
        SkSurfaceProps props(0, kRGB_H_SkPixelGeometry);
        auto ii = SkImageInfo::Make(64, 64, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

        sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii,
                                                            skgpu::Mipmapped::kNo, &props);
        if (!surface) {
            return false;
        }

        SkPaint paint;
        surface->getCanvas()->drawRect({16, 16, 48, 48 }, paint);
    }

    std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
    if (!recording) {
        return false;
    }

    skgpu::graphite::InsertRecordingInfo info;
    info.fRecording = recording.get();
    if (!newContext->insertRecording(info)) {
        return false;
    }
    newTestContext->syncedSubmit(newContext);

    newContext->syncPipelineData();

    return true;
}

} // anonymous namespace

namespace skgpu::graphite {

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(PersistentStorageTest,
                                           skiatest::IsVulkanContextType,
                                           reporter,
                                           origContext,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNextRelease) {

    sk_gpu_test::GraphiteMemoryPipelineStorage memoryPipelineStorage;

    // Draw twice, once with a cold cache, and again with a warm cache. Verify that the
    // pipelines from the warmed cache are found and used - so no new pipelines
    // are created.
    REPORTER_ASSERT(reporter, draw(origTestContext, origOptions, &memoryPipelineStorage));

    memoryPipelineStorage.resetCacheStats();

    REPORTER_ASSERT(reporter, draw(origTestContext, origOptions, &memoryPipelineStorage));

    REPORTER_ASSERT(reporter, !memoryPipelineStorage.numLoads());
    REPORTER_ASSERT(reporter, !memoryPipelineStorage.numStores());
}

}  // namespace skgpu::graphite

//#endif // SK_BUILD_FOR_ANDROID
