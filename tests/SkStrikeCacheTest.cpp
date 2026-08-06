/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkExecutor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrike.h"  // IWYU pragma: keep
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTaskGroup.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <chrono>
#include <vector>

extern bool gSkUseThreadLocalStrikeCaches_IAcknowledgeThisIsIncrediblyExperimental;

DEF_TEST(SkStrikeCache_CachePurge, Reporter) {
    SkStrikeCache cache;

    sk_sp<SkTypeface> typeface =
            ToolUtils::CreatePortableTypeface("serif", SkFontStyle::Italic());

    SkFont font;
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setSubpixel(true);
    font.setTypeface(typeface);

    SkPaint defaultPaint;
    SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
            font, defaultPaint, SkSurfaceProps(0, kUnknown_SkPixelGeometry),
            SkScalerContextFlags::kNone, SkMatrix::I());

    // Initially empty cache
    REPORTER_ASSERT(Reporter, cache.getTotalMemoryUsed() == 0);

    {
        sk_sp<SkStrike> strike = strikeSpec.findOrCreateStrike(&cache);
    }

    // Stuff in cache.
    REPORTER_ASSERT(Reporter, cache.getTotalMemoryUsed() > 0);

    cache.purgeAll();

    // Purged cache.
    REPORTER_ASSERT(Reporter, cache.getTotalMemoryUsed() == 0);

    // Smallest cache.
    cache.setCacheSizeLimit(0);
    {
        sk_sp<SkStrike> strike = strikeSpec.findOrCreateStrike(&cache);
        REPORTER_ASSERT(Reporter, cache.getTotalMemoryUsed() == 0);
    }
    REPORTER_ASSERT(Reporter, cache.getTotalMemoryUsed() == 0);
}

DEF_TEST(SkStrikeCache_MultiThreadedMemory, reporter) {
    static constexpr int kThreadCount = 4;
    static constexpr int kWorkloadLoops = 50;

    sk_sp<SkTypeface> typefaceSerif =
            ToolUtils::CreatePortableTypeface("serif", SkFontStyle::Normal());
    sk_sp<SkTypeface> typefaceSans =
            ToolUtils::CreatePortableTypeface("sans-serif", SkFontStyle::Bold());

    auto runMultithreadedWorkload = [&](bool useThreadLocal) -> std::pair<size_t, double> {
        gSkUseThreadLocalStrikeCaches_IAcknowledgeThisIsIncrediblyExperimental = useThreadLocal;
        SkGraphics::PurgeFontCache();

        auto executor = SkExecutor::MakeFIFOThreadPool(kThreadCount);
        auto startTime = std::chrono::high_resolution_clock::now();

        auto workerTask = [&](int threadIndex) {
            // Workload simulating text rendering across multiple worker threads in Flutter
            SkFont font1(typefaceSerif, 12.0f + threadIndex * 2.0f);
            SkFont font2(typefaceSans, 16.0f + threadIndex * 2.0f);

            SkPaint paint;
            paint.setAntiAlias(true);
            SkSurfaceProps props(0, kUnknown_SkPixelGeometry);

            for (int loop = 0; loop < kWorkloadLoops; ++loop) {
                // Shared and thread-unique glyph specs
                SkStrikeSpec spec1 = SkStrikeSpec::MakeMask(
                        font1, paint, props, SkScalerContextFlags::kNone, SkMatrix::I());
                SkStrikeSpec spec2 = SkStrikeSpec::MakeMask(
                        font2, paint, props, SkScalerContextFlags::kNone, SkMatrix::I());

                sk_sp<SkStrike> strike1 =
                        SkStrikeCache::GlobalStrikeCache()->findOrCreateStrike(spec1);
                sk_sp<SkStrike> strike2 =
                        SkStrikeCache::GlobalStrikeCache()->findOrCreateStrike(spec2);

                // Touch glyphs to allocate glyph images/metrics
                strike1->lock();
                for (SkUnichar c = 'A'; c <= 'Z'; ++c) {
                    SkPackedGlyphID packedID(font1.unicharToGlyph(c));
                    strike1->digestFor(skglyph::kDirectMask, packedID);
                }
                strike1->unlock();

                strike2->lock();
                for (SkUnichar c = 'a' + threadIndex * 2; c <= 'z'; ++c) {
                    SkPackedGlyphID packedID(font2.unicharToGlyph(c));
                    strike2->digestFor(skglyph::kDirectMask, packedID);
                }
                strike2->unlock();
            }
        };

        SkTaskGroup(*executor).batch(kThreadCount, workerTask);

        auto endTime = std::chrono::high_resolution_clock::now();
        double elapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        size_t totalMemoryBytes = SkGraphics::GetTotalStrikeCacheBytesUsed();

        return {totalMemoryBytes, elapsedMs};
    };

    // 1. Run Baseline (Single Global Cache)
    auto [baselineMemory, baselineTimeMs] = runMultithreadedWorkload(false);

    // 2. Run Test (Thread-Local Strike Caches)
    auto [threadLocalMemory, threadLocalTimeMs] = runMultithreadedWorkload(true);

    // Reset flag to default
    gSkUseThreadLocalStrikeCaches_IAcknowledgeThisIsIncrediblyExperimental = false;

    // 3. Print Detailed Comparison Output
    double memoryIncreasePct = baselineMemory > 0
                                       ? ((double)threadLocalMemory - (double)baselineMemory) /
                                                 (double)baselineMemory * 100.0
                                       : 0.0;

    SkDebugf("\n=======================================================\n");
    SkDebugf("  STRIKE CACHE MULTI-THREADED MEMORY MEASUREMENT REPORT \n");
    SkDebugf("=======================================================\n");
    SkDebugf("  Threads:                      %d\n", kThreadCount);
    SkDebugf("  Global Cache Memory Used:      %zu Bytes (%.2f KB)\n",
             baselineMemory,
             baselineMemory / 1024.0);
    SkDebugf("  Thread-Local Memory Used:     %zu Bytes (%.2f KB)\n",
             threadLocalMemory,
             threadLocalMemory / 1024.0);
    SkDebugf("  Memory Difference (Delta):    %+zd Bytes (%.2f KB)\n",
             (ssize_t)threadLocalMemory - (ssize_t)baselineMemory,
             ((double)threadLocalMemory - (double)baselineMemory) / 1024.0);
    SkDebugf("  Memory Overhead Percentage:   %+.2f%%\n", memoryIncreasePct);
    SkDebugf("  Global Cache Duration:        %.2f ms\n", baselineTimeMs);
    SkDebugf("  Thread-Local Cache Duration:   %.2f ms\n", threadLocalTimeMs);
    SkDebugf("=======================================================\n\n");

    REPORTER_ASSERT(reporter, baselineMemory > 0);
    REPORTER_ASSERT(reporter, threadLocalMemory > 0);
}
