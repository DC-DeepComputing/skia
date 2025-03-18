/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkTypeface_fontations.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/core/SkColorPriv.h"
#include "src/ports/SkTypeface_FreeType.h"
#include "tools/Resources.h"
#include "tools/TestFontDataProvider.h"

namespace skiagm {

namespace {

constexpr int kGmWidth = 1000;
constexpr int kMargin = 30;
constexpr float kFontSize = 24;
constexpr float kLangYIncrementScale = 1.9;

/** Compare bitmap A and B, in this case originating from text rendering results with FreeType and
 * Fontations + Skia path rendering, compute individual pixel differences for the rectangles that
 * must match in size. Produce a highlighted difference bitmap, in which any pixel becomes white for
 * which a difference was determined. */
void comparePixels(const SkPixmap& pixmapA,
                   const SkPixmap& pixmapB,
                   SkBitmap* outPixelDiffBitmap,
                   SkBitmap* outHighlightDiffBitmap) {
    if (pixmapA.dimensions() != pixmapB.dimensions()) {
        return;
    }
    if (pixmapA.dimensions() != outPixelDiffBitmap->dimensions()) {
        return;
    }

    SkISize dimensions = pixmapA.dimensions();
    for (int32_t x = 0; x < dimensions.fWidth; x++) {
        for (int32_t y = 0; y < dimensions.fHeight; y++) {
            SkColor c0 = pixmapA.getColor(x, y);
            SkColor c1 = pixmapB.getColor(x, y);
            int dr = SkGetPackedR32(c0) - SkGetPackedR32(c1);
            int dg = SkGetPackedG32(c0) - SkGetPackedG32(c1);
            int db = SkGetPackedB32(c0) - SkGetPackedB32(c1);

            *(outPixelDiffBitmap->getAddr32(x, y)) =
                    SkPackARGB32(0xFF, SkAbs32(dr), SkAbs32(dg), SkAbs32(db));

            if (dr != 0 || dg != 0 || db != 0) {
                *(outHighlightDiffBitmap->getAddr32(x, y)) = SK_ColorWHITE;
            } else {
                *(outHighlightDiffBitmap->getAddr32(x, y)) = SK_ColorBLACK;
            }
        }
    }
}

}  // namespace

class FontationsFtCompareGM : public GM {
public:
    enum SimulatePixelGeometry { kLeaveAsIs, kSimulateUnknown };

    FontationsFtCompareGM(std::string testName,
                          std::string fontNameFilterRegexp,
                          std::string langFilterRegexp,
                          SimulatePixelGeometry simulatePixelGeometry,
                          SkFontHinting hintingMode = SkFontHinting::kNone)
            : fTestDataIterator(fontNameFilterRegexp, langFilterRegexp)
            , fTestName(testName.c_str())
            , fSimulatePixelGeometry(simulatePixelGeometry)
            , fHintingMode(hintingMode) {
        this->setBGColor(SK_ColorWHITE);
    }

protected:
    SkString getName() const override {
        SkString testName = SkStringPrintf("fontations_compare_ft_%s", fTestName.c_str());
        switch (fHintingMode) {
            case SkFontHinting::kNormal: {
                testName.append("_hint_normal");
                break;
            }
            case SkFontHinting::kSlight: {
                testName.append("_hint_slight");
                break;
            }
            case SkFontHinting::kFull: {
                testName.append("_hint_full");
                break;
            }
            case SkFontHinting::kNone: {
                testName.append("_hint_none");
                break;
            }
        }

        if (fSimulatePixelGeometry == SimulatePixelGeometry::kSimulateUnknown) {
            testName.append("_unknown_px_geometry");
        }
        return testName;
    }

    SkISize getISize() override {
        TestFontDataProvider::TestSet testSet;
        fTestDataIterator.rewind();
        fTestDataIterator.next(&testSet);

        return SkISize::Make(kGmWidth,
                             testSet.langSamples.size() * kFontSize * kLangYIncrementScale + 100);
    }

    bool wrapCanvasUnknownGeometry(SkCanvas* source, sk_sp<SkSurface>& target) {
        SkPixmap canvasPixmap;
        if (!source->peekPixels(&canvasPixmap)) {
            return false;
        }

        SkSurfaceProps canvasSurfaceProps = source->getBaseProps();
        SkSurfaceProps unknownGeometrySurfaceProps = canvasSurfaceProps.cloneWithPixelGeometry(
                SkPixelGeometry::kUnknown_SkPixelGeometry);
        target = SkSurfaces::WrapPixels(canvasPixmap, &unknownGeometrySurfaceProps);
        return true;
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);

        fTestDataIterator.rewind();
        TestFontDataProvider::TestSet testSet;

        while (fTestDataIterator.next(&testSet)) {
            sk_sp<SkTypeface> testTypeface = SkTypeface_Make_Fontations(
                    SkStream::MakeFromFile(testSet.fontFilename.c_str()), SkFontArguments());
            sk_sp<SkTypeface> ftTypeface = SkTypeface_FreeType::MakeFromStream(
                    SkStream::MakeFromFile(testSet.fontFilename.c_str()), SkFontArguments());

            if (!testTypeface || !ftTypeface) {
                *errorMsg = "Unable to initialize typeface.";
                return DrawResult::kSkip;
            }

            auto configureFont = [this](SkFont& font) {
                font.setSize(kFontSize);
                font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
                font.setSubpixel(true);
                font.setHinting(fHintingMode);
                font.setForceAutoHinting(true);
            };

            SkFont font(testTypeface);
            configureFont(font);

            SkFont ftFont(ftTypeface);
            configureFont(ftFont);
            enum class DrawPhase { Fontations, FreeType, Comparison };

            SkCanvas* drawCanvas = canvas;

            // See https://issues.skia.org/issues/396360753
            // We would like Fontations anti-aliasing on a surface with unknown pixel geometry to
            // look like the FreeType backend in order to avoid perceived regressions
            // in contrast/sharpness.
            // Simulate the unknown geometry case for tests that request it.
            sk_sp<SkSurface> surface = nullptr;
            if (!wrapCanvasUnknownGeometry(canvas, surface)) {
                return DrawResult::kFail;
            }
            drawCanvas = surface->getCanvas();

            SkRect maxBounds = SkRect::MakeEmpty();
            for (auto phase : {DrawPhase::Fontations, DrawPhase::FreeType, DrawPhase::Comparison}) {
                SkScalar yCoord = kFontSize * 1.5f;

                for (auto& langEntry : testSet.langSamples) {
                    auto shapeAndDrawToCanvas = [drawCanvas, paint, langEntry](const SkFont& font,
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
                        drawCanvas->drawTextBlob(blob.get(), coord.x(), coord.y(), paint);
                        return blob->bounds();
                    };

                    auto roundToDevicePixels = [drawCanvas](SkPoint& point) {
                        SkMatrix ctm = drawCanvas->getLocalToDeviceAs3x3();
                        SkPoint mapped = ctm.mapPoint(point);
                        SkPoint mappedRounded =
                                SkPoint::Make(roundf(mapped.x()), roundf(mapped.y()));
                        SkMatrix inverse;
                        bool inverseExists = ctm.invert(&inverse);
                        SkASSERT(inverseExists);
                        if (inverseExists) {
                            point = inverse.mapPoint(mappedRounded);
                        }
                    };

                    auto fontationsCoord = [yCoord, roundToDevicePixels]() {
                        SkPoint fontationsCoord = SkPoint::Make(kMargin, yCoord);
                        roundToDevicePixels(fontationsCoord);
                        return fontationsCoord;
                    };

                    auto freetypeCoord = [yCoord, maxBounds, roundToDevicePixels]() {
                        SkPoint freetypeCoord = SkPoint::Make(
                                2 * kMargin + maxBounds.left() + maxBounds.width(), yCoord);
                        roundToDevicePixels(freetypeCoord);
                        return freetypeCoord;
                    };

                    switch (phase) {
                        case DrawPhase::Fontations: {
                            SkRect boundsFontations = shapeAndDrawToCanvas(font, fontationsCoord());
                            /* Determine maximum of column width across all language samples. */
                            boundsFontations.roundOut();
                            maxBounds.join(boundsFontations);
                            break;
                        }
                        case DrawPhase::FreeType: {
                            shapeAndDrawToCanvas(ftFont, freetypeCoord());
                            break;
                        }
                        case DrawPhase::Comparison: {
                            /* Read back pixels from equally sized rectangles from the space in
                             * SkCanvas where Fontations and FreeType sample texts were drawn,
                             * compare them using pixel comparisons similar to SkDiff, draw a
                             * comparison as faint pixel differences, and as an amplified
                             * visualization in which each differing pixel is drawn as white. */
                            SkPoint fontationsOrigin = fontationsCoord();
                            SkPoint freetypeOrigin = freetypeCoord();
                            SkRect fontationsBBox(maxBounds.makeOffset(fontationsOrigin));
                            SkRect freetypeBBox(maxBounds.makeOffset(freetypeOrigin));

                            SkMatrix ctm = drawCanvas->getLocalToDeviceAs3x3();
                            ctm.mapRect(&fontationsBBox, fontationsBBox);
                            ctm.mapRect(&freetypeBBox, freetypeBBox);

                            SkIRect fontationsIBox(fontationsBBox.roundOut());
                            SkIRect freetypeIBox(freetypeBBox.roundOut());

                            SkISize pixelDimensions(fontationsIBox.size());
                            SkImageInfo canvasImageInfo = drawCanvas->imageInfo();
                            SkImageInfo diffImageInfo =
                                    SkImageInfo::Make(pixelDimensions,
                                                      SkColorType::kN32_SkColorType,
                                                      SkAlphaType::kUnpremul_SkAlphaType);

                            SkBitmap diffBitmap, highlightDiffBitmap;
                            diffBitmap.allocPixels(diffImageInfo, 0);
                            highlightDiffBitmap.allocPixels(diffImageInfo, 0);

                            // Workaround OveridePaintFilterCanvas limitations
                            // by getting pixel access through peekPixels()
                            // instead of readPixels(). Then use same pixmap to
                            // later write back the comparison results.
                            SkPixmap canvasPixmap;
                            if (!drawCanvas->peekPixels(&canvasPixmap)) {
                                break;
                            }

                            SkPixmap fontationsPixmap, freetypePixmap;
                            if (!canvasPixmap.extractSubset(&fontationsPixmap, fontationsIBox) ||
                                !canvasPixmap.extractSubset(&freetypePixmap, freetypeIBox)) {
                                break;
                            }

                            comparePixels(fontationsPixmap,
                                          freetypePixmap,
                                          &diffBitmap,
                                          &highlightDiffBitmap);

                            /* Place comparison results as two extra columns, shift up to account
                               for placement of rectangles vs. SkTextBlobs (baseline shift). */
                            SkPoint comparisonCoord = ctm.mapPoint(SkPoint::Make(
                                    3 * kMargin + maxBounds.width() * 2, yCoord + maxBounds.top()));
                            SkPoint whiteCoord = ctm.mapPoint(SkPoint::Make(
                                    4 * kMargin + maxBounds.width() * 3, yCoord + maxBounds.top()));

                            SkSurfaceProps canvasSurfaceProps = drawCanvas->getBaseProps();
                            sk_sp<SkSurface> writeBackSurface =
                                    SkSurfaces::WrapPixels(canvasPixmap, &canvasSurfaceProps);

                            writeBackSurface->writePixels(
                                    diffBitmap, comparisonCoord.x(), comparisonCoord.y());
                            writeBackSurface->writePixels(
                                    highlightDiffBitmap, whiteCoord.x(), whiteCoord.y());
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
    SimulatePixelGeometry fSimulatePixelGeometry;
    SkFontHinting fHintingMode;
    sk_sp<SkTypeface> fReportTypeface;
    std::unique_ptr<SkFontArguments::VariationPosition::Coordinate[]> fCoordinates;
};

DEF_GM(return new FontationsFtCompareGM("NotoSans_Jpan_CFF",
                                        "Noto Sans JP CFF",
                                        "ja_Jpan",
                                        FontationsFtCompareGM::SimulatePixelGeometry::kLeaveAsIs,
                                        SkFontHinting::kSlight));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Jpan_CFF",
                                        "Noto Sans JP CFF",
                                        "ja_Jpan",
                                        FontationsFtCompareGM::SimulatePixelGeometry::kLeaveAsIs,
                                        SkFontHinting::kNormal));


}  // namespace skiagm
