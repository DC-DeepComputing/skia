/*
 * Copyright 2026 Google LLC
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
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/StorageContext.h"
#include "src/gpu/graphite/task/DrawTask.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"
#include "src/shaders/gradients/SkLinearGradient.h"

#include <vector>

namespace skgpu::graphite {

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageContextAlignmentTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkPoint pts[2] = {{0, 0}, {100, 100}};
    SkColor4f colors[2] = {SkColors::kRed, SkColors::kBlue};
    auto grad1 =
            sk_make_sp<SkLinearGradient>(pts, SkGradient{{colors, {}, SkTileMode::kClamp}, {}});
    auto grad2 =
            sk_make_sp<SkLinearGradient>(pts, SkGradient{{colors, {}, SkTileMode::kRepeat}, {}});

    StorageContext ctxStorage;
    StorageContext* ctxHandle = &ctxStorage;

    // 1. Allocate gradient data for shader1 (2 stops: 4 floats offsets + 8 floats colors = 12 floats = 48 bytes)
    auto [ptr1, offset1] = ctxHandle->allocateGradientData(2, grad1.get());
    REPORTER_ASSERT(reporter, ptr1 != nullptr);
    REPORTER_ASSERT(reporter, offset1 == 0);
    for (int i = 0; i < 12; ++i) {
        ptr1[i] = 10.f + i;
    }

    // 2. Allocate gradient data again for shader1 (deduplication check)
    auto [ptr1Dup, offset1Dup] = ctxHandle->allocateGradientData(2, grad1.get());
    REPORTER_ASSERT(reporter, ptr1Dup == nullptr);
    REPORTER_ASSERT(reporter, offset1Dup == offset1);

    // 3. Allocate gradient data for shader2 (2 stops: 12 floats = 48 bytes)
    auto [ptr2, offset2] = ctxHandle->allocateGradientData(2, grad2.get());
    REPORTER_ASSERT(reporter, ptr2 != nullptr);
    REPORTER_ASSERT(reporter, offset2 == 12);  // 12 floats offset
    for (int i = 0; i < 12; ++i) {
        ptr2[i] = 30.f + i;
    }

    // Record vertex requirement with stride 16 and align 16, setting running LCM to 16
    ctxHandle->recordAlignment(/*stride=*/16, /*align=*/16);

    ctxHandle->finalizePrecachedStorageData();

    // Finalize storage buffer allocation and check 16-byte alignment
    DrawTask drawTask(/*target=*/nullptr);
    auto storageResult = ctxHandle->finalize(
            recorder->priv().drawBufferManager(), recorder.get(), &drawTask);
    if (std::holds_alternative<BindBufferInfo>(storageResult)) {
        auto bindInfo = std::get<BindBufferInfo>(storageResult);
        REPORTER_ASSERT(reporter, bindInfo.fBuffer != nullptr);
        REPORTER_ASSERT(reporter, bindInfo.fSize == 96);
        REPORTER_ASSERT(reporter, bindInfo.fSize % 16 == 0);

        const char* bufferData = static_cast<const char*>(
                const_cast<Buffer*>(bindInfo.fBuffer)->map()) + bindInfo.fOffset;
        const float* floatData = reinterpret_cast<const float*>(bufferData);
        for (int i = 0; i < 12; ++i) {
            REPORTER_ASSERT(reporter, floatData[i] == 10.f + i);
            REPORTER_ASSERT(reporter, floatData[12 + i] == 30.f + i);
        }
    } else {
        auto proxy = std::get<sk_sp<TextureProxy>>(storageResult);
        REPORTER_ASSERT(reporter, proxy != nullptr);
    }
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageContextPaddingAlignmentTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkPoint pts[2] = {{0, 0}, {100, 100}};
    SkColor4f colors[2] = {SkColors::kRed, SkColors::kBlue};
    auto grad = sk_make_sp<SkLinearGradient>(pts, SkGradient{{colors, {}, SkTileMode::kClamp}, {}});

    StorageContext ctxStorage;
    StorageContext* ctxHandle = &ctxStorage;

    // Allocate gradient data for 1 shader with 2 stops = 12 floats = 48 bytes
    auto [ptr, offset] = ctxHandle->allocateGradientData(2, grad.get());
    REPORTER_ASSERT(reporter, ptr != nullptr);
    REPORTER_ASSERT(reporter, offset == 0);
    for (int i = 0; i < 12; ++i) {
        ptr[i] = 30.f + i;
    }

    // Record vertex requirement with stride 32 and align 16, setting running LCM to 32
    ctxHandle->recordAlignment(/*stride=*/32, /*align=*/16);

    ctxHandle->finalizePrecachedStorageData();

    // Finalize: 48 bytes should be padded to 64 bytes (aligned to 32 bytes)
    DrawTask drawTask(/*target=*/nullptr);
    auto storageResult = ctxHandle->finalize(
            recorder->priv().drawBufferManager(), recorder.get(), &drawTask);
    if (std::holds_alternative<BindBufferInfo>(storageResult)) {
        auto bindInfo = std::get<BindBufferInfo>(storageResult);
        REPORTER_ASSERT(reporter, bindInfo.fBuffer != nullptr);
        REPORTER_ASSERT(reporter, bindInfo.fSize == 64);
        REPORTER_ASSERT(reporter, bindInfo.fSize % 32 == 0);

        const char* bufferData = static_cast<const char*>(
                const_cast<Buffer*>(bindInfo.fBuffer)->map()) + bindInfo.fOffset;
        const float* floatData = reinterpret_cast<const float*>(bufferData);
        for (int i = 0; i < 12; ++i) {
            REPORTER_ASSERT(reporter, floatData[i] == 30.f + i);
        }
        for (int i = 48; i < 64; ++i) {
            REPORTER_ASSERT(reporter, bufferData[i] == 0);
        }
    } else {
        auto proxy = std::get<sk_sp<TextureProxy>>(storageResult);
        REPORTER_ASSERT(reporter, proxy != nullptr);
    }
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageContextAppendVertexTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkPoint pts[2] = {{0, 0}, {100, 100}};
    SkColor4f colors[2] = {SkColors::kRed, SkColors::kBlue};
    auto grad = sk_make_sp<SkLinearGradient>(pts, SkGradient{{colors, {}, SkTileMode::kClamp}, {}});

    StorageContext ctxStorage;
    StorageContext* ctxHandle = &ctxStorage;

    // 1. Allocate gradient data (12 floats = 48 bytes)
    auto [gradPtr, gradOffset] = ctxHandle->allocateGradientData(2, grad.get());
    REPORTER_ASSERT(reporter, gradPtr != nullptr);
    REPORTER_ASSERT(reporter, gradOffset == 0);
    for (int i = 0; i < 12; ++i) {
        gradPtr[i] = 40.f + i;
    }

    // 2. Record vertex alignment requirement: stride 24, align 16 -> running LCM = 48
    ctxHandle->recordAlignment(/*stride=*/24, /*align=*/16);

    // 3. Finalize precached storage data: 48 bytes aligned to running LCM (48 bytes)
    ctxHandle->finalizePrecachedStorageData();

    // 4. Append vertices with stride 24, align 16, count 2 -> 48 bytes
    float verts[12] = {0.f, 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f};
    uint32_t vOffset = ctxHandle->appendVertices(verts, /*count=*/2, /*stride=*/24, /*align=*/16);
    REPORTER_ASSERT(reporter, vOffset == 48);

    // 5. Finalize storage buffer
    DrawTask drawTask(/*target=*/nullptr);
    auto storageResult = ctxHandle->finalize(
            recorder->priv().drawBufferManager(), recorder.get(), &drawTask);
    if (std::holds_alternative<BindBufferInfo>(storageResult)) {
        auto bindInfo = std::get<BindBufferInfo>(storageResult);
        REPORTER_ASSERT(reporter, bindInfo.fBuffer != nullptr);
        // Total size = 48 (aligned gradient) + 48 (vertices) = 96 bytes
        REPORTER_ASSERT(reporter, bindInfo.fSize == 96);
        REPORTER_ASSERT(reporter, bindInfo.fSize % 48 == 0);

        const char* bufferData = static_cast<const char*>(
                const_cast<Buffer*>(bindInfo.fBuffer)->map()) + bindInfo.fOffset;
        const float* floatData = reinterpret_cast<const float*>(bufferData);
        for (int i = 0; i < 12; ++i) {
            REPORTER_ASSERT(reporter, floatData[i] == 40.f + i);
        }
        REPORTER_ASSERT(reporter, memcmp(bufferData + 48, verts, sizeof(verts)) == 0);
    } else {
        auto proxy = std::get<sk_sp<TextureProxy>>(storageResult);
        REPORTER_ASSERT(reporter, proxy != nullptr);
    }
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageContextMultipleRenderStepsTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkPoint pts[2] = {{0, 0}, {100, 100}};
    SkColor4f colors[2] = {SkColors::kRed, SkColors::kBlue};
    auto grad = sk_make_sp<SkLinearGradient>(pts, SkGradient{{colors, {}, SkTileMode::kClamp}, {}});

    StorageContext ctxStorage;
    StorageContext* ctxHandle = &ctxStorage;

    // 1. Allocate gradient data (12 floats = 48 bytes)
    auto [gradPtr, gradOffset] = ctxHandle->allocateGradientData(2, grad.get());
    REPORTER_ASSERT(reporter, gradPtr != nullptr);
    REPORTER_ASSERT(reporter, gradOffset == 0);
    for (int i = 0; i < 12; ++i) {
        gradPtr[i] = 50.f + i;
    }

    // 2. Record alignments from multiple render steps:
    // Step A: stride 24, align 16 -> LCM = 48
    // Step B: stride 32, align 16 -> LCM = 32
    // Running LCM = LCM(48, 32) = 96
    ctxHandle->recordAlignment(/*stride=*/24, /*align=*/16);
    ctxHandle->recordAlignment(/*stride=*/32, /*align=*/16);

    // 3. Finalize precached storage data: 48 bytes aligned to running LCM (96 bytes)
    ctxHandle->finalizePrecachedStorageData();

    // 4. Step A appends 1 vertex of 24 bytes (stride 24, align 16)
    float dataA[6] = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f};
    uint32_t offsetA = ctxHandle->appendVertices(dataA, /*count=*/1, /*stride=*/24, /*align=*/16);
    REPORTER_ASSERT(reporter, offsetA == 96);

    // 5. Step B appends 1 vertex of 32 bytes (stride 32, align 16)
    // Local vertex size is currently 24; next 32-byte aligned offset is 32 (8 bytes zero-padding)
    float dataB[8] = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f};
    uint32_t offsetB = ctxHandle->appendVertices(dataB, /*count=*/1, /*stride=*/32, /*align=*/16);
    REPORTER_ASSERT(reporter, offsetB == 96 + 32);  // 128

    // 6. Finalize storage buffer: Total size = 96 (gradient) + 32 (local offset) + 32 (dataB) = 160
    DrawTask drawTask(/*target=*/nullptr);
    auto storageResult = ctxHandle->finalize(
            recorder->priv().drawBufferManager(), recorder.get(), &drawTask);
    if (std::holds_alternative<BindBufferInfo>(storageResult)) {
        auto bindInfo = std::get<BindBufferInfo>(storageResult);
        REPORTER_ASSERT(reporter, bindInfo.fBuffer != nullptr);
        REPORTER_ASSERT(reporter, bindInfo.fSize == 160);
        REPORTER_ASSERT(reporter, bindInfo.fSize % 32 == 0);

        const char* bufferData = static_cast<const char*>(
                const_cast<Buffer*>(bindInfo.fBuffer)->map()) + bindInfo.fOffset;
        const float* floatData = reinterpret_cast<const float*>(bufferData);
        for (int i = 0; i < 12; ++i) {
            REPORTER_ASSERT(reporter, floatData[i] == 50.f + i);
        }
        for (int i = 48; i < 96; ++i) {
            REPORTER_ASSERT(reporter, bufferData[i] == 0);
        }
        REPORTER_ASSERT(reporter, memcmp(bufferData + 96, dataA, sizeof(dataA)) == 0);
        for (int i = 120; i < 128; ++i) {
            REPORTER_ASSERT(reporter, bufferData[i] == 0);
        }
        REPORTER_ASSERT(reporter, memcmp(bufferData + 128, dataB, sizeof(dataB)) == 0);
    } else {
        auto proxy = std::get<sk_sp<TextureProxy>>(storageResult);
        REPORTER_ASSERT(reporter, proxy != nullptr);
    }
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageContextLCMVariantsTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    struct TestCase {
        size_t stride;
        size_t align;
        uint32_t expectedLCM;
    };

    const TestCase testCases[] = {
            {/*stride=*/12, /*align=*/16, /*expectedLCM=*/48},
            {/*stride=*/20, /*align=*/16, /*expectedLCM=*/80},
            {/*stride=*/24, /*align=*/16, /*expectedLCM=*/48},
            {/*stride=*/28, /*align=*/16, /*expectedLCM=*/112},
            {/*stride=*/36, /*align=*/16, /*expectedLCM=*/144},
            {/*stride=*/40, /*align=*/16, /*expectedLCM=*/80},
            {/*stride=*/64, /*align=*/16, /*expectedLCM=*/64},
    };

    SkPoint pts[2] = {{0, 0}, {100, 100}};
    SkColor4f colors[2] = {SkColors::kRed, SkColors::kBlue};
    auto grad = sk_make_sp<SkLinearGradient>(pts, SkGradient{{colors, {}, SkTileMode::kClamp}, {}});

    for (const auto& tc : testCases) {
        StorageContext ctx;
        // Allocate 2 gradient stops = 12 floats = 48 bytes
        auto [gradPtr, gradOffset] = ctx.allocateGradientData(2, grad.get());
        REPORTER_ASSERT(reporter, gradPtr != nullptr);
        REPORTER_ASSERT(reporter, gradOffset == 0);
        for (int i = 0; i < 12; ++i) {
            gradPtr[i] = 60.f + i;
        }

        ctx.recordAlignment(tc.stride, tc.align);
        ctx.finalizePrecachedStorageData();

        uint32_t expectedPaddedGradSize = SkAlignNonPow2(48u, tc.expectedLCM);

        // Append 2 vertices
        std::vector<char> vert(tc.stride * 2, 0);
        for (size_t i = 0; i < vert.size(); ++i) {
            vert[i] = static_cast<char>((i + 1) & 0x7F);
        }
        uint32_t vOffset = ctx.appendVertices(vert.data(), /*count=*/2, tc.stride, tc.align);
        REPORTER_ASSERT(reporter, vOffset == expectedPaddedGradSize);
        REPORTER_ASSERT(reporter, vOffset % tc.align == 0);
        REPORTER_ASSERT(reporter, vOffset % tc.expectedLCM == 0);

        DrawTask drawTask(/*target=*/nullptr);
        auto storageResult = ctx.finalize(
                recorder->priv().drawBufferManager(), recorder.get(), &drawTask);
        if (std::holds_alternative<BindBufferInfo>(storageResult)) {
            auto bindInfo = std::get<BindBufferInfo>(storageResult);
            REPORTER_ASSERT(reporter, bindInfo.fBuffer != nullptr);
            REPORTER_ASSERT(reporter, bindInfo.fSize == expectedPaddedGradSize + tc.stride * 2);

            const char* bufferData = static_cast<const char*>(
                    const_cast<Buffer*>(bindInfo.fBuffer)->map()) + bindInfo.fOffset;
            const float* floatData = reinterpret_cast<const float*>(bufferData);
            for (int i = 0; i < 12; ++i) {
                REPORTER_ASSERT(reporter, floatData[i] == 60.f + i);
            }
            for (size_t i = 48; i < expectedPaddedGradSize; ++i) {
                REPORTER_ASSERT(reporter, bufferData[i] == 0);
            }
            REPORTER_ASSERT(reporter,
                            memcmp(bufferData + expectedPaddedGradSize,
                                   vert.data(),
                                   vert.size()) == 0);
        } else {
            auto proxy = std::get<sk_sp<TextureProxy>>(storageResult);
            REPORTER_ASSERT(reporter, proxy != nullptr);
        }
    }
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageContextVertexOnlyAndResetTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    StorageContext ctx;

    REPORTER_ASSERT(reporter, ctx.isEmpty());

    // 1. Finalize on empty context returns null buffer
    DrawTask drawTask(/*target=*/nullptr);
    ctx.finalizePrecachedStorageData();
    auto emptyResult = ctx.finalize(
            recorder->priv().drawBufferManager(), recorder.get(), &drawTask);
    auto emptyInfo = std::get<BindBufferInfo>(emptyResult);
    REPORTER_ASSERT(reporter, emptyInfo.fBuffer == nullptr);
    REPORTER_ASSERT(reporter, emptyInfo.fSize == 0);

    // 2. Vertex-only allocation without gradients
    ctx.recordAlignment(/*stride=*/24, /*align=*/16);
    ctx.finalizePrecachedStorageData();

    float vert[6] = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f};
    uint32_t offset = ctx.appendVertices(vert, /*count=*/1, /*stride=*/24, /*align=*/16);
    REPORTER_ASSERT(reporter, offset == 0);
    REPORTER_ASSERT(reporter, !ctx.isEmpty());

    auto vertexResult = ctx.finalize(
            recorder->priv().drawBufferManager(), recorder.get(), &drawTask);
    if (std::holds_alternative<BindBufferInfo>(vertexResult)) {
        auto bindInfo = std::get<BindBufferInfo>(vertexResult);
        REPORTER_ASSERT(reporter, bindInfo.fBuffer != nullptr);
        REPORTER_ASSERT(reporter, bindInfo.fSize == 24);

        const char* bufferData = static_cast<const char*>(
                const_cast<Buffer*>(bindInfo.fBuffer)->map()) + bindInfo.fOffset;
        REPORTER_ASSERT(reporter, memcmp(bufferData, vert, sizeof(vert)) == 0);
    } else {
        auto proxy = std::get<sk_sp<TextureProxy>>(vertexResult);
        REPORTER_ASSERT(reporter, proxy != nullptr);
    }

    // 3. Reset cache and verify clean state
    ctx.resetCache();
    REPORTER_ASSERT(reporter, ctx.isEmpty());

    // 4. Subsequent allocation after reset starts at offset 0
    SkPoint pts[2] = {{0, 0}, {100, 100}};
    SkColor4f colors[2] = {SkColors::kRed, SkColors::kBlue};
    auto grad = sk_make_sp<SkLinearGradient>(pts, SkGradient{{colors, {}, SkTileMode::kClamp}, {}});
    auto [gradPtr, gradOffset] = ctx.allocateGradientData(2, grad.get());
    REPORTER_ASSERT(reporter, gradPtr != nullptr);
    REPORTER_ASSERT(reporter, gradOffset == 0);
    for (int i = 0; i < 12; ++i) {
        gradPtr[i] = 70.f + i;
    }

    DrawTask drawTaskAfterReset(/*target=*/nullptr);
    ctx.finalizePrecachedStorageData();
    auto postResetResult = ctx.finalize(
            recorder->priv().drawBufferManager(), recorder.get(), &drawTaskAfterReset);
    if (std::holds_alternative<BindBufferInfo>(postResetResult)) {
        auto resetInfo = std::get<BindBufferInfo>(postResetResult);
        REPORTER_ASSERT(reporter, resetInfo.fBuffer != nullptr);

        const char* resetBufferData = static_cast<const char*>(
                const_cast<Buffer*>(resetInfo.fBuffer)->map()) + resetInfo.fOffset;
        const float* resetFloats = reinterpret_cast<const float*>(resetBufferData);
        for (int i = 0; i < 12; ++i) {
            REPORTER_ASSERT(reporter, resetFloats[i] == 70.f + i);
        }
    } else {
        auto proxy = std::get<sk_sp<TextureProxy>>(postResetResult);
        REPORTER_ASSERT(reporter, proxy != nullptr);
    }
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(StorageContextMultiStopGradientTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    StorageContext ctx;

    SkPoint pts[2] = {{0, 0}, {100, 100}};

    // 1. 9 stops gradient (9 stops > 8):
    // alignedOffsetsCount = SkAlign4(9) = 12 floats
    // colorsCount = 9 * 4 = 36 floats
    // total = SkAlign4(12 + 36) = 48 floats (192 bytes)
    std::vector<SkColor4f> colors9(9, SkColors::kRed);
    std::vector<SkScalar> pos9 = {0.f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 1.0f};
    auto grad9 = sk_make_sp<SkLinearGradient>(
            pts, SkGradient{SkGradient::Colors{colors9, pos9, SkTileMode::kClamp}, {}});

    auto [ptr9, offset9] = ctx.allocateGradientData(9, grad9.get());
    REPORTER_ASSERT(reporter, ptr9 != nullptr);
    REPORTER_ASSERT(reporter, offset9 == 0);

    // Populate offsets and colors
    for (int i = 0; i < 9; ++i) {
        ptr9[i] = pos9[i];
    }
    for (int i = 9; i < 12; ++i) {
        ptr9[i] = 0.f; // padding
    }
    for (int i = 0; i < 9; ++i) {
        ptr9[12 + i * 4 + 0] = static_cast<float>(i);
        ptr9[12 + i * 4 + 1] = static_cast<float>(i) * 0.1f;
        ptr9[12 + i * 4 + 2] = 0.5f;
        ptr9[12 + i * 4 + 3] = 1.0f;
    }

    // 2. 17 stops gradient:
    // alignedOffsetsCount = SkAlign4(17) = 20 floats
    // colorsCount = 17 * 4 = 68 floats
    // total = SkAlign4(20 + 68) = 88 floats (352 bytes)
    std::vector<SkColor4f> colors17(17, SkColors::kBlue);
    std::vector<SkScalar> pos17(17);
    for (int i = 0; i < 17; ++i) {
        pos17[i] = static_cast<float>(i) / 16.0f;
    }
    auto grad17 = sk_make_sp<SkLinearGradient>(
            pts, SkGradient{SkGradient::Colors{colors17, pos17, SkTileMode::kRepeat}, {}});

    auto [ptr17, offset17] = ctx.allocateGradientData(17, grad17.get());
    REPORTER_ASSERT(reporter, ptr17 != nullptr);
    REPORTER_ASSERT(reporter, offset17 == 48);

    for (int i = 0; i < 17; ++i) {
        ptr17[i] = pos17[i];
    }
    for (int i = 17; i < 20; ++i) {
        ptr17[i] = 0.f; // padding
    }
    for (int i = 0; i < 17; ++i) {
        ptr17[20 + i * 4 + 0] = static_cast<float>(i);
        ptr17[20 + i * 4 + 1] = static_cast<float>(i) * 0.05f;
        ptr17[20 + i * 4 + 2] = 0.25f;
        ptr17[20 + i * 4 + 3] = 1.0f;
    }

    ctx.finalizePrecachedStorageData();

    DrawTask drawTask(/*target=*/nullptr);
    auto storageResult = ctx.finalize(
            recorder->priv().drawBufferManager(), recorder.get(), &drawTask);
    if (std::holds_alternative<BindBufferInfo>(storageResult)) {
        auto bindInfo = std::get<BindBufferInfo>(storageResult);
        REPORTER_ASSERT(reporter, bindInfo.fBuffer != nullptr);
        REPORTER_ASSERT(reporter, bindInfo.fSize == (48 + 88) * sizeof(float)); // 544 bytes
        REPORTER_ASSERT(reporter, bindInfo.fSize % 16 == 0);

        const char* bufferData = static_cast<const char*>(
                const_cast<Buffer*>(bindInfo.fBuffer)->map()) + bindInfo.fOffset;
        const float* floatData = reinterpret_cast<const float*>(bufferData);

        // Check grad9 data
        for (int i = 0; i < 9; ++i) {
            REPORTER_ASSERT(reporter, floatData[i] == pos9[i]);
        }
        for (int i = 0; i < 9; ++i) {
            REPORTER_ASSERT(reporter, floatData[12 + i * 4 + 0] == static_cast<float>(i));
            REPORTER_ASSERT(reporter, floatData[12 + i * 4 + 1] == static_cast<float>(i) * 0.1f);
        }

        // Check grad17 data
        const float* float17 = floatData + 48;
        for (int i = 0; i < 17; ++i) {
            REPORTER_ASSERT(reporter, float17[i] == pos17[i]);
        }
        for (int i = 0; i < 17; ++i) {
            REPORTER_ASSERT(reporter, float17[20 + i * 4 + 0] == static_cast<float>(i));
            REPORTER_ASSERT(reporter, float17[20 + i * 4 + 1] == static_cast<float>(i) * 0.05f);
        }
    } else {
        auto proxy = std::get<sk_sp<TextureProxy>>(storageResult);
        REPORTER_ASSERT(reporter, proxy != nullptr);
    }
}

}  // namespace skgpu::graphite

