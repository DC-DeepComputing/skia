/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkGradient.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/StorageBufferManager.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

namespace skgpu::graphite {

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageBufferManagerGradientsOnlyTest, reporter, context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkPoint pts[2] = {{0, 0}, {100, 100}};
    SkColor4f colors[2] = {SkColors::kRed, SkColors::kBlue};
    sk_sp<SkShader> shader1 = SkShaders::LinearGradient(pts, {{colors, {}, SkTileMode::kClamp}, {}});
    sk_sp<SkShader> shader2 = SkShaders::LinearGradient(pts, {{colors, {}, SkTileMode::kRepeat}, {}});

    const auto* grad1 = static_cast<const SkGradientBaseShader*>(shader1.get());
    const auto* grad2 = static_cast<const SkGradientBaseShader*>(shader2.get());

    StorageContext ctxStorage;
    StorageContext* ctxHandle = &ctxStorage;

    // 1. Allocate gradient data for shader1
    auto [ptr1, offset1] = ctxHandle->allocateGradientData(2, grad1);
    REPORTER_ASSERT(reporter, ptr1 != nullptr);
    REPORTER_ASSERT(reporter, offset1 == 0);

    // 2. Allocate gradient data again for shader1 (deduplication check)
    auto [ptr1Dup, offset1Dup] = ctxHandle->allocateGradientData(2, grad1);
    REPORTER_ASSERT(reporter, ptr1Dup == nullptr);
    REPORTER_ASSERT(reporter, offset1Dup == offset1);

    // 3. Allocate gradient data for shader2
    auto [ptr2, offset2] = ctxHandle->allocateGradientData(2, grad2);
    REPORTER_ASSERT(reporter, ptr2 != nullptr);
    REPORTER_ASSERT(reporter, offset2 == 10); // 2 stops * 5 floats = 10 floats offset

    // Finalize
    auto bindInfo = ctxHandle->finalize(recorder->priv().drawBufferManager());
    REPORTER_ASSERT(reporter, bindInfo.fBuffer != nullptr);
    REPORTER_ASSERT(reporter, bindInfo.fSize == 80);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageBufferManagerAppendOnlyTest, reporter, context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    StorageContext ctxStorage;
    StorageContext* ctxHandle = &ctxStorage;
    ctxHandle->finalizeCacheStorageData();

    // Append 1: 4 elements of 8 bytes = 32 bytes
    auto alloc1 = ctxHandle->appendVertexStorage(4, 8);
    REPORTER_ASSERT(reporter, static_cast<bool>(alloc1.fWriter));
    REPORTER_ASSERT(reporter, alloc1.fOffset == 0);

    // Append 2: 2 elements of 16 bytes = 32 bytes (aligned to 16 bytes)
    auto alloc2 = ctxHandle->appendVertexStorage(2, 16);
    REPORTER_ASSERT(reporter, static_cast<bool>(alloc2.fWriter));
    REPORTER_ASSERT(reporter, alloc2.fOffset == 32);

    auto bindInfo = ctxHandle->finalize(recorder->priv().drawBufferManager());
    REPORTER_ASSERT(reporter, bindInfo.fBuffer != nullptr);
    REPORTER_ASSERT(reporter, bindInfo.fSize == 64);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageBufferManagerGradientsAndAppendTest, reporter, context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkPoint pts[2] = {{0, 0}, {100, 100}};
    SkColor4f colors[2] = {SkColors::kRed, SkColors::kBlue};
    sk_sp<SkShader> shader = SkShaders::LinearGradient(pts, {{colors, {}, SkTileMode::kClamp}, {}});
    const auto* grad = static_cast<const SkGradientBaseShader*>(shader.get());

    StorageContext ctxStorage;
    StorageContext* ctxHandle = &ctxStorage;

    // Gradient allocation: 2 stops * 5 floats * 4 bytes = 40 bytes
    auto [gPtr, gOffset] = ctxHandle->allocateGradientData(2, grad);
    REPORTER_ASSERT(reporter, gPtr != nullptr);
    REPORTER_ASSERT(reporter, gOffset == 0);

    // Finalize cached gradient storage region (pads to 16 bytes: 40 -> 48 bytes)
    ctxHandle->finalizeCacheStorageData();

    // Appended allocation: Should be 16-byte aligned relative to 40 gradient bytes -> starts at byte 48!
    auto alloc = ctxHandle->appendVertexStorage(4, 8);
    REPORTER_ASSERT(reporter, static_cast<bool>(alloc.fWriter));
    REPORTER_ASSERT(reporter, alloc.fOffset == 48);

    auto bindInfo = ctxHandle->finalize(recorder->priv().drawBufferManager());
    REPORTER_ASSERT(reporter, bindInfo.fBuffer != nullptr);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageBufferManagerMultiStreamTest, reporter, context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkPoint pts[2] = {{0, 0}, {100, 100}};
    SkColor4f colors[2] = {SkColors::kRed, SkColors::kBlue};
    sk_sp<SkShader> shader1 = SkShaders::LinearGradient(pts, {{colors, {}, SkTileMode::kClamp}, {}});
    sk_sp<SkShader> shader2 = SkShaders::LinearGradient(pts, {{colors, {}, SkTileMode::kRepeat}, {}});
    const auto* grad1 = static_cast<const SkGradientBaseShader*>(shader1.get());
    const auto* grad2 = static_cast<const SkGradientBaseShader*>(shader2.get());

    // Stream 0: Add to gradient (new grad1), append
    StorageContext ctxStorage0;
    StorageContext* ctx0 = &ctxStorage0;
    auto [gPtr0, gOffset0] = ctx0->allocateGradientData(2, grad1);
    REPORTER_ASSERT(reporter, gPtr0 != nullptr);
    REPORTER_ASSERT(reporter, gOffset0 == 0);
    ctx0->finalizeCacheStorageData();
    auto app0 = ctx0->appendVertexStorage(2, 8);
    REPORTER_ASSERT(reporter, app0.fOffset == 48);

    // Stream 1: Add to gradient (grad1 deduplicated against local cache), append
    StorageContext ctxStorage1;
    StorageContext* ctx1 = &ctxStorage1;
    auto [gPtr1_a, gOffset1_a] = ctx1->allocateGradientData(2, grad1);
    REPORTER_ASSERT(reporter, gPtr1_a != nullptr);
    REPORTER_ASSERT(reporter, gOffset1_a == 0);
    auto [gPtr1_b, gOffset1_b] = ctx1->allocateGradientData(2, grad1);
    REPORTER_ASSERT(reporter, gPtr1_b == nullptr); // Local deduplicated!
    REPORTER_ASSERT(reporter, gOffset1_b == gOffset1_a);
    ctx1->finalizeCacheStorageData();
    auto app1 = ctx1->appendVertexStorage(4, 8);
    REPORTER_ASSERT(reporter, app1.fOffset == 48);

    // Stream 2: Add to gradient (grad1), add to gradient (cached grad1), add to gradient (new grad2), append
    StorageContext ctxStorage2;
    StorageContext* ctx2 = &ctxStorage2;
    auto [gPtr2_a, gOffset2_a] = ctx2->allocateGradientData(2, grad1);
    REPORTER_ASSERT(reporter, gPtr2_a != nullptr);
    REPORTER_ASSERT(reporter, gOffset2_a == 0);
    auto [gPtr2_b, gOffset2_b] = ctx2->allocateGradientData(2, grad1);
    REPORTER_ASSERT(reporter, gPtr2_b == nullptr);
    REPORTER_ASSERT(reporter, gOffset2_b == gOffset2_a);
    auto [gPtr2_c, gOffset2_c] = ctx2->allocateGradientData(2, grad2);
    REPORTER_ASSERT(reporter, gPtr2_c != nullptr);
    REPORTER_ASSERT(reporter, gOffset2_c == 10); // 10 floats offset
    ctx2->finalizeCacheStorageData();
    auto app2 = ctx2->appendVertexStorage(2, 8);
    REPORTER_ASSERT(reporter, app2.fOffset == 80); // 80 is 16-byte aligned

    // Finalize all streams independently
    auto bind0 = ctx0->finalize(recorder->priv().drawBufferManager());
    REPORTER_ASSERT(reporter, bind0.fBuffer != nullptr);

    auto bind1 = ctx1->finalize(recorder->priv().drawBufferManager());
    REPORTER_ASSERT(reporter, bind1.fBuffer != nullptr);

    auto bind2 = ctx2->finalize(recorder->priv().drawBufferManager());
    REPORTER_ASSERT(reporter, bind2.fBuffer != nullptr);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageBufferManagerInterleavedMultiStreamTest, reporter, context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkPoint pts[2] = {{0, 0}, {100, 100}};
    SkColor4f colors[2] = {SkColors::kRed, SkColors::kBlue};
    sk_sp<SkShader> shader1 = SkShaders::LinearGradient(pts, {{colors, {}, SkTileMode::kClamp}, {}});
    sk_sp<SkShader> shader2 = SkShaders::LinearGradient(pts, {{colors, {}, SkTileMode::kRepeat}, {}});
    const auto* grad1 = static_cast<const SkGradientBaseShader*>(shader1.get());
    const auto* grad2 = static_cast<const SkGradientBaseShader*>(shader2.get());

    // 1. Context Registration & Gradient Allocations
    StorageContext ctxStorage0;
    StorageContext* ctx0 = &ctxStorage0;
    auto [g0_s1, off0_s1] = ctx0->allocateGradientData(2, grad1);
    REPORTER_ASSERT(reporter, g0_s1 != nullptr);
    REPORTER_ASSERT(reporter, off0_s1 == 0);

    StorageContext ctxStorage1;
    StorageContext* ctx1 = &ctxStorage1;
    auto [g1_s1, off1_s1] = ctx1->allocateGradientData(2, grad1);
    REPORTER_ASSERT(reporter, g1_s1 != nullptr);
    REPORTER_ASSERT(reporter, off1_s1 == 0);

    // Interleave: allocating shader2 into ctx0
    auto [g0_s2, off0_s2] = ctx0->allocateGradientData(2, grad2);
    REPORTER_ASSERT(reporter, g0_s2 != nullptr);
    REPORTER_ASSERT(reporter, off0_s2 == 10);

    StorageContext ctxStorage2;
    StorageContext* ctx2 = &ctxStorage2;
    auto [g2_s1, off2_s1] = ctx2->allocateGradientData(2, grad1);
    REPORTER_ASSERT(reporter, g2_s1 != nullptr);
    REPORTER_ASSERT(reporter, off2_s1 == 0);

    // Interleave: duplicate allocation in ctx1
    auto [g1_s1_dup, off1_s1_dup] = ctx1->allocateGradientData(2, grad1);
    REPORTER_ASSERT(reporter, g1_s1_dup == nullptr);
    REPORTER_ASSERT(reporter, off1_s1_dup == off1_s1);

    // Interleave: allocating shader2 into ctx2
    auto [g2_s2, off2_s2] = ctx2->allocateGradientData(2, grad2);
    REPORTER_ASSERT(reporter, g2_s2 != nullptr);
    REPORTER_ASSERT(reporter, off2_s2 == 10);

    // 2. Interleaved Cache Storage Finalization
    ctx0->finalizeCacheStorageData();
    ctx1->finalizeCacheStorageData();
    ctx2->finalizeCacheStorageData();

    // 3. Heavily Interleaved Appends Across Streams
    // Round 1
    auto a0_1 = ctx0->appendVertexStorage(2, 8); // 16 bytes
    REPORTER_ASSERT(reporter, a0_1.fOffset == 80);

    auto a1_1 = ctx1->appendVertexStorage(1, 16); // 16 bytes
    REPORTER_ASSERT(reporter, a1_1.fOffset == 48);

    auto a2_1 = ctx2->appendVertexStorage(4, 8); // 32 bytes
    REPORTER_ASSERT(reporter, a2_1.fOffset == 80);

    // Round 2 (interleaved append order)
    auto a0_2 = ctx0->appendVertexStorage(3, 16); // 48 bytes
    REPORTER_ASSERT(reporter, a0_2.fOffset == 96); // 80 + 16

    auto a1_2 = ctx1->appendVertexStorage(2, 8); // 16 bytes
    REPORTER_ASSERT(reporter, a1_2.fOffset == 64); // 48 + 16

    auto a2_2 = ctx2->appendVertexStorage(2, 16); // 32 bytes
    REPORTER_ASSERT(reporter, a2_2.fOffset == 112); // 80 + 32

    // Round 3 (interleaved append order)
    auto a0_3 = ctx0->appendVertexStorage(1, 8); // 8 bytes
    REPORTER_ASSERT(reporter, a0_3.fOffset == 144); // 96 + 48

    auto a1_3 = ctx1->appendVertexStorage(4, 16); // 64 bytes
    REPORTER_ASSERT(reporter, a1_3.fOffset == 80); // 64 + 16

    auto a2_3 = ctx2->appendVertexStorage(1, 8); // 8 bytes
    REPORTER_ASSERT(reporter, a2_3.fOffset == 144); // 112 + 32

    // 4. Finalize all streams independently
    auto bind0 = ctx0->finalize(recorder->priv().drawBufferManager());
    REPORTER_ASSERT(reporter, bind0.fBuffer != nullptr);

    auto bind1 = ctx1->finalize(recorder->priv().drawBufferManager());
    REPORTER_ASSERT(reporter, bind1.fBuffer != nullptr);

    auto bind2 = ctx2->finalize(recorder->priv().drawBufferManager());
    REPORTER_ASSERT(reporter, bind2.fBuffer != nullptr);
}

}  // namespace skgpu::graphite
