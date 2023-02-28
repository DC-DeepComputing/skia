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
#include <ICU4XWordSegmenter.hpp>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class SkUnicode_icu4x : public SkUnicode {
public:
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
        SkASSERT(false);
        return false;
    }

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
        SkASSERT(false);
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
        const auto provider = ICU4XDataProvider::create_test();

        auto r1 = ICU4XWordSegmenter::create_dictionary(provider);
        const auto segmenter = ICU4XWordSegmenter::create_dictionary(provider).ok().value();
        std::string_view string_view(utf8);
        auto iterator = segmenter.segment_utf8(string_view);
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
        SkASSERT(false);
        return SkString();
    }

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override {
        SkASSERT(false);
    }
private:
    friend class SkBreakIterator_icu4x;
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

std::unique_ptr<SkBidiIterator> SkUnicode_icu4x::makeBidiIterator(const uint16_t text[], int count,
                                                 SkBidiIterator::Direction dir) {
    SkASSERT(false); return nullptr;
}
std::unique_ptr<SkBidiIterator> SkUnicode_icu4x::makeBidiIterator(const char text[],
                                                 int count,
                                                 SkBidiIterator::Direction dir) {
    SkASSERT(false); return nullptr;
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
