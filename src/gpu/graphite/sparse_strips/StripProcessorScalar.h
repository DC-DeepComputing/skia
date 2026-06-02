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

    // Once all tiles at the same geometric location have been combined, we need to convert the
    // accumulated winding for each pixel into alpha values consumable by the fragment shader.
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

    // The core function of make strips. It takes the line, coarse winding, and intersection mask
    // and converts it to subsample winding. There are two core techniques that make MSAA
    // calculation on the CPU feasible:
    //  1. LUT Lookup (Sub-pixel Coverage)
    //     Evaluating the line equation to determine winding at 8 or 16 sub-sample locations per
    //     pixel is prohibitively expensive on the CPU. Instead, we precompute a fixed set of
    //     sub-pixel coverage patterns into a look-up table. As we step horizontally across a row,
    //     we use the line's trajectory relative to the current pixel to index into the LUT.
    //
    //  2) Hierarchical Winding: A naive scaline renderer would require carrying 32 scanlines to
    //     across every subsample location in the tile. Instead, the winding is calculated
    //     *hierarchically*: the winding is only carried at the top left corner of the tile. Then,
    //     we use the crossing from the top-left corner to the subsample location.
    //
    //                               2x2 Tile 8xMSAA
    //  +----------------+----------------+    C +----------------+----------------+
    //  |-o--------------|-o->            |    | | o              | o              |
    //  |-----------o----|----------o->   |    v |           o    |            o   |
    //  |--------o-------|--------o->     |    | |        o       |        o       |
    //  |------o---------|------o->       |    v |               o|               o|
    //  |---o------------|---o->          |    | |   o            |   o            |
    //  |-----o----------|-----o->        |    v |         o      |         o      |
    //  |-------o--------|-------o->      |    | |             o  |             o  |
    //  |---------o------|---------o->    |    v |     o          |     o          |
    //  +----------------+----------------+    | +----------------+----------------+
    //  |-o--------------|-o->            |    v | o              | o              |
    //  |-----------o----|---------o->    |    | |            o   |            o   |
    //  |--------o-------|-------o->      |    v |        o       |        o       |
    //  |------o---------|-----o->        |    | |               o|               o|
    //  |---o------------|---o->          |    v |   o            |   o            |
    //  |-----o----------|-----o->        |    | |         o      |         o      |
    //  |-------o--------|-------o->      |    v |             o  |             o  |
    //  |---------o------|---------o->    |    L-|---------o------|---------o->    |
    //  +----------------+----------------+      +----------------+----------------+
    //           Naive Scanline                             Hierarchical
    //
    //  ------------------------------------------------------------------------------------------
    // Rasterize Line To Tile Conceptual Flow:
    //  Although this is not the logical flow of the function, it is helpful to think of the
    //  function itself as having a "Coarse Phase," which calculates the "aliased winding" for each
    //  pixel which is not intersected by the line, and a "Fine Phase," which calculates
    //  "antialiased winding" for pixels intersected by the line.
    //
    // Coarse Phase:
    //  0. Backdrop Coarse Winding
    //     The overall winding state is tracked hierarchically, anchored at the top-left corner
    //     of the tile. When processing begins for a new spatial tile, this "backdrop" coarse
    //     winding is used to "seed" the initial coverage mask. Tiles rasterized at the same spatial
    //     location continue to accumulate their winding contributions. This accumulation does not
    //     alter the current tile's backdrop, but instead computes the net winding at the top-right
    //     corner of the spatial tile, which in turn becomes the top-left backdrop for the next
    //     tile in the row.
    //
    //  1. Crossing Top & Propagating Right
    //     As the line moves vertically through a tile, standard scanline rules apply: crossing the
    //     top edge of a pixel dictates that all pixels to the right of the crossing have their
    //     winding toggled.
    //
    //     +------*---------+----------------+
    //     | o    *         | #              |
    //     |      *    o    |            #   |
    //     |      * o       |        #       |
    //     |      *        o|               #|
    //     |   o  *         |   #            |
    //     |      *  o      |         #      |
    //     |      *      o  |             #  |
    //     |     o*         |     #          |
    //     +------*---------+----------------+
    //     | o    *         | #              |
    //     |      *    o    |            #   |
    //     |      * o       |        #       |
    //     |      *        o|               #|
    //     |   o  *         |   #            |
    //     |      *  o      |         #      |
    //     |      *      o  |             #  |
    //     |     o*         |     #          |
    //     +------*---------+----------------+
    //
    //  2. Fill Left
    //     Because lines do not necessarily span the entire height of the tile, the "Crossing Top"
    //     rule alone is insufficient as it does not consider winding effects below the line. So it
    //     is supplemented with the "Fill Left" rule: if a line segment crosses the left boundary
    //     of the tile, we toggle all pixels below that edge.
    //
    //     +---------*------+----------------+
    //     | o     *        | o              |
    //     |     *     o    |            o   |
    //     |   *    o       |        o       |
    //     | *             o|               o|
    //     *   o            |   o            |
    //     |         o      |         o      |
    //     |             o  |             o  |
    //     |     o          |     o          |
    //     +----------------+----------------+
    //     | #              | #              |
    //     |            #   |            #   |
    //     |        #       |        #       |
    //     |               #|               #|
    //     |   #            |   #            |
    //     |         #      |         #      |
    //     |             #  |             #  |
    //     |     #          |    #           |
    //     +----------------+----------------+
    //
    // Fine Phase:
    //  1. LUT lookup: Explain how we compute the winding for each pixel using a LUT based on the
    //     slope and intercept of the line relative to the pixel.
    //  2. Hierarchical Winding: (Explain)
    //
    SK_ALWAYS_INLINE void rasterizeLineToTile(const Tile& tile, std::array<float, 4> tileBounds) {
        Line line = fPolyline.getLine(tile.lineIdx());
        bool canonicalXDir = line.p1.fX >= line.p0.fX;
        bool canonicalYDir = line.p1.fY >= line.p0.fY;

        // We track a single "coarse" winding value for the top-left corner of the tile.
        // If the tile indicates it carries a coarse winding from previous processing,
        // we accumulate it here based on the fill rule (Winding vs. EvenOdd).
        uint32_t windingBit = tile.coarseWinding() ? 1 : 0;
        if constexpr (kIsWinding) {
            fCoarseWinding += (canonicalYDir ? 1 : -1) * static_cast<int32_t>(windingBit);
        } else {
            fCoarseWinding ^= static_cast<int32_t>(windingBit);
        }

        // Cull lines that exist entirely to the left of the tile.
        float rightEdge = canonicalXDir ? line.p1.fX : line.p0.fX;
        if (rightEdge < 0.0f) {
            return;
        }

        // Calculate line derivatives (slopes) to step through the tile efficiently.
        float dx = line.p1.fX - line.p0.fX;
        float dy = line.p1.fY - line.p0.fY;
        float idx = (std::abs(dx) <= Strip::kStripEpsilon) ? 0.0f : 1.0f / dx;
        float idy = (std::abs(dy) <= Strip::kStripEpsilon) ? 0.0f : 1.0f / dy;
        float dxdy = dx * idy;

        std::array<float, 4> derivs = {dx, dy, idx, idy};

        // 2. CLIP TO TILE
        // The path segment is infinite or spans multiple tiles; here we restrict
        // it strictly to the bounding box of the current spatial tile.
        auto clipped = Tile::ClipToTile<kTileWidth, kTileHeight>(
                line, tileBounds, derivs, tile.intersectionMask(), canonicalXDir, canonicalYDir);
        SkPoint pTop = clipped[0];
        SkPoint pBot = clipped[1];

        // 3. FILL LEFT
        // If this tile inherits a winding state from an edge that existed entirely to its left
        // (meaning the polygon physically covers this tile horizontally), we propagate that
        // inside/outside state across all pixels below the Y-intersection edge.
        if (tile.left()) {
            float yEdge = pTop.fX < pBot.fX ? pTop.fY : pBot.fY;
            this->fillLeft(yEdge, canonicalXDir);
        }

        // Ignore perfectly horizontal lines that lie exactly on pixel boundaries,
        // as they don't change sub-pixel coverage vertically.
        if (std::abs(dy) < Strip::kStripEpsilon && pTop.fY == std::floor(pTop.fY)) {
            return;
        }

        // Determine the vertical span of pixels this line touches within the tile.
        int32_t startY = static_cast<int32_t>(std::floor(pTop.fY));
        int32_t endY = static_cast<int32_t>(std::ceil(pBot.fY));

        auto rows = this->findRowIntersections(pTop, pBot, dxdy, startY, endY);
        LineStepParams params = this->computeLineStepParams(pTop, pBot, dx, dy);

        // Iterate top-to-bottom through each row of pixels the line intersects.
        for (int32_t row = startY; row < endY; ++row) {
            float p_top_x = rows[row].fX;
            float p_top_y = rows[row].fY;
            float p_bot_x = rows[row + 1].fX;
            float p_bot_y = rows[row + 1].fY;

            if (std::isnan(p_top_x) || std::isnan(p_bot_x)) {
                continue;
            }

            // Determine horizontal pixel span for this specific row.
            float xMin = std::fmin(p_top_x, p_bot_x);
            float xMax = std::fmax(p_top_x, p_bot_x);

            int32_t xStart = std::clamp(static_cast<int32_t>(std::floor(xMin)), 0, kTileWidth - 1);
            int32_t xEnd = std::clamp(static_cast<int32_t>(std::floor(xMax)), 0, kTileWidth - 1);

            float t_val = params.tBase + params.stepY * static_cast<float>(row) +
                          params.stepX * static_cast<float>(xStart);

            bool is_startY = (row == startY);
            bool is_endY_minus_1 = (row == endY - 1);

            // Check if the geometry touches the absolute top edge of the current pixel.
            bool pixelTopTouch = (p_top_y != std::floor(p_top_y));

            // Default bumping state. If it touches the top and moves right,
            // we will need to offset the LUT mask to align with our top-left anchor.
            bool default_bumped = pixelTopTouch && params.xDir;

            int canonical_start_x = params.xDir ? xStart : xEnd;
            int canonical_end_x = params.xDir ? xEnd : xStart;

            // 4. START AND END MASKS
            // The Look-Up Table (LUT) assumes lines are infinite. We use start and end masks
            // to truncate the line into a finite segment if it begins or ends inside the pixel.
            uint8_t start_mask_val = 0xff;
            bool start_bumped = default_bumped;
            if (is_startY) {
                // Determine if we need to bump the very first pixel.
                start_bumped = (p_top_x == 0.0f && pixelTopTouch);
                if (!start_bumped) {
                    // Truncate sub-samples that are vertically "above" the segment's starting Y point.
                    // We bit-shift 0xff to zero out the bits above the fractional start coordinate.
                    uint32_t shift = static_cast<uint32_t>(
                            std::round(8.0f * (p_top_y - static_cast<float>(row))));
                    start_mask_val = static_cast<uint8_t>(0xff << shift);
                }
            }

            uint8_t end_mask_val = 0xff;
            if (is_endY_minus_1 && p_bot_x != 0.0f) {
                // Truncate sub-samples that are vertically "below" the segment's ending Y point
                // by using a bitwise NOT (~) on the shifted mask.
                uint32_t shift = static_cast<uint32_t>(
                        std::round(8.0f * (p_bot_y - static_cast<float>(row))));
                end_mask_val = static_cast<uint8_t>(~(0xff << shift));
            }

            // Only apply these masks to the row where the segment actually starts or ends.
            if (!is_startY) start_mask_val = 0xff;
            if (!is_endY_minus_1) end_mask_val = 0xff;

            // Iterate left-to-right through the pixels in the current row.
            for (int32_t column = xStart; column <= xEnd; ++column) {

                // Fetch the sub-sample coverage mask from the pre-computed Look-Up Table (LUT)
                // based on the line's calculated trajectory (u, v).
                int u = std::clamp(static_cast<int>(std::floor(t_val * Strip::kLutMaskWidthF)),
                                   0,
                                   Strip::kLutMaskWidthExcl);
                SkASSERT(params.v * Strip::kLutMaskWidth + u < fMaskLut.size());
                uint8_t maskVal = fMaskLut[params.v * Strip::kLutMaskWidth + u];

                // Apply the start/end truncation masks if we are at the geometric ends of the segment.
                uint8_t edgeMask = 0xff;
                if (column == canonical_start_x) edgeMask &= start_mask_val;
                if (column == canonical_end_x) edgeMask &= end_mask_val;
                maskVal &= edgeMask;

                // 5. BUMPING
                // Determine if this specific pixel requires a "bump".
                bool bumped =
                        (is_startY && column == canonical_start_x) ? start_bumped : default_bumped;

                // Iterate through every sub-sample (e.g., 8 or 16 depending on MSAA level)
                for (int32_t k = 0; k < Strip::kNumSubSamples; ++k) {
                    // Extract the 1 or 0 from the LUT for this specific sub-sample.
                    int32_t maskBit = (maskVal & (1 << k)) ? 1 : 0;

                    if constexpr (kIsWinding) {
                        // Subtract the `bumped` state. This aligns the LUT's pixel-center
                        // logic with the Hierarchical Winding's top-left anchor point logic,
                        // preventing double-counting or skipped winding at pixel boundaries.
                        maskBit -= bumped;

                        // Accumulate the final mask bit into the tile's total coverage mask.
                        if (maskBit != 0) {
                            if (canonicalYDir) {
                                fCoverageMask[row][column][k] += maskBit;
                            } else {
                                fCoverageMask[row][column][k] -= maskBit;
                            }
                        }
                    } else {
                        // EvenOdd rule just uses XOR.
                        fCoverageMask[row][column][k] ^= (maskBit ^ bumped);
                    }
                }

                t_val += params.stepX;
            }

            // 6. PROPAGATING WINDING RIGHT
            // Standard scanline algorithm rule: A line crossing down a pixel affects the winding
            // of all pixels to its right on that scanline.
            bool crossedTop = (row > startY) || (p_top_y == std::floor(p_top_y));
            if (crossedTop) {
                int32_t val;
                if constexpr (kIsWinding) {
                    val = canonicalYDir ? 1 : -1;
                } else {
                    val = 1;
                }
                // Apply the calculated winding value to all remaining columns to the right.
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

    // For testing, we need to know the fill rule result for each subsample location, not just the
    // summed alpha that we store in the alpha buffer. So on testing builds, we store the exact
    // results in a mask for each pixel in the tile.
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

    // To convert fCoverageMask to alpha, we naively iterate through each subsample location and
    // check the winding against the fill rule. In the SIMD verison, this is done more effeciently.
    // Inverse fills, are handled by inverting the coverage.
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
