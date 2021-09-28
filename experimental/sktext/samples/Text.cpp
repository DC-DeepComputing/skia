// Copyright 2021 Google LLC.

#include "experimental/sktext/editor/Editor.h"
#include "experimental/sktext/src/Paint.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "include/core/SkTime.h"
#include "samplecode/Sample.h"
#include "src/core/SkOSFile.h"
#include "src/shaders/SkColorShader.h"
#include "src/utils/SkOSPath.h"
#include "src/utils/SkUTF.h"
#include "tools/Resources.h"
#include "tools/flags/CommandLineFlags.h"

using namespace skia::text;
using namespace skia::editor;

namespace {

std::u16string mirror(const std::u16string& text) {
    std::u16string result;
    result += u"\u202E";
    for (auto i = text.size(); i > 0; --i) {
        result += text[i - 1];
    }
    //for (auto ch : text) {
    //    result += ch;
    //}
    result += u"\u202C";
    return result;
}

std::u16string normal(const std::u16string& text) {
    std::u16string result;
    //result += u"\u202D";
    for (auto ch : text) {
        result += ch;
    }
    //result += u"\u202C";
    return result;
}

class TextSample_HelloWorld : public Sample {
protected:
    SkString name() override { return SkString("TextSample_HelloWorld"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        skia::text::Paint::drawText(u"Hello word", canvas, 0, 0);
    }

private:
    using INHERITED = Sample;
};

class TextSample_Align_Dir : public Sample {

public:
    TextSample_Align_Dir() : fUnicode(SkUnicode::Make()) { }
protected:
    SkString name() override { return SkString("TextSample_Align_Dir"); }

    void drawLine(SkCanvas* canvas, SkScalar w, SkScalar h,
                  const std::u16string& text,
                  TextAlign align,
                  TextDirection direction = TextDirection::kLtr) {
        const std::u16string& ellipsis = u"\u2026";
        SkScalar margin = 20;

        SkAutoCanvasRestore acr(canvas, true);

        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(SK_ColorWHITE);

        SkPaint foregroundPaint(SkColors::kBlack);
        SkPaint backgroundPaint(SkColors::kLtGray);
        Paint::drawText(direction == TextDirection::kRtl ? mirror(text) : normal(text),
                        canvas,
                        direction, align,
                        foregroundPaint, backgroundPaint,
                        SkString("Roboto"), 12.0f, SkFontStyle::Normal(),
                        0, 0);
    }

    void onDrawContent(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorDKGRAY);
        SkScalar width = this->width() / 4;
        SkScalar height = this->height() / 2;

        const std::u16string line = u"One line of text";

        drawLine(canvas, width, height, line, TextAlign::kLeft, TextDirection::kLtr);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kRight, TextDirection::kLtr);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kCenter, TextDirection::kLtr);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kJustify, TextDirection::kLtr);
        canvas->translate(-width * 3, height);

        drawLine(canvas, width, height, line, TextAlign::kLeft, TextDirection::kRtl);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kRight, TextDirection::kRtl);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kCenter, TextDirection::kRtl);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kJustify, TextDirection::kRtl);
        canvas->translate(width, 0);

    }

private:
    using INHERITED = Sample;
    std::unique_ptr<SkUnicode> fUnicode;
};

class TextSample_LongLTR : public Sample {
protected:
    SkString name() override { return SkString("TextSample_LongLTR"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        Paint::drawText(u"A very_very_very_very_very_very_very_very_very_very "
                "very_very_very_very_very_very_very_very_very_very very very very very very very "
                "very very very very very very very very very very very very very very very very "
                "very very very very very very very very very very very very very long text", canvas, this->width());

    }

private:
    using INHERITED = Sample;
    std::unique_ptr<SkUnicode> fUnicode;
};

class TextSample_LongRTL : public Sample {
protected:
    SkString name() override { return SkString("TextSample_LongRTL"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        Paint::drawText(u"LONG MIRRORED TEXT SHOULD SHOW RIGHT TO LEFT (AS NORMAL)", canvas, 0, 0);
    }

private:
    using INHERITED = Sample;
    std::unique_ptr<SkUnicode> fUnicode;
};

class Zalgo {
    private:
    std::u16string COMBINING_DOWN = u"\u0316\u0317\u0318\u0319\u031c\u031d\u031e\u031f\u0320\u0324\u0325\u0326\u0329\u032a\u032b\u032c\u032d\u032e\u032f\u0330\u0331\u0332\u0333\u0339\u033a\u033b\u033c\u0345\u0347\u0348\u0349\u034d\u034e\u0353\u0354\u0355\u0356\u0359\u035a\u0323";
    std::u16string COMBINING_UP = u"\u030d\u030e\u0304\u0305\u033f\u0311\u0306\u0310\u0352\u0357\u0351\u0307\u0308\u030a\u0342\u0343\u0344\u034a\u034b\u034c\u0303\u0302\u030c\u0350\u0300\u0301\u030b\u030f\u0312\u0313\u0314\u033d\u0309\u0363\u0364\u0365\u0366\u0367\u0368\u0369\u036a\u036b\u036c\u036d\u036e\u035b\u0346\u031a";
    std::u16string COMBINING_MIDDLE = u"\u0315\u031b\u0340\u0341\u0358\u0321\u0322\u0327\u0328\u0334\u0335\u0336\u034f\u035c\u035d\u035e\u035f\u0360\u0362\u0338\u0337\u0361\u0489";

    std::u16string randomMarks(std::u16string& combiningMarks, int limit) {
        std::u16string result;
        if (limit < 0) {
            limit = combiningMarks.size();
        }
        auto num = std::rand() % (limit / 1);
        for (size_t i = 0; i < num; ++i) {
            auto index = std::rand() % combiningMarks.size();
            result += combiningMarks[index];
        }
        return result;
    }

public:
    std::u16string zalgo(std::u16string victim, int limit) {
        std::u16string result;
        const char16_t* current = victim.data();
        const char16_t* end = current + victim.size();
        while (current < end) {
            SkUnichar unichar = SkUTF::NextUTF16((const uint16_t**)&current, (const uint16_t*)end);
            uint16_t buffer[2];
            size_t count = SkUTF::ToUTF16(unichar, buffer);
            result += buffer[0];
            if (count > 1) {
                result += buffer[1];
            }
            result += randomMarks(COMBINING_UP, limit);
            result += randomMarks(COMBINING_MIDDLE, limit);
            result += randomMarks(COMBINING_DOWN, limit);
        }
        return result;
    }
};

class TextSample_Zalgo : public Sample {
public:
    TextSample_Zalgo() {
        fFontChain = sk_sp<MultipleFontChain>(new MultipleFontChain({ "Noto Serif CJK JP", "Noto Color Emoji", "Roboto", }, 30.0f, SkFontStyle::Normal()));
        fForeground.setColor(SkColors::kBlack);
    }

    SkString name() override { return SkString("TextSample_Zalgo"); }

    bool onAnimate(double nanos) override {
        if (++fIndex > fLimit) {
            fRedraw = true;
            fIndex = 0;
        } else {
            fRepeat = true;
        }
        return true;
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        if (fFontChain->empty()) return;

        if (fRedraw || fIcuDrawableText == nullptr) {
            Zalgo zalgo;
            auto utf16 = zalgo.zalgo(u"English", -1);
            //utf16 = u"E😀n😀g😀l😀i😀s😀h😀";
            //utf16 = u"E￰ﾘﾀn￰ﾘﾀg￰ﾘﾀl￰ﾘﾀi￰ﾘﾀs￰ﾘﾀh￰ﾘﾀ";
            UnicodeText unicodeText(SkUnicode::Make(),
                                    SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
            if (!unicodeText.getUnicode()) return;
            auto utf8 = unicodeText.getUnicode()->convertUtf16ToUtf8(utf16);
            SkDebugf(">>> %s <<<\n", utf8.c_str());
            FontBlock fontBlock(utf16.size(), fFontChain);

            auto icuResolver =
                    unicodeText.resolveFonts<IcuFontResolvedText>(SkSpan<FontBlock>(&fontBlock, 1),
                                                                  TextDirection::kLtr,
                                                                  std::locale().name().c_str());
            auto icuResolvedFonts = icuResolver->resolvedFonts();

            auto harfbuzzResolver = unicodeText.resolveFonts<HarfbuzzFontResolvedText>(
                    SkSpan<FontBlock>(&fontBlock, 1),
                    TextDirection::kLtr,
                    std::locale().name().c_str());
            auto harfbuzzResolvedFonts = harfbuzzResolver->resolvedFonts();
            // 0045 FFF0 FF9F FF98 006E FFF0 FF9F FF98 0067 FFF0 FF9F FF98 006C FFF0 FF9F FF98 0069 FFF0 FF9F FF98 0073 FFF0 FF9F FF98 0068 FFF0 FF9F FF98
            bool match = true;
            match &= (icuResolvedFonts.size() == harfbuzzResolvedFonts.size());
            if (match) {
                for (auto i = 0ul; i < icuResolvedFonts.size(); ++i) {
                    auto& icu = icuResolvedFonts[i];
                    auto& hb = harfbuzzResolvedFonts[i];
                    match &= (icu.textRange == hb.textRange);
                    match &= (icu.typeface->uniqueID() == hb.typeface->uniqueID());
                }
            }
            if (!match) {
                SkDebugf("Did't match!\n\n");
            } else {
                SkDebugf("\n");
            }

            auto draw = [&](FontResolvedText* fontResolvedText) -> std::unique_ptr<DrawableText> {
                auto shapedText = fontResolvedText->shape(&unicodeText, TextDirection::kLtr);
                auto runs = shapedText->getLogicalRuns();
                fForeground.setColor(SK_ColorBLACK);
                size_t unresolved = 0;
                for (auto& run : runs) {
                    if (run.getRunType() != LogicalRunType::kText) {
                        continue;
                    }
                    for (auto i = 0; i < run.size(); ++i) {
                        if (run.getGlyph(i) == 0) {
                            ++unresolved;
                        }
                    }
                }

                if (unresolved) {
                    fForeground.setColor(SK_ColorRED);
                    SkDebugf(">>> unresolved: %d <<<\n", unresolved);
                }

                auto wrappedText =
                        shapedText->wrap(&unicodeText, SK_ScalarInfinity, SK_ScalarInfinity);
                wrappedText->format(TextAlign::kLeft, TextDirection::kLtr);
                DecoratedBlock decoratedBlock(
                        utf16.size(), fForeground, SkPaint(SkColors::kLtGray));
                return wrappedText->prepareToDraw(&unicodeText);
            };

            fIcuDrawableText = draw(icuResolver.get());
            fHarfbuzzDrawableText = draw(harfbuzzResolver.get());
            fRedraw = false;
        }
        for (auto& textBLob : fIcuDrawableText->getTextBlobs()) {
            canvas->drawTextBlob(textBLob, 0, this->height() / 2.0, SkPaint(SkColors::kBlue));
        }
        for (auto& textBLob : fHarfbuzzDrawableText->getTextBlobs()) {
            canvas->drawTextBlob(textBLob, this->width() / 2.0, this->height() / 2.0, SkPaint(SkColors::kGreen));
        }
    }
private:
    bool fRedraw = true;
    bool fRepeat = false;
    size_t fIndex = 0;
    size_t fLimit = 500;
    std::unique_ptr<DrawableText> fIcuDrawableText;
    std::unique_ptr<DrawableText> fHarfbuzzDrawableText;
    sk_sp<MultipleFontChain> fFontChain;
    SkPaint fForeground;
    using INHERITED = Sample;
};

class TextSample_Chrome_CombiningCharacterQueries : public Sample {

public:
    TextSample_Chrome_CombiningCharacterQueries() : fUnicode(SkUnicode::Make()) { }
protected:
    SkString name() override { return SkString("TextSample_Chrome_CombiningCharacterQueries"); }

    void drawLine(SkCanvas* canvas, SkScalar x1, SkScalar x2,
                  const std::u16string& utf16,
                  const char* fontFamily,
                  TextDirection textDirection) {

        UnicodeText unicodeText(SkUnicode::Make(),
                                SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
        if (!unicodeText.getUnicode()) return;
        auto utf8 = unicodeText.getUnicode()->convertUtf16ToUtf8(utf16);
        SkDebugf(">>> %s <<<\n", utf8.c_str());
        auto fontChain = sk_sp<MultipleFontChain>(new MultipleFontChain({ fontFamily, "serif", "Noto Naskh Arabic"}, 40.0f, SkFontStyle::Normal()));
        FontBlock fontBlock(utf16.size(), fontChain);

        auto icuResolver =
                unicodeText.resolveFonts<IcuFontResolvedText>(SkSpan<FontBlock>(&fontBlock, 1),
                                                              textDirection,
                                                              std::locale().name().c_str());
        auto icuResolvedFonts = icuResolver->resolvedFonts();

        auto harfbuzzResolver = unicodeText.resolveFonts<HarfbuzzFontResolvedText>(
                SkSpan<FontBlock>(&fontBlock, 1),
                textDirection,
                std::locale().name().c_str());
        auto harfbuzzResolvedFonts = harfbuzzResolver->resolvedFonts();
        bool match = true;
        match &= (icuResolvedFonts.size() == harfbuzzResolvedFonts.size());
        if (match) {
            for (auto i = 0ul; i < icuResolvedFonts.size(); ++i) {
                auto& icu = icuResolvedFonts[i];
                auto& hb = harfbuzzResolvedFonts[i];
                match &= (icu.textRange == hb.textRange);
                match &= (icu.typeface->uniqueID() == hb.typeface->uniqueID());
            }
        }
        if (!match) {
            SkDebugf("Did't match!\n\n");
        } else {
            SkDebugf("\n");
        }

        auto draw = [&](FontResolvedText* fontResolvedText) -> std::unique_ptr<DrawableText> {
            SkPaint foregroundPaint(SkColors::kBlack);
            SkPaint backgroundPaint(SkColors::kLtGray);

            auto shapedText = fontResolvedText->shape(&unicodeText, textDirection);
            auto runs = shapedText->getLogicalRuns();
            foregroundPaint.setColor(SK_ColorBLACK);
            size_t unresolved = 0;
            for (auto& run : runs) {
                if (run.getRunType() != LogicalRunType::kText) {
                    continue;
                }
                for (auto i = 0; i < run.size(); ++i) {
                    if (run.getGlyph(i) == 0) {
                        ++unresolved;
                    }
                }
            }

            if (unresolved) {
                foregroundPaint.setColor(SK_ColorRED);
                SkDebugf(">>> unresolved: %d <<<\n", unresolved);
            }

            auto wrappedText = shapedText->wrap(&unicodeText, SK_ScalarInfinity, SK_ScalarInfinity);
            wrappedText->format(TextAlign::kLeft, TextDirection::kLtr);
            DecoratedBlock decoratedBlock(
                    utf16.size(), foregroundPaint, SkPaint(SkColors::kLtGray));
            return wrappedText->prepareToDraw(&unicodeText);
        };

        auto icuDrawableText = draw(icuResolver.get());
        auto harfbuzzDrawableText = draw(harfbuzzResolver.get());

        for (auto& textBLob : icuDrawableText->getTextBlobs()) {
            canvas->drawTextBlob(textBLob, x1, 0, SkPaint(SkColors::kBlue));
        }
        for (auto& textBLob : harfbuzzDrawableText->getTextBlobs()) {
            canvas->drawTextBlob(textBLob, x2, 0, SkPaint(SkColors::kGreen));
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->drawColor(SK_ColorWHITE);

        SkScalar width = this->width() / 6;
        SkScalar height = this->height() / 6;
        canvas->translate(0, height);
        Paint::drawText(u"Multiple combining diacritics in LTR:", canvas, 0, 0);
        drawLine(canvas, width * 3, width * 3.5, u"ab\u030cc\u030c\u030c", "cursive", TextDirection::kLtr);
        drawLine(canvas, width * 4, width * 4.5, u"ab\u030cc\u030c\u030c", "serif", TextDirection::kLtr);
        drawLine(canvas, width * 5, width * 5.5, u"ab\u030cc\u030c\u030c", "sans-serif", TextDirection::kLtr);

        canvas->translate(0, height);
        Paint::drawText(u"Multiple combining diacritics in RTL:", canvas, 0, 0);
        drawLine(canvas, width * 3, width * 3.5, u"\u200Fa\u200Fb\u030c\u200Fc\u030c\u030c", "cursive", TextDirection::kRtl);
        drawLine(canvas, width * 4, width * 4.5, u"\u200Fa\u200Fb\u030c\u200Fc\u030c\u030c", "serif", TextDirection::kRtl);
        drawLine(canvas, width * 5, width * 5.5, u"\u200Fa\u200Fb\u030c\u200Fc\u030c\u030c", "sans-serif", TextDirection::kRtl);

        canvas->translate(0, height);
        Paint::drawText(u"Multiple combining diacritics on ligatures in LTR:", canvas, 0, 0);
        drawLine(canvas, width * 3, width * 3.5, u"fi\u030c ffi\u030c\u030c", "cursive", TextDirection::kLtr);
        drawLine(canvas, width * 4, width * 4.5, u"fi\u030c ffi\u030c\u030c", "serif", TextDirection::kLtr);
        drawLine(canvas, width * 5, width * 5.5, u"fi\u030c ffi\u030c\u030c", "sans-serif", TextDirection::kLtr);

        canvas->translate(0, height);
        Paint::drawText(u"Combining diacritics on ligatures in mixed bidi runs:", canvas, 0, 0);
        drawLine(canvas, width * 3, width * 3.5, u"ff\u030C\u0640\u0640\u0644\u0627ff\u030C", "cursive", TextDirection::kLtr);
        drawLine(canvas, width * 4, width * 4.5, u"ff\u030C\u0640\u0640\u0644\u0627ff\u030C", "serif", TextDirection::kLtr);
        drawLine(canvas, width * 5, width * 5.5, u"ff\u030C\u0640\u0640\u0644\u0627ff\u030C", "sans-serif", TextDirection::kLtr);

        canvas->translate(0, height);
        Paint::drawText(u"Combining diacritics which are wider than characters in mixed bidi runs:", canvas, 0, 0);
        drawLine(canvas, width * 3, width * 3.5, u" \u030C\uFDB0i\u0333", "cursive", TextDirection::kLtr);
        drawLine(canvas, width * 4, width * 4.5, u".\u030C\uFDB0i\u0333", "serif", TextDirection::kLtr);
        drawLine(canvas, width * 5, width * 5.5, u".\u030C\uFDB0i\u0333", "sans-serif", TextDirection::kLtr);
    }

private:
    using INHERITED = Sample;
    std::unique_ptr<SkUnicode> fUnicode;
};
}  // namespace

DEF_SAMPLE(return new TextSample_HelloWorld();)
DEF_SAMPLE(return new TextSample_Align_Dir();)
DEF_SAMPLE(return new TextSample_LongLTR();)
DEF_SAMPLE(return new TextSample_LongRTL();)
DEF_SAMPLE(return new TextSample_Zalgo();)
DEF_SAMPLE(return new TextSample_Chrome_CombiningCharacterQueries();)
