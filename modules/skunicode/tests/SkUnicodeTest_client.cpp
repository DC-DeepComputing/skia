                                           /*
* Copyright 2023 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTArray.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/base/SkUTF.h"

#include <cstdint>
#include <memory>
#include <vector>

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

    bool isBreakingWhitespace(SkUnichar unichar) const override {
        return fClient->isBreakingWhitespace(unichar);
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

std::unique_ptr<SkUnicode> SkUnicode::MakeTestClientUnicode(SkSpan<char> text) {
    return std::make_unique<SkUnicode_test>(text);
}
