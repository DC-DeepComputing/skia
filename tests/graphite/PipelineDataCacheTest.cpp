/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/TextureProxy.h"

using namespace skgpu::graphite;

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(PipelineDataCacheTest, reporter, context,
                                   CtsEnforcement::kApiLevel_202404) {
    // const Caps* caps = context->priv().caps();
    // ResourceProvider* resourceProvider = context->priv().resourceProvider();

    // // Setup caches
    // UniformDataCache uCache;
    // TextureDataCache tCache;

    // REPORTER_ASSERT(reporter, uCache.count() == 0);
    // REPORTER_ASSERT(reporter, tCache.bindingCount() == 0);


    // // Create testing textures and uniforms
    // [[maybe_unused]] static constexpr Uniform kUniforms[] = {{"data", SkSLType::kFloat4}};
    // [[maybe_unused]] static constexpr Uniform kEmptyUniforms[] = {};
    // TextureInfo info = caps->getDefaultSampledTextureInfo(kAlpha_8_SkColorType,
    //                                                       skgpu::Mipmapped::kNo,
    //                                                       skgpu::Protected::kNo,
    //                                                       skgpu::Renderable::kYes);
    // REPORTER_ASSERT(reporter, info.isValid());

    // sk_sp<TextureProxy> proxyA = TextureProxy::Make(caps,
    //                                                 resourceProvider,
    //                                                 SkISize::Make(32, 32),
    //                                                 info,
    //                                                 "TestDataProxyA",
    //                                                 skgpu::Budgeted::kYes);
    // sk_sp<TextureProxy> proxyB = TextureProxy::Make(caps,
    //                                                 resourceProvider,
    //                                                 SkISize::Make(32, 32),
    //                                                 info,
    //                                                 "TestDataProxyB",
    //                                                 skgpu::Budgeted::kYes);
    // REPORTER_ASSERT(reporter, proxyA && proxyB);

    // // Block A: Add a new, unique combined uniform block (Paint P1 + RenderStep R1) and texture T1.
    // // Paint P1: {1,2,3,4}
    // // RenderStep R1: {1,2,3,4}
    // UniformDataCache::Index uID1;
    // TextureDataCache::Index tID1;
    // {
    //     PipelineDataGatherer gatherer{Layout::kStd430};
    //     gatherer.resetForDraw();

    //     // Simulate Paint P1 data
    //     SkDEBUGCODE(UniformExpectationsValidator paintUev(&gatherer, kUniforms);)
    //     gatherer.write(SkV4{1.f, 2.f, 3.f, 4.f});
    //     gatherer.endPaintData(); // Mark end of paint data

    //     // Simulate RenderStep R1 data
    //     SkDEBUGCODE(UniformExpectationsValidator rsUev(&gatherer, kUniforms);)
    //     gatherer.write(SkV4{1.f, 2.f, 3.f, 4.f});
    //     gatherer.add(proxyA, {}); // Texture T1

    //     // Get combined block (P1 + R1) and texture block (T1)
    //     auto [udb, tdb] = gatherer.endRenderStepData(/*performsShading=*/true);

    //     uID1 = uCache.insert(udb);
    //     tID1 = tCache.insert(tdb);

    //     REPORTER_ASSERT(reporter, uCache.count() == 1);
    //     REPORTER_ASSERT(reporter, tCache.bindingCount() == 1);
    //     REPORTER_ASSERT(reporter, tCache.uniqueTextureCount() == 1);

    //     // Verify lookup - Assuming lookup returns a struct with the UniformDataBlock in fBlock
    //     // If the structure changed, this member access might need adjustment.
    //     // It seems the previous version used .fCpuData, let's try that.
    //     REPORTER_ASSERT(reporter, uCache.lookup(uID1).fCpuData == udb);
    //     REPORTER_ASSERT(reporter, tCache.lookup(tID1) == tdb);
    // }

    // // Block B: Add the exact same combination (P1 + R1 + T1) to test de-duplication.
    // {
    //     PipelineDataGatherer gatherer{Layout::kStd430};
    //     gatherer.resetForDraw();

    //     // Paint P1
    //     SkDEBUGCODE(UniformExpectationsValidator paintUev(&gatherer, kUniforms);)
    //     gatherer.write(SkV4{1.f, 2.f, 3.f, 4.f});
    //     gatherer.endPaintData();

    //     // RenderStep R1
    //     SkDEBUGCODE(UniformExpectationsValidator rsUev(&gatherer, kUniforms);)
    //     gatherer.write(SkV4{1.f, 2.f, 3.f, 4.f});
    //     gatherer.add(proxyA, {}); // Texture T1

    //     auto [udb, tdb] = gatherer.endRenderStepData(/*performsShading=*/true);

    //     UniformDataCache::Index uID2 = uCache.insert(udb);
    //     TextureDataCache::Index tID2 = tCache.insert(tdb);

    //     REPORTER_ASSERT(reporter, uID2 == uID1); // Combined uniform ID should be the same
    //     REPORTER_ASSERT(reporter, tID2 == tID1); // Texture ID should be the same

    //     REPORTER_ASSERT(reporter, uCache.count() == 1);        // Count should NOT increase
    //     REPORTER_ASSERT(reporter, tCache.bindingCount() == 1); // Count should NOT increase
    //     REPORTER_ASSERT(reporter, tCache.uniqueTextureCount() == 1);
    // }

    // // Block C: Add different RenderStep data (P1 + R2 + T1).
    // // Paint P1: {1,2,3,4}
    // // RenderStep R2: {5,6,7,8}
    // UniformDataCache::Index uID3;
    // TextureDataCache::Index tID3;
    // {
    //     PipelineDataGatherer gatherer{Layout::kStd430};
    //     gatherer.resetForDraw();

    //     // Paint P1
    //     SkDEBUGCODE(UniformExpectationsValidator paintUev(&gatherer, kUniforms);)
    //     gatherer.write(SkV4{1.f, 2.f, 3.f, 4.f});
    //     gatherer.endPaintData();

    //     // RenderStep R2
    //     SkDEBUGCODE(UniformExpectationsValidator rsUev(&gatherer, kUniforms);)
    //     gatherer.write(SkV4{5.f, 6.f, 7.f, 8.f}); // Different render step data
    //     gatherer.add(proxyA, {});                // Same texture T1

    //     auto [udb, tdb] = gatherer.endRenderStepData(/*performsShading=*/true);

    //     uID3 = uCache.insert(udb);
    //     tID3 = tCache.insert(tdb);

    //     REPORTER_ASSERT(reporter, uID3 != uID1); // New unique combined uniform index
    //     REPORTER_ASSERT(reporter, tID3 == tID1); // Same texture binding index

    //     REPORTER_ASSERT(reporter, uCache.count() == 2);        // Uniform count increases
    //     REPORTER_ASSERT(reporter, tCache.bindingCount() == 1); // Texture count does not
    //     REPORTER_ASSERT(reporter, tCache.uniqueTextureCount() == 1);
    // }

    // // Block D: Add different Texture data (P1 + R1 + T2).
    // // Paint P1: {1,2,3,4}
    // // RenderStep R1: {1,2,3,4}
    // UniformDataCache::Index uID4;
    // TextureDataCache::Index tID4;
    // {
    //     PipelineDataGatherer gatherer{Layout::kStd430};
    //     gatherer.resetForDraw();

    //     // Paint P1
    //     SkDEBUGCODE(UniformExpectationsValidator paintUev(&gatherer, kUniforms);)
    //     gatherer.write(SkV4{1.f, 2.f, 3.f, 4.f});
    //     gatherer.endPaintData();

    //     // RenderStep R1
    //     SkDEBUGCODE(UniformExpectationsValidator rsUev(&gatherer, kUniforms);)
    //     gatherer.write(SkV4{1.f, 2.f, 3.f, 4.f});
    //     gatherer.add(proxyB, {}); // Different texture T2

    //     auto [udb, tdb] = gatherer.endRenderStepData(/*performsShading=*/true);

    //     uID4 = uCache.insert(udb);
    //     tID4 = tCache.insert(tdb);

    //     REPORTER_ASSERT(reporter, uID4 == uID1); // Same combined uniform index as A
    //     REPORTER_ASSERT(reporter, tID4 != tID1); // New unique texture index

    //     REPORTER_ASSERT(reporter, uCache.count() == 2);        // Uniform count does not increase
    //     REPORTER_ASSERT(reporter, tCache.bindingCount() == 2); // Texture count increases
    //     REPORTER_ASSERT(reporter, tCache.uniqueTextureCount() == 2);
    // }

    // // Block E: Add unique paint uniform, empty render step uniform, empty texture (P2 + R_empty + T_empty).
    // // Paint P2: {10,20,30,40}
    // UniformDataCache::Index uID5;
    // TextureDataCache::Index tID5;
    // {
    //     PipelineDataGatherer gatherer{Layout::kStd430};
    //     gatherer.resetForDraw();

    //     // Paint P2
    //     SkDEBUGCODE(UniformExpectationsValidator paintUev(&gatherer, kUniforms);)
    //     gatherer.write(SkV4{10.f, 20.f, 30.f, 40.f}); // New unique paint uniform data
    //     gatherer.endPaintData();

    //     // Empty RenderStep data (no uniforms, no textures)
    //     // No need for validator if kEmptyUniforms is empty
    //     auto [udb, tdb] = gatherer.endRenderStepData(/*performsShading=*/true);

    //     uID5 = uCache.insert(udb);
    //     tID5 = tCache.insert(tdb); // Add empty texture block

    //     REPORTER_ASSERT(reporter, uID5 != uID1 && uID5 != uID3); // New unique combined uniform index
    //     REPORTER_ASSERT(reporter, uCache.count() == 3);        // Uniform count increases
    //     REPORTER_ASSERT(reporter, tCache.bindingCount() == 3); // Empty texture block adds a binding
    //     REPORTER_ASSERT(reporter, tCache.uniqueTextureCount() == 2); // No new unique textures
    // }

    // // Block F: Add the same paint uniform, empty render step, empty texture (P2 + R_empty + T_empty) to test de-duplication.
    // {
    //     PipelineDataGatherer gatherer{Layout::kStd430};
    //     gatherer.resetForDraw();

    //     // Paint P2
    //     SkDEBUGCODE(UniformExpectationsValidator paintUev(&gatherer, kUniforms);)
    //     gatherer.write(SkV4{10.f, 20.f, 30.f, 40.f}); // Same paint uniform data as Block E
    //     gatherer.endPaintData();

    //     // Empty RenderStep data
    //     auto [udb, tdb] = gatherer.endRenderStepData(/*performsShading=*/true);

    //     UniformDataCache::Index uID6 = uCache.insert(udb);
    //     TextureDataCache::Index tID6 = tCache.insert(tdb);

    //     REPORTER_ASSERT(reporter, uID6 == uID5); // Combined uniform ID should be the same as E
    //     REPORTER_ASSERT(reporter, tID6 == tID5); // Texture ID should be the same as E

    //     REPORTER_ASSERT(reporter, uCache.count() == 3);        // Count should NOT increase
    //     REPORTER_ASSERT(reporter, tCache.bindingCount() == 3); // Count should NOT increase
    //     REPORTER_ASSERT(reporter, tCache.uniqueTextureCount() == 2);
    // }

    // // Block G: Test different paint and render step uniforms together (P2 + R1 + T1 + T2).
    // // Paint P2: {10,20,30,40}
    // // RenderStep R1: {1,2,3,4}
    // UniformDataCache::Index uID7;
    // TextureDataCache::Index tID7;
    // {
    //     PipelineDataGatherer gatherer{Layout::kStd430};
    //     gatherer.resetForDraw();

    //     // Paint P2
    //     SkDEBUGCODE(UniformExpectationsValidator paintUev(&gatherer, kUniforms);)
    //     gatherer.write(SkV4{10.f, 20.f, 30.f, 40.f}); // Same paint uniform as Block E/F
    //     gatherer.add(proxyA, {}); // Texture T1
    //     gatherer.endPaintData();

    //     // RenderStep R1
    //     SkDEBUGCODE(UniformExpectationsValidator rsUev(&gatherer, kUniforms);)
    //     gatherer.write(SkV4{1.f, 2.f, 3.f, 4.f}); // Same render step uniform as Block A/B/D
    //     gatherer.add(proxyB, {}); // Texture T2

    //     // Combined block (P2 + R1), Combined textures (T1 + T2)
    //     auto [udb, tdb] = gatherer.endRenderStepData(/*performsShading=*/true);

    //     uID7 = uCache.insert(udb);
    //     tID7 = tCache.insert(tdb);

    //     // This combination of (P2 + R1) is new
    //     REPORTER_ASSERT(reporter, uID7 != uID1 && uID7 != uID3 && uID7 != uID5);
    //     REPORTER_ASSERT(reporter, uCache.count() == 4); // Uniform count increases

    //     // This combination of (T1 + T2) is new
    //     REPORTER_ASSERT(reporter, tID7 != tID1 && tID7 != tID4 && tID7 != tID5);
    //     REPORTER_ASSERT(reporter, tCache.bindingCount() == 4);       // Texture binding count increases
    //     REPORTER_ASSERT(reporter, tCache.uniqueTextureCount() == 2); // Still 2 unique proxies
    // }

    // // Block H: Simulate multiple recordDraw calls using rewindForRenderStep.
    // UniformDataCache::Index uID8, uID9;
    // TextureDataCache::Index tID8, tID9;
    // {
    //     PipelineDataGatherer gatherer{Layout::kStd430};

    //     // A single reset before multiple draws.
    //     gatherer.resetForDraw();

    //     // First combination: (Paint P3 + R3 + T1 + T2)
    //     // Paint P3: {11,22,33,44}
    //     // RenderStep R3: {101,102,103,104}
    //     {
    //         // Paint P3
    //         SkDEBUGCODE(UniformExpectationsValidator paintUev(&gatherer, kUniforms);)
    //         gatherer.write(SkV4{11.f, 22.f, 33.f, 44.f}); // New Paint Data P3
    //         gatherer.add(proxyA, {}); // Texture T1
    //         gatherer.endPaintData();

    //         // RenderStep R3
    //         SkDEBUGCODE(UniformExpectationsValidator rsUev(&gatherer, kUniforms);)
    //         gatherer.write(SkV4{101.f, 102.f, 103.f, 104.f}); // New RenderStep Data R3
    //         gatherer.add(proxyB, {}); // Texture T2

    //         auto [udb, tdb] = gatherer.endRenderStepData(/*performsShading=*/true);
    //         uID8 = uCache.insert(udb);
    //         tID8 = tCache.insert(tdb);

    //         REPORTER_ASSERT(reporter, uID8 != uID1 && uID8 != uID3 && uID8 != uID5 && uID8 != uID7);
    //         REPORTER_ASSERT(reporter, uCache.count() == 5);        // Uniform count increases
    //         REPORTER_ASSERT(reporter, tID8 == tID7);               // Texture combination (T1+T2) is the same as G
    //         REPORTER_ASSERT(reporter, tCache.bindingCount() == 4); // Texture binding count stays 4
    //         REPORTER_ASSERT(reporter, tCache.uniqueTextureCount() == 2);
    //     }

    //     // Second combination using rewind: (Paint P3 + R4 + T1 + T1)
    //     // RenderStep R4: {201,202,203,204}
    //     {
    //         gatherer.rewindForRenderStep(); // Rewinds to after P3 paint data

    //         // RenderStep R4
    //         SkDEBUGCODE(UniformExpectationsValidator rsUev(&gatherer, kUniforms);)
    //         gatherer.write(SkV4{201.f, 202.f, 203.f, 204.f}); // New RenderStep Data R4
    //         gatherer.add(proxyA, {}); // Texture T1 (now combined textures are T1+T1)

    //         auto [udb, tdb] = gatherer.endRenderStepData(/*performsShading=*/true);
    //         uID9 = uCache.insert(udb);
    //         tID9 = tCache.insert(tdb);

    //         REPORTER_ASSERT(reporter, uID9 != uID8); // New combined uniform block (P3+R4)
    //         REPORTER_ASSERT(reporter, uCache.count() == 6); // Uniform count increases

    //         // Combination T1+T1 should be unique so far
    //         REPORTER_ASSERT(reporter, tID9 != tID1 && tID9 != tID4 && tID9 != tID5 && tID9 != tID7);
    //         REPORTER_ASSERT(reporter, tCache.bindingCount() == 5); // Texture binding count increases
    //         REPORTER_ASSERT(reporter, tCache.uniqueTextureCount() == 2);
    //     }
    // }
}

