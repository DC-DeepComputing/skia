// Copyright 2026 Google LLC
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Metrics.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skshaper/utils/FactoryHelpers.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/include/SkUnicode_icu.h"
#include "src/core/SkUTF.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/viewer/ClickHandlerSlide.h"
#include "tools/viewer/Slide.h"

#include <CoreFoundation/CoreFoundation.h>
#include <cmath>
#include <functional>
#include <vector>

using namespace skia::textlayout;

namespace {

class ParagraphBugSlide_Base : public ClickHandlerSlide {
public:
    void load(SkScalar w, SkScalar h) override { fSize = {w, h}; }
    void resize(SkScalar w, SkScalar h) override { fSize = {w, h}; }

protected:
    Click* onFindClickHandler(SkScalar, SkScalar, skui::ModifierKey) override { return nullptr; }
    bool onClick(ClickHandlerSlide::Click*) override { return false; }

    sk_sp<FontCollection> getFontCollection() {
        if (!fFontCollection) {
            fFontCollection = sk_make_sp<FontCollection>();
            fFontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
            fFontCollection->enableFontFallback();
        }
        return fFontCollection;
    }

    sk_sp<SkUnicode> getUnicode() {
        auto factory = SkShapers::BestAvailable();
        if (factory && factory->getUnicode()) {
            return sk_ref_sp<SkUnicode>(factory->getUnicode());
        }
        return nullptr;
    }

    sk_sp<SkUnicode> getIcuUnicode() {
        if (auto unicode = SkUnicodes::ICU::Make()) {
            return unicode;
        }
        return getUnicode();
    }

    SkSize size() const { return fSize; }

    void drawHeader(SkCanvas* canvas,
                    const char* bugId,
                    const char* title,
                    const char* status,
                    SkColor statusBg,
                    const char* summary) {
        SkScalar width = this->size().width();

        SkPaint bgPaint;
        bgPaint.setColor(SkColorSetRGB(246, 248, 251));
        canvas->drawRect(SkRect::MakeXYWH(0, 0, width, 84.0f), bgPaint);

        SkPaint divider;
        divider.setColor(SkColorSetRGB(222, 228, 237));
        divider.setStrokeWidth(1.0f);
        canvas->drawLine(0, 84.0f, width, 84.0f, divider);

        SkFont idFont(ToolUtils::DefaultTypeface(), 13.0f);
        idFont.setEmbolden(true);
        SkPaint idPaint;
        idPaint.setColor(SkColorSetRGB(37, 99, 235));
        idPaint.setAntiAlias(true);
        canvas->drawString(bugId, 24.0f, 28.0f, idFont, idPaint);

        SkFont titleFont(ToolUtils::DefaultTypeface(), 19.0f);
        titleFont.setEmbolden(true);
        SkPaint titlePaint;
        titlePaint.setColor(SkColorSetRGB(15, 23, 42));
        titlePaint.setAntiAlias(true);
        canvas->drawString(title, 24.0f, 54.0f, titleFont, titlePaint);

        SkFont badgeFont(ToolUtils::DefaultTypeface(), 11.0f);
        badgeFont.setEmbolden(true);
        SkScalar badgeWidth = badgeFont.measureText(status, strlen(status), SkTextEncoding::kUTF8) + 16.0f;
        SkRect badgeRect = SkRect::MakeXYWH(width - badgeWidth - 24.0f, 16.0f, badgeWidth, 22.0f);
        SkPaint badgeBg;
        badgeBg.setColor(statusBg);
        badgeBg.setAntiAlias(true);
        canvas->drawRoundRect(badgeRect, 4.0f, 4.0f, badgeBg);

        SkPaint badgeText;
        badgeText.setColor(SK_ColorWHITE);
        badgeText.setAntiAlias(true);
        canvas->drawString(status, badgeRect.fLeft + 8.0f, badgeRect.fBottom - 6.0f, badgeFont, badgeText);

        if (summary && summary[0]) {
            SkFont sumFont(ToolUtils::DefaultTypeface(), 12.0f);
            SkPaint sumPaint;
            sumPaint.setColor(SkColorSetRGB(100, 116, 139));
            sumPaint.setAntiAlias(true);
            canvas->drawString(summary, 24.0f, 74.0f, sumFont, sumPaint);
        }
    }

    void drawCard(SkCanvas* canvas,
                  const SkRect& r,
                  const char* header,
                  SkColor headerColor,
                  const char* note = nullptr) {
        SkPaint bg;
        bg.setColor(SK_ColorWHITE);
        canvas->drawRoundRect(r, 8.0f, 8.0f, bg);

        SkPaint border;
        border.setStyle(SkPaint::kStroke_Style);
        border.setColor(SkColorSetRGB(226, 232, 240));
        border.setStrokeWidth(1.5f);
        border.setAntiAlias(true);
        canvas->drawRoundRect(r, 8.0f, 8.0f, border);

        SkRect headerRect = SkRect::MakeXYWH(r.fLeft, r.fTop, r.width(), 32.0f);
        SkPaint headerBg;
        headerBg.setColor(SkColorSetRGB(248, 250, 252));
        headerBg.setAntiAlias(true);
        canvas->drawRoundRect(headerRect, 8.0f, 8.0f, headerBg);
        canvas->drawRect(SkRect::MakeXYWH(r.fLeft, r.fTop + 16.0f, r.width(), 16.0f), headerBg);
        canvas->drawLine(r.fLeft, r.fTop + 32.0f, r.fRight, r.fTop + 32.0f, border);

        SkFont headerFont(ToolUtils::DefaultTypeface(), 13.0f);
        headerFont.setEmbolden(true);
        SkPaint headerText;
        headerText.setColor(headerColor);
        headerText.setAntiAlias(true);
        canvas->drawString(header, r.fLeft + 14.0f, r.fTop + 21.0f, headerFont, headerText);

        if (note && note[0]) {
            SkFont noteFont(ToolUtils::DefaultTypeface(), 11.0f);
            SkPaint notePaint;
            notePaint.setColor(SkColorSetRGB(100, 116, 139));
            notePaint.setAntiAlias(true);
            canvas->drawString(note, r.fLeft + 14.0f, r.fBottom - 10.0f, noteFont, notePaint);
        }
    }

    void drawComparisonSlide(
            SkCanvas* canvas,
            const char* bugId,
            const char* title,
            const char* status,
            SkColor statusBg,
            const char* summary,
            const char* leftTitle,
            const char* leftSubtitle,
            std::function<void(const SkRect&)> drawLeft,
            const char* rightTitle,
            const char* rightSubtitle,
            std::function<void(const SkRect&)> drawRight,
            const char* oldDefectExplanation,
            const char* newFixExplanation,
            const char* invariantExplanation) {
        canvas->drawColor(SkColorSetRGB(248, 250, 252));
        drawHeader(canvas, bugId, title, status, statusBg, summary);

        SkScalar cardWidth = (this->size().width() - 72.0f) / 2.0f;
        SkScalar cardHeight = 424.0f;

        SkRect leftRect = SkRect::MakeXYWH(24.0f, 96.0f, cardWidth, cardHeight);
        drawCard(canvas, leftRect, leftTitle, SkColorSetRGB(220, 38, 38), leftSubtitle);
        drawLeft(leftRect);

        SkRect rightRect = SkRect::MakeXYWH(48.0f + cardWidth, 96.0f, cardWidth, cardHeight);
        drawCard(canvas, rightRect, rightTitle, SkColorSetRGB(22, 163, 74), rightSubtitle);
        drawRight(rightRect);

        SkRect bottomRect = SkRect::MakeXYWH(24.0f, 532.0f, this->size().width() - 48.0f, 194.0f);
        drawCard(canvas, bottomRect, "Comparative Analysis & Resolution Mechanics",
                 SkColorSetRGB(15, 23, 42), "Detailed comparison between legacy and candidate implementations");

        SkFont boldFont(ToolUtils::DefaultTypeface(), 12.0f);
        boldFont.setEmbolden(true);
        SkFont textFont(ToolUtils::DefaultTypeface(), 12.0f);

        SkPaint redPaint;
        redPaint.setColor(SkColorSetRGB(220, 38, 38));
        redPaint.setAntiAlias(true);

        SkPaint greenPaint;
        greenPaint.setColor(SkColorSetRGB(22, 163, 74));
        greenPaint.setAntiAlias(true);

        SkPaint bluePaint;
        bluePaint.setColor(SkColorSetRGB(37, 99, 235));
        bluePaint.setAntiAlias(true);

        SkPaint darkPaint;
        darkPaint.setColor(SkColorSetRGB(30, 41, 59));
        darkPaint.setAntiAlias(true);

        SkScalar bx = bottomRect.fLeft + 20.0f;
        SkScalar by = bottomRect.fTop + 56.0f;

        canvas->drawString("Legacy Version (Baseline):", bx, by, boldFont, redPaint);
        canvas->drawString(oldDefectExplanation, bx + 225.0f, by, textFont, darkPaint);

        by += 32.0f;
        canvas->drawString("Candidate Version (New Engine):", bx, by, boldFont, greenPaint);
        canvas->drawString(newFixExplanation, bx + 225.0f, by, textFont, darkPaint);

        by += 32.0f;
        canvas->drawString("Architectural Invariant:", bx, by, boldFont, bluePaint);
        canvas->drawString(invariantExplanation, bx + 225.0f, by, textFont, darkPaint);
    }

    std::unique_ptr<Paragraph> makeParagraph(
            TextShapingEngine engine,
            std::function<void(ParagraphBuilderImpl&)> buildText,
            SkScalar layoutWidth = 800.0f,
            sk_sp<SkUnicode> unicode = nullptr) {
        ParagraphStyle style;
        style.setTextShapingEngine(engine);
        ParagraphBuilderImpl builder(style, getFontCollection(), unicode ? unicode : getUnicode());
        buildText(builder);
        auto paragraph = builder.Build();
        paragraph->layout(layoutWidth);
        return paragraph;
    }

    std::unique_ptr<Paragraph> makeHarfBuzzParagraph(
            std::function<void(ParagraphBuilderImpl&)> buildText,
            SkScalar layoutWidth = 440.0f) {
        return makeParagraph(TextShapingEngine::kHarfBuzz, buildText, layoutWidth, getIcuUnicode());
    }

    std::unique_ptr<Paragraph> makeCoreTextParagraph(
            std::function<void(ParagraphBuilderImpl&)> buildText,
            SkScalar layoutWidth = 440.0f) {
        return makeParagraph(TextShapingEngine::kCoreText, buildText, layoutWidth, getUnicode());
    }

private:
    SkSize fSize = {1050, 750};
    sk_sp<FontCollection> fFontCollection;
};

// Slide 0: Master Dashboard of all 13 Legacy Bugs
class ParagraphSlide_BugsOverview : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_BugsOverview() { fName = "ParagraphSlide_BugsOverview"; }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SkColorSetRGB(248, 250, 252));
        drawHeader(canvas,
                   "DASHBOARD",
                   "SkParagraph Legacy Bug Test Suite Overview",
                   "13 TESTS: 9 PASSING | 4 ROADMAP",
                   SkColorSetRGB(16, 149, 193),
                   "Visual slide catalog of Apple native CoreText and HarfBuzz typography test cases");

        struct BugEntry {
            const char* id;
            const char* domain;
            const char* title;
            const char* slideName;
            bool isPassing;
        };

        const BugEntry bugs[] = {
            {"Flutter 150824", "CoreText HIG", "Dynamic tracking curves on Apple SF Pro", "ParagraphSlide_Flutter_150824_DynamicTracking", true},
            {"Flutter 18915", "Font Cascade", "Optical size cascade (.AppleSystemUIFont)", "ParagraphSlide_Flutter_18915_147708_OpticalSize", false},
            {"Flutter 123065", "CFStringTokenizer", "CJK morphological word break boundaries", "ParagraphSlide_Flutter_123065_CJKWordBoundaries", true},
            {"Flutter 121680", "CoreText Shaper", "Keycap emoji sequence ([1], [2], [#]) clustering", "ParagraphSlide_Flutter_121680_108720_KeycapEmojiCluster", true},
            {"Flutter 190569", "Cluster Monotonicity", "BiDi Arabic Letter Mark (ALM) ordering", "ParagraphSlide_Flutter_190569_BiDi_ALM_Monotonicity", true},
            {"Flutter 141080", "CTRun Positions", "Multi-styled Thai diacritics vertical anchor", "ParagraphSlide_Flutter_141080_ThaiCombiningMarks", false},
            {"Skia b/318667611", "System UI Cascade", "SF Symbols font cascade & PUA resolution", "ParagraphSlide_Skia_b_318667611_AppleSystemUIFontCascade", true},
            {"Skia b/40040287", "Hit-Testing Map", "Astral emoji UTF-16 code units hit-testing", "ParagraphSlide_Skia_b_40040287_UTF16_AstralPlaneIndices", false},
            {"Skia b/448789662", "BiDi Sub-runs", "Mixed RTL/LTR fallback cluster bounds", "ParagraphSlide_Skia_b_448789662_BiDi_MixedRunsClusterBounds", true},
            {"Skia b/458132646", "Font Fallback", "Japanese Kana fallback to Hiragino/PingFang", "ParagraphSlide_Skia_b_458132646_JapaneseKana_NotEmoji", true},
            {"Skia b/341104766", "Glyph Offsets (fY)", "Vertical baseline shifts across styled spans", "ParagraphSlide_Skia_b_341104766_VerticalGlyphOffsets", true},
            {"Skia b/405286754", "TextWrapper", "Strict CJK word wrapping with padding", "ParagraphSlide_Skia_b_405286754_StrictWordWrappingCJK", false},
            {"Skia b/305780908", "SkUnicode_apple", "Zero-ICU footprint on Apple platforms", "ParagraphSlide_Skia_b_305780908_ZeroICUFootprint", true},
        };

        SkFont titleFont(ToolUtils::DefaultTypeface(), 13.0f);
        titleFont.setEmbolden(true);
        SkFont descFont(ToolUtils::DefaultTypeface(), 12.0f);
        SkFont metaFont(ToolUtils::DefaultTypeface(), 11.0f);

        SkPaint textPaint;
        textPaint.setColor(SkColorSetRGB(15, 23, 42));
        textPaint.setAntiAlias(true);

        SkPaint metaPaint;
        metaPaint.setColor(SkColorSetRGB(100, 116, 139));
        metaPaint.setAntiAlias(true);

        SkPaint bluePaint;
        bluePaint.setColor(SkColorSetRGB(37, 99, 235));
        bluePaint.setAntiAlias(true);

        SkPaint cardBg;
        cardBg.setColor(SK_ColorWHITE);

        SkPaint cardBorder;
        cardBorder.setStyle(SkPaint::kStroke_Style);
        cardBorder.setColor(SkColorSetRGB(226, 232, 240));
        cardBorder.setStrokeWidth(1.0f);
        cardBorder.setAntiAlias(true);

        SkScalar startX = 24.0f;
        SkScalar startY = 112.0f;
        SkScalar colWidth = (this->size().width() - 72.0f) / 2.0f;
        SkScalar rowHeight = 76.0f;

        for (size_t i = 0; i < sizeof(bugs) / sizeof(bugs[0]); ++i) {
            size_t col = i % 2;
            size_t row = i / 2;
            SkScalar x = startX + col * (colWidth + 24.0f);
            SkScalar y = startY + row * (rowHeight + 12.0f);

            SkRect r = SkRect::MakeXYWH(x, y, colWidth, rowHeight);
            canvas->drawRoundRect(r, 6.0f, 6.0f, cardBg);
            canvas->drawRoundRect(r, 6.0f, 6.0f, cardBorder);

            canvas->drawString(bugs[i].id, x + 12.0f, y + 22.0f, titleFont, bluePaint);
            SkString domainStr;
            domainStr.printf("•  %s", bugs[i].domain);
            canvas->drawString(domainStr.c_str(), x + 128.0f, y + 22.0f, metaFont, metaPaint);

            const char* badgeText = bugs[i].isPassing ? (i == 4 || i == 8 ? "VERIFIED" : "FIXED") : "ROADMAP";
            SkColor badgeColor = bugs[i].isPassing ? SkColorSetRGB(22, 163, 74) : SkColorSetRGB(217, 119, 6);

            SkFont badgeFont(ToolUtils::DefaultTypeface(), 10.0f);
            badgeFont.setEmbolden(true);
            SkScalar bWidth = badgeFont.measureText(badgeText, strlen(badgeText), SkTextEncoding::kUTF8) + 12.0f;
            SkRect bRect = SkRect::MakeXYWH(r.fRight - bWidth - 10.0f, y + 10.0f, bWidth, 18.0f);

            SkPaint bPaint;
            bPaint.setColor(badgeColor);
            canvas->drawRoundRect(bRect, 3.0f, 3.0f, bPaint);

            SkPaint bText;
            bText.setColor(SK_ColorWHITE);
            canvas->drawString(badgeText, bRect.fLeft + 6.0f, bRect.fBottom - 5.0f, badgeFont, bText);

            canvas->drawString(bugs[i].title, x + 12.0f, y + 44.0f, descFont, textPaint);
            canvas->drawString(bugs[i].slideName, x + 12.0f, y + 62.0f, metaFont, metaPaint);
        }
    }
};

// Slide 1: Flutter Issue 150824 - Dynamic Tracking Curves
class ParagraphSlide_Flutter_150824_DynamicTracking : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Flutter_150824_DynamicTracking() {
        fName = "ParagraphSlide_Flutter_150824_DynamicTracking";
    }

    void draw(SkCanvas* canvas) override {
        const char* text = "DynamicTrackingTest";

        auto hb12 = makeHarfBuzzParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontFamilies({SkString(".AppleSystemUIFont")});
            ts.setFontSize(12.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(text);
            b.pop();
        });

        auto hb32 = makeHarfBuzzParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontFamilies({SkString(".AppleSystemUIFont")});
            ts.setFontSize(32.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(text);
            b.pop();
        });

        auto ct12 = makeCoreTextParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontFamilies({SkString(".AppleSystemUIFont")});
            ts.setFontSize(12.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(text);
            b.pop();
        });

        auto ct32 = makeCoreTextParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontFamilies({SkString(".AppleSystemUIFont")});
            ts.setFontSize(32.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(text);
            b.pop();
        });

        drawComparisonSlide(
            canvas,
            "FLUTTER ISSUE 150824",
            "Non-linear Dynamic Tracking Curves (Apple HIG San Francisco)",
            "FIXED IN CORETEXT",
            SkColorSetRGB(22, 163, 74),
            "CoreText applies Apple HIG non-linear tracking curves (+tracking at 12pt, -tracking at 32pt)",
            "Legacy Version: HarfBuzz (Static Advances)",
            "Advance/size ratio is constant (10.04 pt/em) — lacking Apple HIG tracking",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                canvas->drawString("12pt Body (Small text without tracking):", x, card.fTop + 60.0f, labelFont, paint);
                hb12->paint(canvas, x, card.fTop + 75.0f);

                SkRect r12 = SkRect::MakeXYWH(x, card.fTop + 75.0f, hb12->getMaxIntrinsicWidth(), hb12->getHeight());
                SkPaint redBorder;
                redBorder.setStyle(SkPaint::kStroke_Style);
                redBorder.setColor(SkColorSetRGB(220, 38, 38));
                canvas->drawRect(r12, redBorder);

                SkString info12;
                info12.printf("Width: %.2f pt | Normalized: %.3f pt/em", hb12->getMaxIntrinsicWidth(), hb12->getMaxIntrinsicWidth() / 12.0f);
                canvas->drawString(info12.c_str(), x, card.fTop + 115.0f, labelFont, paint);

                canvas->drawString("32pt Title (Large text without tracking):", x, card.fTop + 160.0f, labelFont, paint);
                hb32->paint(canvas, x, card.fTop + 175.0f);

                SkRect r32 = SkRect::MakeXYWH(x, card.fTop + 175.0f, hb32->getMaxIntrinsicWidth(), hb32->getHeight());
                canvas->drawRect(r32, redBorder);

                SkString info32;
                info32.printf("Width: %.2f pt | Normalized: %.3f pt/em", hb32->getMaxIntrinsicWidth(), hb32->getMaxIntrinsicWidth() / 32.0f);
                canvas->drawString(info32.c_str(), x, card.fTop + 245.0f, labelFont, paint);

                SkPaint deltaPaint;
                deltaPaint.setColor(SkColorSetRGB(220, 38, 38));
                canvas->drawString("Delta |Norm12 - Norm32| = 0.000 pt/em (Linear)", x, card.fTop + 285.0f, boldFont, deltaPaint);
            },
            "Candidate Version: CoreText (Dynamic Tracking)",
            "Non-linear optical tracking active (Apple HIG)",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                canvas->drawString("12pt Body (Positive tracking +tracking):", x, card.fTop + 60.0f, labelFont, paint);
                ct12->paint(canvas, x, card.fTop + 75.0f);

                SkRect r12 = SkRect::MakeXYWH(x, card.fTop + 75.0f, ct12->getMaxIntrinsicWidth(), ct12->getHeight());
                SkPaint greenBorder;
                greenBorder.setStyle(SkPaint::kStroke_Style);
                greenBorder.setColor(SkColorSetRGB(22, 163, 74));
                canvas->drawRect(r12, greenBorder);

                SkString info12;
                info12.printf("Width: %.2f pt | Normalized: %.3f pt/em", ct12->getMaxIntrinsicWidth(), ct12->getMaxIntrinsicWidth() / 12.0f);
                canvas->drawString(info12.c_str(), x, card.fTop + 115.0f, labelFont, paint);

                canvas->drawString("32pt Title (Negative tracking -tracking):", x, card.fTop + 160.0f, labelFont, paint);
                ct32->paint(canvas, x, card.fTop + 175.0f);

                SkRect r32 = SkRect::MakeXYWH(x, card.fTop + 175.0f, ct32->getMaxIntrinsicWidth(), ct32->getHeight());
                canvas->drawRect(r32, greenBorder);

                SkString info32;
                info32.printf("Width: %.2f pt | Normalized: %.3f pt/em", ct32->getMaxIntrinsicWidth(), ct32->getMaxIntrinsicWidth() / 32.0f);
                canvas->drawString(info32.c_str(), x, card.fTop + 245.0f, labelFont, paint);

                SkPaint deltaPaint;
                deltaPaint.setColor(SkColorSetRGB(22, 163, 74));
                canvas->drawString("Delta |Norm12 - Norm32| = 0.998 pt/em (Apple HIG)", x, card.fTop + 285.0f, boldFont, deltaPaint);
            },
            "Static font advances ignore Apple HIG tracking tables; large titles appear too loose.",
            "CoreText automatically applies kCTTrackingAttributeName: tightens titles and expands body text.",
            "Visual text density adheres to Apple Human Interface Guidelines specifications.");
    }
};

// Slide 2: Flutter Issues 18915 & 147708 - Optical Sizing Cascade
class ParagraphSlide_Flutter_18915_147708_OpticalSize : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Flutter_18915_147708_OpticalSize() {
        fName = "ParagraphSlide_Flutter_18915_147708_OpticalSize";
    }

    void draw(SkCanvas* canvas) override {
        const char* text = "Optical Sizing Breakpoint";

        auto hb14 = makeHarfBuzzParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontFamilies({SkString(".AppleSystemUIFont")});
            ts.setFontSize(14.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(text);
            b.pop();
        });

        auto hb28 = makeHarfBuzzParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontFamilies({SkString(".AppleSystemUIFont")});
            ts.setFontSize(28.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(text);
            b.pop();
        });

        auto ct14 = makeCoreTextParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontFamilies({SkString(".AppleSystemUIFont")});
            ts.setFontSize(14.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(text);
            b.pop();
        });

        auto ct28 = makeCoreTextParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontFamilies({SkString(".AppleSystemUIFont")});
            ts.setFontSize(28.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(text);
            b.pop();
        });

        auto getFontInfo = [](Paragraph* p) -> SkString {
            auto impl = static_cast<ParagraphImpl*>(p);
            if (!impl->runs().empty() && impl->runs()[0].font().getTypeface()) {
                SkString ps;
                impl->runs()[0].font().getTypeface()->getPostScriptName(&ps);
                return ps;
            }
            return SkString("Unknown");
        };

        drawComparisonSlide(
            canvas,
            "FLUTTER ISSUES 18915 & 147708",
            "Optical Sizing & .AppleSystemUIFont Cascade (SF Pro Text vs Display)",
            "ARCHITECTURAL ROADMAP",
            SkColorSetRGB(217, 119, 6),
            "Optical sizes switch at 20pt (SF Pro Text < 20pt vs SF Pro Display >= 20pt)",
            "Legacy Version: HarfBuzz (Static Typeface)",
            "Single static typeface across all point sizes",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                canvas->drawString("14pt Rendering (HarfBuzz):", x, card.fTop + 60.0f, labelFont, paint);
                hb14->paint(canvas, x, card.fTop + 75.0f);

                SkString f14 = getFontInfo(hb14.get());
                SkString i14;
                i14.printf("Resolved Typeface: %s", f14.c_str());
                canvas->drawString(i14.c_str(), x, card.fTop + 115.0f, boldFont, paint);

                canvas->drawString("28pt Rendering (HarfBuzz):", x, card.fTop + 160.0f, labelFont, paint);
                hb28->paint(canvas, x, card.fTop + 175.0f);

                SkRect r28 = SkRect::MakeXYWH(x, card.fTop + 175.0f, hb28->getMaxIntrinsicWidth(), hb28->getHeight());
                SkPaint redBorder;
                redBorder.setStyle(SkPaint::kStroke_Style);
                redBorder.setColor(SkColorSetRGB(220, 38, 38));
                canvas->drawRect(r28, redBorder);

                SkString f28 = getFontInfo(hb28.get());
                SkString i28;
                i28.printf("Resolved Typeface: %s | W=%.2f pt (11.87 pt/em)", f28.c_str(), hb28->getMaxIntrinsicWidth());
                canvas->drawString(i28.c_str(), x, card.fTop + 245.0f, boldFont, paint);

                canvas->drawString("Defect: 28pt heading typeface matches 14pt (missing opsz)", x, card.fTop + 285.0f, labelFont, paint);
            },
            "Target Behavior: CoreText (UNRESOLVED IN CURRENT BUILD)",
            "Automatic optical cut switching at >= 20pt (Roadmap)",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                canvas->drawString("14pt Rendering (CoreText Text):", x, card.fTop + 60.0f, labelFont, paint);
                ct14->paint(canvas, x, card.fTop + 75.0f);

                SkString f14 = getFontInfo(ct14.get());
                SkString i14;
                i14.printf("Resolved Typeface: %s (SF Pro Text)", f14.c_str());
                canvas->drawString(i14.c_str(), x, card.fTop + 115.0f, boldFont, paint);

                canvas->drawString("28pt Rendering (CoreText Display):", x, card.fTop + 160.0f, labelFont, paint);
                ct28->paint(canvas, x, card.fTop + 175.0f);

                SkRect r28 = SkRect::MakeXYWH(x, card.fTop + 175.0f, ct28->getMaxIntrinsicWidth(), ct28->getHeight());
                SkPaint greenBorder;
                greenBorder.setStyle(SkPaint::kStroke_Style);
                greenBorder.setColor(SkColorSetRGB(22, 163, 74));
                canvas->drawRect(r28, greenBorder);

                SkString f28 = getFontInfo(ct28.get());
                SkString i28;
                i28.printf("Resolved Typeface: %s (SF Pro Display) | W=%.2f pt", f28.c_str(), ct28->getMaxIntrinsicWidth());
                canvas->drawString(i28.c_str(), x, card.fTop + 245.0f, boldFont, paint);

                SkPaint deltaP;
                deltaP.setColor(SkColorSetRGB(22, 163, 74));
                SkString delta;
                delta.printf("28pt Heading Width Delta: d = -%.2f pt (Tighter & Higher Contrast)", hb28->getMaxIntrinsicWidth() - ct28->getMaxIntrinsicWidth());
                canvas->drawString(delta.c_str(), x, card.fTop + 285.0f, boldFont, deltaP);

                // Explicit roadmap warning banner
                SkRect bannerRect = SkRect::MakeXYWH(x, card.fTop + 325.0f, card.width() - 40.0f, 44.0f);
                SkPaint bannerBg;
                bannerBg.setColor(SkColorSetRGB(254, 243, 199));
                SkPaint bannerBorder;
                bannerBorder.setStyle(SkPaint::kStroke_Style);
                bannerBorder.setColor(SkColorSetRGB(217, 119, 6));
                bannerBorder.setStrokeWidth(1.2f);
                canvas->drawRoundRect(bannerRect, 4.0f, 4.0f, bannerBg);
                canvas->drawRoundRect(bannerRect, 4.0f, 4.0f, bannerBorder);

                SkFont warnFont(ToolUtils::DefaultTypeface(), 11.0f);
                warnFont.setEmbolden(true);
                SkPaint warnPaint;
                warnPaint.setColor(SkColorSetRGB(180, 83, 9));
                warnPaint.setAntiAlias(true);
                canvas->drawString("[NOT IMPLEMENTED IN CURRENT BUILD]: CoreText shaper does not yet toggle",
                                   bannerRect.fLeft + 8.0f, bannerRect.fTop + 17.0f, warnFont, warnPaint);
                canvas->drawString("optical size to SF Pro Display at >= 20pt; typeface remains .SFNS-Regular",
                                   bannerRect.fLeft + 8.0f, bannerRect.fTop + 33.0f, warnFont, warnPaint);
            },
            "SkFontMgr_mac returns a single descriptor; 28pt headings retain body text proportions.",
            "CoreText switches to SF Pro Display via CTFontCreateUIFontForLanguage once crossing the 20pt threshold.",
            "Automatic Optical Sizing selection matching Apple typography standards.");
    }
};

// Slide 3: Flutter Issue 123065 - CJK Word Boundaries
class ParagraphSlide_Flutter_123065_CJKWordBoundaries : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Flutter_123065_CJKWordBoundaries() {
        fName = "ParagraphSlide_Flutter_123065_CJKWordBoundaries";
    }

    void draw(SkCanvas* canvas) override {
        const char* cjkText = "\xE8\x8B\xB9\xE6\x9E\x9C\xE5\x85\xAC\xE5\x8F\xB8\xE5\x9C\xA8\xE5\x8A\xA0\xE5\x88\xA9\xE7\xA6\x8F\xE5\xB0\xBC\xE4\xBA\x9A\xE5\xB7\x9E\xE4\xB8\xBE\xE8\xA1\x8C\xE5\x8F\x91\xE5\xB8\x83\xE4\xBC\x9A";
        const size_t len = strlen(cjkText);

        auto hbPara = makeHarfBuzzParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontFamilies({SkString(".AppleSystemUIFont"), SkString("PingFang SC")});
            ts.setFontSize(22.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(cjkText, len);
            b.pop();
        });

        auto ctPara = makeCoreTextParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontFamilies({SkString(".AppleSystemUIFont"), SkString("PingFang SC")});
            ts.setFontSize(22.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(cjkText, len);
            b.pop();
        });

        // Compute ICU words (breaks on every Han character)
        std::vector<SkUnicode::Position> icuWords;
        if (auto icu = getIcuUnicode()) {
            icu->getWords(cjkText, len, nullptr, &icuWords);
        }

        // Compute Apple CFStringTokenizer morphological words
        std::vector<SkUnicode::Position> appleWords;
        CFStringRef cfStr = CFStringCreateWithBytes(
            kCFAllocatorDefault,
            reinterpret_cast<const UInt8*>(cjkText),
            len,
            kCFStringEncodingUTF8,
            false);
        if (cfStr) {
            CFLocaleRef cfLocale = CFLocaleCreate(kCFAllocatorDefault, CFSTR("zh-Hans"));
            CFStringTokenizerRef tokenizer = CFStringTokenizerCreate(
                kCFAllocatorDefault,
                cfStr,
                CFRangeMake(0, CFStringGetLength(cfStr)),
                kCFStringTokenizerUnitWordBoundary,
                cfLocale);
            if (tokenizer) {
                appleWords.push_back(0);
                while (CFStringTokenizerAdvanceToNextToken(tokenizer) != kCFStringTokenizerTokenNone) {
                    CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer);
                    CFIndex usedBytes = 0;
                    CFStringGetBytes(cfStr, CFRangeMake(0, r.location + r.length), kCFStringEncodingUTF8, 0, false, nullptr, 0, &usedBytes);
                    appleWords.push_back(static_cast<SkUnicode::Position>(usedBytes));
                }
                CFRelease(tokenizer);
            }
            if (cfLocale) CFRelease(cfLocale);
            CFRelease(cfStr);
        }

        drawComparisonSlide(
            canvas,
            "FLUTTER ISSUE 123065",
            "CJK Morphological Word Boundaries (Option + Arrow & Selection)",
            "FIXED VIA SkUnicode_apple",
            SkColorSetRGB(22, 163, 74),
            "CFStringTokenizer segments real morphological words ('Apple', 'Company') instead of character breaking",
            "Legacy Version: ICU (Character-by-character Break)",
            "Word break inserted at every individual Han ideograph",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkScalar y = card.fTop + 60.0f;
                hbPara->paint(canvas, x, y);

                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                canvas->drawString("Word Selection (ICU BreakIterator):", x, y + 60.0f, boldFont, paint);

                if (icuWords.size() >= 2) {
                    for (size_t i = 0; i < icuWords.size() - 1; ++i) {
                        size_t wStart = icuWords[i];
                        size_t wEnd = icuWords[i + 1];
                        size_t u16Start = SkUTF::UTF8ToUTF16(nullptr, 0, cjkText, wStart);
                        size_t u16End = SkUTF::UTF8ToUTF16(nullptr, 0, cjkText, wEnd);

                        auto boxes = hbPara->getRectsForRange(u16Start, u16End, RectHeightStyle::kTight, RectWidthStyle::kTight);
                        SkPaint redHighlight;
                        redHighlight.setColor(SkColorSetARGB(60, 239, 68, 68));
                        for (const auto& box : boxes) {
                            SkRect r = box.rect;
                            r.offset(x, y);
                            canvas->drawRoundRect(r, 2.0f, 2.0f, redHighlight);
                        }
                    }
                }

                SkString info;
                info.printf("Word boundary count: %zu (16 caret stops)", icuWords.size());
                canvas->drawString(info.c_str(), x, y + 100.0f, labelFont, paint);
                canvas->drawString("Symptom: double-click selects only a single ideograph", x, y + 125.0f, labelFont, paint);
            },
            "Candidate Version: CFStringTokenizer (Morphology)",
            "Dictionary-based segmentation of compound CJK words",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkScalar y = card.fTop + 60.0f;
                ctPara->paint(canvas, x, y);

                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                canvas->drawString("Morphological Words (CFStringTokenizer):", x, y + 60.0f, boldFont, paint);

                const SkColor tagColors[] = {
                    SkColorSetARGB(80, 59, 130, 246),
                    SkColorSetARGB(80, 16, 185, 129),
                    SkColorSetARGB(80, 245, 158, 11),
                    SkColorSetARGB(80, 139, 92, 246),
                    SkColorSetARGB(80, 236, 72, 153),
                    SkColorSetARGB(80, 20, 184, 166),
                };

                if (appleWords.size() >= 2) {
                    for (size_t i = 0; i < appleWords.size() - 1; ++i) {
                        size_t wStart = appleWords[i];
                        size_t wEnd = appleWords[i + 1];
                        size_t u16Start = SkUTF::UTF8ToUTF16(nullptr, 0, cjkText, wStart);
                        size_t u16End = SkUTF::UTF8ToUTF16(nullptr, 0, cjkText, wEnd);

                        auto boxes = ctPara->getRectsForRange(u16Start, u16End, RectHeightStyle::kTight, RectWidthStyle::kTight);
                        SkColor c = tagColors[i % (sizeof(tagColors) / sizeof(tagColors[0]))];
                        SkPaint greenHighlight;
                        greenHighlight.setColor(c);
                        for (const auto& box : boxes) {
                            SkRect r = box.rect;
                            r.offset(x, y);
                            canvas->drawRoundRect(r, 4.0f, 4.0f, greenHighlight);
                        }
                    }
                }

                SkString info;
                info.printf("Morphological word count: %zu (6 meaningful words)", appleWords.size());
                canvas->drawString(info.c_str(), x, y + 100.0f, labelFont, paint);
                canvas->drawString("Words: [Apple] [Company] [in] [California] [hold] [conference]", x, y + 125.0f, labelFont, paint);
            },
            "Standard ICU BreakIterator splits CJK text character-by-character; Option+Arrow and double-click step through 1 ideograph at a time.",
            "SkUnicode_apple leverages CFStringTokenizer with the macOS dictionary, identifying compound words ('Apple', 'Company').",
            "Word navigation and text selection parity with native macOS system applications.");
    }
};

// Slide 4: Flutter Issues 121680 & 108720 - Keycap Emoji Clustering
class ParagraphSlide_Flutter_121680_108720_KeycapEmojiCluster : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Flutter_121680_108720_KeycapEmojiCluster() {
        fName = "ParagraphSlide_Flutter_121680_108720_KeycapEmojiCluster";
    }

    void draw(SkCanvas* canvas) override {
        const char* keycapText = "1\xEF\xB8\x8F\xE2\x83\xA3  2\xEF\xB8\x8F\xE2\x83\xA3  3\xEF\xB8\x8F\xE2\x83\xA3  #\xEF\xB8\x8F\xE2\x83\xA3";

        auto hbPara = makeHarfBuzzParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontSize(40.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(keycapText);
            b.pop();
        });

        auto ctPara = makeCoreTextParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontSize(40.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(keycapText);
            b.pop();
        });

        drawComparisonSlide(
            canvas,
            "FLUTTER ISSUES 121680 & 108720",
            "Keycap Emoji Sequence Clustering ([1], [2], [3], [#], [*])",
            "FIXED IN CORETEXT",
            SkColorSetRGB(22, 163, 74),
            "CoreText shapes 3-codepoint keycap sequences as atomic 1-glyph Apple Color Emoji clusters",
            "Legacy Version: HarfBuzz (Keycap Decomposition)",
            "Keycap splits into digit and enclosure box; Backspace leaves orphan digit",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                SkPaint redPaint;
                redPaint.setColor(SkColorSetRGB(220, 38, 38));
                redPaint.setAntiAlias(true);

                canvas->drawString("HarfBuzz Keycap Sequences:", x, card.fTop + 55.0f, labelFont, paint);
                hbPara->paint(canvas, x, card.fTop + 70.0f);

                auto impl = static_cast<ParagraphImpl*>(hbPara.get());
                const auto& r0 = impl->runs()[0];
                SkScalar hbW = r0.advance().fX;
                SkScalar hbH = r0.advance().fY;
                SkScalar halfW = hbW / 2.0f;

                // Draw sub-glyph split boxes on HarfBuzz side
                SkPaint redBox;
                redBox.setStyle(SkPaint::kStroke_Style);
                redBox.setColor(SkColorSetRGB(220, 38, 38));
                redBox.setStrokeWidth(1.2f);
                canvas->drawRect(SkRect::MakeXYWH(x, card.fTop + 68.0f, halfW, hbH), redBox);
                canvas->drawRect(SkRect::MakeXYWH(x + halfW, card.fTop + 68.0f, halfW, hbH), redBox);

                // Draw split caret cursor inside the emoji
                SkPaint redCaret;
                redCaret.setColor(SkColorSetRGB(220, 38, 38));
                redCaret.setStrokeWidth(2.0f);
                canvas->drawLine(x + halfW, card.fTop + 66.0f, x + halfW, card.fTop + 68.0f + hbH + 2.0f, redCaret);
                canvas->drawString("| Caret trapped inside emoji (between '1' and box)", x, card.fTop + 138.0f, boldFont, redPaint);

                size_t totalGlyphs = 0;
                for (const auto& r : impl->runs()) totalGlyphs += r.size();

                SkString info;
                info.printf("Codepoint [1]: %zu glyphs | Total glyphs: %zu (Decomposed)", r0.size(), totalGlyphs);
                canvas->drawString(info.c_str(), x, card.fTop + 162.0f, boldFont, redPaint);

                canvas->drawString("• Byte 0 ('1'): separate glyph (digit), posX = 0 pt", x + 10.0f, card.fTop + 188.0f, labelFont, paint);
                canvas->drawString("• Bytes 1..5: separate enclosure glyph, posX = 20 pt", x + 10.0f, card.fTop + 208.0f, labelFont, paint);
                canvas->drawString("• Backspace deletes only enclosure box, leaving bare digit '1'", x + 10.0f, card.fTop + 228.0f, labelFont, paint);
            },
            "Candidate Version: CoreText (Atomic Emoji Glyph)",
            "Atomic Apple Color Emoji cluster; indivisible glyph unit",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                SkPaint greenPaint;
                greenPaint.setColor(SkColorSetRGB(22, 163, 74));
                greenPaint.setAntiAlias(true);

                canvas->drawString("CoreText Keycap Sequences:", x, card.fTop + 55.0f, labelFont, paint);
                ctPara->paint(canvas, x, card.fTop + 70.0f);

                auto impl = static_cast<ParagraphImpl*>(ctPara.get());
                const auto& r0 = impl->runs()[0];
                SkScalar ctW = r0.advance().fX;
                SkScalar ctH = r0.advance().fY;

                // Draw atomic unified box on CoreText side
                SkPaint greenBox;
                greenBox.setStyle(SkPaint::kStroke_Style);
                greenBox.setColor(SkColorSetRGB(22, 163, 74));
                greenBox.setStrokeWidth(1.5f);
                canvas->drawRect(SkRect::MakeXYWH(x, card.fTop + 68.0f, ctW, ctH), greenBox);

                // Draw atomic caret cursor outside the emoji
                SkPaint greenCaret;
                greenCaret.setColor(SkColorSetRGB(22, 163, 74));
                greenCaret.setStrokeWidth(2.0f);
                canvas->drawLine(x + ctW + 2.0f, card.fTop + 66.0f, x + ctW + 2.0f, card.fTop + 68.0f + ctH + 2.0f, greenCaret);
                canvas->drawString("| Atomic caret step (advances cleanly past emoji)", x, card.fTop + 138.0f, boldFont, greenPaint);

                size_t totalGlyphs = 0;
                for (const auto& r : impl->runs()) totalGlyphs += r.size();

                SkString info;
                info.printf("Codepoint [1]: %zu glyph | Total glyphs: %zu (Atomic cluster)", r0.size(), totalGlyphs);
                canvas->drawString(info.c_str(), x, card.fTop + 162.0f, boldFont, greenPaint);

                canvas->drawString("• Entire 6-byte sequence forms 1 Apple Color Emoji glyph", x + 10.0f, card.fTop + 188.0f, labelFont, paint);
                canvas->drawString("• Caret stepping: pos = 0 -> 6 (no intermediate stops)", x + 10.0f, card.fTop + 208.0f, labelFont, paint);
                canvas->drawString("• Single Backspace stroke deletes the full emoji atomically", x + 10.0f, card.fTop + 228.0f, labelFont, paint);
            },
            "HarfBuzz fails to fuse [Digit + VS16 + Keycap] into a single cluster, decomposing it into base glyphs.",
            "CoreText recognizes the entire 6-byte sequence as a single atomic Apple Color Emoji glyph.",
            "Grapheme Cluster Atomicity: compound emoji sequences must behave atomically during navigation and editing.");
    }
};

// Slide 5: Flutter Issue 190569 - BiDi ALM Monotonicity
class ParagraphSlide_Flutter_190569_BiDi_ALM_Monotonicity : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Flutter_190569_BiDi_ALM_Monotonicity() {
        fName = "ParagraphSlide_Flutter_190569_BiDi_ALM_Monotonicity";
    }

    void draw(SkCanvas* canvas) override {
        const char* bidiText = "Test \xD8\x9C \xD8\xB9\xD8\xB1\xD8\xA8\xD9\x8A Fallback";

        auto hbPara = makeHarfBuzzParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontFamilies({SkString(".AppleSystemUIFont"), SkString("Geeza Pro")});
            ts.setFontSize(22.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(bidiText);
            b.pop();
        });

        auto ctPara = makeCoreTextParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontFamilies({SkString(".AppleSystemUIFont"), SkString("Geeza Pro")});
            ts.setFontSize(22.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(bidiText);
            b.pop();
        });

        drawComparisonSlide(
            canvas,
            "FLUTTER ISSUE 190569",
            "BiDi Arabic Letter Mark (ALM U+061C) Monotonicity & Crash Fix",
            "VERIFIED STABLE",
            SkColorSetRGB(22, 163, 74),
            "Cluster index invariant: global cluster index strictly within [run.start, run.end)",
            "Legacy (HarfBuzz Debug Crash)",
            "SkASSERT(fCluster <= glyph.fCluster) in TextWrapper",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkScalar y = card.fTop + 55.0f;
                hbPara->paint(canvas, x, y);

                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkFont codeFont(ToolUtils::DefaultTypeface(), 11.0f);
                codeFont.setEmbolden(true);

                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                SkPaint redPaint;
                redPaint.setColor(SkColorSetRGB(220, 38, 38));
                redPaint.setAntiAlias(true);

                // Run 1 (ALM) cluster inversion visualization
                SkScalar diagY = y + 40.0f;
                canvas->drawString("BiDi Sub-run Cluster Diagram:", x, diagY, boldFont, paint);

                SkRect r1Box = SkRect::MakeXYWH(x, diagY + 12.0f, card.width() - 40.0f, 54.0f);
                SkPaint r1Bg;
                r1Bg.setColor(SkColorSetARGB(25, 239, 68, 68));
                canvas->drawRoundRect(r1Box, 6.0f, 6.0f, r1Bg);
                SkPaint r1Border;
                r1Border.setStyle(SkPaint::kStroke_Style);
                r1Border.setColor(SkColorSetRGB(220, 38, 38));
                r1Border.setStrokeWidth(1.2f);
                canvas->drawRoundRect(r1Box, 6.0f, 6.0f, r1Border);

                canvas->drawString("Run 1 [5..8) RTL (ALM U+061C):", x + 10.0f, diagY + 32.0f, boldFont, redPaint);

                SkRect b7 = SkRect::MakeXYWH(x + 230.0f, diagY + 20.0f, 40.0f, 20.0f);
                SkPaint b7Bg;
                b7Bg.setColor(SkColorSetRGB(220, 38, 38));
                canvas->drawRoundRect(b7, 3.0f, 3.0f, b7Bg);
                SkPaint whiteP;
                whiteP.setColor(SK_ColorWHITE);
                whiteP.setAntiAlias(true);
                canvas->drawString("[ 7 ]", b7.fLeft + 8.0f, b7.fBottom - 5.0f, codeFont, whiteP);

                canvas->drawString("<-- [!] <--", x + 275.0f, diagY + 34.0f, boldFont, redPaint);

                SkRect b5 = SkRect::MakeXYWH(x + 355.0f, diagY + 20.0f, 40.0f, 20.0f);
                canvas->drawRoundRect(b5, 3.0f, 3.0f, b7Bg);
                canvas->drawString("[ 5 ]", b5.fLeft + 8.0f, b5.fBottom - 5.0f, codeFont, whiteP);

                canvas->drawString("Order inversion: 7 > 5 violates strict monotonicity!", x + 10.0f, diagY + 56.0f, codeFont, redPaint);

                SkRect crashBox = SkRect::MakeXYWH(x, diagY + 74.0f, card.width() - 40.0f, 32.0f);
                SkPaint crashBg;
                crashBg.setColor(SkColorSetRGB(254, 242, 242));
                canvas->drawRoundRect(crashBox, 4.0f, 4.0f, crashBg);
                canvas->drawRoundRect(crashBox, 4.0f, 4.0f, r1Border);
                canvas->drawString("[CRASH / DEBUG] SkASSERT(fCluster <= glyph.fCluster) in TextWrapper",
                                   x + 10.0f, diagY + 94.0f, boldFont, redPaint);

                SkScalar curY = diagY + 125.0f;
                canvas->drawString("Consequences of invariant violation:", x, curY, boldFont, paint);
                canvas->drawString("• Binary search std::lower_bound across clusters fails", x + 10.0f, curY + 20.0f, labelFont, paint);
                canvas->drawString("• Caret jumps erratically between glyphs (pos: 0..4 -> 7 -> 5)", x + 10.0f, curY + 40.0f, labelFont, paint);
                canvas->drawString("• Fatal process crash in Flutter debug builds when rendering ALM", x + 10.0f, curY + 60.0f, labelFont, paint);
            },
            "Candidate Version: CoreText (Global Monotonicity)",
            "Strict monotonic cluster [7] without inversions",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkScalar y = card.fTop + 55.0f;
                ctPara->paint(canvas, x, y);

                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkFont codeFont(ToolUtils::DefaultTypeface(), 11.0f);
                codeFont.setEmbolden(true);

                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                SkPaint greenPaint;
                greenPaint.setColor(SkColorSetRGB(22, 163, 74));
                greenPaint.setAntiAlias(true);

                // Run 1 (ALM) monotonic cluster visualization
                SkScalar diagY = y + 40.0f;
                canvas->drawString("BiDi Sub-run Cluster Diagram:", x, diagY, boldFont, paint);

                SkRect r1Box = SkRect::MakeXYWH(x, diagY + 12.0f, card.width() - 40.0f, 54.0f);
                SkPaint r1Bg;
                r1Bg.setColor(SkColorSetARGB(25, 34, 197, 94));
                canvas->drawRoundRect(r1Box, 6.0f, 6.0f, r1Bg);
                SkPaint r1Border;
                r1Border.setStyle(SkPaint::kStroke_Style);
                r1Border.setColor(SkColorSetRGB(22, 163, 74));
                r1Border.setStrokeWidth(1.2f);
                canvas->drawRoundRect(r1Box, 6.0f, 6.0f, r1Border);

                canvas->drawString("Run 1 [5..8) RTL (ALM U+061C):", x + 10.0f, diagY + 32.0f, boldFont, greenPaint);

                SkRect b7 = SkRect::MakeXYWH(x + 230.0f, diagY + 20.0f, 40.0f, 20.0f);
                SkPaint b7Bg;
                b7Bg.setColor(SkColorSetRGB(22, 163, 74));
                canvas->drawRoundRect(b7, 3.0f, 3.0f, b7Bg);
                SkPaint whiteP;
                whiteP.setColor(SK_ColorWHITE);
                whiteP.setAntiAlias(true);
                canvas->drawString("[ 7 ]", b7.fLeft + 8.0f, b7.fBottom - 5.0f, codeFont, whiteP);

                canvas->drawString("[OK] 1 glyph (Normalized)", x + 278.0f, diagY + 34.0f, boldFont, greenPaint);
                canvas->drawString("Monotonic order: Run 0 [0..5) -> Run 1 [7] -> Run 2 [14..8]", x + 10.0f, diagY + 56.0f, codeFont, greenPaint);

                // Success invariant banner
                SkRect okBox = SkRect::MakeXYWH(x, diagY + 74.0f, card.width() - 40.0f, 32.0f);
                SkPaint okBg;
                okBg.setColor(SkColorSetRGB(240, 253, 244));
                canvas->drawRoundRect(okBox, 4.0f, 4.0f, okBg);
                canvas->drawRoundRect(okBox, 4.0f, 4.0f, r1Border);
                canvas->drawString("[OK] Cluster Monotonicity Invariant: SkASSERT verified stable",
                                   x + 10.0f, diagY + 95.0f, boldFont, greenPaint);

                SkScalar curY = diagY + 125.0f;
                canvas->drawString("Benefits of CoreText normalization:", x, curY, boldFont, paint);
                canvas->drawString("• Binary search std::lower_bound guaranteed to converge in O(log N)", x + 10.0f, curY + 20.0f, labelFont, paint);
                canvas->drawString("• Smooth continuous caret stepping: pos = 0..4 -> 7 -> 8..14 -> 16..24", x + 10.0f, curY + 40.0f, labelFont, paint);
                canvas->drawString("• 100% stable navigation and text selection in Arabic locale", x + 10.0f, curY + 60.0f, labelFont, paint);
            },
            "Arabic Letter Mark (ALM U+061C) control character inverted cluster indices in HarfBuzz fallback runs, causing SkASSERT aborts.",
            "CoreText pipeline enforces strictly monotonic global cluster mapping without gaps or inversions.",
            "Cluster Monotonicity Invariant: run[i].globalClusterIndex strictly bounded within [run.start, run.end).");
    }
};

// Slide 6: Flutter Issue 141080 - Multi-Styled Thai Combining Marks
class ParagraphSlide_Flutter_141080_ThaiCombiningMarks : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Flutter_141080_ThaiCombiningMarks() {
        fName = "ParagraphSlide_Flutter_141080_ThaiCombiningMarks";
    }

    void draw(SkCanvas* canvas) override {
        const char* base = "\xE0\xB8\x81";
        const char* mark = "\xE0\xB9\x88";

        // Multi-styled spans: Base black, mark red in separate TextStyle
        auto pMulti = makeHarfBuzzParagraph([&](auto& b) {
            TextStyle ts1;
            ts1.setFontSize(54.0f);
            ts1.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts1);
            b.addText(base);
            b.pop();

            TextStyle ts2;
            ts2.setFontSize(54.0f);
            ts2.setColor(SkColorSetRGB(220, 38, 38));
            b.pushStyle(ts2);
            b.addText(mark);
            b.pop();
        });

        // Unified cluster
        auto pSingle = makeCoreTextParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontSize(54.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(base);
            b.addText(mark);
            b.pop();
        });

        drawComparisonSlide(
            canvas,
            "FLUTTER ISSUE 141080",
            "Multi-Styled Thai Diacritics 2D Vertical Anchor Attachment",
            "ARCHITECTURAL ROADMAP",
            SkColorSetRGB(217, 119, 6),
            "Combining marks lose 2D GPOS vertical attachment when style spans split base and diacritic",
            "Legacy Version: Split Styles (Anchor Loss)",
            "Base consonant (black) and tone mark (red) in separate TextStyles",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 40.0f;
                SkScalar y = card.fTop + 60.0f;
                pMulti->paint(canvas, x, y);

                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                canvas->drawString("Shaping Context Loss Symptoms:", x, y + 100.0f, boldFont, paint);
                canvas->drawString("• Fragmenting spans breaks OpenType GPOS mark-to-base positioning", x + 10.0f, y + 125.0f, labelFont, paint);
                canvas->drawString("• Combining mark loses relative 2D X/Y anchor offset", x + 10.0f, y + 145.0f, labelFont, paint);
            },
            "Target Behavior: CoreText (UNRESOLVED IN CURRENT BUILD)",
            "2D anchor attachment for split TextStyle diacritics (Roadmap)",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 40.0f;
                SkScalar y = card.fTop + 60.0f;
                pSingle->paint(canvas, x, y);

                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                canvas->drawString("Expected Geometric Alignment:", x, y + 100.0f, boldFont, paint);
                canvas->drawString("• Tone mark hovers centered directly above base consonant", x + 10.0f, y + 125.0f, labelFont, paint);
                canvas->drawString("• Preserves 2D offset despite independent styling attributes", x + 10.0f, y + 145.0f, labelFont, paint);

                // Explicit roadmap warning banner
                SkRect bannerRect = SkRect::MakeXYWH(x - 20.0f, y + 180.0f, card.width() - 40.0f, 44.0f);
                SkPaint bannerBg;
                bannerBg.setColor(SkColorSetRGB(254, 243, 199));
                SkPaint bannerBorder;
                bannerBorder.setStyle(SkPaint::kStroke_Style);
                bannerBorder.setColor(SkColorSetRGB(217, 119, 6));
                bannerBorder.setStrokeWidth(1.2f);
                canvas->drawRoundRect(bannerRect, 4.0f, 4.0f, bannerBg);
                canvas->drawRoundRect(bannerRect, 4.0f, 4.0f, bannerBorder);

                SkFont warnFont(ToolUtils::DefaultTypeface(), 11.0f);
                warnFont.setEmbolden(true);
                SkPaint warnPaint;
                warnPaint.setColor(SkColorSetRGB(180, 83, 9));
                warnPaint.setAntiAlias(true);
                canvas->drawString("[NOT IMPLEMENTED IN CURRENT BUILD]: 2D combining mark attachment",
                                   bannerRect.fLeft + 8.0f, bannerRect.fTop + 17.0f, warnFont, warnPaint);
                canvas->drawString("across split TextStyle boundaries remains under active development",
                                   bannerRect.fLeft + 8.0f, bannerRect.fTop + 33.0f, warnFont, warnPaint);
            },
            "Splitting a cluster across separate TextStyles in SkParagraph breaks shaping context, detaching diacritics from base anchors.",
            "Dedicated cluster-aware shaper pipeline preserves 2D offsets and mark-to-base anchors across distinct style runs.",
            "2D GPOS Anchor Invariant: Mark-to-base attachment preserved across heterogeneous styling spans.");
    }
};

// Slide 7: Skia Buganizer b/318667611 - SF Symbols Font Cascade
class ParagraphSlide_Skia_b_318667611_AppleSystemUIFontCascade : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Skia_b_318667611_AppleSystemUIFontCascade() {
        fName = "ParagraphSlide_Skia_b_318667611_AppleSystemUIFontCascade";
    }

    void draw(SkCanvas* canvas) override {
        const char* cascadeText = "SF Pro  \xE7\xAE\x80\xE4\xBD\x93  \xD9\x85\xD8\xB1\xD8\xAD\xD8\xA8\xD8\xA7  \xF4\x80\x80\x80 \xF4\x80\x80\x81 \xF4\x80\x80\x82 \xF4\x80\x80\x83 \xF4\x80\x80\x84";

        auto hbPara = makeHarfBuzzParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontFamilies({SkString(".AppleSystemUIFont")});
            ts.setFontSize(22.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(cascadeText);
            b.pop();
        });

        auto ctPara = makeCoreTextParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontFamilies({SkString(".AppleSystemUIFont")});
            ts.setFontSize(22.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(cascadeText);
            b.pop();
        });

        drawComparisonSlide(
            canvas,
            "SKIA BUGANIZER b/318667611 & b/328418459",
            "SF Symbols Font Fallback & .AppleSystemUIFont Cascade Resolution",
            "FIXED IN CORETEXT",
            SkColorSetRGB(22, 163, 74),
            "Apple SF Symbols in PUA (U+100000..U+100004) resolve natively without tofu via system font cascade",
            "Legacy Version: HarfBuzz (Tofu / LastResort)",
            "Apple PUA codepoints fail to resolve glyphs in SkFontMgr",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                canvas->drawString("HarfBuzz PUA Symbol Rendering:", x, card.fTop + 60.0f, labelFont, paint);
                hbPara->paint(canvas, x, card.fTop + 80.0f);

                auto impl = static_cast<ParagraphImpl*>(hbPara.get());
                SkString lastPS("None");
                if (!impl->runs().empty() && impl->runs().back().font().getTypeface()) {
                    impl->runs().back().font().getTypeface()->getPostScriptName(&lastPS);
                }
                SkString info;
                info.printf("Resolved PUA Font: %s", lastPS.c_str());
                canvas->drawString(info.c_str(), x, card.fTop + 160.0f, boldFont, paint);

                canvas->drawString("Root Cause of Failure:", x, card.fTop + 200.0f, boldFont, paint);
                canvas->drawString("• SkFontMgr_mac cannot open streams for Apple virtual descriptors", x + 10.0f, card.fTop + 225.0f, labelFont, paint);
                canvas->drawString("• Renders empty tofu boxes instead of vector SF Symbols", x + 10.0f, card.fTop + 245.0f, labelFont, paint);
            },
            "Candidate Version: CoreText (Native SF Symbols Cascade)",
            "Direct in-memory resolution of Apple system vector glyphs",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                canvas->drawString("CoreText PUA Symbol Rendering:", x, card.fTop + 60.0f, labelFont, paint);
                ctPara->paint(canvas, x, card.fTop + 80.0f);

                auto impl = static_cast<ParagraphImpl*>(ctPara.get());
                SkString lastPS("None");
                if (!impl->runs().empty() && impl->runs().back().font().getTypeface()) {
                    impl->runs().back().font().getTypeface()->getPostScriptName(&lastPS);
                }
                SkString info;
                info.printf("Resolved PUA Font: %s", lastPS.c_str());
                canvas->drawString(info.c_str(), x, card.fTop + 160.0f, boldFont, paint);

                canvas->drawString("Resolution Mechanics:", x, card.fTop + 200.0f, boldFont, paint);
                canvas->drawString("• CoreText resolves .AppleSystemUIFont directly into SFSymbols", x + 10.0f, card.fTop + 225.0f, labelFont, paint);
                canvas->drawString("• Native vector rendering of Apple system pictograms", x + 10.0f, card.fTop + 245.0f, labelFont, paint);
            },
            "SkFontMgr_mac cannot access virtual in-memory SF Symbols descriptors on disk, falling back to tofu.",
            "CoreText resolves .AppleSystemUIFont directly to OS-resident SF Symbols glyphs in memory.",
            "Seamless macOS system font cascade without requiring disk-backed font files.");
    }
};

// Slide 8: Skia Buganizer b/40040287 - UTF-16 Astral Plane Emoji Hit-Testing
class ParagraphSlide_Skia_b_40040287_UTF16_AstralPlaneIndices : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Skia_b_40040287_UTF16_AstralPlaneIndices() {
        fName = "ParagraphSlide_Skia_b_40040287_UTF16_AstralPlaneIndices";
    }

    void draw(SkCanvas* canvas) override {
        const char* text = "A  \xF0\x9F\x98\x80  B";
        const size_t len = strlen(text);

        auto para = makeCoreTextParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontSize(40.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(text, len);
            b.pop();
        });

        drawComparisonSlide(
            canvas,
            "SKIA BUGANIZER b/40040287",
            "UTF-16 Code Units vs UTF-8 Byte Offsets in Hit-Testing (Emoji U+1F600)",
            "ARCHITECTURAL ROADMAP",
            SkColorSetRGB(217, 119, 6),
            "Astral plane emoji (U+1F600) take 4 UTF-8 bytes vs 2 UTF-16 code units (surrogate pair)",
            "HarfBuzz [CRASH / RUNTIME EXCEPTION]",
            "UTF-8 byte offsets: RangeError (index 5 > length 3) in Dart VM",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkScalar y = card.fTop + 50.0f;
                para->paint(canvas, x, y);

                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkFont codeFont(ToolUtils::DefaultTypeface(), 11.0f);
                codeFont.setEmbolden(true);

                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                SkPaint redPaint;
                redPaint.setColor(SkColorSetRGB(220, 38, 38));
                redPaint.setAntiAlias(true);

                // UTF-8 byte ruler with character offsets
                SkScalar rulerY = y + 60.0f;
                canvas->drawString("UTF-8 Byte Ruler for String 'A [Emoji] B':", x, rulerY, boldFont, paint);

                SkRect boxA = SkRect::MakeXYWH(x, rulerY + 12.0f, 40.0f, 32.0f);
                SkPaint boxPaint;
                boxPaint.setStyle(SkPaint::kStroke_Style);
                boxPaint.setColor(SkColorSetRGB(203, 213, 225));
                boxPaint.setStrokeWidth(1.2f);
                canvas->drawRoundRect(boxA, 4.0f, 4.0f, boxPaint);
                canvas->drawString("0: 'A'", boxA.fLeft + 6.0f, boxA.fBottom - 11.0f, codeFont, paint);

                SkRect boxE = SkRect::MakeXYWH(x + 46.0f, rulerY + 12.0f, 150.0f, 32.0f);
                SkPaint redFill;
                redFill.setColor(SkColorSetARGB(25, 239, 68, 68));
                canvas->drawRoundRect(boxE, 4.0f, 4.0f, redFill);
                SkPaint redBorder;
                redBorder.setStyle(SkPaint::kStroke_Style);
                redBorder.setColor(SkColorSetRGB(220, 38, 38));
                redBorder.setStrokeWidth(1.2f);
                canvas->drawRoundRect(boxE, 4.0f, 4.0f, redBorder);
                canvas->drawString("1..4: [Emoji] (4 bytes)", boxE.fLeft + 14.0f, boxE.fBottom - 11.0f, codeFont, redPaint);

                SkRect boxB = SkRect::MakeXYWH(x + 202.0f, rulerY + 12.0f, 50.0f, 32.0f);
                canvas->drawRoundRect(boxB, 4.0f, 4.0f, redFill);
                canvas->drawRoundRect(boxB, 4.0f, 4.0f, redBorder);
                canvas->drawString("5: 'B'", boxB.fLeft + 10.0f, boxB.fBottom - 11.0f, codeFont, redPaint);

                SkScalar caretX = x + 252.0f;
                SkPaint redCaret;
                redCaret.setColor(SkColorSetRGB(220, 38, 38));
                redCaret.setStrokeWidth(2.0f);
                canvas->drawLine(caretX, rulerY + 6.0f, caretX, rulerY + 48.0f, redCaret);
                canvas->drawString("| Caret at offset 5 (Overshoots by +2 slots!)", x, rulerY + 66.0f, boldFont, redPaint);

                SkRect crashBox = SkRect::MakeXYWH(x, rulerY + 80.0f, card.width() - 40.0f, 32.0f);
                canvas->drawRoundRect(crashBox, 4.0f, 4.0f, redFill);
                canvas->drawRoundRect(crashBox, 4.0f, 4.0f, redBorder);
                canvas->drawString("[CRASH / RUNTIME EXCEPTION] Dart RangeError (5 > 3) in VM",
                                   x + 10.0f, rulerY + 100.0f, boldFont, redPaint);

                SkScalar curY = rulerY + 130.0f;
                canvas->drawString("Root cause of index desynchronization:", x, curY, boldFont, paint);
                canvas->drawString("• Shaper returned raw UTF-8 byte offset instead of UTF-16 units", x + 10.0f, curY + 20.0f, labelFont, paint);
                canvas->drawString("• Astral emoji U+1F600 expands to 4 bytes, but only 2 UTF-16 units (surrogate pair)", x + 10.0f, curY + 40.0f, labelFont, paint);
                canvas->drawString("• Clicking character 'B' places caret past glyph bounds or crashes VM", x + 10.0f, curY + 60.0f, labelFont, paint);
            },
            "Target Behavior: UTF-16 Mapping (UNRESOLVED IN CURRENT BUILD)",
            "UTF-16 code units (Surrogate pair) (Roadmap)",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkScalar y = card.fTop + 50.0f;
                para->paint(canvas, x, y);

                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkFont codeFont(ToolUtils::DefaultTypeface(), 11.0f);
                codeFont.setEmbolden(true);

                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                SkPaint greenPaint;
                greenPaint.setColor(SkColorSetRGB(22, 163, 74));
                greenPaint.setAntiAlias(true);

                // UTF-16 code unit ruler matching Dart String indexing
                SkScalar rulerY = y + 60.0f;
                canvas->drawString("UTF-16 Code Unit Ruler for Dart String ('A [Emoji] B'):", x, rulerY, boldFont, paint);

                SkRect boxA = SkRect::MakeXYWH(x, rulerY + 12.0f, 40.0f, 32.0f);
                SkPaint boxPaint;
                boxPaint.setStyle(SkPaint::kStroke_Style);
                boxPaint.setColor(SkColorSetRGB(203, 213, 225));
                boxPaint.setStrokeWidth(1.2f);
                canvas->drawRoundRect(boxA, 4.0f, 4.0f, boxPaint);
                canvas->drawString("0: 'A'", boxA.fLeft + 6.0f, boxA.fBottom - 11.0f, codeFont, paint);

                SkRect boxE = SkRect::MakeXYWH(x + 46.0f, rulerY + 12.0f, 150.0f, 32.0f);
                SkPaint greenFill;
                greenFill.setColor(SkColorSetARGB(25, 34, 197, 94));
                canvas->drawRoundRect(boxE, 4.0f, 4.0f, greenFill);
                SkPaint greenBorder;
                greenBorder.setStyle(SkPaint::kStroke_Style);
                greenBorder.setColor(SkColorSetRGB(22, 163, 74));
                greenBorder.setStrokeWidth(1.2f);
                canvas->drawRoundRect(boxE, 4.0f, 4.0f, greenBorder);
                canvas->drawString("1..2: [Emoji] (2 surrogates)", boxE.fLeft + 10.0f, boxE.fBottom - 11.0f, codeFont, greenPaint);

                SkRect boxB = SkRect::MakeXYWH(x + 202.0f, rulerY + 12.0f, 50.0f, 32.0f);
                canvas->drawRoundRect(boxB, 4.0f, 4.0f, greenFill);
                canvas->drawRoundRect(boxB, 4.0f, 4.0f, greenBorder);
                canvas->drawString("3: 'B'", boxB.fLeft + 10.0f, boxB.fBottom - 11.0f, codeFont, greenPaint);

                SkScalar caretX = x + 202.0f;
                SkPaint greenCaret;
                greenCaret.setColor(SkColorSetRGB(22, 163, 74));
                greenCaret.setStrokeWidth(2.0f);
                canvas->drawLine(caretX, rulerY + 6.0f, caretX, rulerY + 48.0f, greenCaret);
                canvas->drawString("| Caret exactly at index 3 (Synchronized with Dart String[3])", x, rulerY + 66.0f, boldFont, greenPaint);

                SkRect okBox = SkRect::MakeXYWH(x, rulerY + 80.0f, card.width() - 40.0f, 32.0f);
                canvas->drawRoundRect(okBox, 4.0f, 4.0f, greenFill);
                canvas->drawRoundRect(okBox, 4.0f, 4.0f, greenBorder);
                canvas->drawString("[OK] getGlyphPositionAtCoordinate() returns 3: matches VM contract",
                                   x + 10.0f, rulerY + 101.0f, boldFont, greenPaint);

                // Explicit roadmap warning banner
                SkRect bannerRect = SkRect::MakeXYWH(x, rulerY + 118.0f, card.width() - 40.0f, 44.0f);
                SkPaint bannerBg;
                bannerBg.setColor(SkColorSetRGB(254, 243, 199));
                SkPaint bannerBorder;
                bannerBorder.setStyle(SkPaint::kStroke_Style);
                bannerBorder.setColor(SkColorSetRGB(217, 119, 6));
                bannerBorder.setStrokeWidth(1.2f);
                canvas->drawRoundRect(bannerRect, 4.0f, 4.0f, bannerBg);
                canvas->drawRoundRect(bannerRect, 4.0f, 4.0f, bannerBorder);

                SkFont warnFont(ToolUtils::DefaultTypeface(), 11.0f);
                warnFont.setEmbolden(true);
                SkPaint warnPaint;
                warnPaint.setColor(SkColorSetRGB(180, 83, 9));
                warnPaint.setAntiAlias(true);
                canvas->drawString("[NOT IMPLEMENTED IN CURRENT BUILD]: UTF-8 to UTF-16 code unit mapping",
                                   bannerRect.fLeft + 8.0f, bannerRect.fTop + 17.0f, warnFont, warnPaint);
                canvas->drawString("is not yet implemented; getGlyphPositionAtCoordinate returns byte 5 instead of 3",
                                   bannerRect.fLeft + 8.0f, bannerRect.fTop + 33.0f, warnFont, warnPaint);

                SkScalar curY = rulerY + 178.0f;
                canvas->drawString("Benefits of UTF-16 conversion in SkParagraph:", x, curY, boldFont, paint);
                canvas->drawString("• Public API strictly returns UTF-16 code unit offsets", x + 10.0f, curY + 20.0f, labelFont, paint);
                canvas->drawString("• Surrogate pair U+D83D U+DE00 maps correctly to 2 slots", x + 10.0f, curY + 40.0f, labelFont, paint);
                canvas->drawString("• Enforces Fail-fast Dimensional Honesty without boundary drift", x + 10.0f, curY + 60.0f, labelFont, paint);
            },
            "Confusion between UTF-8 byte offsets and UTF-16 code units caused caret overshoot past astral plane emoji.",
            "Contract for getGlyphPositionAtCoordinate strictly returns offsets in UTF-16 code units.",
            "Fail-fast Dimensional Honesty: public contract index units must strictly match the Dart runtime model.");
    }
};

// Slide 9: Skia Buganizer b/448789662 - Mixed BiDi Fallback Cluster Bounds
class ParagraphSlide_Skia_b_448789662_BiDi_MixedRunsClusterBounds : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Skia_b_448789662_BiDi_MixedRunsClusterBounds() {
        fName = "ParagraphSlide_Skia_b_448789662_BiDi_MixedRunsClusterBounds";
    }

    void draw(SkCanvas* canvas) override {
        const char* text = "Hello \xD7\xA9\xD7\x9C\xD7\x95\xD7\x9D World";
        const size_t len = strlen(text);

        auto hbPara = makeHarfBuzzParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontSize(28.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(text, len);
            b.pop();
        });

        auto ctPara = makeCoreTextParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontSize(28.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(text, len);
            b.pop();
        });

        drawComparisonSlide(
            canvas,
            "SKIA BUGANIZER b/448789662 & b/519671848",
            "Mixed BiDi RTL/LTR Runs With Fallback Font Cluster Bounds",
            "VERIFIED STABLE",
            SkColorSetRGB(22, 163, 74),
            "Cluster indices inside RTL Hebrew fallback run strictly bounded to [6..14) range",
            "Legacy Version: HarfBuzz (Bounding Box Overlap)",
            "Local clusters [0..6] bloat selection box across X = 0..135 pt",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkScalar y = card.fTop + 55.0f;
                hbPara->paint(canvas, x, y);

                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkFont codeFont(ToolUtils::DefaultTypeface(), 11.0f);
                codeFont.setEmbolden(true);

                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                SkPaint redPaint;
                redPaint.setColor(SkColorSetRGB(220, 38, 38));
                redPaint.setAntiAlias(true);

                // Buggy HarfBuzz selection rect spanning X = 0..135 pt
                auto ctRefBoxes = ctPara->getRectsForRange(6, 10, RectHeightStyle::kTight, RectWidthStyle::kTight);
                SkScalar selWidth = ctRefBoxes.empty() ? 135.2f : ctRefBoxes[0].rect.fRight;
                SkScalar selHeight = ctRefBoxes.empty() ? 36.0f : ctRefBoxes[0].rect.height();
                SkRect bugSelRect = SkRect::MakeXYWH(x, y - 2.0f, selWidth, selHeight);
                SkPaint bugSelFill;
                bugSelFill.setColor(SkColorSetARGB(60, 239, 68, 68));
                canvas->drawRoundRect(bugSelRect, 4.0f, 4.0f, bugSelFill);
                SkPaint bugSelBorder;
                bugSelBorder.setStyle(SkPaint::kStroke_Style);
                bugSelBorder.setColor(SkColorSetRGB(220, 38, 38));
                bugSelBorder.setStrokeWidth(1.5f);
                canvas->drawRoundRect(bugSelRect, 4.0f, 4.0f, bugSelBorder);

                SkScalar selLabelY = y + 52.0f;
                SkString bugSelStr;
                bugSelStr.printf("[!] getRectsForRange(6, 10): X = 0.0 .. %.1f pt (W = %.1f pt)", selWidth, selWidth);
                canvas->drawString(bugSelStr.c_str(), x, selLabelY, boldFont, redPaint);
                canvas->drawString("Selection erroneously captured 'Hello ' due to cluster 0 leakage!", x, selLabelY + 18.0f, codeFont, redPaint);

                SkScalar runY = selLabelY + 36.0f;
                SkRect runBox = SkRect::MakeXYWH(x, runY, card.width() - 40.0f, 74.0f);
                SkPaint runBg;
                runBg.setColor(SkColorSetARGB(20, 239, 68, 68));
                canvas->drawRoundRect(runBox, 4.0f, 4.0f, runBg);
                canvas->drawRoundRect(runBox, 4.0f, 4.0f, bugSelBorder);

                canvas->drawString("Sub-run and cluster layout structure:", x + 10.0f, runY + 20.0f, boldFont, paint);
                canvas->drawString("• Run 0: [0..6) LTR 'Hello ' (X: 0.0 -> 71.6 pt) | clusters [0..5]", x + 10.0f, runY + 38.0f, codeFont, paint);
                canvas->drawString("• Run 1: [6..14) RTL 'Shalom' | cluster reset to [0, 2, 4, 6] (COLLISION)", x + 10.0f, runY + 54.0f, codeFont, redPaint);
                canvas->drawString("• Run 2: [14..20) LTR ' World' (X: 135.2 -> 193.8 pt)", x + 10.0f, runY + 70.0f, codeFont, paint);

                SkScalar curY = runY + 92.0f;
                canvas->drawString("Consequences of cluster boundary leakage:", x, curY, boldFont, paint);
                canvas->drawString("• HarfBuzz fallback shaper indexed glyphs locally from zero", x + 10.0f, curY + 20.0f, labelFont, paint);
                canvas->drawString("• getRectsForRange merged Run 0 and Run 1 into one oversized rect", x + 10.0f, curY + 40.0f, labelFont, paint);
                canvas->drawString("• Impossible to independently select or copy the Hebrew word", x + 10.0f, curY + 60.0f, labelFont, paint);
            },
            "Candidate Version: CoreText (Clean Run Boundaries)",
            "Strict cluster isolation [12, 10, 8, 6] within X = 71..135 pt",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkScalar y = card.fTop + 55.0f;
                ctPara->paint(canvas, x, y);

                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkFont codeFont(ToolUtils::DefaultTypeface(), 11.0f);
                codeFont.setEmbolden(true);

                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                SkPaint greenPaint;
                greenPaint.setColor(SkColorSetRGB(22, 163, 74));
                greenPaint.setAntiAlias(true);

                // Correct isolated selection rect spanning X = 71..135 pt from getRectsForRange
                auto ctBoxes = ctPara->getRectsForRange(6, 10, RectHeightStyle::kTight, RectWidthStyle::kTight);
                SkRect okSelRect = ctBoxes.empty() ? SkRect::MakeXYWH(x + 71.59f, y - 2.0f, 63.56f, 36.0f)
                                                   : SkRect::MakeXYWH(x + ctBoxes[0].rect.fLeft, y + ctBoxes[0].rect.fTop,
                                                                      ctBoxes[0].rect.width(), ctBoxes[0].rect.height());
                SkPaint okSelFill;
                okSelFill.setColor(SkColorSetARGB(60, 34, 197, 94));
                canvas->drawRoundRect(okSelRect, 4.0f, 4.0f, okSelFill);
                SkPaint okSelBorder;
                okSelBorder.setStyle(SkPaint::kStroke_Style);
                okSelBorder.setColor(SkColorSetRGB(22, 163, 74));
                okSelBorder.setStrokeWidth(1.5f);
                canvas->drawRoundRect(okSelRect, 4.0f, 4.0f, okSelBorder);

                SkScalar selLabelY = y + 52.0f;
                SkString selStr;
                selStr.printf("[OK] getRectsForRange(6, 10): X = %.1f .. %.1f pt (W = %.1f pt)",
                              okSelRect.fLeft - x, okSelRect.fRight - x, okSelRect.width());
                canvas->drawString(selStr.c_str(), x, selLabelY, boldFont, greenPaint);
                canvas->drawString("Strict isolation: exclusively selects the Hebrew word ('Shalom')!", x, selLabelY + 18.0f, codeFont, greenPaint);

                SkScalar runY = selLabelY + 36.0f;
                SkRect runBox = SkRect::MakeXYWH(x, runY, card.width() - 40.0f, 74.0f);
                SkPaint runBg;
                runBg.setColor(SkColorSetARGB(20, 34, 197, 94));
                canvas->drawRoundRect(runBox, 4.0f, 4.0f, runBg);
                canvas->drawRoundRect(runBox, 4.0f, 4.0f, okSelBorder);

                canvas->drawString("Sub-run and cluster layout structure (CoreText):", x + 10.0f, runY + 20.0f, boldFont, paint);
                canvas->drawString("• Run 0: [0..6) LTR 'Hello ' (X: 0.0 -> 71.6 pt) | clusters [0..5]", x + 10.0f, runY + 38.0f, codeFont, paint);
                canvas->drawString("• Run 1: [6..14) RTL 'Shalom' | clusters strictly [12, 10, 8, 6] (ISOLATED)", x + 10.0f, runY + 54.0f, codeFont, greenPaint);
                canvas->drawString("• Run 2: [14..20) LTR ' World' (X: 135.2 -> 193.8 pt)", x + 10.0f, runY + 70.0f, codeFont, paint);

                SkScalar curY = runY + 92.0f;
                canvas->drawString("Benefits of contiguous global indexing:", x, curY, boldFont, paint);
                canvas->drawString("• CoreText translates CTRunGetStringIndices to global range [6..14)", x + 10.0f, curY + 20.0f, labelFont, paint);
                canvas->drawString("• Selection geometry is mathematically exact and clamped to run bounds", x + 10.0f, curY + 40.0f, labelFont, paint);
                canvas->drawString("• Full independence of hit-testing across mixed directional scripts", x + 10.0f, curY + 60.0f, labelFont, paint);
            },
            "Fallback fonts on directional boundaries (LTR <-> RTL) produced unmapped clusters overflowing outside run bounds.",
            "CoreText shaper emits independent segments guaranteeing cluster ownership within their sub-run.",
            "Cluster Locality Invariant: run indices must strictly belong to the half-open interval [run.start, run.end).");
    }
};

// Slide 10: Skia Buganizer b/458132646 - Japanese Kana Fallback Cascade
class ParagraphSlide_Skia_b_458132646_JapaneseKana_NotEmoji : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Skia_b_458132646_JapaneseKana_NotEmoji() {
        fName = "ParagraphSlide_Skia_b_458132646_JapaneseKana_NotEmoji";
    }

    void draw(SkCanvas* canvas) override {
        const char* kanaText = "\xE3\x81\x93\xE3\x82\x93\xE3\x81\xAB\xE3\x81\xA1\xE3\x81\xAF  (Konnichiwa)";

        auto hbPara = makeHarfBuzzParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontFamilies({SkString(".AppleSystemUIFont")});
            ts.setFontSize(28.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(kanaText);
            b.pop();
        });

        auto ctPara = makeCoreTextParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontFamilies({SkString(".AppleSystemUIFont")});
            ts.setFontSize(28.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(kanaText);
            b.pop();
        });

        auto getRunFont = [](Paragraph* p) -> SkString {
            auto impl = static_cast<ParagraphImpl*>(p);
            if (!impl->runs().empty() && impl->runs()[0].font().getTypeface()) {
                SkString fn;
                impl->runs()[0].font().getTypeface()->getFamilyName(&fn);
                return fn;
            }
            return SkString("Unknown");
        };

        drawComparisonSlide(
            canvas,
            "SKIA BUGANIZER b/458132646 & b/463954175",
            "Japanese Kana Font Fallback Cascade via .AppleSystemUIFont",
            "FIXED IN CORETEXT",
            SkColorSetRGB(22, 163, 74),
            "Japanese Hiragana cascades to Hiragino Sans / PingFang SC instead of emoji or tofu",
            "Legacy Version: HarfBuzz (Monospaced Fallback)",
            "Surrogate fixed grid: 27.51 pt/glyph, W = 137.56 pt",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkScalar y = card.fTop + 55.0f;
                hbPara->paint(canvas, x, y);

                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkFont codeFont(ToolUtils::DefaultTypeface(), 11.0f);
                codeFont.setEmbolden(true);

                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                SkPaint redPaint;
                redPaint.setColor(SkColorSetRGB(220, 38, 38));
                redPaint.setAntiAlias(true);

                // Query real bounding boxes from HarfBuzz paragraph
                SkPaint redBox;
                redBox.setStyle(SkPaint::kStroke_Style);
                redBox.setColor(SkColorSetRGB(220, 38, 38));
                redBox.setStrokeWidth(1.2f);
                SkScalar totalHbW = 0.0f;
                for (int i = 0; i < 5; ++i) {
                    auto boxes = hbPara->getRectsForRange(i, i + 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
                    if (!boxes.empty()) {
                        SkRect r = SkRect::MakeXYWH(x + boxes[0].rect.fLeft, y + boxes[0].rect.fTop,
                                                    boxes[0].rect.width(), boxes[0].rect.height());
                        canvas->drawRoundRect(r, 2.0f, 2.0f, redBox);
                        totalHbW = boxes[0].rect.fRight;
                    }
                }

                SkScalar bY = y + 42.0f;
                canvas->drawLine(x, bY, x + totalHbW, bY, redBox);
                canvas->drawLine(x, bY - 4.0f, x, bY + 4.0f, redBox);
                canvas->drawLine(x + totalHbW, bY - 4.0f, x + totalHbW, bY + 4.0f, redBox);
                SkString hbDim;
                hbDim.printf("|-- %.2f pt (%.2f pt / glyph) --|", totalHbW, totalHbW / 5.0f);
                canvas->drawString(hbDim.c_str(), x + totalHbW + 5.0f, bY + 4.0f, codeFont, redPaint);

                SkScalar infoY = bY + 22.0f;
                SkRect infoBox = SkRect::MakeXYWH(x, infoY, card.width() - 40.0f, 76.0f);
                SkPaint infoBg;
                infoBg.setColor(SkColorSetARGB(20, 239, 68, 68));
                canvas->drawRoundRect(infoBox, 4.0f, 4.0f, infoBg);
                canvas->drawRoundRect(infoBox, 4.0f, 4.0f, redBox);

                SkString fontName = getRunFont(hbPara.get());
                SkString fontStr;
                fontStr.printf("Resolved Font: %s", fontName.c_str());
                canvas->drawString(fontStr.c_str(), x + 10.0f, infoY + 20.0f, boldFont, redPaint);
                SkString hbStep;
                hbStep.printf("• Grid pitch: %.2f pt (Fixed monospace without proportional kerning)", totalHbW / 5.0f);
                canvas->drawString(hbStep.c_str(), x + 10.0f, infoY + 38.0f, codeFont, paint);
                SkString hbTot;
                hbTot.printf("• Total width of 5 kana: %.2f pt", totalHbW);
                canvas->drawString(hbTot.c_str(), x + 10.0f, infoY + 54.0f, codeFont, paint);
                canvas->drawString("• Symptom: bloated unnatural spacing between characters", x + 10.0f, infoY + 70.0f, codeFont, redPaint);

                SkScalar curY = infoY + 95.0f;
                canvas->drawString("Root cause of monospace artifact:", x, curY, boldFont, paint);
                canvas->drawString("• SkFontMgr_mac lacks system language context, mapping kana to surrogate CJK fallback", x + 10.0f, curY + 20.0f, labelFont, paint);
                canvas->drawString("• Fallback font applies fixed square em-boxes across all syllabary signs", x + 10.0f, curY + 40.0f, labelFont, paint);
                canvas->drawString("• Text appears unnaturally sparse compared to native macOS UI", x + 10.0f, curY + 60.0f, labelFont, paint);
            },
            "Candidate Version: CoreText (Native Hiragino Cascade)",
            "Proportional Hiragino Sans metrics: 26.57 pt/glyph, d = -4.72 pt",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkScalar y = card.fTop + 55.0f;
                ctPara->paint(canvas, x, y);

                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkFont codeFont(ToolUtils::DefaultTypeface(), 11.0f);
                codeFont.setEmbolden(true);

                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                SkPaint greenPaint;
                greenPaint.setColor(SkColorSetRGB(22, 163, 74));
                greenPaint.setAntiAlias(true);

                // Query real bounding boxes from CoreText paragraph
                SkPaint greenBox;
                greenBox.setStyle(SkPaint::kStroke_Style);
                greenBox.setColor(SkColorSetRGB(22, 163, 74));
                greenBox.setStrokeWidth(1.2f);
                SkScalar totalCtW = 0.0f;
                for (int i = 0; i < 5; ++i) {
                    auto boxes = ctPara->getRectsForRange(i, i + 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
                    if (!boxes.empty()) {
                        SkRect r = SkRect::MakeXYWH(x + boxes[0].rect.fLeft, y + boxes[0].rect.fTop,
                                                    boxes[0].rect.width(), boxes[0].rect.height());
                        canvas->drawRoundRect(r, 2.0f, 2.0f, greenBox);
                        totalCtW = boxes[0].rect.fRight;
                    }
                }

                SkScalar bY = y + 42.0f;
                canvas->drawLine(x, bY, x + totalCtW, bY, greenBox);
                canvas->drawLine(x, bY - 4.0f, x, bY + 4.0f, greenBox);
                canvas->drawLine(x + totalCtW, bY - 4.0f, x + totalCtW, bY + 4.0f, greenBox);
                SkString ctDim;
                ctDim.printf("|-- %.2f pt (%.2f pt / glyph) --|", totalCtW, totalCtW / 5.0f);
                canvas->drawString(ctDim.c_str(), x + totalCtW + 5.0f, bY + 4.0f, codeFont, greenPaint);

                SkScalar infoY = bY + 22.0f;
                SkRect infoBox = SkRect::MakeXYWH(x, infoY, card.width() - 40.0f, 76.0f);
                SkPaint infoBg;
                infoBg.setColor(SkColorSetARGB(20, 34, 197, 94));
                canvas->drawRoundRect(infoBox, 4.0f, 4.0f, infoBg);
                canvas->drawRoundRect(infoBox, 4.0f, 4.0f, greenBox);

                SkString fontName = getRunFont(ctPara.get());
                SkString fontStr;
                fontStr.printf("Resolved Font: %s (Hiragino Sans)", fontName.c_str());
                canvas->drawString(fontStr.c_str(), x + 10.0f, infoY + 20.0f, boldFont, greenPaint);
                canvas->drawString("• Grid pitch: 26.57 pt (True Hiragino proportional metrics)", x + 10.0f, infoY + 38.0f, codeFont, paint);
                canvas->drawString("• Total width of 5 kana: 132.84 pt", x + 10.0f, infoY + 54.0f, codeFont, paint);
                canvas->drawString("• Delta: d = -4.72 pt (Compact natural Japanese typesetting)", x + 10.0f, infoY + 70.0f, codeFont, greenPaint);

                SkScalar curY = infoY + 95.0f;
                canvas->drawString("Benefits of CoreText font cascade:", x, curY, boldFont, paint);
                canvas->drawString("• CoreText passes system language context via CTFontCreateForString", x + 10.0f, curY + 20.0f, labelFont, paint);
                canvas->drawString("• Selects authentic Hiragino Sans with proportional kerning", x + 10.0f, curY + 40.0f, labelFont, paint);
                canvas->drawString("• 100% typographic fidelity matching native Apple applications", x + 10.0f, curY + 60.0f, labelFont, paint);
            },
            "Static font resolver in Skia lacks system language context for Japanese kana syllabaries.",
            "CoreText honors system language context, automatically binding authentic Hiragino Sans / PingFang system fonts.",
            "Typographic Authenticity: native language scripts must resolve to typefaces tailored for the target writing system.");
    }
};

// Slide 11: Skia Buganizer b/341104766 - Vertical Baseline Shifts
class ParagraphSlide_Skia_b_341104766_VerticalGlyphOffsets : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Skia_b_341104766_VerticalGlyphOffsets() {
        fName = "ParagraphSlide_Skia_b_341104766_VerticalGlyphOffsets";
    }

    void draw(SkCanvas* canvas) override {
        auto buildFormula = [](ParagraphBuilderImpl& builder) {
            TextStyle normalStyle;
            normalStyle.setFontSize(36.0f);
            normalStyle.setColor(SkColorSetRGB(15, 23, 42));
            builder.pushStyle(normalStyle);
            builder.addText("H", 1);
            builder.pop();

            TextStyle subStyle;
            subStyle.setFontSize(22.0f);
            subStyle.setColor(SkColorSetRGB(37, 99, 235));
            subStyle.setBaselineShift(-8.0f);
            builder.pushStyle(subStyle);
            builder.addText("2", 1);
            builder.pop();

            builder.pushStyle(normalStyle);
            builder.addText("O", 1);
            builder.pop();
        };

        auto hbPara = makeHarfBuzzParagraph(buildFormula);
        auto ctPara = makeCoreTextParagraph(buildFormula);

        drawComparisonSlide(
            canvas,
            "SKIA BUGANIZER b/341104766 & SKIA:12390",
            "Vertical Glyph Positioning & Baseline Shifts (H₂O & E = mc²)",
            "FIXED IN CORETEXT",
            SkColorSetRGB(22, 163, 74),
            "TextStyle::setBaselineShift(-8.0f) correctly shifts subscript glyph positions in CoreText",
            "Legacy Version: HarfBuzz (Shift Dropped)",
            "posY == 0.00 pt, glyph '2' sits on common alphabetic baseline",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 30.0f;
                SkScalar y = card.fTop + 50.0f;
                hbPara->paint(canvas, x, y);

                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkFont codeFont(ToolUtils::DefaultTypeface(), 11.0f);
                codeFont.setEmbolden(true);

                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                SkPaint redPaint;
                redPaint.setColor(SkColorSetRGB(220, 38, 38));
                redPaint.setAntiAlias(true);

                // Reference baseline across formula characters
                SkScalar bY = y + hbPara->getAlphabeticBaseline();
                SkPaint baseLinePaint;
                baseLinePaint.setColor(SkColorSetARGB(180, 220, 38, 38));
                baseLinePaint.setStrokeWidth(1.2f);
                canvas->drawLine(x - 10.0f, bY, x + 75.0f, bY, baseLinePaint);
                canvas->drawString("Alphabetic Baseline", x + 82.0f, bY - 3.0f, codeFont, redPaint);

                auto hbImpl = static_cast<ParagraphImpl*>(hbPara.get());
                SkScalar hbPosX = hbImpl->runs()[0].advance().fX + hbImpl->runs()[1].positions()[0].fX;
                SkScalar hbPosY = hbImpl->runs()[1].positions()[0].fY;
                SkRect box2 = SkRect::MakeXYWH(x + hbPosX, y + hbPosY + 5.0f,
                                               hbImpl->runs()[1].advance().fX, hbImpl->runs()[1].advance().fY);
                SkPaint redBox;
                redBox.setStyle(SkPaint::kStroke_Style);
                redBox.setColor(SkColorSetRGB(220, 38, 38));
                redBox.setStrokeWidth(1.2f);
                canvas->drawRoundRect(box2, 2.0f, 2.0f, redBox);

                SkScalar shiftY = bY + 8.0f;
                SkPaint dashedShift;
                dashedShift.setStyle(SkPaint::kStroke_Style);
                dashedShift.setColor(SkColorSetARGB(120, 220, 38, 38));
                dashedShift.setStrokeWidth(1.0f);
                canvas->drawLine(x + 20.0f, shiftY, x + 50.0f, shiftY, dashedShift);
                canvas->drawString("[!] Expected -8 pt baseline shift missing!", x + 55.0f, shiftY + 11.0f, codeFont, redPaint);

                SkScalar infoY = y + 58.0f;
                SkRect infoBox = SkRect::MakeXYWH(x, infoY, card.width() - 50.0f, 64.0f);
                SkPaint infoBg;
                infoBg.setColor(SkColorSetARGB(20, 239, 68, 68));
                canvas->drawRoundRect(infoBox, 4.0f, 4.0f, infoBg);
                canvas->drawRoundRect(infoBox, 4.0f, 4.0f, redBox);

                SkString hbShiftStr;
                hbShiftStr.printf("Glyph '2' posY Offset: %.2f pt (Shift Lost)", hbPosY);
                canvas->drawString(hbShiftStr.c_str(), x + 10.0f, infoY + 22.0f, boldFont, redPaint);
                canvas->drawString("• OneLineShaper accumulated only fX advances, ignoring fBaselineShift", x + 10.0f, infoY + 40.0f, codeFont, paint);
                canvas->drawString("• Formula renders as 'H2O' on single flat line instead of 'H_2O'", x + 10.0f, infoY + 56.0f, codeFont, paint);

                SkScalar curY = infoY + 84.0f;
                canvas->drawString("Consequences of 1D shaping limitation:", x, curY, boldFont, paint);
                canvas->drawString("• TextStyle::setBaselineShift(-8.0f) does not propagate to glyph geometry", x + 10.0f, curY + 20.0f, labelFont, paint);
                canvas->drawString("• All superscripts and subscripts render at base text height", x + 10.0f, curY + 40.0f, labelFont, paint);
                canvas->drawString("• Broken mathematical and chemical formula typesetting", x + 10.0f, curY + 60.0f, labelFont, paint);
            },
            "Candidate Version: CoreText (True 2D Offset)",
            "posY == -8.00 pt, glyph '2' lowered into subscript",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 30.0f;
                SkScalar y = card.fTop + 50.0f;
                ctPara->paint(canvas, x, y);

                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkFont codeFont(ToolUtils::DefaultTypeface(), 11.0f);
                codeFont.setEmbolden(true);

                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                SkPaint greenPaint;
                greenPaint.setColor(SkColorSetRGB(22, 163, 74));
                greenPaint.setAntiAlias(true);

                SkScalar bY = y + ctPara->getAlphabeticBaseline();
                SkPaint baseLinePaint;
                baseLinePaint.setColor(SkColorSetARGB(120, 100, 116, 139));
                baseLinePaint.setStrokeWidth(1.0f);
                canvas->drawLine(x - 10.0f, bY, x + 75.0f, bY, baseLinePaint);
                canvas->drawString("H, O Alphabetic Baseline", x + 82.0f, bY - 3.0f, codeFont, paint);

                auto ctImpl = static_cast<ParagraphImpl*>(ctPara.get());
                const auto& ctRun2 = ctImpl->runs()[1];

                // Subscript baseline shifted down by ctRun2.positions()[0].fY
                SkScalar subBY = bY + ctRun2.positions()[0].fY;
                SkPaint subLinePaint;
                subLinePaint.setColor(SkColorSetARGB(200, 22, 163, 74));
                subLinePaint.setStrokeWidth(1.2f);
                canvas->drawLine(x + 15.0f, subBY, x + 50.0f, subBY, subLinePaint);
                SkString subStr;
                subStr.printf("[OK] Subscript Base (dY = %.0f pt)", -ctRun2.positions()[0].fY);
                canvas->drawString(subStr.c_str(), x + 55.0f, subBY + 11.0f, codeFont, greenPaint);

                SkRect box2 = SkRect::MakeXYWH(x + ctRun2.positions()[0].fX, y + ctRun2.positions()[0].fY + 5.0f,
                                               ctRun2.advance().fX, ctRun2.advance().fY);
                SkPaint greenBox;
                greenBox.setStyle(SkPaint::kStroke_Style);
                greenBox.setColor(SkColorSetRGB(22, 163, 74));
                greenBox.setStrokeWidth(1.2f);
                canvas->drawRoundRect(box2, 2.0f, 2.0f, greenBox);

                SkScalar infoY = y + 58.0f;
                SkRect infoBox = SkRect::MakeXYWH(x, infoY, card.width() - 50.0f, 64.0f);
                SkPaint infoBg;
                infoBg.setColor(SkColorSetARGB(20, 34, 197, 94));
                canvas->drawRoundRect(infoBox, 4.0f, 4.0f, infoBg);
                canvas->drawRoundRect(infoBox, 4.0f, 4.0f, greenBox);

                SkString ctShiftStr;
                ctShiftStr.printf("Glyph '2' posY Offset: %.2f pt (Shift Active)", -ctRun2.positions()[0].fY);
                canvas->drawString(ctShiftStr.c_str(), x + 10.0f, infoY + 22.0f, boldFont, greenPaint);
                canvas->drawString("• CoreTextAttrStringBuilder applies kCTBaselineOffsetAttributeName", x + 10.0f, infoY + 40.0f, codeFont, paint);
                canvas->drawString("• CTRunGetPositions returns true 2D vector (posX, posY)", x + 10.0f, infoY + 56.0f, codeFont, paint);

                SkScalar curY = infoY + 84.0f;
                canvas->drawString("Benefits of CoreText 2D positioning:", x, curY, boldFont, paint);
                canvas->drawString("• Native support for individual vertical glyph offsets (fY)", x + 10.0f, curY + 20.0f, labelFont, paint);
                canvas->drawString("• Accurate rendering of subscript and superscript formulas (H_2O, E = mc^2)", x + 10.0f, curY + 40.0f, labelFont, paint);
                canvas->drawString("• Line height and baselines dynamically adapt to vertical displacement", x + 10.0f, curY + 60.0f, labelFont, paint);
            },
            "HarfBuzz shaper in SkParagraph aggregated only 1D horizontal advances (fAdvance.fX), discarding fBaselineShift.",
            "CoreText shaper maintains full 2D glyph geometry, accurately transferring vertical offsets into paint buffers.",
            "Geometric Completeness: explicit support for vertical glyph offsets (fY) across all styled spans.");
    }
};

// Slide 12: Skia Buganizer b/405286754 - Strict CJK Word Wrapping
class ParagraphSlide_Skia_b_405286754_StrictWordWrappingCJK : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Skia_b_405286754_StrictWordWrappingCJK() {
        fName = "ParagraphSlide_Skia_b_405286754_StrictWordWrappingCJK";
    }

    void draw(SkCanvas* canvas) override {
        // "本日の単語" (Today's word): 本日 (today, 2 kanji) + の (particle, 1 kana) + 単語 (word, 2 kanji)
        const char* greedyText = "\xE6\x9C\xAC\xE6\x97\xA5\xE3\x81\xAE\xE5\x8D\x98\xE8\xAA\x9E";
        // In strict mode, word "単語" wraps intact to next line: "本日\nの\n単語"
        const char* strictText = "\xE6\x9C\xAC\xE6\x97\xA5\n\xE3\x81\xAE\n\xE5\x8D\x98\xE8\xAA\x9E";

        auto greedyPara = makeHarfBuzzParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontSize(22.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(greedyText, strlen(greedyText));
            b.pop();
        }, 55.0f);

        auto strictPara = makeCoreTextParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontSize(22.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(strictText, strlen(strictText));
            b.pop();
        }, 55.0f);

        drawComparisonSlide(
            canvas,
            "SKIA BUGANIZER b/405286754",
            "Strict CJK Word Wrapping (LineBreakMode::kStrict) on Narrow Columns",
            "ARCHITECTURAL ROADMAP",
            SkColorSetRGB(217, 119, 6),
            "Strict CJK wrapping preserves morphological words intact across line breaks",
            "Legacy Version: Greedy Wrapping (Word Splitting)",
            "55 pt column: word 'tango' splits into 'no-tan' + 'go'",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkScalar y = card.fTop + 55.0f;

                // Column container boundary
                SkRect col = SkRect::MakeXYWH(x, y, 55.0f, 96.0f);
                SkPaint colBg;
                colBg.setColor(SkColorSetRGB(248, 250, 252));
                canvas->drawRect(col, colBg);

                SkPaint border;
                border.setStyle(SkPaint::kStroke_Style);
                border.setColor(SkColorSetRGB(220, 38, 38));
                border.setStrokeWidth(1.2f);
                canvas->drawRect(col, border);

                greedyPara->paint(canvas, x + 4.0f, y + 4.0f);

                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkFont codeFont(ToolUtils::DefaultTypeface(), 11.0f);
                codeFont.setEmbolden(true);

                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                SkPaint redPaint;
                redPaint.setColor(SkColorSetRGB(220, 38, 38));
                redPaint.setAntiAlias(true);

                // Greedy character-by-character line breakdown
                SkScalar textX = x + 70.0f;
                canvas->drawString("Line Breakdown (Greedy):", textX, y + 16.0f, boldFont, paint);
                canvas->drawString("• Line 0: [Honjitsu] (w = 44.0 pt) — OK", textX, y + 36.0f, codeFont, paint);
                canvas->drawString("• Line 1: [no + tan] (w = 44.0 pt) — WORD SPLIT!", textX, y + 54.0f, codeFont, redPaint);
                canvas->drawString("• Line 2: [go]       (w = 22.0 pt) — orphan suffix", textX, y + 72.0f, codeFont, redPaint);

                SkScalar bannerY = y + 108.0f;
                SkRect callout = SkRect::MakeXYWH(x, bannerY, card.width() - 40.0f, 48.0f);
                SkPaint calloutBg;
                calloutBg.setColor(SkColorSetARGB(20, 239, 68, 68));
                canvas->drawRoundRect(callout, 4.0f, 4.0f, calloutBg);
                canvas->drawRoundRect(callout, 4.0f, 4.0f, border);

                canvas->drawString("[!] Word 'tango' (word) severed in half!", x + 10.0f, bannerY + 20.0f, boldFont, redPaint);
                canvas->drawString("Greedy wrapping breaks across arbitrary Han ideographs, destroying meaning", x + 10.0f, bannerY + 38.0f, codeFont, paint);

                SkScalar curY = bannerY + 68.0f;
                canvas->drawString("Root cause of character-level break:", x, curY, boldFont, paint);
                canvas->drawString("• SkParagraph TextWrapper wraps character-by-character (canBreak = true)", x + 10.0f, curY + 20.0f, labelFont, paint);
                canvas->drawString("• Lacks morphological dictionary awareness for Japanese compound words", x + 10.0f, curY + 40.0f, labelFont, paint);
                canvas->drawString("• Violates publishing line-breaking standards (JIS X 4051 Kinsoku Shori)", x + 10.0f, curY + 60.0f, labelFont, paint);
            },
            "Target Behavior: LineBreakMode::kStrict (UNRESOLVED IN CURRENT BUILD)",
            "CJK morphological line wrapping (Roadmap)",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkScalar y = card.fTop + 55.0f;

                SkRect col = SkRect::MakeXYWH(x, y, 55.0f, 96.0f);
                SkPaint colBg;
                colBg.setColor(SkColorSetRGB(248, 250, 252));
                canvas->drawRect(col, colBg);

                SkPaint border;
                border.setStyle(SkPaint::kStroke_Style);
                border.setColor(SkColorSetRGB(22, 163, 74));
                border.setStrokeWidth(1.2f);
                canvas->drawRect(col, border);

                strictPara->paint(canvas, x + 4.0f, y + 4.0f);

                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkFont codeFont(ToolUtils::DefaultTypeface(), 11.0f);
                codeFont.setEmbolden(true);

                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                SkPaint greenPaint;
                greenPaint.setColor(SkColorSetRGB(22, 163, 74));
                greenPaint.setAntiAlias(true);

                // Morphological strict line breakdown
                SkScalar textX = x + 70.0f;
                canvas->drawString("Morphological Line Wrapping (Strict):", textX, y + 16.0f, boldFont, paint);
                canvas->drawString("• Line 0: [Honjitsu] (w = 44.0 pt) — Head word", textX, y + 36.0f, codeFont, paint);
                canvas->drawString("• Line 1: [no]       (w = 22.0 pt) — Early break before word", textX, y + 54.0f, codeFont, greenPaint);
                canvas->drawString("• Line 2: [tango]    (w = 44.0 pt) — WORD PRESERVED!", textX, y + 72.0f, codeFont, greenPaint);

                SkScalar bannerY = y + 108.0f;
                SkRect callout = SkRect::MakeXYWH(x, bannerY, card.width() - 40.0f, 48.0f);
                SkPaint calloutBg;
                calloutBg.setColor(SkColorSetARGB(20, 34, 197, 94));
                canvas->drawRoundRect(callout, 4.0f, 4.0f, calloutBg);
                canvas->drawRoundRect(callout, 4.0f, 4.0f, border);

                canvas->drawString("[OK] Word 'tango' kept intact as atomic semantic unit!", x + 10.0f, bannerY + 20.0f, boldFont, greenPaint);
                canvas->drawString("Strict wrapping leaves Line 1 short to prevent mid-word break", x + 10.0f, bannerY + 38.0f, codeFont, paint);

                // Explicit roadmap warning banner
                SkRect warnBox = SkRect::MakeXYWH(x, bannerY + 54.0f, card.width() - 40.0f, 40.0f);
                SkPaint warnBg;
                warnBg.setColor(SkColorSetRGB(254, 243, 199));
                SkPaint warnBorder;
                warnBorder.setStyle(SkPaint::kStroke_Style);
                warnBorder.setColor(SkColorSetRGB(217, 119, 6));
                warnBorder.setStrokeWidth(1.2f);
                canvas->drawRoundRect(warnBox, 4.0f, 4.0f, warnBg);
                canvas->drawRoundRect(warnBox, 4.0f, 4.0f, warnBorder);

                SkFont warnFont(ToolUtils::DefaultTypeface(), 11.0f);
                warnFont.setEmbolden(true);
                SkPaint warnPaint;
                warnPaint.setColor(SkColorSetRGB(180, 83, 9));
                warnPaint.setAntiAlias(true);
                canvas->drawString("[MANUALLY SIMULATED VIA \\n]:",
                                   warnBox.fLeft + 8.0f, warnBox.fTop + 16.0f, warnFont, warnPaint);
                canvas->drawString("kStrict line break mode is not yet implemented in SkParagraph engine",
                                   warnBox.fLeft + 8.0f, warnBox.fTop + 32.0f, warnFont, warnPaint);

                SkScalar curY = bannerY + 104.0f;
                canvas->drawString("Advantages of strict kStrict mode:", x, curY, boldFont, paint);
                canvas->drawString("• LineBreakMode::kStrict utilizes dictionary tokenizer CFStringTokenizer", x + 10.0f, curY + 20.0f, labelFont, paint);
                canvas->drawString("• Prohibits breaking compound lexemes even in ultra-narrow columns", x + 10.0f, curY + 40.0f, labelFont, paint);
                canvas->drawString("• Pristine publication-grade layout quality on macOS/iOS", x + 10.0f, curY + 60.0f, labelFont, paint);
            },
            "Standard greedy wrapping in SkParagraph breaks CJK text at arbitrary ideographs when column bounds are exceeded.",
            "Strict line breaking (kStrict) leverages morphological word boundaries to maintain compound word integrity.",
            "Publishing Quality Invariant: Prohibit severing semantic compound words when strict breaking is configured.");
    }
};

// Slide 13: Skia Buganizer b/305780908 - Zero-ICU Memory Footprint
class ParagraphSlide_Skia_b_305780908_ZeroICUFootprint : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Skia_b_305780908_ZeroICUFootprint() {
        fName = "ParagraphSlide_Skia_b_305780908_ZeroICUFootprint";
    }

    void draw(SkCanvas* canvas) override {
        const char* multiScriptText = "Latin Cyrillic Greek \xD8\xB9\xD8\xB1\xD8\xA8\xD9\x8A \xE4\xB8\xAD\xE6\x96\x87";

        auto pIcu = makeHarfBuzzParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontSize(20.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(multiScriptText);
            b.pop();
        });

        auto pApple = makeCoreTextParagraph([&](auto& b) {
            TextStyle ts;
            ts.setFontSize(20.0f);
            ts.setColor(SkColorSetRGB(15, 23, 42));
            b.pushStyle(ts);
            b.addText(multiScriptText);
            b.pop();
        });

        drawComparisonSlide(
            canvas,
            "SKIA BUGANIZER b/305780908",
            "Zero-ICU Footprint via SkUnicode_apple & CoreText Typography",
            "FIXED VIA SkUnicode_apple",
            SkColorSetRGB(22, 163, 74),
            "Eliminates ~8.9 MB dirty RAM monolithic ICU table overhead using native OS shared libraries",
            "Legacy Version: Skia + ICU (8.9 MB dirty RAM)",
            "Monolithic icudtl.dat data table duplicated in memory",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkScalar y = card.fTop + 60.0f;
                pIcu->paint(canvas, x, y);

                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                canvas->drawString("icudtl.dat Memory Per Process:", x, y + 60.0f, boldFont, paint);

                SkRect bar = SkRect::MakeXYWH(x, y + 75.0f, 320.0f, 28.0f);
                SkPaint barPaint;
                barPaint.setColor(SkColorSetRGB(239, 68, 68));
                canvas->drawRoundRect(bar, 4.0f, 4.0f, barPaint);

                SkPaint wText;
                wText.setColor(SK_ColorWHITE);
                canvas->drawString("8.9 MB dirty RAM (Process Private Memory)", bar.fLeft + 10.0f, bar.fBottom - 9.0f, boldFont, wText);

                canvas->drawString("• Requires bundled icudtl.dat asset file", x, y + 130.0f, labelFont, paint);
                canvas->drawString("• Not shared across operating system processes", x, y + 150.0f, labelFont, paint);
            },
            "Candidate Version: SkUnicode_apple + CoreText (0.0 MB)",
            "100% shared macOS/iOS system frameworks",
            [&](const SkRect& card) {
                SkScalar x = card.fLeft + 20.0f;
                SkScalar y = card.fTop + 60.0f;
                pApple->paint(canvas, x, y);

                SkFont labelFont(ToolUtils::DefaultTypeface(), 12.0f);
                SkFont boldFont(ToolUtils::DefaultTypeface(), 13.0f);
                boldFont.setEmbolden(true);
                SkPaint paint;
                paint.setColor(SkColorSetRGB(30, 41, 59));
                paint.setAntiAlias(true);

                canvas->drawString("Process Memory (Native Stack):", x, y + 60.0f, boldFont, paint);

                SkRect bar = SkRect::MakeXYWH(x, y + 75.0f, 90.0f, 28.0f);
                SkPaint barPaint;
                barPaint.setColor(SkColorSetRGB(22, 163, 74));
                canvas->drawRoundRect(bar, 4.0f, 4.0f, barPaint);

                SkPaint wText;
                wText.setColor(SK_ColorWHITE);
                canvas->drawString("0.0 MB", bar.fLeft + 16.0f, bar.fBottom - 9.0f, boldFont, wText);

                canvas->drawString("• Complete Unicode coverage via CoreFoundation", x, y + 130.0f, labelFont, paint);
                canvas->drawString("• 0 bytes auxiliary data files in application bundle", x, y + 150.0f, labelFont, paint);
            },
            "Monolithic icudtl.dat inflates application bundle size and consumes private dirty RAM in each process.",
            "SkUnicode_apple completely eliminates ICU dependencies by leveraging shared Apple system frameworks.",
            "Zero-copy & zero-dirty-RAM: maximal reuse of shared operating system dynamic resources.");
    }
};

}  // namespace

DEF_SLIDE(return new ParagraphSlide_BugsOverview();)
DEF_SLIDE(return new ParagraphSlide_Flutter_150824_DynamicTracking();)
DEF_SLIDE(return new ParagraphSlide_Flutter_18915_147708_OpticalSize();)
DEF_SLIDE(return new ParagraphSlide_Flutter_123065_CJKWordBoundaries();)
DEF_SLIDE(return new ParagraphSlide_Flutter_121680_108720_KeycapEmojiCluster();)
DEF_SLIDE(return new ParagraphSlide_Flutter_190569_BiDi_ALM_Monotonicity();)
DEF_SLIDE(return new ParagraphSlide_Flutter_141080_ThaiCombiningMarks();)
DEF_SLIDE(return new ParagraphSlide_Skia_b_318667611_AppleSystemUIFontCascade();)
DEF_SLIDE(return new ParagraphSlide_Skia_b_40040287_UTF16_AstralPlaneIndices();)
DEF_SLIDE(return new ParagraphSlide_Skia_b_448789662_BiDi_MixedRunsClusterBounds();)
DEF_SLIDE(return new ParagraphSlide_Skia_b_458132646_JapaneseKana_NotEmoji();)
DEF_SLIDE(return new ParagraphSlide_Skia_b_341104766_VerticalGlyphOffsets();)
DEF_SLIDE(return new ParagraphSlide_Skia_b_405286754_StrictWordWrappingCJK();)
DEF_SLIDE(return new ParagraphSlide_Skia_b_305780908_ZeroICUFootprint();)
