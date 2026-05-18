/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_MakeStripsScalar_impl_DEFINED
#define skgpu_graphite_sparse_strips_MakeStripsScalar_impl_DEFINED

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight, bool kIsWinding>
void Strip::MakeStripsScalarImpl(const Tiles<kTileWidth, kTileHeight>& tileContainer,
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

    int16_t maskBuffer[kTileHeight][kTileWidth][kNumSubSamples];

    auto clearMask = [&](int16_t value) {
        for (int32_t row = 0; row < kTileHeight; ++row) {
            for (int32_t column = 0; column < kTileWidth; ++column) {
                for (int32_t k = 0; k < kNumSubSamples; ++k) {
                    maskBuffer[row][column][k] = value;
                }
            }
        }
    };
    clearMask(0);

    auto shouldFill = [](int32_t w) {
        if constexpr (kIsWinding) {
            return w != 0;
        } else {
            return (w & 1) != 0;
        }
    };

    int32_t windingDelta = 0;
    constexpr int32_t kTileAlphaCount = kTileWidth * kTileHeight;
    int32_t localAlphaIdx = alphaBuf.size();
    size_t megaGuess = tiles.size() * kTileAlphaCount * 2;
    alphaBuf.reserve(localAlphaIdx + megaGuess);

    size_t totalCount = tiles.size();
    Tile prevTile = tiles[0];
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

            for (int32_t row = 0; row < kTileHeight; ++row) {
                for (int32_t column = 0; column < kTileWidth; ++column) {
                    int32_t activeSamples = 0;

                    for (int32_t k = 0; k < kNumSubSamples; ++k) {
                        if (shouldFill(maskBuffer[row][column][k])) {
                            activeSamples++;
                        }
                    }
                    if (isInverse) {
                        activeSamples = kNumSubSamples - activeSamples;
                    }

#if defined(GPU_TEST_UTILS)
                    if (observer) {
                        uint8_t exactMask = 0;
                        for (int32_t k = 0; k < kNumSubSamples; ++k) {
                            if (shouldFill(maskBuffer[row][column][k])) {
                                exactMask |= (1 << k);
                            }
                        }
                        if (isInverse) {
                            exactMask = ~exactMask & ((1 << kNumSubSamples) - 1);
                        }
                        observer(exactMask);
                    }
#endif

                    uint8_t alpha = static_cast<uint8_t>((activeSamples * 255 + 4) / 8);
                    tileAlphaBase[localWriteIdx++] = alpha;
                }
            }

            if (!rowStart) {
                clearMask(static_cast<int16_t>(windingDelta));
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
                clearMask(0);
            }

            if (isSentinel) {
                break;
            }
            currentStrip = Strip(tile.x * kTileWidth,
                                 tile.y * kTileHeight,
                                 localAlphaIdx,
                                 shouldFill(windingDelta));
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
        float idx = (std::abs(dx) <= kStripEpsilon) ? 0.0f : 1.0f / dx;
        float idy = (std::abs(dy) <= kStripEpsilon) ? 0.0f : 1.0f / dy;
        float dxdy = dx * idy;

        if (tileStart) {
            tileMinX = static_cast<float>(tile.x * kTileWidth);
            tileMinY = static_cast<float>(tile.y * kTileHeight);
            tileMaxX = tileMinX + static_cast<float>(kTileWidth);
            tileMaxY = tileMinY + static_cast<float>(kTileHeight);
        }

        std::array<float, 4> bounds = {tileMinX, tileMinY, tileMaxX, tileMaxY};
        std::array<float, 4> derivs = {dx, dy, idx, idy};

        auto clipped = ClipToTile(
                line, bounds, derivs, tile.intersectionMask(), canonicalXDir, canonicalYDir);
        SkPoint pTop = clipped[0];
        SkPoint pBot = clipped[1];

        if (tile.left()) {
            // If the line is perfectly vertical we should choose the bottom edge.
            float yEdge = pTop.fX < pBot.fX ? pTop.fY : pBot.fY;
            int32_t startY = static_cast<int32_t>(std::ceil(yEdge));

            int32_t val;
            if constexpr (kIsWinding) {
                val = canonicalXDir ? -1 : 1;
            } else {
                val = 1;
            }

            for (int32_t row = startY; row < kTileHeight; ++row) {
                for (int32_t column = 0; column < kTileWidth; ++column) {
                    for (int32_t k = 0; k < kNumSubSamples; ++k) {
                        if constexpr (kIsWinding) {
                            maskBuffer[row][column][k] += val;
                        } else {
                            maskBuffer[row][column][k] ^= val;
                        }
                    }
                }
            }
        }

        if (std::abs(dy) < kStripEpsilon && pTop.fY == std::floor(pTop.fY)) {
            continue;
        }

        int32_t startY = static_cast<int32_t>(std::floor(pTop.fY));
        int32_t endY = static_cast<int32_t>(std::ceil(pBot.fY));

        std::array<SkPoint, kTileHeight + 1> rows;
        rows.fill(SkPoint::Make(std::numeric_limits<float>::quiet_NaN(),
                                std::numeric_limits<float>::quiet_NaN()));
        rows[startY] = pTop;
        for (int32_t row = startY + 1; row < endY; ++row) {
            float gy = static_cast<float>(row);
            float gx = pTop.fX + (gy - pTop.fY) * dxdy;
            rows[row] = {gx, gy};
        }
        rows[endY] = pBot;

        // LUT indexing setup
        float nx = dy;
        float ny = -dx;
        if (nx < 0.0f) {
            nx = -nx;
            ny = -ny;
        }
        float D = nx + std::abs(ny);
        float invD = (D < kStripEpsilon) ? 0.0f : 1.0f / D;
        bool isPos = ny <= 0.0f;
        float c_local = nx * pTop.fX + ny * pTop.fY;
        float s = std::abs(ny) * invD;
        int v_mod = std::clamp(
                static_cast<int>(std::floor(s * (kMaskHeight / 2))), 0, (kMaskHeight / 2) - 1);
        int v = isPos ? (v_mod + kMaskHeight / 2) : v_mod;
        float stepX = nx * invD;
        float stepY = ny * invD;
        float t_base = ((isPos ? nx : D) - c_local) * invD;

        bool xDir = pTop.fX <= pBot.fX;
        for (int32_t row = startY; row < endY; ++row) {
            float p_top_x = rows[row].fX;
            float p_top_y = rows[row].fY;
            float p_bot_x = rows[row + 1].fX;
            float p_bot_y = rows[row + 1].fY;

            if (std::isnan(p_top_x) || std::isnan(p_bot_x)) {
                continue;
            }

            float xMin = std::fmin(p_top_x, p_bot_x);
            float xMax = std::fmax(p_top_x, p_bot_x);

            int32_t xStart = std::clamp(static_cast<int32_t>(std::floor(xMin)), 0, kTileWidth - 1);
            int32_t xEnd = std::clamp(static_cast<int32_t>(std::floor(xMax)), 0, kTileWidth - 1);

            float t_val =
                    t_base + stepY * static_cast<float>(row) + stepX * static_cast<float>(xStart);

            // EndCap and bump setup
            bool is_startY = (row == startY);
            bool is_endY_minus_1 = (row == endY - 1);
            bool pixelTopTouch = (p_top_y != std::floor(p_top_y));
            bool default_bumped = pixelTopTouch && xDir;

            int canonical_start_x = xDir ? xStart : xEnd;
            int canonical_end_x = xDir ? xEnd : xStart;

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

            for (int32_t column = xStart; column <= xEnd; ++column) {
                int u = std::clamp(
                        static_cast<int>(std::floor(t_val * kMaskWidthF)), 0, kMaskWidthExcl);
                SkASSERT(v * kMaskWidth + u < maskLut.size());
                uint8_t maskVal = maskLut[v * kMaskWidth + u];

                uint8_t edgeMask = 0xff;
                if (column == canonical_start_x) edgeMask &= start_mask_val;
                if (column == canonical_end_x) edgeMask &= end_mask_val;
                maskVal &= edgeMask;

                bool bumped =
                        (is_startY && column == canonical_start_x) ? start_bumped : default_bumped;
                for (int32_t k = 0; k < kNumSubSamples; ++k) {
                    int32_t maskBit = (maskVal & (1 << k)) ? 1 : 0;
                    if constexpr (kIsWinding) {
                        maskBit -= bumped;
                        if (maskBit != 0) {
                            if (canonicalYDir) {
                                maskBuffer[row][column][k] += maskBit;
                            } else {
                                maskBuffer[row][column][k] -= maskBit;
                            }
                        }
                    } else {
                        maskBuffer[row][column][k] ^= (maskBit ^ bumped);
                    }
                }

                t_val += stepX;
            }

            bool crossedTop = (row > startY) || (p_top_y == std::floor(p_top_y));
            if (crossedTop) {
                int32_t val;
                if constexpr (kIsWinding) {
                    val = canonicalYDir ? 1 : -1;
                } else {
                    val = 1;
                }
                for (int32_t column = xEnd + 1; column < kTileWidth; ++column) {
                    for (int32_t k = 0; k < kNumSubSamples; ++k) {
                        if constexpr (kIsWinding) {
                            maskBuffer[row][column][k] += val;
                        } else {
                            maskBuffer[row][column][k] ^= val;
                        }
                    }
                }
            }
        }
    }

    alphaBuf.resize(localAlphaIdx);
}

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_MakeStripsScalar_impl_DEFINED
