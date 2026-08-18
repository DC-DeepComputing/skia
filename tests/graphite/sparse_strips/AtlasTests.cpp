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

    // Add 1 row of data (16,384 bytes)
    buf.append(AlphaAtlasManager::kAtlasWidthBytes);

    SkTDArray<std::pair<int32_t, sk_sp<TextureProxy>>> allocations;
    atlasManager.getAssociatedProxies(allocations);
    REPORTER_ASSERT(reporter, allocations.size() == 1);

    // Verify Texture dimensions (4096 x 4096)
    sk_sp<TextureProxy> proxy = allocations[0].second;
    REPORTER_ASSERT(reporter, proxy != nullptr);
    REPORTER_ASSERT(reporter, proxy->dimensions().width() == AlphaAtlasManager::kAtlasWidth);
    REPORTER_ASSERT(reporter, proxy->dimensions().height() == AlphaAtlasManager::kAtlasHeight);

    // Verify row byte stride alignment (16,384 bytes/row = 4096 pixels * 4 bytes/pixel)
    int32_t strideBytes = AlphaAtlasManager::kAtlasWidthBytes;
    REPORTER_ASSERT(reporter, strideBytes == AlphaAtlasManager::kAtlasWidth * 4);
    REPORTER_ASSERT(reporter, allocations[0].first == AlphaAtlasManager::kAtlasWidthBytes);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasOverflowToSecondTexture,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Fill Page 1 beyond max single texture size (4096 rows * 16,384 bytes = 64 MB)
    SkTDArray<uint8_t>& buf = atlasManager.getAlphaBuffer();
    int32_t page1Bytes = AlphaAtlasManager::kMaxCapBytes;
    int32_t overflowBytes = AlphaAtlasManager::kAtlasWidthBytes * 10;
    buf.append(page1Bytes + overflowBytes);

    SkTDArray<std::pair<int32_t, sk_sp<TextureProxy>>> allocations;
    atlasManager.getAssociatedProxies(allocations);
    // Should allocate across 2 texture pages
    REPORTER_ASSERT(reporter, allocations.size() == 2);
    REPORTER_ASSERT(reporter, allocations[0].second != allocations[1].second);
    REPORTER_ASSERT(reporter,
                    allocations[0].second->dimensions().height() == AlphaAtlasManager::kAtlasHeight);
}

}  // namespace skgpu::graphite
