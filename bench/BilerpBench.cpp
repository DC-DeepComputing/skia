/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "tools/ToolUtils.h"

class BilerpBench : public Benchmark {
public:
    BilerpBench(float scale)
            : fSrcW(3840), fSrcH(2160), fDstW(3840 * scale), fDstH(2160 * scale), fRec709(false) {
        fName.printf("bilerp_4k_scale_%0.2f", scale);
    }

    BilerpBench(int srcW, int srcH, int dstW, int dstH, bool rec709 = false)
            : fSrcW(srcW), fSrcH(srcH), fDstW(dstW), fDstH(dstH), fRec709(rec709) {
        fName.printf("bilerp_%dx%d_to_%dx%d%s", srcW, srcH, dstW, dstH, rec709 ? "_rec709" : "");
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    SkISize onGetSize() override {
        return SkISize::Make(SkScalarCeilToInt(fDstW), SkScalarCeilToInt(fDstH));
    }

    void onDelayedSetup() override {
        fImage = ToolUtils::create_checkerboard_image(
                fSrcW, fSrcH, SK_ColorGRAY, SK_ColorDKGRAY, 16);
        if (fRec709) {
            fImage = fImage->reinterpretColorSpace(
                    SkColorSpace::MakeRGB(SkNamedTransferFn::kRec709, SkNamedGamut::kSRGB));
            SkImageInfo dstInfo = SkImageInfo::MakeN32Premul(
                    SkScalarCeilToInt(fDstW), SkScalarCeilToInt(fDstH), SkColorSpace::MakeSRGB());
            fSrgbSurface = SkSurfaces::Raster(dstInfo);
        }
        fDst = SkRect::MakeWH(fDstW, fDstH);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkCanvas* targetCanvas = fRec709 ? fSrgbSurface->getCanvas() : canvas;
        SkPaint paint;
        SkSamplingOptions sampling(SkFilterMode::kLinear, SkMipmapMode::kNone);
        for (int i = 0; i < loops; i++) {
            targetCanvas->drawImageRect(fImage, fDst, sampling, &paint);
        }
    }

private:
    SkString fName;
    sk_sp<SkImage> fImage;
    sk_sp<SkSurface> fSrgbSurface;
    SkRect fDst;
    int fSrcW, fSrcH;
    float fDstW, fDstH;
    bool fRec709;
    using INHERITED = Benchmark;
};

DEF_BENCH(return new BilerpBench(1.00f);)
DEF_BENCH(return new BilerpBench(0.99f);)
DEF_BENCH(return new BilerpBench(0.75f);)
DEF_BENCH(return new BilerpBench(0.50f);)
DEF_BENCH(return new BilerpBench(0.33f);)
DEF_BENCH(return new BilerpBench(1920, 1080, 3840, 2160, false);)
DEF_BENCH(return new BilerpBench(1920, 1080, 3840, 2160, true);)
