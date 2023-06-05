/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkStream.h"
#include "include/effects/SkGradientShader.h"
#include "include/encode/SkPngEncoder.h"
#include "src/core/SkSamplingPriv.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"


#if defined(SK_GANESH)
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tests/CtsEnforcement.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Caps.h"          //%%
#include "src/gpu/graphite/ContextPriv.h"   //%%
#endif

extern int gOverrideMaxTextureSize;
extern int gNumTilesDrawn;

namespace {

class HilbertGenerator {
public:
    HilbertGenerator(float desiredSize, float desiredWidth, int desiredDepth)
            : fDesiredSize(desiredSize)
            , fDesiredDepth(desiredDepth)
            , fLength(fDesiredSize / ((0x1 << fDesiredDepth) - 1.0f))
            , fDesiredWidth(desiredWidth)
            , fActualBounds(SkRect::MakeEmpty())
            , fCurPos(SkPoint::Make(0.0f, 0.0f))
            , fCurDir(0)
            , fExpectedLen(fLength * ((0x1 << (2*fDesiredDepth)) - 1.0f))
            , fCurLen(0.0f) {
    }

    void draw(SkCanvas* canvas) {
        this->recursiveDraw(canvas, /* curDepth= */ 0, /* turnRight= */ true);

        SkScalarNearlyEqual(fExpectedLen, fCurLen, 0.01f);
        SkScalarNearlyEqual(fDesiredSize, fActualBounds.width(), 0.01f);
        SkScalarNearlyEqual(fDesiredSize, fActualBounds.height(), 0.01f);
    }

private:
    void turn90(bool turnRight) {
        fCurDir += turnRight ? 90 : -90;
        if (fCurDir >= 360) {
            fCurDir = 0;
        } else if (fCurDir < 0) {
            fCurDir = 270;
        }

        SkASSERT(fCurDir == 0 || fCurDir == 90 || fCurDir == 180 || fCurDir == 270);
    }

#if 0
    void curve(SkCanvas* canvas) {
        SkRect r;
        SkPaint paint;
        paint.setColor(SK_ColorRED);

        canvas->drawArc(r, 0, 360, /* useCenter= */ false, paint);
    }
#endif

    void line(SkCanvas* canvas) {

        SkPoint before = fCurPos;

        SkRect r;
        switch (fCurDir) {
            case 0:
                r.fLeft = fCurPos.fX;
                r.fTop = fCurPos.fY - fDesiredWidth / 2.0f;
                r.fRight = fCurPos.fX + fLength;
                r.fBottom = fCurPos.fY + fDesiredWidth / 2.0f;
                fCurPos.fX += fLength;
                break;
            case 90:
                r.fLeft = fCurPos.fX - fDesiredWidth / 2.0f;
                r.fTop = fCurPos.fY - fLength;
                r.fRight = fCurPos.fX + fDesiredWidth / 2.0f;
                r.fBottom = fCurPos.fY;
                fCurPos.fY -= fLength;
                break;
            case 180:
                r.fLeft = fCurPos.fX - fLength;
                r.fTop = fCurPos.fY - fDesiredWidth / 2.0f;
                r.fRight = fCurPos.fX;
                r.fBottom = fCurPos.fY + fDesiredWidth / 2.0f;
                fCurPos.fX -= fLength;
                break;
            case 270:
                r.fLeft = fCurPos.fX - fDesiredWidth / 2.0f;
                r.fTop = fCurPos.fY;
                r.fRight = fCurPos.fX + fDesiredWidth / 2.0f;
                r.fBottom = fCurPos.fY + fLength;
                fCurPos.fY += fLength;
                break;
            default:
                return;
        }

        SkPoint pts[2] = { before, fCurPos };

        SkColor4f colors[2] = {
                this->getColor(fCurLen),
                this->getColor(fCurLen + fLength),
        };

        fCurLen += fLength;
        if (fActualBounds.isEmpty()) {
            fActualBounds = r;
        } else {
            fActualBounds.join(r);
        }

        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        paint.setShader(SkGradientShader::MakeLinear(pts, colors, /* colorSpace= */ nullptr,
                /* pos= */ nullptr, 2, SkTileMode::kClamp));
        canvas->drawRect(r, paint);
    }

    void recursiveDraw(SkCanvas* canvas, int curDepth, bool turnRight) {
        if (curDepth >= fDesiredDepth) {
            return;
        }

        this->turn90(turnRight);
        this->recursiveDraw(canvas, curDepth + 1, !turnRight);
        this->line(canvas);
        this->turn90(!turnRight);
        this->recursiveDraw(canvas, curDepth + 1, turnRight);
        this->line(canvas);
        this->recursiveDraw(canvas, curDepth + 1, turnRight);
        this->turn90(!turnRight);
        this->line(canvas);
        this->recursiveDraw(canvas, curDepth + 1, !turnRight);
        this->turn90(turnRight);
    }

    SkColor4f getColor(float curLen) {
        static const SkColor4f kColors[] = {
                SkColors::kBlack,
                SkColors::kBlue,
                SkColors::kCyan,
                SkColors::kGreen,
                SkColors::kYellow,
                SkColors::kRed,
                SkColors::kWhite,
        };

        static const float kStops[] = {
                0.0f,
                1.0f/6.0f,
                2.0f/6.0f,
                0.5f,
                4.0f/6.0f,
                5.0f/6.0f,
                1.0f,
        };
        static_assert(std::size(kColors) == std::size(kStops));

        float t = curLen / fExpectedLen;
        if (t <= 0.0f) {
            return kColors[0];
        } else if (t >= 1.0f) {
            return kColors[std::size(kColors)-1];
        }

        for (unsigned int i = 0; i < std::size(kColors)-1; ++i) {
            if (kStops[i] <= t && t <= kStops[i+1]) {
                t = (t - kStops[i]) / (kStops[i+1] - kStops[i]);
                SkASSERT(0.0f <= t && t <= 1.0f);
                return { kColors[i].fR * (1 - t) + kColors[i+1].fR * t,
                         kColors[i].fG * (1 - t) + kColors[i+1].fG * t,
                         kColors[i].fB * (1 - t) + kColors[i+1].fB * t,
                         kColors[i].fA * (1 - t) + kColors[i+1].fA * t };

            }
        }

        return SkColors::kBlack;
    }

    const float fDesiredSize;
    const int fDesiredDepth;
    const float fLength;            // length of a line segment
    const float fDesiredWidth;

    SkRect fActualBounds;

    // The "turtle" state
    SkPoint fCurPos;
    int fCurDir;

    const float fExpectedLen;
    float fCurLen;
};

sk_sp<SkImage> make_big_bitmap_image(int size, int desiredWidth, int desiredDepth) {
    SkBitmap bm;

    bm.allocN32Pixels(size, size, /* isOpaque= */ true);

    SkCanvas canvas(bm);

    canvas.clear(SK_ColorDKGRAY);

    HilbertGenerator gen(/* desiredSize= */ size - 2 * desiredWidth, desiredWidth, desiredDepth);

    canvas.translate(desiredWidth, size - desiredWidth);
    gen.draw(&canvas);

    return bm.asImage();
}

void potentially_write_to_png(const char* directory, int id, const SkBitmap& bm) {
#if 1
    SkString filename;
    filename.appendf("//Users//robertphillips//src//bugs//%s//%d.png", directory, id);
    SkDebugf("filename: %s\n", filename.c_str());

    SkFILEWStream file(filename.c_str());
    SkAssertResult(file.isValid());

    SkAssertResult(SkPngEncoder::Encode(&file, bm.pixmap(), {}));
#endif
}

bool check_pixels(skiatest::Reporter* reporter,
                  const SkBitmap& expected,
                  const SkBitmap& actual,
                  const SkSamplingOptions& sampling,
                  int scale) {
    static const float kTols[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    const char* samplingStr = nullptr;
    if (sampling.isAniso()) {
        samplingStr = "Aniso";
    } else if (sampling.useCubic) {
        samplingStr = "Cubic";
    } else if (sampling.mipmap != SkMipmapMode::kNone) {
        samplingStr = "Mipmap";
    } else if (sampling.filter == SkFilterMode::kLinear) {
        samplingStr = "Linear";
    } else {
        samplingStr = "NN";
    }

    auto error = std::function<ComparePixmapsErrorReporter>(
            [&](int x, int y, const float diffs[4]) {
                SkASSERT(x >= 0 && y >= 0);
                ERRORF(reporter, "%s %d: mismatch at %d, %d (%f, %f, %f %f)",
                       samplingStr, scale,
                       x, y, diffs[0], diffs[1], diffs[2], diffs[3]);
            });

    return ComparePixels(expected.pixmap(), actual.pixmap(), kTols, error);
}

} // anonymous namespace


#if defined(SK_GANESH)

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(BigImageTest_Ganesh,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();

    // We're using the knowledge that the internal tile size
    // is 1024. By creating kImageSize sized images we know we'll get a 4x4 tiling regardless of
    // the sampling.
    static const int kImageSize = 4096 - 4 * 2 * kBicubicFilterTexelPad;
    static const int kOverrideMaxTextureSize = 1024;
    static const int kExpectedNumTiles = 4*4;

    if (dContext->maxTextureSize() < kImageSize) {
        // For the expected images we need to be able to draw w/o tiling
        return;
    }

    sk_sp<SkImage> img = make_big_bitmap_image(kImageSize,
                                               /* desiredWidth= */ 16,
                                               /* desiredDepth= */ 7);

    const SkSamplingOptions kSamplingOptions[] = {
        // TODO: NN is currently broken!
        //SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone),
        SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone),
        // TODO: Mipmapping is currently broken!
        //SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear),
        SkSamplingOptions(SkCubicResampler::CatmullRom()),
    };

    int caseID = 0;
    for (auto sampling : kSamplingOptions) {
        for (int scale : { /* 1, 4,*/ 8 }) {

            auto destII = SkImageInfo::Make(kImageSize/scale, kImageSize/scale,
                                            kRGBA_8888_SkColorType, kPremul_SkAlphaType);

            SkBitmap expected, actual;
            expected.allocPixels(destII);
            actual.allocPixels(destII);

            SkRect destRect = SkRect::MakeWH(destII.width(), destII.height());
            sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo,
                                                                destII);
            SkCanvas* canvas = surface->getCanvas();

            // First, draw w/o tiling
            gOverrideMaxTextureSize = 0;
            gNumTilesDrawn = 0;

            canvas->drawImageRect(img, destRect, sampling);
            SkAssertResult(surface->readPixels(expected, 0, 0));
            REPORTER_ASSERT(reporter, gNumTilesDrawn == 0);

            potentially_write_to_png("expected", caseID, expected);

            // Then, force 4x4 tiling
            gOverrideMaxTextureSize = kOverrideMaxTextureSize;

            canvas->drawImageRect(img, destRect, sampling);
            SkAssertResult(surface->readPixels(actual, 0, 0));
            REPORTER_ASSERT(reporter, gNumTilesDrawn == kExpectedNumTiles);

            potentially_write_to_png("actual", caseID, actual);

            REPORTER_ASSERT(reporter, check_pixels(reporter, expected, actual, sampling, scale));

            ++caseID;
        }
    }
}

#endif // SK_GANESH

#if defined(SK_GRAPHITE)

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(BigImageTest_Graphite,
                                         reporter,
                                         context) {
    using namespace skgpu::graphite;

    const Caps* caps = context->priv().caps();

    const int kRealMaxTextureSize = caps->maxTextureSize();

    // In all cases we'll be creating kImageSize sized images
    static const int kImageSize = 2048;
    if (kRealMaxTextureSize < kImageSize) {
        return;
    }

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    auto ii = SkImageInfo::Make(kImageSize, kImageSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii);


}

#endif // SK_GRAPHITE
