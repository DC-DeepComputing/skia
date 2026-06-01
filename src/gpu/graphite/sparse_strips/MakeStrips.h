/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_MakeStrips_DEFINED
#define skgpu_graphite_sparse_strips_MakeStrips_DEFINED

#include "include/core/SkPathTypes.h"
#include "include/private/base/SkTDArray.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/StripProcessorScalar.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <utility>

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight, bool kIsWinding>
class StripProcessorScalar;

class MakeStrips {
public:
    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static void MsaaScalar(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                           SkTDArray<Strip>& stripBuf,
                           SkTDArray<uint8_t>& alphaBuf,
                           SkPathFillType fillType,
                           const Polyline& polyline,
                           const SkTDArray<uint8_t>& maskLut
#if defined(GPU_TEST_UTILS)
                           , MsaaExactMaskObserver observer = nullptr
#endif
    ) {
        const auto& tiles = tileContainer.getTiles();
        if (tiles.empty()) {
            return;
        }
        Dispatch(fillType, [&](auto isWindingTag, bool isInverse) {
            constexpr bool kIsWinding = decltype(isWindingTag)::value;
            int32_t localAlphaIdx = alphaBuf.size();
            constexpr int32_t kTileAlphaCount = kTileWidth * kTileHeight;
            alphaBuf.reserve(localAlphaIdx + tiles.size() * kTileAlphaCount * 2);

            StripProcessorScalar<kTileWidth, kTileHeight, kIsWinding> processor(
                    stripBuf, alphaBuf, isInverse, polyline, maskLut, localAlphaIdx
#if defined(GPU_TEST_UTILS)
                    , observer
#endif
            );

            TraverseCPU<kTileWidth, kTileHeight>(tileContainer, stripBuf, alphaBuf, processor);
        });
    }

    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static void MsaaSimd(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                         SkTDArray<Strip>& stripBuf,
                         SkTDArray<uint8_t>& alphaBuf,
                         SkPathFillType fillType,
                         const Polyline& polyline,
                         const SkTDArray<uint8_t>& maskLut
#if defined(GPU_TEST_UTILS)
                         , MsaaExactMaskObserver observer = nullptr
#endif
    ) {
        // Stub
    }

private:
    template <typename F>
    static SK_ALWAYS_INLINE void Dispatch(SkPathFillType fillType, F&& f) {
        switch (fillType) {
            case SkPathFillType::kWinding:
                f(std::bool_constant<true>{}, /*isInverse=*/false);
                return;
            case SkPathFillType::kInverseWinding:
                f(std::bool_constant<true>{}, /*isInverse=*/true);
                return;
            case SkPathFillType::kEvenOdd:
                f(std::bool_constant<false>{}, /*isInverse=*/false);
                return;
            case SkPathFillType::kInverseEvenOdd:
                f(std::bool_constant<false>{}, /*isInverse=*/true);
                return;
        }
        SkUNREACHABLE;
    }

    template <uint16_t kTileWidth, uint16_t kTileHeight, typename Processor>
    static SK_ALWAYS_INLINE void TraverseCPU(
            const Tiles<kTileWidth, kTileHeight>& tileContainer,
            SkTDArray<Strip>& stripBuf,
            SkTDArray<uint8_t>& alphaBuf,
            Processor& processor) {
        const auto& tiles = tileContainer.getTiles();
        size_t totalCount = tiles.size();
        Tile prevTile = tiles[0];

        Strip currentStrip(prevTile.x * kTileWidth, prevTile.y * kTileHeight,
                           processor.localAlphaIdx(), false);

        float tileMinX = static_cast<float>(prevTile.x * kTileWidth);
        float tileMinY = static_cast<float>(prevTile.y * kTileHeight);
        float tileMaxX = tileMinX + static_cast<float>(kTileWidth);
        float tileMaxY = tileMinY + static_cast<float>(kTileHeight);

        for (size_t i = 0; i < totalCount; ++i) {
            Tile tile = tiles[i];

            bool tileStart = (tile.x != prevTile.x || tile.y != prevTile.y);
            bool rowStart = (tile.y != prevTile.y);
            bool segStart = tileStart && (rowStart || (tile.x != prevTile.x + 1));

            if (tileStart) {
                processor.resolveTileToAlpha();
                if (!rowStart) {
                    processor.clearMaskWithWindingDelta();
                }
            }

            if (segStart) {
                stripBuf.push_back(currentStrip);

                if (rowStart) {
                    if (processor.windingDelta() != 0) {
                        stripBuf.push_back(Strip(Tile::kSentinelCoord,
                                                 prevTile.y * kTileHeight,
                                                 processor.localAlphaIdx(),
                                                 processor.shouldFill(processor.windingDelta())));
                    }

                    processor.setWindingDelta(0);
                    processor.clearMaskForNewRow();
                }

                currentStrip = Strip(tile.x * kTileWidth,
                                     tile.y * kTileHeight,
                                     processor.localAlphaIdx(),
                                     processor.shouldFill(processor.windingDelta()));
            }

            prevTile = tile;

            if (tileStart) {
                tileMinX = static_cast<float>(tile.x * kTileWidth);
                tileMinY = static_cast<float>(tile.y * kTileHeight);
                tileMaxX = tileMinX + static_cast<float>(kTileWidth);
                tileMaxY = tileMinY + static_cast<float>(kTileHeight);
            }

            processor.rasterizeLineToTile(tile, tileMinX, tileMinY, tileMaxX, tileMaxY);
        }

        processor.resolveTileToAlpha();

        stripBuf.push_back(currentStrip);
        stripBuf.push_back(Strip(Tile::kSentinelCoord,
                                 prevTile.y * kTileHeight,
                                 processor.localAlphaIdx(),
                                 processor.shouldFill(processor.windingDelta())));

        alphaBuf.resize(processor.localAlphaIdx());
    }
};

} // namespace skgpu::graphite

#include "src/gpu/graphite/sparse_strips/StripProcessorScalar.h"

#endif // skgpu_graphite_sparse_strips_MakeStrips_DEFINED
