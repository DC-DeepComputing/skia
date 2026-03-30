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

#include "tests/graphite/sparse_strips/ClipTestUtils.h"
#include "tests/graphite/sparse_strips/compute/TestUtilsCompute.h"
#include "tools/graphite/GraphiteTestContext.h"

using namespace skgpu::graphite;
using namespace skiatest::graphite;

namespace {

struct TestCaseGPU {
    float p0[2];
    float p1[2];
    float bounds[4];
    float derivatives[4];
    uint32_t intersectionData;
    uint32_t canonicalXDir;
    uint32_t canonicalYDir;
    uint32_t pad;
};

template <uint16_t WIDTH, uint16_t HEIGHT>
struct TestMetadata {
    std::string name;
    Line line;
    Tile<WIDTH, HEIGHT> tile;
};

}  // namespace

template <uint16_t WIDTH, uint16_t HEIGHT>
class GPUClipTestRunner {
public:
    static void runAll(skiatest::Reporter* reporter,
                       Context* context,
                       GraphiteTestContext* testContext) {

        using TileType = Tile<WIDTH, HEIGHT>;
        constexpr uint32_t kWorkgroupSize = 256;

        auto cases = MakeClipTestCases<WIDTH, HEIGHT>();

        std::vector<TestCaseGPU> gpuInputs;
        std::vector<TestMetadata<WIDTH, HEIGHT>> metadata;

        for (const auto& testCase : cases) {
            for (const auto& tile : testCase.expectedTiles) {
                uint32_t lineIdx =
                        tile.packedWindingLineIdx >> SparseStripsConstants::INT_MASK_SHIFT;
                if (lineIdx >= testCase.lines.size()) continue;

                const Line& line = testCase.lines[lineIdx];

                float dx = line.p1.fX - line.p0.fX;
                float dy = line.p1.fY - line.p0.fY;
                bool isVertical = std::abs(dx) <= std::numeric_limits<float>::epsilon();
                bool isHorizontal = std::abs(dy) <= std::numeric_limits<float>::epsilon();
                float idx = isVertical ? 0.0f : 1.0f / dx;
                float idy = isHorizontal ? 0.0f : 1.0f / dy;
                float tileMinX = tile.x * static_cast<float>(WIDTH);
                float tileMinY = tile.y * static_cast<float>(HEIGHT);
                float tileMaxX = tileMinX + static_cast<float>(WIDTH);
                float tileMaxY = tileMinY + static_cast<float>(HEIGHT);

                TestCaseGPU gpuCase;
                gpuCase.p0[0] = line.p0.fX; gpuCase.p0[1] = line.p0.fY;
                gpuCase.p1[0] = line.p1.fX; gpuCase.p1[1] = line.p1.fY;
                gpuCase.bounds[0] = tileMinX; gpuCase.bounds[1] = tileMinY;
                gpuCase.bounds[2] = tileMaxX; gpuCase.bounds[3] = tileMaxY;
                gpuCase.derivatives[0] = dx; gpuCase.derivatives[1] = dy;
                gpuCase.derivatives[2] = idx; gpuCase.derivatives[3] = idy;
                gpuCase.intersectionData =
                        tile.packedWindingLineIdx & SparseStripsConstants::INTERSECTION_MASK;
                gpuCase.canonicalXDir = (dx >= 0) ? 1 : 0;
                gpuCase.canonicalYDir = (dy >= 0) ? 1 : 0;
                gpuCase.pad = 0;

                gpuInputs.push_back(gpuCase);
                metadata.push_back({testCase.name, line, tile});
            }
        }

        uint32_t numTests = gpuInputs.size();
        if (numTests == 0) return;

        class ClipComputeStep : public ComputeStep {
        public:
            ClipComputeStep(const std::vector<TestCaseGPU>& testData)
                : ComputeStep(
                    /*name=*/"SparseStripsClipTest",
                    /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
                    /*resources=*/{
                        {ResourceType::kUniformBuffer, DataFlow::kShared, ResourcePolicy::kMapped, 0,
                         "testInfo { num_tests: u32, pad0: u32, pad1: u32, pad2: u32 }"},
                        {ResourceType::kReadOnlyStorageBuffer, DataFlow::kShared, ResourcePolicy::kMapped, 1,
                         "inputBlock { in_data: array<vec4<u32>> }"},
                        {ResourceType::kStorageBuffer, DataFlow::kShared, ResourcePolicy::kMapped, 2,
                         "outputBlock { out_data: array<vec4<f32>> }"}
                    },
                    /*workgroupBuffers=*/{},
                    /*baseFlags=*/Flags::kSupportsNativeShader),
                  fTestData(testData) {

                // Dynamically inject our modular strings
                fShaderText = GetWgslConstants();
                fShaderText += "const TILE_WIDTH: u32 = " + std::to_string(WIDTH) + "u;\n";
                fShaderText += "const TILE_HEIGHT: u32 = " + std::to_string(HEIGHT) + "u;\n";
                fShaderText += kWgslHelpers;

                fShaderText += R"(
                    struct PreMergeTile {
                        packed_xy: u32,
                        packed_winding_line_idx: u32,
                        alpha_index: u32,
                        packed_scanned_winding: u32,
                    };

                    struct TestInfo {
                        num_tests: u32,
                        pad0: u32,
                        pad1: u32,
                        pad2: u32,
                    };

                    @group(0) @binding(0) var<uniform> test_info: TestInfo;
                    @group(0) @binding(1) var<storage, read> in_data: array<vec4<u32>>;
                    @group(0) @binding(2) var<storage, read_write> out_data: array<vec4<f32>>;
                )";

                fShaderText += "@compute @workgroup_size(" + std::to_string(kWorkgroupSize) + ", 1, 1)\n";
                fShaderText += R"(
                    fn main(@builtin(global_invocation_id) global_id: vec3<u32>) {
                        let tid = global_id.x;
                        if (tid >= test_info.num_tests) { return; }

                        let idx_base = tid * 4u;
                        let d0 = in_data[idx_base + 0u];
                        let d1 = in_data[idx_base + 1u];
                        let d2 = in_data[idx_base + 2u];
                        let d3 = in_data[idx_base + 3u];

                        let p0 = vec2<f32>(bitcast<f32>(d0.x), bitcast<f32>(d0.y));
                        let p1 = vec2<f32>(bitcast<f32>(d0.z), bitcast<f32>(d0.w));

                        let tile_min_x = bitcast<f32>(d1.x);
                        let tile_min_y = bitcast<f32>(d1.y);
                        let tile_x_u32 = u32(tile_min_x) / TILE_WIDTH;
                        let tile_y_u32 = u32(tile_min_y) / TILE_HEIGHT;

                        var pmt: PreMergeTile;
                        // Use constants imported from GetWgslConstants()
                        pmt.packed_xy = (tile_y_u32 << Y_SHIFT) | (tile_x_u32 & XY_MASK);
                        pmt.packed_winding_line_idx = d3.x;

                        let dx   = bitcast<f32>(d2.x);
                        let dy   = bitcast<f32>(d2.y);
                        let idx  = bitcast<f32>(d2.z);
                        let idy  = bitcast<f32>(d2.w);

                        let dxdy = dx * idy;
                        let dydx = dy * idx;

                        let canonical_x_dir = d3.y != 0u;
                        let canonical_y_dir = d3.z != 0u;

                        let right_in_viewport = true;
                )";

                // Append the real clip snippet!
                fShaderText += kWgslClip;

                fShaderText += R"(
                        out_data[tid] = vec4<f32>(clipped_top.x, clipped_top.y, clipped_bot.x, clipped_bot.y);
                    }
                )";
            }

            NativeShaderSource nativeShaderSource(NativeShaderFormat format) const override {
                SkASSERT(format == NativeShaderFormat::kWGSL);
                return {fShaderText, "main"};
            }

            size_t calculateBufferSize(int index, const ResourceDesc& r) const override {
                if (index == 1) return sizeof(TestCaseGPU) * fTestData.size();
                if (index == 2) return sizeof(float) * 4 * fTestData.size();
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
                    writer.write(fTestData.data(), fTestData.size() * sizeof(TestCaseGPU));
                }
            }

            void prepareUniformBuffer(int resourceIndex,
                                      const ResourceDesc&,
                                      UniformManager* manager) const override {
                if (resourceIndex == 0) {
#if defined(SK_DEBUG)
                    static const Uniform kUniforms[] = {
                        {"num_tests", SkSLType::kUInt},
                        {"pad0",      SkSLType::kUInt},
                        {"pad1",      SkSLType::kUInt},
                        {"pad2",      SkSLType::kUInt}
                    };
                    manager->setExpectedUniforms(kUniforms, false);
#endif
                    manager->write(static_cast<uint32_t>(fTestData.size()));
                    manager->write(0u);
                    manager->write(0u);
                    manager->write(0u);
                }
            }

        private:
            const std::vector<TestCaseGPU>& fTestData;
            std::string fShaderText;
        };

        ClipComputeStep step(gpuInputs);
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

        // Dispatch
        ComputeTask::DispatchGroupList groups;
        groups.push_back(builder.finalize());
        recorder->priv().add(ComputeTask::Make(std::move(groups)));

        // Readback
        auto outputBuffer = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);
        submit_recording(context, testContext, recorder.get());

        float* outData = static_cast<float*>(
                map_buffer(context, testContext, outputBuffer.get(), outputInfo.fOffset));
        SkASSERT(outputBuffer->isMapped() && outData != nullptr);

        for (size_t i = 0; i < numTests; ++i) {
            std::array<SkPoint, 2> result;
            result[0] = {outData[i * 4 + 0], outData[i * 4 + 1]};
            result[1] = {outData[i * 4 + 2], outData[i * 4 + 3]};

            VerifyClipResult<WIDTH, HEIGHT>(
                    reporter, metadata[i].name, metadata[i].line, metadata[i].tile, result);
        }

        if (outputBuffer->isMapped()) {
            outputBuffer->unmap();
        }
    }
};

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(SparseStrips_Compute_Clip4x4,
                                   reporter,
                                   context,
                                   testContext) {
    if (testContext->contextType() == skgpu::ContextType::kDawn_D3D11) { return; }
    GPUClipTestRunner<4, 4>::runAll(reporter, context, testContext);
}

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(SparseStrips_Compute_Clip8x8,
                                   reporter,
                                   context,
                                   testContext) {
    if (testContext->contextType() == skgpu::ContextType::kDawn_D3D11) { return; }
    GPUClipTestRunner<8, 8>::runAll(reporter, context, testContext);
}
