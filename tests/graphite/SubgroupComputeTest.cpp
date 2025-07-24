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
#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/task/ComputeTask.h"
#include "src/gpu/graphite/task/CopyTask.h"
#include "src/gpu/graphite/task/SynchronizeToCpuTask.h"

#include "tools/graphite/GraphiteTestContext.h"

using namespace skgpu::graphite;
using namespace skiatest::graphite;

namespace {

// Helper function to map a buffer for CPU access.
void* map_buffer(Context* context,
                 skiatest::graphite::GraphiteTestContext* testContext,
                 Buffer* buffer) {
    SkASSERT(buffer);
    if (context->priv().caps()->bufferMapsAreAsync()) {
        buffer->asyncMap();
        while (!buffer->isMapped()) {
            testContext->tick();
        }
    }
    void* ptr = buffer->map();
    SkASSERT(ptr);
    return ptr;
}

// Helper to copy a GPU buffer to a mappable CPU buffer for readback.
sk_sp<Buffer> sync_buffer_to_cpu(Recorder* recorder, const Buffer* buffer) {
    if (recorder->priv().caps()->drawBufferCanBeMappedForReadback()) {
        recorder->priv().add(SynchronizeToCpuTask::Make(sk_ref_sp(buffer)));
        return sk_ref_sp(const_cast<Buffer*>(buffer));
    }

    auto xferBuffer =
            recorder->priv().resourceProvider()->findOrCreateBuffer(buffer->size(),
                                                                     BufferType::kXferGpuToCpu,
                                                                     AccessPattern::kHostVisible,
                                                                     "SubgroupTest_Readback");
    SkASSERT(xferBuffer);
    recorder->priv().add(CopyBufferToBufferTask::Make(buffer, 0, xferBuffer, 0, buffer->size()));
    return xferBuffer;
}

// Helper function to submit all recorded tasks and wait for them to complete.
bool submit_and_wait(Context* context,
                     GraphiteTestContext* testContext,
                     Recorder* recorder) {
    std::unique_ptr<Recording> recording = recorder->snap();
    if (!recording) {
        return false;
    }
    InsertRecordingInfo insertInfo;
    insertInfo.fRecording = recording.get();
    context->insertRecording(insertInfo);
    testContext->syncedSubmit(context);
    return true;
}

// A predicate to determine if a backend should run these tests.
// This can be expanded as you add support for more backends.
bool supported_backend(skiatest::GpuContextType ctxType) {
    return skiatest::IsMetalContextType(ctxType) ||
           skiatest::IsVulkanContextType(ctxType) ||
           skiatest::IsDawnContextType(ctxType);
}

// Macro to define a Graphite test for all subgroup-supporting backends.
#define DEF_SUBGROUP_TEST(name, reporter, context, testContext) \
    DEF_GRAPHITE_TEST_FOR_CONTEXTS(name,                        \
                                   supported_backend,           \
                                   reporter,                    \
                                   context,                     \
                                   testContext,                 \
                                   CtsEnforcement::kApiLevel_T)

// A flexible ComputeStep for testing different subgroup operations.
class SubgroupTestStep : public ComputeStep {
public:
    enum class Op {
        kAdd,
        kMin,
        kMax,
        kBallot,
        kShuffle,
    };

    SubgroupTestStep(Op op, int workgroupSize)
            : ComputeStep("SubgroupTestStep",
                          /*localDispatchSize=*/{(uint32_t)workgroupSize, 1, 1},
                          /*resources=*/
                          {{ResourceType::kStorageBuffer, DataFlow::kPrivate, ResourcePolicy::kMapped,
                            "in_data { uint inData[]; }"},
                           {ResourceType::kStorageBuffer, DataFlow::kShared, ResourcePolicy::kNone, 0,
                            "out_data { uint outData[]; }"}}),
              fOp(op) {}

    std::string computeSkSL() const override {
        switch (fOp) {
            case Op::kAdd:
                return R"(
                    void main() {
                        uint index = sk_GlobalInvocationID.x;
                        outData[index] = subgroupAdd(inData[index]);
                    })";
            case Op::kMin:
                return R"(
                    void main() {
                        uint index = sk_GlobalInvocationID.x;
                        outData[index] = subgroupMin(inData[index]);
                    })";
            case Op::kMax:
                return R"(
                    void main() {
                        uint index = sk_GlobalInvocationID.x;
                        outData[index] = subgroupMax(inData[index]);
                    })";
            case Op::kBallot:
                // Ballot returns true if the Nth bit of the input is set.
                // We test if the input value is odd.
                return R"(
                    void main() {
                        uint index = sk_GlobalInvocationID.x;
                        bool valueIsOdd = (inData[index] & 1) == 1;
                        uint4 ballotResult = subgroupBallot(valueIsOdd);
                        // Store just the first component of the ballot result for simplicity.
                        outData[index] = ballotResult.x;
                    })";
            case Op::kShuffle:
                // Shuffles the data from the lane at `(lane_id + 1) % subgroup_size`.
                return R"(
                    void main() {
                        uint index = sk_GlobalInvocationID.x;
                        uint lane = sk_SubgroupInvocationID;
                        uint targetLane = (lane + 1) % sk_SubgroupSize;
                        outData[index] = subgroupShuffle(inData[index], targetLane);
                    })";
        }
        SkUNREACHABLE;
    }

    size_t calculateBufferSize(int index, const ResourceDesc& r) const override {
        return kDispatchSize * sizeof(uint32_t);
    }

    void prepareStorageBuffer(int resourceIndex,
                                const ResourceDesc&,
                                void* buffer,
                                size_t bufferSize) const override {
        if (resourceIndex != 0) return; // Only prepare the input buffer.

        uint32_t* inputData = static_cast<uint32_t*>(buffer);
        for (uint32_t i = 0; i < kDispatchSize; ++i) {
            // For min/max tests, we want data that isn't trivially ordered.
            inputData[i] = (i * 17 + 1) % 251;
        }
    }

    WorkgroupSize calculateGlobalDispatchSize() const override { return {1, 1, 1}; }

    static constexpr uint32_t kDispatchSize = 128;

private:
    Op fOp;
};

} // anonymous namespace

// Test case for subgroupAdd
DEF_SUBGROUP_TEST(SubgroupAddTest, reporter, context, testContext) {
    if (!context->priv().caps()->subgroupSupport()) {
        return;
    }

    auto recorder = context->makeRecorder();
    SubgroupTestStep step(SubgroupTestStep::Op::kAdd, SubgroupTestStep::kDispatchSize);

    DispatchGroup::Builder builder(recorder.get());
    builder.appendStep(&step);
    BindBufferInfo outputInfo = builder.getSharedBufferResource(0);
    REPORTER_ASSERT(reporter, outputInfo);

    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    auto readbackBuffer = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);
    REPORTER_ASSERT(reporter, submit_and_wait(context, testContext, recorder.get()));

    const uint32_t* results = static_cast<const uint32_t*>(map_buffer(context, testContext,
                                                                       readbackBuffer.get()));
    const int subgroupSize = 32; //TODO read this back!
    for (int sg = 0; sg * subgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        uint32_t expectedSum = 0;
        for (int i = 0; i < subgroupSize; ++i) {
            uint32_t index = sg * subgroupSize + i;
            expectedSum += (index * 17 + 1) % 251;
        }
        for (int i = 0; i < subgroupSize; ++i) {
            uint32_t index = sg * subgroupSize + i;
            REPORTER_ASSERT(reporter, results[index] == expectedSum,
                            "subgroupAdd failed at index %d. Expected %u, Got %u",
                            index, expectedSum, results[index]);
        }
    }
}

// Test case for subgroupMin
DEF_SUBGROUP_TEST(SubgroupMinTest, reporter, context, testContext) {
    if (!context->priv().caps()->subgroupSupport()) {
        return;
    }
    auto recorder = context->makeRecorder();
    SubgroupTestStep step(SubgroupTestStep::Op::kMin, SubgroupTestStep::kDispatchSize);

    DispatchGroup::Builder builder(recorder.get());
    builder.appendStep(&step);
    BindBufferInfo outputInfo = builder.getSharedBufferResource(0);
    REPORTER_ASSERT(reporter, outputInfo);

    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    auto readbackBuffer = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);
    REPORTER_ASSERT(reporter, submit_and_wait(context, testContext, recorder.get()));

    const uint32_t* results = static_cast<const uint32_t*>(map_buffer(context, testContext,
                                                                       readbackBuffer.get()));
    const int subgroupSize = 32; //TODO read this back!
    for (int sg = 0; sg * subgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        uint32_t expectedMin = 251;
        for (int i = 0; i < subgroupSize; ++i) {
            uint32_t index = sg * subgroupSize + i;
            expectedMin = std::min(expectedMin, (index * 17 + 1) % 251);
        }
        for (int i = 0; i < subgroupSize; ++i) {
            uint32_t index = sg * subgroupSize + i;
            REPORTER_ASSERT(reporter, results[index] == expectedMin,
                            "subgroupMin failed at index %d. Expected %u, Got %u",
                            index, expectedMin, results[index]);
        }
    }
}

// Test case for subgroupMax
DEF_SUBGROUP_TEST(SubgroupMaxTest, reporter, context, testContext) {
    if (!context->priv().caps()->subgroupSupport()) {
        return;
    }
    auto recorder = context->makeRecorder();
    SubgroupTestStep step(SubgroupTestStep::Op::kMax, SubgroupTestStep::kDispatchSize);

    DispatchGroup::Builder builder(recorder.get());
    builder.appendStep(&step);
    BindBufferInfo outputInfo = builder.getSharedBufferResource(0);
    REPORTER_ASSERT(reporter, outputInfo);

    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    auto readbackBuffer = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);
    REPORTER_ASSERT(reporter, submit_and_wait(context, testContext, recorder.get()));

    const uint32_t* results = static_cast<const uint32_t*>(map_buffer(context, testContext,
                                                                       readbackBuffer.get()));
    const int subgroupSize = 32; //TODO read this back!
    for (int sg = 0; sg * subgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        uint32_t expectedMax = 0;
        for (int i = 0; i < subgroupSize; ++i) {
            uint32_t index = sg * subgroupSize + i;
            expectedMax = std::max(expectedMax, (index * 17 + 1) % 251);
        }
        for (int i = 0; i < subgroupSize; ++i) {
            uint32_t index = sg * subgroupSize + i;
            REPORTER_ASSERT(reporter, results[index] == expectedMax,
                            "subgroupMax failed at index %d. Expected %u, Got %u",
                            index, expectedMax, results[index]);
        }
    }
}

// Test case for subgroupBallot
DEF_SUBGROUP_TEST(SubgroupBallotTest, reporter, context, testContext) {
    if (!context->priv().caps()->subgroupSupport()) {
        return;
    }
    auto recorder = context->makeRecorder();
    SubgroupTestStep step(SubgroupTestStep::Op::kBallot, SubgroupTestStep::kDispatchSize);

    DispatchGroup::Builder builder(recorder.get());
    builder.appendStep(&step);
    BindBufferInfo outputInfo = builder.getSharedBufferResource(0);
    REPORTER_ASSERT(reporter, outputInfo);

    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    auto readbackBuffer = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);
    REPORTER_ASSERT(reporter, submit_and_wait(context, testContext, recorder.get()));

    const uint32_t* results = static_cast<const uint32_t*>(map_buffer(context, testContext,
                                                                       readbackBuffer.get()));
    const int subgroupSize = 32; //TODO read this back!
    for (int sg = 0; sg * subgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        uint32_t expectedBallot = 0;
        for (int i = 0; i < subgroupSize; ++i) {
            uint32_t index = sg * subgroupSize + i;
            bool valueIsOdd = (((index * 17 + 1) % 251) & 1) == 1;
            if (valueIsOdd) {
                expectedBallot |= (1u << i);
            }
        }
        for (int i = 0; i < subgroupSize; ++i) {
            uint32_t index = sg * subgroupSize + i;
            REPORTER_ASSERT(reporter, results[index] == expectedBallot,
                            "subgroupBallot failed at index %d. Expected %u, Got %u",
                            index, expectedBallot, results[index]);
        }
    }
}

// Test case for subgroupShuffle
DEF_SUBGROUP_TEST(SubgroupShuffleTest, reporter, context, testContext) {
    if (!context->priv().caps()->subgroupSupport()) {
        return;
    }
    auto recorder = context->makeRecorder();
    SubgroupTestStep step(SubgroupTestStep::Op::kShuffle, SubgroupTestStep::kDispatchSize);

    DispatchGroup::Builder builder(recorder.get());
    builder.appendStep(&step);
    BindBufferInfo outputInfo = builder.getSharedBufferResource(0);
    REPORTER_ASSERT(reporter, outputInfo);

    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    auto readbackBuffer = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);
    REPORTER_ASSERT(reporter, submit_and_wait(context, testContext, recorder.get()));

    const uint32_t* results = static_cast<const uint32_t*>(map_buffer(context, testContext,
                                                                       readbackBuffer.get()));
    const int subgroupSize = 32; //TODO read this back!
    for (int sg = 0; sg * subgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        for (int i = 0; i < subgroupSize; ++i) {
            uint32_t index = sg * subgroupSize + i;
            uint32_t targetLaneIndex = sg * subgroupSize + ((i + 1) % subgroupSize);
            uint32_t expectedValue = (targetLaneIndex * 17 + 1) % 251;
            REPORTER_ASSERT(reporter, results[index] == expectedValue,
                            "subgroupShuffle failed at index %d. Expected %u, Got %u",
                            index, expectedValue, results[index]);
        }
    }
}
