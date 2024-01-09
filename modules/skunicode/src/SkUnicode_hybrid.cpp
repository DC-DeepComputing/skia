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
#include "modules/skunicode/src/SkUnicode_hardcoded.h"
#include "modules/skunicode/src/SkUnicode_icu_bidi.h"
#include "modules/skunicode/src/SkUnicode_icu.h"
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
      : fIcu(MakeIcuBasedUnicode()),
        fLibgrapheme(MakeLibgraphemeBasedUnicode()) {}

  std::unique_ptr<SkUnicode> copy() override {
    return std::make_unique<SkUnicode_hybrid>();
  }

  ~SkUnicode_hybrid() override = default;

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

  bool computeCodeUnitFlags(char utf8[],
                            int utf8Units,
                            bool replaceTabs,
                            TArray<SkUnicode::CodeUnitFlags,
                                   true>* results) override {
    results->clear();
    results->push_back_n(utf8Units + 1, CodeUnitFlags::kNoCodeUnitFlag);

    return true;
  }

  bool computeCodeUnitFlags(char16_t utf16[], int utf16Units, bool replaceTabs,
                            TArray<SkUnicode::CodeUnitFlags,
                                   true>* results) override {
    results->clear();
    results->push_back_n(utf16Units + 1, CodeUnitFlags::kNoCodeUnitFlag);

    return true;
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
 private:
  std::unique_ptr<SkUnicode> fIcu;
  std::unique_ptr<SkUnicode> fLibgrapheme;
};

class SkBreakIterator_hybrid : public SkBreakIterator {
    std::unique_ptr<SkUnicode> fIcu;
    std::unique_ptr<SkUnicode> fLibgrapheme;
    const char* fCurrent;
    const char* fBegin;
    const char* fEnd;
    Position fRangeStart;
    Position fRangeEnd;
    std::unique_ptr<SkBreakIterator> fBreakIterator;
    UScriptCode fCurrentScript;
    const UScriptCode FOR_ICU[1] = {UScriptCode::USCRIPT_THAI};
    SkUnicode::BreakType fBreakType;
 public:
    SkBreakIterator_hybrid(SkUnicode::BreakType type)
        : fIcu(SkUnicode::MakeIcuBasedUnicode()),
          fLibgrapheme(SkUnicode::MakeLibgraphemeBasedUnicode()),
          fCurrent(nullptr),
          fBegin(nullptr),
          fEnd(nullptr),
          fBreakIterator(nullptr),
          fCurrentScript(USCRIPT_INVALID_CODE)
          , fBreakType(type) {
        this->consume();
    }

  bool isForIcu(UScriptCode script) {
      return script == UScriptCode::USCRIPT_THAI;
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
      fBegin = utftext8;
      fCurrent = fBegin;
      fEnd = fBegin + utf8Units;
      return this->consume();
  }

  bool setText(const char16_t utftext16[],
               int utf16Units) override { SkDEBUGFAIL("Not implemented"); }

  bool consume() {
      if (fRangeStart == fRangeEnd) {
          // Let's find the next range and start a new iterator over it
          fCurrent = fBegin + fRangeEnd;
          while (fCurrent < fEnd) {
            const char* prev = fCurrent;
            UErrorCode status;
            SkUnichar unichar = SkUTF::NextUTF8(&fCurrent, fEnd);
            auto script = getScript(unichar, &status);
            if (script != fCurrentScript) {
              if (fCurrentScript == UScriptCode::USCRIPT_INHERITED ||
                  fCurrentScript == UScriptCode::USCRIPT_COMMON) {
                fCurrentScript = script;
              } else if (script == UScriptCode::USCRIPT_INHERITED ||
                  script == UScriptCode::USCRIPT_COMMON) {
                continue;
              } else {
                fCurrent = prev;
                break;
              }
            }
          }
          if (fCurrentScript == UScriptCode::USCRIPT_INHERITED) {
            fCurrentScript = UScriptCode::USCRIPT_COMMON;
          }
          fRangeStart = fRangeEnd;
          fRangeEnd = fCurrent - fBegin;

          if (this->isForIcu(fCurrentScript)) {
              fBreakIterator = fIcu->makeBreakIterator(nullptr, fBreakType);
          } else {
              fBreakIterator = fLibgrapheme->makeBreakIterator(nullptr, fBreakType);
          }

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
    return std::make_unique<SkBreakIterator_hybrid>(breakType);
}
std::unique_ptr<SkBreakIterator> SkUnicode_hybrid::makeBreakIterator(BreakType breakType) {
    return std::make_unique<SkBreakIterator_hybrid>(breakType);
}

std::unique_ptr<SkUnicode> SkUnicode::MakeHybridBasedUnicode() {
  return std::make_unique<SkUnicode_hybrid>();
}

