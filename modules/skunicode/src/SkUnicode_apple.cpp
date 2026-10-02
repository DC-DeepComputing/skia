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

// In UTF-8, continuation bytes have the binary prefix 10xxxxxx (masked with 0xC0 == 0x80).
static constexpr uint8_t kUtf8ContinuationMask = 0xC0;
static constexpr uint8_t kUtf8ContinuationBits = 0x80;

// Standard Unicode replacement character (U+FFFD) used for invalid UTF sequences.
static constexpr SkUnichar kReplacementCharacter = 0xFFFD;

// Returns the SkUnichar code point immediately preceding the byte offset `pos` in `utf8`.
// In UTF-8, continuation bytes match the bit pattern 10xxxxxx (0x80..0xBF).
// We walk backwards from `target - 1` to find the sequence lead byte, then decode
// forward using SkUTF::NextUTF8. Returns 0 if pos == 0.
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

// Returns the SkUnichar code point immediately preceding code unit index `pos` in `utf16`.
// Handles UTF-16 surrogate pairs: if the code unit before `pos` is a trailing surrogate
// (0xDC00..0xDFFF) preceded by a leading surrogate (0xD800..0xDBFF), both units are
// decoded into a supplementary code point (e.g. SMP emojis). Returns 0 if pos == 0.
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

// Returns true if `u` is a mandatory (hard) line break character per Unicode Standard Annex #14.
// Checks named constants: LF ('\n'), CR ('\r'), NEL (U+0085), VT (U+000B), FF (U+000C),
// Line Separator (U+2028), and Paragraph Separator (U+2029).
static bool is_hard_break(SkUnichar u) {
    return u == kLineFeed ||
           u == kCarriageReturn ||
           u == kNextLine ||
           u == kVerticalTab ||
           u == kFormFeed ||
           u == kLineSeparator ||
           u == kParagraphSeparator;
}

// Returns true if code point `u` may start an emoji sequence.
// ASCII keycap sequences ('0'-'9', '#', '*') are valid sequence starts.
// NOTE: We use discrete Unicode 15/16 ranges with binary search rather than broad contiguous
// spans. A naive span check like 0x203C..0x3299 falsely classifies ~4,700 BMP characters
// as emojis—including all Japanese Hiragana (U+3040..U+309F), Katakana (U+30A0..U+30FF),
// math operators, and currency symbols—corrupting font selection in OneLineShaper.
static bool is_emoji(SkUnichar u) {
    if ((u >= '0' && u <= '9') || u == '#' || u == '*') {
        return true;
    }
    static constexpr std::pair<SkUnichar, SkUnichar> kEmojiRanges[] = {
        { 0x00A9, 0x00A9 }, // Copyright symbol (©)
        { 0x00AE, 0x00AE }, // Registered trademark (®)
        { 0x203C, 0x203C }, // Double exclamation mark (‼)
        { 0x2049, 0x2049 }, // Exclamation question mark (⁉)
        { 0x2122, 0x2122 }, // Trade mark sign (™)
        { 0x2139, 0x2139 }, // Information source (ℹ)
        { 0x2194, 0x2199 }, // Arrows: left-right, up-down, NW, NE, SE, SW (↔..↙)
        { 0x21A9, 0x21AA }, // Left/right curved arrows (↩, ↪)
        { 0x231A, 0x231B }, // Watch, Hourglass (⌚, ⌛)
        { 0x2328, 0x2328 }, // Keyboard (⌨)
        { 0x23CF, 0x23CF }, // Eject symbol (⏏)
        { 0x23E9, 0x23F3 }, // Media controls & timer symbols (⏩..⏳)
        { 0x23F8, 0x23FA }, // Pause, stop, record buttons (⏸, ⏹, ⏺)
        { 0x24C2, 0x24C2 }, // Circled letter M (Ⓜ)
        { 0x25AA, 0x25AB }, // Small black/white squares (▪, ▫)
        { 0x25B6, 0x25B6 }, // Black right-pointing triangle (▶)
        { 0x25C0, 0x25C0 }, // Black left-pointing triangle (◀)
        { 0x25FB, 0x25FE }, // Medium black/white squares (◻..◾)
        { 0x2600, 0x27BF }, // Misc Symbols & Dingbats (☀️, ☔, ⚡, ⚽, ✈️, ✂️, ✉️, etc.)
        { 0x2934, 0x2935 }, // Curved arrows rightwards up/down (⤴, ⤵)
        { 0x2B05, 0x2B07 }, // Directional black arrows (⬅, ⬆, ⬇)
        { 0x2B1B, 0x2B1C }, // Large black/white squares (⬛, ⬜)
        { 0x2B50, 0x2B50 }, // Medium white star (⭐)
        { 0x2B55, 0x2B55 }, // Heavy large circle (⭕)
        { 0x3030, 0x3030 }, // Wavy dash (〰)
        { 0x303D, 0x303D }, // Part alternation mark (〽)
        { 0x3297, 0x3297 }, // Circled ideograph congratulation (㊗)
        { 0x3299, 0x3299 }, // Circled ideograph secret (㊙)
        { 0x1F004, 0x1F004 }, // Mahjong tile red dragon (🀄)
        { 0x1F0CF, 0x1F0CF }, // Playing card black joker (🃏)
        { 0x1F170, 0x1F171 }, // Negative squared letters A, B (🅰, 🅱)
        { 0x1F17E, 0x1F17F }, // Negative squared letters O, P (🅾, 🅿)
        { 0x1F18E, 0x1F18E }, // Negative squared AB (🆎)
        { 0x1F191, 0x1F19A }, // Squared words: CL, COOL, FREE, ID, NEW, NG, OK, SOS, UP, VS
        { kRegionalIndicatorStart, kRegionalIndicatorEnd }, // Regional indicator symbols A-Z (🇦..🇿)
        { 0x1F201, 0x1F202 }, // Squared Katakana: koko, sa (🈁, 🈂)
        { 0x1F21A, 0x1F21A }, // Squared ideograph free of charge (🈚)
        { 0x1F22F, 0x1F22F }, // Squared ideograph point (🈯)
        { 0x1F232, 0x1F23A }, // Squared ideographs: prohibition, vacant, pass, etc. (🈲..🈺)
        { 0x1F250, 0x1F251 }, // Circled ideographs advantage, accept (🉐, 🉑)
        { 0x1F300, 0x1F5FF }, // Misc Symbols & Pictographs (weather, food, animals, tools)
        { 0x1F600, 0x1F64F }, // Emoticons / Smileys (😀..🙏)
        { 0x1F680, 0x1F6FF }, // Transport & Map Symbols (🚀..🛿)
        { 0x1F7E0, 0x1F7EB }, // Geometric Shapes Extended (colored circles & squares 🟠..🟫)
        { 0x1F7F0, 0x1F7F0 }, // Heavy equals sign (🟰)
        { 0x1F900, 0x1FAFF }, // Supplemental Symbols & Pictographs Extended (🤐..🫿)
    };
    auto it = std::lower_bound(
        std::begin(kEmojiRanges), std::end(kEmojiRanges), u,
        [](const std::pair<SkUnichar, SkUnichar>& range, SkUnichar val) {
            return range.second < val;
        });
    return (it != std::end(kEmojiRanges) && u >= it->first && u <= it->second);
}

// Returns true if code point `u` is an emoji modifier base per Unicode TR51 (emoji-data.txt).
// These are characters that can take Fitzpatrick skin tone modifiers (U+1F3FB..U+1F3FF),
// covering people, hand gestures, and activities across Unicode 6.0 through 15.0.
static bool is_emoji_modifier_base(SkUnichar u) {
    static constexpr std::pair<SkUnichar, SkUnichar> kModifierBaseRanges[] = {
        { 0x261D, 0x261D }, // Index Pointing Up (☝)
        { 0x26F9, 0x26F9 }, // Person Bouncing Ball (⛹)
        { 0x270A, 0x270D }, // Raised Fist, Raised Hand, Victory Hand, Writing Hand (✊, ✋, ✌, ✍)
        { 0x1F385, 0x1F385 }, // Father Christmas (🎅)
        { 0x1F3C2, 0x1F3C4 }, // Snowboarder, Runner, Surfer (🏂, 🏃, 🏄)
        { 0x1F3C7, 0x1F3C7 }, // Horse Racing (🏇)
        { 0x1F3CA, 0x1F3CC }, // Swimmer, Weight Lifter, Golfer (🏊, 🏋, 🏌)
        { 0x1F442, 0x1F443 }, // Ear, Nose (👂, 👃)
        { 0x1F446, 0x1F450 }, // Hand signs: pointing, waving, clapping, open hands (👆..👐)
        { 0x1F466, 0x1F478 }, // People: boys, girls, men, women, families, royalty (👦..👸)
        { 0x1F481, 0x1F487 }, // Gestures: tipping hand, haircut, massage (💁..💇)
        { 0x1F4AA, 0x1F4AA }, // Flexed Biceps (💪)
        { 0x1F574, 0x1F575 }, // Man in Suit Levitating, Sleuth (🕴, 🕵)
        { 0x1F57A, 0x1F57A }, // Man Dancing (🕺)
        { 0x1F590, 0x1F590 }, // Hand with Fingers Splayed (🖐)
        { 0x1F595, 0x1F596 }, // Reversed Middle Finger, Vulcan Salute (🖕, 🖖)
        { 0x1F645, 0x1F647 }, // Person Gesturing No, OK, Bowing (🙅, 🙆, 🙇)
        { 0x1F64B, 0x1F64F }, // Person Raising Hand, Folded Hands (🙋..🙏)
        { 0x1F6A3, 0x1F6A3 }, // Rowboat (🚣)
        { 0x1F6B4, 0x1F6B6 }, // Bicyclist, Mountain Bicyclist, Pedestrian (🚴, 🚵, 🚶)
        { 0x1F6C0, 0x1F6C0 }, // Bath (🛀)
        { 0x1F6CC, 0x1F6CC }, // Person in Bed (🛌)
        { 0x1F90C, 0x1F90C }, // Pinched Fingers (🤌)
        { 0x1F90F, 0x1F90F }, // Pinching Hand (🤏)
        { 0x1F918, 0x1F91F }, // Hand gestures: horns, call me, fingers crossed, love you (🤘..🤟)
        { 0x1F926, 0x1F926 }, // Face Palm (🤦)
        { 0x1F930, 0x1F939 }, // Pregnant Woman..Juggling (🤰..🤹)
        { 0x1F933, 0x1F93E }, // Sports: fencer, cartwheeling, water polo, handball (🤺..🤾)
        { 0x1F977, 0x1F977 }, // Ninja (🥷)
        { 0x1F9B5, 0x1F9B6 }, // Leg, Foot (🦵, 🦶)
        { 0x1F9B8, 0x1F9B9 }, // Superhero, Supervillain (🦸, 🦹)
        { 0x1F9BB, 0x1F9BB }, // Ear with Hearing Aid (🦻)
        { 0x1F9CD, 0x1F9CF }, // Person Standing, Kneeling, Deaf Person (🧍, 🧎, 🧏)
        { 0x1F9D1, 0x1F9DD }, // People & mythical: adult, child, elder, genie, zombie, elf (🧑..🧝)
        { 0x1FAC3, 0x1FAC5 }, // Pregnant Man, Pregnant Person, Person with Crown (🫃..🫅)
        { 0x1FAF0, 0x1FAF8 }, // Hand gestures: crossed fingers, pointing, heart hands (🫰..🫸)
    };
    auto it = std::lower_bound(
        std::begin(kModifierBaseRanges), std::end(kModifierBaseRanges), u,
        [](const std::pair<SkUnichar, SkUnichar>& range, SkUnichar val) {
            return range.second < val;
        });
    return (it != std::end(kModifierBaseRanges) && u >= it->first && u <= it->second);
}

// Returns true if code point `u` is an emoji component per Unicode TR51.
// In addition to skin tones, hair components, keycap symbols, ZWJ (U+200D), and VS-16 (U+FE0F),
// this includes Tag characters (U+E0020..U+E007F) used in subdivision flag sequences (e.g. Scotland).
static bool is_emoji_component(SkUnichar u) {
    return (u >= '0' && u <= '9') ||
           (u == '#' || u == '*') ||
           (u >= kRegionalIndicatorStart && u <= kRegionalIndicatorEnd) ||
           (u >= kModifierFitzpatrickStart && u <= kModifierFitzpatrickEnd) ||
           (u >= kEmojiHairComponentStart && u <= kEmojiHairComponentEnd) ||
           (u >= kTagSpace && u <= kTagCancel) ||
           (u == kZeroWidthJoiner) ||
           (u == kVariationSelector16) ||
           (u == kCombiningEnclosingKeycap);
}

class SkBreakIterator_apple : public SkBreakIterator {
public:
    // CFLocaleRef is created once on iterator construction and cached for all setText() calls
    // to avoid expensive locale allocations and releases in hot layout loops.
    // Grapheme break iteration does not depend on locale and skips locale creation entirely.
    SkBreakIterator_apple(SkUnicode::BreakType type, const char* locale)
        : fType(type), fCFLocale(nullptr) {
        if (locale && locale[0] && fType != SkUnicode::BreakType::kGraphemes) {
            CFStringRef locStr = CFStringCreateWithCString(kCFAllocatorDefault, locale, kCFStringEncodingUTF8);
            if (locStr) {
                fCFLocale = CFLocaleCreate(kCFAllocatorDefault, locStr);
                CFRelease(locStr);
            }
        }
    }

    ~SkBreakIterator_apple() override {
        if (fCFLocale) {
            CFRelease(fCFLocale);
        }
    }

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
        CFStringRef cfStr = CFStringCreateWithBytes(
            kCFAllocatorDefault,
            reinterpret_cast<const UInt8*>(utftext8),
            utf8Units,
            kCFStringEncodingUTF8,
            false
        );
        if (!cfStr) {
            return false;
        }
        bool ok = populateBreaks(cfStr, utf8Units, true, utftext8);
        CFRelease(cfStr);
        return ok;
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
        CFStringRef cfStr = CFStringCreateWithCharactersNoCopy(
            kCFAllocatorDefault,
            reinterpret_cast<const UniChar*>(utftext16),
            utf16Units,
            kCFAllocatorNull
        );
        if (!cfStr) {
            return false;
        }
        bool ok = populateBreaks(cfStr, utf16Units, false, utftext16);
        CFRelease(cfStr);
        return ok;
    }

private:
    static constexpr const Position kDone = -1;
    SkUnicode::BreakType fType;
    CFLocaleRef fCFLocale;
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
            // For sentences, CFStringTokenizer emits token ranges [location, location + length).
            // To match ICU's SkBreakIterator contract (and SkUnicodeTest), sentence break positions
            // represent the start of each sentence plus the end of string (e.g. {0, 13, 26, 41, 70}).
            CFStringTokenizerRef tokenizer = CFStringTokenizerCreate(
                kCFAllocatorDefault,
                cfStr,
                CFRangeMake(0, utf16Len),
                kCFStringTokenizerUnitSentence,
                fCFLocale
            );
            if (!tokenizer) {
                return false;
            }
            fBreaks.push_back(0);
            fStatuses.push_back(0);
            while (CFStringTokenizerAdvanceToNextToken(tokenizer) != kCFStringTokenizerTokenNone) {
                CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer);
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
            CFRelease(tokenizer);
        } else {
            CFOptionFlags unitType = (fType == SkUnicode::BreakType::kLines)
                ? kCFStringTokenizerUnitLineBreak
                : kCFStringTokenizerUnitWordBoundary;

            CFStringTokenizerRef tokenizer = CFStringTokenizerCreate(
                kCFAllocatorDefault,
                cfStr,
                CFRangeMake(0, utf16Len),
                unitType,
                fCFLocale
            );
            if (!tokenizer) {
                return false;
            }

            fBreaks.push_back(0);
            fStatuses.push_back(0);
            while (CFStringTokenizerAdvanceToNextToken(tokenizer) != kCFStringTokenizerTokenNone) {
                CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer);
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
                // to distinguish hard breaks from soft wrapping opportunities.
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
                // is tagged with kHardLineBreak so SkParagraph detects the trailing paragraph break.
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
            CFRelease(tokenizer);
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

    bool isEmoji(SkUnichar u) override {
        return is_emoji(u);
    }

    bool isEmojiComponent(SkUnichar u) override {
        return is_emoji_component(u);
    }

    bool isEmojiModifier(SkUnichar u) override {
        return (u >= kModifierFitzpatrickStart && u <= kModifierFitzpatrickEnd);
    }

    bool isEmojiModifierBase(SkUnichar u) override {
        return is_emoji_modifier_base(u);
    }

    bool isRegionalIndicator(SkUnichar u) override {
        return (u >= kRegionalIndicatorStart && u <= kRegionalIndicatorEnd);
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
        CFStringRef cfStr = CFStringCreateWithBytes(
            kCFAllocatorDefault,
            reinterpret_cast<const UInt8*>(utf8),
            utf8Units,
            kCFStringEncodingUTF8,
            false
        );
        if (!cfStr) {
            return false;
        }
        CFIndex utf16Len = CFStringGetLength(cfStr);
        CFLocaleRef cfLocale = nullptr;
        if (localeStr && localeStr[0]) {
            CFStringRef locStr = CFStringCreateWithCString(kCFAllocatorDefault, localeStr, kCFStringEncodingUTF8);
            if (locStr) {
                cfLocale = CFLocaleCreate(kCFAllocatorDefault, locStr);
                CFRelease(locStr);
            }
        }
        CFStringTokenizerRef tokenizer = CFStringTokenizerCreate(
            kCFAllocatorDefault,
            cfStr,
            CFRangeMake(0, utf16Len),
            kCFStringTokenizerUnitWordBoundary,
            cfLocale
        );
        if (!tokenizer) {
            if (cfLocale) CFRelease(cfLocale);
            CFRelease(cfStr);
            return false;
        }
        results->emplace_back(0);
        while (CFStringTokenizerAdvanceToNextToken(tokenizer) != kCFStringTokenizerTokenNone) {
            CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer);
            if (results->back() != SkTo<Position>(r.location)) {
                results->emplace_back(r.location);
            }
            results->emplace_back(r.location + r.length);
        }
        if (results->empty() || results->back() != SkTo<Position>(utf16Len)) {
            results->emplace_back(utf16Len);
        }
        CFRelease(tokenizer);
        if (cfLocale) CFRelease(cfLocale);
        CFRelease(cfStr);
        return true;
    }

    // Direct single-pass: tokenize the CFString and remap UTF-16 token offsets to UTF-8
    // byte indices via extractUtfConversionMapping in one go, avoiding intermediate
    // UTF-16 vector allocations and redundant round-trip conversions.
    bool getUtf8Words(const char utf8[],
                      int utf8Units,
                      const char* localeStr,
                      std::vector<Position>* results) override {
        if (utf8Units <= 0) {
            return true;
        }
        CFStringRef cfStr = CFStringCreateWithBytes(
            kCFAllocatorDefault,
            reinterpret_cast<const UInt8*>(utf8),
            utf8Units,
            kCFStringEncodingUTF8,
            false
        );
        if (!cfStr) {
            return false;
        }
        CFIndex utf16Len = CFStringGetLength(cfStr);
        CFLocaleRef cfLocale = nullptr;
        if (localeStr && localeStr[0]) {
            CFStringRef locStr = CFStringCreateWithCString(kCFAllocatorDefault, localeStr, kCFStringEncodingUTF8);
            if (locStr) {
                cfLocale = CFLocaleCreate(kCFAllocatorDefault, locStr);
                CFRelease(locStr);
            }
        }
        CFStringTokenizerRef tokenizer = CFStringTokenizerCreate(
            kCFAllocatorDefault,
            cfStr,
            CFRangeMake(0, utf16Len),
            kCFStringTokenizerUnitWordBoundary,
            cfLocale
        );
        if (!tokenizer) {
            if (cfLocale) CFRelease(cfLocale);
            CFRelease(cfStr);
            return false;
        }
        std::vector<Position> mapping;
        SkSpan<const char> text(utf8, utf8Units);
        SkUnicode::extractUtfConversionMapping(
            text,
            [&](size_t i8) { mapping.emplace_back(i8); },
            [&](size_t) {}
        );

        auto mapPos = [&](CFIndex idx16) -> Position {
            if (SkToSizeT(idx16) < mapping.size()) return mapping[idx16];
            return utf8Units;
        };

        results->emplace_back(0);
        while (CFStringTokenizerAdvanceToNextToken(tokenizer) != kCFStringTokenizerTokenNone) {
            CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer);
            Position startPos = mapPos(r.location);
            if (results->back() != startPos) {
                results->emplace_back(startPos);
            }
            results->emplace_back(mapPos(r.location + r.length));
        }
        if (results->empty() || results->back() != static_cast<Position>(utf8Units)) {
            results->emplace_back(utf8Units);
        }
        CFRelease(tokenizer);
        if (cfLocale) CFRelease(cfLocale);
        CFRelease(cfStr);
        return true;
    }

    // Sentence boundaries report the start of each sentence plus the end of string,
    // matching ICU SkUnicode and SkBreakIterator contracts (e.g. {0, 13, 26, 41, 70}).
    bool getSentences(const char utf8[],
                      int utf8Units,
                      const char* localeStr,
                      std::vector<SkUnicode::Position>* results) override {
        if (utf8Units <= 0) {
            return true;
        }
        CFStringRef cfStr = CFStringCreateWithBytes(
            kCFAllocatorDefault,
            reinterpret_cast<const UInt8*>(utf8),
            utf8Units,
            kCFStringEncodingUTF8,
            false
        );
        if (!cfStr) {
            return false;
        }
        CFIndex utf16Len = CFStringGetLength(cfStr);
        CFLocaleRef cfLocale = nullptr;
        if (localeStr && localeStr[0]) {
            CFStringRef locStr = CFStringCreateWithCString(kCFAllocatorDefault, localeStr, kCFStringEncodingUTF8);
            if (locStr) {
                cfLocale = CFLocaleCreate(kCFAllocatorDefault, locStr);
                CFRelease(locStr);
            }
        }
        CFStringTokenizerRef tokenizer = CFStringTokenizerCreate(
            kCFAllocatorDefault,
            cfStr,
            CFRangeMake(0, utf16Len),
            kCFStringTokenizerUnitSentence,
            cfLocale
        );
        if (!tokenizer) {
            if (cfLocale) CFRelease(cfLocale);
            CFRelease(cfStr);
            return false;
        }
        std::vector<Position> mapping;
        SkSpan<const char> text(utf8, utf8Units);
        SkUnicode::extractUtfConversionMapping(
            text,
            [&](size_t i8) { mapping.emplace_back(i8); },
            [&](size_t) {}
        );

        auto mapPos = [&](CFIndex idx16) -> Position {
            if (SkToSizeT(idx16) < mapping.size()) return mapping[idx16];
            return utf8Units;
        };

        results->emplace_back(0);
        while (CFStringTokenizerAdvanceToNextToken(tokenizer) != kCFStringTokenizerTokenNone) {
            CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer);
            Position startPos = mapPos(r.location);
            if (results->back() != startPos) {
                results->emplace_back(startPos);
            }
        }
        if (results->empty() || results->back() != static_cast<Position>(utf8Units)) {
            results->emplace_back(utf8Units);
        }
        CFRelease(tokenizer);
        if (cfLocale) CFRelease(cfLocale);
        CFRelease(cfStr);
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

        CFStringRef cfStr = CFStringCreateWithBytes(
            kCFAllocatorDefault,
            reinterpret_cast<const UInt8*>(utf8),
            utf8Units,
            kCFStringEncodingUTF8,
            false
        );
        if (!cfStr) {
            return false;
        }

        CFIndex utf16Len = CFStringGetLength(cfStr);
        std::vector<Position> utf8ForUtf16;
        SkSpan<const char> text(utf8, utf8Units);
        SkUnicode::extractUtfConversionMapping(
            text,
            [&](size_t i8) { utf8ForUtf16.emplace_back(i8); },
            [&](size_t) {}
        );

        // Line breaks
        CFStringTokenizerRef tokenizer = CFStringTokenizerCreate(
            kCFAllocatorDefault,
            cfStr,
            CFRangeMake(0, utf16Len),
            kCFStringTokenizerUnitLineBreak,
            nullptr
        );
        if (tokenizer) {
            (*results)[0] |= CodeUnitFlags::kSoftLineBreakBefore;
            while (CFStringTokenizerAdvanceToNextToken(tokenizer) != kCFStringTokenizerTokenNone) {
                CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer);
                size_t pos16 = r.location;
                if (pos16 > 0 && pos16 <= utf8ForUtf16.size()) {
                    size_t pos8 = (pos16 < utf8ForUtf16.size()) ? utf8ForUtf16[pos16] : utf8Units;
                    (*results)[pos8] |= CodeUnitFlags::kSoftLineBreakBefore;
                    // CFStringTokenizer line breaks don't distinguish soft from hard breaks.
                    // Decode the preceding codepoint (walking backwards over UTF-8 continuation bytes)
                    // to determine if this is a mandatory (hard) break.
                    SkUnichar prev = get_prev_codepoint_utf8(utf8, pos8);
                    if (this->isHardBreak(prev)) {
                        (*results)[pos8] |= CodeUnitFlags::kHardLineBreakBefore;
                    }
                }
            }
            (*results)[utf8Units] |= CodeUnitFlags::kSoftLineBreakBefore;
            // If the text ends with a newline/hard break, tag the trailing boundary so that
            // SkParagraph's trailing newline detection and height calculations succeed.
            SkUnichar lastChar = get_prev_codepoint_utf8(utf8, utf8Units);
            if (this->isHardBreak(lastChar)) {
                (*results)[utf8Units] |= CodeUnitFlags::kHardLineBreakBefore;
            }
            CFRelease(tokenizer);
        }

        // Grapheme clusters
        CFIndex idx16 = 0;
        while (idx16 < utf16Len) {
            CFRange r = CFStringGetRangeOfComposedCharactersAtIndex(cfStr, idx16);
            if (SkToSizeT(r.location) < utf8ForUtf16.size()) {
                size_t pos8 = utf8ForUtf16[r.location];
                (*results)[pos8] |= CodeUnitFlags::kGraphemeStart;
            }
            idx16 = r.location + r.length;
        }
        (*results)[utf8Units] |= CodeUnitFlags::kGraphemeStart;
        CFRelease(cfStr);

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
                if (replaceTabs) {
                    unichar = ' ';
                    utf8[before] = ' ';
                }
            }
            for (auto i = before; i < after; ++i) {
                if (this->isSpace(unichar)) {
                    results->at(i) |= SkUnicode::kPartOfIntraWordBreak;
                }
                if (this->isWhitespace(unichar)) {
                    results->at(i) |= SkUnicode::kPartOfWhiteSpaceBreak;
                }
                if (this->isControl(unichar)) {
                    results->at(i) |= SkUnicode::kControl;
                }
                if (this->isIdeographic(unichar)) {
                    results->at(i) |= SkUnicode::kIdeographic;
                }
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

        CFStringRef cfStr = CFStringCreateWithCharactersNoCopy(
            kCFAllocatorDefault,
            reinterpret_cast<const UniChar*>(utf16),
            utf16Units,
            kCFAllocatorNull
        );
        if (!cfStr) {
            return false;
        }

        // Line breaks
        CFStringTokenizerRef tokenizer = CFStringTokenizerCreate(
            kCFAllocatorDefault,
            cfStr,
            CFRangeMake(0, utf16Units),
            kCFStringTokenizerUnitLineBreak,
            nullptr
        );
        if (tokenizer) {
            (*results)[0] |= CodeUnitFlags::kSoftLineBreakBefore;
            while (CFStringTokenizerAdvanceToNextToken(tokenizer) != kCFStringTokenizerTokenNone) {
                CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer);
                size_t pos = r.location;
                if (pos > 0 && pos <= SkToSizeT(utf16Units)) {
                    (*results)[pos] |= CodeUnitFlags::kSoftLineBreakBefore;
                    // Decode preceding codepoint (handling surrogate pairs) to detect hard breaks.
                    SkUnichar prevChar = get_prev_codepoint_utf16(utf16, pos);
                    if (this->isHardBreak(prevChar)) {
                        (*results)[pos] |= CodeUnitFlags::kHardLineBreakBefore;
                    }
                }
            }
            (*results)[utf16Units] |= CodeUnitFlags::kSoftLineBreakBefore;
            // Tag trailing boundary if the text ends with a hard break.
            SkUnichar lastChar = get_prev_codepoint_utf16(utf16, utf16Units);
            if (this->isHardBreak(lastChar)) {
                (*results)[utf16Units] |= CodeUnitFlags::kHardLineBreakBefore;
            }
            CFRelease(tokenizer);
        }

        // Grapheme clusters
        CFIndex idx = 0;
        while (idx < utf16Units) {
            CFRange r = CFStringGetRangeOfComposedCharactersAtIndex(cfStr, idx);
            (*results)[r.location] |= CodeUnitFlags::kGraphemeStart;
            idx = r.location + r.length;
        }
        (*results)[utf16Units] |= CodeUnitFlags::kGraphemeStart;
        CFRelease(cfStr);

        // Characters / Codepoint properties
        this->forEachCodepoint((char16_t*)&utf16[0], utf16Units,
            [this, results, replaceTabs, &utf16](SkUnichar unichar, int32_t start, int32_t end) {
                for (auto i = start; i < end; ++i) {
                    if (replaceTabs && this->isTabulation(unichar)) {
                        results->at(i) |= SkUnicode::kTabulation;
                        if (replaceTabs) {
                            unichar = ' ';
                            utf16[start] = ' ';
                        }
                    }
                    if (this->isSpace(unichar)) {
                        results->at(i) |= SkUnicode::kPartOfIntraWordBreak;
                    }
                    if (this->isWhitespace(unichar)) {
                        results->at(i) |= SkUnicode::kPartOfWhiteSpaceBreak;
                    }
                    if (this->isControl(unichar)) {
                        results->at(i) |= SkUnicode::kControl;
                    }
                    if (this->isIdeographic(unichar)) {
                        results->at(i) |= SkUnicode::kIdeographic;
                    }
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
        CFStringRef cfStr = CFStringCreateWithBytes(
            kCFAllocatorDefault,
            reinterpret_cast<const UInt8*>(str.c_str()),
            str.size(),
            kCFStringEncodingUTF8,
            false
        );
        if (!cfStr) {
            return str;
        }
        CFMutableStringRef mutStr = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, cfStr);
        CFRelease(cfStr);
        if (!mutStr) {
            return str;
        }
        CFLocaleRef cfLocale = nullptr;
        if (localeStr && localeStr[0]) {
            CFStringRef locStr = CFStringCreateWithCString(kCFAllocatorDefault, localeStr, kCFStringEncodingUTF8);
            if (locStr) {
                cfLocale = CFLocaleCreate(kCFAllocatorDefault, locStr);
                CFRelease(locStr);
            }
        }
        CFStringUppercase(mutStr, cfLocale);
        if (cfLocale) {
            CFRelease(cfLocale);
        }
        CFIndex len = CFStringGetLength(mutStr);
        CFRange range = CFRangeMake(0, len);
        CFIndex usedBufLen = 0;
        CFStringGetBytes(mutStr, range, kCFStringEncodingUTF8, '?', false, nullptr, 0, &usedBufLen);
        // Extract converted bytes into a temporary buffer before constructing SkString,
        // avoiding casting away constness on SkString internal storage.
        std::vector<char> buffer(usedBufLen);
        CFStringGetBytes(mutStr, range, kCFStringEncodingUTF8, '?', false,
                         reinterpret_cast<UInt8*>(buffer.data()), usedBufLen, nullptr);
        CFRelease(mutStr);
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
