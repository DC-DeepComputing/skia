/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "bench/Tiger.h"
#include "include/core/SkPath.h"
#include "include/private/base/SkTDArray.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight>
class CoverageBench : public Benchmark {
public:
    using MakeStripsFn = void (*)(const Tiles<kTileWidth, kTileHeight>&,
                                  SkTDArray<Strip>&,
                                  SkTDArray<uint8_t>&,
                                  SkPathFillType,
                                  const Polyline&,
                                  const SkTDArray<uint8_t>&,
                                  MsaaExactMaskObserver);

    CoverageBench(const char* name, MakeStripsFn func) : fFunc(func) {
        fName.printf("SparseStrips_%s_%ux%u", name, kTileWidth, kTileHeight);
        fLUT = GenerateMSAALUT<uint8_t>();
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDelayedSetup() override {
        Flatten flattener;
        std::vector<SkPath> tigerPath = Tiger::GetTigerPaths();
        for (auto& verb : tigerPath) {
            flattener.processPaths<FlattenMode::kSimd>(verb, SkMatrix(),
                                                       Tiger::kTigerWidthF, Tiger::kTigerHeightF,
                                                       &fPolyline);
        }
        fTiles.makeTilesMSAA(fPolyline, Tiger::kTigerWidth, Tiger::kTigerHeight);
        fTiles.sortTiles();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkTDArray<Strip> strips;
        SkTDArray<uint8_t> alphas;
        for (int i = 0; i < loops; ++i) {
            fFunc(fTiles, strips, alphas, SkPathFillType::kDefault, fPolyline, fLUT,
                  /*MsaaExactMaskObserver=*/nullptr);
        }
    }

private:
    SkString fName;
    MakeStripsFn fFunc;
    Polyline fPolyline;
    Tiles<kTileWidth, kTileHeight> fTiles;
    SkTDArray<uint8_t> fLUT;
};

}  // namespace skgpu::graphite

DEF_BENCH(return (new skgpu::graphite::CoverageBench<4, 4>(
        "CoverageBench", &skgpu::graphite::MakeStrips::MsaaScalar));)
