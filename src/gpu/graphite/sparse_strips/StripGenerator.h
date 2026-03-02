/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_StripGenerator_DEFINED
#define skgpu_graphite_sparse_strips_StripGenerator_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/ganesh/geometry/GrPathUtils.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripTypes.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <vector>

namespace skgpu::graphite {

template<uint16_t WIDTH, uint16_t HEIGHT>
class StripGenerator {
public:
    StripGenerator(uint16_t viewportWidth, uint16_t viewportHeight, const SkTDArray<uint8_t>* lut,
                   Recorder* recorder)
        : fViewportWidth(viewportWidth), fViewportHeight(viewportHeight) , fLUT(lut)
        , fRecorder(recorder) {}

    template <typename F>
    void processGeometry(const SkPath& path, F&& consumeGeometry) {
        fFlattenned.reset();
        fTiles.reset();
        fStrips.clear();

        fFlattener.processPaths<FlattenMode::kSimd>(path, fViewportWidth, fViewportHeight, fFlattenned);

        fTiles.makeTilesMSAA(fFlattenned, fViewportWidth, fViewportHeight);
        fTiles.sortTiles();

        AlphaAtlasManager* manager = fRecorder->priv().atlasProvider()->alphaAtlasManager();
        if (path.getFillType() != SkPathFillType::kWinding &&
            path.getFillType() != SkPathFillType::kEvenOdd) {
            printf("warn, unsupported fill type\n");
        }
        Strip::MakeStripsMsaa(fTiles, fStrips, manager, path.getFillType(), fFlattenned, fLUT);

        const Strip* prev = nullptr;
        uint16_t prevWidth = 0;

        constexpr size_t kBytesPerTile = WIDTH * HEIGHT;
        SkASSERT(fRecorder);

        if (!fStrips.empty()) {
            for (size_t i = 0; i < fStrips.size() - 1; ++i) {
                const Strip& curr = fStrips[i];
                uint32_t startIdx = curr.packedAlphaIdx & ~(1u << 31);
                uint32_t endIdx;

                // Look ahead to find where this strip's alpha data ends
                if (i + 1 < fStrips.size()) {
                    endIdx = fStrips[i+1].packedAlphaIdx & ~(1u << 31);
                } else {
                    endIdx = manager->currentOffset();
                }

                uint32_t bytesConsumed = endIdx - startIdx;
                uint32_t numTiles = (bytesConsumed + kBytesPerTile - 1) / kBytesPerTile;
                uint16_t currWidth = (numTiles == 0) ? WIDTH : (numTiles * WIDTH);

                consumeGeometry(EndCap<WIDTH, HEIGHT>(curr.x, curr.y, currWidth, startIdx, fRecorder));
                bool shouldFill = (curr.packedAlphaIdx >> 31) & 1;

                if (shouldFill) {
                    SkASSERT(prev);
                    SkASSERT(prev->y == curr.y); // Should always be on same row if filling

                    // GAP START: The end of the previous strip
                    int startX = prev->x + prevWidth;
                    int endX = curr.x;
                    int width = endX - startX;

                    int currentX = startX;

                    // Chop the gap into max 256px wide tiles
                    while (width > 0) {
                        int step = std::min(width, 256);
                        consumeGeometry(WideTile<HEIGHT>(currentX, curr.y, step));

                        currentX += step;
                        width -= step;
                    }
                }

                prev = &curr;
                prevWidth = currWidth;
            }
        }
    }

private:
    const uint16_t fViewportWidth;
    const uint16_t fViewportHeight;

    Flatten fFlattener;
    Tiles<WIDTH, HEIGHT> fTiles;
    Polyline fFlattenned;
    std::vector<Strip> fStrips;
    const SkTDArray<uint8_t>* fLUT;
    bool fDrawn = false;
    Recorder* fRecorder;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_StripGenerator_DEFINED
