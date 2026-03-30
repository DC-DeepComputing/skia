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
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConstants.h"
#include "src/gpu/graphite/sparse_strips/shaders/NonSubgroupWGSL.h"
#include "src/gpu/graphite/task/ComputeTask.h"

#include "tests/graphite/sparse_strips/compute/TestUtilsCompute.h"
#include "tools/graphite/GraphiteTestContext.h"

#include <cmath>
#include <vector>

using namespace skgpu::graphite;
using namespace skiatest::graphite;

namespace {

template <uint16_t WIDTH, uint16_t HEIGHT> struct BitCountingTestCase {
    uint32_t masks[HEIGHT][WIDTH][2];
    uint32_t expected_alphas[HEIGHT];
};

template <uint16_t WIDTH, uint16_t HEIGHT>
std::vector<BitCountingTestCase<WIDTH, HEIGHT>> MakeBitCountingTestCases() {
    static_assert(WIDTH == 4);

    std::vector<BitCountingTestCase<WIDTH, HEIGHT>> cases;

    auto addCase = [&](uint32_t fillVal1, uint32_t fillVal2) {
        BitCountingTestCase<WIDTH, HEIGHT> tc;
        for (int r = 0; r < HEIGHT; ++r) {
            for (int c = 0; c < WIDTH; ++c) {
                tc.masks[r][c][0] = fillVal1;
                tc.masks[r][c][1] = fillVal2;
            }
        }
        cases.push_back(tc);
    };

    addCase(0x80808080, 0x80808080);
    addCase(0xFFFFFFFF, 0xFFFFFFFF);
    addCase(0x80FF80FF, 0xFF80FF80);

    addCase(0xFFFFFFFF, 0x01010101);
    addCase(0x000000FF, 0x00000001);
    addCase(0x0000FF00, 0x00000100);

    addCase(0x00000000, 0x01010101);
    addCase(0x00000100, 0x00000001);

    addCase(0x7F7F7F7F, 0x01010101);
    addCase(0x7F7F7F7F, 0x7F7F7F7F);

    addCase(0xFF00FF00, 0x01000100);
    addCase(0x00FF00FF, 0x00010001);
    addCase(0x00FF00FF, 0xFF00FF00);

    addCase(0xFEFEFEFE, 0x01010101);
    addCase(0x01010101, 0xFFFFFFFF);
    addCase(0x81818181, 0x7E7E7E7E);

    BitCountingTestCase<WIDTH, HEIGHT> grad;
    for (int r = 0; r < HEIGHT; ++r) {
        for (int c = 0; c < WIDTH; ++c) {
            uint32_t m0 = 0, m1 = 0;
            for (int b = 0; b < 4; ++b) {
                uint32_t val1 = (r * WIDTH + c + b) % 2 == 0 ? 0x80 : 0x00;
                m0 |= (val1 << (b * 8));
                uint32_t val2 = (r * WIDTH + c + b + 1) % 2 == 0 ? 0x80 : 0x00;
                m1 |= (val2 << (b * 8));
            }
            grad.masks[r][c][0] = m0;
            grad.masks[r][c][1] = m1;
        }
    }
    cases.push_back(grad);

    for (auto& tc : cases) {
        for (int r = 0; r < HEIGHT; ++r) {
            uint32_t packed_alpha = 0;
            for (int c = 0; c < WIDTH; ++c) {
                int active_count = 0;
                uint32_t m0 = tc.masks[r][c][0];
                uint32_t m1 = tc.masks[r][c][1];

                for (int b = 0; b < 4; ++b) {
                    if (((m0 >> (b * 8)) & 0xFF) != 0x80) active_count++;
                    if (((m1 >> (b * 8)) & 0xFF) != 0x80) active_count++;
                }

                float pixel_alpha = active_count * 0.125f;
                uint32_t alpha_byte = static_cast<uint32_t>(std::round(pixel_alpha * 255.0f));
                packed_alpha |= (alpha_byte << (c * 8));
            }
            tc.expected_alphas[r] = packed_alpha;
        }
    }

    return cases;
}

}  // namespace

template <uint16_t WIDTH, uint16_t HEIGHT> class GPUBitCountingTestRunner {
public:
    static void runAll(skiatest::Reporter* reporter,
                       Context* context,
                       GraphiteTestContext* testContext) {
        static_assert(WIDTH == 4);
        constexpr uint32_t kWorkgroupSize = 256;

        auto testCases = MakeBitCountingTestCases<WIDTH, HEIGHT>();
        uint32_t numTests = testCases.size();
        if (numTests == 0) return;

        class BitCountingComputeStep : public ComputeStep {
        public:
            BitCountingComputeStep(const std::vector<BitCountingTestCase<WIDTH, HEIGHT>>& testData)
                    : ComputeStep(
                              /*name=*/"SparseStripsBitCountingTest",
                              /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
                              /*resources=*/
                              {{ResourceType::kUniformBuffer,
                                DataFlow::kShared,
                                ResourcePolicy::kMapped,
                                0,
                                "testInfo { num_tests: i32, pad0: i32, pad1: i32, pad2: i32 }"},
                               {ResourceType::kReadOnlyStorageBuffer,
                                DataFlow::kShared,
                                ResourcePolicy::kMapped,
                                1,
                                "inputBlock { in_data: array<u32> }"},
                               {ResourceType::kStorageBuffer,
                                DataFlow::kShared,
                                ResourcePolicy::kMapped,
                                2,
                                "outputBlock { out_data: array<u32> }"}},
                              /*workgroupBuffers=*/{},
                              /*baseFlags=*/Flags::kSupportsNativeShader)
                    , fTestData(testData) {
                fShaderText = GetWgslConstants();

                fShaderText += SkStringPrintf(R"(
                            const TILE_WIDTH: u32             = %uu;
                            const TILE_HEIGHT: u32            = %uu;
                            const MSAA_BIAS_SPLAT: vec2<u32>  = vec2<u32>(0x%02X%02X%02X%02Xu);
                            const MSAA_INV_SAMPLES: f32       = %ff;
                        )",
                        WIDTH,
                        HEIGHT,
                        SparseStripsConstants::kMSAABias,
                        SparseStripsConstants::kMSAABias,
                        SparseStripsConstants::kMSAABias,
                        SparseStripsConstants::kMSAABias,
                        SparseStripsConstants::kMSAAInvSamples)
                .c_str();

                fShaderText += R"(
                    struct TestInfo {
                        num_tests: u32,
                        pad0: u32,
                        pad1: u32,
                        pad2: u32,
                    };

                    @group(0) @binding(0) var<uniform> test_info: TestInfo;
                    @group(0) @binding(1) var<storage, read> in_data: array<u32>;
                    @group(0) @binding(2) var<storage, read_write> out_data: array<u32>;
                )";

                fShaderText +=
                        "@compute @workgroup_size(" + std::to_string(kWorkgroupSize) + ", 1, 1)\n";
                fShaderText += R"(
                    fn main(@builtin(global_invocation_id) global_id: vec3<u32>) {
                        let tid = global_id.x;
                        if (tid >= test_info.num_tests) { return; }

                        var tile_masks: array<array<vec2<u32>, TILE_WIDTH>, TILE_HEIGHT>;
                        var final_alphas: array<u32, TILE_HEIGHT>;

                        for (var r = 0u; r < TILE_HEIGHT; r += 1u) {
                            for (var c = 0u; c < TILE_WIDTH; c += 1u) {
                                let idx = tid * (TILE_HEIGHT * TILE_WIDTH * 2u) + r * (TILE_WIDTH * 2u) + c * 2u;
                                tile_masks[r][c] = vec2<u32>(in_data[idx], in_data[idx + 1u]);
                            }
                        }
                )";

                fShaderText += kWgslBitCounting;

                fShaderText += R"(
                        for (var r = 0u; r < TILE_HEIGHT; r += 1u) {
                            out_data[tid * TILE_HEIGHT + r] = final_alphas[r];
                        }
                    }
                )";
            }

            NativeShaderSource nativeShaderSource(NativeShaderFormat format) const override {
                SkASSERT(format == NativeShaderFormat::kWGSL);
                return {fShaderText, "main"};
            }

            size_t calculateBufferSize(int index, const ResourceDesc& r) const override {
                if (index == 1) return sizeof(uint32_t) * HEIGHT * WIDTH * 2 * fTestData.size();
                if (index == 2) return sizeof(uint32_t) * HEIGHT * fTestData.size();
                return 0;
            }

            WorkgroupSize calculateGlobalDispatchSize() const override {
                uint32_t groups = (fTestData.size() + kWorkgroupSize - 1) / kWorkgroupSize;
                return WorkgroupSize(groups, 1, 1);
            }

            void prepareStorageBuffer(int resourceIndex,
                                      const ResourceDesc&,
                                      skgpu::BufferWriter&& writer) const override {
                if (resourceIndex == 1) {
                    for (const auto& tc : fTestData) {
                        writer.write(tc.masks, sizeof(tc.masks));
                    }
                }
            }

            void prepareUniformBuffer(int resourceIndex,
                                      const ResourceDesc&,
                                      UniformManager* manager) const override {
                if (resourceIndex == 0) {
#if defined(SK_DEBUG)
                    static const Uniform kUniforms[] = {{"num_tests", SkSLType::kInt},
                                                        {"pad0", SkSLType::kInt},
                                                        {"pad1", SkSLType::kInt},
                                                        {"pad2", SkSLType::kInt}};
                    manager->setExpectedUniforms(kUniforms, false);
#endif
                    manager->write(static_cast<int32_t>(fTestData.size()));
                    manager->write(0);
                    manager->write(0);
                    manager->write(0);
                }
            }

        private:
            const std::vector<BitCountingTestCase<WIDTH, HEIGHT>>& fTestData;
            std::string fShaderText;
        };

        BitCountingComputeStep step(testCases);
        std::unique_ptr<Recorder> recorder = context->makeRecorder();
        DispatchGroup::Builder builder(recorder.get());
        if (!builder.appendStep(&step)) {
            ERRORF(reporter, "Failed to add ComputeStep to DispatchGroup");
            return;
        }

        BindBufferInfo outputInfo = builder.getSharedBufferResource(2);
        if (!outputInfo) {
            ERRORF(reporter, "Failed to allocate shared compute output buffer");
            return;
        }

        ComputeTask::DispatchGroupList groups;
        groups.push_back(builder.finalize());
        recorder->priv().add(ComputeTask::Make(std::move(groups)));

        auto outputBuffer = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);
        submit_recording(context, testContext, recorder.get());

        uint32_t* outData = static_cast<uint32_t*>(
                map_buffer(context, testContext, outputBuffer.get(), outputInfo.fOffset));
        SkASSERT(outputBuffer->isMapped() && outData != nullptr);

        for (size_t i = 0; i < numTests; ++i) {
            for (int r = 0; r < HEIGHT; ++r) {
                uint32_t expected = testCases[i].expected_alphas[r];
                uint32_t actual = outData[i * HEIGHT + r];
                REPORTER_ASSERT(
                        reporter,
                        expected == actual,
                        "BitCounting failed at test index %zu, row %d. Expected 0x%08X, got 0x%08X",
                        i,
                        r,
                        expected,
                        actual);
            }
        }

        if (outputBuffer->isMapped()) {
            outputBuffer->unmap();
        }
    }
};

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(SparseStrips_Compute_BitCounting,
                                   reporter,
                                   context,
                                   testContext) {
    if (testContext->contextType() == skgpu::ContextType::kDawn_D3D11) {
        return;
    }
    GPUBitCountingTestRunner<4, 4>::runAll(reporter, context, testContext);
}
