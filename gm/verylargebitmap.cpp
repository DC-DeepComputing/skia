/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkGradientShader.h"
#include "tools/ToolUtils.h"

#if defined(SK_GANESH)
#include "include/gpu/GrRecordingContext.h"
#endif

static constexpr int kPad = 10;
static constexpr int kCellSize = 128;

static void draw(SkCanvas* canvas, int width, int height, SkColor colors[2]) {
    const SkPoint center = { SkIntToScalar(width)/2, SkIntToScalar(height)/2 };
    const SkScalar radius = 40;
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeRadial(center, radius, colors, nullptr, 2,
                                                 SkTileMode::kMirror));
    paint.setBlendMode(SkBlendMode::kSrc);
    canvas->drawPaint(paint);
}

static sk_sp<SkImage> make_raster_image(int width, int height, SkColor colors[2]) {
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(width, height)));
    if (!surface) {
        return nullptr;
    }
    draw(surface->getCanvas(), width, height, colors);
    return surface->makeImageSnapshot();
}

static sk_sp<SkImage> make_picture_image(int width, int height, SkColor colors[2]) {
    SkPictureRecorder recorder;
    draw(recorder.beginRecording(SkRect::MakeIWH(width, height)), width, height, colors);
    return SkImages::DeferredFromPicture(recorder.finishRecordingAsPicture(),
                                         {width, height},
                                         nullptr,
                                         nullptr,
                                         SkImages::BitDepth::kU8,
                                         SkColorSpace::MakeSRGB());
}

typedef sk_sp<SkImage> (*ImageMakerProc)(int width, int height, SkColor colors[2]);

static int gimme(SkCanvas* canvas, SkImage* img) {

#if defined(SK_GANESH)
    if (GrRecordingContext* rContext = canvas->recordingContext(); rContext) {
        return rContext->maxTextureSize();
//        if (img->width() >= rContext->maxTextureSize() ||
//            img->height() >= rContext->maxTextureSize()) {
//            return true;
//        }
    }
#endif
#if defined(SK_GRAPHITE)
    if (skgpu::graphite::Recorder* recorder = canvas->recorder(); recorder) {
        return recorder->maxTextureSize();
//        if (img->width() >= recorder->maxTextureSize() ||
//            img->height() >= recorder->maxTextureSize()) {
//            return true;
//        }
    }
#endif

    return 1 << 11;
//    return false;
}

static void draw_too_big(SkCanvas* canvas, sk_sp<SkImage> img) {


}

static void show_image(SkCanvas* canvas, int width, int height, SkColor colors[2],
                       ImageMakerProc proc) {

    SkDebugf("--------- %dx%d == %d\n", width, height, width*height);
    sk_sp<SkImage> tmpImg = proc(width, height, colors);
    if (!tmpImg) {
        return;
    }

    int max = gimme(canvas, tmpImg.get());
    SkDebugf("maxTextureSize: %d\n", max);
//        draw_too_big(canvas, std::move(tmpImg));
//        return;
//    }

    sk_sp<SkImage> image = ToolUtils::MakeTextureImage(canvas, std::move(tmpImg));
    if (!image) {
        return;
    }

    SkPaint borderPaint;

    borderPaint.setStyle(SkPaint::kStroke_Style);

    SkRect dstRect = SkRect::MakeWH(128.f, 128.f);

    canvas->save();
    canvas->clipRect(dstRect);
    canvas->drawImage(image, 0, 0);
    canvas->restore();
    canvas->drawRect(dstRect, borderPaint);

    dstRect.offset(kCellSize + kPad, 0);
    int hw = width / 2;
    int hh = height / 2;
    SkRect subset = SkRect::MakeLTRB(hw - 64, hh - 32, hw + 64, hh + 32);
    canvas->drawImageRect(image, subset, dstRect, SkSamplingOptions(), nullptr,
                          SkCanvas::kStrict_SrcRectConstraint);
    canvas->drawRect(dstRect, borderPaint);

    dstRect.offset(kCellSize + kPad, 0);
    canvas->drawImageRect(image, dstRect, SkSamplingOptions(), nullptr);
    canvas->drawRect(dstRect, borderPaint);
}

class VeryLargeBitmapGM : public skiagm::GM {
    ImageMakerProc  fProc;
    const char*     fName;

public:
    VeryLargeBitmapGM(ImageMakerProc proc, const char name[]) : fProc(proc), fName(name) {}

private:
    SkString onShortName() override { return SkString(fName); }

    SkISize onISize() override { return { 3*kCellSize + 4*kPad, 5*kCellSize + 6*kPad }; }

    void onDraw(SkCanvas* canvas) override {
        const int maxTexSize = gimme(canvas, nullptr);

        int veryBig = 2*maxTexSize;
        int big = maxTexSize;
        int medium = maxTexSize / 2;
        int small = 128;

        SkColor colors[2];

        canvas->translate(kPad, kPad);
        colors[0] = SK_ColorRED;
        colors[1] = SK_ColorGREEN;
        show_image(canvas, small, small, colors, fProc);
        canvas->translate(0, kCellSize + kPad);

        colors[0] = SK_ColorBLUE;
        colors[1] = SK_ColorMAGENTA;
        show_image(canvas, big, small, colors, fProc);
        canvas->translate(0, kCellSize + kPad);

        colors[0] = SK_ColorMAGENTA;
        colors[1] = SK_ColorYELLOW;
        show_image(canvas, medium, medium, colors, fProc);
        canvas->translate(0, kCellSize + kPad);

        colors[0] = SK_ColorGREEN;
        colors[1] = SK_ColorYELLOW;
        // This used to be big enough that we didn't draw on CPU, but now we do.
        show_image(canvas, veryBig, small, colors, fProc);
        canvas->translate(0, kCellSize + kPad);

        colors[0] = SK_ColorCYAN;
        colors[1] = SK_ColorBLUE;
        show_image(canvas, big, big, colors, fProc);
    }
};

DEF_GM( return new VeryLargeBitmapGM(make_raster_image, "verylargebitmap"); )
DEF_GM( return new VeryLargeBitmapGM(make_picture_image, "verylarge_picture_image"); )
