/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

namespace skgpu::graphite {
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasDimensionsAndStride,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Initialize alpha buffer
    SkTDArray<uint8_t>& buf = atlasManager.getAlphaBuffer();
    REPORTER_ASSERT(reporter, buf.empty());

    // Add 1 row of data (32,768 bytes)
    buf.append(AlphaAtlasManager::kAtlasDimsBytes);

    SkTDArray<std::pair<int32_t, sk_sp<TextureProxy>>> allocations;
    bool success = atlasManager.getAssociatedProxies(allocations);
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, allocations.size() == 1);

    // Verify Texture dimensions (8192 x 32)
    sk_sp<TextureProxy> proxy = allocations[0].second;
    REPORTER_ASSERT(reporter, proxy != nullptr);
    REPORTER_ASSERT(reporter, proxy->dimensions().width() == AlphaAtlasManager::kAtlasWidth);
    REPORTER_ASSERT(reporter,
                    proxy->dimensions().height() == AlphaAtlasManager::kInitialAtlasHeight);

    // Verify row byte stride alignment (32,768 bytes/row = 8192 pixels * 4 bytes/pixel)
    int32_t strideBytes = AlphaAtlasManager::kAtlasDimsBytes;
    REPORTER_ASSERT(reporter, strideBytes == AlphaAtlasManager::kAtlasWidth * 4);
    REPORTER_ASSERT(reporter, allocations[0].first == AlphaAtlasManager::kAtlasDimsBytes);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasDynamicGrowth,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Step A: Small data (fits in height 32 = 1 MB)
    SkTDArray<uint8_t>& buf = atlasManager.getAlphaBuffer();
    int32_t smallBytes = AlphaAtlasManager::kAtlasDimsBytes * 10;
    buf.append(smallBytes);

    SkTDArray<std::pair<int32_t, sk_sp<TextureProxy>>> allocations;
    bool success = atlasManager.getAssociatedProxies(allocations);
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, allocations.size() == 1);
    REPORTER_ASSERT(reporter, allocations[0].second->dimensions().height() == 32);

    // Step B: Append more data requiring growth (e.g. 50 rows total = 1.6 MB -> needs height 64)
    SkTDArray<uint8_t>& buf2 = atlasManager.getAlphaBuffer();
    int32_t extraBytes = AlphaAtlasManager::kAtlasDimsBytes * 40;
    buf2.append(extraBytes);

    allocations.clear();
    success = atlasManager.getAssociatedProxies(allocations);
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, allocations.size() == 1);
    // Height should have doubled from 32 to 64
    REPORTER_ASSERT(reporter, allocations[0].second->dimensions().height() == 64);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasOverflowToSecondTexture,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Fill Page 1 beyond max single texture size (4096 rows * 32,768 bytes = 128 MB)
    SkTDArray<uint8_t>& buf = atlasManager.getAlphaBuffer();
    int32_t page1Bytes = AlphaAtlasManager::kMaxCapBytes;
    int32_t overflowBytes = AlphaAtlasManager::kAtlasDimsBytes * 10;
    buf.append(page1Bytes + overflowBytes);

    SkTDArray<std::pair<int32_t, sk_sp<TextureProxy>>> allocations;
    bool success = atlasManager.getAssociatedProxies(allocations);
    REPORTER_ASSERT(reporter, success);
    // Should allocate across 2 texture pages
    REPORTER_ASSERT(reporter, allocations.size() == 2);
    REPORTER_ASSERT(reporter, allocations[0].second != allocations[1].second);
    REPORTER_ASSERT(
            reporter,
            allocations[0].second->dimensions().height() == AlphaAtlasManager::kMaxAtlasHeight);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasBailoutWhenFull,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Exceed total capacity across all 2 pages (2 * 128 MB = 256 MB)
    SkTDArray<uint8_t>& buf = atlasManager.getAlphaBuffer();
    int32_t maxTotalBytes = AlphaAtlasManager::kMaxCapBytes * AlphaAtlasManager::kMaxPages;
    int32_t excessBytes = AlphaAtlasManager::kAtlasDimsBytes * 100;
    buf.append(maxTotalBytes + excessBytes);

    SkTDArray<std::pair<int32_t, sk_sp<TextureProxy>>> allocations;
    bool success = atlasManager.getAssociatedProxies(allocations);
    // Must return false (bailout) when running out of room
    REPORTER_ASSERT(reporter, !success);
    REPORTER_ASSERT(reporter, allocations.empty());
}

}  // namespace skgpu::graphite
