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
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight, typename MSAAType>
class CullBench : public Benchmark {
public:
    CullBench(int percentOnscreen)
            : fPercentOnscreen(percentOnscreen)
            , fLUT(GenerateMSAALUT<MSAAType>()) {
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
        SkTDArray<MSAAType> alphas;
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

                skgpu::graphite::Strip::MakeStripsMsaaSimd<kTileWidth, kTileHeight, MSAAType>(
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
    SkTDArray<MSAAType> fLUT;
};

}  // namespace skgpu::graphite

DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8, skgpu::graphite::SparseStripsConfig::MSAAType>(10));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8, skgpu::graphite::SparseStripsConfig::MSAAType>(20));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8, skgpu::graphite::SparseStripsConfig::MSAAType>(30));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8, skgpu::graphite::SparseStripsConfig::MSAAType>(40));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8, skgpu::graphite::SparseStripsConfig::MSAAType>(50));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8, skgpu::graphite::SparseStripsConfig::MSAAType>(60));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8, skgpu::graphite::SparseStripsConfig::MSAAType>(70));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8, skgpu::graphite::SparseStripsConfig::MSAAType>(80));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8, skgpu::graphite::SparseStripsConfig::MSAAType>(90));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8, skgpu::graphite::SparseStripsConfig::MSAAType>(100));)
