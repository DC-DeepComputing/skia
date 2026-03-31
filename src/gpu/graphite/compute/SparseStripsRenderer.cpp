/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/compute/SparseStripsRenderer.h"
#include "src/gpu/graphite/sparse_strips/shaders/NonSubgroupWGSL.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"

#include "include/core/SkColorType.h"
#include "include/core/SkSize.h"

#include <string>

namespace skgpu::graphite {

namespace {

static constexpr ComputeStep::ResourceDesc kResources[] = {
    {ComputeStep::ResourceType::kUniformBuffer, ComputeStep::DataFlow::kShared,
     ComputeStep::ResourcePolicy::kMapped, SparseStripsSlots::kConfig},
    {ComputeStep::ResourceType::kReadOnlyStorageBuffer, ComputeStep::DataFlow::kShared,
     ComputeStep::ResourcePolicy::kNone, SparseStripsSlots::kMsaaLut},
    {ComputeStep::ResourceType::kReadOnlyStorageBuffer, ComputeStep::DataFlow::kShared,
     ComputeStep::ResourcePolicy::kNone, SparseStripsSlots::kPmtIn},
    {ComputeStep::ResourceType::kStorageBuffer, ComputeStep::DataFlow::kShared,
     ComputeStep::ResourcePolicy::kNone, SparseStripsSlots::kPartIndicator},
    {ComputeStep::ResourceType::kStorageBuffer, ComputeStep::DataFlow::kShared,
     ComputeStep::ResourcePolicy::kNone, SparseStripsSlots::kPartRed},
    {ComputeStep::ResourceType::kWriteOnlyStorageTexture, ComputeStep::DataFlow::kShared,
     ComputeStep::ResourcePolicy::kNone, SparseStripsSlots::kOutput},
    {ComputeStep::ResourceType::kStorageBuffer, ComputeStep::DataFlow::kShared,
     ComputeStep::ResourcePolicy::kNone, SparseStripsSlots::kDebug},
};

std::string BuildMsaaKernel() {
    std::string shader;

    // 1. Structural and Constant Setup
    shader += kWgslStructs;
    shader += GetWgslConstants();
    shader += GetWgslConfigDerived();
    shader += kWgslHelpers;
    shader += kWgslBindings;
    shader += kWgslWorkgroupVars;

    // 2. Core Execution Blocks
    shader += kWgslMain;
    shader += kWgslClip;
    shader += kWgslRowSetup;
    shader += kWgslLeftEdgeFill;
    shader += kWgslMaskGen_SlopeIntercept;
    shader += kWgslScan_Base;
    shader += kWgslScan_SegmentedReduction;

    // 3. Output, Validation, and Bit Counting Wrap-up
    shader += R"(
    if (is_last_tile) {
        if (row_index == 0u) { part_indicator[wgid.x] = u32(is_valid); }
        for (var col = 0u; col < TILE_WIDTH; col += 1u) {
            part_red[wgid.x][row_index][col] = mask[col];
        }
    }

    if (is_end) {
        let alpha_shifted = pmt.alpha_index >> 4u;
        if (row_index == 0u && !is_valid) {
            part_indicator[wgid.x + config.workgroups] = select(alpha_shifted, 0u, is_valid);
        }
        if (!is_valid) {
            for (var col = 0u; col < TILE_WIDTH; col += 1u) {
                part_red[wgid.x + config.workgroups][row_index][col] = mask[col];
            }
        }
        if (is_valid && row_index == 0u) {
            if (right_in_viewport) {
)";
    shader += kWgslBitCounting;

    shader += R"(
            }
        }
    }
}
)";
    return shader;
}

// --------------------- STITCH WRAPPER ---------------------
std::string BuildStitchKernel() {
    std::string shader;

    shader += kWgslStructs;
    shader += GetWgslConstants();
    shader += GetWgslConfigDerived();
    shader += kWgslHelpers;
    shader += kWgslBindings;

    shader += R"(
@compute @workgroup_size(BLOCK_DIM, 1, 1)
fn msaa_stitch(@builtin(local_invocation_id) tid: vec3<u32>,
               @builtin(global_invocation_id) gid: vec3<u32>,
               @builtin(workgroup_id) wgid: vec3<u32>)  {
    if (gid.x >= config.workgroups) {
        return;
    }

    let alpha_index = part_indicator[gid.x + config.workgroups];
    if (alpha_index != 0u) {
        var lookback_id = gid.x - 1u;
        var mask = part_red[gid.x + config.workgroups];
        while (true) {
            var prev = part_red[lookback_id];
            for (var row = 0u; row < TILE_HEIGHT; row += 1u) {
                for (var col = 0u; col < TILE_WIDTH; col += 1u) {
                    let unbiased = prev[row][col] - vec2u(0x80808080u);
                    mask[row][col] += unbiased;
                }
            }

            if (part_indicator[lookback_id] != 0u || lookback_id == 0u) {
                break;
            } else {
                lookback_id -= 1u;
            }
        }

        let tile_masks = mask;
        var final_alphas = vec4u(0u);
)";
    shader += kWgslBitCounting;
    shader += R"(
    }
}
)";
    return shader;
}

} // anonymous namespace

// ----------------------------------------------------------------------------
// SparseStripsMsaaStep Implementation
// ----------------------------------------------------------------------------
SparseStripsMsaaStep::SparseStripsMsaaStep()
        : ComputeStep(
                  /*name=*/"SparseStripsMsaa",
                  /*localDispatchSize=*/{SparseStripsConfig::kWorkgroupSize, 1, 1},
                  /*resources=*/kResources,
                  /*workgroupBuffers=*/{},
                  /*baseFlags=*/Flags::kSupportsNativeShader) {
    fShaderText = BuildMsaaKernel();
}

ComputeStep::NativeShaderSource SparseStripsMsaaStep::nativeShaderSource(
        NativeShaderFormat format) const {
    if (format == NativeShaderFormat::kWGSL) {
        return {fShaderText, "msaa"};
    }
    SK_ABORT("Only native WGSL is supported.");
    return {"", ""};
}

std::tuple<SkISize, SkColorType> SparseStripsMsaaStep::calculateTextureParameters(
        int resourceIndex, const ResourceDesc& desc) const {
    if (desc.fSlot == SparseStripsSlots::kOutput) {
        return {{1, 1}, kRGBA_8888_SkColorType}; //TODO (thomsmit): allow also alpha 8
    }
    SK_ABORT("Unexpected texture request in SparseStripsMsaaStep");
    return {};
}

// ----------------------------------------------------------------------------
// SparseStripsStitchStep Implementation
// ----------------------------------------------------------------------------
SparseStripsStitchStep::SparseStripsStitchStep()
        : ComputeStep(
                  /*name=*/"SparseStripsStitch",
                  /*localDispatchSize=*/{SparseStripsConfig::kWorkgroupSize, 1, 1},
                  /*resources=*/kResources,
                  /*workgroupBuffers=*/{},
                  /*baseFlags=*/Flags::kSupportsNativeShader) {
    fShaderText = BuildStitchKernel();
}

ComputeStep::NativeShaderSource SparseStripsStitchStep::nativeShaderSource(
        NativeShaderFormat format) const {
    if (format == NativeShaderFormat::kWGSL) {
        return {fShaderText, "msaa_stitch"};
    }
    SK_ABORT("Only native WGSL is supported.");
    return {"", ""};
}

}  // namespace skgpu::graphite
