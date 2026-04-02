/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "bench/graphite/sparse_strips/TigerSvgUnscaled.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/private/base/SkTDArray.h"
#include "include/utils/SkParsePath.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight>
class CullBench : public Benchmark {
public:
    CullBench(int percentOnscreen)
            : fPercentOnscreen(percentOnscreen)
            , fLUT(GenerateMSAALUT<uint8_t>()) {
        fName.printf("SparseStrips_CullBench_%ux%u_%d_percent",
                     kTileWidth, kTileHeight, percentOnscreen);
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDelayedSetup() override {
        float tx = Tiger::kImageWidthF * (100.0f - fPercentOnscreen) / 100.0f;
        SkMatrix matrix = SkMatrix::Translate(tx, 0.0f);

        for (const char* svgString : Tiger::kTigerPathStrings) {
            SkPath path;
            if (SkParsePath::FromSVGString(svgString, &path)) {
                fPaths.push_back(path.makeTransform(matrix));
            }
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        Flatten flattener;
        SkTDArray<Strip> strips;
        SkTDArray<uint8_t> alphas;
        WideTiles<kTileHeight> wides;

        int32_t rows = (Tiger::kImageHeight + kTileHeight + 1) / kTileHeight;
        WindingHistogram windingHistogram(rows);
        windingHistogram.resize(rows);

        for (int i = 0; i < loops; ++i) {
            for (const SkPath& path : fPaths) {
                Polyline polyline;
                Tiles<kTileWidth, kTileHeight> tiler;

                flattener.processPaths<FlattenMode::kSimd>(path, SkMatrix(), Tiger::kDim, polyline);

                windingHistogram.clear();
                bool culled = tiler.makeTilesMSAA(polyline, Tiger::kDim, windingHistogram);

                tiler.sortTiles();

                skgpu::graphite::Strip::MakeStripsMsaaSimd(
                        tiler, strips, alphas, path.getFillType(), polyline, fLUT,
                        wides, culled, windingHistogram, Tiger::kDim, nullptr);

                strips.clear();
                alphas.clear();
                wides.clear();
                windingHistogram.clear();
            }
        }
    }

private:
    SkString fName;
    int fPercentOnscreen;
    skia_private::TArray<SkPath> fPaths;
    SkTDArray<uint8_t> fLUT;
};

}  // namespace skgpu::graphite

DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(10));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(20));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(30));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(40));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(50));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(60));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(70));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(80));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(90));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(100));)
