/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "modules/skresources/src/SkAnimCodecPlayer.h"
#include "src/core/SkOSFile.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/timer/TimeUtils.h"

#include <memory>
#include <utility>
#include <vector>

#if defined(SK_ENABLE_SKOTTIE)

static DEFINE_string(animatedWebp, "images/fb19ed.webp", "Animated webp in resources folder");

class AnimatedWebpGM : public skiagm::GM {
private:
    std::unique_ptr<SkCodec>        fCodec;
    int                             fFrame;
    double                          fNextUpdate;
    int                             fTotalFrames;
    std::vector<SkCodec::FrameInfo> fFrameInfos;
    std::vector<SkBitmap>           fFrames;
    float                           fScale = 0.75;

    void drawFrame(SkCanvas* canvas, int frameIndex) {
        if (frameIndex >= (int) fFrames.size()) {
            fFrames.resize(frameIndex + 1);
        }
        SkBitmap& bm = fFrames[frameIndex];
        if (!bm.getPixels()) {
            const auto unscaledSize = fCodec->getInfo().dimensions();
            SkISize scaledSize = { SkScalarFloorToInt(unscaledSize.width()  * fScale) ,
                                               SkScalarFloorToInt(unscaledSize.height() * fScale) };
            const SkImageInfo info = fCodec->getInfo().makeColorType(kN32_SkColorType).makeDimensions(scaledSize);

            bm.allocPixels(info);

            SkCodec::Options opts;
            opts.fFrameIndex = frameIndex;
            const int requiredFrame = fFrameInfos[frameIndex].fRequiredFrame;
            if (requiredFrame != SkCodec::kNoFrame) {
                SkASSERT(requiredFrame >= 0
                         && static_cast<size_t>(requiredFrame) < fFrames.size());
                SkBitmap& requiredBitmap = fFrames[requiredFrame];
                // For simplicity, do not try to cache old frames
                if (requiredBitmap.getPixels() &&
                    ToolUtils::copy_to(&bm, requiredBitmap.colorType(), requiredBitmap)) {
                    opts.fPriorFrame = requiredFrame;
                }
            }

            if (SkCodec::kSuccess != fCodec->getPixels(info, bm.getPixels(),
                                                       bm.rowBytes(), &opts)) {
                SkDebugf("Could not getPixels for frame %i: %s", frameIndex, FLAGS_animatedWebp[0]);
                return;
            }
        }

        canvas->drawImage(bm.asImage(), 0, 0);
    }

public:
    AnimatedWebpGM()
    : fFrame(0)
    , fNextUpdate (-1)
    , fTotalFrames (-1) {}

private:
    SkString getName() const override { return SkString("animatedWebp"); }

    SkISize getISize() override {
        if (this->initCodec()) {
            SkISize dim = fCodec->getInfo().dimensions();
            // Wide enough to display all the frames.
            dim.fWidth *= fTotalFrames;
            // Tall enough to show the row of frames plus an animating version.
            dim.fHeight *= 2;
            return dim;
        }
        return SkISize::Make(640, 480);
    }

    bool initCodec() {
        if (fCodec) {
            return true;
        }
        if (FLAGS_animatedWebp.isEmpty()) {
            SkDebugf("Nothing specified for --animatedWebp!");
            return false;
        }

        std::unique_ptr<SkStream> stream(GetResourceAsStream(FLAGS_animatedWebp[0]));
        if (!stream) {
            return false;
        }

        fCodec = SkCodec::MakeFromStream(std::move(stream));
        if (!fCodec) {
            return false;
        }

        fFrame = 0;
        fFrameInfos = fCodec->getFrameInfo();
        fTotalFrames = fFrameInfos.size();
        return true;
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        if (!this->initCodec()) {
            errorMsg->printf("Could not create codec from %s", FLAGS_animatedWebp[0]);
            return DrawResult::kFail;
        }

        SkAutoCanvasRestore acr(canvas, true);
        canvas->translate(0, SkIntToScalar(fCodec->getInfo().height()));
        this->drawFrame(canvas, fFrame);

        // TODO: fix number indexing thing
        canvas->save();
        int x = 0;
        int y = 0;
        int w = fCodec->getInfo().width() * fScale;
        int h = fCodec->getInfo().height() * fScale;
        for (int frameIndex = 0; frameIndex < fTotalFrames; frameIndex++) {
            this->drawFrame(canvas, frameIndex);
            canvas->translate(SkIntToScalar(w), 0);
            x += SkIntToScalar(w);
            if (frameIndex % 10 == 0) {
              canvas->restore();
              x = 0;
              canvas->translate(0, SkIntToScalar(h));
              y += SkIntToScalar(h);
              canvas->save();
            }

            SkFont font(ToolUtils::DefaultPortableTypeface(), 25);
            SkPaint paint;
            paint.setStroke(true);
            paint.setColor(SK_ColorBLACK);
            SkString text;
            text.printf("%d", frameIndex);
            canvas->drawString(text, x, y, font, paint);


        }
        canvas->restore();
        return DrawResult::kOk;
    }

    bool onAnimate(double nanos) override {
        if (!fCodec || fTotalFrames == 1) {
            return false;
        }

        double secs = TimeUtils::NanosToMSec(nanos) * .3;
        if (fNextUpdate < double(0)) {
            // This is a sentinel that we have not done any updates yet.
            // I'm assuming this gets called *after* onOnceBeforeDraw, so our first frame should
            // already have been retrieved.
            SkASSERT(fFrame == 0);
            fNextUpdate = secs + fFrameInfos[fFrame].fDuration;

            return true;
        }

        if (secs < fNextUpdate) {
            return true;
        }

        while (secs >= fNextUpdate) {
            // Retrieve the next frame.
            fFrame++;
            if (fFrame == fTotalFrames) {
                fFrame = 0;
            }

            // Note that we loop here. This is not safe if we need to draw the intermediate frame
            // in order to draw correctly.
            fNextUpdate += fFrameInfos[fFrame].fDuration;
        }

        return true;
    }
};
DEF_GM(return new AnimatedWebpGM;)


#endif
