// Copyright 2026 Google LLC
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkDashPathEffect.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Metrics.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skunicode/include/SkUnicode.h"
#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_apple.h"
#endif
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu.h"
#endif
#include "src/core/SkUTF.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/viewer/Slide.h"

#include <algorithm>
#include <functional>
#include <vector>

using namespace skia::textlayout;

// Every slide compares the HarfBuzz and CoreText engines, so none exists without both
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT) && !defined(SK_PARAGRAPH_DISABLE_HARFBUZZ)

namespace {

// Palette for engine comparison cards (red = Legacy HarfBuzz, green = Candidate CoreText)
constexpr SkColor kInk = SkColorSetRGB(15, 23, 42);
constexpr SkColor kDark = SkColorSetRGB(30, 41, 59);
constexpr SkColor kMuted = SkColorSetRGB(100, 116, 139);
constexpr SkColor kRed = SkColorSetRGB(220, 38, 38);
constexpr SkColor kGreen = SkColorSetRGB(22, 163, 74);
constexpr SkColor kBlue = SkColorSetRGB(37, 99, 235);
constexpr SkScalar kSpacing = 16.0f;
// Same string as SkParagraph_Flutter_150824_18915_SystemFontOpticalSizeAndTracking, so the
// measured ratios on the tracking slides are the ones the test asserts
constexpr char kTrackingText[] = "DynamicTrackingTest Hello World";

// getRectsForRange takes UTF-16 offsets while Run::textRange() is in UTF-8 bytes
size_t utf16Index(const char* text, size_t utf8Index) {
    const char* ptr = text;
    const char* end = text + utf8Index;
    size_t units = 0;
    while (ptr < end) {
        SkUnichar u = SkUTF::NextUTF8(&ptr, end);
        if (u < 0) {
            break;
        }
        units += u > 0xFFFF ? 2 : 1;
    }
    return units;
}

class ParagraphBugSlide_Base : public Slide {
public:
    void load(SkScalar w, SkScalar h) override { fSize = {w, h}; }
    void resize(SkScalar w, SkScalar h) override { fSize = {w, h}; }

protected:
    SkSize fSize = {1050, 750};

    sk_sp<FontCollection> getFontCollection() {
        if (!fFontCollection) {
            fFontCollection = sk_make_sp<FontCollection>();
            fFontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
            fFontCollection->enableFontFallback();
        }
        return fFontCollection;
    }

    // Same pairing as SkParagraphFlutterBugsTest so slides and tests agree: Apple's SkUnicode
    // for CoreText, ICU for HarfBuzz; ParagraphBuilder::Build rejects a missing one
    sk_sp<SkUnicode> getUnicode(TextShapingEngine engine) {
#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
        if (engine == TextShapingEngine::kCoreText) {
            return SkUnicodes::Apple::Make();
        }
#endif
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
        return SkUnicodes::ICU::Make();
#else
        return nullptr;
#endif
    }

    void drawText(SkCanvas* canvas,
                  const char* str,
                  SkScalar x,
                  SkScalar y,
                  SkColor color,
                  bool bold = false,
                  SkScalar fontSize = 12.0f) {
        SkFont font(ToolUtils::DefaultTypeface(), fontSize);
        font.setEmbolden(bold);
        SkPaint paint;
        paint.setColor(color);
        paint.setAntiAlias(true);
        canvas->drawString(str, x, y, font, paint);
    }

    void drawFrame(SkCanvas* canvas, const SkRect& r, SkColor color, bool dashed = false) {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(color);
        paint.setStrokeWidth(dashed ? 1.5f : 1.2f);
        paint.setAntiAlias(true);
        if (dashed) {
            const SkScalar intervals[] = {4.0f, 3.0f};
            paint.setPathEffect(SkDashPathEffect::Make(intervals, 0.0f));
        }
        canvas->drawRect(r, paint);
    }

    void drawHeader(SkCanvas* canvas,
                    const char* bugId,
                    const char* title,
                    const char* status,
                    SkColor statusBg,
                    const char* summary) {
        SkScalar width = fSize.width();

        SkPaint bgPaint;
        bgPaint.setColor(SkColorSetRGB(246, 248, 251));
        canvas->drawRect(SkRect::MakeXYWH(0, 0, width, 84.0f), bgPaint);

        SkPaint divider;
        divider.setColor(SkColorSetRGB(222, 228, 237));
        divider.setStrokeWidth(1.0f);
        canvas->drawLine(0, 84.0f, width, 84.0f, divider);

        drawText(canvas, bugId, 24.0f, 28.0f, kBlue, true, 13.0f);
        drawText(canvas, title, 24.0f, 54.0f, kInk, true, 19.0f);

        SkFont badgeFont(ToolUtils::DefaultTypeface(), 11.0f);
        badgeFont.setEmbolden(true);
        SkScalar badgeWidth =
                badgeFont.measureText(status, strlen(status), SkTextEncoding::kUTF8) + 16.0f;
        SkRect badgeRect = SkRect::MakeXYWH(width - badgeWidth - 24.0f, 16.0f, badgeWidth, 22.0f);
        SkPaint badgeBg;
        badgeBg.setColor(statusBg);
        badgeBg.setAntiAlias(true);
        canvas->drawRoundRect(badgeRect, 4.0f, 4.0f, badgeBg);
        drawText(canvas,
                 status,
                 badgeRect.fLeft + 8.0f,
                 badgeRect.fBottom - 6.0f,
                 SK_ColorWHITE,
                 true,
                 11.0f);

        if (summary && summary[0]) {
            drawText(canvas, summary, 24.0f, 74.0f, kMuted);
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

        drawText(canvas, header, r.fLeft + 14.0f, r.fTop + 21.0f, headerColor, true, 13.0f);

        if (note && note[0]) {
            drawText(canvas, note, r.fLeft + 14.0f, r.fBottom - 10.0f, kMuted, false, 11.0f);
        }
    }

    void drawComparisonSlide(SkCanvas* canvas,
                             const char* bugId,
                             const char* title,
                             const char* status,
                             SkColor statusBg,
                             const char* summary,
                             const char* leftTitle,
                             const char* leftSubtitle,
                             const std::function<void(const SkRect&)>& drawLeft,
                             const char* rightTitle,
                             const char* rightSubtitle,
                             const std::function<void(const SkRect&)>& drawRight,
                             const char* oldDefectExplanation,
                             const char* newFixExplanation,
                             const char* invariantExplanation) {
        canvas->drawColor(SkColorSetRGB(248, 250, 252));
        drawHeader(canvas, bugId, title, status, statusBg, summary);

        SkScalar cardWidth = (fSize.width() - 72.0f) / 2.0f;
        SkScalar cardHeight = 424.0f;

        SkRect leftRect = SkRect::MakeXYWH(24.0f, 96.0f, cardWidth, cardHeight);
        drawCard(canvas, leftRect, leftTitle, kRed, leftSubtitle);
        drawLeft(leftRect);

        SkRect rightRect = SkRect::MakeXYWH(48.0f + cardWidth, 96.0f, cardWidth, cardHeight);
        drawCard(canvas, rightRect, rightTitle, kGreen, rightSubtitle);
        drawRight(rightRect);

        SkRect bottomRect = SkRect::MakeXYWH(24.0f, 532.0f, fSize.width() - 48.0f, 194.0f);
        drawCard(canvas,
                 bottomRect,
                 "Comparative Analysis & Resolution Mechanics",
                 kInk,
                 "Every number on the cards above is measured from the live layouts");

        SkScalar bx = bottomRect.fLeft + 20.0f;
        SkScalar by = bottomRect.fTop + 56.0f;
        drawText(canvas, "Legacy Version (Baseline):", bx, by, kRed, true);
        drawText(canvas, oldDefectExplanation, bx + 225.0f, by, kDark);
        by += 32.0f;
        drawText(canvas, "Candidate Version (New Engine):", bx, by, kGreen, true);
        drawText(canvas, newFixExplanation, bx + 225.0f, by, kDark);
        by += 32.0f;
        drawText(canvas, "Architectural Invariant:", bx, by, kBlue, true);
        drawText(canvas, invariantExplanation, bx + 225.0f, by, kDark);
    }

    static TextStyle makeStyle(const char* family, SkScalar fontSize, SkScalar spacing = 0.0f) {
        TextStyle ts;
        ts.setFontFamilies({SkString(family)});
        ts.setFontSize(fontSize);
        ts.setLetterSpacing(spacing);
        ts.setColor(kInk);
        return ts;
    }

    std::unique_ptr<Paragraph> makeParagraph(TextShapingEngine engine,
                                             const TextStyle& ts,
                                             const char* text,
                                             SkScalar layoutWidth = 620.0f) {
        ParagraphStyle style;
        style.setTextShapingEngine(engine);
        ParagraphBuilderImpl builder(style, getFontCollection(), getUnicode(engine));
        builder.pushStyle(ts);
        builder.addText(text);
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(layoutWidth);
        return paragraph;
    }

    // The dashed red slot shows how far the HarfBuzz layout of the same text extends past this one
    // Wide rows are drawn scaled down to maxWidth; the printed numbers stay unscaled
    SkScalar drawWidthRow(SkCanvas* canvas,
                          Paragraph* p,
                          SkScalar x,
                          SkScalar y,
                          SkScalar maxWidth,
                          SkScalar fontSize,
                          const char* label,
                          SkColor accent,
                          SkScalar harfBuzzWidth) {
        drawText(canvas, label, x, y, kDark, true);
        y += 10.0f;
        SkScalar w = p->getLongestLine();
        SkScalar scale = std::min(1.0f, maxWidth / std::max(w, harfBuzzWidth));
        canvas->save();
        canvas->translate(x, y);
        canvas->scale(scale, scale);
        p->paint(canvas, 0, 0);
        drawFrame(canvas, SkRect::MakeWH(w, p->getHeight()), accent);
        bool excess = harfBuzzWidth > w + 0.5f;
        if (excess) {
            drawFrame(canvas, SkRect::MakeLTRB(w, 0, harfBuzzWidth, p->getHeight()), kRed, true);
        }
        canvas->restore();
        y += p->getHeight() * scale + 18.0f;
        SkString info;
        info.printf("Width %.2f pt | %.3f pt per em", w, w / fontSize);
        if (excess) {
            info.appendf(" | HarfBuzz is %.2f pt wider (dashed)", harfBuzzWidth - w);
        }
        if (scale < 1.0f) {
            info.appendf(" | drawn at %.0f%%", scale * 100.0f);
        }
        drawText(canvas, info.c_str(), x, y, accent, true);
        return y + 34.0f;
    }

    // Glyphs without ink become dashed advance slots because letterSpacing still widens them
    size_t drawGlyphBoxes(SkCanvas* canvas, Paragraph* p, SkScalar x, SkScalar y, SkColor accent) {
        auto impl = static_cast<ParagraphImpl*>(p);
        std::vector<LineMetrics> lines;
        p->getLineMetrics(lines);
        SkASSERT(!impl->runs().empty() && !lines.empty());
        const Run& run = impl->runs()[0];
        auto glyphs = run.glyphs();
        auto pos = run.positions();
        auto off = run.offsets();
        // Run keeps a trailing sentinel position, so pos[i + 1] exists for every glyph
        SkASSERT(pos.size() == run.size() + 1);
        SkScalar originX = x + lines[0].fLeft;
        SkScalar baseY = y + lines[0].fBaseline;
        size_t phantoms = 0;
        for (size_t i = 0; i < run.size(); ++i) {
            SkScalar gx = originX + pos[i].fX - pos[0].fX + off[i].fX;
            SkScalar gy = baseY + pos[i].fY + off[i].fY;
            SkRect ink = run.font().getBounds(glyphs[i], nullptr);
            if (!ink.isEmpty()) {
                drawFrame(canvas, ink.makeOffset(gx, gy), accent);
                continue;
            }
            ++phantoms;
            SkScalar slot = std::max(pos[i + 1].fX - pos[i].fX, 2.0f);
            drawFrame(canvas,
                      SkRect::MakeLTRB(gx, baseY + run.ascent(), gx + slot, baseY + run.descent()),
                      kRed,
                      true);
        }
        return phantoms;
    }

    void drawSpacingCard(SkCanvas* canvas,
                         const SkRect& card,
                         Paragraph* p0,
                         Paragraph* p16,
                         SkColor accent,
                         const SkString& extra) {
        SkScalar x = card.fLeft + 20.0f;
        SkScalar y = card.fTop + 56.0f;
        const struct {
            Paragraph* fPara;
            SkScalar fSpacing;
        } rows[] = {{p0, 0.0f}, {p16, kSpacing}};
        for (const auto& row : rows) {
            auto impl = static_cast<ParagraphImpl*>(row.fPara);
            SkString label;
            label.printf("letterSpacing = %.0f pt: %zu run(s), %zu glyphs in run 0, width %.2f pt",
                         row.fSpacing,
                         impl->runs().size(),
                         impl->runs().empty() ? 0 : impl->runs()[0].size(),
                         row.fPara->getLongestLine());
            drawText(canvas, label.c_str(), x, y, kDark, true);
            y += 14.0f;
            row.fPara->paint(canvas, x, y);
            size_t phantoms = drawGlyphBoxes(canvas, row.fPara, x, y, accent);
            y += row.fPara->getHeight() + 16.0f;
            if (phantoms > 0) {
                SkString note;
                note.printf("%zu glyph(s) without ink (dashed red) still own an advance slot",
                            phantoms);
                drawText(canvas, note.c_str(), x, y, kRed);
                y += 18.0f;
            }
            y += 14.0f;
        }
        SkScalar delta = p16->getLongestLine() - p0->getLongestLine();
        SkString deltaStr;
        deltaStr.printf("Width delta +%.1f pt = %.0f spaced slots x %.0f pt",
                        delta,
                        delta / kSpacing,
                        kSpacing);
        drawText(canvas, deltaStr.c_str(), x, y, accent, true, 13.0f);
        if (!extra.isEmpty()) {
            drawText(canvas, extra.c_str(), x, y + 22.0f, accent, true, 13.0f);
        }
    }

    void drawSpacingSlide(SkCanvas* canvas,
                          const char* bugId,
                          const char* title,
                          const char* summary,
                          const char* family,
                          SkScalar fontSize,
                          const char* text,
                          const std::function<SkString(Paragraph*, Paragraph*)>& extra,
                          const char* leftSubtitle,
                          const char* rightSubtitle,
                          const char* defect,
                          const char* fix,
                          const char* invariant) {
        auto hb0 = makeParagraph(TextShapingEngine::kDefault, makeStyle(family, fontSize), text);
        auto hb16 = makeParagraph(
                TextShapingEngine::kDefault, makeStyle(family, fontSize, kSpacing), text);
        auto ct0 = makeParagraph(TextShapingEngine::kCoreText, makeStyle(family, fontSize), text);
        auto ct16 = makeParagraph(
                TextShapingEngine::kCoreText, makeStyle(family, fontSize, kSpacing), text);

        drawComparisonSlide(
                canvas,
                bugId,
                title,
                "FIXED IN CORETEXT",
                kGreen,
                summary,
                "Legacy Version: HarfBuzz (Run::addLetterSpacesEvenly)",
                leftSubtitle,
                [&](const SkRect& card) {
                    drawSpacingCard(canvas,
                                    card,
                                    hb0.get(),
                                    hb16.get(),
                                    kRed,
                                    extra(hb0.get(), hb16.get()));
                },
                "Candidate Version: CoreText (kCTTrackingAttributeName)",
                rightSubtitle,
                [&](const SkRect& card) {
                    drawSpacingCard(canvas,
                                    card,
                                    ct0.get(),
                                    ct16.get(),
                                    kGreen,
                                    extra(ct0.get(), ct16.get()));
                },
                defect,
                fix,
                invariant);
    }

    void drawTrackingSlide(SkCanvas* canvas,
                           const char* bugId,
                           const char* title,
                           const char* summary,
                           const char* text,
                           SkScalar smallSize,
                           SkScalar largeSize,
                           SkScalar cupertinoSize,
                           SkScalar cupertinoSpacing,
                           const char* defect,
                           const char* fix,
                           const char* invariant) {
        const char* family = ".AppleSystemUIFont";
        // Unbounded like the test, so getLongestLine() is the single-line width the ratio needs
        const SkScalar kNoWrap = 2000.0f;
        auto hbSmall = makeParagraph(
                TextShapingEngine::kDefault, makeStyle(family, smallSize), text, kNoWrap);
        auto hbLarge = makeParagraph(
                TextShapingEngine::kDefault, makeStyle(family, largeSize), text, kNoWrap);
        auto ctSmall = makeParagraph(
                TextShapingEngine::kCoreText, makeStyle(family, smallSize), text, kNoWrap);
        auto ctLarge = makeParagraph(
                TextShapingEngine::kCoreText, makeStyle(family, largeSize), text, kNoWrap);

        auto drawCardRows = [&](const SkRect& card,
                                Paragraph* small,
                                Paragraph* large,
                                SkColor accent,
                                bool compareToHarfBuzz) {
            SkScalar x = card.fLeft + 20.0f;
            SkScalar y = card.fTop + 56.0f;
            SkScalar maxWidth = card.width() - 40.0f;
            SkString label;
            label.printf("%.0f pt .AppleSystemUIFont:", smallSize);
            y = drawWidthRow(canvas,
                             small,
                             x,
                             y,
                             maxWidth,
                             smallSize,
                             label.c_str(),
                             accent,
                             compareToHarfBuzz ? hbSmall->getLongestLine() : 0.0f);
            label.printf("%.0f pt .AppleSystemUIFont:", largeSize);
            y = drawWidthRow(canvas,
                             large,
                             x,
                             y,
                             maxWidth,
                             largeSize,
                             label.c_str(),
                             accent,
                             compareToHarfBuzz ? hbLarge->getLongestLine() : 0.0f);
            SkString ratio;
            ratio.printf("Width ratio %.0f pt / %.0f pt = %.3f (linear scaling = %.3f)",
                         largeSize,
                         smallSize,
                         large->getLongestLine() / small->getLongestLine(),
                         largeSize / smallSize);
            drawText(canvas, ratio.c_str(), x, y, accent, true, 13.0f);
            // When CupertinoTextThemeData sets explicit letterSpacing, verify it replaces auto
            // trak via NSCTFontUnscaledTrackingAttribute=0 instead of double-tracking
            if (cupertinoSize > 0.0f) {
                TextShapingEngine eng = compareToHarfBuzz ? TextShapingEngine::kCoreText
                                                          : TextShapingEngine::kDefault;
                auto pAuto = makeParagraph(eng, makeStyle(family, cupertinoSize), text, kNoWrap);
                auto pExp = makeParagraph(eng,
                                          makeStyle("CupertinoSystemText",
                                                    cupertinoSize,
                                                    cupertinoSpacing),
                                          text,
                                          kNoWrap);
                SkString expLine;
                if (compareToHarfBuzz) {
                    expLine.printf(
                            "%.0f pt Cupertino letterSpacing=%+.2f: %.2f pt (auto-trak %.2f pt; "
                            "UnscaledTracking=0 avoids double tracking)",
                            cupertinoSize,
                            cupertinoSpacing,
                            pExp->getLongestLine(),
                            pAuto->getLongestLine());
                } else {
                    expLine.printf(
                            "%.0f pt Cupertino letterSpacing=%+.2f: %.2f pt (falls back without "
                            ".SFNS opsz)",
                            cupertinoSize,
                            cupertinoSpacing,
                            pExp->getLongestLine());
                }
                drawText(canvas, expLine.c_str(), x, y + 22.0f, accent, true, 12.0f);
            }
        };

        drawComparisonSlide(
                canvas,
                bugId,
                title,
                "FIXED IN CORETEXT",
                kGreen,
                summary,
                "Legacy Version: HarfBuzz (SkCTFontCreateExactCopy)",
                "Size-frozen CTFont copy: advances scale linearly with point size",
                [&](const SkRect& card) {
                    drawCardRows(card, hbSmall.get(), hbLarge.get(), kRed, false);
                },
                "Candidate Version: CoreText (.SFNS 'opsz' + 'trak')",
                "CTFont created at the target size; dashed red = extra HarfBuzz width",
                [&](const SkRect& card) {
                    drawCardRows(card, ctSmall.get(), ctLarge.get(), kGreen, true);
                },
                defect,
                fix,
                invariant);
    }

private:
    sk_sp<FontCollection> fFontCollection;
};

class ParagraphSlide_BugsOverview : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_BugsOverview() { fName = "ParagraphSlide_BugsOverview"; }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SkColorSetRGB(248, 250, 252));
        // All 10 cataloged Flutter/Skia bugs are resolved by the CoreText shaping pipeline
        drawHeader(canvas,
                   "DASHBOARD",
                   "SkParagraph HarfBuzz vs CoreText: Verified Layout Differences",
                   "10 SLIDES: ALL 10 FIXED IN CORETEXT",
                   kGreen,
                   "Each slide lays out the same text with both engines and draws the measured "
                   "geometry; each is backed by a dm test");

        struct BugEntry {
            const char* id;
            const char* title;
            const char* test;
            const char* badge;
            SkColor badgeColor;
        };
        const BugEntry bugs[] = {
                {"Flutter 150824",
                 "Size-dependent .SFNS tracking (11 pt vs 48 pt)",
                 "SkParagraph_Flutter_150824_18915_SystemFontOpticalSizeAndTracking",
                 "FIXED",
                 kGreen},
                {"Flutter 18915 & 147708",
                 "Optical size: 28 pt / 14 pt width ratio below 2.0",
                 "SkParagraph_Flutter_150824_18915_SystemFontOpticalSizeAndTracking",
                 "FIXED",
                 kGreen},
                {"Flutter 147708",
                 "CupertinoSystemText/Display aliases with fontFamilyFallback",
                 "SkParagraph_Flutter_147708_CupertinoAliasesBypassFallback",
                 "FIXED",
                 kGreen},
                {"Flutter 193812",
                 ".AppleSystemUIFont Rounded, Serif, and Monospaced designs",
                 "SkParagraph_Flutter_193812_SystemDesigns",
                 "FIXED",
                 kGreen},
                {"Flutter 22572",
                 "17 pt body text tracking vs 11 pt (auto trak + Cupertino letterSpacing)",
                 "SkParagraph_Flutter_150824_18915_SystemFontOpticalSizeAndTracking",
                 "FIXED",
                 kGreen},
                {"Flutter 121680 & 108720",
                 "Keycap emoji: zero-ink U+FE0F glyphs take letterSpacing",
                 "SkParagraph_Flutter_121680_108720_KeycapEmojiCluster",
                 "FIXED",
                 kGreen},
                {"Flutter 137976 & 100342",
                 "ZWJ emoji: zero-ink U+FE0F glyphs take letterSpacing",
                 "SkParagraph_Flutter_137976_100342_ZWJEmojiClusterBounds",
                 "FIXED",
                 kGreen},
                {"Flutter 118127",
                 "Combining acute drifts off its base under letterSpacing",
                 "SkParagraph_Flutter_118127_CombiningDiacriticsLetterSpacing",
                 "FIXED",
                 kGreen},
                {"Flutter 144528",
                 "Devanagari matras detach under letterSpacing",
                 "SkParagraph_Flutter_144528_IndicConjunctLetterSpacing",
                 "FIXED",
                 kGreen},
                {"Skia b/318667611",
                 "Unmapped PUA: LastResort tofu vs .SFNS .notdef",
                 "SkParagraph_Skia_b_318667611_AppleSystemUIFontCascade",
                 "FIXED",
                 kGreen},
        };

        const SkScalar x = 24.0f;
        const SkScalar rowWidth = fSize.width() - 48.0f;
        const SkScalar rowHeight = 52.0f;
        SkScalar y = 100.0f;
        SkFont badgeFont(ToolUtils::DefaultTypeface(), 10.0f);
        badgeFont.setEmbolden(true);

        for (const BugEntry& bug : bugs) {
            SkRect rowRect = SkRect::MakeXYWH(x, y, rowWidth, rowHeight);
            SkPaint rowBg;
            rowBg.setColor(SK_ColorWHITE);
            rowBg.setAntiAlias(true);
            canvas->drawRoundRect(rowRect, 6.0f, 6.0f, rowBg);
            drawFrame(canvas, rowRect, SkColorSetRGB(226, 232, 240));

            drawText(canvas, bug.id, x + 14.0f, y + 22.0f, kInk, true, 13.0f);
            drawText(canvas, bug.title, x + 230.0f, y + 22.0f, kDark, false, 13.0f);
            drawText(canvas, bug.test, x + 230.0f, y + 41.0f, kMuted, false, 11.0f);

            SkScalar bWidth =
                    badgeFont.measureText(bug.badge, strlen(bug.badge), SkTextEncoding::kUTF8) +
                    16.0f;
            SkRect bRect =
                    SkRect::MakeXYWH(rowRect.fRight - bWidth - 14.0f, y + 15.0f, bWidth, 22.0f);
            SkPaint bBg;
            bBg.setColor(bug.badgeColor);
            bBg.setAntiAlias(true);
            canvas->drawRoundRect(bRect, 4.0f, 4.0f, bBg);
            drawText(canvas,
                     bug.badge,
                     bRect.fLeft + 8.0f,
                     bRect.fBottom - 7.0f,
                     SK_ColorWHITE,
                     true,
                     10.0f);
            y += rowHeight + 6.0f;
        }
    }
};

class ParagraphSlide_Flutter_150824_DynamicTracking : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Flutter_150824_DynamicTracking() {
        fName = "ParagraphSlide_Flutter_150824_DynamicTracking";
    }

    void draw(SkCanvas* canvas) override {
        // Explicit letterSpacing suppresses auto trak via NSCTFontUnscaledTrackingAttribute=0
        drawTrackingSlide(
                canvas,
                "FLUTTER ISSUE 150824",
                "Size-Dependent Tracking of San Francisco (.AppleSystemUIFont)",
                "CoreText applies .SFNS 'trak' by default and replaces it cleanly when "
                "explicit letterSpacing is set",
                kTrackingText,
                11.0f,
                48.0f,
                17.0f,
                -0.41f,
                "SkCTFontCreateExactCopy pins the CTFont size, so HarfBuzz advances scale "
                "linearly and miss 'trak'.",
                "CoreText applies .SFNS 'trak' when letterSpacing=0 and sets "
                "NSCTFontUnscaledTrackingAttribute=0 when letterSpacing!=0.",
                "Automatic .SFNS tracking and explicit theme letterSpacing never double-track "
                "while optical sizing stays active.");
    }
};

class ParagraphSlide_Flutter_18915_147708_OpticalSize : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Flutter_18915_147708_OpticalSize() {
        fName = "ParagraphSlide_Flutter_18915_147708_OpticalSize";
    }

    void draw(SkCanvas* canvas) override {
        drawTrackingSlide(
                canvas,
                "FLUTTER ISSUES 18915 & 147708",
                "Optical Sizing of .AppleSystemUIFont (Text vs Display)",
                "Doubling the size from 14 pt to 28 pt must not double the width "
                "(see also #147708 slide for CupertinoSystemText/Display fallback)",
                kTrackingText,
                14.0f,
                28.0f,
                0.0f,
                0.0f,
                "HarfBuzz shapes a size-frozen copy, so 28 pt is exactly 2.0x the 14 pt width.",
                "CoreText selects the display optical size and tracking at 28 pt, contracting "
                "the ratio below 2.0.",
                "Optical size and tracking are functions of the point size, never constants of "
                "the typeface.");
    }
};

class ParagraphSlide_Flutter_22572_DefaultTextTrackingHIGDelta : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Flutter_22572_DefaultTextTrackingHIGDelta() {
        fName = "ParagraphSlide_Flutter_22572_DefaultTextTrackingHIGDelta";
    }

    void draw(SkCanvas* canvas) override {
        drawTrackingSlide(
                canvas,
                "FLUTTER ISSUE 22572",
                "Default Tracking of 17 pt Body Text (Apple HIG)",
                "17 pt body text is tighter per em than 11 pt caption text; explicit Cupertino "
                "letterSpacing (-0.41 pt) replaces auto 'trak' without double tracking",
                kTrackingText,
                11.0f,
                17.0f,
                17.0f,
                -0.41f,
                "HarfBuzz renders 11 pt and 17 pt with the same per-em width (no 'trak' or "
                "'opsz').",
                "CoreText applies .SFNS 'trak' at letterSpacing=0 and replaces it via "
                "NSCTFontUnscaledTrackingAttribute=0 when Cupertino sets -0.41 pt.",
                "Body text density matches Apple HIG whether driven by .SFNS 'trak' or "
                "explicit Cupertino letterSpacing.");
    }
};

class ParagraphSlide_Flutter_121680_108720_KeycapEmojiCluster : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Flutter_121680_108720_KeycapEmojiCluster() {
        fName = "ParagraphSlide_Flutter_121680_108720_KeycapEmojiCluster";
    }

    void draw(SkCanvas* canvas) override {
        drawSpacingSlide(
                canvas,
                "FLUTTER ISSUES 121680 & 108720",
                "Keycap Emoji Sequences under Letter Spacing",
                "\"1\\uFE0F\\u20E3 2\\uFE0F\\u20E3 3\\uFE0F\\u20E3\": both engines keep 3 grapheme "
                "clusters, but HarfBuzz emits an extra zero-ink glyph per keycap",
                ".AppleSystemUIFont",
                36.0f,
                "1\xEF\xB8\x8F\xE2\x83\xA3"
                "2\xEF\xB8\x8F\xE2\x83\xA3"
                "3\xEF\xB8\x8F\xE2\x83\xA3",
                [](Paragraph*, Paragraph*) { return SkString(); },
                "Zero-ink U+FE0F glyphs get their own letterSpacing slot",
                "One glyph per keycap; one letterSpacing slot per cluster",
                "Apple Color Emoji morx leaves a zero-ink glyph for U+FE0F; per-glyph spacing "
                "widens every keycap twice.",
                "CoreText emits one glyph per keycap and tracks once per grapheme cluster.",
                "letterSpacing is added once per user-perceived character, never to inkless "
                "glyphs inside a cluster.");
    }
};

class ParagraphSlide_Flutter_137976_100342_ZWJEmojiClusterBounds : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Flutter_137976_100342_ZWJEmojiClusterBounds() {
        fName = "ParagraphSlide_Flutter_137976_100342_ZWJEmojiClusterBounds";
    }

    void draw(SkCanvas* canvas) override {
        drawSpacingSlide(
                canvas,
                "FLUTTER ISSUES 137976 & 100342",
                "ZWJ Emoji Sequences with U+FE0F under Letter Spacing",
                "Rainbow flag, heart on fire, woman technologist: HarfBuzz keeps zero-ink "
                "U+FE0F glyphs inside the first two sequences",
                ".AppleSystemUIFont",
                36.0f,
                "\xF0\x9F\x8F\xB3\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x8C\x88"
                "\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x94\xA5"
                "\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x92\xBB",
                [](Paragraph*, Paragraph*) { return SkString(); },
                "Zero-ink U+FE0F glyphs inflate the cluster advance",
                "One fused glyph per ZWJ sequence",
                "HarfBuzz leaves a zero-ink U+FE0F glyph in FE0F-bearing ZWJ sequences; "
                "letterSpacing widens it.",
                "CoreText fuses each ZWJ sequence into one glyph and tracks once per cluster.",
                "Inkless glyphs inside an emoji cluster must not receive inter-character "
                "spacing.");
    }
};

class ParagraphSlide_Flutter_118127_CombiningDiacriticsLetterSpacing
        : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Flutter_118127_CombiningDiacriticsLetterSpacing() {
        fName = "ParagraphSlide_Flutter_118127_CombiningDiacriticsLetterSpacing";
    }

    void draw(SkCanvas* canvas) override {
        // x + U+0301 has no precomposed NFC form, so the mark stays a separate glyph
        drawSpacingSlide(
                canvas,
                "FLUTTER ISSUE 118127",
                "Combining Acute Accent Attachment under Letter Spacing",
                "\"ax\\u0301b\" in Helvetica: the accent glyph must stay over 'x' when "
                "letterSpacing is applied",
                "Helvetica",
                40.0f,
                "ax\xCC\x81"
                "b",
                [](Paragraph* p0, Paragraph* p16) {
                    auto markOffset = [](Paragraph* p) -> SkScalar {
                        auto runs = static_cast<ParagraphImpl*>(p)->runs();
                        SkASSERT(!runs.empty() && runs[0].size() >= 3);
                        const Run& run = runs[0];
                        auto pos = run.positions();
                        auto off = run.offsets();
                        return (pos[2].fX + off[2].fX) - (pos[1].fX + off[1].fX);
                    };
                    SkString s;
                    s.printf("Accent drift relative to 'x': %+.1f pt",
                             markOffset(p16) - markOffset(p0));
                    return s;
                },
                "Accent glyph is shifted along with the next letter",
                "Accent keeps its mark-to-base offset over 'x'",
                "Run::addLetterSpacesEvenly shifts glyph i by i x letterSpacing, including "
                "the zero-advance mark.",
                "CoreText tracks between grapheme clusters, leaving mark positioning intact.",
                "Combining marks stay anchored to their base glyph under inter-cluster "
                "spacing.");
    }
};

class ParagraphSlide_Flutter_144528_IndicConjunctLetterSpacing : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Flutter_144528_IndicConjunctLetterSpacing() {
        fName = "ParagraphSlide_Flutter_144528_IndicConjunctLetterSpacing";
    }

    void draw(SkCanvas* canvas) override {
        drawSpacingSlide(
                canvas,
                "FLUTTER ISSUE 144528",
                "Devanagari Matra Attachment under Letter Spacing",
                "\"Hindi\" in Kohinoor Devanagari: 2 orthographic syllables shaped into 4 glyphs",
                "Kohinoor Devanagari",
                40.0f,
                "\xE0\xA4\xB9\xE0\xA4\xBF\xE0\xA4\xA8\xE0\xA5\x8D\xE0\xA4\xA6\xE0\xA5\x80",
                [](Paragraph* p0, Paragraph* p16) {
                    auto gap = [](Paragraph* p) -> SkScalar {
                        auto runs = static_cast<ParagraphImpl*>(p)->runs();
                        SkASSERT(!runs.empty() && runs[0].size() >= 2);
                        auto pos = runs[0].positions();
                        return pos[1].fX - pos[0].fX;
                    };
                    SkString s;
                    s.printf("Pre-base matra to consonant gap: %+.1f pt", gap(p16) - gap(p0));
                    return s;
                },
                "Matras pulled away from their consonants",
                "Syllables spaced as intact units",
                "Per-glyph spacing inserts a gap between each matra and the consonant it "
                "belongs to.",
                "CoreText tracks between orthographic syllables, keeping matras attached.",
                "Spacing never splits an orthographic syllable.");
    }
};

class ParagraphSlide_Skia_b_318667611_AppleSystemUIFontCascade : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Skia_b_318667611_AppleSystemUIFontCascade() {
        fName = "ParagraphSlide_Skia_b_318667611_AppleSystemUIFontCascade";
    }

    void draw(SkCanvas* canvas) override {
        const char* text =
                "SF Pro  \xE7\xAE\x80\xE4\xBD\x93  \xD9\x85\xD8\xB1\xD8\xAD\xD8\xA8\xD8\xA7  "
                "\xF4\x80\x80\x80\xF4\x80\x80\x81\xF4\x80\x80\x82";
        auto style = makeStyle(".AppleSystemUIFont", 28.0f);
        auto hb = makeParagraph(TextShapingEngine::kDefault, style, text);
        auto ct = makeParagraph(TextShapingEngine::kCoreText, style, text);

        auto drawRuns = [&](const SkRect& card, Paragraph* p) {
            SkScalar x = card.fLeft + 20.0f;
            SkScalar y = card.fTop + 60.0f;
            p->paint(canvas, x, y);
            auto impl = static_cast<ParagraphImpl*>(p);
            SkScalar listY = y + p->getHeight() + 30.0f;
            size_t index = 0;
            for (const Run& run : impl->runs()) {
                SkString ps;
                run.font().getTypeface()->getPostScriptName(&ps);
                size_t notdefs = 0;
                for (SkGlyphID g : run.glyphs()) {
                    notdefs += g == 0 ? 1 : 0;
                }
                SkColor color = ps.contains("LastResort") ? kRed : notdefs > 0 ? kGreen : kBlue;
                auto rects = p->getRectsForRange(utf16Index(text, run.textRange().start),
                                                 utf16Index(text, run.textRange().end),
                                                 RectHeightStyle::kTight,
                                                 RectWidthStyle::kTight);
                for (const TextBox& box : rects) {
                    drawFrame(canvas, box.rect.makeOffset(x, y), color, color != kBlue);
                }
                SkString line;
                line.printf("Run %zu: %s, %zu glyphs, %zu .notdef",
                            index++,
                            ps.c_str(),
                            run.size(),
                            notdefs);
                drawText(canvas, line.c_str(), x, listY, color, color != kBlue);
                listY += 20.0f;
            }
            SkString summary;
            summary.printf("unresolvedGlyphs() = %d", p->unresolvedGlyphs());
            drawText(canvas, summary.c_str(), x, listY + 10.0f, kDark, true, 13.0f);
        };

        drawComparisonSlide(
                canvas,
                "SKIA BUGANIZER b/318667611 & b/328418459",
                "Unmapped PUA Codepoints: LastResort Tofu vs .SFNS .notdef",
                "FIXED IN CORETEXT",
                kGreen,
                "Latin, CJK and Arabic resolve identically; only U+100000..U+100002 differ "
                "(red = LastResort run, green = .notdef run)",
                "Legacy Version: HarfBuzz (matchFamilyStyleCharacter)",
                "Character fallback lands on LastResort, drawing [?] boxes",
                [&](const SkRect& card) { drawRuns(card, hb.get()); },
                "Candidate Version: CoreText (isLastResortFont suppression)",
                "LastResort is rejected; primary font .notdef is kept",
                [&](const SkRect& card) { drawRuns(card, ct.get()); },
                "HarfBuzz fallback resolves unmapped PUA codepoints to LastResort, hiding them "
                "from unresolvedGlyphs().",
                "CoreTextLineShaper rejects LastResort and keeps .SFNS glyph 0, reported as "
                "unresolved.",
                "Unmapped codepoints render as the primary font .notdef and are reported, never "
                "masked by LastResort.");
    }
};

class ParagraphSlide_Flutter_147708_CupertinoAliases : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Flutter_147708_CupertinoAliases() {
        fName = "ParagraphSlide_Flutter_147708_CupertinoAliases";
    }

    void draw(SkCanvas* canvas) override {
        auto styleWithFallback = [&](const char* family, SkScalar fontSize) {
            TextStyle ts;
            ts.setFontFamilies({SkString(family), SkString("Helvetica")});
            ts.setFontSize(fontSize);
            ts.setColor(kInk);
            return ts;
        };

        const SkScalar kNoWrap = 2000.0f;
        auto hbText = makeParagraph(TextShapingEngine::kDefault,
                                    styleWithFallback("CupertinoSystemText", 14.0f),
                                    kTrackingText,
                                    kNoWrap);
        auto hbDisp = makeParagraph(TextShapingEngine::kDefault,
                                    styleWithFallback("CupertinoSystemDisplay", 28.0f),
                                    kTrackingText,
                                    kNoWrap);
        auto ctText = makeParagraph(TextShapingEngine::kCoreText,
                                    styleWithFallback("CupertinoSystemText", 14.0f),
                                    kTrackingText,
                                    kNoWrap);
        auto ctDisp = makeParagraph(TextShapingEngine::kCoreText,
                                    styleWithFallback("CupertinoSystemDisplay", 28.0f),
                                    kTrackingText,
                                    kNoWrap);

        auto drawCardRows = [&](const SkRect& card,
                                Paragraph* small,
                                Paragraph* large,
                                SkColor accent,
                                bool compareToHarfBuzz) {
            SkScalar x = card.fLeft + 20.0f;
            SkScalar y = card.fTop + 56.0f;
            SkScalar maxWidth = card.width() - 40.0f;
            auto impl = static_cast<ParagraphImpl*>(large);
            SkString ps;
            if (!impl->runs().empty()) {
                impl->runs()[0].font().getTypeface()->getPostScriptName(&ps);
            }
            SkString label14;
            label14.printf("14 pt {CupertinoSystemText, Helvetica} -> %s:", ps.c_str());
            y = drawWidthRow(canvas,
                             small,
                             x,
                             y,
                             maxWidth,
                             14.0f,
                             label14.c_str(),
                             accent,
                             compareToHarfBuzz ? hbText->getLongestLine() : 0.0f);
            SkString label28;
            label28.printf("28 pt {CupertinoSystemDisplay, Helvetica} -> %s:", ps.c_str());
            y = drawWidthRow(canvas,
                             large,
                             x,
                             y,
                             maxWidth,
                             28.0f,
                             label28.c_str(),
                             accent,
                             compareToHarfBuzz ? hbDisp->getLongestLine() : 0.0f);
            SkString ratio;
            ratio.printf("Width ratio 28 pt / 14 pt = %.3f (resolved face: %s)",
                         large->getLongestLine() / small->getLongestLine(),
                         ps.c_str());
            drawText(canvas, ratio.c_str(), x, y, accent, true, 13.0f);
        };

        drawComparisonSlide(
                canvas,
                "FLUTTER ISSUE 147708",
                "CupertinoSystemText & CupertinoSystemDisplay with fontFamilyFallback",
                "FIXED IN CORETEXT",
                kGreen,
                "When fontFamilyFallback is non-empty, Cupertino aliases must resolve to .SFNS "
                "instead of falling through to the first fallback family",
                "Legacy Version: HarfBuzz (matchFamilyStyle Loop)",
                "Misses CupertinoSystemText/Display and falls through to Helvetica",
                [&](const SkRect& card) {
                    drawCardRows(card, hbText.get(), hbDisp.get(), kRed, false);
                },
                "Candidate Version: CoreText (CoreTextFontRegistry)",
                "Resolves Cupertino aliases directly to .SFNS with display optical sizing",
                [&](const SkRect& card) {
                    drawCardRows(card, ctText.get(), ctDisp.get(), kGreen, true);
                },
                "matchFamilyStyle has no entry for CupertinoSystemText/Display, so findTypefaces "
                "skips them and picks the first fontFamilyFallback face (Helvetica).",
                "CoreTextFontRegistry::isAppleSystemFont recognizes both Cupertino aliases and "
                "instantiates .SFNS at the requested point size.",
                "Cupertino system font aliases resolve as primary system UI faces before any "
                "fallback family is consulted.");
    }
};

class ParagraphSlide_Flutter_193812_SystemDesigns : public ParagraphBugSlide_Base {
public:
    ParagraphSlide_Flutter_193812_SystemDesigns() {
        fName = "ParagraphSlide_Flutter_193812_SystemDesigns";
    }

    void draw(SkCanvas* canvas) override {
        const char* sample = "System UI Design: 0123456789 iiiWWW";
        const struct {
            const char* fFamily;
            const char* fExpectedPs;
        } designs[] = {
                {".AppleSystemUIFontRounded", "Rounded"},
                {".AppleSystemUIFontSerif", "NewYork"},
                {".AppleSystemUIFontMonospaced", "Monospaced"},
        };

        auto drawEngineCard = [&](const SkRect& card, TextShapingEngine engine, SkColor accent) {
            SkScalar x = card.fLeft + 20.0f;
            SkScalar y = card.fTop + 54.0f;
            for (const auto& d : designs) {
                auto p = makeParagraph(engine, makeStyle(d.fFamily, 22.0f), sample);
                auto impl = static_cast<ParagraphImpl*>(p.get());
                SkString ps;
                if (!impl->runs().empty()) {
                    impl->runs()[0].font().getTypeface()->getPostScriptName(&ps);
                }
                bool matched = ps.contains(d.fExpectedPs);
                SkString header;
                header.printf("%s -> %s (width %.1f pt)",
                              d.fFamily,
                              ps.c_str(),
                              p->getLongestLine());
                drawText(canvas, header.c_str(), x, y, matched ? kGreen : kRed, true);
                y += 10.0f;
                p->paint(canvas, x, y);
                drawFrame(canvas,
                          SkRect::MakeXYWH(x, y, p->getLongestLine(), p->getHeight()),
                          matched ? kGreen : kRed);
                y += p->getHeight() + 26.0f;
            }
            auto pNarrow = makeParagraph(
                    engine, makeStyle(".AppleSystemUIFontMonospaced", 20.0f), "iiiiiiii");
            auto pWide = makeParagraph(
                    engine, makeStyle(".AppleSystemUIFontMonospaced", 20.0f), "WWWWWWWW");
            SkString monoInfo;
            monoInfo.printf("Monospaced check: \"iiiiiiii\" = %.1f pt vs \"WWWWWWWW\" = %.1f pt",
                            pNarrow->getLongestLine(),
                            pWide->getLongestLine());
            drawText(canvas, monoInfo.c_str(), x, y + 6.0f, accent, true, 13.0f);
        };

        drawComparisonSlide(
                canvas,
                "FLUTTER ISSUE 193812",
                "Apple System Font Design Variants: Rounded, Serif, Monospaced",
                "FIXED IN CORETEXT",
                kGreen,
                ".AppleSystemUIFont{Rounded,Serif,Monospaced} select SF Pro Rounded, New York, "
                "and SF Mono via NSCTFontUIFontDesignTrait",
                "Legacy Version: HarfBuzz (Default Font Manager)",
                "Unrecognized .AppleSystemUIFont* suffixes fall back to proportional Helvetica",
                [&](const SkRect& card) {
                    drawEngineCard(card, TextShapingEngine::kDefault, kRed);
                },
                "Candidate Version: CoreText (NSCTFontUIFontDesignTrait)",
                "Resolves .AppleSystemUIFontRounded, .NewYork, and .AppleSystemUIFontMonospaced",
                [&](const SkRect& card) {
                    drawEngineCard(card, TextShapingEngine::kCoreText, kGreen);
                },
                "Default font manager family matching does not recognize .AppleSystemUIFont "
                "design suffixes and falls back to Helvetica for all three.",
                "CoreTextFontRegistry maps Rounded, Serif, and Monospaced suffixes to "
                "NSCTFontUIFontDesignTrait with kCTFontTraitUIOptimized.",
                "System UI design variants resolve to distinct Apple system design families "
                "while preserving weight and optical sizing.");
    }
};

}  // namespace

DEF_SLIDE(return new ParagraphSlide_BugsOverview();)
DEF_SLIDE(return new ParagraphSlide_Flutter_150824_DynamicTracking();)
DEF_SLIDE(return new ParagraphSlide_Flutter_18915_147708_OpticalSize();)
DEF_SLIDE(return new ParagraphSlide_Flutter_147708_CupertinoAliases();)
DEF_SLIDE(return new ParagraphSlide_Flutter_193812_SystemDesigns();)
DEF_SLIDE(return new ParagraphSlide_Flutter_22572_DefaultTextTrackingHIGDelta();)
DEF_SLIDE(return new ParagraphSlide_Flutter_121680_108720_KeycapEmojiCluster();)
DEF_SLIDE(return new ParagraphSlide_Flutter_137976_100342_ZWJEmojiClusterBounds();)
DEF_SLIDE(return new ParagraphSlide_Flutter_118127_CombiningDiacriticsLetterSpacing();)
DEF_SLIDE(return new ParagraphSlide_Flutter_144528_IndicConjunctLetterSpacing();)
DEF_SLIDE(return new ParagraphSlide_Skia_b_318667611_AppleSystemUIFontCascade();)

#endif  // defined(SK_PARAGRAPH_NATIVE_CORETEXT) && !defined(SK_PARAGRAPH_DISABLE_HARFBUZZ)
