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
    constexpr int kOutput        = 5;
    constexpr int kDebug         = 6;
}

// ----------------------------------------------------------------------------
// SparseStripsMsaaStep
// ----------------------------------------------------------------------------
template <uint16_t kTileWidth, uint16_t kTileHeight, uint32_t kWorkGroupSize>
class SparseStripsMsaaStep final : public ComputeStep {
public:
    static_assert(kTileWidth == 4 && kTileHeight == 4, "Only 4x4 tiles supported currently");

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
template <uint16_t kTileWidth, uint16_t kTileHeight, uint32_t kWorkGroupSize>
class SparseStripsStitchStep final : public ComputeStep {
public:
    static_assert(kTileWidth == 4 && kTileHeight == 4, "Only 4x4 tiles supported currently");

    SparseStripsStitchStep();
    ~SparseStripsStitchStep() override = default;

    NativeShaderSource nativeShaderSource(NativeShaderFormat format) const override;

    WorkgroupSize calculateGlobalDispatchSize() const override {
        SK_ABORT("Global dispatch size must be specified in Builder::appendStep()");
    }

private:
    std::string fShaderText;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_compute_SparseStripsRender_DEFINED
