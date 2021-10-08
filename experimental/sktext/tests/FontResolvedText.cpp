// Copyright 2021 Google LLC.
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImageEncoder.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include "experimental/sktext/include/Text.h"
#include "experimental/sktext/src/Paint.h"

#include <string.h>
#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct GrContextOptions;

#define VeryLongCanvasWidth 1000000
#define TestCanvasWidth 1000
#define TestCanvasHeight 600

#if !defined(SK_BUILD_FOR_UNIX)
#undef DEF_TEST
#define DEF_TEST(name, reporter)                                                          \
    static void test_##name(skiatest::Reporter* reporter, const GrContextOptions&);       \
    skiatest::TestRegistry name##TestRegistry(skiatest::Test(#name, false, test_##name)); \
    void test_##name(skiatest::Reporter* reporter, const GrContextOptions&) {             \
        /* SkDebugf("Disabled:"#name "\n"); */                                            \
    }                                                                                     \
    void disabled_##name(skiatest::Reporter* reporter, const GrContextOptions&)
#endif

using namespace skia::text;

DEF_TEST(SkText_FontResolution1, reporter) {
    TrivialFontChain* fontChain = new TrivialFontChain("Roboto", 40.0f, SkFontStyle::Normal());
    if (fontChain->empty()) return;

    std::u16string utf16(u"Hello world");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    if (!unicodeText.getUnicode()) return;

    FontBlock fontBlock(utf16.size(), sk_ref_sp<FontChain>(fontChain));
    auto fontResolvedText = unicodeText.resolveFonts<HarfbuzzFontResolvedText>(SkSpan<FontBlock>(&fontBlock, 1), TextDirection::kLtr, std::locale().name().c_str());

    auto resolvedFonts = fontResolvedText->resolvedFonts();

    REPORTER_ASSERT(reporter, resolvedFonts.size() == 1);
    REPORTER_ASSERT(reporter, resolvedFonts.front().textRange.width() == utf16.size());
    REPORTER_ASSERT(reporter, resolvedFonts.front().typeface->uniqueID() == fontChain->getTypeface()->uniqueID());
    REPORTER_ASSERT(reporter, resolvedFonts.front().size == 40.0f);
    REPORTER_ASSERT(reporter, resolvedFonts.front().style == SkFontStyle::Normal());
}

DEF_TEST(SkText_FontResolution3, reporter) {
    MultipleFontChain* fontChain = new MultipleFontChain({ "Roboto", "Noto Color Emoji", "Noto Serif CJK JP" }, 40.0f, SkFontStyle::Normal());
    if (fontChain->count() < 3) return;

    std::u16string utf16(u"English English 字典 字典 😀😃😄 😀😃😄");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    if (!unicodeText.getUnicode()) return;

    FontBlock fontBlock(utf16.size(), sk_ref_sp<FontChain>(fontChain));
    auto fontResolvedText = unicodeText.resolveFonts<HarfbuzzFontResolvedText>(SkSpan<FontBlock>(&fontBlock, 1), TextDirection::kLtr, std::locale().name().c_str());

    auto resolvedFonts = fontResolvedText->resolvedFonts();

    TextIndex prev = 0;
    for (auto& rf : resolvedFonts) {
        REPORTER_ASSERT(reporter, prev == rf.textRange.fStart);
        REPORTER_ASSERT(reporter, rf.textRange.width() > 0.0f);
        prev = rf.textRange.fEnd;
    }

    REPORTER_ASSERT(reporter, resolvedFonts.size() == 8 /* 1English 3English spaces + 2Emoji + 2JP */);
    REPORTER_ASSERT(reporter, resolvedFonts[0].textRange.fStart == 0);
    REPORTER_ASSERT(reporter, resolvedFonts[7].textRange.fEnd == utf16.size());
    REPORTER_ASSERT(reporter, resolvedFonts[0].size == 40.0f);
    REPORTER_ASSERT(reporter, resolvedFonts[0].style == SkFontStyle::Normal());
}

class Zalgo {
    private:
    std::u16string COMBINING_DOWN = u"\u0316\u0317\u0318\u0319\u031c\u031d\u031e\u031f\u0320\u0324\u0325\u0326\u0329\u032a\u032b\u032c\u032d\u032e\u032f\u0330\u0331\u0332\u0333\u0339\u033a\u033b\u033c\u0345\u0347\u0348\u0349\u034d\u034e\u0353\u0354\u0355\u0356\u0359\u035a\u0323";
    std::u16string COMBINING_UP = u"\u030d\u030e\u0304\u0305\u033f\u0311\u0306\u0310\u0352\u0357\u0351\u0307\u0308\u030a\u0342\u0343\u0344\u034a\u034b\u034c\u0303\u0302\u030c\u0350\u0300\u0301\u030b\u030f\u0312\u0313\u0314\u033d\u0309\u0363\u0364\u0365\u0366\u0367\u0368\u0369\u036a\u036b\u036c\u036d\u036e\u035b\u0346\u031a";
    std::u16string COMBINING_MIDDLE = u"\u0315\u031b\u0340\u0341\u0358\u0321\u0322\u0327\u0328\u0334\u0335\u0336\u034f\u035c\u035d\u035e\u035f\u0360\u0362\u0338\u0337\u0361\u0489";
    std::u16string randomMarks(std::u16string& combiningMarks) {
        std::u16string result;
        auto num = std::rand() % (combiningMarks.size() / 1);
        for (size_t i = 0; i < num; ++i) {
            auto index = std::rand() % combiningMarks.size();
            result += combiningMarks[index];
        }
        return result;
    }
public:
    std::u16string zalgo(std::string victim) {
        std::u16string result;
        for (auto& c : victim) {
            result += c;
            result += randomMarks(COMBINING_UP);
            result += randomMarks(COMBINING_MIDDLE);
            result += randomMarks(COMBINING_DOWN);
        }
        return result;
    }
};

DEF_TEST(SkText_FontResolution_Zalgo, reporter) {
    TrivialFontChain* fontChain = new TrivialFontChain("Roboto", 40.0f, SkFontStyle::Normal());
    if (fontChain->empty()) return;
    Zalgo zalgo;
    auto utf16 = zalgo.zalgo("SkText");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    if (!unicodeText.getUnicode()) return;
    FontBlock fontBlock(utf16.size(), sk_ref_sp<FontChain>(fontChain));
    auto fontResolvedText = unicodeText.resolveFonts<HarfbuzzFontResolvedText>(SkSpan<FontBlock>(&fontBlock, 1), TextDirection::kLtr, std::locale().name().c_str());
    auto resolvedFonts = fontResolvedText->resolvedFonts();
    REPORTER_ASSERT(reporter, resolvedFonts.size() == 1);
    // However, zalgo text can have many scripts so it would be multiple runs
    auto shapedText = fontResolvedText->shape(&unicodeText, TextDirection::kLtr);
    auto logicalRuns = shapedText->getLogicalRuns();
    for (auto& run : logicalRuns) {
        for (auto i = 0; i < run.size(); ++i) {
            REPORTER_ASSERT(reporter, run.getGlyph(i) != 0);
        }
    }
}
