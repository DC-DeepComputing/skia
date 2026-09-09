/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_StripGenerator_DEFINED
#define skgpu_graphite_sparse_strips_StripGenerator_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <cstdint>

namespace skgpu::graphite {

class DrawContext;
class Recorder;

class StripGenerator {
public:
    static constexpr uint16_t kTileWidth = SparseStripConfig::kTileWidth;
    static constexpr uint16_t kTileHeight = SparseStripConfig::kTileHeight;

    StripGenerator(uint16_t viewportWidth,
                   uint16_t viewportHeight,
                   const SkTDArray<uint8_t>& lut,
                   Recorder* recorder);
    ~StripGenerator();

    bool processGeometry(const SkPath& path, const SkMatrix& ctm);

    bool hasNullCaps() const { return fEnds.hasNullCaps(); }
    bool cleanupNullCaps(DrawContext* dc = nullptr);

    const WideTiles& wides() const { return fWides; }
    const EndCaps& ends() const { return fEnds; }
    WideTiles& wides() { return fWides; }
    EndCaps& ends() { return fEnds; }

private:
    void reset();

    const uint16_t fViewportWidth;
    const uint16_t fViewportHeight;

    Flatten fFlattener;
    Tiles<kTileWidth, kTileHeight> fTiles;
    Polyline fFlattened;
    const SkTDArray<uint8_t>& fLUT;
    Recorder* fRecorder;

    WideTiles fWides;
    EndCaps fEnds;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_StripGenerator_DEFINED
