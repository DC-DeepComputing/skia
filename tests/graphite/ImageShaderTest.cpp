/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/shaders/SkImageShader.h"
#include "tools/ToolUtils.h"

namespace skgpu::graphite {

namespace {

constexpr SkColor4f kRectColor = SkColors::kRed;
constexpr SkColor4f kBgColor = SkColors::kTransparent;

struct Expectation {
    SkPoint pos;
    SkColor4f color;
};

void test_draw(skiatest::Reporter* reporter,
               Context* context,
               SkISize canvasSize,
               SkISize imageSize,
               SkRect srcRect,
               SkRect dstRect,
               SkTileMode tileMode,
               SkSamplingOptions samplingOptions,
               std::vector<Expectation> expectations) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    REPORTER_ASSERT(reporter, recorder);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(
            recorder.get(),
            SkImageInfo::Make(canvasSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    SkCanvas* canvas = surface->getCanvas();

    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::Make(imageSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                       0);
    bitmap.eraseColor(kRectColor);
    bitmap.setImmutable();
    sk_sp<SkImage> image = SkImages::TextureFromImage(recorder.get(), bitmap.asImage(),
                                                      {/*fMipmapped=*/false});

    SkPaint p;
    SkMatrix srcToDst = SkMatrix::RectToRectOrIdentity(srcRect, dstRect);
    p.setShader(SkImageShader::MakeSubset(
            std::move(image), srcRect, tileMode, tileMode, samplingOptions, &srcToDst));
    canvas->drawRect(dstRect, p);

    SkPixmap pm;

    SkBitmap result;
    result.allocPixels(SkImageInfo::Make(canvasSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bool peekPixelsSuccess = result.peekPixels(&pm);
    REPORTER_ASSERT(reporter, peekPixelsSuccess);

    bool readPixelsSuccess = surface->readPixels(pm, 0, 0);
    REPORTER_ASSERT(reporter, readPixelsSuccess);

    for (const Expectation& e : expectations) {
        SkColor4f a = e.color;
        SkColor4f b = pm.getColor4f(e.pos.fX, e.pos.fY);
        REPORTER_ASSERT(reporter,
                        a == b,
                        "At position {%.1f, %.1f}, "
                        "expected {%.1f, %.1f, %.1f, %.1f}, "
                        "found {%.1f, %.1f, %.1f, %.1f}",
                        e.pos.fX, e.pos.fY,
                        a.fR, a.fG, a.fB, a.fA,
                        b.fR, b.fG, b.fB, b.fA);
    }
}

}  // anonymous namespace

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(ImageShaderTest, reporter, context,
                                         CtsEnforcement::kApiLevel_202404) {
    // Test that a subset bound covering less than half of a pixel causes that pixel not to be
    // drawn when using decal tiling and nearest-neighbor filtering. In this case we have a subset
    // that covers 3/4 the pixel column at y=1, all of the y=2 column, and 1/4 the y=3 column.
    test_draw(reporter,
              context,
              /*canvasSize=*/SkISize::Make(100, 100),
              /*imageSize=*/SkISize::Make(4, 4),
              /*srcRect=*/SkRect::MakeLTRB(1.25, 0.0f, 3.25f, 2.0f),
              /*dstRect=*/SkRect::MakeLTRB(0, 0, 80, 80),
              SkTileMode::kDecal,
              SkSamplingOptions(),

              // Pixel that should sample the image at y=1, since that's where the subset starts.
              {{{0, 40}, kRectColor},
               // Pixel that would sample the image at y=3, but the subset bound at y=3.25 prevents
               // us from sampling the image.
               {{75, 40}, kBgColor}});
}

#if defined(SK_ENABLE_YUVA_PACKED_422)
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(ImageShaderPackedYUYVDecalTest, reporter, context,
                                         CtsEnforcement::kNever) {
    // Test that decal tiling on a packed 4:2:2 image is resolved per pixel, not per pixel pair,
    // when the subset bounds fall on odd pixels. The plane holds one texel per 2x1 block of pixels,
    // so pixels 2 and 3 (and 6 and 7) share a texel even though the subset splits them.
    constexpr int kWidth = 8;
    constexpr int kHeight = 2;

    // Channels 0..3 hold (Y0, U, Y1, V). Even pixels are white (Y0 = 1) and odd pixels are black
    // (Y1 = 0); U and V are neutral.
    SkBitmap plane;
    plane.allocPixels(SkImageInfo::Make(kWidth / 2,
                                        kHeight,
                                        kR16G16B16A16_unorm_SkColorType,
                                        kUnpremul_SkAlphaType));
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth / 2; ++x) {
            uint16_t* texel = static_cast<uint16_t*>(plane.getAddr(x, y));
            texel[0] = 0xFFFF;
            texel[1] = 0x8000;
            texel[2] = 0x0000;
            texel[3] = 0x8000;
        }
    }
    plane.setImmutable();

    SkYUVAInfo yuvaInfo({kWidth, kHeight},
                        SkYUVAInfo::PlaneConfig::kYUYV,
                        SkYUVAInfo::Subsampling::k422,
                        kJPEG_Full_SkYUVColorSpace);
    SkPixmap pixmaps[SkYUVAInfo::kMaxPlanes];
    pixmaps[0] = plane.pixmap();
    SkYUVAPixmaps yuvaPixmaps = SkYUVAPixmaps::FromExternalPixmaps(yuvaInfo, pixmaps);
    REPORTER_ASSERT(reporter, yuvaPixmaps.isValid());

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    REPORTER_ASSERT(reporter, recorder);
    sk_sp<SkImage> image = SkImages::TextureFromYUVAPixmaps(recorder.get(), yuvaPixmaps);
    if (!image) {
        // The backend can't texture from a 4-channel 16-bit unorm plane.
        return;
    }

    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(
            recorder.get(),
            SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);

    SkPaint p;
    p.setShader(SkImageShader::MakeSubset(std::move(image),
                                          SkRect::MakeLTRB(3, 0, 7, kHeight),
                                          SkTileMode::kDecal,
                                          SkTileMode::kDecal,
                                          SkSamplingOptions(),
                                          /*localMatrix=*/nullptr));
    canvas->drawRect(SkRect::MakeWH(kWidth, kHeight), p);

    SkBitmap result;
    result.allocPixels(SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType,
                                         kPremul_SkAlphaType));
    SkPixmap pm;
    REPORTER_ASSERT(reporter, result.peekPixels(&pm));
    REPORTER_ASSERT(reporter, surface->readPixels(pm, 0, 0));

    // Pixels 3..6 are inside the subset: odd pixels take Y1 (black), even pixels take Y0 (white).
    // Pixels 2 and 7 are outside of it and must be transparent, even though they share a texel with
    // pixels inside the subset.
    struct Expectation {
        int x;
        float alpha;
        float luma;
    };
    const Expectation kExpectations[] = {{2, 0.f, 0.f},
                                         {3, 1.f, 0.f},
                                         {4, 1.f, 1.f},
                                         {5, 1.f, 0.f},
                                         {6, 1.f, 1.f},
                                         {7, 0.f, 0.f}};
    for (int y = 0; y < kHeight; ++y) {
        for (const Expectation& e : kExpectations) {
            SkColor4f c = pm.getColor4f(e.x, y);
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(c.fA, e.alpha, 0.02f) &&
                            SkScalarNearlyEqual(c.fR, e.luma * e.alpha, 0.02f),
                            "At position {%d, %d}, expected alpha %.1f and luma %.1f, "
                            "found {%.2f, %.2f, %.2f, %.2f}",
                            e.x, y, e.alpha, e.luma, c.fR, c.fG, c.fB, c.fA);
        }
    }
}
#endif  // SK_ENABLE_YUVA_PACKED_422

}  // namespace skgpu::graphite
