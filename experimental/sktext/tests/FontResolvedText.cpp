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

#include <include/core/SkTime.h>
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

namespace {
sk_sp<SkTypeface> addFontFromFile(const std::string& resourceDir, const std::string& path) {
    SkString file_path;
    file_path.printf("%s/%s", resourceDir.c_str(), path.c_str());

    auto data = SkData::MakeFromFileName(file_path.c_str());
    if (!data) {
        return nullptr;
    }

    return SkTypeface::MakeFromData(data);
}
}

DEF_TEST(SkText_FontResolution1, reporter) {
    TrivialFontChain* fontChain = new TrivialFontChain("Roboto", 40.0f, SkFontStyle::Normal());
    if (fontChain->empty()) return;

    std::u16string utf16(u"Hello world");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    if (!unicodeText.getUnicode()) return;

    FontBlock fontBlock(utf16.size(), sk_ref_sp<FontChain>(fontChain));
    auto fontResolvedText = unicodeText.resolveFonts<IcuFontResolvedText>(SkSpan<FontBlock>(&fontBlock, 1), TextDirection::kLtr, std::locale().name().c_str());

    auto resolvedFonts = fontResolvedText->resolvedFonts();

    REPORTER_ASSERT(reporter, resolvedFonts.size() == 1);
    REPORTER_ASSERT(reporter, resolvedFonts.front().textRange.width() == utf16.size());
    REPORTER_ASSERT(reporter, resolvedFonts.front().typeface->uniqueID() == fontChain->getTypeface()->uniqueID());
    REPORTER_ASSERT(reporter, resolvedFonts.front().size == 40.0f);
    REPORTER_ASSERT(reporter, resolvedFonts.front().style == SkFontStyle::Normal());
}

DEF_TEST(SkText_FontResolution2, reporter) {
    MultipleFontChain* fontChain = new MultipleFontChain({ "Roboto", "Noto Color Emoji", "Noto Serif CJK JP" }, 40.0f, SkFontStyle::Normal());
    if (fontChain->count() < 3) return;

    std::u16string utf16(u"English English 字典 字典 😀😃😄 😀😃😄");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    if (!unicodeText.getUnicode()) return;

    FontBlock fontBlock(utf16.size(), sk_ref_sp<FontChain>(fontChain));
    auto fontResolvedText = unicodeText.resolveFonts<IcuFontResolvedText>(SkSpan<FontBlock>(&fontBlock, 1), TextDirection::kLtr, std::locale().name().c_str());

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

DEF_TEST(SkText_FontResolution_Compose, reporter) {
    
    UnicodeText unicodeText(SkUnicode::Make(), SkString(""));
    SkUnichar a, b, ab;
  
    auto compose = [&](SkUnichar a, SkUnichar b, SkUnichar* result) -> bool {
        SkUnichar ab[2] = { a, b };
        return unicodeText.composeCodepoints(SkSpan<SkUnichar>(&ab[0], 2), result);
    };
                            
    /* Not composable */
    REPORTER_ASSERT(reporter, !compose( 0x0041, 0x0042, &ab));
    REPORTER_ASSERT(reporter, !compose( 0x0041, 0, &ab));
    REPORTER_ASSERT(reporter, !compose( 0x0066, 0x0069, &ab));

    /* Singletons should not compose */
    REPORTER_ASSERT(reporter, !compose( 0x212B, 0, &ab) && ab == 0);
    REPORTER_ASSERT(reporter, !compose( 0x00C5, 0, &ab) && ab == 0);
    REPORTER_ASSERT(reporter, !compose( 0x2126, 0, &ab) && ab == 0);
    REPORTER_ASSERT(reporter, !compose( 0x03A9, 0, &ab) && ab == 0);

    /* Non-starter pairs should not compose */
    REPORTER_ASSERT(reporter, !compose( 0x0308, 0x0301, &ab)); /* !0x0344 */
    REPORTER_ASSERT(reporter, !compose( 0x0F71, 0x0F72, &ab)); /* !0x0F73 */

    /* Pairs */
    REPORTER_ASSERT(reporter, compose( 0x0041, 0x030A, &ab) && ab == 0x00C5);
    REPORTER_ASSERT(reporter, compose( 0x006F, 0x0302, &ab) && ab == 0x00F4);
    REPORTER_ASSERT(reporter, compose( 0x1E63, 0x0307, &ab) && ab == 0x1E69);
    REPORTER_ASSERT(reporter, compose( 0x0073, 0x0323, &ab) && ab == 0x1E63);
    REPORTER_ASSERT(reporter, compose( 0x0064, 0x0307, &ab) && ab == 0x1E0B);
    REPORTER_ASSERT(reporter, compose( 0x0064, 0x0323, &ab) && ab == 0x1E0D);

    /* Hangul */
    REPORTER_ASSERT(reporter, compose( 0xD4CC, 0x11B6, &ab) && ab == 0xD4DB);
    REPORTER_ASSERT(reporter, compose( 0x1111, 0x1171, &ab) && ab == 0xD4CC);
    REPORTER_ASSERT(reporter, compose( 0xCE20, 0x11B8, &ab) && ab == 0xCE31);
    REPORTER_ASSERT(reporter, compose( 0x110E, 0x1173, &ab) && ab == 0xCE20);

    REPORTER_ASSERT(reporter, !compose( 0xAC00, 0x11A7, &ab));
    REPORTER_ASSERT(reporter, compose( 0xAC00, 0x11A8, &ab) && ab == 0xAC01);
    REPORTER_ASSERT(reporter, !compose( 0xAC01, 0x11A8, &ab));
}

DEF_TEST(SkText_FontResolution_Decompose, reporter) {
    UnicodeText unicodeText(SkUnicode::Make(), SkString(""));
    SkUnichar a, b, ab;

    /* Not decomposable */
    REPORTER_ASSERT(reporter, !unicodeText.decomposeCodepoints(0x0041, &a, &b));
    REPORTER_ASSERT(reporter, !unicodeText.decomposeCodepoints(0xFB01, &a, &b));
    REPORTER_ASSERT(reporter, !unicodeText.decomposeCodepoints(0x1F1EF, &a, &b));

    /* Singletons */
    REPORTER_ASSERT(reporter, unicodeText.decomposeCodepoints(0x212B, &a, &b) && a == 0x00C5 && b == 0);
    REPORTER_ASSERT(reporter, unicodeText.decomposeCodepoints(0x2126, &a, &b) && a == 0x03A9 && b == 0);

    /* Non-starter pairs decompose, but not compose */
    REPORTER_ASSERT(reporter, unicodeText.decomposeCodepoints(0x0344, &a, &b) && a == 0x0308 && b == 0x0301);
    REPORTER_ASSERT(reporter, unicodeText.decomposeCodepoints(0x0F73, &a, &b) && a == 0x0F71 && b == 0x0F72);

    /* Pairs */
    REPORTER_ASSERT(reporter, unicodeText.decomposeCodepoints(0x00C5, &a, &b) && a == 0x0041 && b == 0x030A);
    REPORTER_ASSERT(reporter, unicodeText.decomposeCodepoints(0x00F4, &a, &b) && a == 0x006F && b == 0x0302);
    REPORTER_ASSERT(reporter, unicodeText.decomposeCodepoints(0x1E69, &a, &b) && a == 0x1E63 && b == 0x0307);
    REPORTER_ASSERT(reporter, unicodeText.decomposeCodepoints(0x1E63, &a, &b) && a == 0x0073 && b == 0x0323);
    REPORTER_ASSERT(reporter, unicodeText.decomposeCodepoints(0x1E0B, &a, &b) && a == 0x0064 && b == 0x0307);
    REPORTER_ASSERT(reporter, unicodeText.decomposeCodepoints(0x1E0D, &a, &b) && a == 0x0064 && b == 0x0323);

    /* Hangul */
    REPORTER_ASSERT(reporter, unicodeText.decomposeCodepoints(0xD4DB, &a, &b) && a == 0xD4CC && b == 0x11B6);
    REPORTER_ASSERT(reporter, unicodeText.decomposeCodepoints(0xD4CC, &a, &b) && a == 0x1111 && b == 0x1171);
    REPORTER_ASSERT(reporter, unicodeText.decomposeCodepoints(0xCE31, &a, &b) && a == 0xCE20 && b == 0x11B8);
    REPORTER_ASSERT(reporter, unicodeText.decomposeCodepoints(0xCE20, &a, &b) && a == 0x110E && b == 0x1173);
}

DEF_TEST(SkText_FontResolution_Normalize, reporter) {

    auto abc = addFontFromFile(GetResourcePath("fonts").c_str(), "abc/abc.ttf");
    if (abc == nullptr) return;

    auto abcGrave = addFontFromFile(GetResourcePath("fonts").c_str(), "abc/abc+grave.ttf");
    if (abcGrave == nullptr) return;

    auto abcAGrave = addFontFromFile(GetResourcePath("fonts").c_str(), "abc/abc+agrave.ttf");
    if (abcAGrave == nullptr) return;

    auto fontChain1 = sk_sp<MultipleFontChain>(new MultipleFontChain({ abc, abcGrave }, 40.0f, SkFontStyle::Normal()));
    auto fontChain2 = sk_sp<MultipleFontChain>(new MultipleFontChain({ abc, abcAGrave }, 40.0f, SkFontStyle::Normal()));

    std::u16string utf16(u"a\u0300à");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    if (!unicodeText.getUnicode()) return;

    auto test = [&](FontChain* fontChain, size_t glyphs) {
        FontBlock fontBlock(utf16.size(), sk_ref_sp<FontChain>(fontChain));
        auto fontResolvedText = unicodeText.resolveFonts<IcuFontResolvedText>(SkSpan<FontBlock>(&fontBlock, 1), TextDirection::kLtr, std::locale().name().c_str());

        auto resolvedFonts = fontResolvedText->resolvedFonts();

        REPORTER_ASSERT(reporter, resolvedFonts.size() == 1);
        REPORTER_ASSERT(reporter, resolvedFonts[0].textRange == TextRange(0, 3));

        auto shapedText = fontResolvedText->shape(&unicodeText, TextDirection::kLtr);
        auto logicalRuns = shapedText->getLogicalRuns();
        REPORTER_ASSERT(reporter, logicalRuns.size() == 1);
        auto& run = logicalRuns[0];
        REPORTER_ASSERT(reporter, run.size() == glyphs * 2);
        for (auto i = 0; i < glyphs; ++i) {
            REPORTER_ASSERT(reporter, run.getGlyph(i) == run.getGlyph(i + glyphs));
            REPORTER_ASSERT(reporter, run.getGlyph(i) != 0);
        }
    };

    // In both cases the text gets resolved with the second font
    // (first time we decompose it to resolve, second - compose)
    test(fontChain1.get(), 2);
    test(fontChain2.get(), 1);
}

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
    // TODO: Only add combining marks after base characters
    std::u16string zalgo(std::u16string victim, int limit = -1) {
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

DEF_TEST(SkText_FontResolution_Zalgo, reporter) {
    MultipleFontChain* fontChain = new MultipleFontChain({ "Noto Serif CJK JP", "Noto Color Emoji", "Roboto" }, 40.0f, SkFontStyle::Normal());
    if (fontChain->empty()) return;

    Zalgo zalgo;
    auto utf16 = zalgo.zalgo(u"English English 字典 字典 😀😃😄 😀😃😄");
    utf16 = u" S̑͛̋ͭ͂́͆͐̉҉͈̻̳͚͍̥̪͚̥̳̦̤͈͉͖̻̲͚̰̥̙͙̝̗̬̜̖̹̹ḵ̴̨̧̨̧̛̬̖̜͚̪̺͉̭̱̯̭͇̙͎̗͇͖̯̩͉̠̞͖͓͇̹̫̯̠̲̦̜̃ͧ̂͌͌̒̓ͪ̄ͪ͂ͧ̎ͭ̇̊̊̈́̅ͪ̋̆̊̉͐̈ͦ̓̇͌̃̍͒͗͌ͭ̀̀́͘͜͟͞ͅͅT̵̡̢̧̺̝̬̭̭̟̹͎̝͉̠̭̟͇̟̗̳͎͕̭͙̈́̂̋̀͊ͦͣͧͩ̒ͧ͊ͪ͗̅͛̉͌̇ͦ̒ͦ̍̊̌̀̇͗ͦ̓́͗ͫ̄̋̀̚̕͠ͅͅẹ̸̶̵̷̷̶̷̢̡̛̳̝̲̫͎͚̙̳͉̱̜ͪͣ̑͂̐͛̕͠͡͡x̷̶̵̨̟̝̱̬̟̯̱͇̤̰̪͈̹̙͍̺͕͓͙̳̰̰̻̩͉̥ͥ͗́ͥͮ̑͆͆̂ͣ̉ͮ̒̓̎͐ͨ͛̌̃̇̿̄ͭͬ̓̄͒ͬ͂̂̓͐͂̉ͥͦ̔ͤ͗̓͋̂ͭ͟͢͠͝͞t̶̡̢̛̓͘͠҉̡͓͚̝͚̰͇̗̥̜͇͖͓̙̬̩͇͔͚̩̹̲̲̳̦̯ͅ";
    utf16 = u"English English 字典 字典 😀😃😄 😀😃😄";

    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    if (!unicodeText.getUnicode()) return;

    FontBlock fontBlock(utf16.size(), sk_ref_sp<FontChain>(fontChain));
    auto fontResolvedText = unicodeText.resolveFonts<IcuFontResolvedText>(SkSpan<FontBlock>(&fontBlock, 1), TextDirection::kLtr, std::locale().name().c_str());
    auto resolvedFonts = fontResolvedText->resolvedFonts();
    REPORTER_ASSERT(reporter, resolvedFonts.size() == 4);

    // However, zalgo text can have many scripts so it would be multiple runs
    auto shapedText = fontResolvedText->shape(&unicodeText, TextDirection::kLtr);
    auto logicalRuns = shapedText->getLogicalRuns();

    for (auto& run : logicalRuns) {
        for (auto i = 0; i < run.size(); ++i) {
            REPORTER_ASSERT(reporter, run.getGlyph(i) != 0);
        }
    }
}

DEF_TEST(SkText_FontResolution_ZalgoComparison, reporter) {

    TrivialFontChain* fontChain = new TrivialFontChain("Roboto", 40.0f, SkFontStyle::Normal());
    if (fontChain->empty()) return;
    double icuTime = 0.0;
    double hbTime = 0.0;
    double shapingTime = 0.0;
    for (auto count = 0ul; count < 1000; ++count) {
        Zalgo zalgo;
        auto utf16 = zalgo.zalgo(u"SkText");
        //utf16 = u"Short text";
        //utf16 = u"S̸̢̢̄͋́̀̕͟͞͠҉̴̡͜͜҉̡̙̥̰̣͔̬͍͙̻̘̝̞͙̖͇̰̹̤̤͓͎̣̜̙͕͈̮̱͞͠͠ḱ̷̡̉ͩ͏̲̰̦̬̭̤̝̠͚̺͎̟͓̮͍͎̲͔̱̙̜͎͚͈͎͙T̶͌̇͒ͧͫͥ̀̉̈̋̑̈̈́̂̒̐̆ͩͩ̏ͣͭ̆̊̀̚͘͡҉̴̸̨̨̡̢̝̭͉̘̲̲͓̦̜̗̫̫͇̪̰̠̯̫̰̜̼̯͍̲͓̣̮̮̟̳͖͙̺̮̦̥́͝͞ͅͅę̷̷̛̐͌ͬ̂͐̽̎̐́̽ͥ̔͑̓̀ͬ͑͋̎ͦ̏̓ͥͫͫ͗͊̄̃̇͆̏̔͒͊̌ͪͤ͛͊͊̉̓ͮ͞͞͏̷͘͏̨̢̣̜̙̲̖̗̳̮̣̳̰͙͍͖̝x̴̶̶̸̡̨̡̢̨̀ͭ̐̔̅ͣͨ̽̆͑͂̃͒ͩ͌͐͌̔̑̇ͪ̇̃͐͗̓̃ͪͮ̄̐̃̊͂̅̈́̓͗̓̂ͥ͛ͫͭͦ̊ͥ̀̕͢͠͡҉͔̰͉̘͇̝̹̟̰̤͕̟̫͉̟̻̫̦̝͓̙̖ţ̧̤͙̦̖̗͉̹̙̜͙͚̭̝̫̯̱̬̲͑̽̎̔͊̓̆̓̀͊͂̄̂̇̎ͮ̆̋̆̍ͧ̓̄̅ͪ͐ͦ̏͐ͣͨ̆̅ͦͩ̉ͬ͐̀̏ͧ̋̍̍́́̚͟͠͞";

        UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
        if (!unicodeText.getUnicode()) return;

        SkString utf8 = unicodeText.getUnicode()->convertUtf16ToUtf8(utf16);
        auto delta = hbTime - icuTime;
        //SkDebugf(">>> %s <<< %g - %g = %g %g %g\n", utf8.c_str(), hbTime * 1e-9, icuTime * 1e-9, delta * 1e-9, shapingTime * 1e-9, delta/(shapingTime) );

        FontBlock fontBlock(utf16.size(), sk_ref_sp<FontChain>(fontChain));
        auto icuStart = SkTime::GetNSecs();
        auto icuResolver = unicodeText.resolveFonts<IcuFontResolvedText>(SkSpan<FontBlock>(&fontBlock, 1), TextDirection::kLtr, std::locale().name().c_str());
        icuTime += (SkTime::GetNSecs() - icuStart);
        auto icuResolvedFonts = icuResolver->resolvedFonts();

        auto hbStart = SkTime::GetNSecs();
        auto harfbuzzResolver = unicodeText.resolveFonts<HarfbuzzFontResolvedText>(SkSpan<FontBlock>(&fontBlock, 1), TextDirection::kLtr, std::locale().name().c_str());
        hbTime += (SkTime::GetNSecs() - hbStart);
        auto harfbuzzResolvedFonts = harfbuzzResolver->resolvedFonts();

        REPORTER_ASSERT(reporter, icuResolvedFonts.size() == harfbuzzResolvedFonts.size());
        for (auto i = 0ul; i < icuResolvedFonts.size(); ++i) {
            auto& icu = icuResolvedFonts[i];
            auto& hb = harfbuzzResolvedFonts[i];
            REPORTER_ASSERT(reporter, icu.textRange == hb.textRange);
            REPORTER_ASSERT(reporter, icu.typeface->uniqueID() == hb.typeface->uniqueID());
        }
        auto shapingStart = SkTime::GetNSecs();
        auto shapedText = harfbuzzResolver->shape(&unicodeText, TextDirection::kLtr);
        shapingTime += (SkTime::GetNSecs() - shapingStart);
    }
}
