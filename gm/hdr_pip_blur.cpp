#include "gm/gm.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRRect.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkImageFilters.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "tools/DecodeUtils.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#endif


static constexpr bool kDoBlur = false;
static constexpr bool kDoHDR = false;
static constexpr bool kDoColorMatrix = true;

static constexpr bool kSinglePass = true;


// Emulates rendering operations and color space conversions on Android running in HDR mode,
// mixing SDR background content, an HDR PiP "video", and an overall blur shade from a swipe.
class HDRPiPBlurGM : public skiagm::GM {
public:
    HDRPiPBlurGM() {}

protected:
    SkString getName() const override { return SkString("hdr-pip-blur"); }

    SkISize getISize() override { return {2560, 1440}; }

    bool runAsBench() const override { return true; }

    void onDraw(SkCanvas* canvas) override {
        SkIRect screenBounds = SkIRect::MakeWH(2560, 1440);

        auto scaledImagePaint = [this](sk_sp<SkImage> image) {
            SkPaint paint = fPaint;
            SkMatrix localMatrix = SkMatrix::RectToRect(SkRect::MakeXYWH(100.f, 100.f, 100.f, 100.f),
                                                        SkRect::Make(image->bounds()));
            sk_sp<SkShader> imageShader = SkShaders::Image(image,
                                                           SkTileMode::kClamp, SkTileMode::kClamp,
                                                           SkFilterMode::kLinear,
                                                           &localMatrix);
            paint.setShader(std::move(imageShader));
            return paint;
        };

        sk_sp<SkImage> input;
        {
            // The main surface is RGBA8 but with a wider gamut sRGB colorspace.
            // NOTE: Replace SkColorSpace::MakeRGB() with SkColorSpace::MakeSRGB() to switch
            sk_sp<SkSurface> content = nullptr;
            if (!kSinglePass) {
                content = canvas->makeSurface(
                    canvas->imageInfo().makeWH(screenBounds.width(), screenBounds.height())
                                       .makeColorType(kRGBA_8888_SkColorType)
                                       .makeColorSpace(
                                        kDoHDR ? SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020,
                                                                       SkNamedGamut::kDisplayP3)
                                               : SkColorSpace::MakeSRGB()));
            }
            SkCanvas* c = content ? content->getCanvas() : canvas;
            c->clear(SkColors::kDkGray);
            c->drawRect(SkRect::Make(screenBounds), scaledImagePaint(fBackgroundImage));
            // c->drawImageRect(fBackgroundImage.get(),
            //                  SkRect::Make(fBackgroundImage->bounds()),
            //                  SkRect::Make(screenBounds),
            //                  SkFilterMode::kLinear,
            //                  &fPaint,
            //                  SkCanvas::kFast_SrcRectConstraint);

            SkRRect pip = SkRRect::MakeRectXY(SkRect::MakeXYWH(1500.f, 700.f, 800.f, 600.f),
                                              20.f, 20.f);
            // c->drawRRect(pip, fPaint);
            c->save();
                // c->clipRRect(pip, true);
                c->drawRect(pip.rect(), scaledImagePaint(fPiPImage));
                // c->drawImageRect(fPiPImage.get(),
                //                 SkRect::MakeIWH(fPiPImage->width(), fPiPImage->height()),
                //                 pip.rect(),
                //                 SkFilterMode::kLinear,
                //                 &pipPaint,
                //                 SkCanvas::kFast_SrcRectConstraint);
            c->restore();

            if (content) {
                input = content->makeTemporaryImage();
            }
        }

        if (kSinglePass) {
            return;
        }

        canvas->save();
        canvas->clipIRect(screenBounds);
        canvas->drawRect(SkRect::Make(screenBounds), scaledImagePaint(input));
        // canvas->drawImage(input.get(), 0.f, 0.f, SkFilterMode::kLinear, &fPaint);

        if (kDoBlur) {
            // Explicitly blur the input image
            // NOTE: Skip calling MakeWithFilter and comment out the last `drawImageRect` call to
            // disable the shade blur.
            SkIRect outSubset;
            SkIPoint outOffset;
            sk_sp<SkImage> blur;
            if (canvas->recorder()) {
                blur = SkImages::MakeWithFilter(canvas->recorder(),
                                                input,
                                                fShadeBlur.get(),
                                                screenBounds, screenBounds,
                                                &outSubset, &outOffset);
            } else if (canvas->recordingContext()) {
                blur = SkImages::MakeWithFilter(canvas->recordingContext(),
                                                input,
                                                fShadeBlur.get(),
                                                screenBounds, screenBounds,
                                                &outSubset, &outOffset);
            } else {
                blur = SkImages::MakeWithFilter(input,
                                                fShadeBlur.get(),
                                                screenBounds, screenBounds,
                                                &outSubset, &outOffset);
            }


            SkPaint fadedBlur;
            fadedBlur.setAlphaf(0.9f);
            canvas->drawImageRect(blur.get(),
                                SkRect::Make(outSubset),
                                SkRect::MakeXYWH(outOffset.x(), outOffset.y(),
                                                outSubset.width(), outSubset.height()),
                                SkFilterMode::kLinear,
                                &fadedBlur,
                                SkCanvas::kFast_SrcRectConstraint);
        }

        canvas->restore();
    }

    void onOnceBeforeDraw() override {
        fPaint.setColor4f({ 0.0f, 0.0f, 0.0f, 1.f });

        // NOTE: comment this out to switch to SDR mode.
        SkColorMatrix cm;
        cm.setSaturation(1.5f);
        if (kDoColorMatrix) {
            fPaint.setColorFilter(SkColorFilters::Matrix(cm));
        }

        // The background image is standard sRGB
        fBackgroundImage = ToolUtils::GetResourceAsImage("images/yellow_rose.png")
                                    ->makeRasterImage(nullptr)
                                    ->makeColorSpace(nullptr, SkColorSpace::MakeSRGB());
        // The pip will be PQ-ish HDR
            // NOTE: Replace SkColorSpace::MakeRGB() with SkColorSpace::MakeSRGB() to switch to SDR
        fPiPImage = ToolUtils::GetResourceAsImage("images/mandrill_512.png")
                             ->makeRasterImage(nullptr)
                             ->makeColorSpace(nullptr,
                                              kDoHDR ?
                                              SkColorSpace::MakeRGB(SkNamedTransferFn::kPQ,
                                                                    SkNamedGamut::kRec2020)
                                              : SkColorSpace::MakeSRGB());

        fShadeBlur = SkImageFilters::Blur(32.f, 32.f, SkTileMode::kClamp, nullptr);
    }

private:
    sk_sp<SkImage> fBackgroundImage;
    sk_sp<SkImage> fPiPImage;

    sk_sp<SkImageFilter> fShadeBlur;
    SkPaint fPaint;
};
DEF_GM( return new HDRPiPBlurGM; )
