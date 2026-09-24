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
#include "include/private/SkTo.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/include/SkUnicode_apple.h"
#include "modules/skunicode/src/SkBidiFactory_icu_subset.h"
#include "modules/skunicode/src/SkUnicode_hardcoded.h"
#include "modules/skunicode/src/SkUnicode_icu_bidi.h"
#include "src/core/SkBitmaskEnum.h"
#include "src/core/SkUTF.h"

#include <CoreFoundation/CoreFoundation.h>

#include <algorithm>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace skia_private;
using namespace SkUnicodes::Apple::CodePoints;

namespace {

template <typename T>
class CFUnique {
public:
    CFUnique() : fRef(nullptr) {}
    explicit CFUnique(T ref) : fRef(ref) {}
    ~CFUnique() {
        if (fRef) {
            CFRelease(fRef);
        }
    }

    CFUnique(const CFUnique&) = delete;
    CFUnique& operator=(const CFUnique&) = delete;

    CFUnique(CFUnique&& other) noexcept : fRef(std::exchange(other.fRef, nullptr)) {}
    CFUnique& operator=(CFUnique&& other) noexcept {
        if (fRef) {
            CFRelease(fRef);
        }
        fRef = std::exchange(other.fRef, nullptr);
        return *this;
    }

    T get() const { return fRef; }
    operator T() const { return fRef; }
    explicit operator bool() const { return fRef != nullptr; }

    void reset(T ref = nullptr) {
        if (fRef) {
            CFRelease(fRef);
        }
        fRef = ref;
    }

    T release() {
        return std::exchange(fRef, nullptr);
    }

private:
    T fRef;
};

static CFLocaleRef create_cf_locale(const char* localeStr) {
    if (!localeStr || !localeStr[0]) {
        return nullptr;
    }
    CFUnique<CFStringRef> locStr(CFStringCreateWithCString(
        kCFAllocatorDefault, localeStr, kCFStringEncodingUTF8));
    return locStr ? CFLocaleCreate(kCFAllocatorDefault, locStr.get()) : nullptr;
}

}  // namespace

// In UTF-8, continuation bytes have the binary prefix 10xxxxxx (masked with 0xC0 == 0x80)
static constexpr uint8_t kUtf8ContinuationMask = 0xC0;
static constexpr uint8_t kUtf8ContinuationBits = 0x80;

// Standard Unicode replacement character (U+FFFD) used for invalid UTF sequences
static constexpr SkUnichar kReplacementCharacter = 0xFFFD;

// Returns the SkUnichar code point immediately preceding the byte offset `pos` in `utf8`
// In UTF-8, continuation bytes match the bit pattern 10xxxxxx (0x80..0xBF)
// We walk backwards from `target - 1` to find the sequence lead byte, then decode
// forward using SkUTF::NextUTF8. Returns 0 if pos == 0
static SkUnichar get_prev_codepoint_utf8(const char* utf8, size_t pos) {
    if (pos == 0) {
        return 0;
    }
    const char* target = utf8 + pos;
    const char* prev_ptr = target - 1;
    while (prev_ptr > utf8 && (*reinterpret_cast<const uint8_t*>(prev_ptr) & kUtf8ContinuationMask) == kUtf8ContinuationBits) {
        --prev_ptr;
    }
    return SkUTF::NextUTF8(&prev_ptr, target);
}

// Returns the SkUnichar code point immediately preceding code unit index `pos` in `utf16`
// Handles UTF-16 surrogate pairs: if the code unit before `pos` is a trailing surrogate
// (0xDC00..0xDFFF) preceded by a leading surrogate (0xD800..0xDBFF), both units are
// decoded into a supplementary code point (e.g. SMP emojis). Returns 0 if pos == 0
static SkUnichar get_prev_codepoint_utf16(const char16_t* utf16, size_t pos) {
    if (pos == 0) {
        return 0;
    }
    const uint16_t* u16 = reinterpret_cast<const uint16_t*>(utf16);
    if (pos > 1 && SkUTF::IsTrailingSurrogateUTF16(u16[pos - 1]) &&
        SkUTF::IsLeadingSurrogateUTF16(u16[pos - 2])) {
        const uint16_t* p = &u16[pos - 2];
        return SkUTF::NextUTF16(&p, &u16[pos]);
    }
    return u16[pos - 1];
}

// Returns true if `u` is a mandatory (hard) line break character per Unicode Standard Annex #14
// Checks named constants: LF ('\n'), CR ('\r'), NEL (U+0085), VT (U+000B), FF (U+000C),
// Line Separator (U+2028), and Paragraph Separator (U+2029)
static bool is_hard_break(SkUnichar u) {
    return u == kLineFeed ||
           u == kCarriageReturn ||
           u == kNextLine ||
           u == kVerticalTab ||
           u == kFormFeed ||
           u == kLineSeparator ||
           u == kParagraphSeparator;
}

class SkBreakIterator_apple : public SkBreakIterator {
public:
    // CFLocaleRef is created once on iterator construction and cached for all setText() calls
    // to avoid expensive locale allocations and releases in hot layout loops
    // Grapheme break iteration does not depend on locale and skips locale creation entirely
    SkBreakIterator_apple(SkUnicode::BreakType type, const char* locale)
        : fType(type) {
        if (fType != SkUnicode::BreakType::kGraphemes) {
            fCFLocale.reset(create_cf_locale(locale));
        }
    }

    ~SkBreakIterator_apple() override = default;

    const std::vector<Position>& getBreaks() const { return fBreaks; }

    Position first() override {
        fIndex = 0;
        return fBreaks.empty() ? 0 : fBreaks[0];
    }
    Position current() override {
        return (fIndex < fBreaks.size()) ? fBreaks[fIndex] : kDone;
    }
    Position next() override {
        if (fIndex + 1 < fBreaks.size()) {
            return fBreaks[++fIndex];
        }
        fIndex = fBreaks.size();
        return kDone;
    }
    Status status() override {
        if (fType == SkUnicode::BreakType::kLines) {
            if (fIndex < fStatuses.size()) {
                return fStatuses[fIndex];
            }
            return static_cast<Status>(SkUnicode::LineBreakType::kSoftLineBreak);
        }
        return 0;
    }
    bool isDone() override {
        return fIndex >= fBreaks.size();
    }

    bool setText(const char utftext8[], int utf8Units) override {
        fBreaks.clear();
        fStatuses.clear();
        fIndex = 0;
        if (utf8Units <= 0) {
            fBreaks.push_back(0);
            fStatuses.push_back(0);
            return true;
        }
        CFUnique<CFStringRef> cfStr(CFStringCreateWithBytes(
            kCFAllocatorDefault,
            reinterpret_cast<const UInt8*>(utftext8),
            utf8Units,
            kCFStringEncodingUTF8,
            false
        ));
        if (!cfStr) {
            return false;
        }
        return populateBreaks(cfStr.get(), utf8Units, true, utftext8);
    }

    bool setText(const char16_t utftext16[], int utf16Units) override {
        fBreaks.clear();
        fStatuses.clear();
        fIndex = 0;
        if (utf16Units <= 0) {
            fBreaks.push_back(0);
            fStatuses.push_back(0);
            return true;
        }
        CFUnique<CFStringRef> cfStr(CFStringCreateWithCharactersNoCopy(
            kCFAllocatorDefault,
            reinterpret_cast<const UniChar*>(utftext16),
            utf16Units,
            kCFAllocatorNull
        ));
        if (!cfStr) {
            return false;
        }
        return populateBreaks(cfStr.get(), utf16Units, false, utftext16);
    }

private:
    static constexpr const Position kDone = -1;
    SkUnicode::BreakType fType;
    CFUnique<CFLocaleRef> fCFLocale;
    std::vector<Position> fBreaks;
    std::vector<Status> fStatuses;
    size_t fIndex = 0;

    bool populateBreaks(CFStringRef cfStr, int units, bool isUtf8, const void* rawText) {
        CFIndex utf16Len = CFStringGetLength(cfStr);

        std::vector<Position> mapping;
        if (isUtf8) {
            SkSpan<const char> text(static_cast<const char*>(rawText), units);
            SkUnicode::extractUtfConversionMapping(
                text,
                [&](size_t i8) { mapping.emplace_back(i8); },
                [&](size_t) {}
            );
        }

        auto mapPos = [&](CFIndex idx16) -> Position {
            if (!isUtf8) return idx16;
            if (SkToSizeT(idx16) < mapping.size()) return mapping[idx16];
            return units;
        };

        if (fType == SkUnicode::BreakType::kGraphemes) {
            CFIndex idx16 = 0;
            while (idx16 < utf16Len) {
                CFRange r = CFStringGetRangeOfComposedCharactersAtIndex(cfStr, idx16);
                fBreaks.push_back(mapPos(r.location));
                fStatuses.push_back(0);
                idx16 = r.location + r.length;
            }
            fBreaks.push_back(mapPos(utf16Len));
            fStatuses.push_back(0);
        } else if (fType == SkUnicode::BreakType::kSentences) {
            // For sentences, CFStringTokenizer emits token ranges [location, location + length)
            // To match ICU's SkBreakIterator contract (and SkUnicodeTest), sentence break positions
            // represent the start of each sentence plus the end of string (e.g. {0, 13, 26, 41, 70})
            CFUnique<CFStringTokenizerRef> tokenizer(CFStringTokenizerCreate(
                kCFAllocatorDefault,
                cfStr,
                CFRangeMake(0, utf16Len),
                kCFStringTokenizerUnitSentence,
                fCFLocale.get()
            ));
            if (!tokenizer) {
                return false;
            }
            fBreaks.push_back(0);
            fStatuses.push_back(0);
            while (CFStringTokenizerAdvanceToNextToken(tokenizer.get()) != kCFStringTokenizerTokenNone) {
                CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer.get());
                Position startPos = mapPos(r.location);
                if (fBreaks.back() != startPos) {
                    fBreaks.push_back(startPos);
                    fStatuses.push_back(0);
                }
            }
            if (fBreaks.empty() || fBreaks.back() != static_cast<Position>(units)) {
                fBreaks.push_back(units);
                fStatuses.push_back(0);
            }
        } else {
            CFOptionFlags unitType = (fType == SkUnicode::BreakType::kLines)
                ? kCFStringTokenizerUnitLineBreak
                : kCFStringTokenizerUnitWordBoundary;

            CFUnique<CFStringTokenizerRef> tokenizer(CFStringTokenizerCreate(
                kCFAllocatorDefault,
                cfStr,
                CFRangeMake(0, utf16Len),
                unitType,
                fCFLocale.get()
            ));
            if (!tokenizer) {
                return false;
            }

            fBreaks.push_back(0);
            fStatuses.push_back(0);
            while (CFStringTokenizerAdvanceToNextToken(tokenizer.get()) != kCFStringTokenizerTokenNone) {
                CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer.get());
                Position startPos = mapPos(r.location);
                if (fBreaks.back() != startPos) {
                    fBreaks.push_back(startPos);
                    fStatuses.push_back(0);
                }
                Position endPos = mapPos(r.location + r.length);
                fBreaks.push_back(endPos);
                Status status = static_cast<Status>(SkUnicode::LineBreakType::kSoftLineBreak);
                // CoreFoundation's kCFStringTokenizerUnitLineBreak generates all potential break
                // opportunities (both soft and mandatory). We inspect the preceding codepoint
                // to distinguish hard breaks from soft wrapping opportunities
                if (fType == SkUnicode::BreakType::kLines) {
                    SkUnichar prev = isUtf8
                        ? get_prev_codepoint_utf8(static_cast<const char*>(rawText), endPos)
                        : get_prev_codepoint_utf16(static_cast<const char16_t*>(rawText), endPos);
                    if (is_hard_break(prev)) {
                        status = static_cast<Status>(SkUnicode::LineBreakType::kHardLineBreak);
                    }
                }
                fStatuses.push_back(status);
            }
            if (fBreaks.empty() || fBreaks.back() != static_cast<Position>(units)) {
                fBreaks.push_back(units);
                Status status = static_cast<Status>(SkUnicode::LineBreakType::kSoftLineBreak);
                // If the text ends with a newline/hard break, ensure the boundary at `units`
                // is tagged with kHardLineBreak so SkParagraph detects the trailing paragraph break
                if (fType == SkUnicode::BreakType::kLines) {
                    SkUnichar prev = isUtf8
                        ? get_prev_codepoint_utf8(static_cast<const char*>(rawText), units)
                        : get_prev_codepoint_utf16(static_cast<const char16_t*>(rawText), units);
                    if (is_hard_break(prev)) {
                        status = static_cast<Status>(SkUnicode::LineBreakType::kHardLineBreak);
                    }
                }
                fStatuses.push_back(status);
            }
        }
        return true;
    }
};

class SkUnicode_apple : public SkUnicodeHardCodedCharProperties {
public:
    SkUnicode_apple() = default;
    ~SkUnicode_apple() override = default;

    // For SkShaper
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[], int count,
                                                     SkBidiIterator::Direction dir) override {
        return fBidiFact->MakeIterator(text, count, dir);
    }

    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[], int count,
                                                     SkBidiIterator::Direction dir) override {
        return fBidiFact->MakeIterator(text, count, dir);
    }

    std::unique_ptr<SkBreakIterator> makeBreakIterator(const char locale[],
                                                       BreakType breakType) override {
        return std::make_unique<SkBreakIterator_apple>(breakType, locale);
    }

    std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType breakType) override {
        return std::make_unique<SkBreakIterator_apple>(breakType, nullptr);
    }

    bool isHardBreak(SkUnichar u) override {
        return is_hard_break(u);
    }

    bool getBidiRegions(const char utf8[],
                        int utf8Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) override {
        return fBidiFact->ExtractBidi(utf8, utf8Units, dir, results);
    }

    bool getWords(const char utf8[], int utf8Units, const char* localeStr,
                  std::vector<Position>* results) override {
        if (utf8Units <= 0) {
            return true;
        }
        CFUnique<CFStringRef> cfStr(CFStringCreateWithBytes(
            kCFAllocatorDefault,
            reinterpret_cast<const UInt8*>(utf8),
            utf8Units,
            kCFStringEncodingUTF8,
            false
        ));
        if (!cfStr) {
            return false;
        }
        CFIndex utf16Len = CFStringGetLength(cfStr.get());
        CFUnique<CFLocaleRef> cfLocale(create_cf_locale(localeStr));
        CFUnique<CFStringTokenizerRef> tokenizer(CFStringTokenizerCreate(
            kCFAllocatorDefault,
            cfStr.get(),
            CFRangeMake(0, utf16Len),
            kCFStringTokenizerUnitWordBoundary,
            cfLocale.get()
        ));
        if (!tokenizer) {
            return false;
        }
        results->emplace_back(0);
        while (CFStringTokenizerAdvanceToNextToken(tokenizer.get()) != kCFStringTokenizerTokenNone) {
            CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer.get());
            if (results->back() != SkTo<Position>(r.location)) {
                results->emplace_back(r.location);
            }
            results->emplace_back(r.location + r.length);
        }
        if (results->empty() || results->back() != SkTo<Position>(utf16Len)) {
            results->emplace_back(utf16Len);
        }
        return true;
    }

    bool getUtf8Words(const char utf8[],
                      int utf8Units,
                      const char* localeStr,
                      std::vector<Position>* results) override {
        if (utf8Units <= 0) {
            return true;
        }
        SkBreakIterator_apple iter(BreakType::kWords, localeStr);
        if (!iter.setText(utf8, utf8Units)) {
            return false;
        }
        results->assign(iter.getBreaks().begin(), iter.getBreaks().end());
        return true;
    }

    bool getSentences(const char utf8[],
                      int utf8Units,
                      const char* localeStr,
                      std::vector<SkUnicode::Position>* results) override {
        if (utf8Units <= 0) {
            return true;
        }
        SkBreakIterator_apple iter(BreakType::kSentences, localeStr);
        if (!iter.setText(utf8, utf8Units)) {
            return false;
        }
        results->assign(iter.getBreaks().begin(), iter.getBreaks().end());
        return true;
    }

    bool computeCodeUnitFlags(char utf8[],
                              int utf8Units,
                              bool replaceTabs,
                              TArray<SkUnicode::CodeUnitFlags, true>* results) override {
        results->clear();
        results->push_back_n(utf8Units + 1, CodeUnitFlags::kNoCodeUnitFlag);
        if (utf8Units <= 0) {
            return true;
        }

        CFUnique<CFStringRef> cfStr(CFStringCreateWithBytes(
            kCFAllocatorDefault,
            reinterpret_cast<const UInt8*>(utf8),
            utf8Units,
            kCFStringEncodingUTF8,
            false
        ));
        if (!cfStr) {
            return false;
        }

        CFIndex utf16Len = CFStringGetLength(cfStr.get());
        std::vector<Position> utf8ForUtf16;
        SkSpan<const char> text(utf8, utf8Units);
        SkUnicode::extractUtfConversionMapping(
            text,
            [&](size_t i8) { utf8ForUtf16.emplace_back(i8); },
            [&](size_t) {}
        );

        // Line breaks
        CFUnique<CFStringTokenizerRef> tokenizer(CFStringTokenizerCreate(
            kCFAllocatorDefault,
            cfStr.get(),
            CFRangeMake(0, utf16Len),
            kCFStringTokenizerUnitLineBreak,
            nullptr
        ));
        if (tokenizer) {
            (*results)[0] |= CodeUnitFlags::kSoftLineBreakBefore;
            while (CFStringTokenizerAdvanceToNextToken(tokenizer.get()) != kCFStringTokenizerTokenNone) {
                CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer.get());
                size_t pos16 = r.location;
                if (pos16 > 0 && pos16 <= utf8ForUtf16.size()) {
                    size_t pos8 = (pos16 < utf8ForUtf16.size()) ? utf8ForUtf16[pos16] : utf8Units;
                    (*results)[pos8] |= CodeUnitFlags::kSoftLineBreakBefore;
                    // CFStringTokenizer line breaks don't distinguish soft from hard breaks
                    // Decode the preceding codepoint (walking backwards over UTF-8 continuation bytes)
                    // to determine if this is a mandatory (hard) break
                    SkUnichar prev = get_prev_codepoint_utf8(utf8, pos8);
                    if (this->isHardBreak(prev)) {
                        (*results)[pos8] |= CodeUnitFlags::kHardLineBreakBefore;
                    }
                }
            }
            (*results)[utf8Units] |= CodeUnitFlags::kSoftLineBreakBefore;
            // If the text ends with a newline/hard break, tag the trailing boundary so that
            // SkParagraph's trailing newline detection and height calculations succeed
            SkUnichar lastChar = get_prev_codepoint_utf8(utf8, utf8Units);
            if (this->isHardBreak(lastChar)) {
                (*results)[utf8Units] |= CodeUnitFlags::kHardLineBreakBefore;
            }
        }

        // Grapheme clusters
        CFIndex idx16 = 0;
        while (idx16 < utf16Len) {
            CFRange r = CFStringGetRangeOfComposedCharactersAtIndex(cfStr.get(), idx16);
            if (SkToSizeT(r.location) < utf8ForUtf16.size()) {
                size_t pos8 = utf8ForUtf16[r.location];
                (*results)[pos8] |= CodeUnitFlags::kGraphemeStart;
            }
            idx16 = r.location + r.length;
        }
        (*results)[utf8Units] |= CodeUnitFlags::kGraphemeStart;

        // Characters / Codepoint properties
        const char* current = utf8;
        const char* end = utf8 + utf8Units;
        while (current < end) {
            auto before = current - utf8;
            SkUnichar unichar = SkUTF::NextUTF8(&current, end);
            if (unichar < 0) unichar = kReplacementCharacter;
            auto after = current - utf8;
            if (replaceTabs && this->isTabulation(unichar)) {
                results->at(before) |= SkUnicode::kTabulation;
                unichar = ' ';
                utf8[before] = ' ';
            }
            SkUnicode::CodeUnitFlags flags = CodeUnitFlags::kNoCodeUnitFlag;
            if (this->isSpace(unichar)) {
                flags |= SkUnicode::kPartOfIntraWordBreak;
            }
            if (this->isWhitespace(unichar)) {
                flags |= SkUnicode::kPartOfWhiteSpaceBreak;
            }
            if (this->isControl(unichar)) {
                flags |= SkUnicode::kControl;
            }
            if (this->isIdeographic(unichar)) {
                flags |= SkUnicode::kIdeographic;
            }
            for (auto i = before; i < after; ++i) {
                results->at(i) |= flags;
            }
        }

        return true;
    }

    bool computeCodeUnitFlags(char16_t utf16[],
                              int utf16Units,
                              bool replaceTabs,
                              TArray<SkUnicode::CodeUnitFlags, true>* results) override {
        results->clear();
        results->push_back_n(utf16Units + 1, CodeUnitFlags::kNoCodeUnitFlag);
        if (utf16Units <= 0) {
            return true;
        }

        CFUnique<CFStringRef> cfStr(CFStringCreateWithCharactersNoCopy(
            kCFAllocatorDefault,
            reinterpret_cast<const UniChar*>(utf16),
            utf16Units,
            kCFAllocatorNull
        ));
        if (!cfStr) {
            return false;
        }

        // Line breaks
        CFUnique<CFStringTokenizerRef> tokenizer(CFStringTokenizerCreate(
            kCFAllocatorDefault,
            cfStr.get(),
            CFRangeMake(0, utf16Units),
            kCFStringTokenizerUnitLineBreak,
            nullptr
        ));
        if (tokenizer) {
            (*results)[0] |= CodeUnitFlags::kSoftLineBreakBefore;
            while (CFStringTokenizerAdvanceToNextToken(tokenizer.get()) != kCFStringTokenizerTokenNone) {
                CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer.get());
                size_t pos = r.location;
                if (pos > 0 && pos <= SkToSizeT(utf16Units)) {
                    (*results)[pos] |= CodeUnitFlags::kSoftLineBreakBefore;
                    // Decode preceding codepoint (handling surrogate pairs) to detect hard breaks
                    SkUnichar prevChar = get_prev_codepoint_utf16(utf16, pos);
                    if (this->isHardBreak(prevChar)) {
                        (*results)[pos] |= CodeUnitFlags::kHardLineBreakBefore;
                    }
                }
            }
            (*results)[utf16Units] |= CodeUnitFlags::kSoftLineBreakBefore;
            // Tag trailing boundary if the text ends with a hard break
            SkUnichar lastChar = get_prev_codepoint_utf16(utf16, utf16Units);
            if (this->isHardBreak(lastChar)) {
                (*results)[utf16Units] |= CodeUnitFlags::kHardLineBreakBefore;
            }
        }

        // Grapheme clusters
        CFIndex idx = 0;
        while (idx < utf16Units) {
            CFRange r = CFStringGetRangeOfComposedCharactersAtIndex(cfStr.get(), idx);
            (*results)[r.location] |= CodeUnitFlags::kGraphemeStart;
            idx = r.location + r.length;
        }
        (*results)[utf16Units] |= CodeUnitFlags::kGraphemeStart;

        // Characters / Codepoint properties
        this->forEachCodepoint((char16_t*)&utf16[0], utf16Units,
            [this, results, replaceTabs, &utf16](SkUnichar unichar, int32_t start, int32_t end) {
                if (replaceTabs && this->isTabulation(unichar)) {
                    results->at(start) |= SkUnicode::kTabulation;
                    unichar = ' ';
                    utf16[start] = ' ';
                }
                SkUnicode::CodeUnitFlags flags = CodeUnitFlags::kNoCodeUnitFlag;
                if (this->isSpace(unichar)) {
                    flags |= SkUnicode::kPartOfIntraWordBreak;
                }
                if (this->isWhitespace(unichar)) {
                    flags |= SkUnicode::kPartOfWhiteSpaceBreak;
                }
                if (this->isControl(unichar)) {
                    flags |= SkUnicode::kControl;
                }
                if (this->isIdeographic(unichar)) {
                    flags |= SkUnicode::kIdeographic;
                }
                for (auto i = start; i < end; ++i) {
                    results->at(i) |= flags;
                }
            });

        return true;
    }

    SkString toUpper(const SkString& str) override {
        return this->toUpper(str, nullptr);
    }

    SkString toUpper(const SkString& str, const char* localeStr) override {
        if (str.isEmpty()) {
            return SkString();
        }
        CFUnique<CFStringRef> cfStr(CFStringCreateWithBytes(
            kCFAllocatorDefault,
            reinterpret_cast<const UInt8*>(str.c_str()),
            str.size(),
            kCFStringEncodingUTF8,
            false
        ));
        if (!cfStr) {
            return str;
        }
        CFUnique<CFMutableStringRef> mutStr(
            CFStringCreateMutableCopy(kCFAllocatorDefault, 0, cfStr.get()));
        if (!mutStr) {
            return str;
        }
        CFUnique<CFLocaleRef> cfLocale(create_cf_locale(localeStr));
        CFStringUppercase(mutStr.get(), cfLocale.get());
        CFIndex len = CFStringGetLength(mutStr.get());
        CFRange range = CFRangeMake(0, len);
        CFIndex usedBufLen = 0;
        CFStringGetBytes(mutStr.get(), range, kCFStringEncodingUTF8, '?', false, nullptr, 0, &usedBufLen);
        // Extract converted bytes into a temporary buffer before constructing SkString,
        // avoiding casting away constness on SkString internal storage
        std::vector<char> buffer(usedBufLen);
        CFStringGetBytes(mutStr.get(), range, kCFStringEncodingUTF8, '?', false,
                         reinterpret_cast<UInt8*>(buffer.data()), usedBufLen, nullptr);
        return SkString(buffer.data(), buffer.size());
    }

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override {
        if (levelsCount == 0) {
            return;
        }
        SkASSERT(runLevels != nullptr);
        fBidiFact->bidi_reorderVisual(runLevels, levelsCount, logicalFromVisual);
    }

private:
    friend class SkBreakIterator_apple;
    sk_sp<SkBidiFactory> fBidiFact = sk_make_sp<SkBidiSubsetFactory>();
};

namespace SkUnicodes::Apple {
sk_sp<SkUnicode> Make() {
    return sk_make_sp<SkUnicode_apple>();
}
}  // namespace SkUnicodes::Apple
