/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/ports/SkTypeface_fontations.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/ports/SkTypeface_FreeType.h"
#include "tools/Resources.h"
#include "tools/TestFontDataProvider.h"

namespace skiagm {

namespace {

constexpr int kGmWidth = 1000;
constexpr int kMargin = 15;
constexpr float kFontSize = 24;
constexpr float kLangYIncrementScale = 1.9;
constexpr bool kVerbose = false;

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
    SkString getName() const override {
        return SkStringPrintf("fontations_compare_ft_%s", fTestName.c_str());
    }

    SkISize getISize() override {
        TestFontDataProvider::TestSet testSet;
        fTestDataIterator.rewind();
        fTestDataIterator.next(&testSet);

        return SkISize::Make(kGmWidth,
                             testSet.langSamples.size() * kFontSize * kLangYIncrementScale + 100);
    }

    void onOnceBeforeDraw() override {
        SkString diffSkSL(R"(
            uniform shader before_map;
            uniform shader after_map;

            half4 main(float2 xy) {
                half4 before = before_map.eval(xy);
                half4 after = after_map.eval(xy);

                half4 diff = abs(before - after);
                diff.a = 1;
                return diff;
            }
        )");
        auto [diffEffect, diffError] = SkRuntimeEffect::MakeForShader(std::move(diffSkSL));
        if (!diffEffect) {
            SkDebugf("RuntimeShader error: %s\n", diffError.c_str());
        }
        fDiffEffect = std::move(diffEffect);

        SkString diffMaskSkSL(R"(
            uniform shader before_map;
            uniform shader after_map;

            half4 main(float2 xy) {
                half4 before = before_map.eval(xy);
                half4 after = after_map.eval(xy);

                half4 diff;
                if (before != after)
                    diff = half4(1,1,1,1);
                else
                    diff = half4(0,0,0,1);
                return diff;
            }
        )");
        auto [diffMaskEffect, diffMaskError] = SkRuntimeEffect::MakeForShader(std::move(diffMaskSkSL));
        if (!diffMaskEffect) {
            SkDebugf("RuntimeShader error: %s\n", diffMaskError.c_str());
        }
        fDiffMaskEffect = std::move(diffMaskEffect);
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);

        fTestDataIterator.rewind();
        TestFontDataProvider::TestSet testSet;

        SkMatrix l2d = canvas->getLocalToDeviceAs3x3();
        SkMatrix d2l;
        if (!l2d.invert(&d2l)) {
            return DrawResult::kFail;
        }

        while (fTestDataIterator.next(&testSet)) {
            sk_sp<SkTypeface> testTypeface = SkTypeface_Make_Fontations(
                    SkStream::MakeFromFile(testSet.fontFilename.c_str()), SkFontArguments());
            sk_sp<SkTypeface> ftTypeface = SkTypeface_FreeType::MakeFromStream(
                    SkStream::MakeFromFile(testSet.fontFilename.c_str()), SkFontArguments());

            if (!testTypeface || !ftTypeface) {
                *errorMsg = "Unable to initialize typeface.";
                return DrawResult::kSkip;
            }

            auto configureFont = [](SkFont& font) {
                font.setSize(kFontSize);
                font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
                font.setSubpixel(true);
                font.setHinting(SkFontHinting::kNone);
                font.setBaselineSnap(false);
            };

            SkFont font(testTypeface);
            configureFont(font);

            SkFont ftFont(ftTypeface);
            configureFont(ftFont);
            enum class DrawPhase { Fontations, FreeType, Comparison };

            SkRect maxBounds = SkRect::MakeEmpty();
            for (auto phase : {DrawPhase::Fontations, DrawPhase::FreeType, DrawPhase::Comparison}) {
                SkScalar yCoord = kFontSize * 1.5f;

                for (auto& langEntry : testSet.langSamples) {
                    auto shapeAndDrawToCanvas = [canvas, paint, langEntry](const SkFont& font,
                                                                           SkPoint coord) {
                        std::string testString(langEntry.sampleShort.c_str(),
                                               langEntry.sampleShort.size());
                        SkTextBlobBuilderRunHandler textBlobBuilder(testString.c_str(), {0, 0});
                        std::unique_ptr<SkShaper> shaper = SkShaper::Make();
                        shaper->shape(testString.c_str(),
                                      testString.size(),
                                      font,
                                      true,
                                      999999, /* Don't linebreak. */
                                      &textBlobBuilder);
                        sk_sp<const SkTextBlob> blob = textBlobBuilder.makeBlob();
                        canvas->drawTextBlob(blob.get(), coord.x(), coord.y(), paint);
                        return blob->bounds();
                    };

                    auto roundToDevicePixels = [l2d, d2l](const SkPoint& point) {
                        SkPoint mapped = l2d.mapPoint(point);
                        SkPoint mappedRounded =
                                SkPoint::Make(roundf(mapped.x()), roundf(mapped.y()));
                        return d2l.mapPoint(mappedRounded);
                    };

                    auto fontationsCoord = [yCoord, roundToDevicePixels]() {
                        return roundToDevicePixels(SkPoint::Make(kMargin, yCoord));
                    };

                    auto freetypeCoord = [yCoord, maxBounds, roundToDevicePixels]() {
                        const SkScalar margin = 2 * kMargin + maxBounds.left() + maxBounds.width();
                        return roundToDevicePixels(SkPoint::Make(margin, yCoord));
                    };

                    switch (phase) {
                        case DrawPhase::Fontations: {
                            /* Determine maximum of column width across all language samples. */
                            maxBounds.join(shapeAndDrawToCanvas(font, fontationsCoord()));
                            break;
                        }
                        case DrawPhase::FreeType: {
                            shapeAndDrawToCanvas(ftFont, freetypeCoord());
                            break;
                        }
                        case DrawPhase::Comparison: {
                            SkPoint fontationsOrigin = fontationsCoord();
                            SkPoint freetypeOrigin = freetypeCoord();
                            SkRect fontationsBBox(maxBounds.makeOffset(fontationsOrigin));
                            SkRect freetypeBBox(maxBounds.makeOffset(freetypeOrigin));
                            if (kVerbose) {
                                SkPaint p;
                                p.setColor(SK_ColorRED);
                                p.setStroke(true);
                                p.setStrokeWidth(2);
                                canvas->drawPoint(fontationsOrigin.x(), fontationsOrigin.y(), p);
                                canvas->drawPoint(freetypeOrigin.x(), freetypeOrigin.y(), p);
                                p.setColor(SK_ColorGREEN);
                                canvas->drawRect(fontationsBBox, p);
                                canvas->drawRect(freetypeBBox, p);
                            }

                            SkRect fontationsDeviceBBox = l2d.mapRect(fontationsBBox);
                            SkRect freetypeDeviceBBox = l2d.mapRect(freetypeBBox);

                            sk_sp<SkSurface> surfaceStorage;
                            SkSurface* surface = canvas->getSurface();
                            if (!surface) {
                                // Unfortunately, it is currently not possible to proxy a surface.
                                SkPixmap canvasPixmap;
                                bool canReadPixels = canvas->peekPixels(&canvasPixmap);
                                if (!canReadPixels) {
                                    break;
                                }
                                surfaceStorage = SkSurfaces::WrapPixels(canvasPixmap);
                                surface = surfaceStorage.get();
                            }

                            SkIRect fontationsDeviceIBox = fontationsDeviceBBox.roundOut();
                            SkIRect freetypeDeviceIBox = freetypeDeviceBBox.roundOut();
                            sk_sp<SkImage> fontationsImage = surface->makeImageSnapshot(fontationsDeviceIBox);
                            sk_sp<SkImage> freetypeImage = surface->makeImageSnapshot(freetypeDeviceIBox);
                            if (!fontationsImage || !freetypeImage) {
                                break;
                            }

                            sk_sp<SkShader> fontationsShader = fontationsImage->makeShader(SkSamplingOptions());
                            sk_sp<SkShader> freeTypeShader = freetypeImage->makeShader(SkSamplingOptions());

                            // Place comparison results as two extra columns.
                            if (fDiffEffect) {
                                SkRuntimeShaderBuilder builder(fDiffEffect);
                                builder.child("before_map") = fontationsShader;
                                builder.child("after_map") = freeTypeShader;
                                SkPaint diffPaint;
                                SkMatrix d2loffset = d2l;
                                // Put the image back where it was sampled from in device space.
                                d2loffset.preTranslate(fontationsDeviceIBox.fLeft,
                                                       fontationsDeviceIBox.fTop);
                                // Then offset it in canvas space.
                                // TODO: round the resulting location to a pixel.
                                d2loffset.postTranslate(3 * kMargin + maxBounds.width() * 2, 0);
                                diffPaint.setShader(builder.makeShader(&d2loffset));

                                SkRect rect = SkRect::Make(fontationsImage->imageInfo().dimensions());
                                canvas->drawRect(d2loffset.mapRect(rect), diffPaint);
                            }
                            if (fDiffMaskEffect) {
                                SkRuntimeShaderBuilder builder(fDiffMaskEffect);
                                builder.child("before_map") = fontationsShader;
                                builder.child("after_map") = freeTypeShader;
                                SkPaint diffPaint;
                                SkMatrix d2loffset = d2l;
                                // This puts the image back where it was sampled from in device space.
                                d2loffset.preTranslate(fontationsDeviceIBox.fLeft,
                                                       fontationsDeviceIBox.fTop);
                                // Then offset it in canvas space.
                                // TODO: round the resulting location to a pixel.
                                d2loffset.postTranslate(4 * kMargin + maxBounds.width() * 3, 0);
                                diffPaint.setShader(builder.makeShader(&d2loffset));

                                SkRect rect = SkRect::Make(fontationsImage->imageInfo().dimensions());
                                canvas->drawRect(d2loffset.mapRect(rect), diffPaint);
                            }
                            break;
                        }
                    }

                    yCoord += font.getSize() * kLangYIncrementScale;
                }
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
    sk_sp<SkRuntimeEffect> fDiffEffect;
    sk_sp<SkRuntimeEffect> fDiffMaskEffect;
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
