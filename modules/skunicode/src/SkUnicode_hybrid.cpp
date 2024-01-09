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
#include "modules/skunicode/src/SkUnicode_hardcoded.h"
#include "modules/skunicode/src/SkUnicode_hybrid.h"
#include "modules/skunicode/src/SkUnicode_icu.h"
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
    struct ScriptRange {
        ScriptRange(UScriptCode c, size_t s, size_t e)
          : code(c), start(s), end(e) { }
        UScriptCode code;
        size_t start;
        size_t end;
    };

    SkUnicode_hybrid(std::vector<UScriptCode> forIcu)
        : fIcu(MakeIcuBasedUnicode())
        , fLibgrapheme(MakeLibgraphemeBasedUnicode())
        , fForIcu(forIcu) {}

    std::unique_ptr<SkUnicode> copy() override {
      return std::make_unique<SkUnicode_hybrid>(fForIcu);
    }

    ~SkUnicode_hybrid() override = default;

    bool isEmoji(SkUnichar utf8) override {
        return fIcu->isEmoji(utf8);
    }
    bool isEmojiComponent(SkUnichar utf8) override {
        return fIcu->isEmojiComponent(utf8);
    }
    bool isEmojiModifierBase(SkUnichar utf8) override {
        return fIcu->isEmojiModifierBase(utf8);
    }
    bool isEmojiModifier(SkUnichar utf8) override {
        return fIcu->isEmojiModifier(utf8);
    }
    bool isRegionalIndicator(SkUnichar utf8) override {
        return fIcu->isRegionalIndicator(utf8);
    }

    // For SkShaper
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[],
                                                     int count,
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

    bool breakTextIntoScriptRanges(SkSpan<const char> text, std::vector<ScriptRange>* ranges) {
        auto currentChar = text.begin();
        while (currentChar < text.end()) {
            const char* prevChar = currentChar;
            UErrorCode status = U_ZERO_ERROR;
            SkUnichar unichar = SkUTF::NextUTF8(&currentChar, text.end());
            auto script = uscript_getScript(unichar, &status);
            if (ranges->empty() || script != ranges->back().code) {
                if (script == UScriptCode::USCRIPT_INHERITED ||
                    script == UScriptCode::USCRIPT_COMMON) {
                    continue;
                }

                if (!ranges->empty()) {
                    if (ranges->back().code == UScriptCode::USCRIPT_INHERITED ||
                        ranges->back().code == UScriptCode::USCRIPT_COMMON) {
                        ranges->back().code = script;
                        continue;
                    }
                    ranges->back().end = prevChar - text.begin();
                }

                ranges->emplace_back(script, prevChar - text.begin(), currentChar - text.begin());
                currentChar = prevChar;
            }
        }
        if (ranges->empty()) {
            ranges->emplace_back(UScriptCode::USCRIPT_COMMON, 0, text.size());
        } else {
            ranges->back().end = text.size();
        }

        return true;
  }

    bool computeCodeUnitFlags(char utf8[],
                              int utf8Units,
                              bool replaceTabs,
                              TArray<SkUnicode::CodeUnitFlags, true>* results) override {
      results->clear();
      results->push_back_n(utf8Units + 1, CodeUnitFlags::kNoCodeUnitFlag);

      std::vector<ScriptRange> ranges;
      if (!this->breakTextIntoScriptRanges(SkSpan<const char>(utf8, utf8Units), &ranges)) {
          return false;
      }
      for (auto& range : ranges) {
          SkSpan<char> rangeText(utf8 + range.start, range.end - range.start);
          TArray<SkUnicode::CodeUnitFlags, true> rangeResults;
          if (this->isForIcu(range.code)) {
              if (!fIcu->computeCodeUnitFlags
                    (rangeText.data(), rangeText.size(), false, &rangeResults)) {
                  return false;
              }
          } else {
              if (!fLibgrapheme->computeCodeUnitFlags
                    (rangeText.data(), rangeText.size(), false, &rangeResults)) {
                  return false;
              }
          }
          for (auto i = range.start; i < range.end; ++i) {
              (*results)[i] = rangeResults[i - range.start];
          }
          if (range.end == utf8Units) {
              results->back() = rangeResults.back();
          }
      }

      return true;
    }

  bool computeCodeUnitFlags(char16_t utf16[], int utf16Units, bool replaceTabs,
                            TArray<SkUnicode::CodeUnitFlags,
                                   true>* results) override {
    SkDEBUGF("Method 'computeCodeUnitFlags' for utf16 is not implemented\n");
    return false;
  }

  bool getWords(const char utf8[],
                int utf8Units,
                const char* locale,
                std::vector<Position>* results) override {
    return fLibgrapheme->getWords(utf8, utf8Units, locale, results);
  }

  SkString toUpper(const SkString& str) override {
    return fLibgrapheme->toUpper(str);
  }

  void reorderVisual(const BidiLevel runLevels[],
                     int levelsCount,
                     int32_t logicalFromVisual[]) override {
    SkUnicode_IcuBidi::bidi_reorderVisual(runLevels,
                                          levelsCount,
                                          logicalFromVisual);
  }

    bool isForIcu(UScriptCode code) {
        return std::find(fForIcu.begin(), fForIcu.end(), code) != fForIcu.end();
    }
 private:
    friend class SkBreakIterator_hybrid;
    std::unique_ptr<SkUnicode> fIcu;
    std::unique_ptr<SkUnicode> fLibgrapheme;
    std::vector<UScriptCode> fForIcu;
};

class SkBreakIterator_hybrid : public SkBreakIterator {
    friend class SkUnicode_hybrid;
    SkUnicode_hybrid* fHybrid;
    size_t fCurrentRange;
    SkSpan<const char> fText;
    std::unique_ptr<SkBreakIterator> fBreakIterator;
    SkUnicode::BreakType fBreakType;
    std::vector<SkUnicode_hybrid::ScriptRange> fScriptRanges;
 public:
    SkBreakIterator_hybrid(SkUnicode_hybrid* hybrid, SkUnicode::BreakType type)
        : fHybrid(hybrid),
          fCurrentRange(0),
          fText(),
          fBreakIterator(nullptr),
          fBreakType(type) {
    }

  Position first() override {
    return fBreakIterator->first();
  }
  Position current() override {
    return fBreakIterator->current();
  }
  Position next() override {
    this->consume();
    return fBreakIterator->next();
  }
  Status status() override {
    return fBreakIterator->status();
  }
  bool isDone() override {
    return fBreakIterator->isDone();
  }

  bool setText(const char utftext8[], int utf8Units) override {
      fText = SkSpan<const char>(utftext8, utf8Units);
      fHybrid->breakTextIntoScriptRanges(SkSpan<const char>(utftext8, utf8Units),
                                         &fScriptRanges);
      return this->consume();
  }

  bool setText(const char16_t utftext16[],
               int utf16Units) override { SkDEBUGFAIL("Not implemented"); }

  bool consume() {
      auto& scriptRange = fScriptRanges[fCurrentRange];
      if (fBreakIterator == nullptr) {
          fCurrentRange = 0;
      } else if (fBreakIterator->isDone()) {
          ++fCurrentRange;
      } else {
          return true;
      }
      // Go to the next range
      if (fCurrentRange >= fScriptRanges.size()) {
          return false;
      }

      // Switch the internal iterator if required
      if (fHybrid->isForIcu(scriptRange.code)) {
          fBreakIterator = fHybrid->fIcu->makeBreakIterator(nullptr, fBreakType);
      } else {
          fBreakIterator = fHybrid->fLibgrapheme->makeBreakIterator(nullptr, fBreakType);
      }
      if (!fBreakIterator->setText(fText.begin() + scriptRange.start,scriptRange.end - scriptRange.start)) {
          return false;
      }

      return true;
  }
};

std::unique_ptr<SkBidiIterator> SkUnicode_hybrid::makeBidiIterator(const uint16_t text[],
                                                                   int count,
                                                                   SkBidiIterator::Direction dir) {
  return SkUnicode_IcuBidi::MakeIterator(text, count, dir);
}
std::unique_ptr<SkBidiIterator> SkUnicode_hybrid::makeBidiIterator(const char text[],
                                                                   int count,
                                                                   SkBidiIterator::Direction dir) {
  return SkUnicode_IcuBidi::MakeIterator(text, count, dir);
}
std::unique_ptr<SkBreakIterator> SkUnicode_hybrid::makeBreakIterator(const char locale[],
                                                                     BreakType breakType) {
    return std::make_unique<SkBreakIterator_hybrid>(this, breakType);
}
std::unique_ptr<SkBreakIterator> SkUnicode_hybrid::makeBreakIterator(BreakType breakType) {
    return std::make_unique<SkBreakIterator_hybrid>(this, breakType);
}
std::unique_ptr<SkUnicode> MakeHybridBasedUnicode(std::vector<UScriptCode> forIcu) {
    return std::make_unique<SkUnicode_hybrid>(forIcu);
}

std::vector<std::pair<bool, size_t>> breakTextIntoRanges(SkUnicode* unicode, SkSpan<const char> text) {
    SkUnicode_hybrid* hybrid = (SkUnicode_hybrid*)unicode;
      std::vector<SkUnicode_hybrid::ScriptRange> scriptRanges;
    if (!hybrid->breakTextIntoScriptRanges(text, &scriptRanges)) {
        return {};
    }
    std::vector<std::pair<bool, size_t>> results;
    for (auto& r : scriptRanges) {
        results.emplace_back(hybrid->isForIcu(r.code), r.start);
    }
    return results;
}

std::unique_ptr<SkUnicode> SkUnicode::MakeHybridBasedUnicode() {
    return ::MakeHybridBasedUnicode({
          UScriptCode::USCRIPT_THAI,
          UScriptCode::USCRIPT_LAO,
          UScriptCode::USCRIPT_MYANMAR,
          UScriptCode::USCRIPT_KHMER,
          UScriptCode::USCRIPT_HANGUL,
          UScriptCode::USCRIPT_HIRAGANA,
          UScriptCode::USCRIPT_KATAKANA,
          UScriptCode::USCRIPT_HAN,
      });
}
