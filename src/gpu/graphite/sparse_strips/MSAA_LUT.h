/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_MSAA_LUT_DEFINED
#define skgpu_graphite_sparse_strips_MSAA_LUT_DEFINED

#include "include/private/base/SkTDArray.h"
#include "src/gpu/graphite/sparse_strips/StripGenerator.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <type_traits>

namespace skgpu::graphite {

// TODO: (thomsmit) try slope based LUT
template <typename T> class MSAA_LUT {
public:
    static constexpr uint32_t kDim = 64;
    static constexpr float    kPackingScale = 0.5;
    static constexpr size_t   kSampleCount = sizeof(T) * 8;

    static SkTDArray<T> Make(const std::array<uint8_t, kSampleCount>& pattern) {
        constexpr float scale = 1.0f / static_cast<float>(kSampleCount);
        std::array<float, kSampleCount> subX;
        std::array<float, kSampleCount> subY;

        for (size_t k = 0; k < kSampleCount; ++k) {
            subX[k] = (static_cast<float>(pattern[k]) + 0.5f) * scale;
            subY[k] = (static_cast<float>(k) + 0.5f) * scale;
        }

        SkTDArray<T> lut;
        lut.reserve(kDim * kDim);
        for (uint32_t j = 0; j < kDim; ++j) {
            for (uint32_t i = 0; i < kDim; ++i) {
                float xf = (static_cast<float>(i) + 0.5f) / static_cast<float>(kDim);
                float yf = (static_cast<float>(j) + 0.5f) / static_cast<float>(kDim);

                float nRevX = 2.0f * (xf - 0.5f);
                float nRevY = 2.0f * (yf - 0.5f);

                float lgRev = std::hypot(nRevX, nRevY);
                if (lgRev < 1e-9f) {
                    lgRev = 1e-9f;
                }

                float cDist = std::max(0.0f, 1.0f - lgRev) * (1.0f / kPackingScale);
                float nCanonicalX = nRevX / lgRev;
                float nCanonicalY = nRevY / lgRev;
                if (nCanonicalX < 0.0f) {
                    nCanonicalX = -nCanonicalX;
                    nCanonicalY = -nCanonicalY;
                    cDist = -cDist;
                }

                float cPlane = cDist + 0.5f * (nCanonicalX + nCanonicalY);

                T mask = 0;
                for (size_t k = 0; k < kSampleCount; ++k) {
                    float val = nCanonicalX * subX[k] + nCanonicalY * subY[k] - cPlane;
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

// MSAA pattern is based on the D3D11 standard multisample pattern:
// https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_standard_multisample_quality_levels
template <typename T>
SkTDArray<T> GenerateMSAALUT() {
    using LUT = MSAA_LUT<T>;
    if constexpr (std::is_same_v<T, uint8_t>) {
        constexpr std::array<uint8_t, LUT::kSampleCount> kPattern = {
            0, 5, 3, 7, 1, 4, 6, 2
        };
        return LUT::Make(kPattern);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        constexpr std::array<uint8_t, LUT::kSampleCount> kPattern = {
            1, 8, 4, 11, 15, 7, 3, 12, 0, 9, 5, 13, 2, 10, 6, 14
        };
        return LUT::Make(kPattern);
    } else {
        static_assert(sizeof(T) == 0, "GenerateMSAALUT only supports uint8_t or uint16_t.");
        return {};
    }
}

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_MSAA_LUT_DEFINED
