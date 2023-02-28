/*
* Copyright 2023 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkBitmaskEnum.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/src/SkUnicode_icu4x.h"
#include "src/base/SkUTF.h"
#include <ICU4XDataProvider.hpp>
#include <ICU4XGraphemeClusterSegmenter.hpp>
#include <ICU4XLineSegmenter.hpp>
#include <ICU4XWordSegmenter.hpp>
#include <ICU4XCodePointSetData.hpp>
#include <ICU4XBidi.hpp>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class SkUnicode_icu4x : public SkUnicode {
public:
    SkUnicode_icu4x()
        : fDataProvider(ICU4XDataProvider::create_test()) { }

    std::unique_ptr<SkUnicode> copy() override {
        return std::make_unique<SkUnicode_icu4x>();
    }

    ~SkUnicode_icu4x() override = default;

    void reset();
    // For SkShaper
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[], int count,
                                                     SkBidiIterator::Direction dir) override;
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[],
                                                     int count,
                                                     SkBidiIterator::Direction dir) override;
    std::unique_ptr<SkBreakIterator> makeBreakIterator(const char locale[],
                                                       BreakType breakType) override;
    std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType breakType) override;
    // For SkParagraph
    bool getBidiRegions(const char utf8[],
                        int utf8Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) override {

        const auto bidi = ICU4XBidi::create(fDataProvider).ok().value();
        std::string_view string_view(utf8, utf8Units);
        auto info = bidi.for_text(string_view, dir == TextDirection::kLTR ? 0 : 1);
        auto currentLevel = info.level_at(0);
        size_t start = 0;

        for (size_t i = 1; i < info.size(); i++) {
            const auto level =  info.level_at(i);
            if (level != currentLevel) {
                (*results).emplace_back(start, i, currentLevel);
                currentLevel = level;
                start = i;
            }
        }
        (*results).emplace_back(start, info.size(), currentLevel);
        return true;
    }

    bool getBidiRegions(const uint16_t utf16[],
                        int utf16Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) {
        auto utf8 = SkUnicode::convertUtf16ToUtf8((char16_t*)utf16, utf16Units);
        return this->getBidiRegions(utf8.data(), utf8.size(), dir, results);
    }

    static bool isControl(SkUnichar utf8) {
        return (utf8 < ' ') || (utf8 >= 0x7f && utf8 <= 0x9f) ||
               (utf8 >= 0x200D && utf8 <= 0x200F) ||
               (utf8 >= 0x202A && utf8 <= 0x202E);
    }

    bool isBreakingWhitespace(SkUnichar unichar) const override {
        const auto whitespaces =
                ICU4XCodePointSetData::load_white_space(fDataProvider).ok().value();
        return whitespaces.contains(unichar);
    }


    static bool isSpace(SkUnichar unichar) {
        std::u16string spaces =
       u"\u0009" // character tabulation
        "\u000A" // line feed
        "\u000B" // line tabulation
        "\u000C" // form feed
        "\u000D" // carriage return
        "\u0020" // space
        "\u00A0" // no-break space
        "\u1680" // ogham space mark
        "\u180E" //
        "\u2000" // en quad
        "\u2001" // em quad
        "\u2002" // en space
        "\u2003" // em space
        "\u2004" // three-per-em space
        "\u2005" // four-per-em space
        "\u2006" // six-per-em space
        "\u2007" // figure space
        "\u2008" // punctuation space
        "\u2009" // thin space
        "\u200A" // hair space
        "\u2028" // line separator
        "\u2029" // paragraph separator
        "\u202F" // narrow no-break space
        "\u205F" // medium mathematical space
        "\u3000" // ideographic space
        "\uFEFF";
        return spaces.find(unichar) != std::u16string::npos;
    }

    static bool isTabulation(SkUnichar utf8) {
        return utf8 == '\t';
    }

    static bool isHardBreak(SkUnichar utf8) {
        return utf8 == '\n';
    }

    bool computeCodeUnitFlags(char utf8[],
                              int utf8Units,
                              bool replaceTabs,
                              SkTArray<SkUnicode::CodeUnitFlags, true>* results) override {
        results->clear();
        results->push_back_n(utf8Units + 1, CodeUnitFlags::kNoCodeUnitFlag);
        this->markLineBreaks(utf8, utf8Units, /*hardLineBreaks=*/false, results);
        //this->markLineBreaks(utf8, utf8Units, /*hardLineBreaks=*/true, results);
        this->markHardLineBreaksHack(utf8, utf8Units, results);
        this->markGraphemes(utf8, utf8Units, results);
        this->markCharacters(utf8, utf8Units, replaceTabs, results);
        return true;
    }

    bool computeCodeUnitFlags(char16_t utf16[], int utf16Units, bool replaceTabs,
                          SkTArray<SkUnicode::CodeUnitFlags, true>* results) override {
        SkASSERT(false);
        return true;
    }

    bool getWords(const char utf8[],
                  int utf8Units,
                  const char* locale,
                  std::vector<Position>* results) override {
        auto utf16 = SkUnicode::convertUtf8ToUtf16(utf8, utf8Units);
        const diplomat::span<const uint16_t> span((uint16_t*)utf16.data(), utf16.size());
        const auto segmenter = ICU4XWordSegmenter::create_dictionary(fDataProvider).ok().value();
        auto iterator = segmenter.segment_utf16(span);
        while (true) {
            int32_t breakpoint = iterator.next();
            if (breakpoint == -1) {
                break;
            }
            results->emplace_back(breakpoint);
        }
        return true;
    }

    SkString toUpper(const SkString& str) override {
        SkDEBUGFAIL("Method 'toUpper' is not implemented yet.\n");
        return SkString();
    }

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override {

        const auto bidi = ICU4XBidi::create(fDataProvider).ok().value();
        const diplomat::span<const uint8_t> levels(&runLevels[0], levelsCount);
        auto map = bidi.reorder_visual(levels);
        SkASSERT(levelsCount == map.len());
        std::vector<int32_t> results;
        for (size_t i = 0; i < map.len(); i++) {
            auto level = map.get(i);
            logicalFromVisual[i] = SkToS32(level);
        }
    }

private:
    friend class SkBreakIterator_icu4x;
    friend class SkBidiIterator_icu4x;

    static bool isHardLineBreak(SkUnichar utf8) {
        return utf8 == '\n';
    }

    bool markHardLineBreaksHack(char utf8[],
                                int utf8Units,
                                SkTArray<SkUnicode::CodeUnitFlags, true>* results) {
        this->forEachCodepoint(&utf8[0], utf8Units,
           [utf8Units, results](SkUnichar unichar, int32_t start, int32_t end, int count) {
            if (SkUnicode_icu4x::isHardLineBreak(unichar)) {
                SkASSERT(count == 1);
                SkASSERT(start < utf8Units);
                (*results)[start + 1] |= CodeUnitFlags::kHardLineBreakBefore;
            }
        });
        return true;
    }

    bool markLineBreaks(char utf8[],
                        int utf8Units,
                        bool hardLineBreaks,
                        SkTArray<SkUnicode::CodeUnitFlags, true>* results) {
      if (utf8Units == 0) {
          return true;
      }
        const auto segmenter = ICU4XLineSegmenter::create_lstm_with_options_v1
                (fDataProvider,
                 (hardLineBreaks)
                     ? ICU4XLineBreakOptionsV1{ .line_break_rule = ICU4XLineBreakRule::Strict,
                                                .word_break_rule = ICU4XWordBreakRule::KeepAll,
                                                .ja_zh = false}
                     : ICU4XLineBreakOptionsV1{ .line_break_rule = ICU4XLineBreakRule::Normal,
                                                .word_break_rule = ICU4XWordBreakRule::Normal,
                                                .ja_zh = false}
                 ).ok().value();
        std::string_view string_view(utf8, utf8Units);
        auto iterator = segmenter.segment_utf8(string_view);
        (*results)[0] |= CodeUnitFlags::kSoftLineBreakBefore;
        while (true) {
            int32_t lineBreak = iterator.next();
            if (lineBreak == -1) {
                break;
            }
            (*results)[lineBreak] |= hardLineBreaks
                                        ? CodeUnitFlags::kHardLineBreakBefore
                                        : CodeUnitFlags::kSoftLineBreakBefore;
        }
        return true;
    }

    bool markGraphemes(const char utf8[],
                       int utf8Units,
                       SkTArray<SkUnicode::CodeUnitFlags, true>* results) {
        const auto segmenter = ICU4XGraphemeClusterSegmenter::create(fDataProvider).ok().value();
        std::string_view string_view(utf8, utf8Units);
        auto iterator = segmenter.segment_utf8(string_view);
        while (true) {
            int32_t graphemeStart = iterator.next();
            if (graphemeStart == -1) {
                break;
            }
            (*results)[graphemeStart] |= CodeUnitFlags::kGraphemeStart;
        }
        return true;
    }

    bool markCharacters(char utf8[],
                        int utf8Units,
                        bool replaceTabs,
                       SkTArray<SkUnicode::CodeUnitFlags, true>* results) {
        const char* current = utf8;
        const char* end = utf8 + utf8Units;
        while (current < end) {
            auto before = current - utf8;
            SkUnichar unichar = SkUTF::NextUTF8(&current, end);
            if (unichar < 0) unichar = 0xFFFD;
            auto after = current - utf8;
            if (replaceTabs && SkUnicode_icu4x::isTabulation(unichar)) {
                results->at(before) |= SkUnicode::kTabulation;
                if (replaceTabs) {
                    unichar = ' ';
                    utf8[before] = ' ';
                }
            }
            for (auto i = before; i < after; ++i) {
                if (SkUnicode_icu4x::isSpace(unichar)) {
                    results->at(i) |= SkUnicode::kPartOfIntraWordBreak;
                }
                if (SkUnicode_icu4x::isBreakingWhitespace(unichar)) {
                    results->at(i) |= SkUnicode::kPartOfWhiteSpaceBreak;
                }
                if (SkUnicode_icu4x::isControl(unichar)) {
                    results->at(i) |= SkUnicode::kControl;
                }
            }
        }
        return true;
    }

    ICU4XDataProvider fDataProvider;
    std::shared_ptr<std::vector<SkUnicode::BidiRegion>> fRegions;
};

class SkBreakIterator_icu4x: public SkBreakIterator {
    Position fLastResult;
    Position fStart;
    Position fEnd;
public:
    SkBreakIterator_icu4x() { }
    Position first() override { SkASSERT(false); return -1; }
    Position current() override { SkASSERT(false); return -1; }
    Position next() override { SkASSERT(false); return -1; }
    Status status() override { SkASSERT(false); return -1; }
    bool isDone() override { SkASSERT(false); return false; }
    bool setText(const char utftext8[], int utf8Units) override { SkASSERT(false); return false; }
    bool setText(const char16_t utftext16[], int utf16Units) override { SkASSERT(false); return false; }
};

class SkBidiIterator_icu4x : public SkBidiIterator {
    std::shared_ptr<std::vector<SkUnicode::BidiRegion>> fRegions;
public:
    explicit SkBidiIterator_icu4x(std::shared_ptr<std::vector<SkUnicode::BidiRegion>> regions)
            : fRegions(regions) { }
    Position getLength() override { return fRegions->size(); }
    Level getLevelAt(Position pos) override {
        auto found = std::lower_bound(
                fRegions->begin(),
                fRegions->end(),
                SkUnicode::BidiRegion(pos, pos, 0),
                [](const SkUnicode::BidiRegion& a, const SkUnicode::BidiRegion& b) {
                    return a.start <= b.start && a.end <= b.end;
                });
        return found->level;
    }
};

std::unique_ptr<SkBidiIterator> SkUnicode_icu4x::makeBidiIterator(const uint16_t text[], int count,
                                                 SkBidiIterator::Direction dir) {
    if (fRegions) {
        fRegions->clear();
    } else {
        fRegions = std::make_shared<std::vector<SkUnicode::BidiRegion>>();
    }

    if (this->getBidiRegions(text, count, dir == SkBidiIterator::Direction::kLTR ? TextDirection::kLTR : TextDirection::kRTL, fRegions.get())) {
        return std::make_unique<SkBidiIterator_icu4x>(fRegions);
    } else {
        return nullptr;
    }
}

std::unique_ptr<SkBidiIterator> SkUnicode_icu4x::makeBidiIterator(const char text[],
                                                 int count,
                                                 SkBidiIterator::Direction dir) {
    if (fRegions) {
        fRegions->clear();
    } else {
        fRegions = std::make_shared<std::vector<SkUnicode::BidiRegion>>();
    }
    if (this->getBidiRegions(text, count, dir == SkBidiIterator::Direction::kLTR ? TextDirection::kLTR : TextDirection::kRTL, fRegions.get())) {
        return std::make_unique<SkBidiIterator_icu4x>(fRegions);
    } else {
        return nullptr;
    }
}

std::unique_ptr<SkBreakIterator> SkUnicode_icu4x::makeBreakIterator(const char locale[],
                                                   BreakType breakType) {
    SkASSERT(false); return nullptr;
}
std::unique_ptr<SkBreakIterator> SkUnicode_icu4x::makeBreakIterator(BreakType breakType) {
    SkASSERT(false); return nullptr;
}

std::unique_ptr<SkUnicode> SkUnicode::MakeIcu4xBasedUnicode() {
    return std::make_unique<SkUnicode_icu4x>();
}
