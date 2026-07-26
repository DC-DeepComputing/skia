/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "bench/Tiger.h"
#include "include/core/SkPath.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight>
class CoverageBench : public Benchmark {
public:
    using MakeStripsFn = void (*)(const Tiles<kTileWidth, kTileHeight>&,
                                  SkTDArray<Strip>*,
                                  SkTDArray<uint8_t>*,
                                  SkPathFillType,
                                  const Polyline&,
                                  const SkTDArray<uint8_t>&,
                                  WideTiles<kTileHeight>&,
                                  bool,
                                  const WindingHistogram&,
                                  const SkIRect&,
                                  MsaaExactMaskObserver);

    CoverageBench(const char* name, MakeStripsFn func)
            : fFunc(func)
            , fCulled((Tiger::kTigerHeight + kTileHeight + 1) / kTileHeight) {
        fName.printf("SparseStrips_%s_%ux%u", name, kTileWidth, kTileHeight);
        fLUT = GenerateMSAALUT<uint8_t>();
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDelayedSetup() override {
        Flatten flattener;
        std::vector<SkPath> tigerPaths = Tiger::GetTigerPaths();
        for (auto& subPath : tigerPaths) {
            flattener.processPaths<FlattenMode::kSimd>(subPath, SkMatrix(),
                                                       SkIRect::MakeWH(Tiger::kTigerWidth, Tiger::kTigerHeight),
                                                       &fPolyline);
        }
        fCulled.resize((Tiger::kTigerHeight + kTileHeight + 1) / kTileHeight);
        fTiles.makeTilesMSAA(fPolyline, SkIRect::MakeWH(Tiger::kTigerWidth, Tiger::kTigerHeight), fCulled);
        fTiles.sortTiles();
    }

    void onDraw(int loops, SkCanvas* /*canvas*/) override {
        SkTDArray<Strip> strips;
        WideTiles<kTileHeight> wides;
        SkTDArray<uint8_t> alphas;
        for (int i = 0; i < loops; ++i) {
            fFunc(fTiles, &strips, &alphas, SkPathFillType::kDefault, fPolyline, fLUT,
                  wides, /*isCulled=*/false, fCulled, SkIRect::MakeWH(Tiger::kTigerWidth, Tiger::kTigerHeight),
                  /*MsaaExactMaskObserver=*/nullptr);
            strips.resize(0);
            alphas.resize(0);
        }
    }

private:
    SkString fName;
    MakeStripsFn fFunc;
    Polyline fPolyline;
    Tiles<kTileWidth, kTileHeight> fTiles;
    SkTDArray<uint8_t> fLUT;
    WindingHistogram fCulled;
};

}  // namespace skgpu::graphite

DEF_BENCH(return (new skgpu::graphite::CoverageBench<4, 4>(
        "CoverageBenchScalar", &skgpu::graphite::MakeStrips::MsaaScalar));)
DEF_BENCH(return (new skgpu::graphite::CoverageBench<4, 4>(
        "CoverageBenchSimd", &skgpu::graphite::MakeStrips::MsaaSimd));)
DEF_BENCH(return (new skgpu::graphite::CoverageBench<8, 8>(
        "CoverageBenchSimd", &skgpu::graphite::MakeStrips::MsaaSimd));)
