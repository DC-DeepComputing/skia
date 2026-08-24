/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_MakeStrips_DEFINED
#define skgpu_graphite_sparse_strips_MakeStrips_DEFINED

#include "include/core/SkPathTypes.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/StripProcessorScalar.h"
#include "src/gpu/graphite/sparse_strips/StripProcessorSimd.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <cstring>
#include <utility>

namespace skgpu::graphite {

/*
 * At this point in the sparse strips pipeline, the path has been stroked, flattened into a
 * polyline, tiled, and sorted. Now, the tiles are consumed by `MakeStrips::*` to directly produce:
 *
 *  1) EndCaps: Runs of rasterized boundary tiles containing fractional per-pixel coverage masks,
 *     which sample from the alpha atlas.
 *
 *  2) WideTiles: Contiguous interior regions with solid 100% full coverage, which are rendered
 *     directly as solid fill rectangles without sampling atlas textures.
 *
 * Two things are required here:
 *
 *  1) Coverage Resolution: Multiple line segments often intersect the exact same spatial tile.
 *     Because the input tiles are generated per-line-segment, the individual winding contributions
 *     must be combined to produce the final coverage mask for that location:
 *
 *     Line 1 (\)       Line 2 (/)        Combined Mask (V)
 *     +----------+     +----------+         +----------+
 *     | \        |     |        / |         | \      / |
 *     |  \       |  +  |       /  |    =    |  \    /  |
 *     |███\      |     |      /███|         |███\  /███|
 *     |████\     |     |     /████|         |████\/████|
 *     +----------+     +----------+         +----------+
 *
 *  2) Direct Geometry Generation (EndCaps & WideTiles): Because the incoming tiles are sorted by y,
 *     then x, runs of contiguous tiles identify boundary edge tiles, while gaps between runs are
 *     checked against the winding fill rule to identify solid interior fill spans.
 *
 *     0           1           2           3           4           5
 *     +-----------+-----------+-----------+-----------+-----------+-----------+
 *     |           |     /     |███████████|███████████|     \     |           |
 *     |  Outside  |   /       |██ Solid ██|██ Solid ██|       \   |  Outside  |
 *     | (No Fill) | / EndCap  |██ Wide  ██|██ Tile  ██| EndCap  \ | (No Fill) |
 *     |           |   (Alpha) |██ (100%)██|██ (100%)██| (Alpha)   |           |
 *     +-----------+-----------+-----------+-----------+-----------+-----------+
 *                 ^           ^                       ^           ^
 *                 |           |                       |           |
 *                 +--EndCap---+<------ WideTile ----->+--EndCap---+
 *                   [x: 1, w: 1]      [x: 2, w: 2]      [x: 4, w: 1]
 *
 * The pipeline emits two geometric primitives directly into their respective render buffers:
 *
 *  1) EndCap (x, y, width, alphaIndex, texPage):
 *     - Coordinates (x, y): The bottom-left pixel coordinate of the first tile in a contiguous run
 *       of boundary/edge tiles.
 *     - Width (width): The total horizontal span in pixels (count of contiguous tiles * kTileWidth).
 *     - Alpha Index (alphaIndex): Offset into the dense alpha atlas page.
 *     - Texture Page (texPage): Associated texture page index in the atlas manager.
 *
 *  2) WideTile (x, y, width):
 *     - Coordinates (x, y): The bottom-left coordinate of the solid fill rectangle.
 *     - Width (width): The total horizontal span in pixels. Rendered with 100% alpha without atlas
 *       sampling.
 *
 * -------------------------------------------------------------------------------------------------
 * Example Row (4x4 sized tile, 16 alphas per tile): Single-Tile EndCaps + Interior WideTile
 * -------------------------------------------------------------------------------------------------
 *
 *   0           1           2           3           4           5
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *   |           |     /     |███████████|███████████|     \     |           |
 *   |  Outside  |   /       |██ Solid ██|██ Solid ██|       \   |  Outside  |
 *   | (No Fill) | / EndCap  |██ Wide  ██|██ Tile  ██| EndCap  \ | (No Fill) |
 *   |           |   1 Tile  |██ (100%)██|██ (100%)██|   1 Tile  |           |
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *               ^           ^                       ^
 *               |           |                       |
 *               [EndCap A]  [WideTile]              [EndCap B]
 *               x: 1        x: 2                    x: 4
 *               width: 1    width: 2                width: 1
 *               alpha: 0                            alpha: 16
 *
 * -------------------------------------------------------------------------------------------------
 * Example Row (4x4 sized tile, 16 alphas per tile): Multi-Tile EndCap (Left Edge Crosses Two Tiles)
 * -------------------------------------------------------------------------------------------------
 *
 *   0           1           2           3           4           5
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *   |           |           |  /        |███████████|███████████|     \     |
 *   |  Outside  |           |/          |██ Solid ██|██ Solid ██|       \   |
 *   | (No Fill) |  EndCap / |           |██ Wide  ██|██ Tile  ██| EndCap  \ |
 *   |           |       /   |  2 Tiles  |██ (100%)██|██ (100%)██|   1 Tile  |
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *               ^                       ^                       ^
 *               |                       |                       |
 *               [EndCap C]              [WideTile]              [EndCap D]
 *               x: 1                    x: 3                    x: 5
 *               width: 2                width: 2                width: 1
 *               alpha: 32                                       alpha: 64
 */
class MakeStrips {
public:
    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static void MsaaScalar(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                           WideTiles<kTileHeight>& wides,
                           EndCaps<kTileHeight>& ends,
                           AlphaAtlasManager* atlasManager,
                           SkTDArray<uint8_t>* alphaBufFallback,
                           SkPathFillType fillType,
                           const Polyline& polyline,
                           const SkTDArray<uint8_t>& maskLut,
                           uint16_t viewportWidth,
                           uint16_t viewportHeight
#if defined(GPU_TEST_UTILS)
                           , MsaaExactMaskObserver observer = nullptr
#endif
    ) {
        Dispatch(fillType, [&](auto isWindingTag, bool isInverse) {
            constexpr bool kIsWinding = decltype(isWindingTag)::value;
            SkTDArray<uint8_t> runAlphaBuf;
            StripProcessorScalar<kTileWidth, kTileHeight, kIsWinding> processor(
                    &runAlphaBuf, isInverse, polyline, maskLut, /*initialAlphaIdx=*/0
#if defined(GPU_TEST_UTILS)
                    , observer
#endif
            );

            TraverseCPU<kTileWidth, kTileHeight>(
                    tileContainer, wides, ends, atlasManager, alphaBufFallback,
                    viewportWidth, viewportHeight, isInverse, &processor, &runAlphaBuf);
        });
    }

    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static void MsaaScalar(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                           WideTiles<kTileHeight>& wides,
                           EndCaps<kTileHeight>& ends,
                           AlphaAtlasManager* atlasManager,
                           SkPathFillType fillType,
                           const Polyline& polyline,
                           const SkTDArray<uint8_t>& maskLut,
                           uint16_t viewportWidth,
                           uint16_t viewportHeight
#if defined(GPU_TEST_UTILS)
                           , MsaaExactMaskObserver observer = nullptr
#endif
    ) {
        MsaaScalar<kTileWidth, kTileHeight>(
                tileContainer, wides, ends, atlasManager, /*alphaBufFallback=*/nullptr,
                fillType, polyline, maskLut, viewportWidth, viewportHeight
#if defined(GPU_TEST_UTILS)
                , observer
#endif
        );
    }

    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static void MsaaSimd(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                         WideTiles<kTileHeight>& wides,
                         EndCaps<kTileHeight>& ends,
                         AlphaAtlasManager* atlasManager,
                         SkTDArray<uint8_t>* alphaBufFallback,
                         SkPathFillType fillType,
                         const Polyline& polyline,
                         const SkTDArray<uint8_t>& maskLut,
                         uint16_t viewportWidth,
                         uint16_t viewportHeight
#if defined(GPU_TEST_UTILS)
                         , MsaaExactMaskObserver observer = nullptr
#endif
    ) {
        Dispatch(fillType, [&](auto isWindingTag, bool isInverse) {
            constexpr bool kIsWinding = decltype(isWindingTag)::value;
            SkTDArray<uint8_t> runAlphaBuf;
            StripProcessorSimd<kTileWidth, kTileHeight, kIsWinding> processor(
                    &runAlphaBuf, isInverse, polyline, maskLut, /*initialAlphaIdx=*/0
#if defined(GPU_TEST_UTILS)
                    , observer
#endif
            );

            TraverseCPU<kTileWidth, kTileHeight>(
                    tileContainer, wides, ends, atlasManager, alphaBufFallback,
                    viewportWidth, viewportHeight, isInverse, &processor, &runAlphaBuf);
        });
    }

    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static void MsaaSimd(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                         WideTiles<kTileHeight>& wides,
                         EndCaps<kTileHeight>& ends,
                         AlphaAtlasManager* atlasManager,
                         SkPathFillType fillType,
                         const Polyline& polyline,
                         const SkTDArray<uint8_t>& maskLut,
                         uint16_t viewportWidth,
                         uint16_t viewportHeight
#if defined(GPU_TEST_UTILS)
                         , MsaaExactMaskObserver observer = nullptr
#endif
    ) {
        MsaaSimd<kTileWidth, kTileHeight>(
                tileContainer, wides, ends, atlasManager, /*alphaBufFallback=*/nullptr,
                fillType, polyline, maskLut, viewportWidth, viewportHeight
#if defined(GPU_TEST_UTILS)
                , observer
#endif
        );
    }

private:
    template <typename F>
    static SK_ALWAYS_INLINE void Dispatch(SkPathFillType fillType, F&& f) {
        switch (fillType) {
            case SkPathFillType::kWinding:
                f(std::bool_constant</*isWinding=*/true>{}, /*isInverse=*/false);
                return;
            case SkPathFillType::kInverseWinding:
                f(std::bool_constant</*isWinding=*/true>{}, /*isInverse=*/true);
                return;
            case SkPathFillType::kEvenOdd:
                f(std::bool_constant</*isWinding=*/false>{}, /*isInverse=*/false);
                return;
            case SkPathFillType::kInverseEvenOdd:
                f(std::bool_constant</*isWinding=*/false>{}, /*isInverse=*/true);
                return;
        }
        SkUNREACHABLE;
    }

    // While the underlying implementation may be scalar or SIMD, the core traversal across
    // the tiles is identical. To reiterate, the goal of MakeStrips is twofold:
    // 1) Combine intersecting segments within the same spatial tile to resolve final coverage.
    // 2) Directly emit `EndCaps` for fractional edge tiles and `WideTiles` for solid interior spans.
    //
    // To achieve this in a single pass, the traversal treats the strictly sorted tile stream
    // as an event-driven state machine governed by three transition events:
    //
    // 1) Tile Start (`tileStart`):
    //    Triggered when the current tile's x or y differs from the previous tile.
    //    Action: All overlapping segments at the previous spatial coordinate have been processed.
    //    The accumulated coverage is pushed to the dense alpha buffer, and the local mask is reset.
    //
    // 2) Segment Start (`segStart`):
    //    Triggered when moving to a new row (`rowStart`), OR when the current tile's x coordinate
    //    skips forward by more than 1 (a non-contiguous gap in the same row).
    //    Action:
    //      a) Finalize the contiguous EndCap run that just completed and push to `ends`.
    //      b) If the scanline winding is interior (`ShouldFill(coarseWinding) ^ isInverse`) and we
    //         are not at a row break, emit an interior `WideTile` to `wides` spanning from the end
    //         of the previous EndCap to the start of the current tile.
    //      c) If `rowStart`, close any inverse background fill on the previous row, reset coarse
    //         winding for the new scanline, and emit row padding for inverse fills.
    //      d) Start tracking a new contiguous EndCap run at the current tile.
    //
    // 3) Row Start (`rowStart`):
    //    Triggered when the current tile's y differs from the previous tile (moves to a new row).
    //    Action: The previous scanline has ended. Resets coarse winding and coverage mask for
    //    the new row, and emits background fill spans when rendering inverse paths.
    template <uint16_t kTileWidth, uint16_t kTileHeight, typename Processor>
    static SK_ALWAYS_INLINE void TraverseCPU(
            const Tiles<kTileWidth, kTileHeight>& tileContainer,
            WideTiles<kTileHeight>& wides,
            EndCaps<kTileHeight>& ends,
            AlphaAtlasManager* atlasManager,
            SkTDArray<uint8_t>* alphaBufFallback,
            uint16_t viewportWidth,
            uint16_t viewportHeight,
            bool isInverse,
            Processor* processor,
            SkTDArray<uint8_t>* runAlphaBuf) {
        auto emitBackground = [&](uint16_t start, uint16_t end, uint16_t width) {
            for (uint16_t row = start; row < end; row += kTileHeight) {
                wides.addTile(0, row, width);
            }
        };

        const auto& tiles = tileContainer.getTiles();
        if (tiles.empty()) {
            if (isInverse) {
                emitBackground(0, viewportHeight + kTileHeight, viewportWidth);
            }
            return;
        }

        size_t totalCount = tiles.size();
        Tile prevTile = tiles[0];

        uint16_t runStartX = prevTile.x;
        uint16_t lastCoveredX = 0;
        bool inRun = true;

        if (isInverse) {
            emitBackground(0, prevTile.y * kTileHeight, viewportWidth);
            if (prevTile.x > 0) {
                wides.addTile(0, prevTile.y * kTileHeight, prevTile.x * kTileWidth);
            }
            lastCoveredX = prevTile.x * kTileWidth;
        } else {
            lastCoveredX = prevTile.x * kTileWidth;
        }

        float prevX = static_cast<float>(prevTile.x * kTileWidth);
        float prevY = static_cast<float>(prevTile.y * kTileHeight);
        std::array<SkPoint, 2> tileBounds = {
                SkPoint::Make(prevX, prevY),
                SkPoint::Make(prevX + static_cast<float>(kTileWidth),
                              prevY + static_cast<float>(kTileHeight))
        };

        auto finalizeRun = [&]() {
            if (!inRun) {
                return;
            }
            uint16_t endCapX = runStartX * kTileWidth;
            uint16_t endCapWidth = (prevTile.x - runStartX + 1) * kTileWidth;
            int32_t numBytes = runAlphaBuf->size();

            if (numBytes > 0) {
                if (atlasManager) {
                    auto alloc = atlasManager->requestAlphaSpace(ends, numBytes);
                    if (alloc.fSuccess) {
                        std::memcpy(alloc.fWritePtr, runAlphaBuf->data(), numBytes);
                        ends.addCap(endCapX, prevTile.y * kTileHeight, endCapWidth,
                                    alloc.fAlphaIndex, alloc.fTexPage);
                    }
                } else if (alphaBufFallback) {
                    int32_t alphaIdx = alphaBufFallback->size();
                    alphaBufFallback->append(numBytes, runAlphaBuf->data());
                    ends.addCap(endCapX, prevTile.y * kTileHeight, endCapWidth,
                                alphaIdx, /*texPage=*/0);
                }
            }
            runAlphaBuf->clear();
            lastCoveredX = endCapX + endCapWidth;
            inRun = false;
        };

        for (size_t i = 0; i < totalCount; ++i) {
            Tile tile = tiles[i];

            // Determine tile traversal events
            bool rowStart = (tile.y != prevTile.y);
            bool tileStart = (tile.x != prevTile.x || rowStart);
            bool segStart = tileStart && (rowStart || (tile.x != prevTile.x + 1));

            if (tileStart) {
                // Moving to a new tile implies that all previous tile's coverage has been combined,
                // resolve the coverage mask winding to alpha, then clear it.
                processor->resolveWindingToAlpha();
                if (!rowStart) {
                    // If we're not a row start, carry the scanline winding by seeding the coverage
                    // mask with the coarse winding.
                    processor->clearWithCoarseWinding();
                }
            }

            if (segStart) {
                // 1. Finalize the contiguous EndCap run
                finalizeRun();

                // 2. If winding is inside, emit the solid WideTile interior span
                bool shouldFill = processor->ShouldFill(processor->coarseWinding()) ^ isInverse;
                if (shouldFill && !rowStart) {
                    uint16_t wideStartX = lastCoveredX;
                    uint16_t wideEndX = tile.x * kTileWidth;
                    if (wideEndX > wideStartX) {
                        wides.addTile(wideStartX, prevTile.y * kTileHeight, wideEndX - wideStartX);
                    }
                    lastCoveredX = wideEndX;
                }

                // 3. Handle Row Breaks
                if (rowStart) {
                    if (shouldFill && isInverse) {
                        uint16_t wideStartX = lastCoveredX;
                        if (viewportWidth > wideStartX) {
                            wides.addTile(wideStartX, prevTile.y * kTileHeight, viewportWidth - wideStartX);
                        }
                    }

                    // Reset coarse winding for the new row
                    processor->setCoarseWinding(0);
                    processor->clearWindingForNewRow();

                    if (isInverse) {
                        emitBackground(
                                (prevTile.y + 1) * kTileHeight, tile.y * kTileHeight, viewportWidth);
                        if (tile.x > 0) {
                            wides.addTile(0, tile.y * kTileHeight, tile.x * kTileWidth);
                        }
                    }
                    lastCoveredX = tile.x * kTileWidth;
                }

                // 4. Start a new contiguous alpha run
                runStartX = tile.x;
                inRun = true;
            }

            prevTile = tile;

            // Lazily recalculate tile bounds only if we have moved to a new tile
            if (tileStart) {
                float x = static_cast<float>(tile.x * kTileWidth);
                float y = static_cast<float>(tile.y * kTileHeight);
                tileBounds = {
                        SkPoint::Make(x, y),
                        SkPoint::Make(x + static_cast<float>(kTileWidth),
                                      y + static_cast<float>(kTileHeight))
                };
            }

            processor->rasterizeLineToTile(tile, tileBounds);
        }

        // Process the last tile and finalize
        processor->resolveWindingToAlpha();
        finalizeRun();

        bool shouldFill = processor->ShouldFill(processor->coarseWinding()) ^ isInverse;
        if (isInverse) {
            if (shouldFill) {
                uint16_t wideStartX = lastCoveredX;
                if (viewportWidth > wideStartX) {
                    wides.addTile(wideStartX, prevTile.y * kTileHeight, viewportWidth - wideStartX);
                }
            }
            emitBackground((prevTile.y + 1) * kTileHeight,
                           viewportHeight + kTileHeight,
                           viewportWidth);
        }
    }
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_MakeStrips_DEFINED
