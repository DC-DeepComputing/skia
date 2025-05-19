/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/RecorderOptionsPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/task/CopyTask.h"
#include "src/gpu/graphite/task/SynchronizeToCpuTask.h"
#include "src/gpu/graphite/task/UploadTask.h"

#include "tools/graphite/GraphiteTestContext.h"

using namespace skgpu::graphite;
using namespace skiatest::graphite;

namespace {

std::unique_ptr<Recording> submit_recording(Context* context,
                                            GraphiteTestContext* testContext,
                                            Recorder* recorder) {
    std::unique_ptr<Recording> recording = recorder->snap();
    if (!recording) {
        return nullptr;
    }

    InsertRecordingInfo insertInfo;
    insertInfo.fRecording = recording.get();
    context->insertRecording(insertInfo);
    testContext->syncedSubmit(context);

    return recording;
}

std::unique_ptr<DrawPass> get_testing_drawpass(Recorder* recorder,
                                               skiatest::Reporter* reporter,
                                               SkISize dimensions = {1, 1},
                                               SkColorType colorType = kRGBA_8888_SkColorType) {
    SkImageInfo bmpInfo = SkImageInfo::Make(dimensions, colorType, kPremul_SkAlphaType);
    SkBitmap bmp;
    if (!bmp.tryAllocPixels(bmpInfo)) {
        ERRORF(reporter, "CreateHeadlessDrawPass: Failed to allocate pixels for SkBitmap.");
        return nullptr;
    }

    sk_sp<TextureProxy> targetProxy = recorder->priv().CreateCachedProxy(recorder,
                                                                         bmp,
                                                                         "TextureProxy");
    if (!targetProxy) {
        ERRORF(reporter, "CreateHeadlessDrawPass: Failed to create cached TextureProxy.");
        return nullptr;
    }

    std::pair<LoadOp, StoreOp> ops = {LoadOp::kClear, StoreOp::kDiscard};
    std::array<float, 4> clearColor = {0.0f, 0.0f, 0.0f, 0.0f};

    std::unique_ptr<DrawPass> drawPass = DrawPass::getTestDrawPass(targetProxy, ops, clearColor);
    if (drawPass == nullptr) {
        ERRORF(reporter, "CreateHeadlessDrawPass: Failed to creae Drawpass.");
        return nullptr;
    }

    return drawPass;
}

void* map_buffer(Context* context,
                 skiatest::graphite::GraphiteTestContext* testContext,
                 Buffer* bufferToMap,
                 size_t readOffset) {
    SkASSERT(bufferToMap);
    if (context->priv().caps()->bufferMapsAreAsync()) {
        bufferToMap->asyncMap();
        while (!bufferToMap->isMapped()) {
            testContext->tick();
        }
    }
    std::byte* mappedBytes = static_cast<std::byte*>(bufferToMap->map());
    SkASSERT(mappedBytes);

    return mappedBytes + readOffset;
}

sk_sp<Buffer> get_readback_buffer(Recorder* recorder, size_t copyBufferSize) {
    return recorder->priv().resourceProvider()->findOrCreateBuffer(copyBufferSize,
                                                                   BufferType::kXferGpuToCpu,
                                                                   AccessPattern::kHostVisible,
                                                                   "VerticesPaddingTest_Copy");
}

template<typename T>
bool map_and_readback_buffer_sync(Context* context,
                                  GraphiteTestContext* testContext,
                                  Buffer* sourceBuffer,
                                  skia_private::TArray<T>& readbackDst,
                                  uint32_t readbackSize) {
    if (sourceBuffer->isMapped()) {
        sourceBuffer->unmap();
    }
    T* read = static_cast<T*>(map_buffer(context, testContext, sourceBuffer, 0));
    if (!read) {
        return false;
    }
    readbackDst = skia_private::TArray<T>(read, readbackSize);
    return true;
}

template<typename T>
bool copy_and_readback_buffer_sync(Context* context,
                                   GraphiteTestContext* testContext,
                                   Recorder* recorder,
                                   Buffer* sourceBuffer,
                                   sk_sp<Buffer> copyBuffer,
                                   skia_private::TArray<T>& readbackDst,
                                   uint32_t readbackSize) {
    SkASSERT(!copyBuffer->isMapped() && readbackSize <= copyBuffer->size());
    recorder->priv().add(CopyBufferToBufferTask::Make(sourceBuffer,
                                                      /*srcOffset=*/0,
                                                      copyBuffer,
                                                      /*dstOffset=*/0,
                                                      readbackSize));
    if (!submit_recording(context, testContext, recorder)) {
        return false;
    }
    T* read = static_cast<T*>(map_buffer(context, testContext, copyBuffer.get(), 0));
    if (!read) {
        return false;
    }
    readbackDst = skia_private::TArray<T>(read, readbackSize);
    copyBuffer->unmap();
    return true;
}

bool is_dawn_or_vulkan_context_type(skiatest::GpuContextType contextType) {
    return skiatest::IsDawnContextType(contextType) || skiatest::IsVulkanContextType(contextType);
}

} // namespace

#define DEF_GRAPHITE_TEST_FOR_DAWN_AND_VULKAN_CONTEXTS(            \
        testName, reporterObj, graphiteContext, testContextObj)    \
    DEF_GRAPHITE_TEST_FOR_CONTEXTS(testName,                       \
                                   is_dawn_or_vulkan_context_type, \
                                   reporterObj,                    \
                                   graphiteContext,                \
                                   testContextObj,                 \
                                   CtsEnforcement::kNever)

DEF_GRAPHITE_TEST_FOR_DAWN_AND_VULKAN_CONTEXTS(StaticVerticesPaddingTest,
                                               reporter,
                                               context,
                                               testContext) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    auto sharedContext = recorder->priv().sharedContext();

    auto initialStaticVertexInfo = sharedContext->globalCache()->getInitialStaticVertexInfo();
    uint32_t vertexInfoElementCount = static_cast<uint32_t>(initialStaticVertexInfo.size());
    if (vertexInfoElementCount & 3) {
        ERRORF(reporter, "Initial static vert information was not correctly populated: "
                         "Element count was %u, expected multiple of 4.", vertexInfoElementCount);
        return;
    }

    auto staticGpuVertexBuffer = sharedContext->globalCache()->getStaticVertexBuffer();
    if (!staticGpuVertexBuffer.has_value()){
        ERRORF(reporter, "Did not get a valid static vertex buffer.");
        return;
    }

    submit_recording(context, testContext, recorder.get());

    // Readback exact size of static data
    const uint32_t readbackSize = staticGpuVertexBuffer.value()->size();
    auto copyBuff = get_readback_buffer(recorder.get(), readbackSize);
    skia_private::TArray<uint8_t> readbackData;
    if (!copy_and_readback_buffer_sync(context,
                                       testContext,
                                       recorder.get(),
                                       staticGpuVertexBuffer.value().get(),
                                       copyBuff,
                                       readbackData,
                                       readbackSize)) {
        ERRORF(reporter, "Bad readback, exiting early.");
        return;
    }

    const uint32_t numRegionsToCheck = vertexInfoElementCount / 4;
    for (uint32_t i = 0; i < numRegionsToCheck; ++i) {
        const uint32_t groupIndex = i * 4;
        const uint32_t regionBaseOffset = initialStaticVertexInfo[groupIndex];
        const uint32_t dataSize = initialStaticVertexInfo[groupIndex + 1];
        const uint32_t paddingSize = initialStaticVertexInfo[groupIndex + 2];
        const uint32_t fourStride = initialStaticVertexInfo[groupIndex + 3];

        const uint32_t regionEndOffset = regionBaseOffset + paddingSize;

        if (regionBaseOffset % fourStride){
            ERRORF(reporter, "Offset was not safely aligned to a count 4 stride: "
                             "Count four stride: %u Offset: %u Expected: %zu\n",
                             fourStride, regionBaseOffset,
                             SkAlignNonPow2(regionBaseOffset, fourStride));
            return;
        }

        if (regionEndOffset % fourStride){
            ERRORF(reporter, "End was not safely aligned to a count 4 stride: "
                             "Count four stride: %u End: %u Expected: %zu\n",
                             fourStride, regionEndOffset,
                             SkAlignNonPow2(regionEndOffset, fourStride));
            return;
        }

        const uint32_t paddingStart = regionBaseOffset + dataSize;
        for (uint32_t byteIndex = paddingStart; byteIndex < regionEndOffset; ++byteIndex) {
            if (readbackData[byteIndex]) {
                ERRORF(reporter, "Buffer was not safely padded at byte: "
                                 "%u Unaligned End of Buffer: %u Aligned End of Buffer: %u\n",
                                 byteIndex, paddingStart, regionEndOffset);
                return;
            }
        }
    }
}

DEF_GRAPHITE_TEST_FOR_DAWN_AND_VULKAN_CONTEXTS(DynamicVerticesPaddingTest,
                                               reporter,
                                               context,
                                               testContext) {
    constexpr uint32_t stride = 4;
    constexpr uint32_t vertBuffSize = 32; // I.e. blocks of 8 verts (vertBuffSize / stride = 8)
    constexpr uint32_t readbackSize = vertBuffSize / stride;

    auto buffSizes = DrawBufferManager::BufferSizes();
    buffSizes.fVertexBufferMinSize = vertBuffSize;
    buffSizes.fVertexBufferMaxSize = vertBuffSize;
    buffSizes.fUseExactBuffSizes = true;
    RecorderOptionsPriv recOptsPriv;
    recOptsPriv.fBufferSizes = std::optional(buffSizes);
    RecorderOptions recOpts;
    recOpts.fRecorderOptionsPriv = &recOptsPriv;
    std::unique_ptr<Recorder> recorder = context->makeRecorder(recOpts);
    DrawBufferManager* bufferMgr = recorder->priv().drawBufferManager();
    if (bufferMgr->hasMappingFailed()) {
        ERRORF(reporter, "Failed to get buffer manager");
        return;
    }

    std::unique_ptr<DrawPass> drawPass = get_testing_drawpass(recorder.get(), reporter);
    std::unique_ptr<HeadlessDrawWriter> dw = drawPass->getHeadlessDrawWriter(bufferMgr);

    auto vertsNewPipeline = [&]() {
        dw->newPipelineState(/*type=*/{}, stride, stride, RenderStateFlags::kAppendVertices,
                             std::nullopt);
        return;
    };

    constexpr uint32_t V = UINT32_MAX;
    auto vertAppend = [&](uint32_t count) {
        DrawWriter::Vertices vdw{*dw};
        auto a = vdw.append(count);
        for (uint32_t i = 0; i < count; ++i) {
            a << V;
        }
        return;
    };

    vertsNewPipeline();
    vertAppend(1);
    vertsNewPipeline();
    vertAppend(2);
    dw->newDynamicState();
    vertAppend(3);
    vertAppend(1);
    dw->newDynamicState();
    vertAppend(1);
    vertsNewPipeline();
    vertAppend(1);
    vertAppend(1);
    vertAppend(1);
    vertAppend(1);
    vertsNewPipeline();
    vertAppend(2);
    vertAppend(3);
    vertAppend(2);
    dw->flush();

    constexpr uint32_t P = 0;
    constexpr uint32_t NEW_BUFF = 0xffffu;
    constexpr uint32_t EXPECTED_VERT_BUFFER_COUNT = 4;
    std::array<uint32_t, 36> expected = {
        V, P, P, P, V, V, P, P, NEW_BUFF,
        V, V, V, V, V, P, P, P, NEW_BUFF,
        V, V, V, V, V, V, P, P, NEW_BUFF,
        V, V, V, V, V, P, P, P, NEW_BUFF
    };

    // This could possibly change in the future, if it does, this test will have to be modified.
    std::unique_ptr<Recording> recording = submit_recording(context, testContext, recorder.get());
    std::vector<sk_sp<Resource>> resRefs = recording->priv().getExtraResourceRefs();
    skia_private::TArray<uint32_t> readbackData;
    uint32_t expIndex = 0;
    auto copyBuff = get_readback_buffer(recorder.get(), readbackSize);
    for (uint32_t i = 0; i < EXPECTED_VERT_BUFFER_COUNT; ++i) {
        SkASSERT(strcmp(resRefs[i]->getLabel().c_str(), "VertexBuffer") == 0);
        if (!map_and_readback_buffer_sync(
                    context,
                    testContext,
                    static_cast<skgpu::graphite::Buffer*>(resRefs[i].get()),
                    readbackData,
                    readbackSize)) {
            ERRORF(reporter, "Bad readback, exiting early.");
            return;
        }
        for(uint32_t j = 0; j < static_cast<uint32_t>(readbackData.size()); ++j) {
            if (readbackData[j] != expected[expIndex]){
                ERRORF(reporter, "Error expected %u: Got: %u At ExpectedIndex: %u \n",
                    expected[expIndex], readbackData[j], expIndex);
                return;
            }
            ++expIndex;
        }
        if (expected[expIndex] != NEW_BUFF) {
            ERRORF(reporter, "Error expected a new buffer.\n");
            return;
        }
        ++expIndex;
    }
}
