/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_MakeStripsSimd_impl_DEFINED
#define skgpu_graphite_sparse_strips_MakeStripsSimd_impl_DEFINED

#include <algorithm>
#include <cmath>
#include <limits>
#include <array>

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight, bool kIsWinding>
void Strip::MakeStripsSimdImpl(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                               SkTDArray<Strip>& stripBuf,
                               SkTDArray<uint8_t>& alphaBuf,
                               bool isInverse,
                               const Polyline& polyline,
                               const SkTDArray<uint8_t>& maskLut
#if defined(GPU_TEST_UTILS)
                               ,
                               MsaaExactMaskObserver observer
#endif
) {
    const auto& tiles = tileContainer.getTiles();
    if (tiles.empty()) {
        return;
    }

    auto shouldFill = [](int32_t w) {
        if constexpr (kIsWinding) {
            return w != 0;
        } else {
            return (w & 1) != 0;
        }
    };

    size_t totalCount = tiles.size();
    Tile prevTile = tiles[0];

    constexpr int32_t kTileAlphaCount = kTileWidth * kTileHeight;
    int32_t localAlphaIdx = alphaBuf.size();
    size_t megaGuess = tiles.size() * kTileAlphaCount * 2;
    alphaBuf.reserve(localAlphaIdx + megaGuess);

    using swar_pixel = skvx::Vec<2, uint32_t>;
    using pixel_bytes = skvx::Vec<8, uint8_t>;
    swar_pixel maskBuffer[kTileHeight][kTileWidth];

    auto clearMask = [&](uint32_t val) {
        // OPTIMIZATION: Removed if (val == 0) memset().
        // Forcing NEON vector stores is faster than a libc memset call for small tile arrays.
        swar_pixel* flat_mask = &maskBuffer[0][0];
        constexpr int totalPixels = kTileWidth * kTileHeight;

        if constexpr (kTileWidth % 8 == 0) {
            skvx::Vec<16, uint32_t> v_val = {val, val, val, val, val, val, val, val,
                                             val, val, val, val, val, val, val, val};
            for (int i = 0; i < totalPixels; i += 8) {
                v_val.store(flat_mask + i);
            }
        } else if constexpr (kTileWidth % 4 == 0) {
            skvx::Vec<8, uint32_t> v_val = {val, val, val, val, val, val, val, val};
            for (int i = 0; i < totalPixels; i += 4) {
                v_val.store(flat_mask + i);
            }
        } else {
            swar_pixel p = {val, val};
            for (int i = 0; i < totalPixels; ++i) {
                flat_mask[i] = p;
            }
        }
    };

    const uint32_t kInitialMaskVal = kIsWinding ? 0x80808080u : 0u;
    clearMask(kInitialMaskVal);

    int32_t windingDelta = 0;
    Strip currentStrip(prevTile.x * kTileWidth, prevTile.y * kTileHeight, localAlphaIdx, false);

    float tileMinX = static_cast<float>(prevTile.x * kTileWidth);
    float tileMinY = static_cast<float>(prevTile.y * kTileHeight);
    float tileMaxX = tileMinX + static_cast<float>(kTileWidth);
    float tileMaxY = tileMinY + static_cast<float>(kTileHeight);

    for (size_t i = 0; i <= totalCount; ++i) {
        bool isSentinel = (i == totalCount);
        Tile tile;

        if (isSentinel) {
            tile = Tile(Tile::kSentinelCoord, Tile::kSentinelCoord, 0, 0);
        } else {
            tile = tiles[i];
        }

        bool tileStart = (tile.x != prevTile.x || tile.y != prevTile.y);
        bool rowStart = (tile.y != prevTile.y);
        bool segStart = tileStart && !((tile.y == prevTile.y) && (tile.x == prevTile.x + 1));

        if (tileStart) {
            if (alphaBuf.size() + kTileAlphaCount > alphaBuf.capacity()) {
                constexpr size_t kChunkSize = 4 * kTileAlphaCount;
                alphaBuf.reserve(alphaBuf.capacity() + kChunkSize);
            }

            uint8_t* tileAlphaBase = alphaBuf.append(kTileAlphaCount);
            int localWriteIdx = 0;

            // Use skvx::Vec to perform true hardware SIMD byte comparisons. We cast the vector into
            // 8-bit byte lanes. In winding mode, the winding sum is biased by +128 (0x80), so a
            // byte has active coverage if it does NOT equal 0x80.
            auto get_counts_wide = [&](auto v) {
                using VecT = decltype(v);
                if constexpr (kIsWinding) {
                    using Vec8 = skvx::Vec<sizeof(VecT), uint8_t>;
                    Vec8 v_bytes = sk_bit_cast<Vec8>(v);
                    Vec8 bias_bytes(0x80);
                    Vec8 cmp = (v_bytes != bias_bytes);
                    VecT active_bytes = sk_bit_cast<VecT>(cmp) & VecT(0x01010101u);
                    return (active_bytes * VecT(0x01010101u)) >> 24;
                } else {
                    VecT ones(0x01010101u);
                    VecT active_bytes = v & ones;
                    return (active_bytes * ones) >> 24;
                }
            };

            auto process_chunk = [&](auto size_tag, int32_t row, int32_t column) {
                constexpr int ChunkSize = decltype(size_tag)::value;

                auto v_mask = skvx::Vec<ChunkSize * 2, uint32_t>::Load(&maskBuffer[row][column]);

                auto counts = get_counts_wide(v_mask);

                skvx::Vec<ChunkSize, uint32_t> activeSamples;
                if constexpr (ChunkSize == 8) {
                    auto counts_lo = skvx::shuffle<0, 2, 4, 6, 8, 10, 12, 14>(counts);
                    auto counts_hi = skvx::shuffle<1, 3, 5, 7, 9, 11, 13, 15>(counts);
                    activeSamples = counts_lo + counts_hi;
                } else {
                    auto counts_lo = skvx::shuffle<0, 2, 4, 6>(counts);
                    auto counts_hi = skvx::shuffle<1, 3, 5, 7>(counts);
                    activeSamples = counts_lo + counts_hi;
                }

                if (isInverse) {
                    activeSamples = skvx::Vec<ChunkSize, uint32_t>(kNumSubSamples) - activeSamples;
                }

                skvx::Vec<ChunkSize, uint32_t> alpha32 = (activeSamples * 255 + 4) >> 3;
                skvx::cast<uint8_t>(alpha32).store(tileAlphaBase + localWriteIdx);
                localWriteIdx += ChunkSize;
                localAlphaIdx += ChunkSize;

#if defined(GPU_TEST_UTILS)
                if (observer) {
                    for (int32_t xx = column; xx < column + ChunkSize; ++xx) {
                        swar_pixel v = maskBuffer[row][xx];

                        uint8_t exactMask = 0;
                        uint32_t lo = v[0];
                        uint32_t hi = v[1];
                        for (int s = 0; s < 4; ++s) {
                            int8_t s_lo = static_cast<int8_t>(lo & 0xFF);
                            int8_t s_hi = static_cast<int8_t>(hi & 0xFF);
                            if constexpr (kIsWinding) {
                                s_lo = static_cast<int8_t>(static_cast<uint8_t>(s_lo) - 0x80);
                                s_hi = static_cast<int8_t>(static_cast<uint8_t>(s_hi) - 0x80);
                            }
                            if (shouldFill(s_lo)) exactMask |= (1 << s);
                            if (shouldFill(s_hi)) exactMask |= (1 << (s + 4));
                            lo >>= 8;
                            hi >>= 8;
                        }
                        if (isInverse) {
                            exactMask = ~exactMask & ((1 << kNumSubSamples) - 1);
                        }
                        observer(exactMask);
                    }
                }
#endif
            };

            for (int32_t row = 0; row < kTileHeight; ++row) {
                if constexpr (kTileWidth % 8 == 0) {
                    for (int32_t column = 0; column < kTileWidth; column += 8) {
                        process_chunk(std::integral_constant<int, 8>{}, row, column);
                    }
                } else if constexpr (kTileWidth % 4 == 0) {
                    for (int32_t column = 0; column < kTileWidth; column += 4) {
                        process_chunk(std::integral_constant<int, 4>{}, row, column);
                    }
                }
            }

            if (!rowStart) {
                uint8_t w_byte;
                if constexpr (kIsWinding) {
                    w_byte = 0x80 + static_cast<uint8_t>(static_cast<int8_t>(windingDelta));
                } else {
                    w_byte = (windingDelta & 1) ? 1 : 0;
                }

                uint32_t w_val = w_byte * 0x01010101u;
                clearMask(w_val);
            }
        }

        if (segStart) {
            stripBuf.push_back(currentStrip);

            if (rowStart) {
                if (windingDelta != 0 || isSentinel) {
                    stripBuf.push_back(Strip(Tile::kSentinelCoord,
                                             prevTile.y * kTileHeight,
                                             localAlphaIdx,
                                             shouldFill(windingDelta)));
                }

                windingDelta = 0;
                clearMask(kInitialMaskVal);
            }

            if (!isSentinel) {
                currentStrip = Strip(tile.x * kTileWidth,
                                     tile.y * kTileHeight,
                                     localAlphaIdx,
                                     shouldFill(windingDelta));
            }
        }

        prevTile = tile;

        if (isSentinel) {
            break;
        }

        Line line = polyline.getLine(tile.lineIdx());
        bool canonicalXDir = line.p1.fX >= line.p0.fX;
        bool canonicalYDir = line.p1.fY >= line.p0.fY;

        uint32_t windingBit = tile.coarseWinding() ? 1 : 0;
        if constexpr (kIsWinding) {
            windingDelta += (canonicalYDir ? 1 : -1) * static_cast<int32_t>(windingBit);
        } else {
            windingDelta ^= static_cast<int32_t>(windingBit);
        }

        float rightEdge = canonicalXDir ? line.p1.fX : line.p0.fX;
        if (rightEdge < 0.0f) {
            continue;
        }

        float dx = line.p1.fX - line.p0.fX;
        float dy = line.p1.fY - line.p0.fY;

        // Hoist D calculation up here so we can batch the math
        float nx = dy;
        float ny = -dx;
        if (nx < 0.0f) {
            nx = -nx;
            ny = -ny;
        }
        float D = nx + std::abs(ny);

        // Vectorize dx, dy, and D into a skvx::float4.
        // This fixes the type deduction error AND computes 3 reciprocals in a single CPU cycle!
        skvx::float4 rcp_inputs{dx, dy, D, 1.0f};
        skvx::float4 recips = skvx::approx_rcp(rcp_inputs);

        float idx = (std::abs(dx) <= kStripEpsilon) ? 0.0f : recips[0];
        float idy = (std::abs(dy) <= kStripEpsilon) ? 0.0f : recips[1];
        float invD = (D < kStripEpsilon) ? 0.0f : recips[2];

        float dxdy = dx * idy;

        if (tileStart) {
            tileMinX = static_cast<float>(tile.x * kTileWidth);
            tileMinY = static_cast<float>(tile.y * kTileHeight);
            tileMaxX = tileMinX + static_cast<float>(kTileWidth);
            tileMaxY = tileMinY + static_cast<float>(kTileHeight);
        }

        std::array<float, 4> bounds = {tileMinX, tileMinY, tileMaxX, tileMaxY};
        std::array<float, 4> derivs = {dx, dy, idx, idy};

        auto clipped = ClipToTile(line,
                                  bounds,
                                  derivs,
                                  tile.intersectionMask(),
                                  canonicalXDir,
                                  canonicalYDir);
        SkPoint pTop = clipped[0];
        SkPoint pBot = clipped[1];

        // --- Wide SIMD L Boundary Fill ---
        if (tile.left()) {
            float yEdge = (pTop.fX < pBot.fX) ? pTop.fY : pBot.fY;
            uint8_t fill_byte;
            if constexpr (kIsWinding) {
                fill_byte = canonicalXDir ? 0xFF : 1;
            } else {
                fill_byte = 1;
            }

            uint32_t fill_32 = fill_byte * 0x01010101u;
            for (int32_t row = static_cast<int32_t>(std::ceil(yEdge)); row < kTileHeight; ++row) {
                if constexpr (kTileWidth % 8 == 0) {
                    skvx::Vec<16, uint32_t> v_fill16_32 = {
                        fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32,
                        fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32 };
                    for (int32_t column = 0; column < kTileWidth; column += 8) {
                        auto v_mask = skvx::Vec<16, uint32_t>::Load(&maskBuffer[row][column]);
                        if constexpr (kIsWinding) {
                            auto m_b = sk_bit_cast<skvx::Vec<64, uint8_t>>(v_mask);
                            auto f_b = sk_bit_cast<skvx::Vec<64, uint8_t>>(v_fill16_32);
                            v_mask = sk_bit_cast<skvx::Vec<16, uint32_t>>(m_b + f_b);
                        } else {
                            v_mask ^= v_fill16_32;
                        }
                        v_mask.store(&maskBuffer[row][column]);
                    }
                } else if constexpr (kTileWidth % 4 == 0) {
                    skvx::Vec<8, uint32_t> v_fill8_32 = {
                        fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32 };
                    for (int32_t column = 0; column < kTileWidth; column += 4) {
                        auto v_mask = skvx::Vec<8, uint32_t>::Load(&maskBuffer[row][column]);
                        if constexpr (kIsWinding) {
                            auto m_b = sk_bit_cast<skvx::Vec<32, uint8_t>>(v_mask);
                            auto f_b = sk_bit_cast<skvx::Vec<32, uint8_t>>(v_fill8_32);
                            v_mask = sk_bit_cast<skvx::Vec<8, uint32_t>>(m_b + f_b);
                        } else {
                            v_mask ^= v_fill8_32;
                        }
                        v_mask.store(&maskBuffer[row][column]);
                    }
                }
            }
        }

        if (std::abs(dy) < kStripEpsilon && pTop.fY == std::floor(pTop.fY)) {
            continue;
        }

        int32_t startY = static_cast<int32_t>(std::floor(pTop.fY));
        int32_t endY = static_cast<int32_t>(std::ceil(pBot.fY));

        constexpr int32_t kPaddedHeight = ((kTileHeight + 1 + 3) / 4) * 4;
        static constexpr auto kDefaultEdgeY = []() {
            std::array<float, kPaddedHeight> arr{};
            for (int32_t j = 0; j < kPaddedHeight; ++j) arr[j] = static_cast<float>(j);
            return arr;
        }();

        float edgeX[kPaddedHeight];
        float edgeY[kPaddedHeight];
        std::copy(kDefaultEdgeY.begin(), kDefaultEdgeY.end(), std::begin(edgeY));

        {
            skvx::float4 v_pTopX(pTop.fX);
            skvx::float4 v_pTopY(pTop.fY);
            skvx::float4 v_dxdy(dxdy);

            int32_t height;
            if constexpr (kTileHeight > 4) {
                height = (endY + 3) / 4 * 4;
            } else {
                height = 4;
            }
            for (int32_t k = 0; k < height; k += 4) {
                skvx::float4 v_gridY = skvx::float4::Load(edgeY + k);
                skvx::float4 v_gridX = v_pTopX + (v_gridY - v_pTopY) * v_dxdy;
                v_gridX.store(edgeX + k);
            }

            edgeX[startY] = pTop.fX;
            edgeY[startY] = pTop.fY;
            edgeX[endY] = pBot.fX;
            edgeY[endY] = pBot.fY;
        }

        bool isPos = ny <= 0.0f;
        float c_local = nx * pTop.fX + ny * pTop.fY;
        float s = std::abs(ny) * invD;
        int v_mod = std::clamp(
                static_cast<int>(std::floor(s * (kMaskHeight / 2))), 0, (kMaskHeight / 2) - 1);
        int v = isPos ? (v_mod + kMaskHeight / 2) : v_mod;
        float stepX = nx * invD;
        float stepY = ny * invD;
        float t_base = ((isPos ? nx : D) - c_local) * invD;

        constexpr float kFixedMult = 64.0f * 65536.0f;
        int32_t stepX_fixed  = static_cast<int32_t>(stepX * kFixedMult);
        int32_t stepY_fixed  = static_cast<int32_t>(stepY * kFixedMult);
        int32_t t_base_fixed = static_cast<int32_t>(t_base * kFixedMult);

        int32_t t_row_fixed = t_base_fixed + (stepY_fixed * startY);
        bool xDir = pTop.fX <= pBot.fX;
        for (int32_t row = startY; row < endY; ++row) {
            float p_top_x = edgeX[row];
            float p_top_y = edgeY[row];
            float p_bot_x = edgeX[row + 1];
            float p_bot_y = edgeY[row + 1];

            float xMin = std::fmin(p_top_x, p_bot_x);
            float xMax = std::fmax(p_top_x, p_bot_x);
            int32_t xStart = std::clamp(static_cast<int32_t>(std::floor(xMin)), 0, kTileWidth - 1);
            int32_t xEnd = std::clamp(static_cast<int32_t>(std::floor(xMax)), 0, kTileWidth - 1);

            int32_t t_fixed = t_row_fixed + (stepX_fixed * xStart);
            int lut_row_offset = v * kMaskWidth;

            bool is_startY = (row == startY);
            bool is_endY_minus_1 = (row == endY - 1);
            bool pixelTopTouch = (p_top_y != std::floor(p_top_y));
            bool default_bumped = pixelTopTouch && xDir;

            uint8_t start_mask_val = 0xff;
            bool start_bumped = default_bumped;
            if (is_startY) {
                start_bumped = (p_top_x == 0.0f && pixelTopTouch);
                if (!start_bumped) {
                    uint32_t shift = static_cast<uint32_t>(
                            std::round(8.0f * (p_top_y - static_cast<float>(row))));
                    start_mask_val = static_cast<uint8_t>(0xff << shift);
                }
            }

            uint8_t end_mask_val = 0xff;
            if (is_endY_minus_1 && p_bot_x != 0.0f) {
                uint32_t shift = static_cast<uint32_t>(
                        std::round(8.0f * (p_bot_y - static_cast<float>(row))));
                end_mask_val = static_cast<uint8_t>(~(0xff << shift));
            }
            if (!is_startY) start_mask_val = 0xff;
            if (!is_endY_minus_1) end_mask_val = 0xff;

            auto processPixel = [&](int32_t column, uint8_t edgeMask, uint32_t bumpVal) {
                int u = std::clamp(t_fixed >> 16, 0, kMaskWidthExcl);
                SkASSERT(v * kMaskWidth + u < maskLut.size());
                uint8_t maskVal = maskLut[lut_row_offset + u] & edgeMask;

                uint32_t mask_lo = ((maskVal & 0x0F) * 0x00204081u) & 0x01010101u;
                uint32_t mask_hi = (((maskVal >> 4) & 0x0F) * 0x00204081u) & 0x01010101u;

                pixel_bytes p_res = sk_bit_cast<pixel_bytes>(swar_pixel(mask_lo, mask_hi));
                pixel_bytes p_mask = sk_bit_cast<pixel_bytes>(maskBuffer[row][column]);

                pixel_bytes p_bump(bumpVal);

                if constexpr (kIsWinding) {
                    p_res -= p_bump;
                    if (canonicalYDir) {
                        p_mask += p_res;
                    } else {
                        p_mask -= p_res;
                    }
                } else {
                    p_res ^= p_bump;
                    p_mask ^= p_res;
                }

                maskBuffer[row][column] = sk_bit_cast<swar_pixel>(p_mask);
                t_fixed += stepX_fixed;
            };

            if (xStart == xEnd) {
                uint8_t combinedMask = start_mask_val & end_mask_val;
                processPixel(xStart, combinedMask, start_bumped);
            } else {
                uint8_t leftMask = xDir ? start_mask_val : end_mask_val;
                processPixel(xStart, leftMask, xDir ? start_bumped : default_bumped);

                for (int32_t column = xStart + 1; column < xEnd; ++column) {
                    processPixel(column, 0xff, default_bumped);
                }

                uint8_t rightMask = xDir ? end_mask_val : start_mask_val;
                processPixel(xEnd, rightMask, xDir ? default_bumped : start_bumped);
            }

            bool crossedTop = (row > startY) || (p_top_y == std::floor(p_top_y));
            if (crossedTop) {
                uint8_t fill_byte;
                if constexpr (kIsWinding) {
                    fill_byte = canonicalYDir ? 1 : 0xFF;
                } else {
                    fill_byte = 1;
                }

                int32_t column = xEnd + 1;
                uint32_t fill_32 = fill_byte * 0x01010101u;
                if constexpr (kTileWidth >= 8) {
                    skvx::Vec<16, uint32_t> v_fill16_32 = {
                        fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32,
                        fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32 };
                    while (column + 8 <= kTileWidth) {
                        auto v_mask = skvx::Vec<16, uint32_t>::Load(&maskBuffer[row][column]);
                        if constexpr (kIsWinding) {
                            auto m_b = sk_bit_cast<skvx::Vec<64, uint8_t>>(v_mask);
                            auto f_b = sk_bit_cast<skvx::Vec<64, uint8_t>>(v_fill16_32);
                            v_mask = sk_bit_cast<skvx::Vec<16, uint32_t>>(m_b + f_b);
                        } else {
                            v_mask ^= v_fill16_32;
                        }
                        v_mask.store(&maskBuffer[row][column]);
                        column += 8;
                    }
                } else if constexpr (kTileWidth >= 4) {
                    skvx::Vec<8, uint32_t> v_fill8_32 = {
                        fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32 };
                    while (column + 4 <= kTileWidth) {
                        auto v_mask = skvx::Vec<8, uint32_t>::Load(&maskBuffer[row][column]);
                        if constexpr (kIsWinding) {
                            auto m_b = sk_bit_cast<skvx::Vec<32, uint8_t>>(v_mask);
                            auto f_b = sk_bit_cast<skvx::Vec<32, uint8_t>>(v_fill8_32);
                            v_mask = sk_bit_cast<skvx::Vec<8, uint32_t>>(m_b + f_b);
                        } else {
                            v_mask ^= v_fill8_32;
                        }
                        v_mask.store(&maskBuffer[row][column]);
                        column += 4;
                    }
                }

                if constexpr (kTileWidth % 4 != 0) {
                    const pixel_bytes p_fill(fill_byte);
                    for (; column < kTileWidth; ++column) {
                        pixel_bytes p_mask = sk_bit_cast<pixel_bytes>(maskBuffer[row][column]);
                        if constexpr (kIsWinding) {
                            p_mask += p_fill;
                        } else {
                            p_mask ^= p_fill;
                        }
                        maskBuffer[row][column] = sk_bit_cast<swar_pixel>(p_mask);
                    }
                }
            }

            t_row_fixed += stepY_fixed;
        }
    }

    alphaBuf.resize(localAlphaIdx);
}

} // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_MakeStripsSimd_impl_DEFINED
