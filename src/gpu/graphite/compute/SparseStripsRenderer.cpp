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
     ComputeStep::ResourcePolicy::kNone, SparseStripsSlots::kOutput0},
    {ComputeStep::ResourceType::kWriteOnlyStorageTexture, ComputeStep::DataFlow::kShared,
     ComputeStep::ResourcePolicy::kNone, SparseStripsSlots::kOutput1},
    {ComputeStep::ResourceType::kStorageBuffer, ComputeStep::DataFlow::kShared,
     ComputeStep::ResourcePolicy::kNone, SparseStripsSlots::kDebug},
    {ComputeStep::ResourceType::kReadOnlyStorageBuffer, ComputeStep::DataFlow::kShared,
     ComputeStep::ResourcePolicy::kNone, SparseStripsSlots::kLines},
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
        if (!is_valid) {
            if (row_index == 0u) {
                part_indicator[wgid.x + config.workgroups] = select(alpha_shifted, 0, is_valid);
            }
            for (var col = 0u; col < TILE_WIDTH; col += 1u) {
                part_red[wgid.x + config.workgroups][row_index][col] = mask[col];
            }
        }
        if (is_valid) {
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
@compute @workgroup_size(STITCH_BLOCK_DIM, 1, 1)
fn msaa_stitch(@builtin(local_invocation_id) tid: vec3<u32>,
               @builtin(global_invocation_id) gid: vec3<u32>,
               @builtin(workgroup_id) wgid: vec3<u32>)  {
    if (gid.x == 0 || gid.x >= config.workgroups) {
        return;
    }

    // This is actually alpha_shifted (pmt.alpha_index >> 4u) from the MSAA step
    let alpha_shifted = part_indicator[gid.x + config.workgroups];
    if (alpha_shifted != 0u) {
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

        // Reconstruct base_texel_idx. Since alpha_shifted is >> 4u and the
        // bit counter expects >> 2u, we shift it back left by 2u.
        let base_texel_idx = alpha_shifted << 2u;

        // Process all rows for this tile
        for (var row_index = 0u; row_index < TILE_HEIGHT; row_index += 1u) {
            var row_pixels: vec4<f32>;
            for (var col = 0u; col < TILE_WIDTH; col += 1u) {
                let cov_mask = mask[row_index][col];

                let diff = cov_mask ^ MSAA_BIAS_SPLAT;
                let subbed = diff - 0x01010101u;
                let zeros = subbed & (~diff) & MSAA_BIAS_SPLAT;
                let actives = zeros ^ MSAA_BIAS_SPLAT;
                let counts = ((actives >> vec2u(7)) * 0x01010101u) >> vec2u(24);
                row_pixels[col] = f32(counts.x + counts.y) * MSAA_INV_SAMPLES;
            }

            let texel_idx = base_texel_idx + row_index;
            let page_local_idx = texel_idx & 0xffffffu;
            let output_coords = vec2<u32>(
                page_local_idx & 4095,
                page_local_idx >> 12
            );

            let tex_page = min(texel_idx >> 24, 1);
            if (tex_page == 0) {
                textureStore(output0, output_coords, row_pixels);
            } else {
                textureStore(output1, output_coords, row_pixels);
            }
        }
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
                  /*localDispatchSize=*/{SparseStripsConfig::kMSAAWorkgroupSize, 1, 1},
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
    if (desc.fSlot == SparseStripsSlots::kOutput0) {
        return {{SparseStripsConfig::kTextureDims, SparseStripsConfig::kTextureDims},
                SparseStripsConfig::kColorType};
    } else if (desc.fSlot == SparseStripsSlots::kOutput1) {
        return {{SparseStripsConfig::kTextureDims, SparseStripsConfig::kTextureDims},
                SparseStripsConfig::kColorType};
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
                  /*localDispatchSize=*/{SparseStripsConfig::kStitchWorkgroupSize, 1, 1},
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

std::tuple<SkISize, SkColorType> SparseStripsStitchStep::calculateTextureParameters(
        int resourceIndex, const ResourceDesc& desc) const {
    if (desc.fSlot == SparseStripsSlots::kOutput0) {
        return {{SparseStripsConfig::kTextureDims, SparseStripsConfig::kTextureDims},
                SparseStripsConfig::kColorType};
    } else if (desc.fSlot == SparseStripsSlots::kOutput1) {
        return {{SparseStripsConfig::kTextureDims, SparseStripsConfig::kTextureDims},
                SparseStripsConfig::kColorType};
    }
    SK_ABORT("Unexpected texture request in SparseStripsMsaaStep");
    return {};
}

}  // namespace skgpu::graphite
