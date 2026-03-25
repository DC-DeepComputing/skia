/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/MaskFormat.h"
#include "src/gpu/graphite/DrawAtlas.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/text/TextAtlasManager.h"

#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

using namespace skgpu::graphite;
using MaskFormat = skgpu::MaskFormat;

namespace skgpu::graphite {
struct DrawAtlasTest {
    static constexpr auto kPlotRecentlyUsedCount       = DrawAtlas::kPlotRecentlyUsedCount;
    static constexpr auto kCompactionRecentlyUsedCount = DrawAtlas::kCompactionRecentlyUsedCount;
};
}

namespace {
constexpr auto kPlotRecentlyUsedCount       = DrawAtlasTest::kPlotRecentlyUsedCount;
constexpr auto kCompactionRecentlyUsedCount = DrawAtlasTest::kCompactionRecentlyUsedCount;

const int kNumPlots = 2;
const int kPlotSize = 32;
const int kAtlasSize = kNumPlots * kPlotSize;
int gEvictCount = 0;

class EvictionCounter : public DrawAtlas::EvictionCallback {
public:
    void evict(const DrawAtlas::RecordVisitor&) override { ++gEvictCount; }
    void relocate(SkIVector translation, const DrawAtlas::RecordVisitor& visitor) override {}
};

void check(skiatest::Reporter* r, int expectedEvictCount) {
    REPORTER_ASSERT(r, gEvictCount == expectedEvictCount);
}

// Simulates a flush: processes pending uploads, advances the flush token, and compacts.
void simulateFlush(DrawAtlas* atlas, Recorder* recorder) {
    atlas->clearPendingUploads();
    recorder->priv().issueFlushToken();
    atlas->compact(recorder);
}

// Simulates a flush then frees GPU resources (deactivates the proxy if nothing is in-flight).
void simulateFlushAndFree(DrawAtlas* atlas, Recorder* recorder) {
    atlas->clearPendingUploads();
    recorder->priv().issueFlushToken();
    atlas->freeGpuResources(recorder->priv().tokenTracker()->nextFlushToken());
}

bool fill_plot(DrawAtlas* atlas, Recorder* recorder, DrawAtlas::AddResult* addResult, int alpha) {
    SkImageInfo ii = SkImageInfo::MakeA8(kPlotSize, kPlotSize);

    SkBitmap data;
    data.allocPixels(ii);
    data.eraseARGB(alpha, 0, 0, 0);

    DrawAtlas::ErrorCode code;
    code = atlas->addRect(recorder,
                          {kPlotSize, kPlotSize},
                          addResult,
                          static_cast<std::byte*>(data.getAddr(0, 0)));
    return code == DrawAtlas::ErrorCode::kSucceeded;
}
}  // anonymous namespace

// This is a basic DrawAtlas test. It verifies that the atlas correctly fills plots, grows its
// texture when full, compacts stale plots, and frees GPU resources.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(BasicDrawAtlas,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    constexpr int kInitialPlots = kNumPlots * kNumPlots;  // 4

    gEvictCount = 0;
    EvictionCounter evictor;
    std::unique_ptr<DrawAtlas> atlas = DrawAtlas::Make(MaskFormat::kA8,
                                                       {{kPlotSize, kPlotSize}, {kNumPlots, kNumPlots}},
                                                       /*maxTextureSize=*/1 << 16,
                                                       DrawAtlas::UseStorageTextures::kNo,
                                                       &evictor,
                                                       /*label=*/"BasicDrawAtlasTest");
    check(reporter, /*expectedEvictCount=*/0);

    // Fill all 4 initial plots and mark them as in-use for the current flush.
    DrawAtlas::AddResult addResult;
    std::vector<DrawAtlas::Record> records;
    for (int i = 0; i < kInitialPlots; ++i) {
        REPORTER_ASSERT(reporter, fill_plot(atlas.get(), recorder.get(), &addResult, i * 32));
        records.push_back(addResult.fRecord);
        atlas->setLastUseToken(records.back(), recorder->priv().tokenTracker()->nextFlushToken());
    }
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == kInitialPlots);
    check(reporter, /*expectedEvictCount=*/0);

    // Adding a 5th plot should succeed by growing the texture.
    REPORTER_ASSERT(reporter, fill_plot(atlas.get(), recorder.get(), &addResult, 0xff));
    records.push_back(addResult.fRecord);
    atlas->setLastUseToken(records.back(), recorder->priv().tokenTracker()->nextFlushToken());
    check(reporter, /*expectedEvictCount=*/0);
    // Atlas grew, but only the filled plots have backing data.
    REPORTER_ASSERT(reporter, atlas->numNonEmptyPlots() == kInitialPlots + 1);

    // Use only the first plot for many flushes. The other plots should become stale
    // and be recycled by compact() after kPlotRecentlyUsedCount flushes.
    // We need +2 because the initial compact before this loop counts all plots as "recently
    // used" (their tokens fall in the initial interval), consuming one iteration.
    for (int i = 0; i < kPlotRecentlyUsedCount + 2; ++i) {
        atlas->setLastUseToken(records[0], recorder->priv().tokenTracker()->nextFlushToken());
        simulateFlush(atlas.get(), recorder.get());
    }
    // The 4 other non-empty plots were recycled (evicted). The 3 empty grown plots don't
    // generate eviction callbacks.
    check(reporter, /*expectedEvictCount=*/kInitialPlots);
    REPORTER_ASSERT(reporter, atlas->numNonEmptyPlots() == 1);

    // freeGpuResources while one plot is still in-use should not free.
    atlas->setLastUseToken(records[0], recorder->priv().tokenTracker()->nextFlushToken());
    atlas->freeGpuResources(recorder->priv().tokenTracker()->nextFlushToken());
    REPORTER_ASSERT(reporter, atlas->getProxy());

    // After a flush with no further use, freeGpuResources should deactivate everything.
    simulateFlush(atlas.get(), recorder.get());
    simulateFlushAndFree(atlas.get(), recorder.get());
    // The 1 remaining non-empty plot is evicted during deactivation.
    check(reporter, /*expectedEvictCount=*/kInitialPlots + 1);
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == 0);
    REPORTER_ASSERT(reporter, !atlas->getProxy());
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(ThrashDrawAtlasCache,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    EvictionCounter evictor;

    constexpr int numPlots = 2;
    constexpr int plotSize = 32;
    constexpr int initialPlots = numPlots * numPlots;  // 4

    std::unique_ptr<DrawAtlas> atlas = DrawAtlas::Make(MaskFormat::kA8,
                                                       {{plotSize, plotSize}, {numPlots, numPlots}},
                                                       /*maxTextureSize=*/1 << 16,
                                                       DrawAtlas::UseStorageTextures::kNo,
                                                       &evictor,
                                                       /*label=*/"ThrashDrawAtlasTest");

    // Fill all initial plots and mark them as in-use.
    gEvictCount = 0;
    std::vector<DrawAtlas::Record> records;
    for (int i = 0; i < initialPlots; ++i) {
        DrawAtlas::AddResult addResult;
        REPORTER_ASSERT(reporter, fill_plot(atlas.get(), recorder.get(), &addResult, i));
        records.push_back(addResult.fRecord);
        atlas->setLastUseToken(records.back(), recorder->priv().tokenTracker()->nextFlushToken());
    }
    check(reporter, /*expectedEvictCount=*/0);
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == initialPlots);

    // Adding when full should grow the atlas, even with in-flight plots.
    DrawAtlas::AddResult addResult;
    REPORTER_ASSERT(reporter, fill_plot(atlas.get(), recorder.get(), &addResult, 0xff));
    records.push_back(addResult.fRecord);
    atlas->setLastUseToken(records.back(), recorder->priv().tokenTracker()->nextFlushToken());
    check(reporter, /*expectedEvictCount=*/0);
    // Atlas grew, but only the filled plots have backing data.
    REPORTER_ASSERT(reporter, atlas->numNonEmptyPlots() == initialPlots + 1);

    // Flush to settle state, then test partial compaction with only one plot in use.
    simulateFlush(atlas.get(), recorder.get());

    // Use only one plot repeatedly, making the others stale.
    // +2 because the initial flush above counts all plots as "recently used".
    for (int i = 0; i < kPlotRecentlyUsedCount + 2; ++i) {
        atlas->setLastUseToken(records[0], recorder->priv().tokenTracker()->nextFlushToken());
        simulateFlush(atlas.get(), recorder.get());
    }

    // The other non-empty plots (initialPlots + 1 filled, minus 1 kept alive) should be recycled.
    check(reporter, /*expectedEvictCount=*/initialPlots);
    REPORTER_ASSERT(reporter, atlas->numNonEmptyPlots() == 1);

    // freeGpuResources after flushing: deactivates the proxy and evicts the last non-empty plot.
    simulateFlushAndFree(atlas.get(), recorder.get());
    check(reporter, /*expectedEvictCount=*/initialPlots + 1);
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == 0);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(DrawAtlasProxyLifetime,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    gEvictCount = 0;
    auto recorder = context->makeRecorder();
    EvictionCounter evictor;
    constexpr int kTotalPlots = kNumPlots * kNumPlots;
    std::unique_ptr<DrawAtlas> atlas = DrawAtlas::Make(MaskFormat::kA8,
                                                       {{kPlotSize, kPlotSize}, {kNumPlots, kNumPlots}},
                                                       /*maxTextureSize=*/1 << 16,
                                                       DrawAtlas::UseStorageTextures::kNo,
                                                       &evictor,
                                                       /*label=*/"DrawAtlasProxyTest");

    // No proxy before first use.
    REPORTER_ASSERT(reporter, !atlas->getProxy());

    // Fill a plot to trigger proxy creation and grab a ref to it.
    DrawAtlas::AddResult addResult;
    DrawAtlas::Record records[kTotalPlots];
    REPORTER_ASSERT(reporter, fill_plot(atlas.get(), recorder.get(), &addResult, 0));
    records[0] = addResult.fRecord;

    sk_sp<TextureProxy> proxy = atlas->getProxy();
    REPORTER_ASSERT(reporter, proxy);
    // Atlas also holds a ref, so not unique.
    REPORTER_ASSERT(reporter, !proxy->unique());

    // Fill the remaining plots.
    for (int i = 1; i < kTotalPlots; ++i) {
        REPORTER_ASSERT(reporter, fill_plot(atlas.get(), recorder.get(), &addResult, i));
        records[i] = addResult.fRecord;
    }
    // Still the same proxy.
    REPORTER_ASSERT(reporter, atlas->getProxy() == proxy);

    // freeGpuResources while plots are in-flight should not release the proxy.
    for (int i = 0; i < kTotalPlots; ++i) {
        atlas->setLastUseToken(records[i], recorder->priv().tokenTracker()->nextFlushToken());
    }
    atlas->freeGpuResources(recorder->priv().tokenTracker()->nextFlushToken());
    REPORTER_ASSERT(reporter, !proxy->unique());

    // After a flush and compact with no further use, freeGpuResources releases the proxy.
    simulateFlush(atlas.get(), recorder.get());
    simulateFlushAndFree(atlas.get(), recorder.get());
    // Our ref is now the only one.
    REPORTER_ASSERT(reporter, proxy->unique());
    REPORTER_ASSERT(reporter, !atlas->getProxy());

    // Adding a new plot creates a new proxy.
    REPORTER_ASSERT(reporter, fill_plot(atlas.get(), recorder.get(), &addResult, 42));
    sk_sp<TextureProxy> proxy2 = atlas->getProxy();
    REPORTER_ASSERT(reporter, proxy2);
    REPORTER_ASSERT(reporter, proxy2 != proxy);

    // Clean up.
    simulateFlushAndFree(atlas.get(), recorder.get());
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == 0);
}

// Verifies that the atlas shrinks its texture after sustained low usage.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(DrawAtlasShrink,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    constexpr int kInitialPlots = kNumPlots * kNumPlots;  // 4

    gEvictCount = 0;
    EvictionCounter evictor;
    std::unique_ptr<DrawAtlas> atlas = DrawAtlas::Make(MaskFormat::kA8,
                                                       {{kPlotSize, kPlotSize}, {kNumPlots, kNumPlots}},
                                                       /*maxTextureSize=*/1 << 16,
                                                       DrawAtlas::UseStorageTextures::kNo,
                                                       &evictor,
                                                       /*label=*/"DrawAtlasShrinkTest");

    // Fill all initial plots, mark in-use, then grow by adding one more.
    DrawAtlas::AddResult addResult;
    std::vector<DrawAtlas::Record> records;
    for (int i = 0; i < kInitialPlots + 1; ++i) {
        REPORTER_ASSERT(reporter, fill_plot(atlas.get(), recorder.get(), &addResult, i));
        records.push_back(addResult.fRecord);
        atlas->setLastUseToken(records.back(), recorder->priv().tokenTracker()->nextFlushToken());
    }
    // Atlas grew from 64x64 to 64x128 (2x4 grid = 8 plots).
    sk_sp<TextureProxy> grownProxy = atlas->getProxy();
    REPORTER_ASSERT(reporter, grownProxy);
    REPORTER_ASSERT(reporter, grownProxy->dimensions().width() == kAtlasSize);
    REPORTER_ASSERT(reporter, grownProxy->dimensions().height() == kAtlasSize * 2);

    // Flush to settle, then use only one plot for long enough to trigger both plot recycling
    // and texture compaction.
    simulateFlush(atlas.get(), recorder.get());

    // We need:
    //   kPlotRecentlyUsedCount + 1 flushes for stale plots to be recycled
    //   (at which point the compaction timer starts)
    //   kCompactionRecentlyUsedCount + 1 more flushes for the shrink to trigger
    int totalFlushes = (kPlotRecentlyUsedCount + 1) + (kCompactionRecentlyUsedCount + 1);
    for (int i = 0; i < totalFlushes; ++i) {
        atlas->setLastUseToken(records[0], recorder->priv().tokenTracker()->nextFlushToken());
        simulateFlush(atlas.get(), recorder.get());
    }

    // The proxy should have shrunk back to the initial size.
    REPORTER_ASSERT(reporter, atlas->getProxy());
    REPORTER_ASSERT(reporter, atlas->getProxy() != grownProxy);
    REPORTER_ASSERT(reporter, atlas->getProxy()->dimensions().width() == kAtlasSize);
    REPORTER_ASSERT(reporter, atlas->getProxy()->dimensions().height() == kAtlasSize);

    // The kept-alive plot should still be present.
    REPORTER_ASSERT(reporter, atlas->numNonEmptyPlots() == 1);

    // Clean up.
    simulateFlushAndFree(atlas.get(), recorder.get());
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////

namespace {
void test_draw_atlas_config(skiatest::Reporter* reporter,
                            size_t maxBytes,
                            MaskFormat maskFormat,
                            SkISize expectedPlotSize,
                            SkISize expectedMinPlotGridDims) {
    TextAtlasManager::AtlasConfig atlasConfig(maxBytes);
    auto config = atlasConfig.config(maskFormat);
    REPORTER_ASSERT(reporter, config.fPlotSize == expectedPlotSize);
    REPORTER_ASSERT(reporter, config.fMinPlotGridDims == expectedMinPlotGridDims);
}
}  // anonymous namespace

DEF_GRAPHITE_TEST(DrawAtlasConfig_Basic, reporter, CtsEnforcement::kNever) {
    // 1/4 MB
    test_draw_atlas_config(reporter, 256 * 1024, MaskFormat::kARGB, {256, 256}, {1, 1});
    test_draw_atlas_config(reporter, 256 * 1024, MaskFormat::kA8,   {256, 256}, {2, 2});
    // 1/2 MB
    test_draw_atlas_config(reporter, 512 * 1024, MaskFormat::kARGB, {256, 256}, {2, 1});
    test_draw_atlas_config(reporter, 512 * 1024, MaskFormat::kA8,   {256, 256}, {4, 2});
    // 1 MB
    test_draw_atlas_config(reporter, 1024 * 1024, MaskFormat::kARGB, {256, 256}, {2, 2});
    test_draw_atlas_config(reporter, 1024 * 1024, MaskFormat::kA8,   {256, 256}, {4, 4});
    // 2 MB
    test_draw_atlas_config(reporter, 2 * 1024 * 1024, MaskFormat::kARGB, {256, 256}, {4, 2});
    test_draw_atlas_config(reporter, 2 * 1024 * 1024, MaskFormat::kA8,   {512, 256}, {4, 4});
    // 4 MB
    test_draw_atlas_config(reporter, 4 * 1024 * 1024, MaskFormat::kARGB, {256, 256}, {4, 4});
    test_draw_atlas_config(reporter, 4 * 1024 * 1024, MaskFormat::kA8,   {512, 512}, {4, 4});
    // 8 MB
    test_draw_atlas_config(reporter, 8 * 1024 * 1024, MaskFormat::kARGB, {256, 256}, {8, 4});
    test_draw_atlas_config(reporter, 8 * 1024 * 1024, MaskFormat::kA8,   {512, 512}, {8, 4});
    // 16 MB (should be same as 8 MB)
    test_draw_atlas_config(reporter, 16 * 1024 * 1024, MaskFormat::kARGB, {256, 256}, {8, 4});
    test_draw_atlas_config(reporter, 16 * 1024 * 1024, MaskFormat::kA8,   {512, 512}, {8, 4});

    // 3 MB (should be same as 2 MB)
    test_draw_atlas_config(reporter, 3 * 1024 * 1024, MaskFormat::kARGB, {256, 256}, {4, 2});
    test_draw_atlas_config(reporter, 3 * 1024 * 1024, MaskFormat::kA8,   {512, 256}, {4, 4});

    // minimum size
    test_draw_atlas_config(reporter, 0, MaskFormat::kARGB, {256, 256}, {1, 1});
    test_draw_atlas_config(reporter, 0, MaskFormat::kA8,   {256, 256}, {2, 2});
}
