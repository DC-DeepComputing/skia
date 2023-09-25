/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkPath.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkTypeface_fontations.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/ports/SkFontHost_FreeType_common.h"
#include "src/utils/SkJSON.h"
#include "tools/Resources.h"
#include "tools/TestFontDataProvider.h"

#include <regex>
#include <string>

namespace skiagm {

namespace {

constexpr int kGmWidth = 900;
constexpr int kMargin = 60;
constexpr float kFontSize = 24;
constexpr float kLangYIncrementScale = 1.8;

const char kReportFontName[] = "fonts/Roboto-Regular.ttf";
// const SkScalar kDumpFontSize = 20.0f;

void comparePixels(const SkBitmap& bitmapA,
                   const SkBitmap& bitmapB,
                   SkBitmap* outBitmap,
                   SkBitmap* whiteBitmap) {
    SkASSERT(bitmapA.colorType() == bitmapB.colorType() &&
             bitmapA.colorType() == outBitmap->colorType() &&
             bitmapA.dimensions() == bitmapB.dimensions() &&
             bitmapA.dimensions() == outBitmap->dimensions());

    SkASSERT(bitmapA.bytesPerPixel() == 4);
    SkISize dimensions = bitmapA.dimensions();
    for (int32_t x = 0; x < dimensions.fWidth; x++) {
        for (int32_t y = 0; y < dimensions.fHeight; y++) {
            SkPMColor c0 = *bitmapA.getAddr32(x, y);
            SkPMColor c1 = *bitmapB.getAddr32(x, y);
            int dr = SkGetPackedR32(c0) - SkGetPackedR32(c1);
            int dg = SkGetPackedG32(c0) - SkGetPackedG32(c1);
            int db = SkGetPackedB32(c0) - SkGetPackedB32(c1);

            *(outBitmap->getAddr32(x, y)) =
                    SkPackARGB32(0xFF, SkAbs32(dr), SkAbs32(dg), SkAbs32(db));

            if (dr != 0 || dg != 0 || db != 0) {
                *(whiteBitmap->getAddr32(x, y)) = SK_ColorWHITE;
            }
        }
    }
}

}  // namespace

class FontationsFtCompareGM : public GM {
public:
    FontationsFtCompareGM(std::string testName,
                          std::string fontNameFilterRegexp,
                          std::string langFilterRegexp)
            : fTestDataIterator(fontNameFilterRegexp, langFilterRegexp)
            , fTestName(testName.c_str()) {
        this->setBGColor(SK_ColorWHITE);
    }

protected:
    void onOnceBeforeDraw() override {
        fReportTypeface =
                SkTypeface_Make_Fontations(GetResourceAsStream(kReportFontName), SkFontArguments());
    }

    SkString getName() const override {
        return SkStringPrintf("fontations_compare_ft_%s", fTestName.c_str());
    }

    SkISize getISize() override { return SkISize::Make(kGmWidth, 600); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);

        SkMatrix localToDevice = canvas->getLocalToDeviceAs3x3();

        fTestDataIterator.rewind();
        TestFontDataProvider::TestSet testSet;
        SkScalar yCoord = kFontSize * 1.5f;

        while (fTestDataIterator.next(&testSet)) {
            sk_sp<SkTypeface> testTypeface = SkTypeface_Make_Fontations(
                    SkStream::MakeFromFile(testSet.fontFilename.c_str()), SkFontArguments());
            sk_sp<SkTypeface> ftTypeface = SkTypeface_FreeType::MakeFromStream(
                    SkStream::MakeFromFile(testSet.fontFilename.c_str()), SkFontArguments());

            if (!testTypeface || !ftTypeface) {
                *errorMsg = "Unable to initialize typeface.";
                return DrawResult::kSkip;
            }

            auto configureFont = [localToDevice](SkFont& font) {
                font.setSize(kFontSize * localToDevice.getScaleY());
                font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
                font.setSubpixel(true);
                font.setHinting(SkFontHinting::kNone);
            };

            SkFont font(testTypeface);
            configureFont(font);

            SkFont ftFont(ftTypeface);
            configureFont(ftFont);

            for (auto& langEntry : testSet.langSamples) {
                SkRect drawSize =
                        SkRect::MakeWH(kFontSize * 6, font.getSize() * kLangYIncrementScale);
                localToDevice.mapRect(&drawSize, drawSize);

                SkImageInfo imageInfo(
                        canvas->imageInfo().makeDimensions(drawSize.roundOut().size()));
                SkSurfaceProps surfaceProps(canvas->getBaseProps());
                SkBitmap fontationsBitmap, freetypeBitmap, comparisonBitmap, whiteBitmap;
                fontationsBitmap.allocPixels(imageInfo, 0);
                freetypeBitmap.allocPixels(imageInfo, 0);
                comparisonBitmap.allocPixels(imageInfo, 0);
                whiteBitmap.allocPixels(imageInfo, 0);

                auto shapeAndDrawToCanvas = [imageInfo, surfaceProps, paint, langEntry](
                                                    const SkFont& font, SkBitmap& bitmap) {
                    SkTextBlobBuilderRunHandler textBlobBuilder(langEntry.sampleShort.c_str(),
                                                                {0, 0});
                    std::unique_ptr<SkShaper> shaper = SkShaper::Make();
                    shaper->shape(langEntry.sampleShort.c_str(),
                                  langEntry.sampleShort.size(),
                                  font,
                                  true,
                                  999999, /* Don't linebreak. */
                                  &textBlobBuilder);
                    sk_sp<const SkTextBlob> blob = textBlobBuilder.makeBlob();

                    std::unique_ptr<SkCanvas> canvas(SkCanvas::MakeRasterDirect(
                            imageInfo, bitmap.getPixels(), bitmap.rowBytes(), &surfaceProps));
                    SkASSERT(canvas);

                    canvas->clear(SK_ColorWHITE);
                    canvas->drawTextBlob(blob.get(), 5, 0, paint);
                };

                shapeAndDrawToCanvas(font, fontationsBitmap);
                shapeAndDrawToCanvas(ftFont, freetypeBitmap);

                SkPoint ftCoord = localToDevice.mapPoint(SkPoint::Make(kMargin, yCoord));
                canvas->writePixels(fontationsBitmap, ftCoord.x(), ftCoord.y());
                SkPoint fontationsCoord = localToDevice.mapPoint(
                        SkPoint::Make(2 * kMargin + drawSize.width(), yCoord));

                canvas->writePixels(freetypeBitmap, fontationsCoord.x(), fontationsCoord.y());

                comparePixels(fontationsBitmap, freetypeBitmap, &comparisonBitmap, &whiteBitmap);

                SkPoint comparisonCoord = localToDevice.mapPoint(
                        SkPoint::Make(3 * kMargin + drawSize.width() * 2, yCoord));
                canvas->writePixels(comparisonBitmap, comparisonCoord.x(), comparisonCoord.y());
                SkPoint whiteCoord = localToDevice.mapPoint(
                        SkPoint::Make(4 * kMargin + drawSize.width() * 3, yCoord));
                canvas->writePixels(whiteBitmap, whiteCoord.x(), whiteCoord.y());

                yCoord += imageInfo.height() / localToDevice.getScaleY();
            }
        }

        return DrawResult::kOk;
    }

private:
    using INHERITED = GM;

    TestFontDataProvider fTestDataIterator;
    SkString fTestName;
    sk_sp<SkTypeface> fReportTypeface;
    std::unique_ptr<SkFontArguments::VariationPosition::Coordinate[]> fCoordinates;
};

DEF_GM(return new FontationsFtCompareGM(
        "NotoSans",
        "Noto Sans",
        "en_Latn|es_Latn|pt_Latn|id_Latn|ru_Cyrl|fr_Latn|tr_Latn|vi_Latn|de_"
        "Latn|it_Latn|pl_Latn|nl_Latn|uk_Cyrl|gl_Latn|ro_Latn|cs_Latn|hu_Latn|"
        "el_Grek|se_Latn|da_Latn|bg_Latn|sk_Latn|fi_Latn|bs_Latn|ca_Latn|no_"
        "Latn|sr_Latn|sr_Cyrl|lt_Latn|hr_Latn|sl_Latn|uz_Latn|uz_Cyrl|lv_Latn|"
        "et_Latn|az_Latn|az_Cyrl|la_Latn|tg_Latn|tg_Cyrl|sw_Latn|mn_Cyrl|kk_"
        "Latn|kk_Cyrl|sq_Latn|af_Latn|ha_Latn|ky_Cyrl"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Deva",
                                        "Noto Sans Devanagari",
                                        "hi_Deva|mr_Deva"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_ar_Arab",
                                        "Noto Sans Arabic",
                                        "ar_Arab|uz_Arab|kk_Arab|ky_Arab"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Beng", "Noto Sans Bengali", "bn_Beng"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Jpan", "Noto Sans JP", "ja_Jpan"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Thai", "Noto Sans Thai", "th_Thai"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Hans", "Noto Sans SC", "zh_Hans"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Hant", "Noto Sans TC", "zh_Hant"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Kore", "Noto Sans KR", "ko_Kore"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Taml", "Noto Sans Tamil", "ta_Taml"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Newa", "Noto Sans Newa", "new_Newa"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Knda", "Noto Sans Kannada", "kn_Knda"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Tglg", "Noto Sans Tagalog", "fil_Tglg"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Telu", "Noto Sans Telugu", "te_Telu"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Gujr", "Noto Sans Gujarati", "gu_Gujr"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Geor", "Noto Sans Georgian", "ka_Geor"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Mlym", "Noto Sans Malayalam", "ml_Mlym"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Khmr", "Noto Sans Khmer", "km_Khmr"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Sinh", "Noto Sans Sinhala", "si_Sinh"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Mymr", "Noto Sans Myanmar", "my_Mymr"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Java", "Noto Sans Javanese", "jv_Java"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Mong", "Noto Sans Mongolian", "mn_Mong"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Armn", "Noto Sans Armenian", "hy_Armn"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Elba", "Noto Sans Elbasan", "sq_Elba"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Vith", "Noto Sans Vithkuqi", "sq_Vith"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Guru", "Noto Sans Gurmukhi", "pa_Guru"));

}  // namespace skiagm
