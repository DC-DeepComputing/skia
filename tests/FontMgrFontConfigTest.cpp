/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkPngEncoder.h"
#include "include/ports/SkFontMgr_fontconfig.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <fontconfig/fontconfig.h>

#include <include/core/SkFontMetrics.h>
#include <modules/skshaper/include/SkShaper.h>
#include <modules/skshaper/include/SkShaper_harfbuzz.h>
#include <modules/skshaper/include/SkShaper_skunicode.h>
#include <modules/skunicode/include/SkUnicode.h>
#include <modules/skunicode/include/SkUnicode_icu.h>
#include <src/core/SkFontScanner.h>
#include <src/ports/SkTypeface_FreeType.h>
#include <array>
#include <memory>

namespace {

bool bitmap_compare(const SkBitmap& ref, const SkBitmap& test) {
    auto count = 0;
    for (int y = 0; y < test.height(); ++y) {
        for (int x = 0; x < test.width(); ++x) {
            SkColor testColor = test.getColor(x, y);
            SkColor refColor = ref.getColor(x, y);
            if (refColor != testColor) {
                ++count;
                SkDebugf("%d: (%d,%d) ", count, x, y);
            }
        }
    }
    return (count == 0);
}

FcConfig* build_fontconfig_with_fontfile(const char* fontFilename) {
    FcConfig* config = FcConfigCreate();

    // FontConfig may modify the passed path (make absolute or other).
    FcConfigSetSysRoot(config, reinterpret_cast<const FcChar8*>(GetResourcePath("").c_str()));
    // FontConfig will lexically compare paths against its version of the sysroot.
    SkString fontFilePath(reinterpret_cast<const char*>(FcConfigGetSysRoot(config)));
    fontFilePath += fontFilename;
    FcConfigAppFontAddFile(config, reinterpret_cast<const FcChar8*>(fontFilePath.c_str()));

    FcConfigBuildFonts(config);
    return config;
}

FcConfig* build_fontconfig_from_resources() {

    SkString path = GetResourcePath("");
    SkString content;
    content.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                   "<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">"
                   "<fontconfig>\n");
    //content.appendf("  <cachedir>/fonts</cachedir\n>");
    content.appendf("  <dir>/fonts</dir>\n");
/*
    content.appendf("    <match target=\"font\">\n"
                          "        <edit name=\"embolden\" mode=\"assign\">\n"
                          "          <bool>true</bool>\n"
                          "        </edit>\n"
                          "    </match>");
*/
    content.appendf("  <match target=\"font\">\n"
                   "    <!-- check to see if the font is just regular -->\n"
                   "    <test name=\"weight\" compare=\"less_eq\">\n"
                   "      <const>medium</const>\n"
                   "    </test>\n"
                   "    <!-- check to see if the pattern requests bold -->\n"
                   "    <test target=\"pattern\" name=\"weight\" compare=\"more\">\n"
                   "      <const>medium</const>\n"
                   "    </test>\n"
                   "    <!--\n"
                   "      set the embolden flag\n"
                   "      needed for applications using cairo, e.g. gucharmap, gedit, ...\n"
                   "    -->\n"
                   "    <edit name=\"embolden\" mode=\"assign\">\n"
                   "      <bool>true</bool>\n"
                   "    </edit>\n"
                   "    <!--\n"
                   "     set weight to bold\n"
                   "     needed for applications using Xft directly, e.g. Firefox, ...\n"
                   "    -->\n"
                   "    <edit name=\"weight\" mode=\"assign\">\n"
                   "      <const>bold</const>\n"
                   "    </edit>\n"
                   "  </match>");

    content.append("</fontconfig>\n");
    FcConfig* fc_config = FcConfigCreate();
    //FcConfigSetSysRoot(fc_config, reinterpret_cast<const FcChar8*>(path.c_str()));
    if (FcConfigParseAndLoadFromMemory(
                fc_config, reinterpret_cast<const FcChar8*>(content.c_str()),
                FcTrue) != FcTrue) {
        SkDebugf("FcConfigParseAndLoadFromMemory\n");
    }
    auto fonts_dir = "/usr/local/google/home/jlavrova/Sources/skia/resources/fonts/";
    if (FcConfigAppFontAddDir(fc_config, reinterpret_cast<const FcChar8*>(fonts_dir)) != FcTrue) {
        SkDebugf("!FcConfigAppFontAddDir\n");
    }
    if (FcConfigBuildFonts(fc_config) != FcTrue) {
        SkDebugf("!FcConfigBuildFonts\n");
    }
    return fc_config;
}

FcConfig* build_somehow(std::vector<SkString> fonts_dirs) {
    FcConfig* const config = FcConfigCreate();
    FcConfigSetSysRoot(config, reinterpret_cast<const FcChar8*>("/build/work/515ad29fb4887ef6079911df2424e500a895/google3/runfiles/google3/third_party/googlefonts/api/ofl"));
    for (auto fonts_dir : fonts_dirs) {
        if (!FcConfigAppFontAddDir(config, reinterpret_cast<const FcChar8*>(fonts_dir.c_str()))) {
            SkDebugf("FcConfigAppFontAddDir: %s\n", fonts_dir.c_str());
            return nullptr;
        }
    }
    if (!FcConfigBuildFonts(config)) {
        SkDebugf("FcConfigBuildFonts\n");
        return nullptr;
    }
    return config;
}

[[maybe_unused]]
static void write_bitmap(const SkBitmap* bm, const char fileName[]) {
    SkFILEWStream file(fileName);
    SkAssertResult(file.isValid());
    SkAssertResult(SkPngEncoder::Encode(&file, bm->pixmap(), {}));
}

}  // namespace

DEF_TEST(FontMgrFontConfig, reporter) {
    FcConfig* config = build_fontconfig_with_fontfile("/fonts/Distortable.ttf");

    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_FontConfig(config));
    sk_sp<SkTypeface> typeface(fontMgr->legacyMakeTypeface("Distortable", SkFontStyle()));
    if (!typeface) {
        ERRORF(reporter, "Could not find typeface. FcVersion: %d", FcGetVersion());
        return;
    }

    SkBitmap bitmapStream;
    bitmapStream.allocN32Pixels(64, 64);
    SkCanvas canvasStream(bitmapStream);
    canvasStream.drawColor(SK_ColorWHITE);

    SkBitmap bitmapClone;
    bitmapClone.allocN32Pixels(64, 64);
    SkCanvas canvasClone(bitmapClone);
    canvasStream.drawColor(SK_ColorWHITE);

    SkPaint paint;
    paint.setColor(SK_ColorGRAY);

    constexpr float kTextSize = 20;

    std::unique_ptr<SkStreamAsset> distortableStream(
        GetResourceAsStream("fonts/Distortable.ttf"));
    if (!distortableStream) {
        return;
    }

    SkPoint point = SkPoint::Make(20.0f, 20.0f);
    SkFourByteTag tag = SkSetFourByteTag('w', 'g', 'h', 't');

    for (int i = 0; i < 10; ++i) {
        SkScalar styleValue =
            SkDoubleToScalar(0.5 + i * ((2.0 - 0.5) / 10));
        SkFontArguments::VariationPosition::Coordinate
            coordinates[] = {{tag, styleValue}};
        SkFontArguments::VariationPosition
            position = {coordinates, std::size(coordinates)};

        SkFont fontStream(
            fontMgr->makeFromStream(distortableStream->duplicate(),
                                    SkFontArguments().setVariationDesignPosition(position)),
            kTextSize);
        fontStream.setEdging(SkFont::Edging::kSubpixelAntiAlias);

        SkFont fontClone(
            typeface->makeClone(SkFontArguments().setVariationDesignPosition(position)), kTextSize);
        fontClone.setEdging(SkFont::Edging::kSubpixelAntiAlias);

        constexpr char text[] = "abc";

        canvasStream.drawColor(SK_ColorWHITE);
        canvasStream.drawString(text, point.fX, point.fY, fontStream, paint);

        canvasClone.drawColor(SK_ColorWHITE);
        canvasClone.drawString(text, point.fX, point.fY, fontClone, paint);

        bool success = bitmap_compare(bitmapStream, bitmapClone);
        REPORTER_ASSERT(reporter, success);
    }
}

UNIX_ONLY_TEST(FontMgrFontConfig_AllBold, reporter) {

    FcConfig* config = build_fontconfig_from_resources();
    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_FontConfig(config));

    constexpr float kTextSize = 20;
    constexpr char text[] = "abc"; // "מדהים" ;

    SkString filePath = GetResourcePath("fonts/NotoSans[wdth,wght].ttf");// "fonts/NotoSansHebrew[wdth,wght].ttf");
    sk_sp<SkTypeface> dataTypeface(fontMgr->makeFromFile(filePath.c_str(), 0));
    if (!dataTypeface) {
        ERRORF(reporter, "Could not find data typeface. FcVersion: %d", FcGetVersion());
        return;
    }

    SkFont dataFont(dataTypeface, kTextSize);
    dataFont.setEmbolden(true);

    sk_sp<SkTypeface> matchTypeface(fontMgr->matchFamilyStyle("Noto Sans", SkFontStyle()));
    if (!matchTypeface) {
        ERRORF(reporter, "Could not find match typeface. FcVersion: %d", FcGetVersion());
        return;
    }
    SkFont matchFont(matchTypeface, kTextSize);

    SkBitmap bitmapData;
    bitmapData.allocN32Pixels(64, 64);
    SkCanvas canvasData(bitmapData);

    SkBitmap bitmapMatch;
    bitmapMatch.allocN32Pixels(64, 64);
    SkCanvas canvasMatch(bitmapMatch);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);

    canvasData.drawColor(SK_ColorGRAY);
    canvasData.drawString(text, 20.0f, 20.0f, dataFont, paint);
    if ((true)) {
        // In case we wonder what's been painted
        SkString dataPath = GetResourcePath("/fonts/data.png");
        write_bitmap(&bitmapData, dataPath.c_str());
    }

    canvasMatch.drawColor(SK_ColorGRAY);
    canvasMatch.drawString(text, 20.0f, 20.0f, matchFont, paint);
    if ((true)) {
        SkString matchPath = GetResourcePath("/fonts/match.png");
        write_bitmap(&bitmapMatch, matchPath.c_str());
    }

    bool success = bitmap_compare(bitmapData, bitmapMatch);
    REPORTER_ASSERT(reporter, success);
}

class CustomTextBlobBuilderRunHandler : public SkShaper::RunHandler {
public:
    CustomTextBlobBuilderRunHandler(const char* utf8_text, SkPoint offset)
            : text_blob_run_handler_(utf8_text, offset) {}

    void beginLine() override {
        min_run_ascent_ = std::numeric_limits<float>::max();
        max_run_descent_ = std::numeric_limits<float>::min();
        total_run_advance_ = 0;
        text_blob_run_handler_.beginLine();
    }

    void runInfo(const RunInfo& info) override {
        text_blob_run_handler_.runInfo(info);

        SkFontMetrics metrics;
        info.fFont.getMetrics(&metrics);

        min_run_ascent_ = std::min(min_run_ascent_, metrics.fAscent);
        max_run_descent_ = std::max(max_run_descent_, metrics.fDescent);
        total_run_advance_ += info.fAdvance.x();
    }

    void commitRunInfo() override { text_blob_run_handler_.commitRunInfo(); }

    Buffer runBuffer(const RunInfo& info) override {
        return text_blob_run_handler_.runBuffer(info);
    }

    void commitRunBuffer(const RunInfo& info) override {
        SkString ff;
        info.fFont.getTypeface()->getFamilyName(&ff);
        SkDebugf("%d:%d - %s\n", info.utf8Range.begin(), info.utf8Range.end(), ff.c_str());
        return text_blob_run_handler_.commitRunBuffer(info);
    }

    void commitLine() override { text_blob_run_handler_.commitLine(); }

    sk_sp<SkTextBlob> makeBlob() { return text_blob_run_handler_.makeBlob(); }

    SkScalar GetMinRunAscent() const { return min_run_ascent_; }
    SkScalar GetMaxRunDescent() const { return max_run_descent_; }
    SkScalar GetTotalRunAdvance() const { return total_run_advance_; }

private:
    SkScalar min_run_ascent_ = std::numeric_limits<float>::max();
    SkScalar max_run_descent_ = std::numeric_limits<float>::min();
    SkScalar total_run_advance_ = 0;
    SkTextBlobBuilderRunHandler text_blob_run_handler_;
};

UNIX_ONLY_TEST(FontMgrFontConfig_Google3Bug, reporter) {

    FcConfig* config = build_somehow({});

    sk_sp<SkFontMgr> fontMgr = SkFontMgr_New_FontConfig(FcConfigReference(config));
    if (fontMgr->countFamilies() == 0) {
        ERRORF(reporter, "No fonts found");
    }

    constexpr float kTextSize = 20;
    //SkString text("זה מדהים");
    SkString text("Google Lens זה מדהים");

    const char* fonts[] = {
            "Noto Sans JP", "Noto Sans SC", "Noto Sans TC", "Noto Naskh Arabic", "Noto Nastaliq Urdu", "Noto Serif Tibetan", "Noto Sans Thaana", "Noto Sans Myanmar"
    };

    const char* font = "Google Sans";
    //SkUnichar unichar(0x05DD);
    SkUnichar unichar('G');
    sk_sp<SkTypeface> matchTypeface(fontMgr->matchFamilyStyleCharacter(font, SkFontStyle(), nullptr, 0, unichar));
    //sk_sp<SkTypeface> matchTypeface(fontMgr->matchFamilyStyle(font, SkFontStyle()));
    if (!matchTypeface) {
        SkDebugf("Could not find match for typeface %s\n", font);
        return;
    }
    SkString found;
    matchTypeface->getFamilyName(&found);
    SkDebugf("Found match for typeface %s: %s\n", font, found.c_str());
    SkFont matchFont(matchTypeface, kTextSize);
    matchFont.setEmbolden(true);

    const char* utf8 = text.data();
    size_t utf8_bytes = text.size();

    const SkBidiIterator::Level default_level = SkBidiIterator::kLTR;

    std::unique_ptr<SkShaper::BiDiRunIterator> bidi =
            SkShapers::unicode::BidiRunIterator(SkUnicodes::ICU::Make(), utf8,
                                                utf8_bytes, default_level);
    std::unique_ptr<SkShaper::LanguageRunIterator> language =
            SkShaper::MakeStdLanguageRunIterator(utf8, utf8_bytes);
    std::unique_ptr<SkShaper::ScriptRunIterator> script =
            SkShapers::HB::ScriptRunIterator(utf8, utf8_bytes);

    std::unique_ptr<SkShaper::FontRunIterator> font_runs =
            SkShaper::MakeFontMgrRunIterator(utf8, utf8_bytes, matchFont, fontMgr);

    CustomTextBlobBuilderRunHandler handler(utf8, SkPoint::Make(0, 0));

    std::unique_ptr<SkShaper> shaper =
            SkShapers::HB::ShapeThenWrap(SkUnicodes::ICU::Make(), fontMgr);
    shaper->shape(utf8, utf8_bytes, *font_runs, *bidi, *script, *language,
                  nullptr, 0, SK_ScalarInfinity, &handler);

    auto text_blob = handler.makeBlob();

    SkBitmap bitmapMatch;
    bitmapMatch.allocN32Pixels(256, 256);
    SkCanvas canvasMatch(bitmapMatch);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);

    canvasMatch.drawColor(SK_ColorGRAY);
    canvasMatch.drawTextBlob(text_blob, 20.0f, 20.0f, paint);
    //canvasMatch.drawString(text, 20.0f, 20.0f, matchFont, paint);
    if ((true)) {
        SkString matchPath = GetResourcePath("/fonts/match.png");
        write_bitmap(&bitmapMatch, matchPath.c_str());
    }
}
