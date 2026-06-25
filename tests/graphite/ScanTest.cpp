/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "src/core/SkFloatBits.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/compute/ArcLengthScan.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/compute/ScanComposer.h"
#include "src/gpu/graphite/task/ComputeTask.h"
#include "src/gpu/graphite/task/CopyTask.h"
#include "src/gpu/graphite/task/SynchronizeToCpuTask.h"
#include "src/sksl/SkSLUtil.h"
#include "tests/Test.h"
#include "tools/graphite/GraphiteTestContext.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <string_view>
#include <vector>

using namespace skgpu::graphite;
using namespace skiatest::graphite;

namespace {

// General ---------------------------------------------------------------------

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

template <typename T>
T* run_step_and_map_output(Context* context,
                           skiatest::graphite::GraphiteTestContext* testContext,
                           skiatest::Reporter* reporter,
                           ComputeStep* step,
                           sk_sp<Buffer>& outputBuffer) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    DispatchGroup::Builder builder(recorder.get());
    if (!builder.appendStep(step)) {
        ERRORF(reporter, "Failed to add ComputeStep to DispatchGroup");
        return nullptr;
    }

    BindBufferInfo outputInfo = builder.getSharedBufferResource(0);
    if (!outputInfo) {
        ERRORF(reporter, "Failed to allocate an output buffer at slot 0");
        return nullptr;
    }

    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    outputBuffer = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);

    std::unique_ptr<Recording> recording = recorder->snap();
    if (!recording) {
        ERRORF(reporter, "Failed to make recording");
        return nullptr;
    }

    InsertRecordingInfo insertInfo;
    insertInfo.fRecording = recording.get();
    context->insertRecording(insertInfo);
    testContext->syncedSubmit(context);

    T* outData = static_cast<T*>(
            map_buffer(context, testContext, outputBuffer.get(), outputInfo.fOffset));
    SkASSERT(outputBuffer->isMapped() && outData != nullptr);
    return outData;
}

// Global scan ------------------------------------------------------------------

class DecoupledFallbackComputeStep : public ComputeStep {
public:
    DecoupledFallbackComputeStep(std::string_view name,
                                 uint32_t problemSize,
                                 uint32_t workgroupSize,
                                 SkSpan<const ResourceDesc> resources)
            : ComputeStep(name, {workgroupSize, 1, 1}, resources)
            , fProblemSize(problemSize)
            , fWorkTiles((problemSize + workgroupSize - 1) / workgroupSize) {}

    void prepareUniformBuffer(int index, const ResourceDesc&, UniformManager* mgr) const override {
        SkASSERT(index == 0);
        SkDEBUGCODE(mgr->setExpectedUniforms(SkSpan<const Uniform>({{"size", SkSLType::kUInt},
                                                                    {"work_tiles", SkSLType::kUInt},
                                                                    {"unused_0", SkSLType::kUInt},
                                                                    {"unused_1", SkSLType::kUInt}}),
                                             /*isSubstruct=*/false);)
        mgr->write(fProblemSize);
        mgr->write(fWorkTiles);
        mgr->write(0u);
        mgr->write(0u);
    }

    WorkgroupSize calculateGlobalDispatchSize() const override {
        return WorkgroupSize(fWorkTiles, 1, 1);
    }

protected:
    uint32_t fProblemSize;
    uint32_t fWorkTiles;
};

struct UintConversionPolicy {
    static inline constexpr std::string_view kScanType = "uint";
    static inline constexpr std::string_view kPreamble = R"(
uint convertToScanType(uint p) {
    return p & 0x3FFFFFFFu;
}
uint convertFromScanType(uint v) {
    return v;
}
uint combine(uint a, uint b) {
    return a + b;
}
)";
};

struct UintPreamblePolicy {
    static inline constexpr std::string_view kPreamble = R"(
const uint BLOCK_DIM = 256u;
const uint PART_SIZE = BLOCK_DIM;
const uint S_MEM_SIZE = BLOCK_DIM;
)";
    static inline constexpr std::string_view kBufferPreamble = "";
};

struct UintLocalScanPolicy {
    static inline constexpr std::string_view kScan = R"(
    uint val = 0u;
    if (curve_id < size) {
        val = 1u;
    }
    wg_partials[sk_LocalInvocationID.x] = val;
    workgroupBarrier();
)";

    static inline constexpr std::string_view kFanout = R"(
    if (curve_id < size) {
        out_data[curve_id] = wg_partials[sk_LocalInvocationID.x] + wg_prev;
    }
)";
};

struct UintWorkgroupScanPolicy {
    static inline constexpr std::string_view kScan = R"(
    {
        uint end = S_MEM_SIZE >> 1;
        for (uint i = 1; i <= end; i <<= 1) {
            uint t = 0u;
            bool above = sk_LocalInvocationID.x >= i;
            if (above) {
                t = wg_partials[sk_LocalInvocationID.x - i];
            }
            workgroupBarrier();

            if (above) {
                wg_partials[sk_LocalInvocationID.x] += t;
            }
            workgroupBarrier();
        }
    }
)";

    static inline constexpr std::string_view kReduce = R"(
                {
                    uint f_val = 0u;
                    if (f_curve_id < size) {
                        f_val = 1u;
                    }
                    wg_fallback[sk_LocalInvocationID.x] = f_val;
                    workgroupBarrier();

                    uint end = S_MEM_SIZE >> 1;
                    for (uint i = 1; i <= end; i <<= 1) {
                        uint t = 0u;
                        bool above = sk_LocalInvocationID.x >= i;
                        if (above) {
                            t = wg_fallback[sk_LocalInvocationID.x - i];
                        }
                        workgroupBarrier();

                        if (above) {
                            wg_fallback[sk_LocalInvocationID.x] += t;
                        }
                        workgroupBarrier();
                    }
                }
)";
};

// Arc scan --------------------------------------------------------------------

float cpu_eval_cubic_arclength(SkV2 c0, SkV2 c1, SkV2 c2, SkV2 c3) {
    auto length_v2 = [](SkV2 v) { return std::hypot(v.x, v.y); };
    float v0 = length_v2(c1 - c0) * 0.15f;
    float v1 = length_v2(-0.558983582205757f * c0 + 0.325650248872424f * c1 +
                         0.208983582205757f * c2 + 0.024349751127576f * c3);
    float v2 = length_v2(c3 - c0 + c2 - c1) * 0.26666666666666666f;
    float v3 = length_v2(-0.024349751127576f * c0 - 0.208983582205757f * c1 -
                         0.325650248872424f * c2 + 0.558983582205757f * c3);
    float v4 = length_v2(c3 - c2) * 0.15f;
    return v0 + v1 + v2 + v3 + v4;
}

float cpu_eval_conic_arclength(SkV2 p0, SkV2 p1, SkV2 p2, float w) {
    auto length_v2 = [](SkV2 v) { return std::hypot(v.x, v.y); };
    float a = length_v2(p1 - p0);
    float b = length_v2(p2 - p1);
    if (a < b) {
        std::swap(a, b);
    }
    if (b == 0.0f) {
        return 4.0f * a;
    }
    float a_n = 1.0f;
    float b_n = b / a;
    float sum = 0.5f * (1.0f - b_n * b_n);

    // Pass 1
    float a1 = (a_n + b_n) * 0.5f;
    float b1 = std::sqrt(a_n * b_n);
    float c1 = (a_n - b_n) * 0.5f;
    sum += 1.0f * (c1 * c1);

    // Pass 2
    float a2 = (a1 + b1) * 0.5f;
    float b2 = std::sqrt(a1 * b1);
    float c2 = (a1 - b1) * 0.5f;
    sum += 2.0f * (c2 * c2);

    // Pass 3
    float c3 = (a2 - b2) * 0.5f;
    sum += 4.0f * (c3 * c3);

    return 6.283185307f * a * (1.0f - sum);
}

float truncate_fp30(float v) {
    uint32_t bits = SkFloat2Bits(v);
    bits = (bits >> 2) << 2;
    return SkBits2Float(bits);
}

struct CpuStrokePatch {
    SkV4 p01;
    SkV4 p23;
    SkV2 prevPoint;
    SkV2 stroke;
    float depth;
    float curveType;
    uint32_t ssboIndex;
    float t;
};

}  // namespace

#define DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(            \
        name, reporter, graphite_context, test_context)           \
    DEF_GRAPHITE_TEST_FOR_CONTEXTS(name,                          \
                                   is_dawn_or_metal_context_type, \
                                   reporter,                      \
                                   graphite_context,              \
                                   test_context,                  \
                                   CtsEnforcement::kNever)

DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(Scan_WorkgroupScanTest,
                                              reporter,
                                              context,
                                              testContext) {
    constexpr uint32_t kProblemSize = 256;
    constexpr uint32_t kWorkgroupSize = 256;

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    class WorkgroupScanComputeStep : public ComputeStep {
    public:
        WorkgroupScanComputeStep()
                : ComputeStep(
                          /*name=*/"WorkgroupScanTestStep",
                          /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
                          /*resources=*/
                          {{// Output buffer:
                            {
                                    /*type=*/ResourceType::kStorageBuffer,
                                    /*flow=*/DataFlow::kShared,
                                    /*policy=*/ResourcePolicy::kMapped,
                                    /*slot=*/0,
                                    /*sksl=*/"outputBlock { uint out_data[]; }",
                            }}}) {}
        ~WorkgroupScanComputeStep() override = default;

        // Note: ArcLengthScan is hard coded to float. So we perform the scan as floats. Relies on
        // the whole numbers being directly representable in f32.
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

        void prepareStorageBuffer(int, const ResourceDesc&, skgpu::BufferWriter&&) const override {}

        WorkgroupSize calculateGlobalDispatchSize() const override {
            return WorkgroupSize(1, 1, 1);
        }
    } step;

    sk_sp<Buffer> outputBuffer;
    uint32_t* outData =
            run_step_and_map_output<uint32_t>(context, testContext, reporter, &step, outputBuffer);
    if (!outData) return;

    for (unsigned int i = 0; i < kProblemSize; ++i) {
        const uint32_t expected = i * (i + 1) / 2;
        const uint32_t found = outData[i];
        REPORTER_ASSERT(reporter,
                        expected == found,
                        "At index %u: expected '%u', found '%u'",
                        i,
                        expected,
                        found);
    }
}

DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(Scan_GlobalScanTest, reporter, context, testContext) {
    constexpr uint32_t kProblemSize = 1 << 20;
    constexpr uint32_t kWorkgroupSize = 256;

    class GlobalScanComputeStep : public DecoupledFallbackComputeStep {
    public:
        GlobalScanComputeStep(uint32_t problemSize, uint32_t workgroupSize)
                : DecoupledFallbackComputeStep(
                          /*name=*/"GlobalScanTestStep",
                          problemSize,
                          workgroupSize,
                          /*resources=*/
                          {{
                                  {ResourceType::kUniformBuffer,
                                   DataFlow::kPrivate,
                                   ResourcePolicy::kMapped,
                                   "ScanUniforms { uint size; uint work_tiles; uint unused_0; uint "
                                   "unused_1; }"},
                                  {ResourceType::kStorageBuffer,
                                   DataFlow::kPrivate,
                                   ResourcePolicy::kClear,
                                   "ScanBumpAndSpine { atomicUint spine[]; }"},
                                  {ResourceType::kStorageBuffer,
                                   DataFlow::kShared,
                                   ResourcePolicy::kMapped,
                                   0,
                                   "OutputBuffer { uint out_data[]; }"},
                          }}) {}
        ~GlobalScanComputeStep() override = default;

        std::string computeSkSL() const override {
            using Composer = ScanComposer<UintPreamblePolicy,
                                          UintLocalScanPolicy,
                                          UintWorkgroupScanPolicy,
                                          UintConversionPolicy>;
            return std::string(Composer::kShader);
        }

        size_t calculateBufferSize(int index, const ResourceDesc&) const override {
            if (index == 1) {
                // Spine buffer: flags/partials for each tile + bump allocator index at
                // spine[work_tiles]
                return sizeof(uint32_t) * (fWorkTiles + 1);
            }
            SkASSERT(index == 2);
            return sizeof(uint32_t) * fProblemSize;
        }

        void prepareStorageBuffer(int, const ResourceDesc&, skgpu::BufferWriter&&) const override {}
    } step(kProblemSize, kWorkgroupSize);

    sk_sp<Buffer> outputBuffer;
    uint32_t* outData =
            run_step_and_map_output<uint32_t>(context, testContext, reporter, &step, outputBuffer);
    if (!outData) return;

    for (unsigned int i = 0; i < kProblemSize; ++i) {
        const uint32_t expected = i + 1;
        const uint32_t found = outData[i];
        REPORTER_ASSERT(reporter,
                        expected == found,
                        "At index %u: expected '%u', found '%u'",
                        i,
                        expected,
                        found);
    }
}

DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(Scan_ArcLengthGlobalScanTest,
                                              reporter,
                                              context,
                                              testContext) {
    constexpr uint32_t kProblemSize = 512;
    constexpr uint32_t kWorkgroupSize = 256;
    const bool infinitySupport = context->priv().caps()->shaderCaps()->fInfinitySupport;

    class ArcLengthGlobalScanComputeStep : public DecoupledFallbackComputeStep {
    public:
        ArcLengthGlobalScanComputeStep(uint32_t problemSize,
                                       uint32_t workgroupSize,
                                       bool infinitySupport)
                : DecoupledFallbackComputeStep(
                          /*name=*/"ArcLengthGlobalScanTestStep",
                          problemSize,
                          workgroupSize,
                          /*resources=*/
                          {{
                                  {ResourceType::kUniformBuffer,
                                   DataFlow::kPrivate,
                                   ResourcePolicy::kMapped,
                                   "ScanUniforms { uint size; uint work_tiles; uint unused_0; uint "
                                   "unused_1; };\n\n"
                                   "struct StrokePatch {\n"
                                   "    float4 p01;\n"
                                   "    float4 p23;\n"
                                   "    float2 prevPoint;\n"
                                   "    float2 stroke;\n"
                                   "    float depth;\n"
                                   "    float curveType;\n"
                                   "    uint ssboIndex;\n"
                                   "    float t;\n"
                                   "}"},
                                  {ResourceType::kStorageBuffer,
                                   DataFlow::kPrivate,
                                   ResourcePolicy::kClear,
                                   "ScanBumpAndSpine { atomicUint spine[]; }"},
                                  {ResourceType::kStorageBuffer,
                                   DataFlow::kShared,
                                   ResourcePolicy::kMapped,
                                   0,
                                   "InputPatches { StrokePatch patches[]; }"},
                          }})
                , fInfinitySupport(infinitySupport) {}
        ~ArcLengthGlobalScanComputeStep() override = default;

        std::string computeSkSL() const override {
            if (fInfinitySupport) {
                using Composer = ScanComposer<ArcLengthScan::Preamble,
                                              ArcLengthScan::LocalScan,
                                              ArcLengthScan::WorkgroupScan,
                                              ArcLengthScan::Conversion>;
                return std::string(Composer::kShader);
            } else {
                using Composer = ScanComposer<ArcLengthScan::PreambleWithCurveType,
                                              ArcLengthScan::LocalScan,
                                              ArcLengthScan::WorkgroupScan,
                                              ArcLengthScan::Conversion>;
                return std::string(Composer::kShader);
            }
        }

        size_t calculateBufferSize(int index, const ResourceDesc&) const override {
            if (index == 1) {
                return sizeof(uint32_t) * (fWorkTiles + 1);
            }
            SkASSERT(index == 2);
            return sizeof(CpuStrokePatch) * fProblemSize;
        }

        void prepareStorageBuffer(int index,
                                  const ResourceDesc&,
                                  skgpu::BufferWriter&& writer) const override {
            if (index != 2) return;
            for (uint32_t i = 0; i < fProblemSize; ++i) {
                bool isConic = (i % 2 == 1);
                SkV2 p0{1.0f * i, 0.0f};
                SkV2 p1{1.0f * i + 1.0f, 2.0f};
                SkV2 p2{1.0f * i + 2.0f, 2.0f};
                SkV2 p3{1.0f * i + 3.0f, 0.0f};
                float w = 0.707f;

                CpuStrokePatch patch;
                patch.p01 = {p0.x, p0.y, p1.x, p1.y};
                if (isConic) {
                    patch.p23 = {p2.x, p2.y, w, SK_FloatInfinity};
                } else {
                    patch.p23 = {p2.x, p2.y, p3.x, p3.y};
                }
                patch.prevPoint = {p0.x - 1.0f, p0.y};
                patch.stroke = {1.0f, 1.0f};
                patch.depth = 1.0f;
                patch.curveType = isConic ? 1.0f : 0.0f;
                patch.ssboIndex = i;
                patch.t = 0.0f;

                writer.write(patch);
            }
        }

    private:
        bool fInfinitySupport;
    } step(kProblemSize, kWorkgroupSize, infinitySupport);

    sk_sp<Buffer> outputBuffer;
    CpuStrokePatch* outPatches = run_step_and_map_output<CpuStrokePatch>(
            context, testContext, reporter, &step, outputBuffer);
    if (!outPatches) return;

    // Compute CPU reference lens and inclusive prefix scan with FP30 truncation
    std::vector<float> lens(kProblemSize);
    for (uint32_t i = 0; i < kProblemSize; ++i) {
        bool isConic = (i % 2 == 1);
        SkV2 p0{1.0f * i, 0.0f};
        SkV2 p1{1.0f * i + 1.0f, 2.0f};
        SkV2 p2{1.0f * i + 2.0f, 2.0f};
        SkV2 p3{1.0f * i + 3.0f, 0.0f};
        float w = 0.707f;
        if (isConic) {
            lens[i] = cpu_eval_conic_arclength(p0, p1, p2, w);
        } else {
            lens[i] = cpu_eval_cubic_arclength(p0, p1, p2, p3);
        }
    }

    uint32_t workTiles = (kProblemSize + kWorkgroupSize - 1) / kWorkgroupSize;
    std::vector<float> expected_t(kProblemSize);
    float wg_prev = 0.0f;
    for (uint32_t tile = 0; tile < workTiles; ++tile) {
        float local_sum = 0.0f;
        for (uint32_t local = 0; local < kWorkgroupSize; ++local) {
            uint32_t idx = tile * kWorkgroupSize + local;
            if (idx < kProblemSize) {
                local_sum += lens[idx];
                expected_t[idx] = local_sum + wg_prev;
            }
        }
        wg_prev += truncate_fp30(local_sum);
    }

    for (unsigned int i = 0; i < kProblemSize; ++i) {
        const float expected = expected_t[i];
        const float found = outPatches[i].t;
        const float absDiff = std::abs(expected - found);
        const float maxAbsDiff = 1e-3f;
        const float relDiff = absDiff / std::max(1.0f, std::abs(expected));
        REPORTER_ASSERT(reporter,
                        relDiff <= 1e-3f || absDiff <= maxAbsDiff,
                        "At index %u: expected '%f', found '%f' (absDiff %f, relDiff %f)",
                        i,
                        expected,
                        found,
                        absDiff,
                        relDiff);
    }
}
