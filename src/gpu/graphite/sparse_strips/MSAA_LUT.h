/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_MSAA_LUT_DEFINED
#define skgpu_graphite_MSAA_LUT_DEFINED

#include "include/private/base/SkTDArray.h"
#include "src/gpu/graphite/sparse_strips/StripGenerator.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <type_traits>

namespace skgpu::graphite {

// TODO: (thomsmit) try slope based LUT
template <typename T, uint32_t LUT_DIM> class MSAA_LUT {
public:
    static constexpr size_t kSampleCount = sizeof(T) * 8;
    static SkTDArray<T> Make(const std::array<uint8_t, kSampleCount>& pattern,
                             float kPackingScale) {
        constexpr float scale = 1.0f / static_cast<float>(kSampleCount);
        std::array<float, kSampleCount> sub_x;
        std::array<float, kSampleCount> sub_y;

        for (size_t k = 0; k < kSampleCount; ++k) {
            sub_x[k] = (static_cast<float>(pattern[k]) + 0.5f) * scale;
            sub_y[k] = (static_cast<float>(k) + 0.5f) * scale;
        }

        SkTDArray<T> lut;
        lut.reserve(LUT_DIM * LUT_DIM);
        for (uint32_t j = 0; j < LUT_DIM; ++j) {
            for (uint32_t i = 0; i < LUT_DIM; ++i) {
                float xf = (static_cast<float>(i) + 0.5f) / static_cast<float>(LUT_DIM);
                float yf = (static_cast<float>(j) + 0.5f) / static_cast<float>(LUT_DIM);

                float n_rev_x = 2.0f * (xf - 0.5f);
                float n_rev_y = 2.0f * (yf - 0.5f);

                float lg_rev = std::hypot(n_rev_x, n_rev_y);
                if (lg_rev < 1e-9f) {
                    lg_rev = 1e-9f;
                }

                float n_lookup_x = n_rev_x / lg_rev;
                float n_lookup_y = n_rev_y / lg_rev;

                float c_dist_unsigned = std::max(0.0f, 1.0f - lg_rev) * (1.0f / kPackingScale);

                float n_canonical_x = n_lookup_x;
                float n_canonical_y = n_lookup_y;
                float c_signed_dist = c_dist_unsigned;

                if (n_lookup_x < 0.0f) {
                    n_canonical_x = -n_lookup_x;
                    n_canonical_y = -n_lookup_y;
                    c_signed_dist = -c_dist_unsigned;
                }

                float c_plane = c_signed_dist + 0.5f * (n_canonical_x + n_canonical_y);

                T mask = 0;
                for (size_t k = 0; k < kSampleCount; ++k) {
                    float val = n_canonical_x * sub_x[k] + n_canonical_y * sub_y[k] - c_plane;
                    if (val > 0.0f) {
                        mask |= (static_cast<T>(1) << k);
                    }
                }
                lut.push_back(mask);
            }
        }
        return lut;
    }
};

template <typename T>
SkTDArray<T> GenerateMSAALUT() {
    using LUT = MSAA_LUT<T, StripGenerator::kMSAADim>;
    if constexpr (std::is_same_v<T, uint8_t>) {
        constexpr std::array<uint8_t, LUT::kSampleCount> kPattern = {
            0, 5, 3, 7, 1, 4, 6, 2
        };
        return LUT::Make(kPattern, StripGenerator::kPackingScale);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        constexpr std::array<uint8_t, LUT::kSampleCount> kPattern = {
            1, 8, 4, 11, 15, 7, 3, 12, 0, 9, 5, 13, 2, 10, 6, 14
        };
        return LUT::Make(kPattern, StripGenerator::kPackingScale);
    } else {
        static_assert(sizeof(T) == 0, "GenerateMSAALUT only supports uint8_t or uint16_t.");
        return {};
    }
}

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_MSAA_LUT_DEFINED
