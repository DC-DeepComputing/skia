/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/compute/ArcLengthScan.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/compute/ScanComposer.h"
#include "src/gpu/graphite/task/ComputeTask.h"
#include "src/gpu/graphite/task/CopyTask.h"
#include "src/gpu/graphite/task/SynchronizeToCpuTask.h"
#include "tests/Test.h"
#include "tools/graphite/GraphiteTestContext.h"

#include <string>
#include <string_view>

using namespace skgpu::graphite;
using namespace skiatest::graphite;

namespace {

void* map_buffer(Context* context,
                 skiatest::graphite::GraphiteTestContext* testContext,
                 Buffer* buffer,
                 size_t offset) {
    SkASSERT(buffer);
    if (context->priv().caps()->bufferMapsAreAsync()) {
        buffer->asyncMap();
        while (!buffer->isMapped()) {
            testContext->tick();
        }
    }
    std::byte* ptr = static_cast<std::byte*>(buffer->map());
    SkASSERT(ptr);

    return ptr + offset;
}

sk_sp<Buffer> sync_buffer_to_cpu(Recorder* recorder, const Buffer* buffer) {
    if (recorder->priv().caps()->drawBufferCanBeMappedForReadback()) {
        recorder->priv().add(SynchronizeToCpuTask::Make(sk_ref_sp(buffer)));
        return sk_ref_sp(buffer);
    }

    auto xferBuffer = recorder->priv().resourceProvider()->findOrCreateNonShareableBuffer(
            buffer->size(),
            BufferType::kXferGpuToCpu,
            AccessPattern::kHostVisible,
            "ScanTest_TransferToCpu");
    SkASSERT(xferBuffer);

    recorder->priv().add(CopyBufferToBufferTask::Make(buffer,
                                                      /*srcOffset=*/0,
                                                      xferBuffer,
                                                      /*dstOffset=*/0,
                                                      buffer->size()));
    return xferBuffer;
}

bool is_dawn_or_metal_context_type(skiatest::GpuContextType ctxType) {
    return skiatest::IsDawnContextType(ctxType) || skiatest::IsMetalContextType(ctxType);
}

}  // namespace

#define DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(            \
        name, reporter, graphite_context, test_context)           \
    DEF_GRAPHITE_TEST_FOR_CONTEXTS(name,                          \
                                   is_dawn_or_metal_context_type, \
                                   reporter,                      \
                                   graphite_context,              \
                                   test_context,                  \
                                   CtsEnforcement::kNever)

// DEF_GRAPHITE_TEST(Scan_ShaderComposerTest, reporter, CtsEnforcement::kApiLevel_202604) {
//     using Composer = ScanComposer<ArcLengthScan::Preamble,
//                                   ArcLengthScan::LocalScan,
//                                   ArcLengthScan::WorkgroupScan,
//                                   ArcLengthScan::Conversion>;

//     SkDebugf("\n=================== GENERATED SCAN SKSL ===================\n");
//     SkDebugf("%.*s\n", (int)Composer::kShader.size(), Composer::kShader.data());
//     SkDebugf("===========================================================\n");

//     REPORTER_ASSERT(reporter, !Composer::kShader.empty());
// }

DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(Scan_WorkgroupScanTest,
                                              reporter,
                                              context,
                                              testContext) {
    constexpr uint32_t kProblemSize = 256;
    constexpr uint32_t kWorkgroupSize = 256;

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    class WorkgroupScanComputeStep : public ComputeStep {
    public:
        WorkgroupScanComputeStep() : ComputeStep(
                /*name=*/"WorkgroupScanTestStep",
                /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
                /*resources=*/{{
                    // Output buffer:
                    {
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kMapped,
                        /*slot=*/0,
                        /*sksl=*/"outputBlock { uint out_data[]; }",
                    }
                }}) {}
        ~WorkgroupScanComputeStep() override = default;

        std::string computeSkSL() const override {
            std::string shader;
            shader += "const uint S_MEM_SIZE = 256u;\n";
            shader += "workgroup float wg_partials[S_MEM_SIZE];\n";
            shader += "void main() {\n";
            shader += "    uint idx = sk_LocalInvocationID.x;\n";
            shader += "    wg_partials[idx] = float(idx);\n";
            shader += "    workgroupBarrier();\n";
            shader += ArcLengthScan::WorkgroupScan::kScan;
            shader += "\n    out_data[idx] = uint(wg_partials[idx]);\n";
            shader += "}\n";
            return shader;
        }

        size_t calculateBufferSize(int index, const ResourceDesc& r) const override {
            SkASSERT(index == 0);
            SkASSERT(r.fSlot == 0);
            SkASSERT(r.fFlow == DataFlow::kShared);
            return sizeof(uint32_t) * kProblemSize;
        }

        void prepareStorageBuffer(int,
                                  const ResourceDesc&,
                                  skgpu::BufferWriter&&) const override {}

        WorkgroupSize calculateGlobalDispatchSize() const override {
            return WorkgroupSize(1, 1, 1);
        }
    } step;

    DispatchGroup::Builder builder(recorder.get());
    if (!builder.appendStep(&step)) {
        ERRORF(reporter, "Failed to add ComputeStep to DispatchGroup");
        return;
    }

    BindBufferInfo outputInfo = builder.getSharedBufferResource(0);
    if (!outputInfo) {
        ERRORF(reporter, "Failed to allocate an output buffer at slot 0");
        return;
    }

    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    auto outputBuffer = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);

    std::unique_ptr<Recording> recording = recorder->snap();
    if (!recording) {
        ERRORF(reporter, "Failed to make recording");
        return;
    }

    InsertRecordingInfo insertInfo;
    insertInfo.fRecording = recording.get();
    context->insertRecording(insertInfo);
    testContext->syncedSubmit(context);

    uint32_t* outData = static_cast<uint32_t*>(
            map_buffer(context, testContext, outputBuffer.get(), outputInfo.fOffset));
    SkASSERT(outputBuffer->isMapped() && outData != nullptr);
    for (unsigned int i = 0; i < kProblemSize; ++i) {
        const uint32_t expected = i * (i + 1) / 2;
        const uint32_t found = outData[i];
        REPORTER_ASSERT(reporter, expected == found, "At index %u: expected '%u', found '%u'", i, expected, found);
    }
}
