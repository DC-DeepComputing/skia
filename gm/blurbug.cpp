/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

 #include "gm/gm.h"
 #include "include/core/SkBlurTypes.h"
 #include "include/core/SkCanvas.h"
 #include "include/core/SkColor.h"
 #include "include/core/SkMaskFilter.h"
 #include "include/core/SkPaint.h"
 #include "include/core/SkPoint.h"
 #include "include/core/SkRect.h"
 #include "include/core/SkScalar.h"
 #include "include/core/SkSize.h"
 #include "include/core/SkString.h"
 #include "src/core/SkBlurMask.h"

 namespace skiagm {

 // This GM exercises the blurred rect nine-patching special cases when the
 // blurred rect is very large and/or very far from the origin.
 // It creates a large blurred rect/rectori then renders the 4 corners and the
 // middle.
 // autoninja -C out/arm64 dm && adb push out/arm64/dm /data/local/tmp && adb shell "cd
 // /data/local/tmp; ./dm --src gm --config grdawn_vk --match blurbug"
 class BlurBugGM : public GM {
 public:
     BlurBugGM() { this->setBGColor(0xFFDDDDDD); }

 protected:
     SkString getName() const override { return SkString("blurbug"); }

     SkISize getISize() override { return SkISize::Make(kWidth, kHeight); }

     void onDraw(SkCanvas* canvas) override {
         const SkScalar kSigma = SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(4));
         const SkRect bigRect = SkRect::MakeWH(100, 100);

         SkPaint blurPaint;
         blurPaint.setColor(SK_ColorBLACK);

         blurPaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, kSigma));
         canvas->drawRect(bigRect, blurPaint);
         blurPaint.setMaskFilter(SkMaskFilter::MakeBlur(kOuter_SkBlurStyle, kSigma));
         canvas->drawRect(bigRect, blurPaint);
     }

 private:
     inline static constexpr int kCloseUpSize = 64;
     inline static constexpr int kWidth = 5 * kCloseUpSize;
     inline static constexpr int kHeight = 2 * (kLastEnum_SkBlurStyle + 1) * kCloseUpSize;

     using INHERITED = GM;
 };

 DEF_GM(return new BlurBugGM;)
 }  // namespace skiagm
