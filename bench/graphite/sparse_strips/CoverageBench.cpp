/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "bench/graphite/sparse_strips/TigerSvgUnscaled.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/private/base/SkTDArray.h"
#include "include/utils/SkParsePath.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight, typename MSAAType>
class CoverageBench : public Benchmark {
public:
    using MakeStripsFn = void (*)(const Tiles<kTileWidth, kTileHeight>&,
                                  SkTDArray<Strip>&,
                                  SkTDArray<MSAAType>&,
                                  SkPathFillType,
                                  const Polyline&,
                                  const SkTDArray<MSAAType>&,
                                  WideTiles<kTileHeight>&,
                                  bool,
                                  const WindingHistogram&,
                                  const SkIRect&,
                                  MsaaExactMaskObserver<MSAAType>
    );

    CoverageBench(const char* name, MakeStripsFn func)
            : fFunc(func)
            , fLUT(GenerateMSAALUT<MSAAType>())
            , fCulled((Tiger::kImageHeight + kTileHeight + 1) / kTileHeight) {
        fName.printf("SparseStrips_%s_%ux%u", name, kTileWidth, kTileHeight);
        fCulled.resize((Tiger::kImageHeight + kTileHeight + 1) / kTileHeight);
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDelayedSetup() override {
        Flatten flattener;
        for (const char* svgString : Tiger::kTigerPathStrings) {
            SkPath path;
            if (SkParsePath::FromSVGString(svgString, &path)) {
                Polyline polyline;
                flattener.processPaths<FlattenMode::kSimd>(path, SkMatrix(), Tiger::kDim, polyline);
                Tiles<kTileWidth, kTileHeight> tiler;
                tiler.reset();
                tiler.makeTilesMSAA(polyline, Tiger::kDim, fCulled);
                tiler.sortTiles();
                fTiles.emplace_back(tiler);
                fPolylines.emplace_back(polyline);
            }
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkTDArray<Strip> strips;
        WideTiles<kTileHeight> wides;
        SkTDArray<MSAAType> alphas;
        for (int i = 0; i < loops; ++i) {
            for (int ii = 0; ii < fTiles.size(); ++ii) {
                fFunc(fTiles[ii], strips, alphas, SkPathFillType::kDefault, fPolylines[ii], fLUT,
                      wides, /*isCulled=*/false, fCulled, Tiger::kDim, nullptr);
            }
        }
    }

private:
    SkString fName;
    MakeStripsFn fFunc;
    skia_private::TArray<Polyline> fPolylines;
    skia_private::TArray<Tiles<kTileWidth, kTileHeight>> fTiles;
    SkTDArray<MSAAType> fLUT;
    WindingHistogram fCulled;
};

}  // namespace skgpu::graphite

DEF_BENCH(return (new skgpu::graphite::CoverageBench<16, 16,
                                                     skgpu::graphite::SparseStripsConfig::MSAAType>(
        "CoverageBenchSimd",
        &skgpu::graphite::Strip::
                MakeStripsMsaaSimd<16, 16, skgpu::graphite::SparseStripsConfig::MSAAType>));)
DEF_BENCH(return (new skgpu::graphite::CoverageBench<8, 8,
                                                     skgpu::graphite::SparseStripsConfig::MSAAType>(
        "CoverageBenchSimd",
        &skgpu::graphite::Strip::
                MakeStripsMsaaSimd<8, 8, skgpu::graphite::SparseStripsConfig::MSAAType>));)
DEF_BENCH(return (new skgpu::graphite::CoverageBench<4, 4,
                                                     skgpu::graphite::SparseStripsConfig::MSAAType>(
        "CoverageBenchSimd",
        &skgpu::graphite::Strip::
                MakeStripsMsaaSimd<4, 4, skgpu::graphite::SparseStripsConfig::MSAAType>));)
DEF_BENCH(return (new skgpu::graphite::CoverageBench<4, 4,
                                                     skgpu::graphite::SparseStripsConfig::MSAAType>(
        "CoverageBench",
        &skgpu::graphite::Strip::
                MakeStripsMsaa<4, 4, skgpu::graphite::SparseStripsConfig::MSAAType>));)
