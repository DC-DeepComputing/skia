/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_SparseStripsConstants_DEFINED
#define skgpu_graphite_SparseStripsConstants_DEFINED

#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"

#include <cstdint>
#include <vector>

namespace skgpu::graphite {

// storage for all magic numbers needed by sparse strips
struct SparseStripsConstants {
    //==============================================================================================
    // For the tiler
    //==============================================================================================
    static constexpr uint32_t BOT_SHIFT      = 1;
    static constexpr uint32_t LEFT_SHIFT     = 2;
    static constexpr uint32_t RIGHT_SHIFT    = 3;
    static constexpr uint32_t WINDING_SHIFT  = 4;
    static constexpr uint32_t INT_MASK_SHIFT = 5;
    static constexpr uint32_t Y_SHIFT        = 16;

    static constexpr uint32_t T = 1 << 0;
    static constexpr uint32_t B = 1 << BOT_SHIFT;
    static constexpr uint32_t L = 1 << LEFT_SHIFT;
    static constexpr uint32_t R = 1 << RIGHT_SHIFT;
    static constexpr uint32_t W = 1 << WINDING_SHIFT;

    static constexpr uint32_t INTERSECTION_MASK  = W | R | L | B | T;
    static constexpr uint32_t XY_MASK            = (1 << Y_SHIFT) - 1;

    static constexpr uint32_t MAX_LINES_PER_PATH = 1 << (32 - INT_MASK_SHIFT);

    static std::string maskToString(uint32_t mask) {
        if (mask == 0) return "0";

        std::vector<std::string> parts;
        if (mask & W) parts.push_back("W");
        if (mask & T) parts.push_back("T");
        if (mask & B) parts.push_back("B");
        if (mask & L) parts.push_back("L");
        if (mask & R) parts.push_back("R");

        std::string s;
        for (size_t i = 0; i < parts.size(); ++i) {
            s += parts[i];
            if (i < parts.size() - 1) {
                s += " | ";
            }
        }
        return s;
    }

    //==============================================================================================
    // For the strip generation
    //==============================================================================================
    static constexpr uint32_t kNumSubSamples  = sizeof(SparseStripsConfig::MSAAType) * 8;
    static constexpr uint32_t kSubSampleMask  = (1 << kNumSubSamples) - 1;
    static constexpr uint32_t kMSAASubAsAdd   = static_cast<uint32_t>(-1) & kSubSampleMask;
    static constexpr uint32_t kMSAABias       = 1 << (kNumSubSamples - 1);
    static constexpr float    kMSAAInvSamples = 1.0f / static_cast<float>(kNumSubSamples);
    static constexpr float    kMSAASamplesF   = static_cast<float>(kNumSubSamples);

    static constexpr uint32_t FILL_SHIFT    = 31;
    static constexpr float    kStripEpsilon = 1e-5f;

    //==============================================================================================
    // For the compute shader
    //==============================================================================================
    static constexpr uint32_t SCANNED_WINDING_SHIFT = 24;    // Upper 8 bits
    static constexpr uint32_t INVALID_START_TILE    = SparseStripsConfig::kWorkgroupSize;
    static constexpr uint32_t FILL_RULE_SHIFT       = 22;    // Next 2 bits
    static constexpr uint32_t FILL_RULE_MASK        = 0x3;
    static constexpr uint32_t IS_END_TILE_SHIFT     = 21;    // Next 1 bit
    static constexpr uint32_t IS_END_TILE_MASK      = 0x1;
    static constexpr uint32_t LOCAL_TILE_ID_MASK    = (1 << IS_END_TILE_SHIFT) - 1;
    static constexpr uint32_t START_TILE_MASK       = 0x1ff; // Lower 9 bits
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_SparseStripsConstants_DEFINED
