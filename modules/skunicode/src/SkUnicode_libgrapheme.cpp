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
#include "include/private/base/SkTArray.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/src/SkUnicode_icu_bidi.h"
extern "C" {
#include <grapheme.h>
}
#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#undef LEN
#define LEN(x) (sizeof(x) / sizeof(*(x)))

using namespace skia_private;

#ifndef SK_UNICODE_ICU_IMPLEMENTATION

const char* SkUnicode_IcuBidi::errorName(UErrorCode status) {
    return u_errorName_skia(status);
}
void SkUnicode_IcuBidi::bidi_close(UBiDi* bidi) {
    ubidi_close_skia(bidi);
}
UBiDiDirection SkUnicode_IcuBidi::bidi_getDirection(const UBiDi* bidi) {
    return ubidi_getDirection_skia(bidi);
}
SkBidiIterator::Position SkUnicode_IcuBidi::bidi_getLength(const UBiDi* bidi) {
    return ubidi_getLength_skia(bidi);
}
SkBidiIterator::Level SkUnicode_IcuBidi::bidi_getLevelAt(const UBiDi* bidi, int pos) {
    return ubidi_getLevelAt_skia(bidi, pos);
}
UBiDi* SkUnicode_IcuBidi::bidi_openSized(int32_t maxLength, int32_t maxRunCount, UErrorCode* pErrorCode) {
    return ubidi_openSized_skia(maxLength, maxRunCount, pErrorCode);
}
void SkUnicode_IcuBidi::bidi_setPara(UBiDi* bidi,
                         const UChar* text,
                         int32_t length,
                         UBiDiLevel paraLevel,
                         UBiDiLevel* embeddingLevels,
                         UErrorCode* status) {
    return ubidi_setPara_skia(bidi, text, length, paraLevel, embeddingLevels, status);
}
void SkUnicode_IcuBidi::bidi_reorderVisual(const SkUnicode::BidiLevel runLevels[],
                               int levelsCount,
                               int32_t logicalFromVisual[]) {
    ubidi_reorderVisual_skia(runLevels, levelsCount, logicalFromVisual);
}
#endif

class SkUnicode_libgrapheme : public SkUnicode {
public:
    SkUnicode_libgrapheme() { }

    ~SkUnicode_libgrapheme() override = default;

    std::unique_ptr<SkUnicode> copy() override {
        return std::make_unique<SkUnicode_libgrapheme>();
    }

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

        // TODO: Is this the right way to handle this?
        if (count > utf8Units) {
            SkDebugf("COUNT EXCEEDED!\n");
            return false;
        }

        BidiRegion region(0, 0, -1);
        const char* current = utf8;
        const char* end = utf8 + utf8Units;
        int codePointIndex = 0;
        while (true) {
            auto level = current == end ? -1 : SkToS16(res32[codePointIndex]);
            if (codePointIndex == 0) {
                region.start = region.end;
                region.level = level;
            } else if (region.level != level) {
                int utf8Index = current - utf8;
                region.end = utf8Index;
                results->emplace_back(region);
                region.start = region.end;
                region.level = level;
            }

            if (SkUTF::NextUTF8(&current, end) == -1) {
                // We reached the end of the string.
                break;
            }
            codePointIndex++;
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
        static constexpr std::array<SkUnichar, 21> whitespaces {
                0x0009, // character tabulation
                0x000A, // line feed
                0x000B, // line tabulation
                0x000C, // form feed
                0x000D, // carriage return
                0x0020, // space
              //0x0085, // next line
              //0x00A0, // no-break space
                0x1680, // ogham space mark
                0x2000, // en quad
                0x2001, // em quad
                0x2002, // en space
                0x2003, // em space
                0x2004, // three-per-em space
                0x2005, // four-per-em space
                0x2006, // six-per-em space
              //0x2007, // figure space
                0x2008, // punctuation space
                0x2009, // thin space
                0x200A, // hair space
                0x2028, // line separator
                0x2029, // paragraph separator
              //0x202F, // narrow no-break space
                0x205F, // medium mathematical space
                0x3000};// ideographic space
        return std::find(whitespaces.begin(), whitespaces.end(), unichar) != whitespaces.end();
    }

    static bool isSpace(SkUnichar unichar) {
        static constexpr std::array<SkUnichar, 25> spaces {
                0x0009, // character tabulation
                0x000A, // line feed
                0x000B, // line tabulation
                0x000C, // form feed
                0x000D, // carriage return
                0x0020, // space
                0x0085, // next line
                0x00A0, // no-break space
                0x1680, // ogham space mark
                0x2000, // en quad
                0x2001, // em quad
                0x2002, // en space
                0x2003, // em space
                0x2004, // three-per-em space
                0x2005, // four-per-em space
                0x2006, // six-per-em space
                0x2007, // figure space
                0x2008, // punctuation space
                0x2009, // thin space
                0x200A, // hair space
                0x2028, // line separator
                0x2029, // paragraph separator
                0x202F, // narrow no-break space
                0x205F, // medium mathematical space
                0x3000}; // ideographic space
        return std::find(spaces.begin(), spaces.end(), unichar) != spaces.end();
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
                              skia_private::TArray<SkUnicode::CodeUnitFlags, true>* results) override {
        results->clear();
        results->push_back_n(utf8Units + 1, CodeUnitFlags::kNoCodeUnitFlag);

        size_t lineBreak = 0;
        (*results)[lineBreak] |= CodeUnitFlags::kSoftLineBreakBefore;
        while (lineBreak < utf8Units) {
            lineBreak += grapheme_next_line_break_utf8(utf8 + lineBreak, utf8Units - lineBreak);
            // Check if the previous code unit is a hard break.
            auto codePoint = utf8[lineBreak - 1];
            (*results)[lineBreak] |= this->isHardBreak(codePoint)
                                    ? CodeUnitFlags::kHardLineBreakBefore
                                    : CodeUnitFlags::kSoftLineBreakBefore;
        }
        (*results)[utf8Units] |= CodeUnitFlags::kSoftLineBreakBefore;

        size_t graphemeBreak = 0;
        (*results)[graphemeBreak] |= CodeUnitFlags::kGraphemeStart;
        while (graphemeBreak < utf8Units) {
            graphemeBreak += grapheme_next_character_break_utf8(utf8 + graphemeBreak, utf8Units - graphemeBreak);
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
                          skia_private::TArray<SkUnicode::CodeUnitFlags, true>* results) override {
        SkASSERT(false);
        return false;
    }

    bool getWords(const char utf8[], int utf8Units, const char* locale, std::vector<Position>* results) override {
        size_t wordBreak = 0;
        while (wordBreak < utf8Units) {
            wordBreak += grapheme_next_word_break_utf8(utf8 + wordBreak, utf8Units - wordBreak);
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
        SkUnicode_IcuBidi::bidi_reorderVisual(runLevels, levelsCount, logicalFromVisual);
    }
private:
    friend class SkBidiIterator_libgrapheme;
    friend class SkBreakIterator_libgrapheme;
};

class SkBidiIterator_libgrapheme : public SkBidiIterator {
    std::vector<SkUnicode::BidiRegion> fBidiRegions;
public:
    explicit SkBidiIterator_libgrapheme(SkUnicode_libgrapheme* unicode,
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

class SkBreakIterator_libgrapheme: public SkBreakIterator {
    SkUnicode_libgrapheme* fUnicode;
    std::vector<SkUnicode::LineBreakBefore> fLineBreaks;
    Position fLastResult;
    Position fStart;
    Position fEnd;
public:
    explicit SkBreakIterator_libgrapheme(SkUnicode_libgrapheme* unicode) : fUnicode(unicode) { }
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
    return std::make_unique<SkBidiIterator_libgrapheme>(this, text, count, dir);
}
std::unique_ptr<SkBreakIterator> SkUnicode_libgrapheme::makeBreakIterator(const char locale[],
                                                   BreakType breakType) {
    return std::make_unique<SkBreakIterator_libgrapheme>(this);
}
std::unique_ptr<SkBreakIterator> SkUnicode_libgrapheme::makeBreakIterator(BreakType breakType) {
    return std::make_unique<SkBreakIterator_libgrapheme>(this);
}
std::unique_ptr<SkUnicode> SkUnicode::MakeLibgraphemeBasedUnicode() {
    return std::make_unique<SkUnicode_libgrapheme>();
}
