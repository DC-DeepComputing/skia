                                           /*
* Copyright 2024 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/src/SkUnicode_hybrid.h"
#include "modules/skunicode/src/SkUnicode_icu.h"
#include "modules/skunicode/src/SkUnicode_libgrapheme.h"
#include "modules/skunicode/src/SkUnicode_hardcoded.h"
#include "modules/skunicode/src/SkUnicode_icu_bidi.h"
#include "src/base/SkBitmaskEnum.h"
#include "src/base/SkUTF.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <array>
#include <unicode/ubidi.h>
#include <unicode/ubrk.h>
#include <unicode/uchar.h>
#include <unicode/uloc.h>
#include <unicode/uscript.h>
#include <unicode/ustring.h>
#include <unicode/utext.h>
#include <unicode/utypes.h>

using namespace skia_private;

// A combination of libgrapheme and icu
class SkUnicode_hybrid : public SkUnicodeHardCodedCharProperties {
public:
    SkUnicode_hybrid()
            : fIcu(MakeIcuBasedUnicode())
            , fLibgrapheme(MakeLibgraphemeBasedUnicode()){ }

    std::unique_ptr<SkUnicode> copy() override {
        return std::make_unique<SkUnicode_hybrid>();
    }

    ~SkUnicode_hybrid() override = default;

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
        return SkUnicode_IcuBidi::ExtractBidi(utf8, utf8Units, dir, results);
    }

    bool getUtf8Words(const char utf8[],
                      int utf8Units,
                      const char* locale,
                      std::vector<Position>* results) override {
        return fLibgrapheme->getWords(utf8, utf8Units, locale, results);
    }

    bool getSentences(const char utf8[],
                      int utf8Units,
                      const char* locale,
                      std::vector<SkUnicode::Position>* results) override {
        SkDEBUGF("Method 'getSentences' is not implemented\n");
        return false;
    }

    bool computeCodeUnitFlags(char utf8[],
                              int utf8Units,
                              bool replaceTabs,
                              TArray<SkUnicode::CodeUnitFlags, true>* results) override {
        results->clear();
        results->push_back_n(utf8Units + 1, CodeUnitFlags::kNoCodeUnitFlag);



        return true;
    }

    bool computeCodeUnitFlags(char16_t utf16[], int utf16Units, bool replaceTabs,
                          TArray<SkUnicode::CodeUnitFlags, true>* results) override {
        results->clear();
        results->push_back_n(utf16Units + 1, CodeUnitFlags::kNoCodeUnitFlag);

        return true;
    }

    bool getWords(const char utf8[], int utf8Units, const char* locale, std::vector<Position>* results) override {
        return fLibgrapheme->getWords(utf8, utf8Units, locale,results);
    }

    SkString toUpper(const SkString& str) override {
        return fLibgrapheme->toUpper(str);
    }

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override {
        SkUnicode_IcuBidi::bidi_reorderVisual(runLevels, levelsCount, logicalFromVisual);
    }
private:
    std::unique_ptr<SkUnicode>  fIcu;
    std::unique_ptr<SkUnicode> fLibgrapheme;
};

class SkBreakIterator_client: public SkBreakIterator {
    std::shared_ptr<SkUnicode_client::Data> fData;
    Position fLastResult;
    Position fStart;
    Position fEnd;
public:
    explicit SkBreakIterator_client(std::shared_ptr<SkUnicode_client::Data> data) : fData(data) { }
    Position first() override
      { return fData->fLineBreaks[fStart + (fLastResult = 0)].pos; }
    Position current() override
      { return fData->fLineBreaks[fStart + fLastResult].pos; }
    Position next() override
      { return fData->fLineBreaks[fStart + fLastResult + 1].pos; }
    Status status() override {
        return fData->fLineBreaks[fStart + fLastResult].breakType ==
                       SkUnicode::LineBreakType::kHardLineBreak
                       ? SkUnicode::CodeUnitFlags::kHardLineBreakBefore
                       : SkUnicode::CodeUnitFlags::kSoftLineBreakBefore;
    }
    bool isDone() override { return fStart + fLastResult == fEnd; }
    bool setText(const char utftext8[], int utf8Units) override {
        SkASSERT(utftext8 >= fData->fText8.data() &&
                 utf8Units <= SkToS16(fData->fText8.size()));
        fStart = utftext8 - fData->fText8.data();
        fEnd = fStart + utf8Units;
        fLastResult = 0;
        return true;
    }
    bool setText(const char16_t utftext16[], int utf16Units) override {
        SkASSERT(utftext16 >= fData->fText16.data() &&
                 utf16Units <= SkToS16(fData->fText16.size()));
        fStart = utftext16 - fData->fText16.data();
        fEnd = fStart + utf16Units;
        fLastResult = 0;
        return true;
    }
};
std::unique_ptr<SkBidiIterator> SkUnicode_client::makeBidiIterator(const uint16_t text[], int count,
                                                 SkBidiIterator::Direction dir) {
    return SkUnicode_IcuBidi::MakeIterator(text, count, dir);
}
std::unique_ptr<SkBidiIterator> SkUnicode_client::makeBidiIterator(const char text[],
                                                 int count,
                                                 SkBidiIterator::Direction dir) {
    return SkUnicode_IcuBidi::MakeIterator(text, count, dir);
}
std::unique_ptr<SkBreakIterator> SkUnicode_client::makeBreakIterator(const char locale[],
                                                   BreakType breakType) {
    return std::make_unique<SkBreakIterator_client>(fData);
}
std::unique_ptr<SkBreakIterator> SkUnicode_client::makeBreakIterator(BreakType breakType) {
    return std::make_unique<SkBreakIterator_client>(fData);
}

std::unique_ptr<SkUnicode> SkUnicode::MakeClientBasedUnicode(
        SkSpan<char> text,
        std::vector<SkUnicode::Position> words,
        std::vector<SkUnicode::Position> graphemeBreaks,
        std::vector<SkUnicode::LineBreakBefore> lineBreaks) {
    return std::make_unique<SkUnicode_client>(text, std::move(words), std::move(graphemeBreaks),
                                              std::move(lineBreaks));
}

