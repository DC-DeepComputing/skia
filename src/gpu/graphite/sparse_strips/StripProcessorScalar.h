/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_StripProcessorScalar_DEFINED
#define skgpu_graphite_sparse_strips_StripProcessorScalar_DEFINED

#include "include/private/base/SkTDArray.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight, bool kIsWinding>
class StripProcessorScalar {
public:
    StripProcessorScalar(SkTDArray<Strip>* stripBuf,
                         SkTDArray<uint8_t>* alphaBuf,
                         bool isInverse,
                         const Polyline& polyline,
                         const SkTDArray<uint8_t>& maskLut,
                         int32_t initialAlphaIdx
#if defined(GPU_TEST_UTILS)
                         , MsaaExactMaskObserver observer
#endif
                         )
            : fStripBuf(stripBuf)
            , fAlphaBuf(alphaBuf)
            , fIsInverse(isInverse)
            , fPolyline(polyline)
            , fMaskLut(maskLut)
            , fLocalAlphaIdx(initialAlphaIdx)
#if defined(GPU_TEST_UTILS)
            , fObserver(observer)
#endif
    {
        this->clearMaskForNewRow();
    }

    SK_ALWAYS_INLINE void clearMask(int16_t value) {
        for (int32_t row = 0; row < kTileHeight; ++row) {
            for (int32_t column = 0; column < kTileWidth; ++column) {
                for (int32_t k = 0; k < Strip::kNumSubSamples; ++k) {
                    fCoverageMask[row][column][k] = value;
                }
            }
        }
    }

    SK_ALWAYS_INLINE void clearMaskWithCoarseWinding() {
        this->clearMask(static_cast<int16_t>(fCoarseWinding));
    }

    SK_ALWAYS_INLINE void clearMaskForNewRow() { clearMask(0); }

    SK_ALWAYS_INLINE static bool ShouldFill(int32_t w) {
        if constexpr (kIsWinding) {
            return w != 0;       // NonZero
        } else {
            return (w & 1) != 0; // EvenOdd
        }
    }

    SK_ALWAYS_INLINE int32_t coarseWinding() const { return fCoarseWinding; }
    SK_ALWAYS_INLINE void setCoarseWinding(int32_t val) { fCoarseWinding = val; }
    SK_ALWAYS_INLINE int32_t localAlphaIdx() const { return fLocalAlphaIdx; }

    // In order to render the alpha, we need to convert fCoverageMask, which holds the accumulated
    // winding for each of the subsample locations, into actual alpha coverage per pixel. In the
    // scalar version here, we do this naively, by iterating through each subsample location and
    // checking the winding against the fill rule. Inverse fills, are handled by inverting the
    // coverage.
    //
    // For testing, we need to know fill rule results for each subsample location, not just the
    // alpha coverage that we store in the alpha buffer. So on testing builds, we store the exact
    // fill mask for each pixel in a tile.
    SK_ALWAYS_INLINE void resolveTileToAlpha() {
        uint8_t* tileAlphaBase = this->reserveAlphaBuffer();
        int localWriteIdx = 0;

        for (int32_t row = 0; row < kTileHeight; ++row) {
            for (int32_t column = 0; column < kTileWidth; ++column) {
                this->processPixel(row, column, tileAlphaBase, localWriteIdx);
            }
        }
        fLocalAlphaIdx += kTilePixelCount;
    }

    SK_ALWAYS_INLINE void rasterizeLineToTile(
            const Tile& tile, float tileMinX, float tileMinY, float tileMaxX, float tileMaxY) {
        Line line = fPolyline.getLine(tile.lineIdx());
        bool canonicalXDir = line.p1.fX >= line.p0.fX;
        bool canonicalYDir = line.p1.fY >= line.p0.fY;

        uint32_t windingBit = tile.coarseWinding() ? 1 : 0;
        if constexpr (kIsWinding) {
            fCoarseWinding += (canonicalYDir ? 1 : -1) * static_cast<int32_t>(windingBit);
        } else {
            fCoarseWinding ^= static_cast<int32_t>(windingBit);
        }

        float rightEdge = canonicalXDir ? line.p1.fX : line.p0.fX;
        if (rightEdge < 0.0f) {
            return;
        }

        float dx = line.p1.fX - line.p0.fX;
        float dy = line.p1.fY - line.p0.fY;
        float idx = (std::abs(dx) <= Strip::kStripEpsilon) ? 0.0f : 1.0f / dx;
        float idy = (std::abs(dy) <= Strip::kStripEpsilon) ? 0.0f : 1.0f / dy;
        float dxdy = dx * idy;

        std::array<float, 4> bounds = {tileMinX, tileMinY, tileMaxX, tileMaxY};
        std::array<float, 4> derivs = {dx, dy, idx, idy};

        auto clipped = Tile::ClipToTile<kTileWidth, kTileHeight>(
                line, bounds, derivs, tile.intersectionMask(), canonicalXDir, canonicalYDir);
        SkPoint pTop = clipped[0];
        SkPoint pBot = clipped[1];

        if (tile.left()) {
            float yEdge = pTop.fX < pBot.fX ? pTop.fY : pBot.fY;
            this->fillLeft(yEdge, canonicalXDir);
        }

        if (std::abs(dy) < Strip::kStripEpsilon && pTop.fY == std::floor(pTop.fY)) {
            return;
        }

        int32_t startY = static_cast<int32_t>(std::floor(pTop.fY));
        int32_t endY = static_cast<int32_t>(std::ceil(pBot.fY));

        auto rows = this->findRowIntersections(pTop, pBot, dxdy, startY, endY);
        LineStepParams params = this->computeLineStepParams(pTop, pBot, dx, dy);

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

            float t_val = params.tBase + params.stepY * static_cast<float>(row) +
                          params.stepX * static_cast<float>(xStart);

            bool is_startY = (row == startY);
            bool is_endY_minus_1 = (row == endY - 1);
            bool pixelTopTouch = (p_top_y != std::floor(p_top_y));
            bool default_bumped = pixelTopTouch && params.xDir;

            int canonical_start_x = params.xDir ? xStart : xEnd;
            int canonical_end_x = params.xDir ? xEnd : xStart;

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
                int u = std::clamp(static_cast<int>(std::floor(t_val * Strip::kLutMaskWidthF)),
                                   0,
                                   Strip::kLutMaskWidthExcl);
                SkASSERT(params.v * Strip::kLutMaskWidth + u < fMaskLut.size());
                uint8_t maskVal = fMaskLut[params.v * Strip::kLutMaskWidth + u];

                uint8_t edgeMask = 0xff;
                if (column == canonical_start_x) edgeMask &= start_mask_val;
                if (column == canonical_end_x) edgeMask &= end_mask_val;
                maskVal &= edgeMask;

                bool bumped =
                        (is_startY && column == canonical_start_x) ? start_bumped : default_bumped;
                for (int32_t k = 0; k < Strip::kNumSubSamples; ++k) {
                    int32_t maskBit = (maskVal & (1 << k)) ? 1 : 0;
                    if constexpr (kIsWinding) {
                        maskBit -= bumped;
                        if (maskBit != 0) {
                            if (canonicalYDir) {
                                fCoverageMask[row][column][k] += maskBit;
                            } else {
                                fCoverageMask[row][column][k] -= maskBit;
                            }
                        }
                    } else {
                        fCoverageMask[row][column][k] ^= (maskBit ^ bumped);
                    }
                }

                t_val += params.stepX;
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
                    for (int32_t k = 0; k < Strip::kNumSubSamples; ++k) {
                        if constexpr (kIsWinding) {
                            fCoverageMask[row][column][k] += val;
                        } else {
                            fCoverageMask[row][column][k] ^= val;
                        }
                    }
                }
            }
        }
    }

private:
    static constexpr int32_t kTilePixelCount = kTileWidth * kTileHeight;

    struct LineStepParams {
        int v;
        float stepX;
        float stepY;
        float tBase;
        bool xDir;
    };

    SK_ALWAYS_INLINE uint8_t* reserveAlphaBuffer() {
        if (this->fAlphaBuf->size() + kTilePixelCount > this->fAlphaBuf->capacity()) {
            constexpr size_t kChunkSize = 4 * kTilePixelCount;
            this->fAlphaBuf->reserve(this->fAlphaBuf->capacity() + kChunkSize);
        }
        return this->fAlphaBuf->append(kTilePixelCount);
    }

    [[maybe_unused]]
    SK_ALWAYS_INLINE void observePixel(int32_t row, int32_t column) {
        uint8_t exactMask = 0;
        for (int32_t k = 0; k < Strip::kNumSubSamples; ++k) {
            if (ShouldFill(fCoverageMask[row][column][k])) {
                exactMask |= (1 << k);
            }
        }
        if (fIsInverse) {
            exactMask = ~exactMask & ((1 << Strip::kNumSubSamples) - 1);
        }
        fObserver(exactMask);
    }

    SK_ALWAYS_INLINE void processPixel(int32_t row,
                                       int32_t column,
                                       uint8_t* tileAlphaBase,
                                       int& localWriteIdx) {
        int32_t activeSamples = 0;

        for (int32_t k = 0; k < Strip::kNumSubSamples; ++k) {
            if (ShouldFill(fCoverageMask[row][column][k])) {
                activeSamples++;
            }
        }
        if (fIsInverse) {
            activeSamples = Strip::kNumSubSamples - activeSamples;
        }

#if defined(GPU_TEST_UTILS)
        if (fObserver) {
            this->observePixel(row, column);
        }
#endif

        uint8_t alpha = static_cast<uint8_t>((activeSamples * 255 + 4) / 8);
        tileAlphaBase[localWriteIdx++] = alpha;
    }

    SK_ALWAYS_INLINE void fillLeft(float yEdge, bool canonicalXDir) {
        int32_t startY = static_cast<int32_t>(std::ceil(yEdge));

        int32_t val;
        if constexpr (kIsWinding) {
            val = canonicalXDir ? -1 : 1;
        } else {
            val = 1;
        }

        for (int32_t row = startY; row < kTileHeight; ++row) {
            for (int32_t column = 0; column < kTileWidth; ++column) {
                for (int32_t k = 0; k < Strip::kNumSubSamples; ++k) {
                    if constexpr (kIsWinding) {
                        fCoverageMask[row][column][k] += val;
                    } else {
                        fCoverageMask[row][column][k] ^= val;
                    }
                }
            }
        }
    }

    SK_ALWAYS_INLINE std::array<SkPoint, kTileHeight + 1> findRowIntersections(
            SkPoint pTop, SkPoint pBot, float dxdy, int32_t startY, int32_t endY) {
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
        return rows;
    }

    SK_ALWAYS_INLINE LineStepParams computeLineStepParams(SkPoint pTop,
                                                          SkPoint pBot,
                                                          float dx,
                                                          float dy) const {
        float nx = dy;
        float ny = -dx;
        if (nx < 0.0f) {
            nx = -nx;
            ny = -ny;
        }
        float D = nx + std::abs(ny);
        float invD = (D < Strip::kStripEpsilon) ? 0.0f : 1.0f / D;
        bool isPos = ny <= 0.0f;
        float cLocal = nx * pTop.fX + ny * pTop.fY;
        float s = std::abs(ny) * invD;
        int vMod = std::clamp(static_cast<int>(std::floor(s * (Strip::kLutMaskHeight / 2))),
                              0,
                              (Strip::kLutMaskHeight / 2) - 1);
        int v = isPos ? (vMod + Strip::kLutMaskHeight / 2) : vMod;
        float stepX = nx * invD;
        float stepY = ny * invD;
        float tBase = ((isPos ? nx : D) - cLocal) * invD;
        bool xDir = pTop.fX <= pBot.fX;

        return {v, stepX, stepY, tBase, xDir};
    }

    // Holds the raw accumulated winding, per subsample per pixel, for the spatial tile currently
    // being processed.
    int16_t fCoverageMask[kTileHeight][kTileWidth][Strip::kNumSubSamples];
    // Integer winding at the top left corner of the tile.
    int32_t fCoarseWinding = 0;
    // Buffer to accumulate the generated strips.
    SkTDArray<Strip>* fStripBuf;
    // Buffer to accumulate the generated alpha values.
    SkTDArray<uint8_t>* fAlphaBuf;
    // Toggles inverse winding behavior.
    bool fIsInverse;
    // Reference to the polyline container which holds the flattened paths
    const Polyline& fPolyline;
    // Reference to the slope intercept lookup table used to evaluate subsample winding.
    const SkTDArray<uint8_t>& fMaskLut;
    // Alpha index, independent of the contents of the alpha buffer. Separated for atlasing.
    int32_t fLocalAlphaIdx;
#if defined(GPU_TEST_UTILS)
    // Hook, used to test the correctness of the coverage generation.
    MsaaExactMaskObserver fObserver;
#endif
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_StripProcessorScalar_DEFINED
