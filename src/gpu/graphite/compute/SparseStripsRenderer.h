/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_compute_SparseStripsRender_DEFINED
#define skgpu_graphite_compute_SparseStripsRender_DEFINED

#include "src/gpu/graphite/compute/ComputeStep.h"
#include <string>

namespace skgpu::graphite {

namespace SparseStripsSlots {
    constexpr int kConfig        = 0;
    constexpr int kMsaaLut       = 1;
    constexpr int kPmtIn         = 2;
    constexpr int kPartIndicator = 3;
    constexpr int kPartRed       = 4;
    constexpr int kOutput0       = 5;
    constexpr int kOutput1       = 6;
    constexpr int kDebug         = 7;
    constexpr int kLines         = 8;
}

// ----------------------------------------------------------------------------
// SparseStripsMsaaStep
// ----------------------------------------------------------------------------
class SparseStripsMsaaStep final : public ComputeStep {
public:
    SparseStripsMsaaStep();
    ~SparseStripsMsaaStep() override = default;

    NativeShaderSource nativeShaderSource(NativeShaderFormat format) const override;

    std::tuple<SkISize, SkColorType> calculateTextureParameters(
            int resourceIndex, const ResourceDesc&) const override;

    WorkgroupSize calculateGlobalDispatchSize() const override {
        SK_ABORT("Global dispatch size must be specified in Builder::appendStep()");
    }

private:
    std::string fShaderText;
};

// ----------------------------------------------------------------------------
// SparseStripsStitchStep
// ----------------------------------------------------------------------------
class SparseStripsStitchStep final : public ComputeStep {
public:
    SparseStripsStitchStep();
    ~SparseStripsStitchStep() override = default;

    NativeShaderSource nativeShaderSource(NativeShaderFormat format) const override;

    std::tuple<SkISize, SkColorType> calculateTextureParameters(
            int resourceIndex, const ResourceDesc&) const override;

    WorkgroupSize calculateGlobalDispatchSize() const override {
        SK_ABORT("Global dispatch size must be specified in Builder::appendStep()");
    }

private:
    std::string fShaderText;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_compute_SparseStripsRender_DEFINED
