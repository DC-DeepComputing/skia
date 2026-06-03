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

    // Once all tiles at the same geometric location have been combined, the coverage mask must be
    // converted from winding to alpha values consumable by the fragment shader.
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
    //     Evaluating the line equation to determine winding at 8 or 16 subsample locations per
    //     pixel is prohibitively expensive on the CPU. Instead, a fixed set of subsample coverage
    //     patterns are precomputed into a look-up table. As each row is processed, the line's
    //     trajectory relative to the current pixel is used to index into the LUT.
    //
    //  2) Hierarchical Winding: A naive scaline renderer at 8xMSAA and a 4x4 tile would require
    //     carrying 4 x 8 = 32 scanlines across the tiles. Instead, the winding is calculated
    //     *hierarchically*: the "coarse winding" is only carried at the top left corner of the tile
    //     and each subsample reconstructs its winding from that point using the intersected line.
    //
    //                               2x2 Tile 8xMSAA
    //  +----------------+----------------+    C +----------------+----------------+
    //  |-o--------------|-o->            |    | | o              | o              |
    //  |-----------o----|-----------o->  |    v |           o    |           o    |
    //  |--------o-------|--------o->     |    | |        o       |        o       |
    //  |---------------o|---------------o|    v |               o|               o|
    //  |---o------------|---o->          |    | |   o            |   o            |
    //  |---------o------|---------o->    |    v |         o      |         o      |
    //  |-------------o--|-------------o->|    | |             o  |             o  |
    //  |-----o----------|-----o->        |    v |     o          |     o          |
    //  +----------------+----------------+    | +----------------+----------------+
    //  |-o--------------|-o->            |    v | o              | o              |
    //  |-----------o----|-----------o->  |    | |           o    |           o    |
    //  |--------o-------|--------o->     |    v |        o       |        o       |
    //  |---------------o|---------------o|    | |               o|               o|
    //  |---o------------|---o->          |    v |   o            |   o            |
    //  |---------o------|---------o->    |    | |         o      |         o      |
    //  |-------------o--|-------------o->|    v |             o  |             o  |
    //  |-----o----------|-----o->        |    └-|---->o          |     o          |
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
    //  0. Coarse Winding
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

        // Accumulate the coarse winding for the next spatial tile, fCoverageMask has already been
        // seeded during tile state transition.
        uint32_t windingBit = tile.coarseWinding() ? 1 : 0;
        if constexpr (kIsWinding) {
            fCoarseWinding += (canonicalYDir ? 1 : -1) * static_cast<int32_t>(windingBit);
        } else {
            fCoarseWinding ^= static_cast<int32_t>(windingBit);
        }

        // TODO (thomsmit): remove once culling lands
        // Cull lines that exist entirely to the left of the tile.
        float rightEdge = canonicalXDir ? line.p1.fX : line.p0.fX;
        if (rightEdge < 0.0f) {
            return;
        }

        float dx = line.p1.fX - line.p0.fX;
        float dy = line.p1.fY - line.p0.fY;
        float idx = (std::abs(dx) <= Strip::kStripEpsilon) ? 0.0f : 1.0f / dx;
        float idy = (std::abs(dy) <= Strip::kStripEpsilon) ? 0.0f : 1.0f / dy;
        float dxdy = dx * idy;

        std::array<float, 4> derivs = {dx, dy, idx, idy};

        auto clipped = Tile::ClipToTile<kTileWidth, kTileHeight>(
                line, tileBounds, derivs, tile.intersectionMask(), canonicalXDir, canonicalYDir);
        SkPoint pTop = clipped[0];
        SkPoint pBot = clipped[1];

        // Fill left
        if (tile.left()) {
            float yEdge = pTop.fX < pBot.fX ? pTop.fY : pBot.fY;
            this->fillLeft(yEdge, canonicalXDir);
        }

        // Ignore perfectly horizontal lines that lie exactly on pixel boundaries, as their winding
        // contribution is accounted for by fillLeft()
        if (std::abs(dy) < Strip::kStripEpsilon && pTop.fY == std::floor(pTop.fY)) {
            return;
        }

        // Use the tile intersection points to get the vertical span in (integer) pixels
        int32_t startY = static_cast<int32_t>(std::floor(pTop.fY));
        int32_t endY = static_cast<int32_t>(std::ceil(pBot.fY));

        // Find x-intersection for each pixel row; rows which have no intersection will return NaN
        auto rows = this->findRowIntersections(pTop, pBot, dxdy, startY, endY);
        LineStepParams params = this->computeLineStepParams(pTop, pBot, dx, dy);

        // For each intersected pixel row, determine the "fine" coverage.
        for (int32_t row = startY; row < endY; ++row) {
            float pTopX = rows[row].fX;
            float pTopY = rows[row].fY;
            float pBotX = rows[row + 1].fX;
            float pBotY = rows[row + 1].fY;

            // This should never happen, and the SIMD implementation simply asserts this.
            if (std::isnan(pTopX) || std::isnan(pBotX)) {
                continue;
            }

            // Determine horizontal pixel span for this specific row.
            float xMin = std::fmin(pTopX, pBotX);
            float xMax = std::fmax(pTopX, pBotX);

            int32_t xStart = std::clamp(static_cast<int32_t>(std::floor(xMin)), 0, kTileWidth - 1);
            int32_t xEnd = std::clamp(static_cast<int32_t>(std::floor(xMax)), 0, kTileWidth - 1);

            float tVal = params.tBase + params.stepY * static_cast<float>(row) +
                         params.stepX * static_cast<float>(xStart);

            bool isStartY = (row == startY);
            bool isEndYMinus1 = (row == endY - 1);

            // Check if the geometry touches the absolute top edge of the current pixel.
            bool pixelTopTouch = (pTopY != std::floor(pTopY));
            bool defaultBumped = pixelTopTouch && params.isLeftToRight;

            int canonicalStartX = params.isLeftToRight ? xStart : xEnd;
            int canonicalEndX = params.isLeftToRight ? xEnd : xStart;

            // The Look-Up Table (LUT) assumes lines are infinite (always fully traverses a pixel).
            // However, if line ends inside a pixel, start and end masks must be used to truncate
            // the line's winding contribution.
            uint8_t startMaskVal = 0xff;
            bool startBumped = defaultBumped;
            if (isStartY) {
                startBumped = (pTopX == 0.0f && pixelTopTouch);
                if (!startBumped) {
                    // Truncate subsamples above the segment's starting y.
                    uint32_t shift = static_cast<uint32_t>(
                            std::round(8.0f * (pTopY - static_cast<float>(row))));
                    startMaskVal = static_cast<uint8_t>(0xff << shift);
                }
            }

            uint8_t endMaskVal = 0xff;
            if (isEndYMinus1 && pBotX != 0.0f) {
                // Truncate subsamples below the segment's ending y.
                uint32_t shift = static_cast<uint32_t>(
                        std::round(8.0f * (pBotY - static_cast<float>(row))));
                endMaskVal = static_cast<uint8_t>(~(0xff << shift));
            }

            // Iterate left-to-right through the pixels in the current row.
            for (int32_t column = xStart; column <= xEnd; ++column) {
                // Fetch the subsample coverage mask from the pre-computed Look-Up Table (LUT) based
                // on the line's calculated trajectory (u, v).
                int u = std::clamp(static_cast<int>(std::floor(tVal * Strip::kLutMaskWidthF)),
                                   0,
                                   Strip::kLutMaskWidthExcl);
                SkASSERT(params.lutRow * Strip::kLutMaskWidth + u < fMaskLut.size());
                uint8_t maskVal = fMaskLut[params.lutRow * Strip::kLutMaskWidth + u];

                // Apply the truncation masks if we are at the geometric ends of the segment.
                uint8_t edgeMask = 0xff;
                if (column == canonicalStartX) edgeMask &= startMaskVal;
                if (column == canonicalEndX) edgeMask &= endMaskVal;
                maskVal &= edgeMask;

                // Adjust the mask to avoid double counting the winding with the backdrop.
                bool bumped =
                        (isStartY && column == canonicalStartX) ? startBumped : defaultBumped;

                for (int32_t k = 0; k < Strip::kNumSubSamples; ++k) {
                    // Extract the 1 or 0 from the LUT for this specific sub-sample.
                    int32_t maskBit = (maskVal & (1 << k)) ? 1 : 0;

                    if constexpr (kIsWinding) {
                        // Subtract the `bumped` state. This aligns the LUT's pixel-center logic
                        // with the Hierarchical Winding's top-left anchor point logic, preventing
                        // double-counting or skipped winding at pixel boundaries.
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

                tVal += params.stepX;
            }

            // CrossingTop
            bool crossedTop = (row > startY) || (pTopY == std::floor(pTopY));
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

    // Holds the parameters needed to evaluate the LUT index (u, v) as we traverse the tile.
    struct LineStepParams {
        int lutRow;
        float stepX;
        float stepY;
        float tBase;
        bool isLeftToRight;
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

    // Computes the LineStepParams used to project pixel coordinates into the normalized
    // parameter space (s, t) of the precomputed lookup table. (See MSAA_LUT.h for more context.)
    //
    // 1. Orthogonal Normal: Computes a normal (normalX, normalY) orthogonal to the line vector
    //    (dx, dy). By ensuring normalX >= 0, the normal always points rightwards.
    // 2. Normalized Slope (s): Uses the Manhattan distance D = normalX + |normalY| = |dy| + |dx| to
    //    compute the normalized slope parameter s = |normalY| / D. This maps slopes in
    //    [0, infinity) to the range [0.0, 1.0], which determines the LUT row 'lutRow' (partitioned
    //    by the slope's sign).
    // 3. Step Increments: stepX and stepY represent the change in the translation parameter 't'
    //    per pixel stepped along X and Y, scaled by 1/D.
    // 4. Normalized Translation (t): tBase represents the translation parameter 't' at the origin
    //    (0, 0), incorporating the line equation constant localLineConstant and phase-shifted by
    //    normalX or D depending on the sign of normalY (hasPositiveSlope) to map exactly to the
    //    LUT's horizontal index 'u'.
    SK_ALWAYS_INLINE LineStepParams computeLineStepParams(SkPoint pTop,
                                                          SkPoint pBot,
                                                          float dx,
                                                          float dy) const {
        float normalX = dy;
        float normalY = -dx;
        if (normalX < 0.0f) {
            normalX = -normalX;
            normalY = -normalY;
        }
        float manhattanDistance = normalX + std::abs(normalY);
        float invManhattanDistance =
                (manhattanDistance < Strip::kStripEpsilon) ? 0.0f : 1.0f / manhattanDistance;
        bool hasPositiveSlope = normalY <= 0.0f;
        float localLineConstant = normalX * pTop.fX + normalY * pTop.fY;
        float normalizedSlope = std::abs(normalY) * invManhattanDistance;
        int lutRowOffset = std::clamp(
                static_cast<int>(std::floor(normalizedSlope * (Strip::kLutMaskHeight / 2))),
                0,
                (Strip::kLutMaskHeight / 2) - 1);
        int lutRow = hasPositiveSlope ? (lutRowOffset + Strip::kLutMaskHeight / 2) : lutRowOffset;
        float stepX = normalX * invManhattanDistance;
        float stepY = normalY * invManhattanDistance;
        float tBase =
                ((hasPositiveSlope ? normalX : manhattanDistance) - localLineConstant) *
                invManhattanDistance;
        bool isLeftToRight = pTop.fX <= pBot.fX;

        return {lutRow, stepX, stepY, tBase, isLeftToRight};
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
