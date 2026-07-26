/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_MakeStrips_DEFINED
#define skgpu_graphite_sparse_strips_MakeStrips_DEFINED

#include "include/core/SkPathTypes.h"
#include "include/core/SkRect.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/StripProcessorScalar.h"
#include "src/gpu/graphite/sparse_strips/StripProcessorSimd.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"

#include <utility>

namespace skgpu::graphite {

/*
 * At this point in the sparse strips pipeline, the path has been stroked, flattened into a
 * polyline, tiled, and sorted. Now, the tiles are consumed by `MakeStrips::*` to produce "strips,"
 * a difference encoded representation of the alpha of a path. Two things are required here:
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
 *  2) Strip Generation (Difference Encoding): Because the incoming tiles are now sorted by y, then
 *     x, the difference in coordinates between consecutive tiles is used to identify contiguous
 *     interior regions which can be rendered trivially without calculating per-pixel coverage, and
 *     do not require storing a full coverage mask (as they have a solid fill).
 *
 *     0           1           2           3           4           5
 *     +-----------+-----------+-----------+-----------+-----------+-----------+
 *     |           |     /     |███████████|███████████|     \     |           |
 *     |  Outside  |   /       |██ Solid ██|██ Solid ██|       \   |  Outside  |
 *     | (No Fill) | /  Edge   |██ Fill  ██|██ Fill  ██|  Edge   \ | (No Fill) |
 *     |           |    Tile   |███████████|███████████|  Tile     |           |
 *     +-----------+-----------+-----------+-----------+-----------+-----------+
 *                             ^                       ^
 *                             | <-- Implicit Gap -->  |
 *
 * A strip consists of three primary components:
 *  1) Bottom-Left Coordinates (x, y): the location at the bottom left corner, of the left-most
 *     tile, in a run of alpha tiles.
 *
 *  2) Alpha Index (alphaIdx): The offset into the buffer that stores the final per-pixel coverage
 *     values. This is always a multiple of the tile size. The difference in alpha index between two
 *     successive strips is used to determine the number of alpha tiles prior to a possible fill.
 *
 *  3) Fill State (shouldFill): A boolean flag packed into the highest bit of the `alphaIdx`. It
 *     dictates whether the empty space between *this* strip and the *next* strip is inside the
 *     shape. If true, the renderer solid-fills the implicit gap starting from the right edge of
 *     this strip's final alpha tile up to the left edge of the next strip's starting tile.
 *
 * -------------------------------------------------------------------------------------------------
 * Example Row (4x4 sized tile, 16 alphas per tile): Single-Tile Strip
 * -------------------------------------------------------------------------------------------------
 *
 *   0           1           2           3           4           5
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *   |           |     /     |███████████|███████████|     \     |           |
 *   |  Outside  |   /       |██ Solid ██|██ Solid ██|       \   |  Outside  |
 *   | (No Fill) | /  Alpha  |██ Fill  ██|██ Fill  ██|  Alpha  \ | (No Fill) |
 *   |           |    1 Tile |███████████|███████████|    1 Tile |           |
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *               ^                                   ^
 *               |                                   |
 *               [Strip A]                           [Strip B]
 *               x: 1                                x: 4
 *               alphaIdx: 0                         alphaIdx: 16
 *               shouldFill: 1 (true)                shouldFill: 0 (false)
 *
 * -------------------------------------------------------------------------------------------------
 * Example Row (4x4 sized tile, 16 alphas per tile): Multi-Tile Strip (Left Edge Crosses Two Tiles)
 * -------------------------------------------------------------------------------------------------
 *
 *   0           1           2           3           4           5
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *   |           |           |  /        |███████████|███████████|     \     |
 *   |  Outside  |           |/          |██ Solid ██|██ Solid ██|       \   |
 *   | (No Fill) |  Alpha  / |           |██ Fill  ██|██ Fill  ██|  Alpha  \ |
 *   |           |       /   |  2 Tiles  |███████████|███████████|    1 Tile |
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *               ^                                               ^
 *               |                                               |
 *               [Strip C]                                       [Strip D]
 *               x: 1                                            x: 5
 *               alphaIdx: 32                                    alphaIdx: 64
 *               shouldFill: 1 (true)                            shouldFill: 0 (false)
 */
class MakeStrips {
public:
    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static void MsaaScalar(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                           SkTDArray<Strip>* stripBuf,
                           SkTDArray<uint8_t>* alphaBuf,
                           SkPathFillType fillType,
                           const Polyline& polyline,
                           const SkTDArray<uint8_t>& maskLut,
                           WideTiles<kTileHeight>& wides,
                           bool isCulled,
                           const WindingHistogram& windingHistogram,
                           const SkIRect& clipRect
#if defined(GPU_TEST_UTILS)
                           , MsaaExactMaskObserver observer = nullptr
#endif
    ) {
        Dispatch(fillType, [&](auto isWindingTag, bool isInverse) {
            constexpr bool kIsWinding = decltype(isWindingTag)::value;
            int32_t localAlphaIdx = alphaBuf->size();
            StripProcessorScalar<kTileWidth, kTileHeight, kIsWinding> processor(
                    stripBuf, alphaBuf, isInverse, polyline, maskLut, localAlphaIdx
#if defined(GPU_TEST_UTILS)
                    , observer
#endif
            );

            TraverseCPU<kTileWidth, kTileHeight>(
                    tileContainer, stripBuf, alphaBuf, wides, isCulled, windingHistogram,
                    clipRect, isInverse, &processor);
        });
    }

    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static void MsaaSimd(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                         SkTDArray<Strip>* stripBuf,
                         SkTDArray<uint8_t>* alphaBuf,
                         SkPathFillType fillType,
                         const Polyline& polyline,
                         const SkTDArray<uint8_t>& maskLut,
                         WideTiles<kTileHeight>& wides,
                         bool isCulled,
                         const WindingHistogram& windingHistogram,
                         const SkIRect& clipRect
#if defined(GPU_TEST_UTILS)
                         , MsaaExactMaskObserver observer = nullptr
#endif
    ) {
        Dispatch(fillType, [&](auto isWindingTag, bool isInverse) {
            constexpr bool kIsWinding = decltype(isWindingTag)::value;
            int32_t localAlphaIdx = alphaBuf->size();
            StripProcessorSimd<kTileWidth, kTileHeight, kIsWinding> processor(
                    stripBuf, alphaBuf, isInverse, polyline, maskLut, localAlphaIdx
#if defined(GPU_TEST_UTILS)
                    , observer
#endif
            );

            TraverseCPU<kTileWidth, kTileHeight>(
                    tileContainer, stripBuf, alphaBuf, wides, isCulled, windingHistogram,
                    clipRect, isInverse, &processor);
        });
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

    template <uint16_t kTileWidth, uint16_t kTileHeight, typename Processor>
    static SK_ALWAYS_INLINE void TraverseCPU(
            const Tiles<kTileWidth, kTileHeight>& tileContainer,
            SkTDArray<Strip>* stripBuf,
            SkTDArray<uint8_t>* alphaBuf,
            WideTiles<kTileHeight>& wides,
            bool isCulled,
            const WindingHistogram& windingHistogram,
            const SkIRect& clipRect,
            bool isInverse,
            Processor* processor) {
        auto processEmptyTileRows = [&](uint16_t startTileY, uint16_t endTileY) {
            if (startTileY >= endTileY) {
                return;
            }

            uint16_t y = startTileY;
            uint16_t startX = static_cast<uint16_t>(clipRect.left());
            uint16_t width = static_cast<uint16_t>(clipRect.width());

            if (isCulled && isInverse) {
                const auto& activeRows = windingHistogram.activeRows();
                while (y < endTileY) {
                    int wordIdx = y >> 5;
                    uint16_t wordEnd = std::min<uint16_t>(endTileY, (wordIdx + 1) << 5);

                    uint32_t mask = activeRows[wordIdx];
                    mask &= (~0u << (y & 31));

                    uint32_t endShift = wordEnd & 31;
                    if (endShift != 0) {
                        mask &= (1u << endShift) - 1;
                    }

                    while (mask != 0) {
                        int bit = std::countr_zero(mask);
                        uint16_t nextActiveY = (wordIdx << 5) + bit;

                        for (; y < nextActiveY; ++y) {
                            wides.addTile(startX, y * kTileHeight, width);
                        }

                        if (!processor->ShouldFill(windingHistogram[y])) {
                            wides.addTile(startX, y * kTileHeight, width);
                        }

                        y++;
                        mask &= mask - 1;
                    }

                    for (; y < wordEnd; ++y) {
                        wides.addTile(startX, y * kTileHeight, width);
                    }
                }
            } else if (isCulled) {
                const auto& activeRows = windingHistogram.activeRows();
                while (y < endTileY) {
                    int wordIdx = y >> 5;
                    uint16_t wordEnd = std::min<uint16_t>(endTileY, (wordIdx + 1) << 5);

                    uint32_t mask = activeRows[wordIdx];
                    mask &= (~0u << (y & 31));

                    uint32_t endShift = wordEnd & 31;
                    if (endShift != 0) {
                        mask &= (1u << endShift) - 1;
                    }

                    while (mask != 0) {
                        int bit = std::countr_zero(mask);
                        y = (wordIdx << 5) + bit;

                        if (processor->ShouldFill(windingHistogram[y])) {
                            wides.addTile(startX, y * kTileHeight, width);
                        }
                        mask &= mask - 1;
                    }

                    y = wordEnd;
                }
            } else if (isInverse) {
                for (; y < endTileY; ++y) {
                    wides.addTile(startX, y * kTileHeight, width);
                }
            }
        };

        const auto& tiles = tileContainer.getTiles();
        if (tiles.empty()) {
            processEmptyTileRows(0, windingHistogram.size());
            return;
        }

        size_t totalCount = tiles.size();
        Tile prevTile = tiles[0];

        int32_t windingDelta = windingHistogram[prevTile.y];
        processor->setCoarseWinding(windingDelta);
        processor->clearWithCoarseWinding();

        processEmptyTileRows(0, prevTile.y);
        bool fillLeftStart = processor->ShouldFill(windingDelta) ^ isInverse;
        if (isInverse || isCulled) {
            if (fillLeftStart) {
                stripBuf->push_back(Strip(static_cast<uint16_t>(clipRect.left()),
                                          prevTile.y * kTileHeight,
                                          processor->localAlphaIdx(),
                                          false));
            }
        }

        Strip currentStrip(
                prevTile.x * kTileWidth,
                prevTile.y * kTileHeight,
                processor->localAlphaIdx(),
                fillLeftStart);

        float prevX = static_cast<float>(prevTile.x * kTileWidth);
        float prevY = static_cast<float>(prevTile.y * kTileHeight);
        std::array<SkPoint, 2> tileBounds = {
                SkPoint::Make(prevX, prevY),
                SkPoint::Make(prevX + static_cast<float>(kTileWidth),
                              prevY + static_cast<float>(kTileHeight))
        };

        for (size_t i = 0; i < totalCount; ++i) {
            Tile tile = tiles[i];

            // Determine tile traversal events
            bool rowStart = (tile.y != prevTile.y);
            bool tileStart = (tile.x != prevTile.x || rowStart);
            bool segStart = tileStart && (rowStart || (tile.x != prevTile.x + 1));

            if (tileStart) {
                processor->resolveWindingToAlpha();
                if (!rowStart) {
                    processor->clearWithCoarseWinding();
                }
            }

            if (segStart) {
                stripBuf->push_back(currentStrip);

                bool fillLeftCaptive = false;
                if (rowStart) {
                    if (processor->coarseWinding() != 0 || isInverse) {
                        stripBuf->push_back(Strip::MakeCap(
                                prevTile.y * kTileHeight,
                                processor->localAlphaIdx(),
                                processor->ShouldFill(processor->coarseWinding()) ^ isInverse));
                    }

                    processEmptyTileRows(prevTile.y + 1, tile.y);

                    windingDelta = windingHistogram[tile.y];
                    processor->setCoarseWinding(windingDelta);
                    processor->clearWithCoarseWinding();
                    fillLeftCaptive = processor->ShouldFill(windingDelta) ^ isInverse;

                    if (fillLeftCaptive) {
                        stripBuf->push_back(Strip(static_cast<uint16_t>(clipRect.left()),
                                                  tile.y * kTileHeight,
                                                  processor->localAlphaIdx(),
                                                  false));
                    }
                } else {
                    fillLeftCaptive = processor->ShouldFill(processor->coarseWinding()) ^ isInverse;
                }

                currentStrip = Strip(tile.x * kTileWidth,
                                     tile.y * kTileHeight,
                                     processor->localAlphaIdx(),
                                     fillLeftCaptive);
            }

            prevTile = tile;

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

        processor->resolveWindingToAlpha();
        stripBuf->push_back(currentStrip);
        stripBuf->push_back(Strip::MakeCap(prevTile.y * kTileHeight,
                                           processor->localAlphaIdx(),
                                           processor->ShouldFill(
                                                    processor->coarseWinding()) ^ isInverse));

        processEmptyTileRows(prevTile.y + 1, windingHistogram.size());

        alphaBuf->resize(processor->localAlphaIdx());
    }
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_MakeStrips_DEFINED
