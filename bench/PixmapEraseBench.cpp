/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkString.h"

class PixmapEraseBench : public Benchmark {
    SkColorType fColorType;
    SkBitmap    fBitmap;
    SkPixmap    fPixmap;
    SkString    fName;

public:
    PixmapEraseBench(SkColorType ct, const char* name) : fColorType(ct) {
        fName.printf("pixmap_erase_%s", name);
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

    void onDelayedSetup() override {
        fBitmap.allocPixels(SkImageInfo::Make(1024, 1024, fColorType, kPremul_SkAlphaType));
        fBitmap.peekPixels(&fPixmap);
    }

    void onDraw(int loops, SkCanvas*) override {
        SkColor4f color = { 1.0f, 0.5f, 0.25f, 1.0f };
        for (int i = 0; i < loops; i++) {
            fPixmap.erase(color);
        }
    }

private:
    using INHERITED = Benchmark;
};

DEF_BENCH(return new PixmapEraseBench(kRGBA_8888_SkColorType, "8888");)
DEF_BENCH(return new PixmapEraseBench(kRGBA_F16_SkColorType, "F16");)
DEF_BENCH(return new PixmapEraseBench(kRGBA_F32_SkColorType, "F32");)
DEF_BENCH(return new PixmapEraseBench(kRGB_565_SkColorType, "565");)
DEF_BENCH(return new PixmapEraseBench(kAlpha_8_SkColorType, "A8");)
