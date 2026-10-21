/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/Token.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <cstring>
#include <optional>
#include <utility>

namespace skgpu::graphite {

namespace {

constexpr uint16_t kOverflowSlot = AlphaAtlasManager::kOverflowSlot;

// Writes a run of `numBytes` alphas set to `value` and finalizes it. If `ends` is provided, the
// run's EndCap is added to it, the way MakeStrips does.
std::optional<std::pair<int32_t, uint16_t>> add_run(AlphaAtlasManager* atlasManager,
                                                    EndCaps* ends,
                                                    int32_t numBytes,
                                                    uint8_t value) {
    uint8_t* dst = atlasManager->requestAlphaSpace(numBytes);
    if (!dst) {
        return std::nullopt;
    }
    std::memset(dst, value, numBytes);
    auto alloc = atlasManager->finalizeRun();
    if (alloc && ends) {
        if (alloc->second == kOverflowSlot) {
            ends->markFirstOverflowCap();
        }
        ends->addCap(/*x=*/0,
                     /*y=*/0,
                     static_cast<uint16_t>(numBytes / SparseStripConfig::kTileHeight),
                     alloc->first,
                     alloc->second);
    }
    return alloc;
}

// Fills both regular pages of a fresh atlas (1 row in slot 0, then 2 rows in slot 1), so that
// subsequent runs spill into the overflow staging buffer.
void fill_initial_pages(skiatest::Reporter* reporter, AlphaAtlasManager* atlasManager) {
    auto alloc0 = add_run(atlasManager, nullptr, SparseStripConfig::kAtlasWidthBytes, 0x01);
    REPORTER_ASSERT(reporter, alloc0.has_value() && alloc0->second == 0);
    auto alloc1 = add_run(atlasManager, nullptr, SparseStripConfig::kAtlasWidthBytes * 2, 0x02);
    REPORTER_ASSERT(reporter, alloc1.has_value() && alloc1->second == 1);
    REPORTER_ASSERT(reporter, atlasManager->numPages() == 2);
}

}  // anonymous namespace

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasDimensionsAndStride,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());
    // Initial allocation (e.g. 512 bytes = 8 tiles of 8x8)
    REPORTER_ASSERT(reporter, atlasManager.requestAlphaSpace(512) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, alphaIndex == 0);
        REPORTER_ASSERT(reporter, texPage == 0);
    }
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
    REPORTER_ASSERT(reporter, atlasManager.requestAlphaSpace(1024) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, alphaIndex == 512);
        REPORTER_ASSERT(reporter, texPage == 0);
    }
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasDynamicGrowthAndDoubling,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Page 0: allocate exactly 1 row (32,768 bytes)
    REPORTER_ASSERT(reporter,
                    atlasManager.requestAlphaSpace(SparseStripConfig::kAtlasWidthBytes) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 0);
    }
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(0)->dimensions().height() == 1);

    // Page 1: allocating 1 more tile exceeds Page 0, triggers doubling -> 2 rows (65,536 bytes)
    REPORTER_ASSERT(reporter, atlasManager.requestAlphaSpace(64) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, alphaIndex == 0);  // Atomic on new page, starts at 0
        REPORTER_ASSERT(reporter, texPage == 1);
    }
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(1)->dimensions().height() == 2);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasOverflowStagingAndCommit,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());
    EndCaps ends;

    // Page 0: 1 row, Page 1: 2 rows
    auto alloc0 = add_run(&atlasManager, &ends, SparseStripConfig::kAtlasWidthBytes, 0x01);
    REPORTER_ASSERT(reporter, alloc0.has_value() && alloc0->second == 0);
    auto alloc1 = add_run(&atlasManager, &ends, SparseStripConfig::kAtlasWidthBytes * 2, 0x02);
    REPORTER_ASSERT(reporter, alloc1.has_value() && alloc1->second == 1);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);
    REPORTER_ASSERT(reporter, !ends.hasOverflowCaps());
    REPORTER_ASSERT(reporter, ends.firstOverflowCapIndex() == -1);

    // Exceeding Page 1: the alphas are staged for the overflow page, so MakeStrips can run to
    // completion without a flush.
    auto alloc2 = add_run(&atlasManager, &ends, 64, 0xAB);
    REPORTER_ASSERT(reporter, alloc2.has_value());
    REPORTER_ASSERT(reporter, alloc2->second == kOverflowSlot);
    REPORTER_ASSERT(reporter, alloc2->first == 0);
    REPORTER_ASSERT(reporter, atlasManager.hasOverflowStaging());
    REPORTER_ASSERT(reporter, atlasManager.overflowStagingData()[0] == 0xAB);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(kOverflowSlot) == nullptr);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);
    REPORTER_ASSERT(reporter, ends.hasPendingOverflow());
    REPORTER_ASSERT(reporter, ends.firstOverflowCapIndex() == 2);

    // Subsequent overflow caps do not overwrite firstOverflowCapIndex
    auto alloc3 = add_run(&atlasManager, &ends, 32, 0xCD);
    REPORTER_ASSERT(reporter, alloc3.has_value());
    REPORTER_ASSERT(reporter, alloc3->second == kOverflowSlot);
    REPORTER_ASSERT(reporter, alloc3->first == 64);
    REPORTER_ASSERT(reporter, ends.firstOverflowCapIndex() == 2);

    // The regular pages are bound once strip generation completes, with a placeholder for the
    // overflow slot.
    atlasManager.bindPages(&ends);
    sk_sp<TextureProxy> page0 = atlasManager.getPageProxy(0);
    sk_sp<TextureProxy> page1 = atlasManager.getPageProxy(1);
    REPORTER_ASSERT(reporter, ends.proxies()[0] == page0);
    REPORTER_ASSERT(reporter, ends.proxies()[1] == page1);
    REPORTER_ASSERT(reporter, ends.proxies()[kOverflowSlot] != nullptr);

    // Committing copies the staged alphas into the overflow page and rebases the overflow caps,
    // without touching the regular page caps or bindings.
    REPORTER_ASSERT(reporter, atlasManager.commitOverflow(&ends));
    REPORTER_ASSERT(reporter, !ends.hasPendingOverflow());
    REPORTER_ASSERT(reporter, !atlasManager.hasOverflowStaging());
    sk_sp<TextureProxy> overflowPage = atlasManager.getPageProxy(kOverflowSlot);
    REPORTER_ASSERT(reporter, overflowPage != nullptr);
    REPORTER_ASSERT(reporter, ends.proxies()[0] == page0);
    REPORTER_ASSERT(reporter, ends.proxies()[1] == page1);
    REPORTER_ASSERT(reporter, ends.proxies()[kOverflowSlot] == overflowPage);
    REPORTER_ASSERT(reporter, ends.caps()[0].fTexPage == 0 && ends.caps()[0].fAlphaIndex == 0);
    REPORTER_ASSERT(reporter, ends.caps()[1].fTexPage == 1 && ends.caps()[1].fAlphaIndex == 0);
    REPORTER_ASSERT(reporter, ends.caps()[2].fTexPage == kOverflowSlot);
    REPORTER_ASSERT(reporter, ends.caps()[2].fAlphaIndex == 0);
    REPORTER_ASSERT(reporter, ends.caps()[3].fTexPage == kOverflowSlot);
    REPORTER_ASSERT(reporter, ends.caps()[3].fAlphaIndex == 64);
    REPORTER_ASSERT(reporter, atlasManager.getPageData(kOverflowSlot)[0] == 0xAB);
    REPORTER_ASSERT(reporter, atlasManager.getPageData(kOverflowSlot)[64] == 0xCD);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);

    // Flush: recordUploads retires older slot 0, and allocation resumes from the freed slot. The
    // recorded EndCaps keep the retired page alive.
    atlasManager.recordUploads(nullptr);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(0) == nullptr);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(1) == page1);
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 0);
    REPORTER_ASSERT(reporter, ends.proxies()[0] == page0);

    auto alloc4 = add_run(&atlasManager, nullptr, 64, 0xEF);
    REPORTER_ASSERT(reporter, alloc4.has_value());
    REPORTER_ASSERT(reporter, alloc4->second == 0 && alloc4->first == 0);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasStraddleToOverflow,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Fill Page 0
    REPORTER_ASSERT(reporter,
                    atlasManager.requestAlphaSpace(SparseStripConfig::kAtlasWidthBytes) != nullptr);
    auto alloc0 = atlasManager.finalizeRun();
    REPORTER_ASSERT(reporter, alloc0.has_value());

    // Fill Page 1 almost full, leaving 64 bytes
    int32_t page1Capacity = SparseStripConfig::kAtlasWidthBytes * 2;
    REPORTER_ASSERT(reporter, atlasManager.requestAlphaSpace(page1Capacity - 64) != nullptr);
    auto alloc1 = atlasManager.finalizeRun();
    REPORTER_ASSERT(reporter, alloc1.has_value());
    REPORTER_ASSERT(reporter, alloc1->second == 1);

    // Now start a run that straddles past Page 1's capacity:
    // Chunk 1: 64 bytes (fills Page 1)
    uint8_t* p1 = atlasManager.requestAlphaSpace(64);
    REPORTER_ASSERT(reporter, p1 != nullptr);
    for (int i = 0; i < 64; ++i) {
        p1[i] = static_cast<uint8_t>(i + 1);
    }

    // Chunk 2: 64 bytes (overruns Page 1)
    uint8_t* p2 = atlasManager.requestAlphaSpace(64);
    REPORTER_ASSERT(reporter, p2 != nullptr);
    for (int i = 0; i < 64; ++i) {
        p2[i] = static_cast<uint8_t>(i + 65);
    }

    // Finalize run: should memmove the entire 128 bytes to the overflow staging buffer!
    auto straddleAlloc = atlasManager.finalizeRun();
    REPORTER_ASSERT(reporter, straddleAlloc.has_value());
    REPORTER_ASSERT(reporter, straddleAlloc->second == kOverflowSlot);
    REPORTER_ASSERT(reporter, straddleAlloc->first == 0);
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == kOverflowSlot);
    REPORTER_ASSERT(reporter, atlasManager.hasOverflowStaging());

    const uint8_t* stagingData = atlasManager.overflowStagingData();
    REPORTER_ASSERT(reporter, stagingData != nullptr);
    for (int i = 0; i < 128; ++i) {
        REPORTER_ASSERT(reporter, stagingData[i] == static_cast<uint8_t>(i + 1));
    }

    // Subsequent runs continue in the staging buffer after the straddled run.
    auto nextAlloc = add_run(&atlasManager, nullptr, 64, 0x77);
    REPORTER_ASSERT(reporter, nextAlloc.has_value());
    REPORTER_ASSERT(reporter, nextAlloc->second == kOverflowSlot);
    REPORTER_ASSERT(reporter, nextAlloc->first == 128);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasEndCapAtomicity,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Allocate almost all of Page 0, leaving 64 bytes
    REPORTER_ASSERT(
            reporter,
            atlasManager.requestAlphaSpace(SparseStripConfig::kAtlasWidthBytes - 64) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 0);
        REPORTER_ASSERT(reporter, alphaIndex == 0);
    }

    // Request 128 bytes (which cannot fit in the remaining 64 bytes of Page 0).
    // The allocation must be atomic to Page 1: entire 128 bytes at offset 0 of Page 1.
    REPORTER_ASSERT(reporter, atlasManager.requestAlphaSpace(128) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 1);
        REPORTER_ASSERT(reporter, alphaIndex == 0);
    }
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasFlipFlopAtFlush,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Slot 0: 1 row (32,768 bytes)
    REPORTER_ASSERT(reporter,
                    atlasManager.requestAlphaSpace(SparseStripConfig::kAtlasWidthBytes) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 0);
    }
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 0);
    sk_sp<TextureProxy> initialSlot0 = atlasManager.getPageProxy(0);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);

    // Slot 1: 2 rows (65,536 bytes) via XOR flip-flop (0 ^ 1 = 1)
    REPORTER_ASSERT(reporter, atlasManager.requestAlphaSpace(64) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 1);
    }
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
    REPORTER_ASSERT(reporter, atlasManager.requestAlphaSpace(64) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 1);
    }
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);

    // Fill Slot 1 and trigger flip-flop to Slot 0 (1 ^ 1 = 0) -> doubled to 4 rows!
    REPORTER_ASSERT(
            reporter,
            atlasManager.requestAlphaSpace(SparseStripConfig::kAtlasWidthBytes * 2) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 0);
    }
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

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasStraddleAndMemmove,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Page 0 has 1 row (32,768 bytes).
    // Fill Page 0 up to 64 bytes before the end.
    int32_t almostFull = SparseStripConfig::kAtlasWidthBytes - 64;
    REPORTER_ASSERT(reporter, atlasManager.requestAlphaSpace(almostFull) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 0);
        REPORTER_ASSERT(reporter, alphaIndex == 0);
    }

    // Now start a run of 128 bytes (two 64-byte chunks).
    // Chunk 1 (64 bytes): fills the rest of Page 0.
    uint8_t* ptr1 = atlasManager.requestAlphaSpace(64);
    REPORTER_ASSERT(reporter, ptr1 != nullptr);
    for (int i = 0; i < 64; ++i) {
        ptr1[i] = static_cast<uint8_t>(i + 1);
    }

    // Chunk 2 (64 bytes): straddles past Page 0's capacity into overrun!
    uint8_t* ptr2 = atlasManager.requestAlphaSpace(64);
    REPORTER_ASSERT(reporter, ptr2 != nullptr);
    for (int i = 0; i < 64; ++i) {
        ptr2[i] = static_cast<uint8_t>(i + 65);
    }

    // Finalize the run: should detect straddle, create Page 1, and copy 128 bytes to offset 0 of
    // Page 1!
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 1);
        REPORTER_ASSERT(reporter, alphaIndex == 0);
    }
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 1);

    // Verify that the written bytes were accurately moved into Page 1 at offset 0
    sk_sp<TextureProxy> proxy1 = atlasManager.getPageProxy(1);
    REPORTER_ASSERT(reporter, proxy1 != nullptr);
    REPORTER_ASSERT(reporter, proxy1->dimensions().height() == 2);  // doubled rows

    const uint8_t* page1Data = atlasManager.getPageData(1);
    REPORTER_ASSERT(reporter, page1Data != nullptr);
    for (int i = 0; i < 128; ++i) {
        REPORTER_ASSERT(reporter, page1Data[i] == static_cast<uint8_t>(i + 1));
    }
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasOverflowAppendThenOverwrite,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());
    fill_initial_pages(reporter, &atlasManager);

    // Draw A creates the overflow page, which is at least as large as the next regular page.
    EndCaps endsA;
    atlasManager.beginDraw();
    auto allocA = add_run(&atlasManager, &endsA, 64, 0xA1);
    REPORTER_ASSERT(reporter, allocA.has_value() && allocA->second == kOverflowSlot);
    REPORTER_ASSERT(reporter, atlasManager.commitOverflow(&endsA));
    sk_sp<TextureProxy> overflowPage = atlasManager.getPageProxy(kOverflowSlot);
    REPORTER_ASSERT(reporter, overflowPage != nullptr);
    REPORTER_ASSERT(reporter, overflowPage->dimensions().height() == 4);
    REPORTER_ASSERT(reporter, endsA.caps()[0].fAlphaIndex == 0);
    const uint32_t firstContentsID = atlasManager.pageID(kOverflowSlot);

    // Draw B, in the same flush epoch, appends after A since A may still sample its alphas.
    EndCaps endsB;
    atlasManager.beginDraw();
    auto allocB = add_run(&atlasManager, &endsB, 128, 0xB2);
    REPORTER_ASSERT(reporter, allocB.has_value());
    REPORTER_ASSERT(reporter, allocB->second == kOverflowSlot && allocB->first == 0);
    REPORTER_ASSERT(reporter, !atlasManager.overflowCommitNeedsFlush());
    REPORTER_ASSERT(reporter, atlasManager.commitOverflow(&endsB));
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(kOverflowSlot) == overflowPage);
    REPORTER_ASSERT(reporter, endsB.caps()[0].fAlphaIndex == 64);
    REPORTER_ASSERT(reporter, atlasManager.pageID(kOverflowSlot) == firstContentsID);
    const uint8_t* overflowData = atlasManager.getPageData(kOverflowSlot);
    REPORTER_ASSERT(reporter, overflowData[0] == 0xA1 && overflowData[63] == 0xA1);
    REPORTER_ASSERT(reporter, overflowData[64] == 0xB2 && overflowData[191] == 0xB2);

    // A full flush issues a new flush token, after which no draw can still sample the overflow
    // page. Retiring the older regular page resumes regular allocation from the freed slot.
    recorder->priv().issueFlushToken();
    atlasManager.recordUploads(nullptr);
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 0);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);

    // Exhaust the regular pages again. Slot 0's new page continues the doubling (4 rows).
    auto refill = add_run(&atlasManager, nullptr, SparseStripConfig::kAtlasWidthBytes * 4, 0x03);
    REPORTER_ASSERT(reporter, refill.has_value() && refill->second == 0);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(0)->dimensions().height() == 4);

    // Draw C overwrites the overflow page in place: same texture, offset 0, new contents ID.
    EndCaps endsC;
    atlasManager.beginDraw();
    auto allocC = add_run(&atlasManager, &endsC, 64, 0xC3);
    REPORTER_ASSERT(reporter, allocC.has_value() && allocC->second == kOverflowSlot);
    REPORTER_ASSERT(reporter, atlasManager.commitOverflow(&endsC));
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(kOverflowSlot) == overflowPage);
    REPORTER_ASSERT(reporter, endsC.caps()[0].fAlphaIndex == 0);
    REPORTER_ASSERT(reporter, atlasManager.getPageData(kOverflowSlot)[0] == 0xC3);
    REPORTER_ASSERT(reporter, atlasManager.pageID(kOverflowSlot) != firstContentsID);
    REPORTER_ASSERT(reporter, atlasManager.parkedOverflowPageCount() == 0);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasOverflowReplaceWhenFull,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());
    fill_initial_pages(reporter, &atlasManager);

    EndCaps endsA;
    atlasManager.beginDraw();
    REPORTER_ASSERT(reporter, add_run(&atlasManager, &endsA, 64, 0xA1).has_value());
    REPORTER_ASSERT(reporter, atlasManager.commitOverflow(&endsA));
    sk_sp<TextureProxy> firstOverflowPage = atlasManager.getPageProxy(kOverflowSlot);
    REPORTER_ASSERT(reporter, firstOverflowPage->dimensions().height() == 4);

    // Draw B doesn't fit after A's alphas, and A may still be sampling the page, so the page is
    // replaced by a larger one instead of flushing.
    EndCaps endsB;
    atlasManager.beginDraw();
    auto allocB = add_run(&atlasManager, &endsB, SparseStripConfig::kAtlasWidthBytes * 4, 0xB2);
    REPORTER_ASSERT(reporter, allocB.has_value() && allocB->second == kOverflowSlot);
    REPORTER_ASSERT(reporter, !atlasManager.overflowCommitNeedsFlush());
    REPORTER_ASSERT(reporter, atlasManager.commitOverflow(&endsB));
    sk_sp<TextureProxy> secondOverflowPage = atlasManager.getPageProxy(kOverflowSlot);
    REPORTER_ASSERT(reporter, secondOverflowPage != nullptr);
    REPORTER_ASSERT(reporter, secondOverflowPage != firstOverflowPage);
    REPORTER_ASSERT(reporter, secondOverflowPage->dimensions().height() == 8);
    REPORTER_ASSERT(reporter, endsB.caps()[0].fAlphaIndex == 0);
    REPORTER_ASSERT(reporter, atlasManager.getPageData(kOverflowSlot)[0] == 0xB2);

    // Each draw keeps sampling the page its alphas were committed to.
    REPORTER_ASSERT(reporter, endsA.proxies()[kOverflowSlot] == firstOverflowPage);
    REPORTER_ASSERT(reporter, endsB.proxies()[kOverflowSlot] == secondOverflowPage);

    // The replaced page is parked until its pending rows are uploaded at the next flush.
    REPORTER_ASSERT(reporter, atlasManager.parkedOverflowPageCount() == 1);
    atlasManager.recordUploads(nullptr);
    REPORTER_ASSERT(reporter, atlasManager.parkedOverflowPageCount() == 0);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(kOverflowSlot) == secondOverflowPage);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasOverflowNotOverwrittenWhileInFlight,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());
    fill_initial_pages(reporter, &atlasManager);

    EndCaps endsA;
    {
        AlphaAtlasManager::DrawScope outerScope(&atlasManager, &endsA);
        atlasManager.beginDraw();
        REPORTER_ASSERT(reporter, add_run(&atlasManager, &endsA, 64, 0xA1).has_value());
        atlasManager.bindPages(&endsA);
        REPORTER_ASSERT(reporter, atlasManager.commitOverflow(&endsA));
        sk_sp<TextureProxy> overflowPage = atlasManager.getPageProxy(kOverflowSlot);

        // Before draw A is recorded, a nested draw (e.g. from shading.toKey()) triggers a full
        // flush, which uploads the atlas and issues a new flush token. On their own, those would
        // allow overwriting the overflow page, but draw A is still in flight.
        recorder->priv().issueFlushToken();
        atlasManager.recordUploads(nullptr);
        EndCaps endsB;
        {
            AlphaAtlasManager::DrawScope innerScope(&atlasManager, &endsB);
            atlasManager.beginDraw();
            // The flush retired slot 0; fill its new 4 row page so the nested draw overflows.
            auto refill = add_run(
                    &atlasManager, nullptr, SparseStripConfig::kAtlasWidthBytes * 4, 0x03);
            REPORTER_ASSERT(reporter, refill.has_value() && refill->second == 0);
            auto allocB = add_run(&atlasManager, &endsB, 64, 0xB2);
            REPORTER_ASSERT(reporter, allocB.has_value() && allocB->second == kOverflowSlot);
            REPORTER_ASSERT(reporter, !atlasManager.overflowCommitNeedsFlush());
            REPORTER_ASSERT(reporter, atlasManager.commitOverflow(&endsB));
            REPORTER_ASSERT(reporter, atlasManager.getPageProxy(kOverflowSlot) == overflowPage);
            // Appended after draw A's (row-padded, uploaded) alphas instead of overwriting them.
            REPORTER_ASSERT(reporter,
                            endsB.caps()[0].fAlphaIndex == SparseStripConfig::kAtlasWidthBytes);
            REPORTER_ASSERT(reporter, atlasManager.getPageData(kOverflowSlot)[0] == 0xA1);
        }
    }

    // Closing the scopes marks the overflow page as used by the current flush, so a later draw in
    // the same flush epoch still appends.
    REPORTER_ASSERT(reporter,
                    atlasManager.lastUseToken(kOverflowSlot) ==
                            recorder->priv().tokenTracker()->nextFlushToken());
    EndCaps endsC;
    atlasManager.beginDraw();
    auto allocC = add_run(&atlasManager, &endsC, 64, 0xC3);
    REPORTER_ASSERT(reporter, allocC.has_value() && allocC->second == kOverflowSlot);
    REPORTER_ASSERT(reporter, atlasManager.commitOverflow(&endsC));
    REPORTER_ASSERT(reporter,
                    endsC.caps()[0].fAlphaIndex == SparseStripConfig::kAtlasWidthBytes + 64);
    REPORTER_ASSERT(reporter, atlasManager.getPageData(kOverflowSlot)[0] == 0xA1);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasOverflowGrowsNextPage,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());
    fill_initial_pages(reporter, &atlasManager);

    // Spill 5 rows worth of alphas into the overflow page.
    EndCaps ends;
    atlasManager.beginDraw();
    REPORTER_ASSERT(
            reporter,
            add_run(&atlasManager, &ends, SparseStripConfig::kAtlasWidthBytes * 5, 0x5A).has_value());
    REPORTER_ASSERT(reporter, atlasManager.commitOverflow(&ends));

    // After the flush retires slot 0, its next page absorbs the overflow demand rather than only
    // doubling to 4 rows.
    atlasManager.recordUploads(nullptr);
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 0);
    auto alloc = add_run(&atlasManager, nullptr, 64, 0x01);
    REPORTER_ASSERT(reporter, alloc.has_value() && alloc->second == 0);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(0)->dimensions().height() == 5);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasPageResidency,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());
    REPORTER_ASSERT(reporter, atlasManager.pageID(0) == AlphaAtlasManager::kInvalidPageID);

    auto alloc0 = add_run(&atlasManager, nullptr, SparseStripConfig::kAtlasWidthBytes, 0x01);
    REPORTER_ASSERT(reporter, alloc0.has_value() && alloc0->second == 0);
    const uint32_t id0 = atlasManager.pageID(0);
    REPORTER_ASSERT(reporter, id0 != AlphaAtlasManager::kInvalidPageID);
    REPORTER_ASSERT(reporter, atlasManager.findResidentSlot(id0) == 0);

    EndCaps ends;
    auto alloc1 = add_run(&atlasManager, &ends, 64, 0x02);
    REPORTER_ASSERT(reporter, alloc1.has_value() && alloc1->second == 1);
    const uint32_t id1 = atlasManager.pageID(1);
    REPORTER_ASSERT(reporter, id1 != AlphaAtlasManager::kInvalidPageID && id1 != id0);
    REPORTER_ASSERT(reporter, atlasManager.findResidentSlot(id1) == 1);

    // A draw binding a page marks it as used by the current flush once it's recorded.
    REPORTER_ASSERT(reporter, atlasManager.lastUseToken(1) == skgpu::Token::InvalidToken());
    atlasManager.bindPages(&ends);
    { AlphaAtlasManager::DrawScope scope(&atlasManager, &ends); }
    REPORTER_ASSERT(reporter,
                    atlasManager.lastUseToken(1) ==
                            recorder->priv().tokenTracker()->nextFlushToken());

    // Retiring a page at flush time ends the residency of its alphas.
    atlasManager.recordUploads(nullptr);
    REPORTER_ASSERT(reporter, !atlasManager.findResidentSlot(id0).has_value());
    REPORTER_ASSERT(reporter, atlasManager.findResidentSlot(id1) == 1);

    // Evicting ends the residency of everything recorded so far, but keeps the page itself.
    sk_sp<TextureProxy> page1 = atlasManager.getPageProxy(1);
    atlasManager.evictAtlases();
    REPORTER_ASSERT(reporter, !atlasManager.findResidentSlot(id1).has_value());
    REPORTER_ASSERT(reporter, atlasManager.pageID(1) != id1);
    REPORTER_ASSERT(reporter, atlasManager.findResidentSlot(atlasManager.pageID(1)) == 1);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(1) == page1);

    // The overflow page is overwritten in place, so its contents are never resident.
    AlphaAtlasManager overflowingAtlas(recorder.get());
    fill_initial_pages(reporter, &overflowingAtlas);
    EndCaps overflowEnds;
    REPORTER_ASSERT(reporter, add_run(&overflowingAtlas, &overflowEnds, 64, 0x03).has_value());
    REPORTER_ASSERT(reporter, overflowingAtlas.commitOverflow(&overflowEnds));
    const uint32_t overflowID = overflowingAtlas.pageID(kOverflowSlot);
    REPORTER_ASSERT(reporter, overflowID != AlphaAtlasManager::kInvalidPageID);
    REPORTER_ASSERT(reporter, !overflowingAtlas.findResidentSlot(overflowID).has_value());
}

}  // namespace skgpu::graphite
