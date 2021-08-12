/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkTraceEvent.h"

#include "modules/svg/include/SkSVGDOM.h"

#include <filesystem>
#include <regex>
#include <string>
#include <vector>

#if !defined(OS_LINUX)
#include "include/ports/SkFontMgr_empty.h"
#endif

constexpr unsigned kPixelSize = 400;
constexpr float kFontSizeScale = 0.85f;

namespace {

sk_sp<SkTypeface> makeMemoryFont(const char* path) {
    sk_sp<SkData> font_data = SkData::MakeFromFileName(path);
    sk_sp<SkData> data_in_memory = SkData::MakeWithCopy(font_data->bytes(), font_data->size());

#if !defined(OS_LINUX)
    auto font_mgr = sk_sp<SkFontMgr>(SkFontMgr_New_Custom_Empty());
#else
    auto font_mgr = sk_sp<SkFontMgr>(SkFontMgr::RefDefault());
#endif
    std::unique_ptr<SkStreamAsset> font_memory_stream =
            std::make_unique<SkMemoryStream>(data_in_memory);

    return font_mgr->makeFromStream(std::move(font_memory_stream));
}

}

class EmojiFontBench : public Benchmark {
public:
  enum BenchMode { kSvg, kColrV1, kCbdt };

    EmojiFontBench(BenchMode mode) : fMode(mode) {}

    const char* onGetName() override {
        SkString benchname;
        switch (fMode) {
            case kColrV1:
                benchname = "colrv1";
                break;
            case kSvg:
                benchname = "svg";
                break;
            case kCbdt:
                benchname = "cbdt";
                break;
        }
        fName.printf("emojibench_%s", benchname.c_str());
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override { return backend == kRaster_Backend; }

    void onDelayedSetup() override {

        fNotoEmojiColrV1Typeface = makeMemoryFont("third_party/externals/color-fonts/fonts/noto-glyf_colr_1.ttf");
        fNotoEmojiCbdtTypeface = makeMemoryFont("third_party/externals/noto-emoji/fonts/NotoColorEmoji.ttf");

        SkASSERT(fNotoEmojiColrV1Typeface);
        SkASSERT(fNotoEmojiColrV1Typeface->countGlyphs());
        fColrV1EmojiFont = SkFont(fNotoEmojiColrV1Typeface, kPixelSize * kFontSizeScale);
        fCbdtEmojiFont = SkFont(fNotoEmojiCbdtTypeface, kPixelSize * kFontSizeScale);
        fCbdtEmojiFont.setEmbeddedBitmaps(true);

        std::string path = "third_party/externals/noto-emoji/svg/";
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            std::regex base_emoji_regex(".*emoji_u([0-9a-f]*).svg");
            std::string entry_path = entry.path().u8string();
            std::smatch emoji_codepoint_match;
            if (std::regex_match(entry_path, emoji_codepoint_match, base_emoji_regex)) {
                std::vector<std::string> exceptions = {"fe82b"};
                bool exception_found = false;
                for (auto& exception : exceptions) {
                    if (entry_path.find(exception) != std::string::npos) {
                        exception_found = true;
                        break;
                    }
                }
                if (exception_found) continue;

                fFileList.push_back(entry_path);
                std::string codepoint_string = emoji_codepoint_match[1].str();

                SkGlyphID glyphId = fNotoEmojiColrV1Typeface->unicharToGlyph(strtoul(codepoint_string.c_str(), nullptr, 16));
                SkASSERT(glyphId);

                SkGlyphID cbdtGlyphId = fNotoEmojiCbdtTypeface->unicharToGlyph(strtoul(codepoint_string.c_str(), nullptr, 16));
                if (!cbdtGlyphId) {
                  SkDebugf("Missing in cbdt: %d 0x%s\n", cbdtGlyphId, codepoint_string.c_str());
                }
                SkASSERT(cbdtGlyphId);

                if (glyphId && cbdtGlyphId) {
                  fColrV1GlyphIds.push_back(glyphId);
                  fCbdtGlyphIds.push_back(cbdtGlyphId);
                }
            }
        }

        SkASSERT(fFileList.size() == fColrV1GlyphIds.size());
        for (size_t i = 0; i < fFileList.size(); ++i) {
            sk_sp<SkData> svg_data = SkData::MakeFromFileName(fFileList[i].c_str());
            if (!svg_data) {
                printf("svg loading failed for %s\n", fFileList[i].c_str());
            }
            sk_sp<SkData> data_in_memory =
                    SkData::MakeWithCopy(svg_data->bytes(), svg_data->size());
            SkASSERT(data_in_memory->size());
            fSvgs.push_back(data_in_memory);
        }

        SkDebugf("Glyph count: %ld\n", fColrV1GlyphIds.size()); // 1369 SVG files matching the pattern.
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        switch (fMode) {
            case BenchMode::kSvg:
                onDrawSvg(loops, canvas);
                break;
            case BenchMode::kColrV1:
                onDrawColrV1(loops, canvas);
                break;
            case BenchMode::kCbdt:
                onDrawCbdt(loops, canvas);
                break;
        }
    }

    SkIPoint onGetSize() override { return {kPixelSize + 100, kPixelSize + 100}; }

    void onPreDraw(SkCanvas* canvas) override { canvas->clear(SK_ColorWHITE); }

private:
    void drawSvgGlyph(int index, SkCanvas* canvas) {
        TRACE_EVENT0("skia", TRACE_FUNC);
        SkMemoryStream svg_stream(fSvgs[index]);
        sk_sp<SkSVGDOM> svgDom = SkSVGDOM::MakeFromStream(svg_stream);
        svgDom->setContainerSize(SkSize::Make(kPixelSize, kPixelSize));
        svgDom->render(canvas);
    }

    void drawColrV1Glyph(int index, SkCanvas* canvas) {
        TRACE_EVENT0("skia", TRACE_FUNC);
        const uint16_t testCodepoint = fColrV1GlyphIds[index];
        SkPaint paint;
        canvas->drawSimpleText(
                &testCodepoint, 2, SkTextEncoding::kGlyphID, 20, kPixelSize + 20, fColrV1EmojiFont, paint);
    }

    void drawCbdtGlyph(int index, SkCanvas* canvas) {
        TRACE_EVENT0("skia", TRACE_FUNC);
        const uint16_t testCodepoint = fCbdtGlyphIds[index];
        SkPaint paint;
        canvas->drawSimpleText(
                &testCodepoint, 2, SkTextEncoding::kGlyphID, 20, kPixelSize + 20, fCbdtEmojiFont, paint);
    }

      void onDrawSvg(int loops, SkCanvas* canvas) {
        for (int l = 0; l < loops; ++l) {
            SkASSERT(fSvgs.size() == fColrV1GlyphIds.size());
            for (size_t i = 0; i < fSvgs.size(); ++i) {
              drawSvgGlyph(i, canvas);
            }
        }
    }

    void onDrawColrV1(int loops, SkCanvas* canvas) {
        for (int l = 0; l < loops; ++l) {
            SkASSERT(fSvgs.size() == fColrV1GlyphIds.size());
            for (size_t i = 0; i < fColrV1GlyphIds.size(); ++i) {
              drawColrV1Glyph(i, canvas);
            }
        }
    }

    void onDrawCbdt(int loops, SkCanvas* canvas) {
        for (int l = 0; l < loops; ++l) {
            SkASSERT(fSvgs.size() == fColrV1GlyphIds.size());
            for (size_t i = 0; i < fColrV1GlyphIds.size(); ++i) {
              drawCbdtGlyph(i, canvas);
            }
        }
    }

    std::vector<std::string> fFileList;
    std::vector<SkGlyphID> fColrV1GlyphIds;
    std::vector<SkGlyphID> fCbdtGlyphIds;

    BenchMode fMode;
    SkString fName;
    SkFont fColrV1EmojiFont;
    SkFont fCbdtEmojiFont;
    sk_sp<SkTypeface> fNotoEmojiColrV1Typeface;
    sk_sp<SkTypeface> fNotoEmojiCbdtTypeface;
    std::vector<sk_sp<SkData>> fSvgs;
};

DEF_BENCH(return new EmojiFontBench(EmojiFontBench::BenchMode::kSvg);)
DEF_BENCH(return new EmojiFontBench(EmojiFontBench::BenchMode::kColrV1);)
DEF_BENCH(return new EmojiFontBench(EmojiFontBench::BenchMode::kCbdt);)
