/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/task/ComputeTask.h"

#include "src/gpu/graphite/sparse_strips/SparseStripsConstants.h"
#include "src/gpu/graphite/sparse_strips/shaders/NonSubgroupWGSL.h"

#include "tests/graphite/sparse_strips/compute/TestUtilsCompute.h"
#include "tools/graphite/GraphiteTestContext.h"

#include <memory>
#include <string>
#include <vector>

using namespace skgpu::graphite;
using namespace skiatest::graphite;

class GPUSegmentedReductionTestRunner {
public:
    static void runAll(skiatest::Reporter* reporter,
                       Context* context,
                       GraphiteTestContext* testContext) {

        constexpr uint32_t kPartSize = 32;
        constexpr uint32_t kWorkgroupSize = kPartSize * 4;

        runStep(reporter, context, testContext, "ScanBaseOnly", false, kPartSize, kWorkgroupSize);
        runStep(reporter, context, testContext, "ScanAndSegmentedReduce", true, kPartSize, kWorkgroupSize);
    }

private:
    static void runStep(skiatest::Reporter* reporter,
                        Context* context,
                        GraphiteTestContext* testContext,
                        const std::string& stepName,
                        bool includeReduction,
                        uint32_t partSize,
                        uint32_t workgroupSize) {

        class ScanComputeStep : public ComputeStep {
        public:
            ScanComputeStep(const std::string& name, bool includeReduction, uint32_t partSize, uint32_t wgSize)
                : ComputeStep(
                    /*name=*/name,
                    /*localDispatchSize=*/{wgSize, 1, 1},
                    /*resources=*/{
                        {ResourceType::kStorageBuffer, DataFlow::kShared, ResourcePolicy::kMapped, 0,
                         "outputBlock { out_data: array<vec2<u32>> }"} // Output buffer
                    },
                    /*workgroupBuffers=*/{},
                    /*baseFlags=*/Flags::kSupportsNativeShader),
                  fIncludeReduction(includeReduction), fPartSize(partSize), fWgSize(wgSize) {

                fShaderText = GetWgslConstants();
                fShaderText += "const PART_SIZE: u32 = " + std::to_string(fPartSize) + "u;\n";
                fShaderText += "const TILE_WIDTH: u32 = 4u;\n"; // Real TILE_WIDTH
                fShaderText += "const TILE_HEIGHT: u32 = 4u;\n";
                fShaderText += "const VEC2_COLS: u32 = TILE_WIDTH * 2u / 2u;\n";
                fShaderText += "const MSAA_BIAS_SPLAT: vec2<u32> = vec2<u32>(0x80808080u, 0x80808080u);\n";
                fShaderText += kWgslHelpers;

                fShaderText += R"(
                    struct PreMergeTile {
                        packed_scanned_winding: u32,
                    };
                )";

                fShaderText += kWgslWorkgroupVars;

                fShaderText += R"(
                    @group(0) @binding(0) var<storage, read_write> out_data: array<vec2<u32>>;
                )";

                fShaderText += "@compute @workgroup_size(" + std::to_string(fWgSize) + ", 1, 1)\n";
                fShaderText += R"(
                    fn main(@builtin(local_invocation_id) tid: vec3<u32>) {
                        var mask: array<vec2<u32>, TILE_WIDTH>;

                        for (var i = 0u; i < TILE_WIDTH; i += 1u) {
                            mask[i] = vec2<u32>(0x81818181u, 0x81818181u);
                        }

                        // Mock PMT - Segment sizes: 8, 7, 5, 3, 2, 1, 6 (to pad to 32)
                        var pmt: PreMergeTile;
                        let t_idx = tid.x >> 2u;

                        var start_id = 0u;
                        var is_end_mock = 0u;

                        if (t_idx <= 7u) {
                            start_id = 0u;
                            is_end_mock = u32(t_idx == 7u);
                        } else if (t_idx <= 14u) {
                            start_id = 8u;
                            is_end_mock = u32(t_idx == 14u);
                        } else if (t_idx <= 19u) {
                            start_id = 15u;
                            is_end_mock = u32(t_idx == 19u);
                        } else if (t_idx <= 22u) {
                            start_id = 20u;
                            is_end_mock = u32(t_idx == 22u);
                        } else if (t_idx <= 24u) {
                            start_id = 23u;
                            is_end_mock = u32(t_idx == 24u);
                        } else if (t_idx == 25u) {
                            start_id = 25u;
                            is_end_mock = 1u;
                        } else {
                            start_id = 26u;
                            is_end_mock = u32(t_idx == 31u);
                        }

                        pmt.packed_scanned_winding = (start_id & START_TILE_MASK) |
                                                     (is_end_mock << IS_END_TILE_SHIFT);
                )";

                fShaderText += kWgslScan_Base;
                if (fIncludeReduction) {
                    fShaderText += kWgslScan_SegmentedReduction;
                }

                fShaderText += R"(
                        for (var i = 0u; i < TILE_WIDTH; i += 1u) {
                            out_data[tid.x * TILE_WIDTH + i] = mask[i];
                        }
                    }
                )";
            }

            NativeShaderSource nativeShaderSource(NativeShaderFormat format) const override {
                return {fShaderText, "main"};
            }

            size_t calculateBufferSize(int index, const ResourceDesc& r) const override {
                // TILE_WIDTH is 4, each is a vec2<u32> (2 uints)
                return sizeof(uint32_t) * 2 * fWgSize * 4;
            }

            WorkgroupSize calculateGlobalDispatchSize() const override {
                return WorkgroupSize(1, 1, 1);
            }

            void prepareStorageBuffer(int resourceIndex,
                                      const ResourceDesc& r,
                                      skgpu::BufferWriter&& writer) const override {
                if (resourceIndex == 0) {
                    size_t size = calculateBufferSize(resourceIndex, r);
                    std::vector<uint8_t> zeros(size, 0);
                    writer.write(zeros.data(), size);
                }
            }

        private:
            bool fIncludeReduction;
            uint32_t fPartSize;
            uint32_t fWgSize;
            std::string fShaderText;
        };

        ScanComputeStep step(stepName, includeReduction, partSize, workgroupSize);
        std::unique_ptr<Recorder> recorder = context->makeRecorder();
        DispatchGroup::Builder builder(recorder.get());
        if (!builder.appendStep(&step)) return;

        BindBufferInfo outputInfo = builder.getSharedBufferResource(0);

        ComputeTask::DispatchGroupList groups;
        groups.push_back(builder.finalize());
        recorder->priv().add(ComputeTask::Make(std::move(groups)));

        auto outputBuffer = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);
        submit_recording(context, testContext, recorder.get());

        uint32_t* outData = static_cast<uint32_t*>(
                map_buffer(context, testContext, outputBuffer.get(), outputInfo.fOffset));
        SkASSERT(outputBuffer->isMapped() && outData != nullptr);

        auto getStartId = [](uint32_t t) -> uint32_t {
            if (t <= 7)  return 0;
            if (t <= 14) return 8;
            if (t <= 19) return 15;
            if (t <= 22) return 20;
            if (t <= 24) return 23;
            if (t == 25) return 25;
            return 26;
        };

        auto isEndTile = [](uint32_t t) -> bool {
            return t == 7 || t == 14 || t == 19 || t == 22 || t == 24 || t == 25 || t == 31;
        };

        for (uint32_t t = 0; t < workgroupSize; ++t) {
            uint32_t tileIndex = t >> 2;

            uint32_t expectedSum = tileIndex + 1;

            if (includeReduction) {
                // Segmented reduction natively fires on explicitly marked end tiles
                // OR on the very last tile of the part (PART_SIZE - 1)
                bool isEndOrLast = isEndTile(tileIndex) || tileIndex == partSize - 1;
                uint32_t startId = getStartId(tileIndex);

                if (isEndOrLast && startId != 0) {
                    expectedSum -= startId;
                }
            }

            uint32_t expectedByte = expectedSum + 0x80;
            uint32_t expectedPacked = (expectedByte << 24) | (expectedByte << 16) | (expectedByte << 8) | expectedByte;

            for (uint32_t col = 0; col < 4; ++col) {
                uint32_t actualX = outData[(t * 4 + col) * 2 + 0];
                uint32_t actualY = outData[(t * 4 + col) * 2 + 1];

                REPORTER_ASSERT(
                    reporter,
                    actualX == expectedPacked && actualY == expectedPacked,
                    "[%s] Failed at thread %u, tile %u, col %u. Expected 0x%08X, got X:0x%08X Y:0x%08X",
                    stepName.c_str(), t, tileIndex, col, expectedPacked, actualX, actualY);
            }
        }

        if (outputBuffer->isMapped()) {
            outputBuffer->unmap();
        }
    }
};

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(SparseStrips_Compute_ScanAndReduction, reporter, context, testContext) {
    if (testContext->contextType() == skgpu::ContextType::kDawn_D3D11) { return; }
    GPUSegmentedReductionTestRunner::runAll(reporter, context, testContext);
}
