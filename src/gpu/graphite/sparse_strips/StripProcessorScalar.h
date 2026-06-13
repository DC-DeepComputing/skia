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
                this->processPixel(row, column, tileAlphaBase, localWriteIdx++);
            }
        }
        fLocalAlphaIdx += kTilePixelCount;
    }

    // The core function of make strips. It takes the line, coarse winding, and intersection mask
    // and converts it to subsample winding. There are two core techniques that make MSAA
    // calculation on the CPU feasible:
    //
    //  1. LUT Lookup (Sub-pixel Coverage)
    //     Evaluating the line equation to determine winding at 8 or 16 subsample locations per
    //     pixel is prohibitively expensive on the CPU. Instead, a fixed set of subsample coverage
    //     patterns are precomputed into a look-up table.
    //
    //  2) Hierarchical Winding: A naive scaline renderer at 8xMSAA and a 4x4 tile would require
    //     carrying 4 x 8 = 32 scanlines across the tiles. Instead, the winding is calculated
    //     *hierarchically*: the "coarse winding" is only carried at the top left corner of the tile
    //     and each subsample reconstructs its winding from that point using the intersected line.
    //
    //                               2x2 Tile 8xMSAA
    //  +----------------+----------------+    C +----------------+----------------+
    //  |-o--------------|-o->            |    | | o              | o              |
    //  |------------o---|------------o-> |    v |            o   |            o   |
    //  |--------o-------|--------o->     |    | |        o       |        o       |
    //  |---------------o|---------------o|    v |               o|               o|
    //  |---o------------|---o->          |    | |   o            |   o            |
    //  |---------o------|---------o->    |    v |         o      |         o      |
    //  |-------------o--|-------------o->|    | |             o  |             o  |
    //  |-----o----------|-----o->        |    v |     o          |     o          |
    //  +----------------+----------------+    | +----------------+----------------+
    //  |-o--------------|-o->            |    v | o              | o              |
    //  |------------o---|------------o-> |    | |            o   |            o   |
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
    //     top edge of a pixel dictates that all pixels to the right of the crossing pixel have
    //     their winding toggled.
    //
    //                            Crossed top at X = 0
    //                             |
    //                             V
    //                      +------X---------+----------------+
    //                      | o    *         | #              |
    //                      |      *     o   |            #   |  <--Pixels X > 0 filled
    //                      |      * o       |        #       |
    //                      |      *        o|               #|
    //                      |   o  *         |   #            |
    //                      |      *  o      |         #      |
    //                      |      *      o  |             #  |
    //                      |     o*         |     #          |
    //                      +------X---------+----------------+
    //                      | o    *         | #              |
    //                      |      *    o    |            #   |
    //                      |      * o       |        #       |
    //                      |      *        o|               #|
    //                      |   o  *         |   #            |
    //                      |      *  o      |         #      |
    //                      |      *      o  |             #  |
    //                      |     o*         |     #          |
    //                      +------X---------+----------------+
    //
    //  2. Fill Left
    //     Because lines do not necessarily span the entire height of the tile, the "Crossing Top"
    //     rule alone is insufficient as it does not consider winding effects below the line. So it
    //     is supplemented with the "Fill Left" rule: if a line segment crosses the left boundary
    //     of the tile, we toggle all pixels below that edge.
    //
    //                      +---------X------+----------------+
    //                      | o     *        | o              |
    //                      |     *      o   |            o   |
    //                      |   *    o       |        o       |
    //                      | *             o|               o|
    // Crosses left     --> X   o            |   o            |
    // inside pixel Y = 0   |         o      |         o      |
    //                      |             o  |             o  |
    //                      |     o          |     o          |
    //                      +----------------+----------------+
    //                      | #              | #              |
    //                      |            #   |            #   |
    //                      |        #       |        #       |
    //                      |               #|               #|
    //                      |   #            |   #            | <-- Pixels Y > 0 filled
    //                      |         #      |         #      |
    //                      |             #  |             #  |
    //                      |     #          |    #           |
    //                      +----------------+----------------+
    //
    //  ------------------------------------------------------------------------------------------
    // Fine Phase:
    //  0. Per Row Intersections:
    //     The intersection of the line and each row edge is found, using the tile-edge
    //     intersections as the first and last entry.
    //
    //                        Already given by ClipToTile
    //                         |
    //                         V
    //                      +--X-------------+----------------+
    //                      | o *            | o              |
    //                      |    *       o   |            o   |
    //                      |      * o       |        o       |
    //                      |       *       o|               o|
    //                      |   o    *       |   o            |
    //                      |         o      |         o      |
    //                      |           *  o |             o  |
    //                      |     o      *   |     o          |
    //                      +-------------X--+----------------+
    //                      | o            * | o              |
    //                      |            o  *|            o   |
    //                      |        o       *        o       |
    //                      |               o|*              o|
    //                      |   o            | * o            |
    //                      |         o      |  *      o      |
    //                      |             o  |   *         o  |
    //                      |     o          |    o           |
    //                      +----------------+-----X----------+
    //                                             ^
    //                                             |
    //                                             Already given by ClipToTile
    //
    //  1. LUT Lookup
    //     The LUT is indexed by a normalized slope and parametric `t`. Because the line is
    //     invariant across the pixels of the tiles, we perform an initial setup---
    //     calculateLineStepParams()---before processing pixels. This produces the normalized
    //     slope and initial paramtric `t` (tBase) value. Instead of evaluating the line equation
    //     at each pixel and recalculating `t`, we instead also calculate paramtric step values
    //     that map to traversing right and down, and dda step tBase. See calculateLineStepParams()
    //     for more detail
    //
    //                      +--X-------------+----------------+
    //                      | # *            | o              |
    //                      |    *       o   |            o   |
    //                      |      * o       |        o       |
    //                      |       *       o|               o|
    //                      |   #    *       |   o            |
    //                      |         #      |         o      |
    //                      |           *  o |             o  |
    //                      |     #      *   |     o          |
    //                      +-------------X--+----------------+
    //                      | #            * | o              |
    //                      |            #  *|            o   |
    //                      |        #       *        o       |
    //                      |               #|*              o|
    //                      |   #            | * o            |
    //                      |         #      |  *      o      |
    //                      |             #  |   *         o  |
    //                      |     #          |    #           |
    //                      +----------------+-----X----------+
    //
    //  2. Truncation
    //     The LUT assumes that the line contributes winding to the *entire* pixel. This fails when
    //     a line terminates:
    //          A) Inside the pixel.
    //          B) On a horizontal pixel edge.
    //     Because paths are watertight, a sibling line always shares this endpoint. To prevent
    //     double-counting winding, both lines must truncate their masks above or below the
    //     endpoint's horizontal half-plane. Pixel traversal (like tile traversal) is vertically
    //     exclusive but horizontally inclusive. Therefore, a line ending exactly on a pixel's left
    //     edge still generates a LUT mask, undergoes truncation, and combines cleanly with its
    //     sibling.
    //
    //                             Line terminates inside pixel
    //                              |
    //                              V
    //                      +--X-------------+----------------+
    //                      | # *            | o              |
    //                      |    *       o   |            o   |
    //                      |      * o       |        o       |
    //                      |       *       o|               o|
    // Mask truncated    -->| - # -  *-   -  |   o            |
    // around Y = ~0.5      |         o      |         o      |
    //                      |              o |             o  |
    //                      |     o          |     o          |
    //                      +----------------+----------------+
    //                      | o              | o              |
    //                      |            o   |            o   |
    //                      |        o       |        o       |
    //                      |               o|               o|
    //                      |   o            |   o            |
    //                      |         o      |         o      |
    //                      |             o  |             o  |
    //                      |     o          |    o           |
    //                      +----------------+----------------+
    //
    //     For pixel-aligned left-edge touches (e.g., Y = 1.0):
    //          A) Bottom endpoint: Vertical exclusivity means processing stops *before*
    //             row Y = 1. Its winding contribution comes entirely from fillLeft().
    //          B) Top endpoint: No truncation is needed; rule (A) guarantees no sibling
    //             pixel contribution exists to double-count.
    //          NOTE: This logic assumes integer Y-coordinates are directly representable
    //          in f32 (which is guaranteed up to 16,777,216, or 2^24).
    //
    //     For fractional left-edge touches (e.g., Y = 1.15), the current tile cannot determine
    //     if the line continues into the left tile or terminates on the edge:
    //          A) Continues left: The mask spans the pixel and should NOT be truncated.
    //          B) Terminates on edge: The mask *should* be truncated.
    //
    //     Due to inclusive horizontal traversal, the sibling line will always process a tile
    //     at this exact location. Even if the sibling lies entirely in the left tile, it generates
    //     an empty "single-touch" tile here solely to provide fillLeft() and this single pixel's
    //     coverage. (Note: ClipToTile clamps grazing touches into 0-width vertical lines, which
    //     guarantees sortedXDir = true).
    //
    //     Because we cannot resolve the ambiguity locally, we *never* truncate left-edge touches.
    //     Instead, we allow the masks to overlap and rely on `startBumping` (see below) to
    //     destructively cancel out the excess coverage with the sibling.
    //
    //          Example here
    //
    //  3. Winding and Sidedness
    //     The mask returned by the LUT is entirely pixel-local; it only represents which subsamples
    //     fall on the "positive" side of the line equation within the pixel. Thus mask returned
    //     by the LUT must be modified for "bumped"---which half of the mask represents
    //     interior/exterior the geometry.
    //
    //     The contribution of a pixel is given by `canonicalYDir * (LUT - sidedness)` where:
    //          -- LUT is the value of a single subsample location e.g. 0/1
    //          -- canonicalYDir determines the *direction* of the winding contribution, e.g. 1/-1.
    //             Note that in the case of evenOdd fill rule, this is unnecessary, as the result of
    //             XOR is equivalent regardless of direction.
    //          -- bumped is application of the winding rule at a pixel level, and is the
    //             determinant of "sidedness." It is given by `pTop.fY != floor(pTop.fY) && sortedXDir`.
    //             essentially, it determines whether
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
        // TODO (thomsmit): are ceil(), which is used to find the starting point of the fillLeft(),
        // and floor(), which is used to determine pixelTopTouch, guaranteed to be the exact same
        // value if the leftIntersectionY is perfectly pixel aligned? Does casting to int guarantee
        // this?
        if (tile.hasLeftIntersection()) {
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

            // Compute the initial normalized translation parameter `tVal` at the starting pixel
            // (xStart, row) of the span using the linear projection formula:
            // t(x, y) = tBase + stepY * y + stepX * x
            float tVal = params.fTBase + params.fStepY * static_cast<float>(row) +
                         params.fStepX * static_cast<float>(xStart);

            bool isStartY = (row == startY);
            bool isEndYMinus1 = (row == endY - 1);

            // Check if the geometry touches the absolute top edge of the current pixel.
            bool pixelTopTouch = (pTopY != std::floor(pTopY));
            bool defaultBumped = pixelTopTouch && params.fSortedXDir;

            int canonicalStartX = params.fSortedXDir ? xStart : xEnd;
            int canonicalEndX = params.fSortedXDir ? xEnd : xStart;

            // The Look-Up Table (LUT) assumes that lines always fully traverse a pixel. However, if
            // the line terminates inside a pixel, start and end masks must be used to truncate the
            // line's winding contribution.
            uint8_t startMaskVal = 0xff;
            bool startBumped = defaultBumped;
            if (isStartY) {
                // TODO (thomsmit): investigate if it is possible to determine startBumped by
                // checking if the left edge touch is the raw line end point? This is probably the
                // most robust way to do this.
                bool touchLeft = pTopX == 0.0f;
                startBumped = pixelTopTouch && touchLeft;
                if (!touchLeft) {
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
                // on the line's calculated trajectory (u, v). `u` is the column index, obtained by
                // scaling the translation parameter `tVal` (which ranges from 0.0 to 1.0) to the
                // LUT's width (64) and flooring the result.
                int u = std::clamp(static_cast<int>(std::floor(tVal * Strip::kLutMaskWidthF)),
                                   0,
                                   Strip::kLutMaskWidthExcl);
                SkASSERT(params.fLutRow * Strip::kLutMaskWidth + u < fMaskLut.size());
                uint8_t maskVal = fMaskLut[params.fLutRow * Strip::kLutMaskWidth + u];

                // Apply the truncation masks if we are at the geometric ends of the segment.
                uint8_t edgeMask = 0xff;
                if (column == canonicalStartX) edgeMask &= startMaskVal;
                if (column == canonicalEndX) edgeMask &= endMaskVal;
                maskVal &= edgeMask;

                // Adjust the mask to avoid double counting the winding with the backdrop.
                // The lutMask mayAlready be in the correct configuration, requiring no adjustment.
                // The logic for the first pixel is slightly different.
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

                // Step to the next column: since the X coordinate increases by 1.0, incrementally
                // add stepX to the translation parameter `tVal` (DDA).
                tVal += params.fStepX;
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

    struct LineStepParams {
        int fLutRow;
        float fStepX;
        float fStepY;
        float fTBase;
        bool fSortedXDir;
    };

    SK_ALWAYS_INLINE uint8_t* reserveAlphaBuffer() {
        if (fAlphaBuf->size() + kTilePixelCount > fAlphaBuf->capacity()) {
            constexpr size_t kChunkSize = 4 * kTilePixelCount;
            fAlphaBuf->reserve(fAlphaBuf->capacity() + kChunkSize);
        }
        return fAlphaBuf->append(kTilePixelCount);
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
    // check the winding against the fill rule. In the SIMD version, this is done more effeciently.
    // Inverse fills, are handled by inverting the coverage.
    SK_ALWAYS_INLINE void processPixel(int32_t row,
                                       int32_t column,
                                       uint8_t* tileAlphaBase,
                                       int32_t localWriteIdx) {
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

        uint8_t alpha = static_cast<uint8_t>((activeSamples * 255 + (Strip::kNumSubSamples / 2)) /
                        Strip::kNumSubSamples);
        tileAlphaBase[localWriteIdx] = alpha;
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

    /*
    * Computes the LineStepParams used to project integer pixel coordinates into the continuous
    * normalized parameter space (s, t) of the precomputed MSAA look-up table:
    *
    * 0. DDA
    * In theory, to find the LUT column 't' for any pixel (X, Y), we could evaluate the full line
    * equation from scratch. In practice, this is slow. Because the line is straight, the change 't'
    * as we move 1 pixel right (X + 1) or 1 pixel down (Y + 1) is constant. This allows us to use a
    * approach: t(X, Y) = tBase + (X * stepX) + (Y * stepY). Thus, we calculate `t` once upfront,
    * and step from there.
    *
    * 1. The Manhattan Squash (Preventing the Vertical Singularity):
    * The original mathematical model squashes infinite slopes into a bounded [0, 1] parameter
    * using the curve: s = 1 / (m + 1), where m = |dy/dx|.
    * * However, calculating m directly causes a fatal divide-by-zero singularity for perfectly
    * vertical lines (dx = 0). If we eliminate the complex fraction by multiplying the numerator
    * and denominator by |dx|, the equation mathematically resolves to:
    * s = |dx| / (|dy| + |dx|)
    * * This is algebraically identical, but perfectly safe from singularities. Notice that the
    * denominator is exactly the Manhattan distance of the line's rightward-pointing normal vector:
    * D = normalX + |normalY|
    * * Thus, we compute the exact same 's' parameter using purely additive, branchless math:
    * s = |normalY| / D
    *
    * 2. DDA Stepping (stepX and stepY):
    * These represent the partial derivatives of the coverage equation.
    * If we expand our continuous parameterization: (x - (1 - t))(1 - s) - (y - t)s >= 0
    * - Moving 1 pixel right (x + 1) changes the value by exactly (1 - s).
    * Mathematically: 1 - (|normalY| / D) = normalX / D.
    * Thus, stepX = normalX * invManhattanDistance.
    * - Moving 1 pixel down (y + 1) changes the value by exactly -s.
    * Mathematically: normalY / D. (If dy > 0, normalY is negative, yielding -s).
    * Thus, stepY = normalY * invManhattanDistance.
    *
    * 3. The Origin Seed (tBase):
    * To use incremental stepping, we need a starting point. `tBase` calculates the exact
    * normalized translation 't' at the global coordinate origin (0, 0).
    * - The unnormalized line offset at the origin is `-localLineConstant`.
    * - For positive slopes, we shift this by `normalX` to align the phase with the LUT.
    * - For negative slopes, because the LUT generation mathematically flips the Y-axis
    * (y = 1.0 - y) to reuse the positive-slope data, the Y-intercept physically shifts.
    * We compensate by phase-shifting the offset by the full Manhattan distance.
    * Finally, this offset is multiplied by invManhattanDistance to map it into the [0.0, 1.0]
    * bounds of the LUT's horizontal 'u' axis.
    *
    * Note: While these parameters could be cached per line, empirical profiling shows it is faster
    * to calculate them on-demand in modern rendering pipelines to avoid memory bandwidth bottlenecks.
    */
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
        // For lines with a negative slope, the LUT vertically flips the coordinates (y -> 1 - y)
        // to reuse the positive-slope equation. Substituting y = 1 - y' into the line equation
        // changes the translation offset from -localLineConstant to (manhattanDistance -
        // localLineConstant). Thus, tBase uses `normalX` for positive slopes and
        // `manhattanDistance` for negative slopes.
        float tBase =
                ((hasPositiveSlope ? normalX : manhattanDistance) - localLineConstant) *
                invManhattanDistance;

        // NOTE: This is NOT the canonicalXDir, this is the direction of the top to bottom points
        bool sortedXDir = pTop.fX <= pBot.fX;

        return {lutRow, stepX, stepY, tBase, sortedXDir};
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
