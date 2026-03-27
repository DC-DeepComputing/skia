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
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight> class CoverageBench : public Benchmark {
public:
    using MakeStripsFn = void (*)(const Tiles<kTileWidth, kTileHeight>&,
                                  SkTDArray<Strip>&,
                                  SkTDArray<uint8_t>&,
                                  SkPathFillType,
                                  const Polyline&,
                                  const SkTDArray<uint8_t>&,
                                  WideTiles<kTileHeight>&,
                                  uint16_t viewportWidth,
                                  uint16_t viewportHeight,
                                  MsaaExactMaskObserver);

    CoverageBench(const char* name, MakeStripsFn func) : fFunc(func) {
        fName.printf("SparseStrips_%s_%ux%u", name, kTileWidth, kTileHeight);
        fLUT = GenerateMSAALUT<uint8_t>();
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDelayedSetup() override {
        Flatten flattener;

        for (const char* svgString : Tiger::kTigerPathStrings) {
            SkPath path;
            if (SkParsePath::FromSVGString(svgString, &path)) {
                Polyline polyline;
                flattener.processPaths<FlattenMode::kSimd>(
                        path, SkMatrix(), Tiger::kImageWidthF, Tiger::kImageHeightF, polyline);
                Tiles<kTileWidth, kTileHeight> tiler;
                tiler.reset();
                tiler.makeTilesMSAA(polyline, Tiger::kImageWidth, Tiger::kImageHeight);
                tiler.sortTiles();
                fTiles.emplace_back(tiler);
                fPolylines.emplace_back(polyline);
            }
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkTDArray<Strip> strips;
        WideTiles<kTileHeight> wides;
        SkTDArray<uint8_t> alphas;
        for (int i = 0; i < loops; ++i) {
            for (int ii = 0; ii < fTiles.size(); ++ii) {
                fFunc(fTiles[ii], strips, alphas, SkPathFillType::kDefault, fPolylines[ii], fLUT,
                      wides, Tiger::kImageWidth, Tiger::kImageHeight, nullptr);
            }
        }
    }

private:
    SkString fName;
    MakeStripsFn fFunc;
    skia_private::TArray<Polyline> fPolylines;
    skia_private::TArray<Tiles<kTileWidth, kTileHeight>> fTiles;
    SkTDArray<uint8_t> fLUT;
};

}  // namespace skgpu::graphite

DEF_BENCH(return (new skgpu::graphite::CoverageBench<16, 16>(
        "CoverageBenchSimd", &skgpu::graphite::Strip::MakeStripsMsaaSimd));)
DEF_BENCH(return (new skgpu::graphite::CoverageBench<8, 8>(
        "CoverageBenchSimd", &skgpu::graphite::Strip::MakeStripsMsaaSimd));)
DEF_BENCH(return (new skgpu::graphite::CoverageBench<4, 4>(
        "CoverageBenchSimd", &skgpu::graphite::Strip::MakeStripsMsaaSimd));)
DEF_BENCH(return (new skgpu::graphite::CoverageBench<4, 4>(
        "CoverageBench", &skgpu::graphite::Strip::MakeStripsMsaa));)
