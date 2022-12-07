                                           /*
* Copyright 2022 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkBitmaskEnum.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTo.h"
#include "modules/skunicode/include/SkUnicode.h"
extern "C" {
#include <grapheme.h>
}
#include "src/utils/SkUTF.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#undef LEN
#define LEN(x) (sizeof(x) / sizeof(*(x)))

class SkUnicode_libgrapheme : public SkUnicode {
public:
    SkUnicode_libgrapheme() { }

    ~SkUnicode_libgrapheme() override = default;

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
        int_least32_t res32 [512];
        auto count = grapheme_get_bidirectional_embedding_levels_utf8(
                utf8,
                utf8Units,
                dir == TextDirection::kRTL
                    ? grapheme_bidirectional_override::GRAPHEME_BIDIRECTIONAL_OVERRIDE_RTL
                    : grapheme_bidirectional_override::GRAPHEME_BIDIRECTIONAL_OVERRIDE_LTR,
                &res32[0],
                LEN(res32));
        if (count > utf8Units) {
            return false;
        }
        BidiRegion region(0, 0, -1);
        for (auto i = 0; i <= utf8Units; ++i) {
            auto level = i == utf8Units ? -1 : SkToS16(res32[i]);
            if (i == 0) {
                region.start = region.end;
                region.level = level;
            } else if (region.level != level) {
                region.end = i;
                results->emplace_back(region);
                region.start = region.end;
                region.level = level;
            }
        }
        return true;
    }

    // TODO: Take if from the Client or hard code here?
    static bool isControl(SkUnichar utf8) {
        return (utf8 < ' ') || (utf8 >= 0x7f && utf8 <= 0x9f) ||
               (utf8 >= 0x200D && utf8 <= 0x200F) ||
               (utf8 >= 0x202A && utf8 <= 0x202E);
    }

    static bool isWhitespace(SkUnichar unichar) {
        std::u16string whitespaces =
       u"\u0009" // character tabulation
        "\u000A" // line feed
        "\u000B" // line tabulation
        "\u000C" // form feed
        "\u000D" // carriage return
        "\u0020" // space
      //"\u0085" // next line
      //"\u00A0" // no-break space
        "\u1680" // ogham space mark
        "\u2000" // en quad
        "\u2001" // em quad
        "\u2002" // en space
        "\u2003" // em space
        "\u2004" // three-per-em space
        "\u2005" // four-per-em space
        "\u2006" // six-per-em space
      //"\u2007" // figure space
        "\u2008" // punctuation space
        "\u2009" // thin space
        "\u200A" // hair space
        "\u2028" // line separator
        "\u2029" // paragraph separator
      //"\u202F" // narrow no-break space
        "\u205F" // medium mathematical space
        "\u3000";// ideographic space
        return whitespaces.find(unichar) != std::u16string::npos;
    }

    static bool isSpace(SkUnichar unichar) {
        std::u16string spaces =
       u"\u0009" // character tabulation
        "\u000A" // line feed
        "\u000B" // line tabulation
        "\u000C" // form feed
        "\u000D" // carriage return
        "\u0020" // space
        "\u0085" // next line
        "\u00A0" // no-break space
        "\u1680" // ogham space mark
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
        "\u3000"; // ideographic space
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
        results->reset();
        results->push_back_n(utf8Units + 1, CodeUnitFlags::kNoCodeUnitFlag);

        size_t lineBreak = 0;
        for (size_t pos = 0; pos < utf8Units; pos += lineBreak) {
            lineBreak = grapheme_next_line_break_utf8(utf8 + pos, utf8Units - pos);
            auto codePoint = utf8[lineBreak];
            (*results)[lineBreak] |= isHardBreak(codePoint)
                                    ? CodeUnitFlags::kHardLineBreakBefore
                                    : CodeUnitFlags::kSoftLineBreakBefore;
        }

        size_t graphemeBreak = 0;
        for (size_t pos = 0; pos < utf8Units; pos += graphemeBreak) {
            graphemeBreak = grapheme_next_character_break_utf8(utf8 + pos, utf8Units - pos);
            (*results)[graphemeBreak] |= CodeUnitFlags::kGraphemeStart;
        }

        const char* current = utf8;
        const char* end = utf8 + utf8Units;
        while (current < end) {
            auto before = current - utf8;
            SkUnichar unichar = SkUTF::NextUTF8(&current, end);
            if (unichar < 0) unichar = 0xFFFD;
            auto after = current - utf8;
            if (replaceTabs && SkUnicode_libgrapheme::isTabulation(unichar)) {
                results->at(before) |= SkUnicode::kTabulation;
                if (replaceTabs) {
                    unichar = ' ';
                    utf8[before] = ' ';
                }
            }
            for (auto i = before; i < after; ++i) {
                if (SkUnicode_libgrapheme::isSpace(unichar)) {
                    results->at(i) |= SkUnicode::kPartOfIntraWordBreak;
                }
                if (SkUnicode_libgrapheme::isWhitespace(unichar)) {
                    results->at(i) |= SkUnicode::kPartOfWhiteSpaceBreak;
                }
                if (SkUnicode_libgrapheme::isControl(unichar)) {
                    results->at(i) |= SkUnicode::kControl;
                }
            }
        }
        return true;
    }

    bool computeCodeUnitFlags(char16_t utf16[], int utf16Units, bool replaceTabs,
                          SkTArray<SkUnicode::CodeUnitFlags, true>* results) override {
        SkASSERT(false);
        return false;
    }

    bool getWords(const char utf8[], int utf8Units, std::vector<Position>* results) override {
        size_t wordBreak = 0;
        for (size_t pos = 0; pos < utf8Units; pos += wordBreak) {
            wordBreak = grapheme_next_word_break_utf8(utf8 + pos, utf8Units - pos);
            results->emplace_back(wordBreak);
        }
        return true;
    }

    SkString toUpper(const SkString& str) override {

        SkString res(" ", str.size());
        auto result = grapheme_to_uppercase_utf8(str.data(), str.size(), res.data(), res.size());
        return res;
    }

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override {
        // This is a temp solution until we "borrow" reorderVisual code
        static std::unique_ptr<SkUnicode> icu = SkUnicode::Make();
        return icu->reorderVisual(runLevels, levelsCount, logicalFromVisual);
    }
private:
    friend class SkBidiIterator_client;
    friend class SkBreakIterator_client;
};

class SkBidiIterator_client : public SkBidiIterator {
    std::vector<SkUnicode::BidiRegion> fBidiRegions;
public:
    explicit SkBidiIterator_client(SkUnicode_libgrapheme* unicode,
                                   const char* text,
                                   int size,
                                   SkBidiIterator::Direction dir) {
        unicode->getBidiRegions(text,
                                size,
                                dir == Direction::kRTL
                                    ? SkUnicode::TextDirection::kRTL
                                    : SkUnicode::TextDirection::kLTR,
                                &fBidiRegions);
    }
    Position getLength() override { return fBidiRegions.size(); }
    Level getLevelAt(Position pos) override {
        auto found = std::lower_bound(
                fBidiRegions.begin(),
                fBidiRegions.end(),
                SkUnicode::BidiRegion(pos, pos, 0),
                [](const SkUnicode::BidiRegion& a, const SkUnicode::BidiRegion& b) {
                    return a.start <= b.start && a.end <= b.end;
                });
        return found->level;
    }
};

class SkBreakIterator_client: public SkBreakIterator {
    SkUnicode_libgrapheme* fUnicode;
    std::vector<SkUnicode::LineBreakBefore> fLineBreaks;
    Position fLastResult;
    Position fStart;
    Position fEnd;
public:
    explicit SkBreakIterator_client(SkUnicode_libgrapheme* unicode) : fUnicode(unicode) { }
    Position first() override
      { return fLineBreaks[fStart + (fLastResult = 0)].pos; }
    Position current() override
      { return fLineBreaks[fStart + fLastResult].pos; }
    Position next() override
      { return fLineBreaks[fStart + fLastResult + 1].pos; }
    Status status() override {
        return fLineBreaks[fStart + fLastResult].breakType ==
                       SkUnicode::LineBreakType::kHardLineBreak
                       ? SkUnicode::CodeUnitFlags::kHardLineBreakBefore
                       : SkUnicode::CodeUnitFlags::kSoftLineBreakBefore;
    }
    bool isDone() override { return fStart + fLastResult == fEnd; }
    bool setText(const char utftext8[], int utf8Units) override {
        fLineBreaks.clear();
        size_t lineBreak = 0;
        for (size_t pos = 0; pos < utf8Units; pos += lineBreak) {
            lineBreak = grapheme_next_line_break_utf8(utftext8 + pos, utf8Units - pos);
            auto codePoint = utftext8[lineBreak];
            fLineBreaks.emplace_back(lineBreak,
                                     fUnicode->isHardBreak(codePoint)
                                    ? SkUnicode::LineBreakType::kHardLineBreak
                                    : SkUnicode::LineBreakType::kSoftLineBreak);
        }
        fStart = 0;
        fEnd = utf8Units;
        fLastResult = 0;
        return true;
    }
    bool setText(const char16_t utftext16[], int utf16Units) override {
        SkASSERT(false);
        return false;
    }
};

std::unique_ptr<SkBidiIterator> SkUnicode_libgrapheme::makeBidiIterator(const uint16_t text[], int count,
                                                 SkBidiIterator::Direction dir) {
    SkASSERT(false);
    return nullptr;
}
std::unique_ptr<SkBidiIterator> SkUnicode_libgrapheme::makeBidiIterator(const char text[],
                                                 int count,
                                                 SkBidiIterator::Direction dir) {
    return std::make_unique<SkBidiIterator_client>(this, text, count, dir);
}
std::unique_ptr<SkBreakIterator> SkUnicode_libgrapheme::makeBreakIterator(const char locale[],
                                                   BreakType breakType) {
    return std::make_unique<SkBreakIterator_client>(this);
}
std::unique_ptr<SkBreakIterator> SkUnicode_libgrapheme::makeBreakIterator(BreakType breakType) {
    return std::make_unique<SkBreakIterator_client>(this);
}
std::unique_ptr<SkUnicode> SkUnicode::Make() {
    return std::make_unique<SkUnicode_libgrapheme>();
}
std::unique_ptr<SkUnicode> SkUnicode::Make(SkSpan<char> text,
                                           std::vector<SkUnicode::BidiRegion> bidiRegions,
                                           std::vector<SkUnicode::Position> words,
                                           std::vector<SkUnicode::Position> graphemeBreaks,
                                           std::vector<SkUnicode::LineBreakBefore> lineBreaks) {
    SkASSERT(false);
    return nullptr;
}