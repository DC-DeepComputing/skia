/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCubicMap.h"
#include "src/core/SkAutoPixmapStorage.h"

class CubicMapBench : public Benchmark {
public:
    CubicMapBench(SkPoint p1, SkPoint p2) : fCMap(p1, p2) {
        fName.printf("cubicmap_%g_%g_%g_%g", p1.fX, p1.fY, p2.fX, p2.fY);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int outer = 0; outer < 100; ++outer) {
            for (int i = 0; i < loops; ++i) {
                for (SkScalar x = 0; x <= 1; x += 1.0f / 512) {
                    fCMap.computeYFromX(x);
                }
            }
        }
    }

private:
    SkCubicMap  fCMap;
    SkString    fName;

    using INHERITED = Benchmark;
};

DEF_BENCH( return new CubicMapBench({1, 0}, {0,0}); )
DEF_BENCH( return new CubicMapBench({1, 0}, {0,1}); )
DEF_BENCH( return new CubicMapBench({1, 0}, {1,0}); )
DEF_BENCH( return new CubicMapBench({1, 0}, {1,1}); )

DEF_BENCH( return new CubicMapBench({0, 1}, {0,0}); )
DEF_BENCH( return new CubicMapBench({0, 1}, {0,1}); )
DEF_BENCH( return new CubicMapBench({0, 1}, {1,0}); )
DEF_BENCH( return new CubicMapBench({0, 1}, {1,1}); )

DEF_BENCH( return new CubicMapBench({0, 0}, {1,1}); )
DEF_BENCH( return new CubicMapBench({1, 1}, {0,0}); )

enum class CubicMode {
    kMitchell,
    kCatmullRom
};

class ScalePixelsBench : public Benchmark {
public:
    ScalePixelsBench(CubicMode mode) : fMode(mode) {
        fName.printf("scale_pixels_cubic_%s",
                     fMode == CubicMode::kMitchell ? "mitchell" : "catmull");
        SkImageInfo dst = SkImageInfo::Make(
                            1024, 1024, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                    src = dst.makeDimensions({512, 512});
        fSrc.alloc(src);
        fDst.alloc(dst);
    }

    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override {
        SkSamplingOptions sampling{fMode == CubicMode::kMitchell ? SkCubicResampler::Mitchell()
                                                                 : SkCubicResampler::CatmullRom()};
        for (int i = 0; i < loops; i++) {
            fSrc.scalePixels(fDst, sampling);
        }
    }

private:
    CubicMode           fMode;
    SkString            fName;
    SkAutoPixmapStorage fSrc, fDst;

    using INHERITED = Benchmark;
};

DEF_BENCH( return new ScalePixelsBench(CubicMode::kMitchell); )
DEF_BENCH( return new ScalePixelsBench(CubicMode::kCatmullRom); )
