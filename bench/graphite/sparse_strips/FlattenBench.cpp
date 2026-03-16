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

namespace skgpu::graphite {

template<FlattenMode kMode>
class FlattenBench : public Benchmark {
public:
    FlattenBench() {
        fName.printf("SparseStrips_Flatten%sBench_", kMode == FlattenMode::kScalar ? "Scalar" :
                                                                                    "SIMD");
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        for (const char* svgString : Tiger::kTigerPathStrings) {
            SkPath path;
            if (SkParsePath::FromSVGString(svgString, &path)) {
                fPaths.emplace_back(path);
            }
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        Flatten flattener;
        Polyline polyline;
        for (int i = 0; i < loops; ++i) {
            for (auto& path : fPaths) {
                flattener.processPaths<kMode>(path, Tiger::kImageWidthF, Tiger::kImageHeightF,
                                              polyline);
            }
        }
    }

private:

    SkString fName;
    skia_private::TArray<SkPath> fPaths;
};

}  // namespace skgpu::graphite

DEF_BENCH(return (new skgpu::graphite::FlattenBench<skgpu::graphite::FlattenMode::kScalar>()); )
DEF_BENCH(return (new skgpu::graphite::FlattenBench<skgpu::graphite::FlattenMode::kSimd>()); )
