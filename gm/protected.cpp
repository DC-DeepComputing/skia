/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#if 0
#include "include/core/SkColor.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/base/SkRandom.h"
#endif

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkImageFilters.h"
#include "include/gpu/GrDirectContext.h"
#include "tests/Test.h"

bool context_supports_protected(GrDirectContext* dContext);
sk_sp<SkSurface> create_protected_sksurface(GrDirectContext* dContext,
                                            skiatest::Reporter* reporter,
                                            bool textureable,
                                            bool isProtected);
sk_sp<SkImage> create_protected_skimage(GrDirectContext* dContext,
                                        skiatest::Reporter* reporter,
                                        SkISize size,
                                        SkColor4f color,
                                        bool isProtected);

namespace skiagm {

class ProtectedGM : public GpuGM {
public:
    ProtectedGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString onShortName() override {
        return SkString("protected1");
    }

    SkISize onISize() override {
        return SkISize::Make(256, 256);
    }

    DrawResult onDraw(GrRecordingContext* rContext, SkCanvas* canvas, SkString* errorMsg) override {
        GrDirectContext* dContext = GrAsDirectContext(rContext);
        if (!dContext) {
            *errorMsg = kErrorMsg_DrawSkippedGpuOnly;
            return DrawResult::kSkip;
        }

        if (!fImage) {
            fImage = create_protected_skimage(dContext, &fReporter, { 256, 256 }, SkColors::kRed,
                                              /* isProtected= */ true);
        }

        SkPaint paint;
        paint.setShader(fImage->makeShader({}));
        paint.setImageFilter(SkImageFilters::Blur(10, 10, nullptr));

        canvas->drawRect(SkRect::MakeWH(256, 256), paint);
        return DrawResult::kOk;
    }

private:
    sk_sp<SkImage> fImage;

    struct Reporter : public skiatest::Reporter {
        void reportFailed(const skiatest::Failure& failure) override {
            SkDebugf("%s\n", failure.toString().c_str());
        }
        bool allowExtendedTest() const override {
            return false;
        }
        bool verbose() const override { return false; }
    } fReporter;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ProtectedGM;)

}  // namespace skiagm
