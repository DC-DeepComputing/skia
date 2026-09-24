/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkSurface.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skshaper/utils/FactoryHelpers.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "tools/fonts/FontToolUtils.h"

#if defined(SK_FONTMGR_CORETEXT_AVAILABLE)
#include "include/ports/SkFontMgr_mac_ct.h"
#endif

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#include <mach/mach.h>
#else
#include <sys/resource.h>
#endif

#include <chrono>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

using namespace skia::textlayout;

namespace {

// Operating memory measurement
struct MemorySnapshot {
    size_t physFootprintBytes = 0;
    size_t residentBytes = 0;
};

MemorySnapshot GetCurrentMemory() {
    MemorySnapshot snap;
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    task_vm_info_data_t vm_info;
    mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_VM_INFO, (task_info_t)&vm_info, &count) == KERN_SUCCESS) {
        snap.physFootprintBytes = (size_t)vm_info.phys_footprint;
    }
    mach_task_basic_info_data_t basic_info;
    mach_msg_type_number_t basic_count = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&basic_info, &basic_count) == KERN_SUCCESS) {
        snap.residentBytes = (size_t)basic_info.resident_size;
    }
#else
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        snap.physFootprintBytes = (size_t)usage.ru_maxrss * 1024;
        snap.residentBytes = snap.physFootprintBytes;
    }
#endif
    return snap;
}

std::string FormatBytes(size_t bytes) {
    char buf[64];
    if (bytes >= 1024 * 1024) {
        snprintf(buf, sizeof(buf), "%.2f MB", (double)bytes / (1024.0 * 1024.0));
    } else if (bytes >= 1024) {
        snprintf(buf, sizeof(buf), "%.1f KB", (double)bytes / 1024.0);
    } else {
        snprintf(buf, sizeof(buf), "%zu B", bytes);
    }
    return std::string(buf);
}

std::string FormatSignedBytes(int64_t bytes) {
    if (bytes < 0) {
        return "-" + FormatBytes((size_t)(-bytes));
    } else {
        return "+" + FormatBytes((size_t)bytes);
    }
}

sk_sp<SkUnicode> GetActiveUnicode() {
    return SkShapers::BestAvailableUnicode();
}

sk_sp<FontCollection> CreateFontCollection() {
    auto fontCollection = sk_make_sp<FontCollection>();
#if defined(SK_FONTMGR_CORETEXT_AVAILABLE)
    fontCollection->setDefaultFontManager(SkFontMgr_New_CoreText(nullptr));
#else
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
#endif
    fontCollection->enableFontFallback();
    return fontCollection;
}

struct BenchResult {
    std::string name;
    size_t iterations;
    double totalMs;
    double avgUs;
    double opsPerSec;
    int64_t memDeltaBytes;
};

BenchResult RunBenchmark(const std::string& name,
                         size_t warmupRuns,
                         size_t benchRuns,
                         std::function<void()> func) {
    // Warm-up phase
    for (size_t i = 0; i < warmupRuns; ++i) {
        func();
    }

    MemorySnapshot beforeMem = GetCurrentMemory();
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < benchRuns; ++i) {
        func();
    }

    auto end = std::chrono::high_resolution_clock::now();
    MemorySnapshot afterMem = GetCurrentMemory();

    double totalMs = std::chrono::duration<double, std::milli>(end - start).count();
    double avgUs = (totalMs * 1000.0) / (double)benchRuns;
    double opsPerSec = ((double)benchRuns / totalMs) * 1000.0;
    int64_t memDelta = (int64_t)afterMem.physFootprintBytes - (int64_t)beforeMem.physFootprintBytes;

    return BenchResult{name, benchRuns, totalMs, avgUs, opsPerSec, memDelta};
}

} // namespace

int main(int argc, char** argv) {
    SkGraphics::Init();

    std::cout << "=========================================================================\n";
    std::cout << "          SkParagraph Performance & Operating Memory Benchmark          \n";
    std::cout << "=========================================================================\n\n";

    // Detect Active Pipeline
    std::cout << "Active Engine Configuration:\n";
#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE)
    std::cout << "  * Shaper Engine  : HarfBuzz (SkShapers::HB)\n";
#elif defined(SK_SHAPER_CORETEXT_AVAILABLE)
    std::cout << "  * Shaper Engine  : Apple CoreText (SkShapers::CT)\n";
#else
    std::cout << "  * Shaper Engine  : Primitive\n";
#endif

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
    std::cout << "  * Unicode Engine : ICU (SkUnicode_icu)\n";
#elif defined(SK_UNICODE_APPLE_IMPLEMENTATION)
    std::cout << "  * Unicode Engine : Apple Native (SkUnicode_apple / CFStringTokenizer)\n";
#else
    std::cout << "  * Unicode Engine : Default\n";
#endif

    MemorySnapshot initMem = GetCurrentMemory();
    std::cout << "  * Initial Resident Memory : " << FormatBytes(initMem.residentBytes) << "\n";
    std::cout << "  * Initial Phys Footprint  : " << FormatBytes(initMem.physFootprintBytes) << "\n\n";

    auto fontCollection = CreateFontCollection();
    auto unicode = GetActiveUnicode();
    if (!unicode) {
        std::cerr << "Error: Failed to initialize SkUnicode engine!\n";
        return 1;
    }

    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(800, 1200));
    SkCanvas* canvas = surface->getCanvas();

    std::vector<BenchResult> results;

    // -------------------------------------------------------------------------
    // Benchmark 1: Short UI Labels (Buttons, Badges, Tab Titles)
    // -------------------------------------------------------------------------
    {
        const char* shortLabel = "Submit Payment ($49.99)";
        ParagraphStyle pStyle;
        TextStyle tStyle;
        tStyle.setFontSize(14.0f);
        pStyle.setTextStyle(tStyle);

        results.push_back(RunBenchmark("1. Short UI Label (Build + Layout)", 100, 10000, [&]() {
            ParagraphBuilderImpl builder(pStyle, fontCollection, unicode);
            builder.pushStyle(tStyle);
            builder.addText(shortLabel);
            auto paragraph = builder.Build();
            paragraph->layout(300.0f);
        }));
    }

    // -------------------------------------------------------------------------
    // Benchmark 2: Multi-Style Rich Text (Headline + Subtitle + Bold Tag)
    // -------------------------------------------------------------------------
    {
        ParagraphStyle pStyle;
        TextStyle titleStyle;
        titleStyle.setFontSize(22.0f);
        titleStyle.setColor(SK_ColorBLACK);

        TextStyle bodyStyle;
        bodyStyle.setFontSize(14.0f);
        bodyStyle.setColor(SK_ColorDKGRAY);

        TextStyle tagStyle;
        tagStyle.setFontSize(12.0f);
        tagStyle.setColor(SK_ColorBLUE);

        results.push_back(RunBenchmark("2. Multi-Style Card Text (Build + Layout)", 100, 5000, [&]() {
            ParagraphBuilderImpl builder(pStyle, fontCollection, unicode);
            builder.pushStyle(titleStyle);
            builder.addText("Breaking Technology News\n");
            builder.pop();
            builder.pushStyle(bodyStyle);
            builder.addText("High performance native typography without heavy runtime dependencies.\n");
            builder.pop();
            builder.pushStyle(tagStyle);
            builder.addText("#flutter #skia #coretext #apple");
            builder.pop();
            auto paragraph = builder.Build();
            paragraph->layout(350.0f);
        }));
    }

    // -------------------------------------------------------------------------
    // Benchmark 3: Long Paragraph Article with Word-Wrapping
    // -------------------------------------------------------------------------
    {
        const char* longArticle =
            "Typography is the art and technique of arranging type to make written language legible, readable "
            "and appealing when displayed. The arrangement of type involves selecting typefaces, point sizes, "
            "line lengths, line-spacing, and letter-spacing, and adjusting the space between pairs of letters. "
            "Modern digital rendering requires sophisticated grapheme cluster break detection, bidirectional "
            "text algorithm resolution, and OpenType/AAT glyph shaping matrices.";

        ParagraphStyle pStyle;
        TextStyle tStyle;
        tStyle.setFontSize(15.0f);
        pStyle.setTextStyle(tStyle);

        results.push_back(RunBenchmark("3. Article Multiline Wrap (Build + Layout)", 50, 3000, [&]() {
            ParagraphBuilderImpl builder(pStyle, fontCollection, unicode);
            builder.pushStyle(tStyle);
            builder.addText(longArticle);
            auto paragraph = builder.Build();
            paragraph->layout(320.0f);
        }));
    }

    // -------------------------------------------------------------------------
    // Benchmark 4: Complex BiDi & Multi-Script (Arabic + Latin + Digits)
    // -------------------------------------------------------------------------
    {
        const char* bidiText =
            "Welcome أهلاً وسهلاً بكم في نظام الطباعة السريع 2026 - Level 4 Verification שלום!";

        ParagraphStyle pStyle;
        TextStyle tStyle;
        tStyle.setFontSize(16.0f);
        pStyle.setTextStyle(tStyle);

        results.push_back(RunBenchmark("4. BiDi Arabic/Hebrew/Latin (Build + Layout)", 50, 3000, [&]() {
            ParagraphBuilderImpl builder(pStyle, fontCollection, unicode);
            builder.pushStyle(tStyle);
            builder.addText(bidiText);
            auto paragraph = builder.Build();
            paragraph->layout(400.0f);
        }));
    }

    // -------------------------------------------------------------------------
    // Benchmark 5: Extended Compound Emojis & Modifiers (ZWJ, Skin Tones)
    // -------------------------------------------------------------------------
    {
        const char* emojiText =
            "👨‍👩‍👧‍👦 👩🏽‍💻 🏳️‍🌈 0️⃣ #️⃣ 🆗 🚀 🏎️ 🏄🏼‍♀️ 👨🏻‍🌾 🧑🏾‍🍳 👩🏼‍🚀 👩🏽‍🎨";

        ParagraphStyle pStyle;
        TextStyle tStyle;
        tStyle.setFontSize(20.0f);
        pStyle.setTextStyle(tStyle);

        results.push_back(RunBenchmark("5. Complex Emojis & Modifiers (Build + Layout)", 50, 3000, [&]() {
            ParagraphBuilderImpl builder(pStyle, fontCollection, unicode);
            builder.pushStyle(tStyle);
            builder.addText(emojiText);
            auto paragraph = builder.Build();
            paragraph->layout(400.0f);
        }));
    }

    // -------------------------------------------------------------------------
    // Benchmark 6: Full Pipeline (Build + Layout + Paint to Canvas)
    // -------------------------------------------------------------------------
    {
        const char* sampleText =
            "Full rendering pipeline execution measuring text shaping, glyph positioning, "
            "text blob generation, and 2D canvas drawing commands.";

        ParagraphStyle pStyle;
        TextStyle tStyle;
        tStyle.setFontSize(14.0f);
        pStyle.setTextStyle(tStyle);

        results.push_back(RunBenchmark("6. Full Pipeline (Build + Layout + Paint)", 50, 3000, [&]() {
            ParagraphBuilderImpl builder(pStyle, fontCollection, unicode);
            builder.pushStyle(tStyle);
            builder.addText(sampleText);
            auto paragraph = builder.Build();
            paragraph->layout(300.0f);
            paragraph->paint(canvas, 10.0f, 10.0f);
        }));
    }

    // -------------------------------------------------------------------------
    // Benchmark 7: Hit-Testing & Rects-For-Range Queries
    // -------------------------------------------------------------------------
    {
        const char* testText = "Interactive text selection, cursor navigation, and bounding box queries.";
        ParagraphStyle pStyle;
        TextStyle tStyle;
        tStyle.setFontSize(14.0f);
        pStyle.setTextStyle(tStyle);

        ParagraphBuilderImpl builder(pStyle, fontCollection, unicode);
        builder.pushStyle(tStyle);
        builder.addText(testText);
        auto paragraph = builder.Build();
        paragraph->layout(300.0f);

        results.push_back(RunBenchmark("7. Hit Testing (getRectsForRange queries)", 100, 10000, [&]() {
            for (size_t i = 0; i < 60; i += 10) {
                auto boxes = paragraph->getRectsForRange(i, i + 8, RectHeightStyle::kTight, RectWidthStyle::kTight);
                (void)boxes;
            }
        }));
    }

    // -------------------------------------------------------------------------
    // Benchmark 8: Mass Sustained Allocation Test (Retain 5,000 Paragraph Objects)
    // -------------------------------------------------------------------------
    {
        std::vector<std::unique_ptr<Paragraph>> retainedParagraphs;
        retainedParagraphs.reserve(5000);

        ParagraphStyle pStyle;
        TextStyle tStyle;
        tStyle.setFontSize(14.0f);
        pStyle.setTextStyle(tStyle);

        MemorySnapshot beforeSustained = GetCurrentMemory();
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 5000; ++i) {
            ParagraphBuilderImpl builder(pStyle, fontCollection, unicode);
            builder.pushStyle(tStyle);
            builder.addText("Retained paragraph instance #");
            builder.addText(std::to_string(i).c_str());
            auto p = builder.Build();
            p->layout(250.0f);
            retainedParagraphs.push_back(std::move(p));
        }

        auto end = std::chrono::high_resolution_clock::now();
        MemorySnapshot peakSustained = GetCurrentMemory();
        double sustainedMs = std::chrono::duration<double, std::milli>(end - start).count();

        int64_t sustainedDelta = (int64_t)peakSustained.physFootprintBytes - (int64_t)beforeSustained.physFootprintBytes;
        results.push_back(BenchResult{
            "8. Retain 5,000 Paragraphs in Heap (Stress)",
            5000,
            sustainedMs,
            (sustainedMs * 1000.0) / 5000.0,
            (5000.0 / sustainedMs) * 1000.0,
            sustainedDelta
        });

        retainedParagraphs.clear();
    }

    // -------------------------------------------------------------------------
    // Print Benchmark Report Table
    // -------------------------------------------------------------------------
    std::cout << "----------------------------------------------------------------------------------------------------------\n";
    std::cout << std::left
              << std::setw(46) << "Benchmark Workload"
              << std::setw(12) << "Runs"
              << std::setw(15) << "Avg Time (us)"
              << std::setw(18) << "Throughput (ops/s)"
              << std::setw(15) << "Mem Footprint"
              << "\n";
    std::cout << "----------------------------------------------------------------------------------------------------------\n";

    for (const auto& res : results) {
        std::cout << std::left
                  << std::setw(46) << res.name
                  << std::setw(12) << res.iterations
                  << std::setw(15) << std::fixed << std::setprecision(2) << res.avgUs
                  << std::setw(18) << std::fixed << std::setprecision(0) << res.opsPerSec
                  << std::setw(15) << FormatSignedBytes(res.memDeltaBytes)
                  << "\n";
    }
    std::cout << "----------------------------------------------------------------------------------------------------------\n\n";

    MemorySnapshot finalMem = GetCurrentMemory();
    std::cout << "Final Memory Summary:\n";
    std::cout << "  * Final Resident Memory   : " << FormatBytes(finalMem.residentBytes) << "\n";
    std::cout << "  * Final Phys Footprint    : " << FormatBytes(finalMem.physFootprintBytes) << "\n";
    std::cout << "  * Peak Footprint Overhead : " << FormatSignedBytes((int64_t)finalMem.physFootprintBytes - (int64_t)initMem.physFootprintBytes) << "\n";
    std::cout << "=========================================================================\n\n";

    return 0;
}
