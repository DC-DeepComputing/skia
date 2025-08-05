/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkScalar.h"
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

#include <string>

using namespace skgpu::graphite;
using namespace skiatest::graphite;

namespace {

enum class SubgroupTestType {
    kI32,
    kU32,
    kF32,
};

const char* to_sksl_type_string(SubgroupTestType type) {
    switch (type) {
        case SubgroupTestType::kI32:
            return "int";
        case SubgroupTestType::kU32:
            return "uint";
        case SubgroupTestType::kF32:
            return "float";
    }
    SkUNREACHABLE;
}

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

bool supported_backend(skiatest::GpuContextType ctxType) {
    return skiatest::IsMetalContextType(ctxType) || skiatest::IsVulkanContextType(ctxType) ||
           skiatest::IsDawnContextType(ctxType);
}

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

    static constexpr uint32_t kDispatchSize = 128;

    SubgroupTestStep(Op op, SubgroupTestType type, int workgroupSize)
            : SubgroupTestStep(
                      op,
                      type,
                      workgroupSize,
                      "in_data { " + std::string(to_sksl_type_string(type)) + " inData[]; }",
                      "out_data { " + std::string(to_sksl_type_string(type)) + " outData[]; }") {}

    std::string computeSkSL() const override {
        const std::string typeStr = to_sksl_type_string(fType);
        // All shaders also write out the subgroup size to the last element of the output buffer.
        const std::string suffix = "if (sk_GlobalInvocationID.x == 0) { outData[128] = " + typeStr +
                                   "(sk_SubgroupSize); } }";
        auto getShader = [&](const char* body) {
            return std::string("void main() { ") + body + suffix;
        };

        switch (fOp) {
            case Op::kAdd:
                return getShader(
                        "outData[sk_GlobalInvocationID.x] = "
                        "subgroupAdd(inData[sk_GlobalInvocationID.x]);");
            case Op::kAll:
                return getShader(("outData[sk_GlobalInvocationID.x] = " + typeStr +
                                  "(subgroupAll(inData[sk_GlobalInvocationID.x] != 0));")
                                         .c_str());
            case Op::kAny:
                return getShader(("outData[sk_GlobalInvocationID.x] = " + typeStr +
                                  "(subgroupAny(inData[sk_GlobalInvocationID.x] != 0));")
                                         .c_str());
            case Op::kBallot: {
                const char* condition =
                        (fType == SubgroupTestType::kF32)
                                ? "inData[sk_GlobalInvocationID.x] > 0.0"
                                : "(uint(inData[sk_GlobalInvocationID.x]) & 1) != 0";
                return getShader(("outData[sk_GlobalInvocationID.x] = " + typeStr +
                                  "(subgroupBallot(" + condition + ").x);").c_str());
            }
            case Op::kBroadcast:
                return getShader(
                        "outData[sk_GlobalInvocationID.x] = "
                        "subgroupBroadcast(inData[sk_GlobalInvocationID.x], 0); ");
            case Op::kExclusiveSum:
                return getShader(
                        "outData[sk_GlobalInvocationID.x] = "
                        "subgroupExclusiveSum(inData[sk_GlobalInvocationID.x]);");
            case Op::kInclusiveSum:
                return getShader(
                        "outData[sk_GlobalInvocationID.x] = "
                        "subgroupInclusiveSum(inData[sk_GlobalInvocationID.x]);");
            case Op::kMax:
                return getShader(
                        "outData[sk_GlobalInvocationID.x] = "
                        "subgroupMax(inData[sk_GlobalInvocationID.x]);");
            case Op::kMin:
                return getShader(
                        "outData[sk_GlobalInvocationID.x] = "
                        "subgroupMin(inData[sk_GlobalInvocationID.x]);");
            case Op::kShuffle:
                return getShader(
                        "outData[sk_GlobalInvocationID.x] = "
                        "subgroupShuffle(inData[sk_GlobalInvocationID.x], "
                        "(sk_LaneID + 1) % sk_SubgroupSize); ");
        }
        SkUNREACHABLE;
    }

    size_t calculateBufferSize(int index, const ResourceDesc& r) const override {
        size_t elementSize = 0;
        switch (fType) {
            case SubgroupTestType::kI32:
                elementSize = sizeof(int32_t);
                break;
            case SubgroupTestType::kU32:
                elementSize = sizeof(uint32_t);
                break;
            case SubgroupTestType::kF32:
                elementSize = sizeof(float);
                break;
        }
        // The output buffer is one element larger to store the subgroup size.
        size_t size = kDispatchSize * elementSize;
        if (index == 1) {
            size += elementSize;
        }
        return size;
    }

    void prepareStorageBuffer(int resourceIndex,
                              const ResourceDesc&,
                              void* buffer,
                              size_t bufferSize) const override;

    WorkgroupSize calculateGlobalDispatchSize() const override { return {1, 1, 1}; }

private:
    SubgroupTestStep(
            Op op, SubgroupTestType type, int workgroupSize, std::string inStr, std::string outStr)
            : ComputeStep("SubgroupTestStep",
                          /*localDispatchSize=*/{(uint32_t)workgroupSize, 1, 1},
                          /*resources=*/
                          {{ResourceType::kStorageBuffer,
                            DataFlow::kPrivate,
                            ResourcePolicy::kMapped,
                            inStr.c_str()},
                           {ResourceType::kStorageBuffer,
                            DataFlow::kShared,
                            ResourcePolicy::kNone,
                            0,
                            outStr.c_str()}})
            , fOp(op)
            , fType(type)
            , fInStr(std::move(inStr))
            , fOutStr(std::move(outStr)) {}

    template <typename T> void fillData(void* buffer) const;

    Op fOp;
    SubgroupTestType fType;
    std::string fInStr;
    std::string fOutStr;
};

// Encapsulates the boilerplate for running a subgroup compute test.
const void* run_subgroup_test(skiatest::Reporter* reporter,
                              Context* context,
                              GraphiteTestContext* testContext,
                              SubgroupTestStep::Op op,
                              SubgroupTestType type) {
    auto recorder = context->makeRecorder();
    SubgroupTestStep step(op, type, SubgroupTestStep::kDispatchSize);
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
    return map_buffer(context, testContext, readbackBuffer.get());
}

// Helper to run the boilerplate preamble for a subgroup test.
template <typename T>
struct SubgroupTestResult {
    const T* fResults = nullptr;
    uint32_t fSubgroupSize = 0;

    operator bool() const { return fResults != nullptr; }
};

template <typename T>
SubgroupTestResult<T> run_subgroup_test_preamble(skiatest::Reporter* reporter,
                                                 Context* context,
                                                 GraphiteTestContext* testContext,
                                                 SubgroupTestStep::Op op,
                                                 SubgroupTestType type) {
    if (!context->priv().caps()->subgroupSupport()) {
        return {};
    }
    const T* results =
            static_cast<const T*>(run_subgroup_test(reporter, context, testContext, op, type));
    REPORTER_ASSERT(reporter, results);
    if (!results) {
        return {};
    }
    const uint32_t subgroupSize = (uint32_t)results[SubgroupTestStep::kDispatchSize];
    if (!subgroupSize) {
        return {};
    }
    return {results, subgroupSize};
}

template <typename T>
T get_test_value(uint32_t index) {
    if constexpr (std::is_floating_point_v<T>) {
        // Use values that can be represented exactly to avoid precision issues.
        return (T)((((index * 17 + 1) % 251) - 125) / 256.0f);
    } else {
        return (T)((index * 17 + 1) % 251);
    }
}

void SubgroupTestStep::prepareStorageBuffer(int resourceIndex,
                                          const ResourceDesc&,
                                          void* buffer,
                                          size_t bufferSize) const {
    if (resourceIndex != 0) {
        return;
    }

    switch (fType) {
        case SubgroupTestType::kI32:
            this->fillData<int32_t>(buffer);
            break;
        case SubgroupTestType::kU32:
            this->fillData<uint32_t>(buffer);
            break;
        case SubgroupTestType::kF32:
            this->fillData<float>(buffer);
            break;
    }
}

template <typename T>
void SubgroupTestStep::fillData(void* buffer) const {
    T* inputData = static_cast<T*>(buffer);
    for (uint32_t i = 0; i < kDispatchSize; ++i) {
        if (fOp == Op::kAll) {
            inputData[i] = (i < kDispatchSize / 2) ? 1 : (i % 2);
        } else if (fOp == Op::kAny) {
            inputData[i] = (i < kDispatchSize / 2) ? 0 : (i % 2);
        } else {
            inputData[i] = get_test_value<T>(i);
        }
    }
}

}  // anonymous namespace

template <typename T>
void run_add_test(skiatest::Reporter* reporter,
                  Context* context,
                  GraphiteTestContext* testContext,
                  SubgroupTestType type) {
    const auto test = run_subgroup_test_preamble<T>(
            reporter, context, testContext, SubgroupTestStep::Op::kAdd, type);
    if (!test) {
        return;
    }

    for (uint32_t sg = 0; sg * test.fSubgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        T expectedSum = 0;
        for (uint32_t i = 0; i < test.fSubgroupSize; ++i) {
            uint32_t index = sg * test.fSubgroupSize + i;
            expectedSum += get_test_value<T>(index);
        }
        for (uint32_t i = 0; i < test.fSubgroupSize; ++i) {
            uint32_t index = sg * test.fSubgroupSize + i;
            if constexpr (std::is_floating_point_v<T>) {
                REPORTER_ASSERT(reporter,
                                SkScalarNearlyEqual(test.fResults[index], expectedSum, 1e-3f),
                                "sg %u, lane %u: Expected %f, Got %f",
                                sg,
                                i,
                                expectedSum,
                                test.fResults[index]);
            } else {
                REPORTER_ASSERT(reporter,
                                test.fResults[index] == expectedSum,
                                "sg %u, lane %u: Expected %d, Got %d",
                                sg,
                                i,
                                (int)expectedSum,
                                (int)test.fResults[index]);
            }
        }
    }
}

template <typename T>
void run_all_any_test(skiatest::Reporter* reporter,
                      Context* context,
                      GraphiteTestContext* testContext,
                      SubgroupTestType type,
                      SubgroupTestStep::Op op) {
    const auto test = run_subgroup_test_preamble<T>(reporter, context, testContext, op, type);
    if (!test) {
        return;
    }

    REPORTER_ASSERT(reporter, SubgroupTestStep::kDispatchSize / 2 >= test.fSubgroupSize);
    if (op == SubgroupTestStep::Op::kAll) {
        REPORTER_ASSERT(reporter, test.fResults[0] == 1, "Expected subgroup 0 to be all true");
        REPORTER_ASSERT(reporter,
                        test.fResults[SubgroupTestStep::kDispatchSize / 2] == 0,
                        "Expected a later subgroup to not be all true");
    } else {
        REPORTER_ASSERT(reporter, test.fResults[0] == 0, "Expected subgroup 0 to not have any true");
        REPORTER_ASSERT(reporter,
                        test.fResults[SubgroupTestStep::kDispatchSize / 2] == 1,
                        "Expected a later subgroup to have any true");
    }
}

template <typename T>
void run_ballot_test(skiatest::Reporter* reporter,
                     Context* context,
                     GraphiteTestContext* testContext,
                     SubgroupTestType type) {
    const auto test = run_subgroup_test_preamble<T>(
            reporter, context, testContext, SubgroupTestStep::Op::kBallot, type);
    if (!test) {
        return;
    }

    for (uint32_t sg = 0; sg * test.fSubgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        uint32_t expectedBallot = 0;
        for (uint32_t i = 0; i < test.fSubgroupSize; ++i) {
            uint32_t index = sg * test.fSubgroupSize + i;
            bool bit;
            if constexpr (std::is_floating_point_v<T>) {
                bit = get_test_value<T>(index) > 0;
            } else {
                bit = (get_test_value<T>(index) & 1) != 0;
            }
            if (bit) {
                expectedBallot |= (1u << i);
            }
        }
        REPORTER_ASSERT(reporter,
                        (uint32_t)test.fResults[sg * test.fSubgroupSize] == expectedBallot,
                        "Expected %u, Got %u",
                        expectedBallot,
                        (uint32_t)test.fResults[sg * test.fSubgroupSize]);
    }
}

template <typename T>
void run_broadcast_test(skiatest::Reporter* reporter,
                        Context* context,
                        GraphiteTestContext* testContext,
                        SubgroupTestType type) {
    const auto test = run_subgroup_test_preamble<T>(
            reporter, context, testContext, SubgroupTestStep::Op::kBroadcast, type);
    if (!test) {
        return;
    }

    for (uint32_t sg = 0; sg * test.fSubgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        uint32_t offset = sg * test.fSubgroupSize;
        T expectedValue = get_test_value<T>(offset);

        for (uint32_t i = 0; i < test.fSubgroupSize; ++i) {
            uint32_t index = sg * test.fSubgroupSize + i;
            if constexpr (std::is_floating_point_v<T>) {
                REPORTER_ASSERT(reporter,
                                SkScalarNearlyEqual(test.fResults[index], expectedValue),
                                "Expected %f, Got %f",
                                expectedValue,
                                test.fResults[index]);
            } else {
                REPORTER_ASSERT(reporter,
                                test.fResults[index] == expectedValue,
                                "Expected %d, Got %d",
                                (int)expectedValue,
                                (int)test.fResults[index]);
            }
        }
    }
}

template <typename T>
void run_scan_test(skiatest::Reporter* reporter,
                   Context* context,
                   GraphiteTestContext* testContext,
                   SubgroupTestType type,
                   SubgroupTestStep::Op op) {
    const auto test = run_subgroup_test_preamble<T>(reporter, context, testContext, op, type);
    if (!test) {
        return;
    }

    for (uint32_t sg = 0; sg * test.fSubgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        T runningSum = 0;
        for (uint32_t i = 0; i < test.fSubgroupSize; ++i) {
            uint32_t index = sg * test.fSubgroupSize + i;
            T value = get_test_value<T>(index);

            T expectedValue = runningSum;
            if (op == SubgroupTestStep::Op::kInclusiveSum) {
                expectedValue += value;
            }

            if constexpr (std::is_floating_point_v<T>) {
                REPORTER_ASSERT(reporter,
                                SkScalarNearlyEqual(test.fResults[index], expectedValue, 1e-3f),
                                "sg %u, lane %u: Expected %f, Got %f",
                                sg,
                                i,
                                expectedValue,
                                test.fResults[index]);
            } else {
                REPORTER_ASSERT(reporter,
                                test.fResults[index] == expectedValue,
                                "sg %u, lane %u: Expected %d, Got %d",
                                sg,
                                i,
                                (int)expectedValue,
                                (int)test.fResults[index]);
            }
            runningSum += value;
        }
    }
}

template <typename T>
void run_min_max_test(skiatest::Reporter* reporter,
                      Context* context,
                      GraphiteTestContext* testContext,
                      SubgroupTestType type,
                      SubgroupTestStep::Op op) {
    const auto test = run_subgroup_test_preamble<T>(reporter, context, testContext, op, type);
    if (!test) {
        return;
    }

    for (uint32_t sg = 0; sg * test.fSubgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        T expectedValue;
        if constexpr (std::is_floating_point_v<T>) {
            expectedValue = (op == SubgroupTestStep::Op::kMax) ? -1.0f : 1.0f;
        } else {
            expectedValue = (op == SubgroupTestStep::Op::kMax) ? 0 : 251;
        }

        for (uint32_t i = 0; i < test.fSubgroupSize; ++i) {
            uint32_t index = sg * test.fSubgroupSize + i;
            T value = get_test_value<T>(index);
            if (op == SubgroupTestStep::Op::kMax) {
                expectedValue = std::max(expectedValue, value);
            } else {
                expectedValue = std::min(expectedValue, value);
            }
        }

        for (uint32_t i = 0; i < test.fSubgroupSize; ++i) {
            uint32_t index = sg * test.fSubgroupSize + i;
            if constexpr (std::is_floating_point_v<T>) {
                REPORTER_ASSERT(reporter,
                                SkScalarNearlyEqual(test.fResults[index], expectedValue),
                                "Expected %f, Got %f",
                                expectedValue,
                                test.fResults[index]);
            } else {
                REPORTER_ASSERT(reporter,
                                test.fResults[index] == expectedValue,
                                "Expected %d, Got %d",
                                (int)expectedValue,
                                (int)test.fResults[index]);
            }
        }
    }
}

template <typename T>
void run_shuffle_test(skiatest::Reporter* reporter,
                      Context* context,
                      GraphiteTestContext* testContext,
                      SubgroupTestType type) {
    const auto test = run_subgroup_test_preamble<T>(
            reporter, context, testContext, SubgroupTestStep::Op::kShuffle, type);
    if (!test) {
        return;
    }

    for (uint32_t sg = 0; sg * test.fSubgroupSize < SubgroupTestStep::kDispatchSize; ++sg) {
        for (uint32_t i = 0; i < test.fSubgroupSize; ++i) {
            uint32_t index = sg * test.fSubgroupSize + i;
            uint32_t targetLaneIndex = sg * test.fSubgroupSize + ((i + 1) % test.fSubgroupSize);
            T expectedValue = get_test_value<T>(targetLaneIndex);

            if constexpr (std::is_floating_point_v<T>) {
                REPORTER_ASSERT(reporter,
                                SkScalarNearlyEqual(test.fResults[index], expectedValue),
                                "Expected %f, Got %f",
                                expectedValue,
                                test.fResults[index]);
            } else {
                REPORTER_ASSERT(reporter,
                                test.fResults[index] == expectedValue,
                                "Expected %d, Got %d",
                                (int)expectedValue,
                                (int)test.fResults[index]);
            }
        }
    }
}

DEF_SUBGROUP_TEST(SubgroupAddTest, reporter, context, testContext) {
    run_add_test<uint32_t>(reporter, context, testContext, SubgroupTestType::kU32);
    run_add_test<int32_t>(reporter, context, testContext, SubgroupTestType::kI32);
    run_add_test<float>(reporter, context, testContext, SubgroupTestType::kF32);
}

DEF_SUBGROUP_TEST(SubgroupAllTest, reporter, context, testContext) {
    run_all_any_test<uint32_t>(
            reporter, context, testContext, SubgroupTestType::kU32, SubgroupTestStep::Op::kAll);
    run_all_any_test<int32_t>(
            reporter, context, testContext, SubgroupTestType::kI32, SubgroupTestStep::Op::kAll);
    run_all_any_test<float>(
            reporter, context, testContext, SubgroupTestType::kF32, SubgroupTestStep::Op::kAll);
}

DEF_SUBGROUP_TEST(SubgroupAnyTest, reporter, context, testContext) {
    run_all_any_test<uint32_t>(
            reporter, context, testContext, SubgroupTestType::kU32, SubgroupTestStep::Op::kAny);
    run_all_any_test<int32_t>(
            reporter, context, testContext, SubgroupTestType::kI32, SubgroupTestStep::Op::kAny);
    run_all_any_test<float>(
            reporter, context, testContext, SubgroupTestType::kF32, SubgroupTestStep::Op::kAny);
}

DEF_SUBGROUP_TEST(SubgroupBallotTest, reporter, context, testContext) {
    run_ballot_test<uint32_t>(reporter, context, testContext, SubgroupTestType::kU32);
    run_ballot_test<int32_t>(reporter, context, testContext, SubgroupTestType::kI32);
    run_ballot_test<float>(reporter, context, testContext, SubgroupTestType::kF32);
}

DEF_SUBGROUP_TEST(SubgroupBroadcastTest, reporter, context, testContext) {
    run_broadcast_test<uint32_t>(reporter, context, testContext, SubgroupTestType::kU32);
    run_broadcast_test<int32_t>(reporter, context, testContext, SubgroupTestType::kI32);
    run_broadcast_test<float>(reporter, context, testContext, SubgroupTestType::kF32);
}

DEF_SUBGROUP_TEST(SubgroupExclusiveSumTest, reporter, context, testContext) {
    run_scan_test<uint32_t>(reporter,
                            context,
                            testContext,
                            SubgroupTestType::kU32,
                            SubgroupTestStep::Op::kExclusiveSum);
    run_scan_test<int32_t>(reporter,
                           context,
                           testContext,
                           SubgroupTestType::kI32,
                           SubgroupTestStep::Op::kExclusiveSum);
    run_scan_test<float>(reporter,
                         context,
                         testContext,
                         SubgroupTestType::kF32,
                         SubgroupTestStep::Op::kExclusiveSum);
}

DEF_SUBGROUP_TEST(SubgroupInclusiveSumTest, reporter, context, testContext) {
    run_scan_test<uint32_t>(reporter,
                            context,
                            testContext,
                            SubgroupTestType::kU32,
                            SubgroupTestStep::Op::kInclusiveSum);
    run_scan_test<int32_t>(reporter,
                           context,
                           testContext,
                           SubgroupTestType::kI32,
                           SubgroupTestStep::Op::kInclusiveSum);
    run_scan_test<float>(reporter,
                         context,
                         testContext,
                         SubgroupTestType::kF32,
                         SubgroupTestStep::Op::kInclusiveSum);
}

DEF_SUBGROUP_TEST(SubgroupMaxTest, reporter, context, testContext) {
    run_min_max_test<uint32_t>(
            reporter, context, testContext, SubgroupTestType::kU32, SubgroupTestStep::Op::kMax);
    run_min_max_test<int32_t>(
            reporter, context, testContext, SubgroupTestType::kI32, SubgroupTestStep::Op::kMax);
    run_min_max_test<float>(
            reporter, context, testContext, SubgroupTestType::kF32, SubgroupTestStep::Op::kMax);
}

DEF_SUBGROUP_TEST(SubgroupMinTest, reporter, context, testContext) {
    run_min_max_test<uint32_t>(
            reporter, context, testContext, SubgroupTestType::kU32, SubgroupTestStep::Op::kMin);
    run_min_max_test<int32_t>(
            reporter, context, testContext, SubgroupTestType::kI32, SubgroupTestStep::Op::kMin);
    run_min_max_test<float>(
            reporter, context, testContext, SubgroupTestType::kF32, SubgroupTestStep::Op::kMin);
}

DEF_SUBGROUP_TEST(SubgroupShuffleTest, reporter, context, testContext) {
    run_shuffle_test<uint32_t>(reporter, context, testContext, SubgroupTestType::kU32);
    run_shuffle_test<int32_t>(reporter, context, testContext, SubgroupTestType::kI32);
    run_shuffle_test<float>(reporter, context, testContext, SubgroupTestType::kF32);
}
