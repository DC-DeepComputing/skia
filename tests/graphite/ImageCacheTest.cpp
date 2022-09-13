/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkPictureRecorder.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recording.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "tests/TestUtils.h"

using namespace skgpu::graphite;

namespace {

// This test creates a bunch of solid yellow images in different ways and then draws them into a
// smaller surface (w/ src mode) that has been initialized to solid blue. This tests out the
// image cache since Graphite has some specific expectations about which automatic conversions
// will happen. To be specific, by default:
//
//    raster-backed image                                   --> dropped draw (blue)
//
//    non-mipmapped Graphite-backed drawn w/ mipmapping     --> mipmapping is dropped (yellow)
//    non-mipmapped Graphite-backed drawn w/o mipmapping    --> drawn (yellow)
//
//    mipmapped Graphite-backed drawn w/ mipmapping         --> drawn (yellow)
//    mipmapped Graphite-backed drawn w/o mipmapping        --> drawn (yellow)
//
//    picture-backed image mipmapped and non-mipmapped      --> drawn (yellow)
//
//    SkBitmap-backed generator-based image w/ and w/o mips --> drawn (yellow)
//
const SkISize kSurfaceSize = { 16, 16 };
const SkISize kImageSize = { 32, 32 };

constexpr SkColor4f kImageColor = SkColors::kYellow;
constexpr SkColor4f kBackgroundColor = SkColors::kBlue;

sk_sp<SkImage> create_picture_backed_image() {
    SkIRect r = SkIRect::MakeWH(kImageSize.width(), kImageSize.height());
    SkPaint paint;
    paint.setColor(kImageColor);

    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(SkRect::Make(r));
    canvas->drawIRect(r, paint);
    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();

    return SkImage::MakeFromPicture(std::move(picture),
                                    r.size(),
                                    /* matrix= */ nullptr,
                                    /* paint= */ nullptr,
                                    SkImage::BitDepth::kU8,
                                    SkColorSpace::MakeSRGB());
}

class BitmapBackedGenerator final : public SkImageGenerator {
public:
    BitmapBackedGenerator()
            : SkImageGenerator(SkImageInfo::Make(kImageSize.width(),
                                                 kImageSize.height(),
                                                 kRGBA_8888_SkColorType,
                                                 kPremul_SkAlphaType)) {
    }

    bool onGetPixels(const SkImageInfo& dstInfo,
                     void* pixels,
                     size_t rowBytes,
                     const Options&) override {

        if (dstInfo.dimensions() != kImageSize) {
            return false;
        }

        SkBitmap bm;
        if (!bm.tryAllocPixels(dstInfo)) {
            return false;
        }

        bm.eraseColor(kImageColor);

        return bm.readPixels(dstInfo, pixels, rowBytes, 0, 0);
    }

private:

};

sk_sp<SkImage> create_bitmap_generator_image() {
    std::unique_ptr<SkImageGenerator> gen(new BitmapBackedGenerator());

    return SkImage::MakeFromGenerator(std::move(gen));
}

bool check_img(skiatest::Reporter* reporter, Context* context, Recorder* recorder,
               SkImage* imageToDraw, const SkColor4f& expectedColor) {
    SkImageInfo ii = SkImageInfo::Make(kSurfaceSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    SkBitmap result;
    result.allocPixels(ii);
    SkPixmap pm;

    SkAssertResult(result.peekPixels(&pm));

    {
        sk_sp<SkSurface> surface = SkSurface::MakeGraphite(recorder, ii);
        if (!surface) {
            ERRORF(reporter, "Surface creation failed");
            return false;
        }

        SkCanvas* canvas = surface->getCanvas();

        canvas->clear(kBackgroundColor);
        canvas->drawImage(imageToDraw, -8, -8);

        // For now, we cast and call directly into Surface. Once we have a better idea of
        // what the public API for synchronous graphite readPixels we can update this call to use
        // that instead.
        if (!static_cast<skgpu::graphite::Surface*>(surface.get())->onReadPixels(context,
                                                                                 recorder,
                                                                                 pm,
                                                                                 0,
                                                                                 0)) {
            ERRORF(reporter, "readPixels failed");
            return false;
        }
    }

    std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
    if (!recording) {
        ERRORF(reporter, "snapping a Recording failed");
        return false;
    }

    skgpu::graphite::InsertRecordingInfo info;
    info.fRecording = recording.get();
    context->insertRecording(info);
    context->submit(skgpu::graphite::SyncToCpu::kYes);

    auto error = std::function<ComparePixmapsErrorReporter>(
            [reporter](int x, int y, const float diffs[4]) {
                ERRORF(reporter, "Mismatch of (%f, %f, %f, %f)",
                       diffs[0], diffs[1], diffs[2], diffs[3]);
            });
    static constexpr float kTol[] = {0, 0, 0, 0};
    CheckSolidPixels(expectedColor, pm, kTol, error);

    return true;
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(ImageCacheTest_Graphite, reporter, context) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    {
        sk_sp<SkImage> image = create_picture_backed_image();

        check_img(reporter, context, recorder.get(), image.get(), kImageColor);
    }

    {
        sk_sp<SkImage> image = create_bitmap_generator_image();

        check_img(reporter, context, recorder.get(), image.get(), kImageColor);
    }
}
