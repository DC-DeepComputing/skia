/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
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
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <string.h>
#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <thread>

#include "modules/skunicode/include/SkUnicode.h"

namespace {
#if defined(SK_UNICODE_ICU_IMPLEMENTATION) && defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
class SkUnicode_test : public SkUnicode {
public:
    SkUnicode_test() { }

    SkUnicode_test(SkSpan<char> text) {

        auto icu = SkUnicode::Make();

        std::vector<SkUnicode::Position> words;
        icu->getWords(text.data(), text.size(), nullptr, &words);

        SkTArray<SkUnicode::CodeUnitFlags, true> codeUnitFlags;
        icu->computeCodeUnitFlags(text.data(), text.size(), false, &codeUnitFlags);

        std::vector<SkUnicode::Position> graphemeBreaks;
        std::vector<SkUnicode::LineBreakBefore> lineBreaks;
        Position pos = 0;
        for (auto& flag : codeUnitFlags) {
            if (SkUnicode::isGraphemeStart(flag)) {
                graphemeBreaks.emplace_back(pos);
            }
            if (SkUnicode::isHardLineBreak(flag)) {
                lineBreaks.emplace_back(pos, SkUnicode::LineBreakType::kHardLineBreak);
            }
            if (SkUnicode::isSoftLineBreak(flag)) {
                lineBreaks.emplace_back(pos, SkUnicode::LineBreakType::kSoftLineBreak);
            }
            ++pos;
        }

        fClient = SkUnicode::MakeClientBasedUnicode(text, words, graphemeBreaks, lineBreaks);
    }

    std::unique_ptr<SkUnicode> copy() override {
        auto unicode = std::make_unique<SkUnicode_test>();
        unicode->fClient = fClient->copy();
        return unicode;
    }

    ~SkUnicode_test() override = default;

    // For SkShaper
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[], int count,
                                                     SkBidiIterator::Direction dir) override;
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[],
                                                     int count,
                                                     SkBidiIterator::Direction dir) override;
    std::unique_ptr<SkBreakIterator> makeBreakIterator(const char locale[],
                                                       BreakType breakType) override;
    std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType breakType) override;

    bool getBidiRegions(const char utf8[],
                        int utf8Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) override {
        return fClient->getBidiRegions(utf8, utf8Units, dir, results);
    }

    bool computeCodeUnitFlags(char utf8[], int utf8Units, bool replaceTabs,
                          SkTArray<SkUnicode::CodeUnitFlags, true>* results) override {
        return fClient->computeCodeUnitFlags(utf8, utf8Units, replaceTabs, results);
    }

    bool computeCodeUnitFlags(char16_t utf16[], int utf16Units, bool replaceTabs,
                          SkTArray<SkUnicode::CodeUnitFlags, true>* results) override {
        return fClient->computeCodeUnitFlags(utf16, utf16Units, replaceTabs, results);
    }

    bool getWords(const char utf8[], int utf8Units, const char* locale, std::vector<Position>* results) override {
        return fClient->getWords(utf8, utf8Units, locale, results);
    }

    SkString toUpper(const SkString& str) override {
        return fClient->toUpper(str);
    }

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override {
        fClient->reorderVisual(runLevels, levelsCount, logicalFromVisual);
    }
private:
    std::unique_ptr<SkUnicode> fClient;
};

std::unique_ptr<SkBidiIterator> SkUnicode_test::makeBidiIterator(const uint16_t text[], int count,
                                                 SkBidiIterator::Direction dir) {
    return fClient->makeBidiIterator(text, count, dir);
}
std::unique_ptr<SkBidiIterator> SkUnicode_test::makeBidiIterator(const char text[],
                                                 int count,
                                                 SkBidiIterator::Direction dir) {
    return fClient->makeBidiIterator(text, count, dir);
}
std::unique_ptr<SkBreakIterator> SkUnicode_test::makeBreakIterator(const char locale[],
                                                                   BreakType breakType) {
    return fClient->makeBreakIterator(locale, breakType);
}
std::unique_ptr<SkBreakIterator> SkUnicode_test::makeBreakIterator(BreakType breakType) {
    return fClient->makeBreakIterator(breakType);
}
#endif
}  // namespace

#ifdef SK_UNICODE_ICU4X_IMPLEMENTATION
UNIX_ONLY_TEST(SkUnicode_GetWords, reporter) {
    SkString text("1 22 333 4444 55555 666666 7777777");

    auto icu4x = SkUnicode::MakeIcu4xBasedUnicode();
    auto icu = SkUnicode::MakeIcuBasedUnicode();
    auto client = std::make_unique<SkUnicode_test>(text);

    std::vector<SkUnicode::Position> icu4x_results;
    std::vector<SkUnicode::Position> icu_results;
    std::vector<SkUnicode::Position> client_results;
    auto icu4x_result = icu4x->getWords(text.data(), text.size(), "en", &icu4x_results);
    auto icu_result = icu->getWords(text.data(), text.size(), "en", &icu_results);
    auto client_result = icu->getWords(text.data(), text.size(), "en", &client_results);

    REPORTER_ASSERT(reporter, icu4x_result && icu_result && client_result);
    REPORTER_ASSERT(reporter, icu4x_results.size() == 13 &&
                              icu_results.size() == 13 &&
                              client_results.size() == 13);
    for (auto i = 0; i < 13; ++i) {
        REPORTER_ASSERT(reporter, icu4x_results[i] ==icu_results[i] &&
                                  client_results[i] ==icu_results[i]);
    }
};
#endif