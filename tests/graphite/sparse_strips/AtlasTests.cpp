/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

namespace skgpu::graphite {

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasDimensionsAndStride,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());
    EndCaps<8> ends;

    // Initial allocation (e.g. 512 bytes = 8 tiles of 8x8)
    auto alloc1 = atlasManager.requestAlphaSpace(ends, 512);
    REPORTER_ASSERT(reporter, alloc1.fSuccess);
    REPORTER_ASSERT(reporter, alloc1.fAlphaIndex == 0);
    REPORTER_ASSERT(reporter, alloc1.fTexPage == 0);
    REPORTER_ASSERT(reporter, ends.proxies().size() == 1);

    // Initial page dimensions: 8192 x 1
    sk_sp<TextureProxy> proxy = ends.proxies()[0];
    REPORTER_ASSERT(reporter, proxy != nullptr);
    REPORTER_ASSERT(reporter, proxy->dimensions().width() == SparseStripConfig::kAtlasWidth);
    REPORTER_ASSERT(reporter, proxy->dimensions().height() == 1);

    // Verify row byte stride alignment (32,768 bytes/row = 8192 pixels * 4 bytes/pixel)
    int32_t strideBytes = SparseStripConfig::kAtlasWidthBytes;
    REPORTER_ASSERT(reporter, strideBytes == 32768);

    // Subsequent allocation on same row/page
    auto alloc2 = atlasManager.requestAlphaSpace(ends, 1024);
    REPORTER_ASSERT(reporter, alloc2.fSuccess);
    REPORTER_ASSERT(reporter, alloc2.fAlphaIndex == 512);
    REPORTER_ASSERT(reporter, alloc2.fTexPage == 0);
    REPORTER_ASSERT(reporter, ends.proxies().size() == 1);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasDynamicGrowthAndDoubling,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());
    EndCaps<8> ends;

    // Page 1: allocate exactly 1 row (32,768 bytes)
    auto alloc1 = atlasManager.requestAlphaSpace(ends, SparseStripConfig::kAtlasWidthBytes);
    REPORTER_ASSERT(reporter, alloc1.fSuccess);
    REPORTER_ASSERT(reporter, alloc1.fTexPage == 0);
    REPORTER_ASSERT(reporter, ends.proxies()[0]->dimensions().height() == 1);

    // Page 2: allocating 1 more tile exceeds Page 1, triggers doubling -> 2 rows (65,536 bytes)
    auto alloc2 = atlasManager.requestAlphaSpace(ends, 64);
    REPORTER_ASSERT(reporter, alloc2.fSuccess);
    REPORTER_ASSERT(reporter, alloc2.fAlphaIndex == 0); // Atomic on new page, starts at 0
    REPORTER_ASSERT(reporter, alloc2.fTexPage == 1);
    REPORTER_ASSERT(reporter, ends.proxies().size() == 2);
    REPORTER_ASSERT(reporter, ends.proxies()[1]->dimensions().height() == 2);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasGracefulFailureOnThirdPage,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());
    EndCaps<8> ends;

    // Page 1: 1 row
    auto alloc1 = atlasManager.requestAlphaSpace(ends, SparseStripConfig::kAtlasWidthBytes);
    REPORTER_ASSERT(reporter, alloc1.fSuccess);
    REPORTER_ASSERT(reporter, ends.proxies().size() == 1);

    // Page 2: 2 rows
    auto alloc2 = atlasManager.requestAlphaSpace(ends, SparseStripConfig::kAtlasWidthBytes * 2);
    REPORTER_ASSERT(reporter, alloc2.fSuccess);
    REPORTER_ASSERT(reporter, ends.proxies().size() == 2);

    // Attempting to exceed Page 2 within the same draw: needs a 3rd distinct page,
    // which exceeds kMaxTexturePages (2) for this draw. Should fail gracefully!
    auto alloc3 = atlasManager.requestAlphaSpace(ends, 64);
    REPORTER_ASSERT(reporter, !alloc3.fSuccess);
    REPORTER_ASSERT(reporter, alloc3.fWritePtr == nullptr);
    REPORTER_ASSERT(reporter, ends.proxies().size() == 2);
}

}  // namespace skgpu::graphite
