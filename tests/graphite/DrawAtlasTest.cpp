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
    static constexpr auto kPlotRecentlyUsedCount = DrawAtlas::kPlotRecentlyUsedCount;
    static constexpr auto kAtlasRecentlyUsedCount = DrawAtlas::kAtlasRecentlyUsedCount;
};
}

namespace {
constexpr auto kPlotRecentlyUsedCount = DrawAtlasTest::kPlotRecentlyUsedCount;
constexpr auto kAtlasRecentlyUsedCount = DrawAtlasTest::kAtlasRecentlyUsedCount;

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

// This is a basic DrawAtlas test. It verifies that the atlas correctly fills the plots,
// evicts the LRU plot when full, compacts stale plots, and frees GPU resources.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(BasicDrawAtlas,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    constexpr int kTotalPlots = kNumPlots * kNumPlots;

    gEvictCount = 0;
    EvictionCounter evictor;
    std::unique_ptr<DrawAtlas> atlas = DrawAtlas::Make(MaskFormat::kA8,
                                                       kAtlasSize, kAtlasSize,
                                                       kPlotSize, kPlotSize,
                                                       DrawAtlas::UseStorageTextures::kNo,
                                                       &evictor,
                                                       /*label=*/"BasicDrawAtlasTest");
    check(reporter, /*expectedEvictCount=*/0);

    // Fill all 4 plots and mark them as in-use for the current flush.
    DrawAtlas::AddResult addResult;
    DrawAtlas::Record records[kTotalPlots];
    for (int i = 0; i < kTotalPlots; ++i) {
        bool result = fill_plot(atlas.get(), recorder.get(), &addResult, i * 32);
        REPORTER_ASSERT(reporter, result);
        records[i] = addResult.fRecord;
        atlas->setLastUseToken(records[i], recorder->priv().tokenTracker()->nextFlushToken());
    }
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == kTotalPlots);
    check(reporter, /*expectedEvictCount=*/0);

    // All plots are in-flight (lastUseToken == nextFlushToken), so a 5th add must fail.
    REPORTER_ASSERT(reporter, !fill_plot(atlas.get(), recorder.get(), &addResult, 0xff));

    // After a flush, the plots become evictable.
    recorder->priv().issueFlushToken();
    atlas->compact(recorder->priv().tokenTracker()->nextFlushToken());

    // Now the 5th add should succeed by evicting the LRU plot.
    REPORTER_ASSERT(reporter, fill_plot(atlas.get(), recorder.get(), &addResult, 0xff));
    check(reporter, /*expectedEvictCount=*/1);
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == kTotalPlots);
    DrawAtlas::Record keepAliveRecord = addResult.fRecord;

    // Use only the newly added plot for many flushes. The other 3 plots should become stale
    // and be recycled by compact() after kPlotRecentlyUsedCount flushes.
    for (int i = 0; i < kPlotRecentlyUsedCount + 1; ++i) {
        atlas->setLastUseToken(keepAliveRecord,
                               recorder->priv().tokenTracker()->nextFlushToken());
        recorder->priv().issueFlushToken();
        atlas->compact(recorder->priv().tokenTracker()->nextFlushToken());
    }
    // 3 stale plots were recycled (evicted).
    check(reporter, /*expectedEvictCount=*/1 + 3);
    REPORTER_ASSERT(reporter, atlas->numNonEmptyPlots() == 1);
    // compact recycles with kReuse_KeepData, so allocations are retained.
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == kTotalPlots);

    // freeGpuResources while one plot is still in-use should not free.
    atlas->setLastUseToken(keepAliveRecord,
                           recorder->priv().tokenTracker()->nextFlushToken());
    atlas->freeGpuResources(recorder->priv().tokenTracker()->nextFlushToken());
    REPORTER_ASSERT(reporter, atlas->getProxy());

    // After a flush with no further use, freeGpuResources should deactivate everything.
    recorder->priv().issueFlushToken();
    atlas->compact(recorder->priv().tokenTracker()->nextFlushToken());
    recorder->priv().issueFlushToken();
    atlas->freeGpuResources(recorder->priv().tokenTracker()->nextFlushToken());
    // The 1 remaining non-empty plot is evicted during deactivation.
    check(reporter, /*expectedEvictCount=*/1 + 3 + 1);
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
    constexpr int atlasSize = numPlots * plotSize;
    constexpr int totalPlots = numPlots * numPlots;

    std::unique_ptr<DrawAtlas> atlas = DrawAtlas::Make(MaskFormat::kA8,
                                                       atlasSize,
                                                       atlasSize,
                                                       plotSize,
                                                       plotSize,
                                                       DrawAtlas::UseStorageTextures::kNo,
                                                       &evictor,
                                                       /*label=*/"ThrashDrawAtlasTest");
    std::vector<DrawAtlas::Record> records(totalPlots);

    // Test kTryAgain failure and recovery.
    // Fill all plots and mark them as in-use for the current flush.
    gEvictCount = 0;
    for (int i = 0; i < totalPlots; ++i) {
        DrawAtlas::AddResult addResult;
        REPORTER_ASSERT(reporter, fill_plot(atlas.get(), recorder.get(), &addResult, i));
        records[i] = addResult.fRecord;
        atlas->setLastUseToken(records[i], recorder->priv().tokenTracker()->nextFlushToken());
    }
    check(reporter, /*expectedEvictCount=*/0);
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == totalPlots);

    // Try to add one more. All plots are in-flight, so this must fail.
    DrawAtlas::AddResult addResult;
    REPORTER_ASSERT(reporter, !fill_plot(atlas.get(), recorder.get(), &addResult, 0xff));

    // Now, simulate a flush. This makes the plots evictable.
    recorder->priv().issueFlushToken();
    atlas->compact(recorder->priv().tokenTracker()->nextFlushToken());

    // Try adding again. It should succeed by evicting the least recently used plot.
    REPORTER_ASSERT(reporter, fill_plot(atlas.get(), recorder.get(), &addResult, 0xff));
    check(reporter, /*expectedEvictCount=*/1);
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == totalPlots);

    // evictAllPlots should evict all plots (including the newly added one).
    gEvictCount = 0;
    atlas->evictAllPlots();
    REPORTER_ASSERT(reporter, gEvictCount == totalPlots);
    // evictAllPlots should free their backing storage.
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == 0);

    // Test partial compaction: refill, then use only one plot.
    gEvictCount = 0;
    for (int i = 0; i < totalPlots; ++i) {
        REPORTER_ASSERT(reporter, fill_plot(atlas.get(), recorder.get(), &addResult, i));
        records[i] = addResult.fRecord;
        atlas->setLastUseToken(records[i], recorder->priv().tokenTracker()->nextFlushToken());
    }
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == totalPlots);
    recorder->priv().issueFlushToken();
    atlas->compact(recorder->priv().tokenTracker()->nextFlushToken());

    // Use only one plot repeatedly, making the other 3 stale.
    // Loop one more than kPlotRecentlyUsedCount to ensure eviction.
    for (int i = 0; i < kPlotRecentlyUsedCount + 1; ++i) {
        atlas->setLastUseToken(records[0], recorder->priv().tokenTracker()->nextFlushToken());
        recorder->priv().issueFlushToken();
        atlas->compact(recorder->priv().tokenTracker()->nextFlushToken());
    }

    // The other 3 plots should have been recycled. compact uses kReuse_KeepData so the
    // backing data is retained but the plots are empty.
    check(reporter, /*expectedEvictCount=*/totalPlots - 1);
    REPORTER_ASSERT(reporter, atlas->numNonEmptyPlots() == 1);

    // freeGpuResources after flushing: deactivates the proxy and evicts the last non-empty plot.
    recorder->priv().issueFlushToken();
    atlas->freeGpuResources(recorder->priv().tokenTracker()->nextFlushToken());
    check(reporter, /*expectedEvictCount=*/totalPlots);
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
                                                       kAtlasSize,
                                                       kAtlasSize,
                                                       kPlotSize,
                                                       kPlotSize,
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
    recorder->priv().issueFlushToken();
    atlas->compact(recorder->priv().tokenTracker()->nextFlushToken());
    recorder->priv().issueFlushToken();
    atlas->freeGpuResources(recorder->priv().tokenTracker()->nextFlushToken());
    // Our ref is now the only one.
    REPORTER_ASSERT(reporter, proxy->unique());
    REPORTER_ASSERT(reporter, !atlas->getProxy());

    // Adding a new plot creates a new proxy.
    REPORTER_ASSERT(reporter, fill_plot(atlas.get(), recorder.get(), &addResult, 42));
    sk_sp<TextureProxy> proxy2 = atlas->getProxy();
    REPORTER_ASSERT(reporter, proxy2);
    REPORTER_ASSERT(reporter, proxy2 != proxy);

    // Clean up.
    recorder->priv().issueFlushToken();
    atlas->freeGpuResources(recorder->priv().tokenTracker()->nextFlushToken());
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////

namespace {
void test_draw_atlas_config(skiatest::Reporter* reporter, int maxTextureSize, size_t maxBytes,
                            MaskFormat maskFormat, SkISize expectedDimensions,
                            SkISize expectedPlotDimensions) {
    TextAtlasManager::AtlasConfig config(maxTextureSize, maxBytes);
    REPORTER_ASSERT(reporter, config.atlasDimensions(maskFormat) == expectedDimensions);
    REPORTER_ASSERT(reporter, config.plotDimensions(maskFormat) == expectedPlotDimensions);
}
}  // anonymous namespace

DEF_GRAPHITE_TEST(DrawAtlasConfig_Basic, reporter, CtsEnforcement::kNever) {
    // 1/4 MB
    test_draw_atlas_config(reporter, 65536, 256 * 1024, MaskFormat::kARGB,
                           { 256, 256 }, { 256, 256 });
    test_draw_atlas_config(reporter, 65536, 256 * 1024, MaskFormat::kA8,
                           { 512, 512 }, { 256, 256 });
    // 1/2 MB
    test_draw_atlas_config(reporter, 65536, 512 * 1024, MaskFormat::kARGB,
                           { 512, 256 }, { 256, 256 });
    test_draw_atlas_config(reporter, 65536, 512 * 1024, MaskFormat::kA8,
                           { 1024, 512 }, { 256, 256 });
    // 1 MB
    test_draw_atlas_config(reporter, 65536, 1024 * 1024, MaskFormat::kARGB,
                           { 512, 512 }, { 256, 256 });
    test_draw_atlas_config(reporter, 65536, 1024 * 1024, MaskFormat::kA8,
                           { 1024, 1024 }, { 256, 256 });
    // 2 MB
    test_draw_atlas_config(reporter, 65536, 2 * 1024 * 1024, MaskFormat::kARGB,
                           { 1024, 512 }, { 256, 256 });
    test_draw_atlas_config(reporter, 65536, 2 * 1024 * 1024, MaskFormat::kA8,
                           { 2048, 1024 }, { 512, 256 });
    // 4 MB
    test_draw_atlas_config(reporter, 65536, 4 * 1024 * 1024, MaskFormat::kARGB,
                           { 1024, 1024 }, { 256, 256 });
    test_draw_atlas_config(reporter, 65536, 4 * 1024 * 1024, MaskFormat::kA8,
                           { 2048, 2048 }, { 512, 512 });
    // 8 MB
    test_draw_atlas_config(reporter, 65536, 8 * 1024 * 1024, MaskFormat::kARGB,
                           { 2048, 1024 }, { 256, 256 });
    test_draw_atlas_config(reporter, 65536, 8 * 1024 * 1024, MaskFormat::kA8,
                           { 4096, 2048 }, { 512, 512 });
    // 16 MB (should be same as 8 MB)
    test_draw_atlas_config(reporter, 65536, 16 * 1024 * 1024, MaskFormat::kARGB,
                           { 2048, 1024 }, { 256, 256 });
    test_draw_atlas_config(reporter, 65536, 16 * 1024 * 1024, MaskFormat::kA8,
                           { 4096, 2048 }, { 512, 512 });

    // 4MB, restricted texture size
    test_draw_atlas_config(reporter, 1024, 8 * 1024 * 1024, MaskFormat::kARGB,
                           { 1024, 1024 }, { 256, 256 });
    test_draw_atlas_config(reporter, 1024, 8 * 1024 * 1024, MaskFormat::kA8,
                           { 1024, 1024 }, { 256, 256 });

    // 3 MB (should be same as 2 MB)
    test_draw_atlas_config(reporter, 65536, 3 * 1024 * 1024, MaskFormat::kARGB,
                           { 1024, 512 }, { 256, 256 });
    test_draw_atlas_config(reporter, 65536, 3 * 1024 * 1024, MaskFormat::kA8,
                           { 2048, 1024 }, { 512, 256 });

    // minimum size
    test_draw_atlas_config(reporter, 65536, 0, MaskFormat::kARGB,
                           { 256, 256 }, { 256, 256 });
    test_draw_atlas_config(reporter, 65536, 0, MaskFormat::kA8,
                           { 512, 512 }, { 256, 256 });
}
