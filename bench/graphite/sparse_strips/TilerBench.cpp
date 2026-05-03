/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "bench/Tiger.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <vector>

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
        std::vector<SkPath> tigerPath = Tiger::GetTigerPaths();
        for (auto& verb : tigerPath) {
            flattener.processPaths<FlattenMode::kSimd>(verb, SkMatrix(),
                                                       Tiger::kTigerWidthF, Tiger::kTigerHeightF,
                                                       &fPolyline);
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; ++i) {
            Tiles<kTileWidth, kTileHeight> tiler;
            tiler.reset();
            tiler.makeTilesMSAA(fPolyline, Tiger::kTigerHeight, Tiger::kTigerWidth);
        }
    }

private:

    SkString fName;
    Polyline fPolyline;
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
        std::vector<SkPath> tigerPath = Tiger::GetTigerPaths();
        for (auto& verb : tigerPath) {
            flattener.processPaths<FlattenMode::kSimd>(verb, SkMatrix(),
                                                       Tiger::kTigerWidthF, Tiger::kTigerHeightF,
                                                       &fPolyline);
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; ++i) {
            Tiles<kTileWidth, kTileHeight> tiler;
            tiler.reset();
            tiler.makeTilesMSAA(fPolyline, Tiger::kTigerHeight, Tiger::kTigerWidth);
            tiler.sortTiles();
        }
    }

private:

    SkString fName;
    Polyline fPolyline;
};

}  // namespace skgpu::graphite

DEF_BENCH(return (new skgpu::graphite::TilerBench<4, 4>()); )
DEF_BENCH(return (new skgpu::graphite::TilerBench<8, 8>()); )
DEF_BENCH(return (new skgpu::graphite::TilerSortBench<4, 4>()); )
DEF_BENCH(return (new skgpu::graphite::TilerSortBench<8, 8>()); )
