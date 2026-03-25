/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/compute/SparseStripsRenderer.h"
#include "src/gpu/graphite/compute/SparseStripsSharedWGSL.h"

#include "include/core/SkColorType.h"
#include "include/core/SkSize.h"

#include <cmath>
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

std::string BuildMsaaKernel(uint32_t blockDim) {
    uint32_t lgBlockDim = static_cast<uint32_t>(std::log2(blockDim));
    std::string shader = kWgslHeader;

    // Inject dynamic constants
    shader += "const BLOCK_DIM: u32 = " + std::to_string(blockDim) + "u;\n";
    shader += "const LG_BLOCK_DIM: u32 = " + std::to_string(lgBlockDim) + "u;\n";
    shader += "const SCAN_CONST: u32 = LG_BLOCK_DIM - 1u;\n";
    shader += "const TILE_SIZE: u32 = TILE_HEIGHT * TILE_WIDTH;\n";
    shader += "const PART_SIZE: u32 = BLOCK_DIM / 4u;\n";

    shader += "@compute @workgroup_size(" + std::to_string(blockDim) + ", 1, 1)\n";
    shader += R"(
fn msaa(@builtin(local_invocation_id) tid: vec3<u32>,
        @builtin(global_invocation_id) gid: vec3<u32>,
        @builtin(workgroup_id) wgid: vec3<u32>)  {
    var pmt: PreMergeTile;
    if ((gid.x >> 2) < config.pmt_count) {
        pmt = pmt_in[gid.x >> 2];
    } else {
        pmt = PreMergeTile(0xffffffffu, 0u, 0u, 0u, vec4f(0.0f));
    }

    let p0 = pmt.line.xy;
    let p1 = pmt.line.zw;

    let d = p1 - p0;
    let is_vertical = abs(d.x) <= EPSILON;
    let is_horizontal = abs(d.y) <= EPSILON;
    let idx = select(1.0f / d.x, 0.0f, is_vertical);
    let idy = select(1.0f / d.y, 0.0f, is_horizontal);
    let dxdy = d.x * idy;
    let dydx = d.y * idx;

    let canonical_x_dir = p1.x >= p0.x;
    let canonical_y_dir = p1.y >= p0.y;

    let right = select(p0.x, p1.x, canonical_x_dir);
    let right_in_viewport = right >= 0.0f;
)";

    shader += kWgslClip;
    shader += kWgslMaskGen;
    shader += kWgslScan;

    // Output block wrapping the bit counting string
    shader += R"(
    if (is_last_tile) {
        if (row_index == 0) { part_indicator[wgid.x] = u32(is_valid); }
        for (var col = 0u; col < TILE_WIDTH; col += 1u) {
            part_red[wgid.x][row_index][col] = mask[col];
        }
    }

    if (is_end) {
        let alpha_shifted = pmt.alpha_index >> 4;
        if (row_index == 0 && !is_valid) {
            part_indicator[wgid.x + config.workgroups] = select(alpha_shifted, 0, is_valid);
        }
        if (!is_valid) {
            for (var col = 0u; col < TILE_WIDTH; col += 1u) {
                part_red[wgid.x + config.workgroups][row_index][col] = mask[col];
            }
        }
        if (is_valid && row_index == 0) {
)";

    shader += kWgslBitCounting;

    shader += R"(
        }
    }
}
)";
    return shader;
}

// --------------------- STITCH WRAPPER ---------------------
std::string BuildStitchKernel(uint32_t blockDim) {
    uint32_t lgBlockDim = static_cast<uint32_t>(std::log2(blockDim));
    std::string shader = kWgslHeader;

    shader += "const BLOCK_DIM: u32 = " + std::to_string(blockDim) + "u;\n";
    shader += "const LG_BLOCK_DIM: u32 = " + std::to_string(lgBlockDim) + "u;\n";
    shader += "const SCAN_CONST: u32 = LG_BLOCK_DIM - 1u;\n";
    shader += "const TILE_SIZE: u32 = TILE_HEIGHT * TILE_WIDTH;\n";
    shader += "const PART_SIZE: u32 = BLOCK_DIM / 4u;\n";

    shader += "@compute @workgroup_size(" + std::to_string(blockDim) + ", 1, 1)\n";
    shader += R"(
fn msaa_stitch(@builtin(local_invocation_id) tid: vec3<u32>,
               @builtin(global_invocation_id) gid: vec3<u32>,
               @builtin(workgroup_id) wgid: vec3<u32>)  {
    if (gid.x >= config.workgroups) {
        return;
    }

    let alpha_index = part_indicator[gid.x + config.workgroups];
    if (alpha_index != 0u) {
        var lookback_id = gid.x - 1;
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

        var final_alphas = vec4u(0);
        for (var col = 0u; col < TILE_WIDTH; col += 1u) {
            var col_pixels: vec4<f32>;
            for (var y = 0u; y < TILE_HEIGHT; y += 1u) {
                let diff = mask[y][col] ^ vec2u(0x80808080u);
                let subbed = diff - 0x01010101u;
                let zeros = subbed & (~diff) & vec2u(0x80808080u);
                let actives = zeros ^ vec2u(0x80808080u);
                let counts = ((actives >> vec2u(7)) * 0x01010101u) >> vec2u(24);
                col_pixels[y] = f32(counts.x + counts.y) * 0.125;
            }
            final_alphas[col] = pack4x8unorm(abs(col_pixels));
        }
        let tex_dims = textureDimensions(output);
        let tex_width = tex_dims.x;
        let output_coords = vec2<u32>(
            alpha_index % tex_width,
            alpha_index / tex_width
        );
        if (output_coords.y < tex_dims.y) {
            textureStore(output, output_coords, final_alphas);
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
template <uint16_t kTileWidth, uint16_t kTileHeight, uint32_t kWorkGroupSize>
SparseStripsMsaaStep<kTileWidth, kTileHeight, kWorkGroupSize>::SparseStripsMsaaStep()
        : ComputeStep(
                  /*name=*/"SparseStripsMsaa",
                  /*localDispatchSize=*/{kWorkGroupSize, 1, 1},
                  /*resources=*/kResources,
                  /*workgroupBuffers=*/{},
                  /*baseFlags=*/Flags::kSupportsNativeShader) {
    fShaderText = BuildMsaaKernel(kWorkGroupSize);
}

template <uint16_t kTileWidth, uint16_t kTileHeight, uint32_t kWorkGroupSize>
ComputeStep::NativeShaderSource SparseStripsMsaaStep<kTileWidth, kTileHeight, kWorkGroupSize>::nativeShaderSource(
        NativeShaderFormat format) const {
    if (format == NativeShaderFormat::kWGSL) {
        return {fShaderText, "msaa"};
    }
    SK_ABORT("Only native WGSL is supported.");
    return {"", ""};
}

template <uint16_t kTileWidth, uint16_t kTileHeight, uint32_t kWorkGroupSize>
std::tuple<SkISize, SkColorType> SparseStripsMsaaStep<kTileWidth, kTileHeight, kWorkGroupSize>::calculateTextureParameters(
        int resourceIndex, const ResourceDesc& desc) const {
    if (desc.fSlot == SparseStripsSlots::kOutput) {
        return {{1, 1}, kRGBA_8888_SkColorType};
    }
    SK_ABORT("Unexpected texture request in SparseStripsMsaaStep");
    return {};
}

// ----------------------------------------------------------------------------
// SparseStripsStitchStep Implementation
// ----------------------------------------------------------------------------
template <uint16_t kTileWidth, uint16_t kTileHeight, uint32_t kWorkGroupSize>
SparseStripsStitchStep<kTileWidth, kTileHeight, kWorkGroupSize>::SparseStripsStitchStep()
        : ComputeStep(
                  /*name=*/"SparseStripsStitch",
                  /*localDispatchSize=*/{kWorkGroupSize, 1, 1},
                  /*resources=*/kResources,
                  /*workgroupBuffers=*/{},
                  /*baseFlags=*/Flags::kSupportsNativeShader) {
    fShaderText = BuildStitchKernel(kWorkGroupSize);
}

template <uint16_t kTileWidth, uint16_t kTileHeight, uint32_t kWorkGroupSize>
ComputeStep::NativeShaderSource SparseStripsStitchStep<kTileWidth, kTileHeight, kWorkGroupSize>::nativeShaderSource(
        NativeShaderFormat format) const {
    if (format == NativeShaderFormat::kWGSL) {
        return {fShaderText, "msaa_stitch"};
    }
    SK_ABORT("Only native WGSL is supported.");
    return {"", ""};
}

// ----------------------------------------------------------------------------
// Explicit Template Instantiations
// ----------------------------------------------------------------------------
// The MSAA pass typically runs at 256 threads.
template class SparseStripsMsaaStep<4, 4, 256>;

// The Stitch pass typically runs at 128 threads.
template class SparseStripsStitchStep<4, 4, 128>;

}  // namespace skgpu::graphite
