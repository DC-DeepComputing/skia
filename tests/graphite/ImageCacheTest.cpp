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
#include "include/core/SkSpan.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recording.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"

using namespace skgpu::graphite;

namespace {

const SkISize kSurfaceSize = { 16, 16 };
const SkISize kImageSize = { 32, 32 };

constexpr SkColor4f kImageColor = SkColors::kYellow;
constexpr SkColor4f kBackgroundColor = SkColors::kBlue;

/* 1 */
sk_sp<SkImage> create_raster_backed_image(Recorder*) {
    SkImageInfo ii = SkImageInfo::Make(kImageSize.width(),
                                       kImageSize.height(),
                                       kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);
    SkBitmap bm;
    if (!bm.tryAllocPixels(ii)) {
        return nullptr;
    }

    bm.eraseColor(kImageColor);

    return SkImage::MakeFromBitmap(bm);
}

/* 2 */
sk_sp<SkImage> create_gpu_backed_image_no_mipmaps(Recorder* recorder) {
    sk_sp<SkImage> raster = create_raster_backed_image(recorder);
    return raster->makeTextureImage(recorder, { Mipmapped::kNo });
}

/* 2 */
sk_sp<SkImage> create_gpu_backed_image_with_mipmaps(Recorder* recorder) {
    sk_sp<SkImage> raster = create_raster_backed_image(recorder);
    return raster->makeTextureImage(recorder, { Mipmapped::kYes });
}

/* 4 */
sk_sp<SkImage> create_picture_backed_image(Recorder*) {
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

/* 5 */
sk_sp<SkImage> create_bitmap_generator_image(Recorder*) {

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
    };

    std::unique_ptr<SkImageGenerator> gen(new BitmapBackedGenerator());

    return SkImage::MakeFromGenerator(std::move(gen));
}

bool check_img(skiatest::Reporter* reporter,
               Context* context,
               Recorder* recorder,
               SkImage* imageToDraw,
               Mipmapped mipmapped,
               const char* testcase,
               const SkColor4f& expectedColor) {
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
        if (mipmapped == Mipmapped::kYes) {
            canvas->drawImageRect(imageToDraw,
                                  SkRect::MakeWH(kSurfaceSize.width(), kSurfaceSize.height()),
                                  SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear));
        } else {
            canvas->drawImage(imageToDraw, -8, -8);
        }

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

    SkDebugf("testcase %s %s: expecting (%.1f %.1f %.1f %.1f)\n",
             testcase,
             (mipmapped == Mipmapped::kYes) ? "w/ mipmaps" : "w/o mipmaps",
             expectedColor.fR, expectedColor.fG, expectedColor.fB, expectedColor.fA);

    auto error = std::function<ComparePixmapsErrorReporter>(
            [reporter](int x, int y, const float diffs[4]) {
                ERRORF(reporter, "Mismatch of (%.1f, %.1f, %.1f, %.1f)",
                       diffs[0], diffs[1], diffs[2], diffs[3]);
            });
    static constexpr float kTol[] = {0, 0, 0, 0};
    CheckSolidPixels(expectedColor, pm, kTol, error);

    return true;
}

using FactoryT = sk_sp<SkImage> (*)(Recorder*);

struct TestCase {
    const char* fTestCase;
    FactoryT    fFactory;
    SkColor4f   fExpectedColors[2];   /* [ w/o mipmaps, w/ mipmaps ] */
};

void run_test(skiatest::Reporter* reporter,
              Context* context,
              Recorder* recorder,
              SkSpan<TestCase> testcases) {

    for (auto t : testcases) {
        for (auto mm : { Mipmapped::kNo, Mipmapped::kYes }) {
            sk_sp<SkImage> image = t.fFactory(recorder);

            check_img(reporter, context, recorder, image.get(), mm,
                      t.fTestCase, t.fExpectedColors[static_cast<int>(mm)]);
        }
    }
}

} // anonymous namespace

// This test creates a bunch of solid yellow images in different ways and then draws them into a
// smaller surface (w/ src mode) that has been initialized to solid blue. This tests out the
// image cache since Graphite has some specific expectations about which automatic conversions
// will happen.

// For the default ImageCache the expectations are:
//
//    1) raster-backed image                                   --> dropped draw (blue)
//
//    2) non-mipmapped Graphite-backed drawn w/ mipmapping     --> mipmapping is dropped (yellow)
//       non-mipmapped Graphite-backed drawn w/o mipmapping    --> drawn (yellow)
//
//    3) mipmapped Graphite-backed drawn w/ mipmapping         --> drawn (yellow)
//       mipmapped Graphite-backed drawn w/o mipmapping        --> drawn (yellow)
//
//    4) picture-backed image mipmapped and non-mipmapped      --> drawn (yellow)
//
//    5) SkBitmap-backed generator-based image w/ and w/o mips --> drawn (yellow)
//
DEF_GRAPHITE_TEST_FOR_CONTEXTS(ImageCacheTest_Graphite_Default, reporter, context) {
    TestCase testcases[] = {
        { "1", create_raster_backed_image,           { kBackgroundColor, kBackgroundColor } },
        { "2", create_gpu_backed_image_no_mipmaps,   { kImageColor,      kImageColor } },
        { "3", create_gpu_backed_image_with_mipmaps, { kImageColor,      kImageColor } },
        { "4", create_picture_backed_image,          { kImageColor,      kImageColor } },
        { "5", create_bitmap_generator_image,        { kImageColor,      kImageColor }  },
    };

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    run_test(reporter, context, recorder.get(), testcases);
}

// For the testing ImageCache the expectations are:
//
//    1) raster-backed image                                   --> dropped draw (blue)
//
//    2) non-mipmapped Graphite-backed drawn w/ mipmapping     --> mipmapping is dropped (yellow)
//       non-mipmapped Graphite-backed drawn w/o mipmapping    --> drawn (yellow)
//
//    3) mipmapped Graphite-backed drawn w/ mipmapping         --> drawn (yellow)
//       mipmapped Graphite-backed drawn w/o mipmapping        --> drawn (yellow)
//
//    4) picture-backed image mipmapped and non-mipmapped      --> drawn (yellow)
//
//    5) SkBitmap-backed generator-based image w/ and w/o mips --> drawn (yellow)
//
DEF_GRAPHITE_TEST_FOR_CONTEXTS(ImageCacheTest_Graphite_Testing, reporter, context) {
    TestCase testcases[] = {
        { "1", create_raster_backed_image,           { kBackgroundColor, kBackgroundColor } },
        { "2", create_gpu_backed_image_no_mipmaps,   { kImageColor,      kImageColor } },
        { "3", create_gpu_backed_image_with_mipmaps, { kImageColor,      kImageColor } },
        { "4", create_picture_backed_image,          { kImageColor,      kImageColor } },
        { "5", create_bitmap_generator_image,        { kImageColor,      kImageColor }  },
    };

    std::unique_ptr<skgpu::graphite::Recorder> recorder =
            context->makeRecorder(ToolUtils::CreateTestingRecorderOptions());

    run_test(reporter, context, recorder.get(), testcases);
}
