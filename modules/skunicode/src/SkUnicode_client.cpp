/*
* Copyright 2022 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "include/core/SkString.h"
#include "include/private/SkMutex.h"
#include "include/private/SkOnce.h"
#include "include/private/SkTFitsIn.h"
#include "include/private/SkTHash.h"
#include "include/private/SkTemplates.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/utils/SkUTF.h"
#include <vector>
#include <functional>

class SkBidiIterator_client;
class SkBreakIterator_client;
class SkScriptIterator_client;
class SkUnicode_client : public SkUnicode {
public:
    ~SkUnicode_client() override = default;

    void reset() {
        fPrefilled = false;
        fText8 = SkSpan<const char>(nullptr, 0);
        fText16 = SkSpan<const char16_t>(nullptr, 0);
        fBidiRegions.clear();
        fLineBreaks.clear();
        fGraphemes.clear();
        fUTF8IndexForUTF16Index.reset();
        fUTF16IndexForUTF8Index.reset();
    }

    void prefill(const SkString& text,
                 std::vector<SkUnicode::BidiRegion> bidiRegions,
                 std::vector<SkUnicode::Position> grapemes,
                 std::vector<SkUnicode::LineBreakBefore> lineBreaks) {
        fBidiRegions = std::move(bidiRegions);
        fGraphemes = std::move(grapemes);
        fLineBreaks = std::move(lineBreaks);
        extractMapping(text,
                       [&](size_t index) {
                            fUTF8IndexForUTF16Index.emplace_back(index);
                       },
                       [&](size_t index) {
                            fUTF16IndexForUTF8Index.emplace_back(index);
                       });
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
    std::unique_ptr<SkScriptIterator> makeScriptIterator() override;

    // For SkParagraph
    bool getBidiRegions(const char utf8[],
                        int utf8Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) override {
        if (fPrefilled) {
            SkASSERT(utf8Units == fText8.size() && strncmp(utf8, fText8.data(), utf8Units) == 0);
            *results = fBidiRegions;
            return true;
        } else {
            return false;
        }
    }

    bool getAllCodepoints(const char utf8[],
                          int utf8Units,
                          bool replaceTabs,
                          SkTArray<SkUnicode::CodeUnitFlags, true>* results) override {
        if (fPrefilled) {
            SkASSERT(utf8Units == fText8.size() && strncmp(utf8, fText8.data(), utf8Units) == 0);
            *results = this->mergeCodeUniFlags();
            return true;
        } else {
            return false;
        }
    }

    bool getAllCodepoints(const char16_t utf16[],
                          int utf16Units,
                          bool replaceTabs,
                          SkTArray<SkUnicode::CodeUnitFlags, true>* results) override {
        if (fPrefilled) {
            std::u16string str(utf16, utf16Units);
            SkASSERT(utf16Units == fText16.size() && str.compare(fText16.data()) == 0);
            *results = this->mergeCodeUniFlags();
            return true;
        } else {
            return false;
        }
    }

    bool getWords(const char utf8[], int utf8Units, std::vector<Position>* results) override {
        if (fPrefilled) {
            SkASSERT(utf8Units == fText8.size() && strncmp(utf8, fText8.data(), utf8Units) == 0);
            *results = fWords;
            return true;
        } else {
            return false;
        }
    }

    SkString toUpper(const SkString& str) override {
        SkASSERT(false);
        return SkString(fText8.data(), fText8.size());
    }

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override {
        // We have to get this information from the client
        // as a separate step after text layout
        SkASSERT(false);
    }

private:
    friend class SkBidiIterator_client;
    friend class SkBreakIterator_client;

    SkTArray<CodeUnitFlags, true> mergeCodeUniFlags() {
        SkTArray<CodeUnitFlags, true> result;
        result.push_back_n(fText8.size(), CodeUnitFlags::kNoCodeUnitFlag);
        for (auto& lineBreak : fLineBreaks) {
            result[lineBreak.pos] |=
                lineBreak.breakType == LineBreakType::kHardLineBreak
                    ? CodeUnitFlags::kHardLineBreakBefore
                    : CodeUnitFlags::kSoftLineBreakBefore;
        }
        for (auto& grapheme : fGraphemes) {
            result[grapheme] |= CodeUnitFlags::kGraphemeStart;
        }
        // TODO: add tabulation, control, whitespaces, intrawhitespaces
        return result;
    }

    bool fPrefilled = false;
    SkSpan<const char> fText8;
    SkSpan<const char16_t> fText16;
    std::vector<BidiRegion> fBidiRegions;
    std::vector<Position> fWords;
    std::vector<LineBreakBefore> fLineBreaks;
    std::vector<Position> fGraphemes;
    SkTArray<size_t, true> fUTF8IndexForUTF16Index;
    SkTArray<size_t, true> fUTF16IndexForUTF8Index;
};

class SkBidiIterator_client : public SkBidiIterator {
    SkUnicode_client* fUnicode;
public:
    explicit SkBidiIterator_client(SkUnicode_client* unicode) : fUnicode(unicode) { }
    Position getLength() override { return fUnicode->fBidiRegions.size(); }
    Level getLevelAt(Position pos) override {
        auto found = std::lower_bound(
                fUnicode->fBidiRegions.begin(),
                fUnicode->fBidiRegions.end(),
                SkUnicode::BidiRegion(pos, pos, 0),
                [](const SkUnicode::BidiRegion& a, const SkUnicode::BidiRegion& b) {
                    return a.start <= b.start && a.end <= b.end;
                });
        return found->level;
    }
};

class SkBreakIterator_client: public SkBreakIterator {
    SkUnicode_client* fUnicode;
    Position fLastResult;
    Position fStart;
    Position fEnd;

public:
    explicit SkBreakIterator_client(SkUnicode_client* unicode) : fUnicode(unicode) { }

    Position first() override
      { return fUnicode->fLineBreaks[fStart + (fLastResult = 0)].pos; }
    Position current() override
      { return fUnicode->fLineBreaks[fStart + fLastResult].pos; }
    Position next() override
      { return fUnicode->fLineBreaks[fStart + fLastResult + 1].pos; }
    bool isDone() override { return fStart + fLastResult == fEnd; }

    bool setText(const char utftext8[], int utf8Units) override {
        SkASSERT(utftext8 >= fUnicode->fText8.data() && utf8Units <= fUnicode->fText8.size());
        fStart = utftext8 - fUnicode->fText8.data();
        fEnd = fStart + utf8Units;
        fLastResult = 0;
        return true;
    }

    bool setText(const char16_t utftext16[], int utf16Units) override {
        SkASSERT(utftext16 >= fUnicode->fText16.data() && utf16Units <= fUnicode->fText16.size());
        fStart = utftext16 - fUnicode->fText16.data();
        fEnd = fStart + utf16Units;
        fLastResult = 0;
        return true;
    }
};

void SkBidiIterator::ReorderVisual(const Level runLevels[],
                                   int levelsCount,
                                   int32_t logicalFromVisual[]) {
    SkASSERT(false);
}

std::unique_ptr<SkBidiIterator> SkUnicode_client::makeBidiIterator(const uint16_t text[], int count,
                                                 SkBidiIterator::Direction dir) {
    return std::make_unique<SkBidiIterator_client>(this);
}
std::unique_ptr<SkBidiIterator> SkUnicode_client::makeBidiIterator(const char text[],
                                                 int count,
                                                 SkBidiIterator::Direction dir) {
    return std::make_unique<SkBidiIterator_client>(this);
}

std::unique_ptr<SkBreakIterator> SkUnicode_client::makeBreakIterator(const char locale[],
                                                   BreakType breakType) {
    return std::make_unique<SkBreakIterator_client>(this);
}
std::unique_ptr<SkBreakIterator> SkUnicode_client::makeBreakIterator(BreakType breakType) {
    return std::make_unique<SkBreakIterator_client>(this);
}

std::unique_ptr<SkScriptIterator> SkUnicode_client::makeScriptIterator() {
    return nullptr;
}

std::unique_ptr<SkUnicode> SkUnicode::Make() {
    return std::make_unique<SkUnicode_client>();
}
