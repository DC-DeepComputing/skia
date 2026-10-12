/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkAssert.h"
#include "include/private/SkTArray.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/include/SkUnicode_apple.h"
#include "modules/skunicode/src/SkBidiFactory_icu_subset.h"
#include "modules/skunicode/src/SkUnicode_hardcoded.h"
#include "modules/skunicode/src/SkUnicode_icu_bidi.h"
#include "src/core/SkBitmaskEnum.h"
#include "src/core/SkUTF.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#include <CoreFoundation/CoreFoundation.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace skia_private;

namespace {

// Exclude CR and NEL from kCFCharacterSetNewline to match UAX #14 LF/BK mandatory breaks
// and prevent splitting CRLF pairs during codepoint iteration
bool is_hard_break(SkUnichar unichar) {
    if (unichar == '\r' || unichar == 0x0085) {
        return false;
    }
    return CFCharacterSetIsLongCharacterMember(
            CFCharacterSetGetPredefined(kCFCharacterSetNewline), unichar);
}

// Wrap caller-owned buffers with kCFAllocatorNull to avoid copying text when the encoding is valid
SkUniqueCFRef<CFStringRef> MakeCFStringNoCopy(const char utf8[], int utf8Units) {
    return SkUniqueCFRef<CFStringRef>(CFStringCreateWithBytesNoCopy(kCFAllocatorDefault,
                                                                    (const UInt8*)utf8,
                                                                    utf8Units,
                                                                    kCFStringEncodingUTF8,
                                                                    false,
                                                                    kCFAllocatorNull));
}

SkUniqueCFRef<CFStringRef> MakeCFStringNoCopy(const char16_t utf16[], int utf16Units) {
    return SkUniqueCFRef<CFStringRef>(CFStringCreateWithCharactersNoCopy(
            kCFAllocatorDefault, (const UniChar*)utf16, utf16Units, kCFAllocatorNull));
}

// Returning nullptr instructs CoreFoundation to use the default locale without allocating
SkUniqueCFRef<CFLocaleRef> MakeCFLocale(const char* locale) {
    if (locale == nullptr || locale[0] == '\0') {
        return nullptr;
    }
    SkUniqueCFRef<CFStringRef> locStr(
            CFStringCreateWithCString(kCFAllocatorDefault, locale, kCFStringEncodingUTF8));
    if (!locStr) {
        return nullptr;
    }
    return SkUniqueCFRef<CFLocaleRef>(CFLocaleCreate(kCFAllocatorDefault, locStr.get()));
}

// CoreFoundation reports boundaries in UTF-16 code units; populate only the UTF-16-to-UTF-8
// direction into inline stack storage to map CFRange offsets back to UTF-8 byte indices
void ExtractUtf16ToUtf8Mapping(const char utf8[], int utf8Units, STArray<256, size_t>* u16ToU8) {
    SkUnicode::extractUtfConversionMapping(
            SkSpan<const char>(utf8, utf8Units),
            [&](size_t u8) { u16ToU8->push_back(u8); },
            [](size_t) {});
}

// Clamp UTF-16 indices to [0, utf8Units] so past-the-end CFRange endpoints map to the buffer end
SkUnicode::Position MapUtf16ToUtf8(const STArray<256, size_t>& u16ToU8,
                                   CFIndex pos16,
                                   int utf8Units) {
    if (pos16 <= 0) {
        return 0;
    }
    if (SkToInt(pos16) < u16ToU8.size()) {
        return SkTo<SkUnicode::Position>(u16ToU8[pos16]);
    }
    return utf8Units;
}

template <typename Callback>
bool Tokenize(CFStringRef str,
              CFIndex len16,
              CFOptionFlags unit,
              const char* locale,
              SkUniqueCFRef<CFStringTokenizerRef>* cachedTokenizer,
              Callback&& callback) {
    if (len16 <= 0 || !str) {
        return true;
    }
    // Reuse the caller's cached tokenizer on hot paths via CFStringTokenizerSetString, falling
    // back to a temporary local instance for one-off locale-specific queries
    SkUniqueCFRef<CFStringTokenizerRef> tempTokenizer;
    SkUniqueCFRef<CFStringTokenizerRef>& tokenizer =
            cachedTokenizer ? *cachedTokenizer : tempTokenizer;
    if (!tokenizer) {
        SkUniqueCFRef<CFLocaleRef> cfLoc = MakeCFLocale(locale);
        tokenizer.reset(CFStringTokenizerCreate(
                kCFAllocatorDefault, str, CFRangeMake(0, len16), unit, cfLoc.get()));
    } else {
        CFStringTokenizerSetString(tokenizer.get(), str, CFRangeMake(0, len16));
    }
    if (!tokenizer) {
        return false;
    }

    CFStringTokenizerTokenType tokenType = CFStringTokenizerAdvanceToNextToken(tokenizer.get());
    CFIndex guard = 0;
    const CFIndex maxSteps = 2 * len16 + 2;
    while (tokenType != kCFStringTokenizerTokenNone && ++guard <= maxSteps) {
        callback(CFStringTokenizerGetCurrentTokenRange(tokenizer.get()));
        tokenType = CFStringTokenizerAdvanceToNextToken(tokenizer.get());
    }

    if (cachedTokenizer) {
        // Unbind the temporary string to release non-owning byte references
        CFStringTokenizerSetString(tokenizer.get(), CFSTR(""), CFRangeMake(0, 0));
    }
    return true;
}

// Emit start-of-cluster and end-of-text UTF-16 offsets with guaranteed forward progress even on
// zero-length ranges from malformed surrogates
template <typename Callback>
void ForEachGraphemeBoundary(CFStringRef str, CFIndex len16, Callback&& callback) {
    CFIndex idx = 0;
    while (idx < len16) {
        CFRange r = CFStringGetRangeOfComposedCharactersAtIndex(str, idx);
        callback(r.location);
        CFIndex next = std::max(idx + 1, r.location + std::max<CFIndex>(r.length, 1));
        // CFStringGetRangeOfComposedCharactersAtIndex splits CRLF; coalesce per UAX #29 GB3
        if (next < len16 &&
            CFStringGetCharacterAtIndex(str, idx) == '\r' &&
            CFStringGetCharacterAtIndex(str, next) == '\n') {
            ++next;
        }
        idx = next;
    }
    callback(len16);
}

class SkBreakIterator_apple : public SkBreakIterator {
public:
    SkBreakIterator_apple(SkUnicode::BreakType type, const char* locale)
            : fType(type), fLocale(locale ? locale : "") {}

    ~SkBreakIterator_apple() override = default;

    Position first() override { return fBreaks.empty() ? 0 : fBreaks[(fIndex = 0)]; }
    Position current() override { return fBreaks.empty() ? 0 : fBreaks[fIndex]; }
    Position next() override {
        if (fIndex + 1 < SkTo<Position>(fBreaks.size())) {
            return fBreaks[++fIndex];
        }
        fIndex = fBreaks.size();
        return -1;
    }
    Status status() override {
        if (fIndex < SkTo<Position>(fStatuses.size())) {
            return fStatuses[fIndex];
        }
        return 0;
    }
    bool isDone() override { return fIndex >= SkTo<Position>(fBreaks.size()); }

    bool setText(const char utftext8[], int utf8Units) override {
        // Reject negative lengths or null non-empty buffers; empty inputs still emit a boundary
        // at 0 to match ICU BreakIterator::first() and SkParagraph empty-text expectations
        this->reset();
        if (utf8Units < 0 || (utf8Units > 0 && !utftext8)) {
            return false;
        }
        if (utf8Units == 0) {
            this->addBreak(0, fType == SkUnicode::BreakType::kLines
                                      ? SkUnicode::CodeUnitFlags::kSoftLineBreakBefore
                                      : 0);
            return true;
        }

        SkUniqueCFRef<CFStringRef> str = MakeCFStringNoCopy(utftext8, utf8Units);
        if (!str) {
            return false;
        }

        // Populate soft line, grapheme, word, or sentence boundaries mapped to UTF-8 byte offsets
        STArray<256, size_t> u16ToU8;
        ExtractUtf16ToUtf8Mapping(utftext8, utf8Units, &u16ToU8);
        if (!this->populateBreaks(str.get(),
                                  CFStringGetLength(str.get()),
                                  utf8Units,
                                  [&](CFIndex pos16) {
                                      return MapUtf16ToUtf8(u16ToU8, pos16, utf8Units);
                                  })) {
            return false;
        }

        // CFStringTokenizer does not distinguish mandatory breaks from soft wrap opportunities;
        // scan codepoints to tag positions after LF/BK characters with kHardLineBreakBefore
        if (fType == SkUnicode::BreakType::kLines) {
            const char* current = utftext8;
            const char* end = utftext8 + utf8Units;
            while (current < end) {
                SkUnichar unichar = SkUTF::NextUTF8(&current, end);
                // Normalize malformed sequences before querying CFCharacterSet
                if (unichar < 0) {
                    unichar = 0xFFFD;
                }
                if (is_hard_break(unichar)) {
                    this->addHardBreak(SkTo<Position>(current - utftext8));
                }
            }
        }
        return true;
    }

    bool setText(const char16_t utftext16[], int utf16Units) override {
        // Reject negative lengths or null non-empty buffers; empty inputs still emit a boundary
        // at 0 to match ICU BreakIterator::first() and SkParagraph empty-text expectations
        this->reset();
        if (utf16Units < 0 || (utf16Units > 0 && !utftext16)) {
            return false;
        }
        if (utf16Units == 0) {
            this->addBreak(0, fType == SkUnicode::BreakType::kLines
                                      ? SkUnicode::CodeUnitFlags::kSoftLineBreakBefore
                                      : 0);
            return true;
        }

        SkUniqueCFRef<CFStringRef> str = MakeCFStringNoCopy(utftext16, utf16Units);
        if (!str) {
            return false;
        }

        // Populate soft line, grapheme, word, or sentence boundaries directly in UTF-16 coordinates
        if (!this->populateBreaks(str.get(),
                                  utf16Units,
                                  utf16Units,
                                  [](CFIndex pos16) { return SkTo<Position>(pos16); })) {
            return false;
        }

        // Tag positions immediately following UAX #14 LF/BK codepoints with kHardLineBreakBefore
        if (fType == SkUnicode::BreakType::kLines) {
            const auto* u16 = reinterpret_cast<const uint16_t*>(utftext16);
            const uint16_t* current = u16;
            const uint16_t* end = u16 + utf16Units;
            while (current < end) {
                SkUnichar unichar = SkUTF::NextUTF16(&current, end);
                // Normalize unpaired surrogates before querying CFCharacterSet
                if (unichar < 0) {
                    unichar = 0xFFFD;
                }
                if (is_hard_break(unichar)) {
                    this->addHardBreak(SkTo<Position>(current - u16));
                }
            }
        }
        return true;
    }

private:
    void reset() {
        fBreaks.clear();
        fStatuses.clear();
        fIndex = 0;
    }

    // Maintain strictly increasing break positions and deduplicate overlapping tokenizer boundaries
    void addBreak(Position pos, Status status) {
        if (fBreaks.empty() || fBreaks.back() < pos) {
            fBreaks.push_back(pos);
            fStatuses.push_back(status);
        }
    }

    // Upgrade an existing soft break at pos to a hard break, or insert a new break carrying both
    // hard and soft flags when CFStringTokenizer omitted the boundary
    void addHardBreak(Position pos) {
        auto it = std::lower_bound(fBreaks.begin(), fBreaks.end(), pos);
        auto idx = std::distance(fBreaks.begin(), it);
        if (it != fBreaks.end() && *it == pos) {
            fStatuses[idx] |= SkUnicode::CodeUnitFlags::kHardLineBreakBefore;
        } else {
            fBreaks.insert(it, pos);
            fStatuses.insert(fStatuses.begin() + idx,
                             SkUnicode::CodeUnitFlags::kHardLineBreakBefore |
                                     SkUnicode::CodeUnitFlags::kSoftLineBreakBefore);
        }
    }

    // Always bracket token boundaries with 0 and totalUnits so inputs skipped by CFStringTokenizer
    // (such as standalone CR or punctuation-only strings) still yield valid [0, end] break ranges
    template <typename MapFn>
    bool populateBreaks(CFStringRef str, CFIndex len16, Position totalUnits, MapFn&& map16) {
        const char* loc = fLocale.empty() ? nullptr : fLocale.c_str();
        if (fType == SkUnicode::BreakType::kLines) {
            this->addBreak(0, SkUnicode::CodeUnitFlags::kSoftLineBreakBefore);
            if (!Tokenize(str, len16, kCFStringTokenizerUnitLineBreak, loc, &fTokenizer,
                          [&](CFRange r) {
                              this->addBreak(map16(r.location + r.length),
                                             SkUnicode::CodeUnitFlags::kSoftLineBreakBefore);
                          })) {
                return false;
            }
            this->addBreak(totalUnits, SkUnicode::CodeUnitFlags::kSoftLineBreakBefore);
        } else if (fType == SkUnicode::BreakType::kGraphemes) {
            ForEachGraphemeBoundary(str, len16, [&](CFIndex pos16) {
                this->addBreak(map16(pos16), 0);
            });
            this->addBreak(totalUnits, 0);
        } else {
            CFOptionFlags unit = (fType == SkUnicode::BreakType::kWords)
                                         ? kCFStringTokenizerUnitWordBoundary
                                         : kCFStringTokenizerUnitSentence;
            this->addBreak(0, 0);
            if (!Tokenize(str, len16, unit, loc, &fTokenizer, [&](CFRange r) {
                    this->addBreak(map16(r.location), 0);
                })) {
                return false;
            }
            this->addBreak(totalUnits, 0);
        }
        return true;
    }

    SkUnicode::BreakType fType;
    std::string fLocale;
    SkUniqueCFRef<CFStringTokenizerRef> fTokenizer;
    std::vector<Position> fBreaks;
    std::vector<Status> fStatuses;
    Position fIndex = 0;
};

}  // namespace

class SkUnicode_apple : public SkUnicodeHardCodedCharProperties {
public:
    SkUnicode_apple() = default;
    ~SkUnicode_apple() override = default;

    bool isControl(SkUnichar unichar) override {
        // Include Zl/Zp line/paragraph separators to match u_iscntrl
        if (unichar == 0x2028 || unichar == 0x2029) {
            return true;
        }
        // CoreFoundation replicates Plane 14 Tag page 0 across U+E0100..U+EFFFF,
        // falsely matching Variation Selectors Supplement (Mn)
        if (unichar >= 0xE0100 && unichar <= 0xEFFFF) {
            return false;
        }
        return CFCharacterSetIsLongCharacterMember(
                CFCharacterSetGetPredefined(kCFCharacterSetControl), unichar);
    }

    bool isWhitespace(SkUnichar unichar) override {
        // Non-breaking spaces and NEL must not be treated as soft break opportunities; delegate
        // the remaining check to isSpace so U+001C..U+001F and U+200B handling stays consistent
        if (unichar == 0x0085 || unichar == 0x00A0 || unichar == 0x2007 || unichar == 0x202F) {
            return false;
        }
        return this->isSpace(unichar);
    }

    bool isSpace(SkUnichar unichar) override {
        // Exclude ZWSP (Cf) and include ASCII information separators (FS..US) to match u_isspace
        if (unichar == 0x200B) {
            return false;
        }
        if (unichar >= 0x001C && unichar <= 0x001F) {
            return true;
        }
        return CFCharacterSetIsLongCharacterMember(
                CFCharacterSetGetPredefined(kCFCharacterSetWhitespaceAndNewline), unichar);
    }

    // Share UAX #14 LF/BK mandatory break detection with SkBreakIterator_apple
    bool isHardBreak(SkUnichar unichar) override {
        return is_hard_break(unichar);
    }

    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[],
                                                     int count,
                                                     SkBidiIterator::Direction dir) override {
        return fBidiFact->MakeIterator(text, count, dir);
    }

    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[],
                                                     int count,
                                                     SkBidiIterator::Direction dir) override {
        return fBidiFact->MakeIterator(text, count, dir);
    }

    std::unique_ptr<SkBreakIterator> makeBreakIterator(const char locale[],
                                                       BreakType breakType) override {
        return std::make_unique<SkBreakIterator_apple>(breakType, locale);
    }

    std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType breakType) override {
        return this->makeBreakIterator(nullptr, breakType);
    }

    SkString toUpper(const SkString& str) override { return this->toUpper(str, nullptr); }

    SkString toUpper(const SkString& str, const char* locale) override {
        if (str.isEmpty()) {
            return SkString();
        }
        // Manage CFString and CFLocale lifetimes via SkUniqueCFRef across early returns
        SkUniqueCFRef<CFStringRef> cfStr = MakeCFStringNoCopy(str.c_str(), SkToInt(str.size()));
        if (!cfStr) {
            return str;
        }
        SkUniqueCFRef<CFMutableStringRef> mutStr(
                CFStringCreateMutableCopy(kCFAllocatorDefault, 0, cfStr.get()));
        if (!mutStr) {
            return str;
        }
        SkUniqueCFRef<CFLocaleRef> cfLoc = MakeCFLocale(locale);
        CFStringUppercase(mutStr.get(), cfLoc.get());
        CFIndex len = CFStringGetLength(mutStr.get());
        CFIndex maxBytes = CFStringGetMaximumSizeForEncoding(len, kCFStringEncodingUTF8);
        SkString result;
        result.resize(maxBytes);
        CFIndex usedBytes = 0;
        CFStringGetBytes(mutStr.get(),
                         CFRangeMake(0, len),
                         kCFStringEncodingUTF8,
                         0,
                         false,
                         (UInt8*)result.data(),
                         maxBytes,
                         &usedBytes);
        // Trim output buffer from worst-case UTF-8 reservation to the actual encoded byte count
        result.resize(usedBytes);
        return result;
    }

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override {
        if (levelsCount <= 0) {
            return;
        }
        SkASSERT(runLevels != nullptr);
        SkASSERT(logicalFromVisual != nullptr);
        fBidiFact->bidi_reorderVisual(runLevels, levelsCount, logicalFromVisual);
    }

    bool getBidiRegions(const char utf8[],
                        int utf8Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) override {
        return fBidiFact->ExtractBidi(utf8, utf8Units, dir, results);
    }

    bool getWords(const char utf8[],
                  int utf8Units,
                  const char* locale,
                  std::vector<Position>* results) override {
        results->clear();
        // Reject negative lengths or null non-empty buffers, while treating empty text as a valid
        // no-op that returns true with an empty result vector
        if (utf8Units < 0 || (utf8Units > 0 && !utf8)) {
            return false;
        }
        if (utf8Units == 0) {
            return true;
        }

        SkUniqueCFRef<CFStringRef> str = MakeCFStringNoCopy(utf8, utf8Units);
        if (!str) {
            return false;
        }

        // Tokenize word boundaries in UTF-16 code units and append the end-of-text sentinel
        CFIndex len16 = CFStringGetLength(str.get());
        bool ok = Tokenize(str.get(), len16, kCFStringTokenizerUnitWordBoundary, locale, nullptr,
                           [&](CFRange r) { results->push_back(r.location); });
        if (!results->empty()) {
            results->push_back(len16);
        }
        return ok;
    }

    bool getUtf8Words(const char utf8[],
                      int utf8Units,
                      const char* locale,
                      std::vector<Position>* results) override {
        results->clear();
        // Reject negative lengths or null non-empty buffers; empty text is a valid no-op
        if (utf8Units < 0 || (utf8Units > 0 && !utf8)) {
            return false;
        }
        if (utf8Units == 0) {
            return true;
        }

        std::vector<Position> u16Words;
        if (!this->getWords(utf8, utf8Units, locale, &u16Words)) {
            return false;
        }

        // Map UTF-16 word boundaries from getWords to UTF-8 byte offsets, clamping the trailing
        // end-of-string boundary to utf8Units
        STArray<256, size_t> u16ToU8;
        ExtractUtf16ToUtf8Mapping(utf8, utf8Units, &u16ToU8);

        results->reserve(u16Words.size());
        for (auto w16 : u16Words) {
            results->push_back(MapUtf16ToUtf8(u16ToU8, w16, utf8Units));
        }
        return true;
    }

    bool getSentences(const char utf8[],
                      int utf8Units,
                      const char* locale,
                      std::vector<Position>* results) override {
        results->clear();
        // Reject negative lengths or null non-empty buffers; empty text is a valid no-op
        if (utf8Units < 0 || (utf8Units > 0 && !utf8)) {
            return false;
        }
        if (utf8Units == 0) {
            return true;
        }

        SkUniqueCFRef<CFStringRef> str = MakeCFStringNoCopy(utf8, utf8Units);
        if (!str) {
            return false;
        }

        // Tokenize sentence starts in UTF-16 and map them directly to UTF-8 byte offsets
        CFIndex len16 = CFStringGetLength(str.get());
        STArray<256, size_t> u16ToU8;
        ExtractUtf16ToUtf8Mapping(utf8, utf8Units, &u16ToU8);

        bool ok = Tokenize(str.get(), len16, kCFStringTokenizerUnitSentence, locale, nullptr,
                           [&](CFRange r) {
                               results->push_back(MapUtf16ToUtf8(u16ToU8, r.location, utf8Units));
                           });
        if (!results->empty()) {
            results->push_back(utf8Units);
        }
        return ok;
    }

    bool computeCodeUnitFlags(char utf8[],
                              int utf8Units,
                              bool replaceTabs,
                              TArray<CodeUnitFlags, true>* results) override {
        results->clear();
        // Reject negative lengths or null non-empty buffers; empty text still requires a 1-element
        // sentinel with kSoftLineBreakBefore | kGraphemeStart at index 0 to match ICU and Libgrapheme
        if (utf8Units < 0 || (utf8Units > 0 && !utf8)) {
            return false;
        }
        results->push_back_n(utf8Units + 1, CodeUnitFlags::kNoCodeUnitFlag);
        if (utf8Units == 0) {
            (*results)[0] = CodeUnitFlags::kSoftLineBreakBefore | CodeUnitFlags::kGraphemeStart;
            return true;
        }

        SkUniqueCFRef<CFStringRef> str = MakeCFStringNoCopy(utf8, utf8Units);
        if (!str) {
            return false;
        }

        // Map UTF-16 grapheme and line break boundaries to UTF-8 byte positions and apply
        // per-codepoint property flags across all bytes of each sequence
        STArray<256, size_t> u16ToU8;
        ExtractUtf16ToUtf8Mapping(utf8, utf8Units, &u16ToU8);
        this->computeBreakFlags(
                str.get(),
                CFStringGetLength(str.get()),
                utf8Units,
                [&](CFIndex pos16) { return MapUtf16ToUtf8(u16ToU8, pos16, utf8Units); },
                results);

        this->forEachCodepoint(utf8,
                               utf8Units,
                               [&](SkUnichar unichar, int32_t start, int32_t end, size_t) {
                                   this->applyCodepointFlags(
                                           unichar, start, end, replaceTabs, utf8, results);
                               });
        return true;
    }

    bool computeCodeUnitFlags(char16_t utf16[],
                              int utf16Units,
                              bool replaceTabs,
                              TArray<CodeUnitFlags, true>* results) override {
        results->clear();
        // Reject negative lengths or null non-empty buffers; empty text still requires a 1-element
        // sentinel with kSoftLineBreakBefore | kGraphemeStart at index 0 to match ICU and Libgrapheme
        if (utf16Units < 0 || (utf16Units > 0 && !utf16)) {
            return false;
        }
        results->push_back_n(utf16Units + 1, CodeUnitFlags::kNoCodeUnitFlag);
        if (utf16Units == 0) {
            (*results)[0] = CodeUnitFlags::kSoftLineBreakBefore | CodeUnitFlags::kGraphemeStart;
            return true;
        }

        SkUniqueCFRef<CFStringRef> str = MakeCFStringNoCopy(utf16, utf16Units);
        if (!str) {
            return false;
        }

        // Populate grapheme, line break, and codepoint property flags directly in UTF-16 coordinates
        this->computeBreakFlags(str.get(),
                                utf16Units,
                                utf16Units,
                                [](CFIndex pos16) { return SkTo<Position>(pos16); },
                                results);

        this->forEachCodepoint(
                utf16, utf16Units, [&](SkUnichar unichar, int32_t start, int32_t end) {
                    this->applyCodepointFlags(unichar, start, end, replaceTabs, utf16, results);
                });
        return true;
    }

private:
    // Share grapheme and soft line break flag population between UTF-8 and UTF-16 overloads via
    // a coordinate-mapping callback, always marking index 0 and the trailing sentinel at units
    template <typename MapFn>
    void computeBreakFlags(CFStringRef str,
                           CFIndex len16,
                           int units,
                           MapFn&& map16,
                           TArray<CodeUnitFlags, true>* results) {
        ForEachGraphemeBoundary(str, len16, [&](CFIndex pos16) {
            (*results)[map16(pos16)] |= CodeUnitFlags::kGraphemeStart;
        });
        (*results)[units] |= CodeUnitFlags::kGraphemeStart;

        (*results)[0] |= CodeUnitFlags::kSoftLineBreakBefore;
        Tokenize(str, len16, kCFStringTokenizerUnitLineBreak, nullptr, &fLineTokenizer,
                 [&](CFRange r) {
                     (*results)[map16(r.location + r.length)] |=
                             CodeUnitFlags::kSoftLineBreakBefore;
                 });
        (*results)[units] |= CodeUnitFlags::kSoftLineBreakBefore;
    }

    // Share per-codepoint property flag assignment and optional tab-to-space replacement between
    // UTF-8 and UTF-16 overloads; hard line breaks mark the position immediately after the newline
    template <typename CharT>
    void applyCodepointFlags(SkUnichar unichar,
                             int32_t start,
                             int32_t end,
                             bool replaceTabs,
                             CharT text[],
                             TArray<CodeUnitFlags, true>* results) {
        if (this->isHardBreak(unichar)) {
            (*results)[end] |=
                    (CodeUnitFlags::kHardLineBreakBefore | CodeUnitFlags::kSoftLineBreakBefore);
        }
        if (replaceTabs && this->isTabulation(unichar)) {
            results->at(start) |= CodeUnitFlags::kTabulation;
            unichar = ' ';
            text[start] = ' ';
        }
        CodeUnitFlags flags = CodeUnitFlags::kNoCodeUnitFlag;
        if (this->isSpace(unichar)) {
            flags |= CodeUnitFlags::kPartOfIntraWordBreak;
        }
        if (this->isWhitespace(unichar)) {
            flags |= CodeUnitFlags::kPartOfWhiteSpaceBreak;
        }
        if (this->isControl(unichar)) {
            flags |= CodeUnitFlags::kControl;
        }
        if (this->isIdeographic(unichar)) {
            flags |= CodeUnitFlags::kIdeographic;
        }
        for (auto i = start; i < end; ++i) {
            results->at(i) |= flags;
        }
    }

    sk_sp<SkBidiFactory> fBidiFact = sk_make_sp<SkBidiSubsetFactory>();
    SkUniqueCFRef<CFStringTokenizerRef> fLineTokenizer;
};

namespace SkUnicodes::Apple {
sk_sp<SkUnicode> Make() { return sk_make_sp<SkUnicode_apple>(); }
}  // namespace SkUnicodes::Apple
