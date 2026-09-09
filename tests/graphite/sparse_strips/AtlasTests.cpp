/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathBuilder.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/StripGenerator.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

namespace skgpu::graphite {

namespace {

std::optional<AlphaAtlasManager::AlphaAllocation> request_alpha_space(AlphaAtlasManager* mgr,
                                                                     int32_t numBytes) {
    if (!mgr->requestAlphaSpace(numBytes)) {
        return std::nullopt;
    }
    return mgr->finalizeRun(numBytes);
}

}  // namespace

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasDimensionsAndStride,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());
    // Initial allocation (e.g. 512 bytes = 8 tiles of 8x8)
    auto alloc1 = request_alpha_space(&atlasManager, 512);
    REPORTER_ASSERT(reporter, alloc1.has_value());
    REPORTER_ASSERT(reporter, alloc1->fAlphaIndex == 0);
    REPORTER_ASSERT(reporter, alloc1->fTexPage == 0);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);

    // Initial page dimensions: 8192 x 1
    sk_sp<TextureProxy> proxy = atlasManager.getPageProxy(0);
    REPORTER_ASSERT(reporter, proxy != nullptr);
    REPORTER_ASSERT(reporter, proxy->dimensions().width() == SparseStripConfig::kAtlasWidth);
    REPORTER_ASSERT(reporter, proxy->dimensions().height() == 1);

    // Verify row byte stride alignment (32,768 bytes/row = 8192 pixels * 4 bytes/pixel)
    int32_t strideBytes = SparseStripConfig::kAtlasWidthBytes;
    REPORTER_ASSERT(reporter, strideBytes == 32768);

    // Subsequent allocation on same row/page
    auto alloc2 = request_alpha_space(&atlasManager, 1024);
    REPORTER_ASSERT(reporter, alloc2.has_value());
    REPORTER_ASSERT(reporter, alloc2->fAlphaIndex == 512);
    REPORTER_ASSERT(reporter, alloc2->fTexPage == 0);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasDynamicGrowthAndDoubling,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Page 0: allocate exactly 1 row (32,768 bytes)
    auto alloc1 = request_alpha_space(&atlasManager, SparseStripConfig::kAtlasWidthBytes);
    REPORTER_ASSERT(reporter, alloc1.has_value());
    REPORTER_ASSERT(reporter, alloc1->fTexPage == 0);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(0)->dimensions().height() == 1);

    // Page 1: allocating 1 more tile exceeds Page 0, triggers doubling -> 2 rows (65,536 bytes)
    auto alloc2 = request_alpha_space(&atlasManager, 64);
    REPORTER_ASSERT(reporter, alloc2.has_value());
    REPORTER_ASSERT(reporter, alloc2->fAlphaIndex == 0); // Atomic on new page, starts at 0
    REPORTER_ASSERT(reporter, alloc2->fTexPage == 1);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(1)->dimensions().height() == 2);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasStrictTwoPageLimit,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Page 0: 1 row
    auto alloc1 = request_alpha_space(&atlasManager, SparseStripConfig::kAtlasWidthBytes);
    REPORTER_ASSERT(reporter, alloc1.has_value());
    REPORTER_ASSERT(reporter, alloc1->fTexPage == 0);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);

    // Page 1: 2 rows
    auto alloc2 = request_alpha_space(&atlasManager, SparseStripConfig::kAtlasWidthBytes * 2);
    REPORTER_ASSERT(reporter, alloc2.has_value());
    REPORTER_ASSERT(reporter, alloc2->fTexPage == 1);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);

    // Attempting to exceed Page 1 when out of proxy room: allocation gets kInvalidTexPage!
    auto alloc3 = request_alpha_space(&atlasManager, 64);
    REPORTER_ASSERT(reporter, alloc3.has_value());
    REPORTER_ASSERT(reporter, alloc3->fTexPage == SparseStripConfig::kInvalidTexPage);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasEndCapAtomicity,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Allocate almost all of Page 0, leaving 64 bytes
    auto alloc1 = request_alpha_space(&atlasManager, SparseStripConfig::kAtlasWidthBytes - 64);
    REPORTER_ASSERT(reporter, alloc1.has_value());
    REPORTER_ASSERT(reporter, alloc1->fTexPage == 0);
    REPORTER_ASSERT(reporter, alloc1->fAlphaIndex == 0);

    // Request 128 bytes (which cannot fit in the remaining 64 bytes of Page 0).
    // The allocation must be atomic to Page 1: entire 128 bytes at offset 0 of Page 1.
    auto alloc2 = request_alpha_space(&atlasManager, 128);
    REPORTER_ASSERT(reporter, alloc2.has_value());
    REPORTER_ASSERT(reporter, alloc2->fTexPage == 1);
    REPORTER_ASSERT(reporter, alloc2->fAlphaIndex == 0);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasFlipFlopAtFlush,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Slot 0: 1 row (32,768 bytes)
    auto alloc1 = request_alpha_space(&atlasManager, SparseStripConfig::kAtlasWidthBytes);
    REPORTER_ASSERT(reporter, alloc1.has_value());
    REPORTER_ASSERT(reporter, alloc1->fTexPage == 0);
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 0);
    sk_sp<TextureProxy> initialSlot0 = atlasManager.getPageProxy(0);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);

    // Slot 1: 2 rows (65,536 bytes) via XOR flip-flop (0 ^ 1 = 1)
    auto alloc2 = request_alpha_space(&atlasManager, 64);
    REPORTER_ASSERT(reporter, alloc2.has_value());
    REPORTER_ASSERT(reporter, alloc2->fTexPage == 1);
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 1);
    sk_sp<TextureProxy> initialSlot1 = atlasManager.getPageProxy(1);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);

    // Flush: recordUploads(nullptr) retires older slot 0 (1 ^ 1 = 0).
    // Slot 1 stays right in place in slot 1 (no moving needed)!
    atlasManager.recordUploads(nullptr);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(0) == nullptr);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(1) == initialSlot1);
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 1);

    // Continue allocating in current active slot (Slot 1):
    auto alloc3 = request_alpha_space(&atlasManager, 64);
    REPORTER_ASSERT(reporter, alloc3.has_value());
    REPORTER_ASSERT(reporter, alloc3->fTexPage == 1);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);

    // Fill Slot 1 and trigger flip-flop to Slot 0 (1 ^ 1 = 0) -> doubled to 4 rows!
    auto alloc4 = request_alpha_space(&atlasManager, SparseStripConfig::kAtlasWidthBytes * 2);
    REPORTER_ASSERT(reporter, alloc4.has_value());
    REPORTER_ASSERT(reporter, alloc4->fTexPage == 0);
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 0);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(0)->dimensions().height() == 4);

    // Second flush: retires older slot 1 (0 ^ 1 = 1). Slot 0 remains active!
    atlasManager.recordUploads(nullptr);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(1) == nullptr);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(0)->dimensions().height() == 4);
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 0);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasMidRunPageRelocation,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // 1. Fill Page 0 so only 64 bytes (1 tile) remain.
    auto alloc1 = request_alpha_space(&atlasManager, SparseStripConfig::kAtlasWidthBytes - 64);
    REPORTER_ASSERT(reporter, alloc1.has_value());
    REPORTER_ASSERT(reporter, alloc1->fTexPage == 0);
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 0);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);

    // 2. Begin a multi-tile run: tile 1 fits on Page 0 (activeSlot 0)
    uint8_t* t1 = atlasManager.requestAlphaSpace(64);
    REPORTER_ASSERT(reporter, t1 != nullptr);
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 0);
    std::memset(t1, 0xAA, 64);

    // 3. Tile 2 exceeds Page 0! Triggers cross-page relocation of tile 1 to Page 1
    uint8_t* t2 = atlasManager.requestAlphaSpace(64);
    REPORTER_ASSERT(reporter, t2 != nullptr);
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 1);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);
    std::memset(t2, 0xBB, 64);

    // 4. Finalize the run: both tiles reside on Page 1 starting at offset 0
    auto alloc2 = atlasManager.finalizeRun();
    REPORTER_ASSERT(reporter, alloc2.has_value());
    REPORTER_ASSERT(reporter, alloc2->fTexPage == 1);
    REPORTER_ASSERT(reporter, alloc2->fAlphaIndex == 0);
    REPORTER_ASSERT(reporter, alloc2->fWritePtr[0] == 0xAA);
    REPORTER_ASSERT(reporter, alloc2->fWritePtr[64] == 0xBB);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasNullEndCapsAndCleanup,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager* atlasManager = recorder->priv().atlasProvider()->alphaAtlasManager();
    REPORTER_ASSERT(reporter, atlasManager != nullptr);

    // 1. Fill Slot 0 (1 row = 32,768 bytes).
    auto alloc1 = request_alpha_space(atlasManager, SparseStripConfig::kAtlasWidthBytes);
    REPORTER_ASSERT(reporter, alloc1.has_value());
    REPORTER_ASSERT(reporter, alloc1->fTexPage == 0);
    std::memset(alloc1->fWritePtr, 0xAA, SparseStripConfig::kAtlasWidthBytes);

    // 2. Fill Slot 1 (2 rows = 65,536 bytes).
    auto alloc2 = request_alpha_space(atlasManager, SparseStripConfig::kAtlasWidthBytes * 2);
    REPORTER_ASSERT(reporter, alloc2.has_value());
    REPORTER_ASSERT(reporter, alloc2->fTexPage == 1);
    REPORTER_ASSERT(reporter, atlasManager->numPages() == 2);
    REPORTER_ASSERT(reporter, atlasManager->activeSlot() == 1);
    std::memset(alloc2->fWritePtr, 0xBB, SparseStripConfig::kAtlasWidthBytes * 2);

    // 3. Process geometry when both proxy slots are full: MakeStrips ALWAYS completes!
    // Out-of-room endcaps receive fTexPage == kInvalidTexPage.
    SkTDArray<uint8_t> lut = GenerateMSAALUT<uint8_t>();
    StripGenerator generator(100, 100, lut, recorder.get());

    SkPath path = SkPathBuilder().moveTo(10, 10)
                                 .lineTo(90, 10)
                                 .lineTo(90, 90)
                                 .lineTo(10, 90)
                                 .close()
                                 .detach();

    bool success = generator.processGeometry(path, SkMatrix::I());
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, !generator.ends().empty());
    REPORTER_ASSERT(reporter, generator.hasNullCaps());

    // Verify at least one endcap has fTexPage == kInvalidTexPage
    bool hasInvalidPage = false;
    for (const auto& cap : generator.ends().caps()) {
        if (cap.fTexPage == SparseStripConfig::kInvalidTexPage) {
            hasInvalidPage = true;
            break;
        }
    }
    REPORTER_ASSERT(reporter, hasInvalidPage);

    // 4. Simulate flush of tracked devices: recordUploads retires older slot 0!
    atlasManager->recordUploads(nullptr);

    // 5. Cleanup function handles all null endcaps!
    bool cleaned = generator.cleanupNullCaps();
    REPORTER_ASSERT(reporter, cleaned);
    REPORTER_ASSERT(reporter, !generator.hasNullCaps());
    REPORTER_ASSERT(reporter, !generator.ends().proxies().empty());

    for (const auto& cap : generator.ends().caps()) {
        REPORTER_ASSERT(reporter, cap.fTexPage != SparseStripConfig::kInvalidTexPage);
        REPORTER_ASSERT(reporter, cap.fAlphaIndex >= 0);
    }
}

DEF_TEST(SparseStrips_AtlasTestUtilsEarlyOut, reporter) {
    AlphaAtlasManager atlasManager;
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 0);

    auto alloc1 = request_alpha_space(&atlasManager, 512);
    REPORTER_ASSERT(reporter, alloc1.has_value());
    REPORTER_ASSERT(reporter, alloc1->fWritePtr != nullptr);
    REPORTER_ASSERT(reporter, alloc1->fAlphaIndex == 0);
    REPORTER_ASSERT(reporter, alloc1->fTexPage == 0);

    std::memset(alloc1->fWritePtr, 0xAA, 512);

    auto alloc2 = request_alpha_space(&atlasManager, 1024);
    REPORTER_ASSERT(reporter, alloc2.has_value());
    REPORTER_ASSERT(reporter, alloc2->fWritePtr != nullptr);
    REPORTER_ASSERT(reporter, alloc2->fAlphaIndex == 512);
    REPORTER_ASSERT(reporter, alloc2->fTexPage == 0);
    std::memset(alloc2->fWritePtr, 0xBB, 1024);

    REPORTER_ASSERT(reporter, alloc1->fWritePtr[0] == 0xAA);
    REPORTER_ASSERT(reporter, alloc2->fWritePtr[0] == 0xBB);

    atlasManager.freeGpuResources();
    auto alloc3 = request_alpha_space(&atlasManager, 256);
    REPORTER_ASSERT(reporter, alloc3.has_value());
    REPORTER_ASSERT(reporter, alloc3->fAlphaIndex == 0);
}

}  // namespace skgpu::graphite
