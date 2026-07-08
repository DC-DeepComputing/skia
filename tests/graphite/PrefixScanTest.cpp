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
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/compute/PrefixScanComposer.h"
#include "src/gpu/graphite/compute/shaders/ArcLengthSegPrefixScan.h"
#include "src/gpu/graphite/task/ComputeTask.h"
#include "src/gpu/graphite/task/CopyTask.h"
#include "src/gpu/graphite/task/SynchronizeToCpuTask.h"
#include "src/gpu/tessellate/ArcLengthCalculator.h"
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
            "PrefixScanTest_TransferToCpu");
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

// Global prefix scan -----------------------------------------------------------

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
                                             /*isSubstruct=*/false);) mgr->write(fProblemSize);
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
    static inline constexpr std::string_view kPrefixScanType = "uint";
    static inline constexpr std::string_view kPreamble =
            "uint convertToPrefixScanType(uint p) {\n"
            "    return p & 0x3FFFFFFFu;\n"
            "}\n"
            "uint convertFromPrefixScanType(uint v) {\n"
            "    return v;\n"
            "}\n"
            "uint combine(uint a, uint b) {\n"
            "    return a + b;\n"
            "}\n";
};

struct UintPreamblePolicy {
    static inline constexpr std::string_view kPreamble =
            "const uint BLOCK_DIM = 256u;\n"
            "const uint PART_SIZE = BLOCK_DIM;\n"
            "const uint S_MEM_SIZE = BLOCK_DIM;\n";
    static inline constexpr std::string_view kBufferPreamble = "";
};

struct UintLocalPrefixScanPolicy {
    static inline constexpr std::string_view kPrefixScan =
            "    uint val = 0u;\n"
            "    if (curve_id < size) {\n"
            "        val = 1u;\n"
            "    }\n"
            "    is_inclusive = tile_id == 0u;\n"
            "    wg_partials[sk_LocalInvocationID.x] = val;\n"
            "    workgroupBarrier();\n";

    static inline constexpr std::string_view kFanout =
            "    if (curve_id < size) {\n"
            "        out_data[curve_id] = wg_partials[sk_LocalInvocationID.x] + wg_prev;\n"
            "    }\n";
};

struct UintWorkgroupPrefixScanPolicy {
    static inline constexpr std::string_view kPrefixScan =
            "    {\n"
            "        uint end = S_MEM_SIZE >> 1;\n"
            "        for (uint i = 1; i <= end; i <<= 1) {\n"
            "            uint t = 0u;\n"
            "            bool above = sk_LocalInvocationID.x >= i;\n"
            "            if (above) {\n"
            "                t = wg_partials[sk_LocalInvocationID.x - i];\n"
            "            }\n"
            "            workgroupBarrier();\n"
            "\n"
            "            if (above) {\n"
            "                wg_partials[sk_LocalInvocationID.x] += t;\n"
            "            }\n"
            "            workgroupBarrier();\n"
            "        }\n"
            "    }\n";

    static inline constexpr std::string_view kFallback =
            "                {\n"
            "                    uint f_val = 0u;\n"
            "                    if (f_curve_id < size) {\n"
            "                        f_val = 1u;\n"
            "                    }\n"
            "                    wg_fallback[sk_LocalInvocationID.x] = f_val;\n"
            "                    workgroupBarrier();\n"
            "\n"
            "                    uint end = S_MEM_SIZE >> 1;\n"
            "                    for (uint i = 1; i <= end; i <<= 1) {\n"
            "                        uint t = 0u;\n"
            "                        bool above = sk_LocalInvocationID.x >= i;\n"
            "                        if (above) {\n"
            "                            t = wg_fallback[sk_LocalInvocationID.x - i];\n"
            "                        }\n"
            "                        workgroupBarrier();\n"
            "\n"
            "                        if (above) {\n"
            "                            wg_fallback[sk_LocalInvocationID.x] += t;\n"
            "                        }\n"
            "                        workgroupBarrier();\n"
            "                    }\n"
            "                }\n";
};

// Arc prefix scan -------------------------------------------------------------

struct CpuStrokePatch {
    SkV4 p01;
    SkV4 p23;
    float curveType;
    uint32_t pathIndex;
    float t;
    uint32_t padding;
};

float truncate_fp30(float v) {
    uint32_t bits = SkFloat2Bits(v);
    bits = (bits >> 2) << 2;
    return SkBits2Float(bits);
}

void run_arclength_prefixScan_test(skiatest::Reporter* reporter,
                             Context* context,
                             skiatest::graphite::GraphiteTestContext* testContext,
                             std::vector<uint32_t> pathAttributes,
                             int loops = 1) {
    constexpr uint32_t kProblemSize = 1 << 19;
    constexpr uint32_t kWorkgroupSize = 256;
    const bool infinitySupport = context->priv().caps()->shaderCaps()->fInfinitySupport;

    // Compute CPU reference lens and inclusive prefix scan with FP30 truncation once
    std::vector<float> lens(kProblemSize);
    for (uint32_t i = 0; i < kProblemSize; ++i) {
        bool isConic = (i % 2 == 1);
        SkV2 p0{1.0f * i, 0.0f};
        SkV2 p1{1.0f * i + 1.0f, 2.0f};
        SkV2 p2{1.0f * i + 2.0f, 2.0f};
        SkV2 p3{1.0f * i + 3.0f, 0.0f};
        float w = 0.707f;
        if (isConic) {
            lens[i] = skgpu::tess::ArcLengthCalculator::ConicLength(
                    {p0.x, p0.y}, {p1.x, p1.y}, {p2.x, p2.y}, w);
        } else {
            lens[i] = skgpu::tess::ArcLengthCalculator::CubicLength(
                    {p0.x, p0.y}, {p1.x, p1.y}, {p2.x, p2.y}, {p3.x, p3.y});
        }
    }

    std::vector<float> expectedCurveLens(kProblemSize);
    std::vector<float> expectedPathLens(pathAttributes.size());
    for (size_t p = 0; p < pathAttributes.size(); ++p) {
        uint32_t start = pathAttributes[p];
        uint32_t end = (p + 1 < pathAttributes.size()) ? pathAttributes[p + 1] : kProblemSize;

        uint32_t pathStartTile = start / kWorkgroupSize;
        uint32_t pathEndTile = (end - 1) / kWorkgroupSize;

        float wgPrev = 0.0f;
        for (uint32_t tile = pathStartTile; tile <= pathEndTile; ++tile) {
            uint32_t tileStart = tile * kWorkgroupSize;
            uint32_t tileEnd = std::min(end, (tile + 1) * kWorkgroupSize);

            float localSum = 0.0f;
            uint32_t localStart = std::max(start, tileStart);
            for (uint32_t i = localStart; i < tileEnd; ++i) {
                localSum += lens[i];
                expectedCurveLens[i] = localSum + wgPrev;
            }

            if (tile == pathStartTile) {
                wgPrev = truncate_fp30(localSum);
            } else {
                wgPrev = truncate_fp30(localSum + wgPrev);
            }
        }
        expectedPathLens[p] = expectedCurveLens[end - 1];
    }

    class ArcLengthGlobalPrefixScanComputeStep : public DecoupledFallbackComputeStep {
    public:
        ArcLengthGlobalPrefixScanComputeStep(uint32_t problemSize,
                                       uint32_t workgroupSize,
                                       bool infinitySupport,
                                       std::vector<uint32_t> pathAttributes)
                : DecoupledFallbackComputeStep(
                          /*name=*/"ArcLengthGlobalPrefixScanTestStep",
                          problemSize,
                          workgroupSize,
                          /*resources=*/
                          {{
                                  {ResourceType::kUniformBuffer,
                                   DataFlow::kPrivate,
                                   ResourcePolicy::kMapped,
                                   "PrefixScanUniforms { uint size; uint work_tiles; uint num_paths;"
                                   "uint unused_1; };\n\n"
                                   "struct StrokePatch {\n"
                                   "    float4 p01;\n"
                                   "    float4 p23;\n"
                                   "    float curveType;\n"
                                   "    uint pathIndex;\n"
                                   "    float t;\n"
                                   "    uint padding;\n"
                                   "};\n\n"
                                   "struct PathAttributes {\n"
                                   "    uint startIndex;\n"
                                   "    float totalLength;\n"
                                   "    float padding1;\n"
                                   "    float padding2;\n"
                                   "}"},
                                  {ResourceType::kStorageBuffer,
                                   DataFlow::kPrivate,
                                   ResourcePolicy::kClear,
                                   "PrefixScanBumpAndSpine { atomicUint spine[]; }"},
                                  {ResourceType::kStorageBuffer,
                                   DataFlow::kShared,
                                   ResourcePolicy::kMapped,
                                   0,
                                   "InputPatches { StrokePatch patches[]; }"},
                                  {ResourceType::kStorageBuffer,
                                   DataFlow::kShared,
                                   ResourcePolicy::kMapped,
                                   1,
                                   "PathStorageBuffer { PathAttributes pathData[]; }"},
                          }})
                , fInfinitySupport(infinitySupport)
                , fPathAttributes(std::move(pathAttributes)) {}
        ~ArcLengthGlobalPrefixScanComputeStep() override = default;

        std::string computeSkSL() const override {
            if (fInfinitySupport) {
                using Composer = PrefixScanComposer<ArcLengthSegPrefixScan::Preamble,
                                              ArcLengthSegPrefixScan::LocalPrefixScan,
                                              ArcLengthSegPrefixScan::WorkgroupPrefixScan,
                                              ArcLengthSegPrefixScan::Conversion>;
                return std::string(Composer::kShader);
            } else {
                using Composer = PrefixScanComposer<ArcLengthSegPrefixScan::PreambleWithCurveType,
                                              ArcLengthSegPrefixScan::LocalPrefixScan,
                                              ArcLengthSegPrefixScan::WorkgroupPrefixScan,
                                              ArcLengthSegPrefixScan::Conversion>;
                return std::string(Composer::kShader);
            }
        }

        void prepareUniformBuffer(int index, const ResourceDesc&, UniformManager* mgr) const override {
            SkASSERT(index == 0);
            SkDEBUGCODE(mgr->setExpectedUniforms(SkSpan<const Uniform>({{"size", SkSLType::kUInt},
                                                                        {"work_tiles", SkSLType::kUInt},
                                                                        {"num_paths", SkSLType::kUInt},
                                                                        {"unused_1", SkSLType::kUInt}}),
                                                 /*isSubstruct=*/false);)
            mgr->write(fProblemSize);
            mgr->write(fWorkTiles);
            mgr->write(static_cast<uint32_t>(fPathAttributes.size()));
            mgr->write(0u);
        }

        size_t calculateBufferSize(int index, const ResourceDesc&) const override {
            if (index == 1) {
                return sizeof(uint32_t) * (fWorkTiles + 1);
            }
            if (index == 2) {
                return sizeof(CpuStrokePatch) * fProblemSize;
            }
            if (index == 3) {
                return sizeof(uint32_t) * fPathAttributes.size() * 4;
            }
            return 0;
        }

        void prepareStorageBuffer(int index,
                                  const ResourceDesc&,
                                  skgpu::BufferWriter&& writer) const override {
            if (index == 2) {
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
                    patch.curveType = isConic ? 1.0f : 0.0f;
                    uint32_t pathIndex = 0;
                    for (size_t p = 0; p < fPathAttributes.size(); ++p) {
                        if (i >= fPathAttributes[p]) {
                            pathIndex = p;
                        }
                    }
                    patch.pathIndex = pathIndex;
                    patch.t = 0.0f;

                    writer.write(patch);
                }
            } else if (index == 3) {
                struct PathAttributes {
                    uint32_t startIndex;
                    float totalLength;
                    float padding1;
                    float padding2;
                };
                std::vector<PathAttributes> strokeData(fPathAttributes.size());
                for (size_t i = 0; i < fPathAttributes.size(); ++i) {
                    strokeData[i] = {fPathAttributes[i], 0.f, 0.f, 0.f};
                }
                writer.write(strokeData.data(), strokeData.size() * sizeof(PathAttributes));
            }
        }

    private:
        bool fInfinitySupport;
        std::vector<uint32_t> fPathAttributes;
    } step(kProblemSize, kWorkgroupSize, infinitySupport, pathAttributes);

    for (int loop = 0; loop < loops; ++loop) {
        std::unique_ptr<Recorder> recorder = context->makeRecorder();
        DispatchGroup::Builder builder(recorder.get());
        if (!builder.appendStep(&step)) {
            ERRORF(reporter, "Failed to add ComputeStep to DispatchGroup");
            return;
        }

        BindBufferInfo outputInfo = builder.getSharedBufferResource(0);
        BindBufferInfo pathAttrInfo = builder.getSharedBufferResource(1);
        if (!outputInfo || !pathAttrInfo) {
            ERRORF(reporter, "Failed to allocate shared buffers");
            return;
        }

        ComputeTask::DispatchGroupList groups;
        groups.push_back(builder.finalize());
        recorder->priv().add(ComputeTask::Make(std::move(groups)));

        sk_sp<Buffer> outputBuffer = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);
        sk_sp<Buffer> pathAttrBuffer = sync_buffer_to_cpu(recorder.get(), pathAttrInfo.fBuffer);

        std::unique_ptr<Recording> recording = recorder->snap();
        if (!recording) {
            ERRORF(reporter, "Failed to make recording");
            return;
        }

        InsertRecordingInfo insertInfo;
        insertInfo.fRecording = recording.get();
        context->insertRecording(insertInfo);
        testContext->syncedSubmit(context);

        CpuStrokePatch* outPatches = static_cast<CpuStrokePatch*>(
                map_buffer(context, testContext, outputBuffer.get(), outputInfo.fOffset));
        struct CpuPathAttributes {
            uint32_t startIndex;
            float totalLength;
            float padding1;
            float padding2;
        };
        CpuPathAttributes* outPathAttrs = static_cast<CpuPathAttributes*>(
                map_buffer(context, testContext, pathAttrBuffer.get(), pathAttrInfo.fOffset));
        if (!outPatches || !outPathAttrs) return;

        for (unsigned int i = 0; i < kProblemSize; ++i) {
            const float expected = expectedCurveLens[i];
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

        for (size_t p = 0; p < pathAttributes.size(); ++p) {
            // Assert start index is preserved
            REPORTER_ASSERT(reporter,
                            outPathAttrs[p].startIndex == pathAttributes[p],
                            "At path %zu: expected start index '%u', found '%u'",
                            p,
                            pathAttributes[p],
                            outPathAttrs[p].startIndex);

            const float expected = expectedPathLens[p];
            const float found = outPathAttrs[p].totalLength;
            const float absDiff = std::abs(expected - found);
            const float maxAbsDiff = 1e-3f;
            const float relDiff = absDiff / std::max(1.0f, std::abs(expected));
            REPORTER_ASSERT(reporter,
                            relDiff <= 1e-3f || absDiff <= maxAbsDiff,
                            "At path %zu: expected total length '%f', found '%f' (absDiff %f, relDiff %f)",
                            p,
                            expected,
                            found,
                            absDiff,
                            relDiff);
        }
    }
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

DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(PrefixScan_WorkgroupScanTest,
                                              reporter,
                                              context,
                                              testContext) {
    constexpr uint32_t kProblemSize = 256;
    constexpr uint32_t kWorkgroupSize = 256;

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    class WorkgroupPrefixScanComputeStep : public ComputeStep {
    public:
        WorkgroupPrefixScanComputeStep()
                : ComputeStep(
                          /*name=*/"WorkgroupPrefixScanTestStep",
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
        ~WorkgroupPrefixScanComputeStep() override = default;

        // Note: ArcLengthSegPrefixScan is hard coded to float. So we perform the scan as floats. Relies on
        // the whole numbers being directly representable in f32.
        std::string computeSkSL() const override {
            std::string shader;
            shader += "const uint S_MEM_SIZE = 256u;\n";
            shader += "const uint PART_SIZE = S_MEM_SIZE;\n";
            shader += "workgroup float wg_partials[S_MEM_SIZE];\n";

            // Mock environment for ArcLengthSegPrefixScan::WorkgroupPrefixScan::kPrefixScan's segmented scan logic.
            // Since we are running a single non-segmented test, we configure the mock to result
            // in a local_subtract of 0.
            shader += "struct StrokePatch {\n";
            shader += "    uint pathIndex;\n";
            shader += "};\n";
            shader += "const uint size = S_MEM_SIZE;\n";
            shader += "const uint tile_id = 0u;\n";
            shader += "uint start_index = 0u;\n";
            shader += "workgroup StrokePatch patches[S_MEM_SIZE];\n";
            shader += "struct PathAttributes {\n";
            shader += "    uint startIndex;\n";
            shader += "    float totalLength;\n";
            shader += "    float padding1;\n";
            shader += "    float padding2;\n";
            shader += "};\n";
            shader += "workgroup PathAttributes pathData[2];\n";

            shader += "void main() {\n";
            shader += "    uint idx = sk_LocalInvocationID.x;\n";
            shader += "    uint curve_id = idx;\n";
            shader += "    wg_partials[idx] = float(idx);\n";
            shader += "    patches[idx].pathIndex = 0u;\n";
            shader += "    if (idx == 0u) {\n";
            shader += "        pathData[0].startIndex = 0u;\n";
            shader += "    }\n";
            shader += "    workgroupBarrier();\n";

            shader += ArcLengthSegPrefixScan::WorkgroupPrefixScan::kPrefixScan;
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

DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(PrefixScan_GlobalScanTest, reporter, context, testContext) {
    constexpr uint32_t kProblemSize = 1 << 20;
    constexpr uint32_t kWorkgroupSize = 256;

    class GlobalPrefixScanComputeStep : public DecoupledFallbackComputeStep {
    public:
        GlobalPrefixScanComputeStep(uint32_t problemSize, uint32_t workgroupSize)
                : DecoupledFallbackComputeStep(
                          /*name=*/"GlobalPrefixScanTestStep",
                          problemSize,
                          workgroupSize,
                          /*resources=*/
                          {{
                                  {ResourceType::kUniformBuffer,
                                   DataFlow::kPrivate,
                                   ResourcePolicy::kMapped,
                                   "PrefixScanUniforms { uint size; uint work_tiles; uint unused_0; uint "
                                   "unused_1; }"},
                                  {ResourceType::kStorageBuffer,
                                   DataFlow::kPrivate,
                                   ResourcePolicy::kClear,
                                   "PrefixScanBumpAndSpine { atomicUint spine[]; }"},
                                  {ResourceType::kStorageBuffer,
                                   DataFlow::kShared,
                                   ResourcePolicy::kMapped,
                                   0,
                                   "OutputBuffer { uint out_data[]; }"},
                          }}) {}
        ~GlobalPrefixScanComputeStep() override = default;

        std::string computeSkSL() const override {
            using Composer = PrefixScanComposer<UintPreamblePolicy,
                                          UintLocalPrefixScanPolicy,
                                          UintWorkgroupPrefixScanPolicy,
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

// Test using a single segment
DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(PrefixScan_ArcLengthGlobalScanTest,
                                              reporter,
                                              context,
                                              testContext) {
    run_arclength_prefixScan_test(reporter, context, testContext, {0});
}

// Test using multiple segments
DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(PrefixScan_ArcLengthSegmentedGlobalScanTest,
                                              reporter,
                                              context,
                                              testContext) {
    run_arclength_prefixScan_test(reporter, context, testContext, {0, 10, 256, 257, 510, 1024, 1530,
                                                             65534, 283192}, 50);
}
