/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/compute/ArcLengthComputeStep.h"

#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/compute/PrefixScanComposer.h"
#include "src/gpu/graphite/compute/shaders/ArcLengthSegPrefixScan.h"

namespace skgpu::graphite {

ArcLengthComputeStep::ArcLengthComputeStep(bool infinitySupport)
        : ComputeStep("ArcLengthCompute",
                      /*localDispatchSize=*/{256, 1, 1},
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
        , fInfinitySupport(infinitySupport) {}

std::string ArcLengthComputeStep::computeSkSL() const {
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

void ArcLengthComputeStep::prepareUniformBuffer(int index, const ResourceDesc&, UniformManager* mgr) const {
    SkASSERT(index == 0);
    SkDEBUGCODE(mgr->setExpectedUniforms(SkSpan<const Uniform>({{"size", SkSLType::kUInt},
                                                                {"work_tiles", SkSLType::kUInt},
                                                                {"num_paths", SkSLType::kUInt},
                                                                {"unused_1", SkSLType::kUInt}}),
                                         /*isSubstruct=*/false);)
    mgr->write(fProblemSize);
    mgr->write(fWorkTiles);
    mgr->write(fNumPaths);
    mgr->write(0u);
}

size_t ArcLengthComputeStep::calculateBufferSize(int resourceIndex, const ResourceDesc&) const {
    SkASSERT(resourceIndex == 1);
    return sizeof(uint32_t) * (fWorkTiles + 1);
}

WorkgroupSize ArcLengthComputeStep::calculateGlobalDispatchSize() const {
    return WorkgroupSize(fWorkTiles, 1, 1);
}

}  // namespace skgpu::graphite
