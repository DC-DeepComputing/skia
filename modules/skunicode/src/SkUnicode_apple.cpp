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
#include "modules/skunicode/src/SkUnicode_hardcoded.h"
#include "src/core/SkBitmaskEnum.h"
#include "src/core/SkUTF.h"

#include <CoreFoundation/CoreFoundation.h>
#include <CoreText/CoreText.h>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

using namespace skia_private;

namespace {

class SkBidiIterator_apple : public SkBidiIterator {
public:
    explicit SkBidiIterator_apple(std::vector<Level> levels) : fLevels(std::move(levels)) {}
    Position getLength() override { return SkTo<Position>(fLevels.size()); }
    Level getLevelAt(Position pos) override {
        if (pos >= 0 && pos < SkTo<Position>(fLevels.size())) {
            return fLevels[pos];
        }
        return 0;
    }
private:
    std::vector<Level> fLevels;
};

class SkBreakIterator_apple : public SkBreakIterator {
public:
    SkBreakIterator_apple(SkUnicode::BreakType type, const char* locale)
            : fType(type), fLocale(locale ? locale : "") {}

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
        fBreaks.clear();
        fStatuses.clear();
        fIndex = 0;
        if (utf8Units <= 0) {
            return true;
        }

        CFStringRef str = CFStringCreateWithBytesNoCopy(
            kCFAllocatorDefault, (const UInt8*)utftext8, utf8Units,
            kCFStringEncodingUTF8, false, kCFAllocatorNull);
        if (!str) {
            return false;
        }

        CFIndex len16 = CFStringGetLength(str);
        if (fType == SkUnicode::BreakType::kLines) {
            CFLocaleRef cfLoc = nullptr;
            if (!fLocale.empty()) {
                CFStringRef locStr = CFStringCreateWithCString(kCFAllocatorDefault, fLocale.c_str(), kCFStringEncodingUTF8);
                if (locStr) {
                    cfLoc = CFLocaleCreate(kCFAllocatorDefault, locStr);
                    CFRelease(locStr);
                }
            }
            CFStringTokenizerRef tokenizer = CFStringTokenizerCreate(
                kCFAllocatorDefault, str, CFRangeMake(0, len16),
                kCFStringTokenizerUnitLineBreak, cfLoc);
            if (cfLoc) {
                CFRelease(cfLoc);
            }

            fBreaks.push_back(0);
            fStatuses.push_back(SkUnicode::CodeUnitFlags::kSoftLineBreakBefore);

            CFStringTokenizerTokenType tokenType = CFStringTokenizerAdvanceToNextToken(tokenizer);
            while (tokenType != kCFStringTokenizerTokenNone) {
                CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer);
                fBreaks.push_back(r.location + r.length);
                fStatuses.push_back(SkUnicode::CodeUnitFlags::kSoftLineBreakBefore);
                tokenType = CFStringTokenizerAdvanceToNextToken(tokenizer);
            }
            CFRelease(tokenizer);
        } else if (fType == SkUnicode::BreakType::kWords) {
            CFStringTokenizerRef tokenizer = CFStringTokenizerCreate(
                kCFAllocatorDefault, str, CFRangeMake(0, len16),
                kCFStringTokenizerUnitWordBoundary, nullptr);

            CFStringTokenizerTokenType tokenType = CFStringTokenizerAdvanceToNextToken(tokenizer);
            while (tokenType != kCFStringTokenizerTokenNone) {
                CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer);
                fBreaks.push_back(r.location);
                fStatuses.push_back(0);
                tokenType = CFStringTokenizerAdvanceToNextToken(tokenizer);
            }
            if (!fBreaks.empty()) {
                fBreaks.push_back(len16);
                fStatuses.push_back(0);
            }
            CFRelease(tokenizer);
        } else if (fType == SkUnicode::BreakType::kGraphemes) {
            CFIndex idx = 0;
            while (idx < len16) {
                CFRange r = CFStringGetRangeOfComposedCharactersAtIndex(str, idx);
                fBreaks.push_back(r.location);
                fStatuses.push_back(SkUnicode::CodeUnitFlags::kGraphemeStart);
                idx = r.location + r.length;
            }
            fBreaks.push_back(len16);
            fStatuses.push_back(SkUnicode::CodeUnitFlags::kGraphemeStart);
        } else if (fType == SkUnicode::BreakType::kSentences) {
            CFStringTokenizerRef tokenizer = CFStringTokenizerCreate(
                kCFAllocatorDefault, str, CFRangeMake(0, len16),
                kCFStringTokenizerUnitSentence, nullptr);

            CFStringTokenizerTokenType tokenType = CFStringTokenizerAdvanceToNextToken(tokenizer);
            while (tokenType != kCFStringTokenizerTokenNone) {
                CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer);
                fBreaks.push_back(r.location);
                fStatuses.push_back(0);
                tokenType = CFStringTokenizerAdvanceToNextToken(tokenizer);
            }
            if (!fBreaks.empty()) {
                fBreaks.push_back(len16);
                fStatuses.push_back(0);
            }
            CFRelease(tokenizer);
        }

        CFRelease(str);
        return true;
    }

    bool setText(const char16_t utftext16[], int utf16Units) override {
        SkString utf8 = SkUnicode::convertUtf16ToUtf8(utftext16, utf16Units);
        return this->setText(utf8.c_str(), utf8.size());
    }

private:
    SkUnicode::BreakType fType;
    std::string fLocale;
    std::vector<Position> fBreaks;
    std::vector<Status> fStatuses;
    Position fIndex = 0;
};

} // namespace

class SkUnicode_apple : public SkUnicodeHardCodedCharProperties {
public:
    SkUnicode_apple() = default;
    ~SkUnicode_apple() override = default;

    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[], int count,
                                                     SkBidiIterator::Direction dir) override {
        if (count <= 0) {
            return std::make_unique<SkBidiIterator_apple>(std::vector<SkBidiIterator::Level>());
        }

        CFStringRef str = CFStringCreateWithCharactersNoCopy(
            kCFAllocatorDefault, (const UniChar*)text, count, kCFAllocatorNull);
        if (!str) {
            return nullptr;
        }

        CTWritingDirection baseDir = (dir == SkBidiIterator::kRTL)
                                     ? kCTWritingDirectionRightToLeft
                                     : kCTWritingDirectionLeftToRight;
        CTParagraphStyleSetting setting = { kCTParagraphStyleSpecifierBaseWritingDirection, sizeof(baseDir), &baseDir };
        CTParagraphStyleRef style = CTParagraphStyleCreate(&setting, 1);

        const void* keys[] = { kCTParagraphStyleAttributeName };
        const void* values[] = { style };
        CFDictionaryRef attrs = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 1,
                                                   &kCFTypeDictionaryKeyCallBacks,
                                                   &kCFTypeDictionaryValueCallBacks);
        CFAttributedStringRef attrStr = CFAttributedStringCreate(kCFAllocatorDefault, str, attrs);

        CTTypesetterRef ts = CTTypesetterCreateWithAttributedString(attrStr);
        CTLineRef line = CTTypesetterCreateLine(ts, CFRangeMake(0, count));

        std::vector<SkBidiIterator::Level> levels(count, 0);
        CFArrayRef runs = CTLineGetGlyphRuns(line);
        CFIndex runCount = CFArrayGetCount(runs);
        for (CFIndex i = 0; i < runCount; ++i) {
            CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(runs, i);
            CFRange r = CTRunGetStringRange(run);
            CTRunStatus st = CTRunGetStatus(run);
            bool isRTL = (st & kCTRunStatusRightToLeft) != 0;
            SkBidiIterator::Level lvl = 0;
            if (dir == SkBidiIterator::kRTL) {
                lvl = isRTL ? 1 : 2;
            } else {
                lvl = isRTL ? 1 : 0;
            }
            for (CFIndex k = r.location; k < r.location + r.length && k < count; ++k) {
                levels[k] = lvl;
            }
        }

        CFRelease(line);
        CFRelease(ts);
        CFRelease(attrStr);
        CFRelease(attrs);
        CFRelease(style);
        CFRelease(str);

        return std::make_unique<SkBidiIterator_apple>(std::move(levels));
    }

    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[], int count,
                                                     SkBidiIterator::Direction dir) override {
        auto utf16 = convertUtf8ToUtf16(text, count);
        return this->makeBidiIterator((const uint16_t*)utf16.data(), utf16.size(), dir);
    }

    std::unique_ptr<SkBreakIterator> makeBreakIterator(const char locale[], BreakType breakType) override {
        return std::make_unique<SkBreakIterator_apple>(breakType, locale);
    }

    std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType breakType) override {
        return this->makeBreakIterator(nullptr, breakType);
    }

    bool isEmoji(SkUnichar unichar) override {
        if (unichar == '#' || unichar == '*' || (unichar >= '0' && unichar <= '9')) {
            return true;
        }
        static constexpr std::pair<SkUnichar, SkUnichar> emojiRanges[] = {
            {0x203C, 0x203C}, {0x2049, 0x2049}, {0x2122, 0x2122}, {0x2139, 0x2139},
            {0x2194, 0x2199}, {0x21A9, 0x21AA}, {0x2300, 0x23FF}, {0x25AA, 0x25AB},
            {0x25B6, 0x25B6}, {0x25C0, 0x25C0}, {0x25FB, 0x25FE}, {0x2600, 0x27BF},
            {0x2934, 0x2935}, {0x2B05, 0x2B07}, {0x2B1B, 0x2B1C}, {0x2B50, 0x2B50},
            {0x2B55, 0x2B55}, {0x3030, 0x3030}, {0x303D, 0x303D}, {0x3297, 0x3297},
            {0x3299, 0x3299}, {0x1F000, 0x1F02F}, {0x1F0A0, 0x1F0FF}, {0x1F100, 0x1F64F},
            {0x1F680, 0x1F6FF}, {0x1F700, 0x1F77F}, {0x1F780, 0x1F7FF}, {0x1F800, 0x1F8FF},
            {0x1F900, 0x1FAFF}
        };
        for (const auto& r : emojiRanges) {
            if (unichar >= r.first && unichar <= r.second) {
                return true;
            }
        }
        return false;
    }

    bool isEmojiComponent(SkUnichar unichar) override {
        return (unichar >= 0x1F3FB && unichar <= 0x1F3FF) ||
               (unichar >= 0x1F1E6 && unichar <= 0x1F1FF) ||
               unichar == 0x200D || unichar == 0xFE0F;
    }

    bool isEmojiModifier(SkUnichar unichar) override {
        return unichar >= 0x1F3FB && unichar <= 0x1F3FF;
    }

    bool isEmojiModifierBase(SkUnichar unichar) override {
        return this->isEmoji(unichar) && !this->isEmojiModifier(unichar);
    }

    bool isRegionalIndicator(SkUnichar unichar) override {
        return unichar >= 0x1F1E6 && unichar <= 0x1F1FF;
    }

    SkString toUpper(const SkString& str) override {
        return this->toUpper(str, nullptr);
    }

    SkString toUpper(const SkString& str, const char* locale) override {
        if (str.isEmpty()) {
            return SkString();
        }
        CFStringRef cfStr = CFStringCreateWithBytesNoCopy(
            kCFAllocatorDefault, (const UInt8*)str.c_str(), str.size(),
            kCFStringEncodingUTF8, false, kCFAllocatorNull);
        if (!cfStr) {
            return str;
        }
        CFMutableStringRef mutStr = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, cfStr);
        CFRelease(cfStr);
        if (!mutStr) {
            return str;
        }
        CFLocaleRef cfLoc = nullptr;
        if (locale) {
            CFStringRef locStr = CFStringCreateWithCString(kCFAllocatorDefault, locale, kCFStringEncodingUTF8);
            if (locStr) {
                cfLoc = CFLocaleCreate(kCFAllocatorDefault, locStr);
                CFRelease(locStr);
            }
        }
        CFStringUppercase(mutStr, cfLoc);
        if (cfLoc) {
            CFRelease(cfLoc);
        }
        CFIndex len = CFStringGetLength(mutStr);
        CFIndex maxBytes = CFStringGetMaximumSizeForEncoding(len, kCFStringEncodingUTF8);
        SkString result;
        result.resize(maxBytes);
        CFIndex usedBytes = 0;
        CFStringGetBytes(mutStr, CFRangeMake(0, len), kCFStringEncodingUTF8, 0, false,
                         (UInt8*)result.data(), maxBytes, &usedBytes);
        result.resize(usedBytes);
        CFRelease(mutStr);
        return result;
    }

    void reorderVisual(const BidiLevel runLevels[], int levelsCount, int32_t logicalFromVisual[]) override {
        if (levelsCount <= 0) {
            return;
        }
        SkASSERT(runLevels != nullptr);
        SkASSERT(logicalFromVisual != nullptr);
        for (int i = 0; i < levelsCount; ++i) {
            logicalFromVisual[i] = i;
        }
        BidiLevel lowestOdd = 255;
        BidiLevel highest = 0;
        for (int i = 0; i < levelsCount; ++i) {
            BidiLevel l = runLevels[i];
            if (l > highest) {
                highest = l;
            }
            if ((l & 1) && l < lowestOdd) {
                lowestOdd = l;
            }
        }
        if (lowestOdd > highest) {
            return;
        }
        for (BidiLevel level = highest; level >= lowestOdd; --level) {
            for (int i = 0; i < levelsCount; ++i) {
                if (runLevels[i] >= level) {
                    int start = i;
                    while (i + 1 < levelsCount && runLevels[i + 1] >= level) {
                        ++i;
                    }
                    int end = i;
                    while (start < end) {
                        std::swap(logicalFromVisual[start++], logicalFromVisual[end--]);
                    }
                }
            }
            if (level == 0) {
                break;
            }
        }
    }

    bool getBidiRegions(const char utf8[], int utf8Units, TextDirection dir,
                        std::vector<BidiRegion>* results) override {
        if (utf8Units == 0) {
            return true;
        }

        CFStringRef str = CFStringCreateWithBytesNoCopy(
            kCFAllocatorDefault, (const UInt8*)utf8, utf8Units,
            kCFStringEncodingUTF8, false, kCFAllocatorNull);
        if (!str) {
            return false;
        }

        CFIndex len16 = CFStringGetLength(str);
        if (len16 == 0) {
            CFRelease(str);
            return true;
        }

        CTWritingDirection baseDir = (dir == TextDirection::kRTL)
                                     ? kCTWritingDirectionRightToLeft
                                     : kCTWritingDirectionLeftToRight;
        CTParagraphStyleSetting setting = { kCTParagraphStyleSpecifierBaseWritingDirection, sizeof(baseDir), &baseDir };
        CTParagraphStyleRef style = CTParagraphStyleCreate(&setting, 1);

        const void* keys[] = { kCTParagraphStyleAttributeName };
        const void* values[] = { style };
        CFDictionaryRef attrs = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 1,
                                                   &kCFTypeDictionaryKeyCallBacks,
                                                   &kCFTypeDictionaryValueCallBacks);
        CFAttributedStringRef attrStr = CFAttributedStringCreate(kCFAllocatorDefault, str, attrs);

        CTTypesetterRef ts = CTTypesetterCreateWithAttributedString(attrStr);
        CTLineRef line = CTTypesetterCreateLine(ts, CFRangeMake(0, len16));

        std::vector<uint8_t> u16Levels(len16, 0);
        CFArrayRef runs = CTLineGetGlyphRuns(line);
        CFIndex runCount = CFArrayGetCount(runs);
        for (CFIndex i = 0; i < runCount; ++i) {
            CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(runs, i);
            CFRange r = CTRunGetStringRange(run);
            CTRunStatus st = CTRunGetStatus(run);
            bool isRTL = (st & kCTRunStatusRightToLeft) != 0;
            uint8_t lvl = 0;
            if (dir == TextDirection::kRTL) {
                lvl = isRTL ? 1 : 2;
            } else {
                lvl = isRTL ? 1 : 0;
            }
            for (CFIndex k = r.location; k < r.location + r.length && k < len16; ++k) {
                u16Levels[k] = lvl;
            }
        }

        std::vector<size_t> u16ToU8;
        extractUtfConversionMapping(
            SkSpan<const char>(utf8, utf8Units),
            [&](size_t u8) { u16ToU8.push_back(u8); },
            [&](size_t u16) {}
        );

        size_t regionStart8 = 0;
        uint8_t currentLevel = u16Levels[0];
        for (CFIndex i = 1; i < len16; ++i) {
            if (u16Levels[i] != currentLevel) {
                size_t regionEnd8 = u16ToU8[i];
                if (regionEnd8 > regionStart8) {
                    results->emplace_back(regionStart8, regionEnd8, currentLevel);
                    regionStart8 = regionEnd8;
                    currentLevel = u16Levels[i];
                }
            }
        }
        results->emplace_back(regionStart8, (size_t)utf8Units, currentLevel);

        CFRelease(line);
        CFRelease(ts);
        CFRelease(attrStr);
        CFRelease(attrs);
        CFRelease(style);
        CFRelease(str);
        return true;
    }

    bool getWords(const char utf8[], int utf8Units, const char* locale,
                  std::vector<Position>* results) override {
        if (utf8Units <= 0) {
            return true;
        }

        CFStringRef str = CFStringCreateWithBytesNoCopy(
            kCFAllocatorDefault, (const UInt8*)utf8, utf8Units,
            kCFStringEncodingUTF8, false, kCFAllocatorNull);
        if (!str) {
            return false;
        }

        CFIndex len16 = CFStringGetLength(str);
        CFLocaleRef cfLoc = nullptr;
        if (locale) {
            CFStringRef locStr = CFStringCreateWithCString(kCFAllocatorDefault, locale, kCFStringEncodingUTF8);
            if (locStr) {
                cfLoc = CFLocaleCreate(kCFAllocatorDefault, locStr);
                CFRelease(locStr);
            }
        }

        CFStringTokenizerRef tokenizer = CFStringTokenizerCreate(
            kCFAllocatorDefault, str, CFRangeMake(0, len16),
            kCFStringTokenizerUnitWordBoundary, cfLoc);
        if (cfLoc) {
            CFRelease(cfLoc);
        }

        CFStringTokenizerTokenType tokenType = CFStringTokenizerAdvanceToNextToken(tokenizer);
        while (tokenType != kCFStringTokenizerTokenNone) {
            CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer);
            results->push_back(r.location);
            tokenType = CFStringTokenizerAdvanceToNextToken(tokenizer);
        }
        if (!results->empty()) {
            results->push_back(len16);
        }

        CFRelease(tokenizer);
        CFRelease(str);
        return true;
    }

    bool getUtf8Words(const char utf8[], int utf8Units, const char* locale,
                      std::vector<Position>* results) override {
        std::vector<Position> u16Words;
        if (!this->getWords(utf8, utf8Units, locale, &u16Words)) {
            return false;
        }

        std::vector<size_t> u16ToU8;
        extractUtfConversionMapping(
            SkSpan<const char>(utf8, utf8Units),
            [&](size_t u8) { u16ToU8.push_back(u8); },
            [&](size_t u16) {}
        );

        for (auto w16 : u16Words) {
            if (w16 < u16ToU8.size()) {
                results->push_back(u16ToU8[w16]);
            }
        }
        return true;
    }

    bool getSentences(const char utf8[], int utf8Units, const char* locale,
                      std::vector<Position>* results) override {
        if (utf8Units <= 0) {
            return true;
        }

        CFStringRef str = CFStringCreateWithBytesNoCopy(
            kCFAllocatorDefault, (const UInt8*)utf8, utf8Units,
            kCFStringEncodingUTF8, false, kCFAllocatorNull);
        if (!str) {
            return false;
        }

        CFIndex len16 = CFStringGetLength(str);
        CFLocaleRef cfLoc = nullptr;
        if (locale) {
            CFStringRef locStr = CFStringCreateWithCString(kCFAllocatorDefault, locale, kCFStringEncodingUTF8);
            if (locStr) {
                cfLoc = CFLocaleCreate(kCFAllocatorDefault, locStr);
                CFRelease(locStr);
            }
        }

        CFStringTokenizerRef tokenizer = CFStringTokenizerCreate(
            kCFAllocatorDefault, str, CFRangeMake(0, len16),
            kCFStringTokenizerUnitSentence, cfLoc);
        if (cfLoc) {
            CFRelease(cfLoc);
        }

        std::vector<size_t> u16ToU8;
        extractUtfConversionMapping(
            SkSpan<const char>(utf8, utf8Units),
            [&](size_t u8) { u16ToU8.push_back(u8); },
            [&](size_t u16) {}
        );

        CFStringTokenizerTokenType tokenType = CFStringTokenizerAdvanceToNextToken(tokenizer);
        while (tokenType != kCFStringTokenizerTokenNone) {
            CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer);
            if (SkToSizeT(r.location) < u16ToU8.size()) {
                results->push_back(u16ToU8[r.location]);
            }
            tokenType = CFStringTokenizerAdvanceToNextToken(tokenizer);
        }
        if (!results->empty()) {
            results->push_back(utf8Units);
        }

        CFRelease(tokenizer);
        CFRelease(str);
        return true;
    }

    bool computeCodeUnitFlags(char utf8[], int utf8Units, bool replaceTabs,
                              TArray<CodeUnitFlags, true>* results) override {
        results->clear();
        results->push_back_n(utf8Units + 1, CodeUnitFlags::kNoCodeUnitFlag);

        if (utf8Units == 0) {
            return true;
        }

        CFStringRef str = CFStringCreateWithBytesNoCopy(
            kCFAllocatorDefault, (const UInt8*)utf8, utf8Units,
            kCFStringEncodingUTF8, false, kCFAllocatorNull);
        if (!str) {
            return false;
        }

        CFIndex len16 = CFStringGetLength(str);
        std::vector<size_t> u16ToU8;
        extractUtfConversionMapping(
            SkSpan<const char>(utf8, utf8Units),
            [&](size_t u8) { u16ToU8.push_back(u8); },
            [&](size_t u16) {}
        );

        CFIndex idx = 0;
        while (idx < len16) {
            CFRange r = CFStringGetRangeOfComposedCharactersAtIndex(str, idx);
            if (SkToSizeT(r.location) < u16ToU8.size()) {
                (*results)[u16ToU8[r.location]] |= CodeUnitFlags::kGraphemeStart;
            }
            CFIndex advance = std::max<CFIndex>(r.length, 1);
            idx = r.location + advance;
        }
        (*results)[utf8Units] |= CodeUnitFlags::kGraphemeStart;

        CFStringTokenizerRef lineTokenizer = CFStringTokenizerCreate(
            kCFAllocatorDefault, str, CFRangeMake(0, len16),
            kCFStringTokenizerUnitLineBreak, nullptr);

        (*results)[0] |= CodeUnitFlags::kSoftLineBreakBefore;
        CFStringTokenizerTokenType tokenType = CFStringTokenizerAdvanceToNextToken(lineTokenizer);
        while (tokenType != kCFStringTokenizerTokenNone) {
            CFRange r = CFStringTokenizerGetCurrentTokenRange(lineTokenizer);
            CFIndex boundary16 = r.location + r.length;
            if (SkToSizeT(boundary16) < u16ToU8.size()) {
                (*results)[u16ToU8[boundary16]] |= CodeUnitFlags::kSoftLineBreakBefore;
            }
            tokenType = CFStringTokenizerAdvanceToNextToken(lineTokenizer);
        }
        (*results)[utf8Units] |= CodeUnitFlags::kSoftLineBreakBefore;
        CFRelease(lineTokenizer);

        const char* current = utf8;
        const char* end = utf8 + utf8Units;
        while (current < end) {
            auto before = current - utf8;
            SkUnichar unichar = SkUTF::NextUTF8(&current, end);
            if (unichar < 0) {
                unichar = 0xFFFD;
            }
            auto after = current - utf8;

            if (this->isHardBreak(unichar)) {
                (*results)[after] |= (CodeUnitFlags::kHardLineBreakBefore | CodeUnitFlags::kSoftLineBreakBefore);
            }

            if (replaceTabs && this->isTabulation(unichar)) {
                results->at(before) |= CodeUnitFlags::kTabulation;
                unichar = ' ';
                utf8[before] = ' ';
            }

            for (auto i = before; i < after; ++i) {
                if (this->isSpace(unichar)) {
                    results->at(i) |= CodeUnitFlags::kPartOfIntraWordBreak;
                }
                if (this->isWhitespace(unichar)) {
                    results->at(i) |= CodeUnitFlags::kPartOfWhiteSpaceBreak;
                }
                if (this->isControl(unichar)) {
                    results->at(i) |= CodeUnitFlags::kControl;
                }
                if (this->isIdeographic(unichar)) {
                    results->at(i) |= CodeUnitFlags::kIdeographic;
                }
            }
        }

        CFRelease(str);
        return true;
    }

    bool computeCodeUnitFlags(char16_t utf16[], int utf16Units, bool replaceTabs,
                              TArray<CodeUnitFlags, true>* results) override {
        results->clear();
        results->push_back_n(utf16Units + 1, CodeUnitFlags::kNoCodeUnitFlag);

        if (utf16Units == 0) {
            return true;
        }

        CFStringRef str = CFStringCreateWithCharactersNoCopy(
            kCFAllocatorDefault, (const UniChar*)utf16, utf16Units, kCFAllocatorNull);
        if (!str) {
            return false;
        }

        CFIndex idx = 0;
        while (idx < utf16Units) {
            CFRange r = CFStringGetRangeOfComposedCharactersAtIndex(str, idx);
            (*results)[r.location] |= CodeUnitFlags::kGraphemeStart;
            CFIndex advance = std::max<CFIndex>(r.length, 1);
            idx = r.location + advance;
        }
        (*results)[utf16Units] |= CodeUnitFlags::kGraphemeStart;

        CFStringTokenizerRef lineTokenizer = CFStringTokenizerCreate(
            kCFAllocatorDefault, str, CFRangeMake(0, utf16Units),
            kCFStringTokenizerUnitLineBreak, nullptr);

        (*results)[0] |= CodeUnitFlags::kSoftLineBreakBefore;
        CFStringTokenizerTokenType tokenType = CFStringTokenizerAdvanceToNextToken(lineTokenizer);
        while (tokenType != kCFStringTokenizerTokenNone) {
            CFRange r = CFStringTokenizerGetCurrentTokenRange(lineTokenizer);
            (*results)[r.location + r.length] |= CodeUnitFlags::kSoftLineBreakBefore;
            tokenType = CFStringTokenizerAdvanceToNextToken(lineTokenizer);
        }
        (*results)[utf16Units] |= CodeUnitFlags::kSoftLineBreakBefore;
        CFRelease(lineTokenizer);

        this->forEachCodepoint((char16_t*)&utf16[0], utf16Units,
            [this, results, replaceTabs, &utf16](SkUnichar unichar, int32_t start, int32_t end) {
                if (this->isHardBreak(unichar)) {
                    (*results)[end] |= (CodeUnitFlags::kHardLineBreakBefore | CodeUnitFlags::kSoftLineBreakBefore);
                }
                for (auto i = start; i < end; ++i) {
                    if (replaceTabs && this->isTabulation(unichar)) {
                        results->at(i) |= CodeUnitFlags::kTabulation;
                        unichar = ' ';
                        utf16[start] = ' ';
                    }
                    if (this->isSpace(unichar)) {
                        results->at(i) |= CodeUnitFlags::kPartOfIntraWordBreak;
                    }
                    if (this->isWhitespace(unichar)) {
                        results->at(i) |= CodeUnitFlags::kPartOfWhiteSpaceBreak;
                    }
                    if (this->isControl(unichar)) {
                        results->at(i) |= CodeUnitFlags::kControl;
                    }
                    if (this->isIdeographic(unichar)) {
                        results->at(i) |= CodeUnitFlags::kIdeographic;
                    }
                }
            });

        CFRelease(str);
        return true;
    }
};

namespace SkUnicodes::Apple {
sk_sp<SkUnicode> Make() {
    return sk_make_sp<SkUnicode_apple>();
}
} // namespace SkUnicodes::Apple
