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
        fNotoEmojiCbdtTypeface = makeMemoryFont("third_party/externals/noto-emoji/fonts/NotoEmoji-Regular.ttf");

        SkASSERT(fNotoEmojiColrV1Typeface);
        SkASSERT(fNotoEmojiColrV1Typeface->countGlyphs());
        fColrV1EmojiFont = SkFont(fNotoEmojiColrV1Typeface, kPixelSize * kFontSizeScale);
        fCbdtEmojiFont = SkFont(fNotoEmojiCbdtTypeface, kPixelSize * kFontSizeScale);

        std::string path = "third_party/externals/noto-emoji/svg/";
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            std::regex base_emoji_regex(".*emoji_u([0-9a-f]*).svg");
            std::string entry_path = entry.path().u8string();
            std::smatch emoji_codepoint_match;
            if (std::regex_match(entry_path, emoji_codepoint_match, base_emoji_regex)) {
                std::vector<std::string> exceptions = {
                        "1f93c", "1f9d6", "1f9e2", "1f921", "1f935", "1f9ed", "1f5f3", "1f3fc",
                        "1f3f5", "1fab0", "1fa9f", "26f9",  "2626",  "1fa90", "1fa84", "26d1",
                        "1fa85", "262a",  "1fa91", "26f8",  "1f698", "1fab1", "1f3f4", "1f3fb",
                        "23cf",  "1f934", "1f9ee", "1f9da", "1f920", "1f9e3", "1f9d7", "1f92f",
                        "1f9e1", "1f92d", "1f9d5", "1f936", "1f922", "1f9dc", "1f329", "1fab3",
                        "1fa78", "1fa9e", "1fa93", "1fa92", "002a",  "2618",  "26d3",  "1fa86",
                        "1fa9d", "1fab2", "1fa79", "1f328", "1f3f7", "1f3fa", "1f923", "1f9db",
                        "1f9ef", "1f937", "1f92e", "1f9d4", "1f9e0", "1f93a", "1f933", "1f9eb",
                        "1f9f8", "1f9df", "1f927", "1f93e", "1f9e4", "1f9d0", "1f92a", "1f3f3",
                        "1f3fe", "1fab6", "1fac2", "1fa82", "262f",  "1fa96", "2620",  "1fa9a",
                        "1fa97", "1fa83", "1f3fd", "1f5e8", "1f9d1", "1f93d", "1f9e5", "1f926",
                        "1f932", "1f9f9", "1f9ec", "1f924", "1f9de", "1f9ea", "1f930", "1f92b",
                        "1f918", "1f9d3", "1f9e7", "1f93f", "1f7e8", "1f3d8", "1f3ff", "1fac1",
                        "1fab5", "1fa95", "262e",  "1fa81", "1fa9c", "2623",  "2622",  "1fa9b",
                        "1fa80", "1fa94", "1fab4", "1fac0", "1faa8", "1f3d9", "1f5fa", "1f7e9",
                        "1f9e6", "1f919", "1f92c", "1f9d2", "1f931", "1f925", "1f9dd", "1f94d",
                        "1f9b5", "1f9c1", "1f942", "1f9bc", "1f9a9", "1f956", "1f99c", "1f981",
                        "1f995", "1f573", "1f5a4", "23ed",  "1f4ff", "2692",  "1fad3", "1f6f3",
                        "265f",  "1fad2", "23ee",  "1f5a5", "1f994", "1f980", "1f99b", "1f98f",
                        "1f9cf", "1f957", "1f943", "1f9a8", "1f9bb", "1f9c0", "1f95a", "1f94e",
                        "1f9b4", "1f95c", "1f969", "1f9c2", "1f9b6", "1f955", "1f9cd", "1f941",
                        "1f98d", "1f996", "1f982", "1f570", "1fad0", "26b1",  "26b0",  "1f6f0",
                        "fe82b", "1fad1", "23ef",  "1f4fd", "1f983", "1f997", "1f98e", "1f99a",
                        "1f9ba", "1f940", "1f954", "1f9ce", "1f9b7", "1f94f", "1f968", "1f95b",
                        "1f9c3", "1f9ca", "1f950", "1f944", "1f9be", "1f9c7", "1f95f", "1f978",
                        "1f94b", "1f9b3", "1f993", "1f987", "1f99e", "1f98a", "1f549", "1f575",
                        "1f37e", "23f8",  "269b",  "1fad5", "2694",  "1f6f4", "1f6fb", "26cf",
                        "1f6fc", "1f6e9", "2642",  "1f6f5", "2695",  "1fad4", "269c",  "23f9",
                        "1f37d", "1f399", "1f574", "1f56f", "1f99d", "1f986", "1f992", "1f94c",
                        "1f9b2", "1f9c6", "1f945", "1f9bd", "1f951", "1f9bf", "1f947", "1f953",
                        "1f9cb", "1f9b0", "1f94a", "1f95e", "1f9c4", "1f984", "1f990", "1f98b",
                        "1f99f", "1f576", "1f37f", "2697",  "1fad6", "1f6f7", "2640",  "1f6fa",
                        "1f6f6", "2696",  "1f577", "1f57a", "2721",  "1f98c", "1f991", "1f985",
                        "1f95d", "1f9c5", "1f9b1", "1f952", "1f96e", "1f9a0", "1f97a", "1f963",
                        "1f9c8", "1f9af", "1f977", "1f988", "1f58c", "1f5b1", "1f397", "1f39a",
                        "1f3ce", "23fa",  "2328",  "26a7",  "1f6d2", "1f42a", "1f4f8", "1f3cd",
                        "1f396", "1f5c4", "1f54a", "1f58b", "1f590", "1f989", "1f976", "1f962",
                        "1f9c9", "1f9a1", "1f96d", "1f97b", "1f948", "1f9a3", "1f96f", "1f974",
                        "1f9ae", "1f960", "1f579", "1f54c", "1f5b2", "1f39b", "1f3cf", "1f6eb",
                        "1f6f8", "1f6d0", "1f6e4", "267e",  "1f6e5", "1f6d1", "1f6f9", "1f6ec",
                        "2699",  "1f43f", "1f54b", "1f578", "1f58a", "1f587", "271d",  "1f961",
                        "1f975", "1f9ad", "1f949", "1f97c", "1f9a2", "1f971", "1f965", "1f9a6",
                        "1f959", "1f96c", "270d",  "1f5c3", "23f1",  "1f3c5", "1f3cc", "1f6d5",
                        "1f6e1", "26c8",  "1f6e0", "1f3cb", "1f39f", "1f5c2", "1f596", "1f58d",
                        "1f96b", "1f958", "1f9a7", "1f97f", "1f964", "1f9aa", "1f970", "1f966",
                        "1f972", "1f9b9", "1f9ac", "1f97d", "1f9a5", "1f999", "1f54e", "1f5a8",
                        "23f2",  "1f6e2", "1f6d6", "1f6d7", "1f6e3", "1f39e", "1f5bc", "1f54d",
                        "1f595", "1f998", "1f97e", "1f9a4", "1f96a", "1f973", "1f9ab", "1f9b8",
                        "1f967", "1f9f7", "1f90f", "1f928", "1f91b", "1f9fa", "1f914", "1f7eb",
                        "1f7e4", "1f5dd", "1f5d2", "1f3db", "1f32e", "1f3d4", "1faa5", "1fa72",
                        "1f6cb", "1fa99", "26f0",  "26f1",  "1fa98", "1f6cc", "1fa73", "1faa4",
                        "1f32d", "1f3d5", "1f3dc", "1f336", "1f5d3", "1f5de", "1f7e5", "1f915",
                        "1f91c", "1f929", "1f9f6", "1f91a", "1f90e", "1f9f4", "1f917", "1f9e8",
                        "1f9fb", "1f7ea", "1f7e7", "1f5d1", "1f3da", "1f3d7", "1f32f", "1faa6",
                        "1fa71", "2604",  "1f644", "2638",  "2639",  "1fa70", "1faa7", "1f3d6",
                        "1f321", "1f7e6", "1f9fc", "1f9e9", "1f916", "1f90d", "1f9f5", "1f912",
                        "1f9d9", "1f91d", "1f9f1", "1f7e2", "1f5ef", "1f32c", "1f3d2", "1f325",
                        "1f3dd", "1fa74", "1faa3", "1f641", "1f6cd", "1f6ce", "26f7",  "1faa2",
                        "1f324", "1f3de", "1f32b", "1f3d3", "1f5dc", "1f5e1", "1f7e3", "1f9f0",
                        "1f91e", "1f9ff", "1f913", "1f9d8", "1f9fd", "1f911", "1f90c", "1f939",
                        "1f9f2", "1f7e1", "1f5e3", "1f3d1", "1f3f9", "1f326", "1fa7a", "1faa0",
                        "2602",  "1f642", "26e9",  "2603",  "1f643", "1f6cf", "26f4",  "1faa1",
                        "1f441", "1f3df", "1f327", "1f3f8", "1f3d0", "1f32a", "2763",  "1f7e0",
                        "1f91f", "1f938", "1f9f3", "1f910", "1f9fe"};
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
