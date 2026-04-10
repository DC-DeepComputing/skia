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
#include "src/gpu/graphite/sparse_strips/CompressedRowTiler.h"
#include "src/gpu/graphite/sparse_strips/RowTiler.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"

namespace skgpu::graphite {

template<uint16_t kTileWidth, uint16_t kTileHeight>
class TilerBench : public Benchmark {
public:
    TilerBench() : fCulled((Tiger::kImageHeight + kTileHeight + 1) / kTileHeight) {
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
                flattener.processPaths<FlattenMode::kSimd>(path, SkMatrix(), Tiger::kDim, polyline);
                fPolylines.emplace_back(polyline);
            }
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; ++i) {
            Tiles<kTileWidth, kTileHeight> tiler;
            for (auto& poly : fPolylines) {
                tiler.reset();
                tiler.makeTilesMSAA(poly, Tiger::kDim, fCulled);
            }
        }
    }

private:
    SkString fName;
    WindingHistogram fCulled;
    skia_private::TArray<Polyline> fPolylines;
};

template<uint16_t kTileWidth, uint16_t kTileHeight>
class TilerSortBench : public Benchmark {
public:
    TilerSortBench() : fCulled((Tiger::kImageHeight + kTileHeight + 1) / kTileHeight) {
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
                flattener.processPaths<FlattenMode::kSimd>(path, SkMatrix(), Tiger::kDim, polyline);
                fPolylines.emplace_back(polyline);
            }
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; ++i) {
            Tiles<kTileWidth, kTileHeight> tiler;
            for (auto& poly : fPolylines) {
                tiler.reset();
                tiler.makeTilesMSAA(poly, Tiger::kDim, fCulled);
                tiler.sortTiles();
            }
        }
    }

private:
    SkString fName;
    WindingHistogram fCulled;
    skia_private::TArray<Polyline> fPolylines;
};

template<uint16_t kTileWidth, uint16_t kTileHeight>
class RowTilerBench : public Benchmark {
public:
    RowTilerBench() : fCulled((Tiger::kImageHeight + kTileHeight + 1) / kTileHeight) {
        fName.printf("SparseStrips_RowTilerBench_%ux%u", kTileWidth, kTileHeight);
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
                flattener.processPaths<FlattenMode::kSimd>(path, SkMatrix(), Tiger::kDim, polyline);
                fPolylines.emplace_back(polyline);
            }
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; ++i) {
            // Reverted to original template args since RowTiles still expects them
            RowTiles<kTileWidth, kTileHeight> tiler;
            for (auto& poly : fPolylines) {
                tiler.reset();
                tiler.makeTilesMSAA(poly, Tiger::kDim, fCulled);
            }
        }
    }

private:
    SkString fName;
    WindingHistogram fCulled;
    skia_private::TArray<Polyline> fPolylines;
};

template<uint16_t kTileWidth, uint16_t kTileHeight>
class RowTilerSortBench : public Benchmark {
public:
    RowTilerSortBench() : fCulled((Tiger::kImageHeight + kTileHeight + 1) / kTileHeight) {
        fName.printf("SparseStrips_RowTilerSortBench_%ux%u", kTileWidth, kTileHeight);
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
                flattener.processPaths<FlattenMode::kSimd>(path, SkMatrix(), Tiger::kDim, polyline);
                fPolylines.emplace_back(polyline);
            }
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; ++i) {
            // Reverted to original template args since RowTiles still expects them
            RowTiles<kTileWidth, kTileHeight> tiler;
            for (auto& poly : fPolylines) {
                tiler.reset();
                tiler.makeTilesMSAA(poly, Tiger::kDim, fCulled);
                tiler.sortTiles();
            }
        }
    }

private:
    SkString fName;
    WindingHistogram fCulled;
    skia_private::TArray<Polyline> fPolylines;
};

template<uint16_t kTileWidth, uint16_t kTileHeight>
class CompressedRowTilerBench : public Benchmark {
public:
    CompressedRowTilerBench() : fCulled((Tiger::kImageHeight + kTileHeight + 1) / kTileHeight) {
        fName.printf("SparseStrips_CompressedRowTilerBench_%ux%u", kTileWidth, kTileHeight);
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
                flattener.processPaths<FlattenMode::kSimd>(path, SkMatrix(), Tiger::kDim, polyline);
                fPolylines.emplace_back(polyline);
            }
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; ++i) {
            // Updated to use the correct class name from your new header!
            CompressedRowTiles<> tiler;
            for (auto& poly : fPolylines) {
                tiler.reset();
                tiler.makeCompressedTilesMSAA(poly, Tiger::kDim, fCulled);
            }
        }
    }

private:
    SkString fName;
    WindingHistogram fCulled;
    skia_private::TArray<Polyline> fPolylines;
};

template<uint16_t kTileWidth, uint16_t kTileHeight>
class CompressedRowTilerSortBench : public Benchmark {
public:
    CompressedRowTilerSortBench() : fCulled((Tiger::kImageHeight + kTileHeight + 1) / kTileHeight) {
        fName.printf("SparseStrips_CompressedRowTilerSortBench_%ux%u", kTileWidth, kTileHeight);
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
                flattener.processPaths<FlattenMode::kSimd>(path, SkMatrix(), Tiger::kDim, polyline);
                fPolylines.emplace_back(polyline);
            }
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; ++i) {
            // Updated to use the correct class name from your new header!
            CompressedRowTiles<> tiler;
            for (auto& poly : fPolylines) {
                tiler.reset();
                tiler.makeCompressedTilesMSAA(poly, Tiger::kDim, fCulled);
                tiler.sortTiles();
            }
        }
    }

private:
    SkString fName;
    WindingHistogram fCulled;
    skia_private::TArray<Polyline> fPolylines;
};

}  // namespace skgpu::graphite

DEF_BENCH(return (new skgpu::graphite::TilerBench<4, 4>()); )
DEF_BENCH(return (new skgpu::graphite::TilerBench<8, 8>()); )
DEF_BENCH(return (new skgpu::graphite::TilerSortBench<4, 4>()); )
DEF_BENCH(return (new skgpu::graphite::TilerSortBench<8, 8>()); )

// Added RowTiles 8x8 benches
DEF_BENCH(return (new skgpu::graphite::RowTilerBench<8, 8>()); )
DEF_BENCH(return (new skgpu::graphite::RowTilerSortBench<8, 8>()); )

// Added CompressedRowTiles 8x8 benches
DEF_BENCH(return (new skgpu::graphite::CompressedRowTilerBench<8, 8>()); )
DEF_BENCH(return (new skgpu::graphite::CompressedRowTilerSortBench<8, 8>()); )
