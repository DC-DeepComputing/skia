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
    auto textureInfo = recorder->priv().caps()->getDefaultStorageTextureInfo(colorType);
    sk_sp<TextureProxy> targetProxy = TextureProxy::Make(recorder->priv().caps(),
                                                         recorder->priv().resourceProvider(),
                                                         dimensions,
                                                         textureInfo,
                                                         "PlaceholderTextureProxy",
                                                         skgpu::Budgeted::kYes);
    if (!targetProxy) {
        ERRORF(reporter, "GetTestingDrawPass: Failed to create cached TextureProxy.");
        return nullptr;
    }

    std::pair<LoadOp, StoreOp> ops = {LoadOp::kClear, StoreOp::kDiscard};
    std::array<float, 4> clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
    std::unique_ptr<DrawPass> drawPass = DrawPass::getTestDrawPass(targetProxy, ops, clearColor);
    if (drawPass == nullptr) {
        ERRORF(reporter, "GetTestingDrawPass: Failed to create Drawpass.");
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

    auto staticVertexCopyRanges = sharedContext->globalCache()->getStaticVertexCopyRanges();
    if (staticVertexCopyRanges.empty()) {
        ERRORF(reporter, "Did not get the static vertex information (copy ranges)");
        return;
    }

    uint32_t vertexInfoElementCount = static_cast<uint32_t>(staticVertexCopyRanges.size());
    if (vertexInfoElementCount & 3) {
        ERRORF(reporter, "Initial static vert information was not correctly populated: "
                         "Element count was %u, expected multiple of 4.", vertexInfoElementCount);
        return;
    }

    auto staticGpuVertexBuffer = sharedContext->globalCache()->getStaticVertexBuffer();
    if (!staticGpuVertexBuffer){
        ERRORF(reporter, "Did not get a valid static vertex buffer.");
        return;
    }

    submit_recording(context, testContext, recorder.get());

    // Readback exact size of static data
    const uint32_t readbackSize = staticGpuVertexBuffer->size();
    auto copyBuff = get_readback_buffer(recorder.get(), readbackSize);
    skia_private::TArray<uint8_t> readbackData;
    if (!copy_and_readback_buffer_sync(context,
                                       testContext,
                                       recorder.get(),
                                       staticGpuVertexBuffer.get(),
                                       copyBuff,
                                       readbackData,
                                       readbackSize)) {
        ERRORF(reporter, "Bad readback, exiting early.");
        return;
    }

    const uint32_t numRegionsToCheck = vertexInfoElementCount / 4;
    for (uint32_t i = 0; i < numRegionsToCheck; ++i) {
        auto cr = staticVertexCopyRanges[i];
        const uint32_t regionEndOffset = cr.fOffset + cr.fSize;
        if (cr.fOffset % cr.fRequiredAlignment) {
            ERRORF(reporter,
                   "Offset was not safely aligned to a count 4 stride: "
                   "Count four stride: %zu Offset: %u Expected: %zu\n",
                   cr.fRequiredAlignment,
                   cr.fOffset,
                   SkAlignNonPow2(cr.fOffset, cr.fRequiredAlignment));
            return;
        }

        if (regionEndOffset % cr.fRequiredAlignment) {
            ERRORF(reporter,
                   "End was not safely aligned to a count 4 stride: "
                   "Count four stride: %zu End: %u Expected: %zu\n",
                   cr.fRequiredAlignment,
                   regionEndOffset,
                   SkAlignNonPow2(regionEndOffset, cr.fRequiredAlignment));
            return;
        }

        const uint32_t paddingStart = cr.fOffset + cr.fUnalignedSize;
        for (uint32_t byteIndex = paddingStart; byteIndex < regionEndOffset; ++byteIndex) {
            if (readbackData[byteIndex]) {
                ERRORF(reporter,
                       "Buffer was not safely padded at byte: "
                       "%u Unaligned End of Buffer: %u Aligned End of Buffer: %u\n",
                       byteIndex,
                       paddingStart,
                       regionEndOffset);
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

    auto buffOpts = DrawBufferManager::DrawBufferManagerOptions();
    buffOpts.fVertexBufferMinSize = vertBuffSize;
    buffOpts.fVertexBufferMaxSize = vertBuffSize;
    buffOpts.fUseExactBuffSizes = true;
    RecorderOptionsPriv recOptsPriv;
    recOptsPriv.fDbmOptions = std::optional(buffOpts);
    RecorderOptions recOpts;
    recOpts.fRecorderOptionsPriv = &recOptsPriv;
    std::unique_ptr<Recorder> recorder = context->makeRecorder(recOpts);
    DrawBufferManager* bufferMgr = recorder->priv().drawBufferManager();
    if (bufferMgr->hasMappingFailed()) {
        ERRORF(reporter, "Failed to get buffer manager");
        return;
    }
    std::unique_ptr<DrawPass> drawPass = get_testing_drawpass(recorder.get(), reporter);
    std::unique_ptr<DrawWriter> dw = drawPass->getDrawWriter(bufferMgr);

    auto vertsNewPipeline = [&]() {
        dw->newPipelineState(/*type=*/{}, stride, stride, RenderStateFlags::kAppendVertices,
                             std::nullopt);
        return;
    };

    constexpr uint32_t V = UINT32_MAX;
    auto vertAppend = [&](uint32_t count) -> BindBufferInfo {
        DrawWriter::Vertices vdw{*dw};
        auto a = vdw.append(count);
        for (uint32_t i = 0; i < count; ++i) {
            a << V;
        }
        return dw->getLastAppendedBuffer();
    };

    BindBufferInfo lastBBI;
    skia_private::TArray<BindBufferInfo> bindBuffers(4);
    auto checkAndAdvance = [&](const BindBufferInfo& currentBBI,
                               bool expectSameBuffer) {
        bool isSameBuffer = lastBBI.fBuffer == currentBBI.fBuffer;
        if (expectSameBuffer != isSameBuffer) {
            if (expectSameBuffer) {
                ERRORF(reporter, "Error: Expected same buffer.\n");
            } else {
                ERRORF(reporter, "Error: Expected new buffer.\n");
            }
            return true;
        } else if (!isSameBuffer) {
            bindBuffers.push_back(currentBBI);
        }
        lastBBI = currentBBI;
        return false;
    };

    vertsNewPipeline();
    lastBBI = vertAppend(1);
    bindBuffers.push_back(lastBBI);
    vertsNewPipeline();
    if (checkAndAdvance(vertAppend(2), true))  { return; }
    dw->newDynamicState();
    if (checkAndAdvance(vertAppend(3), false)) { return; }
    if (checkAndAdvance(vertAppend(1), true))  { return; }
    dw->newDynamicState();
    if (checkAndAdvance(vertAppend(1), true))  { return; }
    vertsNewPipeline();
    if (checkAndAdvance(vertAppend(1), false)) { return; }
    if (checkAndAdvance(vertAppend(1), true))  { return; }
    if (checkAndAdvance(vertAppend(1), true))  { return; }
    if (checkAndAdvance(vertAppend(1), true))  { return; }
    vertsNewPipeline();
    if (checkAndAdvance(vertAppend(2), true))  { return; }
    if (checkAndAdvance(vertAppend(3), false)) { return; }
    if (checkAndAdvance(vertAppend(2), true))  { return; }
    dw->flush();

    constexpr uint32_t P = 0;
    constexpr uint32_t EXPECTED_VERT_BUFFER_COUNT = 4;
    std::array<std::array<uint32_t, readbackSize>, EXPECTED_VERT_BUFFER_COUNT> expected = {{
        {V, P, P, P, V, V, P, P},
        {V, V, V, V, V, P, P, P},
        {V, V, V, V, V, V, P, P},
        {V, V, V, V, V, P, P, P}
    }};

    skia_private::TArray<uint32_t> readbackData;
    std::array<sk_sp<Buffer>, EXPECTED_VERT_BUFFER_COUNT> copyBuffers;
    const bool mappableDrawBuffers = recorder->priv().caps()->drawBufferCanBeMapped();
    if (!mappableDrawBuffers) {
        for(uint32_t i = 0; i < EXPECTED_VERT_BUFFER_COUNT; ++i) {
            copyBuffers[i] = get_readback_buffer(recorder.get(), vertBuffSize);
            recorder->priv().add(CopyBufferToBufferTask::Make(bindBuffers[i].fBuffer,
                                                            /*srcOffset=*/0,
                                                            copyBuffers[i],
                                                            /*dstOffset=*/0,
                                                            vertBuffSize));
        }
    }
    std::unique_ptr<Recording> recording = submit_recording(context, testContext, recorder.get());

    for (uint32_t i = 0; i < EXPECTED_VERT_BUFFER_COUNT; ++i) {
        if (!map_and_readback_buffer_sync(
                    context,
                    testContext,
                    mappableDrawBuffers ? bindBuffers[i].fBuffer : copyBuffers[i].get(),
                    readbackData,
                    readbackSize)) {
            ERRORF(reporter, "Bad readback, exiting early.");
            return;
        }
        for(uint32_t j = 0; j < static_cast<uint32_t>(readbackData.size()); ++j) {
            if (readbackData[j] != expected[i][j]){
                ERRORF(reporter, "Error expected %u: Got: %u At ExpectedIndex: %u \n",
                    expected[i][j], readbackData[j], i * EXPECTED_VERT_BUFFER_COUNT + j);
                return;
            }
        }
    }
}
