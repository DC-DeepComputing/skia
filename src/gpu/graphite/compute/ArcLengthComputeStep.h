/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_compute_ArcLengthComputeStep_DEFINED
#define skgpu_graphite_compute_ArcLengthComputeStep_DEFINED

#include "src/gpu/graphite/compute/ComputeStep.h"

namespace skgpu::graphite {

class ArcLengthComputeStep final : public ComputeStep {
public:
    ArcLengthComputeStep(bool infinitySupport);
    ~ArcLengthComputeStep() override = default;

    std::string computeSkSL() const override;

    void prepareUniformBuffer(int index, const ResourceDesc&, UniformManager* mgr) const override;
    size_t calculateBufferSize(int resourceIndex, const ResourceDesc&) const override;
    WorkgroupSize calculateGlobalDispatchSize() const override;

    void updateProblemSize(uint32_t numPatches) {
        fProblemSize += numPatches;
        fWorkTiles = (fProblemSize + 256 - 1) / 256;
    }
    void incrementPathCount() { fNumPaths += 1; }

    uint32_t problemSize() const { return fProblemSize; }
    uint32_t workTiles() const { return fWorkTiles; }
    uint32_t numPaths() const { return fNumPaths; }

private:
    bool fInfinitySupport;
    uint32_t fProblemSize = 0;
    uint32_t fWorkTiles = 0;
    uint32_t fNumPaths = 0;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_compute_ArcLengthComputeStep_DEFINED
