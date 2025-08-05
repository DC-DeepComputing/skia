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
    if (!buffer) {
        return nullptr;
    }
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

sk_sp<Buffer> sync_buffer_to_cpu(Recorder* recorder, const Buffer* buffer) {
    auto xferBuffer =
            recorder->priv().resourceProvider()->findOrCreateBuffer(buffer->size(),
                                                                    BufferType::kXferGpuToCpu,
                                                                    AccessPattern::kHostVisible,
                                                                    "SubgroupTest_Readback");
    if (!xferBuffer) {
        return nullptr;
    }
    recorder->priv().add(CopyBufferToBufferTask::Make(buffer, 0, xferBuffer, 0, buffer->size()));
    return xferBuffer;
}

// Helper function to submit all recorded tasks and wait for them to complete.
bool submit_and_wait(Context* context, GraphiteTestContext* testContext, Recorder* recorder) {
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
bool supported_backend(skiatest::GpuContextType ctxType) {
    return skiatest::IsMetalContextType(ctxType) || skiatest::IsVulkanContextType(ctxType) ||
           skiatest::IsDawnContextType(ctxType);
}

// Macro to define a Graphite test for all subgroup-supporting backends.
#define DEF_SUBGROUP_TEST(name, reporter, context, testContext) \
    DEF_GRAPHITE_TEST_FOR_CONTEXTS(                             \
            name, supported_backend, reporter, context, testContext, CtsEnforcement::kApiLevel_T)

// A flexible ComputeStep for testing different subgroup operations.
class SubgroupTestStep : public ComputeStep {
public:
    enum class Op {
        kAdd,
        kAll,
        kAny,
        kBallot,
        kBroadcast,
        kExclusiveSum,
        kInclusiveSum,
        kMax,
        kMin,
        kShuffle,
    };

    SubgroupTestStep(Op op, int workgroupSize)
            : ComputeStep("SubgroupTestStep",
                          /*localDispatchSize=*/{(uint32_t)workgroupSize, 1, 1},
                          /*resources=*/
                          {{ResourceType::kStorageBuffer,
                            DataFlow::kPrivate,
                            ResourcePolicy::kMapped,
                            "in_data { uint inData[]; }"},
                           {ResourceType::kStorageBuffer,
                            DataFlow::kShared,
                            ResourcePolicy::kNone,
                            0,
                            "out_data { uint outData[]; }"}})
            , fOp(op) {}

    std::string computeSkSL() const override {
        switch (fOp) {
            case Op::kAdd:
                return "void main() { outData[sk_GlobalInvocationID.x] = "
                       "subgroupAdd(inData[sk_GlobalInvocationID.x]); }";
            case Op::kAll:
                return "void main() { outData[sk_GlobalInvocationID.x] = "
                       "subgroupAll(inData[sk_GlobalInvocationID.x] != 0 ? true : false) ? 1 : 0; "
                       "}";
            case Op::kAny:
                return "void main() { outData[sk_GlobalInvocationID.x] = "
                       "subgroupAny(inData[sk_GlobalInvocationID.x] != 0 ? true : false) ? 1 : 0; "
                       "}";
            case Op::kBallot:
                return "void main() { outData[sk_GlobalInvocationID.x] = "
                       "subgroupBallot((inData[sk_GlobalInvocationID.x] & 1) != 0).x; }";
            case Op::kBroadcast:
                return "void main() { outData[sk_GlobalInvocationID.x] = "
                       "subgroupBroadcast(inData[sk_GlobalInvocationID.x], 0); }";
            case Op::kExclusiveSum:
                return "void main() { outData[sk_GlobalInvocationID.x] = "
                       "subgroupExclusiveSum(inData[sk_GlobalInvocationID.x]); }";
            case Op::kInclusiveSum:
                return "void main() { outData[sk_GlobalInvocationID.x] = "
                       "subgroupInclusiveSum(inData[sk_GlobalInvocationID.x]); }";
            case Op::kMax:
                return "void main() { outData[sk_GlobalInvocationID.x] = "
                       "subgroupMax(inData[sk_GlobalInvocationID.x]); }";
            case Op::kMin:
                return "void main() { outData[sk_GlobalInvocationID.x] = "
                       "subgroupMin(inData[sk_GlobalInvocationID.x]); }";
            // TODO CHANGE THIS FROM 32
            case Op::kShuffle:
                return "void main() { uint subgroupSize = 32; uint lane = sk_GlobalInvocationID.x "
                       "& (subgroupSize - 1); "
                       "outData[sk_GlobalInvocationID.x] = "
                       "subgroupShuffle(inData[sk_GlobalInvocationID.x], (lane + 1) % "
                       "subgroupSize); }";
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
        if (resourceIndex != 0) return;

        uint32_t* inputData = static_cast<uint32_t*>(buffer);
        for (uint32_t i = 0; i < kDispatchSize; ++i) {
            if (fOp == Op::kAll) {
                // For subgroupAll, make one subgroup all true, the others mixed.
                inputData[i] = (i < 32) ? 1 : (i % 2);
            } else if (fOp == Op::kAny) {
                // For subgroupAny, make one subgroup all false, the others mixed.
                inputData[i] = (i < 32) ? 0 : (i % 2);
            } else {
                inputData[i] = (i * 17 + 1) % 251;
            }
        }
    }

    WorkgroupSize calculateGlobalDispatchSize() const override { return {1, 1, 1}; }
    static constexpr uint32_t kDispatchSize = 128;

private:
    Op fOp;
};

// Encapsulates the boilerplate for running a subgroup compute test.
const uint32_t* run_subgroup_test(skiatest::Reporter* reporter,
                                  Context* context,
                                  GraphiteTestContext* testContext,
                                  SubgroupTestStep::Op op) {
    auto recorder = context->makeRecorder();
    SubgroupTestStep step(op, SubgroupTestStep::kDispatchSize);
    DispatchGroup::Builder builder(recorder.get());
    builder.appendStep(&step);
    BindBufferInfo outputInfo = builder.getSharedBufferResource(0);
    REPORTER_ASSERT(reporter, outputInfo);
    if (!outputInfo) return nullptr;

    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    auto readbackBuffer = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);
    if (!submit_and_wait(context, testContext, recorder.get())) {
        REPORTER_ASSERT(reporter, false, "submit_and_wait failed");
        return nullptr;
    }
    return static_cast<const uint32_t*>(map_buffer(context, testContext, readbackBuffer.get()));
}

}  // anonymous namespace

DEF_SUBGROUP_TEST(SubgroupAddTest, reporter, context, testContext) {
    if (!context->priv().caps()->subgroupSupport()) return;
    const uint32_t* results =
            run_subgroup_test(reporter, context, testContext, SubgroupTestStep::Op::kAdd);
    REPORTER_ASSERT(reporter, results);
    if (!results) return;
    const int subgroupSize = 32;
    for (uint32_t sg = 0; sg * subgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        uint32_t expectedSum = 0;
        for (int i = 0; i < subgroupSize; ++i)
            expectedSum += ((sg * subgroupSize + i) * 17 + 1) % 251;
        for (int i = 0; i < subgroupSize; ++i) {
            uint32_t index = sg * subgroupSize + i;
            REPORTER_ASSERT(reporter,
                            results[index] == expectedSum,
                            "Expected %u, Got %u",
                            expectedSum,
                            results[index]);
        }
    }
}

DEF_SUBGROUP_TEST(SubgroupAllTest, reporter, context, testContext) {
    if (!context->priv().caps()->subgroupSupport()) return;
    const uint32_t* results =
            run_subgroup_test(reporter, context, testContext, SubgroupTestStep::Op::kAll);
    REPORTER_ASSERT(reporter, results);
    if (!results) return;
    REPORTER_ASSERT(reporter, results[0] == 1, "Expected subgroup 0 to be all true");
    REPORTER_ASSERT(reporter, results[32] == 0, "Expected subgroup 1 to not be all true");
}

DEF_SUBGROUP_TEST(SubgroupAnyTest, reporter, context, testContext) {
    if (!context->priv().caps()->subgroupSupport()) return;
    const uint32_t* results =
            run_subgroup_test(reporter, context, testContext, SubgroupTestStep::Op::kAny);
    REPORTER_ASSERT(reporter, results);
    if (!results) return;
    REPORTER_ASSERT(reporter, results[0] == 0, "Expected subgroup 0 to not have any true");
    REPORTER_ASSERT(reporter, results[32] == 1, "Expected subgroup 1 to have any true");
}

DEF_SUBGROUP_TEST(SubgroupBallotTest, reporter, context, testContext) {
    if (!context->priv().caps()->subgroupSupport()) return;
    const uint32_t* results =
            run_subgroup_test(reporter, context, testContext, SubgroupTestStep::Op::kBallot);
    REPORTER_ASSERT(reporter, results);
    if (!results) return;
    const int subgroupSize = 32;
    for (uint32_t sg = 0; sg * subgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        uint32_t expectedBallot = 0;
        for (int i = 0; i < subgroupSize; ++i) {
            if (((((sg * subgroupSize + i) * 17 + 1) % 251) & 1) != 0) {
                expectedBallot |= (1u << i);
            }
        }
        REPORTER_ASSERT(reporter,
                        results[sg * subgroupSize] == expectedBallot,
                        "Expected %u, Got %u",
                        expectedBallot,
                        results[sg * subgroupSize]);
    }
}

DEF_SUBGROUP_TEST(SubgroupBroadcastTest, reporter, context, testContext) {
    if (!context->priv().caps()->subgroupSupport()) return;
    const uint32_t* results =
            run_subgroup_test(reporter, context, testContext, SubgroupTestStep::Op::kBroadcast);
    REPORTER_ASSERT(reporter, results);
    if (!results) return;
    const int subgroupSize = 32;
    for (uint32_t sg = 0; sg * subgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        uint32_t expectedValue = ((sg * subgroupSize) * 17 + 1) % 251;
        for (int i = 0; i < subgroupSize; ++i) {
            uint32_t index = sg * subgroupSize + i;
            REPORTER_ASSERT(reporter,
                            results[index] == expectedValue,
                            "Expected %u, Got %u",
                            expectedValue,
                            results[index]);
        }
    }
}

DEF_SUBGROUP_TEST(SubgroupExclusiveSumTest, reporter, context, testContext) {
    if (!context->priv().caps()->subgroupSupport()) return;
    const uint32_t* results =
            run_subgroup_test(reporter, context, testContext, SubgroupTestStep::Op::kExclusiveSum);
    REPORTER_ASSERT(reporter, results);
    if (!results) return;
    const int subgroupSize = 32;
    for (uint32_t sg = 0; sg * subgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        uint32_t runningSum = 0;
        for (int i = 0; i < subgroupSize; ++i) {
            uint32_t index = sg * subgroupSize + i;
            REPORTER_ASSERT(reporter,
                            results[index] == runningSum,
                            "Expected %u, Got %u",
                            runningSum,
                            results[index]);
            runningSum += ((index * 17 + 1) % 251);
        }
    }
}

DEF_SUBGROUP_TEST(SubgroupInclusiveSumTest, reporter, context, testContext) {
    if (!context->priv().caps()->subgroupSupport()) return;
    const uint32_t* results =
            run_subgroup_test(reporter, context, testContext, SubgroupTestStep::Op::kInclusiveSum);
    REPORTER_ASSERT(reporter, results);
    if (!results) return;
    const int subgroupSize = 32;
    for (uint32_t sg = 0; sg * subgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        uint32_t runningSum = 0;
        for (int i = 0; i < subgroupSize; ++i) {
            uint32_t index = sg * subgroupSize + i;
            runningSum += ((index * 17 + 1) % 251);
            REPORTER_ASSERT(reporter,
                            results[index] == runningSum,
                            "Expected %u, Got %u",
                            runningSum,
                            results[index]);
        }
    }
}

DEF_SUBGROUP_TEST(SubgroupMaxTest, reporter, context, testContext) {
    if (!context->priv().caps()->subgroupSupport()) return;
    const uint32_t* results =
            run_subgroup_test(reporter, context, testContext, SubgroupTestStep::Op::kMax);
    REPORTER_ASSERT(reporter, results);
    if (!results) return;
    const int subgroupSize = 32;
    for (uint32_t sg = 0; sg * subgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        uint32_t expectedMax = 0;
        for (int i = 0; i < subgroupSize; ++i)
            expectedMax = std::max(expectedMax, ((sg * subgroupSize + i) * 17 + 1) % 251);
        for (int i = 0; i < subgroupSize; ++i) {
            uint32_t index = sg * subgroupSize + i;
            REPORTER_ASSERT(reporter,
                            results[index] == expectedMax,
                            "Expected %u, Got %u",
                            expectedMax,
                            results[index]);
        }
    }
}

DEF_SUBGROUP_TEST(SubgroupMinTest, reporter, context, testContext) {
    if (!context->priv().caps()->subgroupSupport()) return;
    const uint32_t* results =
            run_subgroup_test(reporter, context, testContext, SubgroupTestStep::Op::kMin);
    REPORTER_ASSERT(reporter, results);
    if (!results) return;
    const int subgroupSize = 32;
    for (uint32_t sg = 0; sg * subgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        uint32_t expectedMin = 251;
        for (int i = 0; i < subgroupSize; ++i)
            expectedMin = std::min(expectedMin, ((sg * subgroupSize + i) * 17 + 1) % 251);
        for (int i = 0; i < subgroupSize; ++i) {
            uint32_t index = sg * subgroupSize + i;
            REPORTER_ASSERT(reporter,
                            results[index] == expectedMin,
                            "Expected %u, Got %u",
                            expectedMin,
                            results[index]);
        }
    }
}

DEF_SUBGROUP_TEST(SubgroupShuffleTest, reporter, context, testContext) {
    if (!context->priv().caps()->subgroupSupport()) return;
    const uint32_t* results =
            run_subgroup_test(reporter, context, testContext, SubgroupTestStep::Op::kShuffle);
    REPORTER_ASSERT(reporter, results);
    if (!results) return;
    const int subgroupSize = 32;
    for (uint32_t sg = 0; sg * subgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        for (int i = 0; i < subgroupSize; ++i) {
            uint32_t index = sg * subgroupSize + i;
            uint32_t targetLaneIndex = sg * subgroupSize + ((i + 1) % subgroupSize);
            uint32_t expectedValue = (targetLaneIndex * 17 + 1) % 251;
            REPORTER_ASSERT(reporter,
                            results[index] == expectedValue,
                            "Expected %u, Got %u",
                            expectedValue,
                            results[index]);
        }
    }
}
