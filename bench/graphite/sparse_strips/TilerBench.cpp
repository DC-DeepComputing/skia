/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "bench/graphite/sparse_strips/TigerSvgUnscaled.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/private/base/SkTArray.h"
#include "include/utils/SkParsePath.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

namespace skgpu::graphite {

template<uint16_t kTileWidth, uint16_t kTileHeight>
class TilerBench : public Benchmark {
public:
    TilerBench() {
        fName.printf("SparseStrips_TilerBench_%ux%u", kTileWidth, kTileHeight);
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        Flatten flattener;
        for (const char* svgString : Tiger::kTigerPathStrings) {
            SkPath path;
            if (SkParsePath::FromSVGString(svgString, &path)) {
                Polyline polyline;
                flattener.processPaths<FlattenMode::kSimd>(path, SkMatrix(), Tiger::kImageWidthF,
                                                           Tiger::kImageHeightF, polyline);
                fPolylines.emplace_back(polyline);
            }
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; ++i) {
            Tiles<kTileWidth, kTileHeight> tiler;
            for (auto& poly : fPolylines) {
                tiler.reset();
                tiler.makeTilesMSAA(poly, Tiger::kImageWidth, Tiger::kImageHeight);
            }
        }
    }

private:

    SkString fName;
    skia_private::TArray<Polyline> fPolylines;
};

template<uint16_t kTileWidth, uint16_t kTileHeight>
class TilerSortBench : public Benchmark {
public:
    TilerSortBench() {
        fName.printf("SparseStrips_TilerSortBench_%ux%u", kTileWidth, kTileHeight);
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        Flatten flattener;
        for (const char* svgString : Tiger::kTigerPathStrings) {
            SkPath path;
            if (SkParsePath::FromSVGString(svgString, &path)) {
                Polyline polyline;
                flattener.processPaths<FlattenMode::kSimd>(path, SkMatrix(), Tiger::kImageWidthF,
                                                           Tiger::kImageHeightF, polyline);
                fPolylines.emplace_back(polyline);
            }
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; ++i) {
            Tiles<kTileWidth, kTileHeight> tiler;
            for (auto& poly : fPolylines) {
                tiler.reset();
                tiler.makeTilesMSAA(poly, Tiger::kImageWidth, Tiger::kImageHeight);
                tiler.sortTiles();
            }
        }
    }

private:

    SkString fName;
    skia_private::TArray<Polyline> fPolylines;
};

}  // namespace skgpu::graphite

DEF_BENCH(return (new skgpu::graphite::TilerBench<4, 4>()); )
DEF_BENCH(return (new skgpu::graphite::TilerBench<8, 8>()); )
DEF_BENCH(return (new skgpu::graphite::TilerSortBench<4, 4>()); )
DEF_BENCH(return (new skgpu::graphite::TilerSortBench<8, 8>()); )
