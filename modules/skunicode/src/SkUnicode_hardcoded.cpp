/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkDebug.h"
#include "modules/skunicode/src/SkUnicode_hardcoded.h"
#include <algorithm>
#include <array>
#include <utility>

bool SkUnicodeHardCodedCharProperties::isControl(SkUnichar utf8) {
    return (utf8 < ' ') || (utf8 >= 0x7f && utf8 <= 0x9f) ||
           (utf8 >= 0x200D && utf8 <= 0x200F) ||
           (utf8 >= 0x202A && utf8 <= 0x202E);
}

bool SkUnicodeHardCodedCharProperties::isWhitespace(SkUnichar unichar) {
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

bool SkUnicodeHardCodedCharProperties::isSpace(SkUnichar unichar) {
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

bool SkUnicodeHardCodedCharProperties::isTabulation(SkUnichar utf8) {
    return utf8 == '\t';
}

bool SkUnicodeHardCodedCharProperties::isHardBreak(SkUnichar utf8) {
    return utf8 == '\n' || utf8 == u'\u2028';
}

namespace {

// =========================================================================
// Emoji Syntax & Structure Code Points (Unicode Technical Report #51: TR51)
// =========================================================================

// Zero Width Joiner (ZWJ, U+200D): Glues multiple emojis into a single sequence glyph
constexpr SkUnichar kZeroWidthJoiner          = 0x200D;
// Combining Enclosing Keycap (U+20E3): Applied after '0'-'9', '#', '*' to produce keycaps (e.g. 1️⃣)
constexpr SkUnichar kCombiningEnclosingKeycap = 0x20E3;
// Variation Selector-16 (VS-16, U+FE0F): Forces emoji/colorful presentation (e.g. ⚠️ vs ⚠)
constexpr SkUnichar kVariationSelector16      = 0xFE0F;

// Regional Indicator Symbols (U+1F1E6..U+1F1FF):
// Pairs of regional indicators encode ISO 3166-1 alpha-2 country flags (e.g., 🇯🇵 = JP)
constexpr SkUnichar kRegionalIndicatorStart   = 0x1F1E6;
constexpr SkUnichar kRegionalIndicatorEnd     = 0x1F1FF;

// Fitzpatrick Skin Tone Modifiers (U+1F3FB..U+1F3FF):
// Applied to modifier-base emojis to select human skin tones from the Fitzpatrick scale
constexpr SkUnichar kModifierFitzpatrickStart = 0x1F3FB;
constexpr SkUnichar kModifierFitzpatrickEnd   = 0x1F3FF;

// Emoji Hair Components (U+1F9B0..U+1F9B3):
// Combined via ZWJ to specify hair color/style (e.g. Woman + ZWJ + Red Hair)
constexpr SkUnichar kEmojiHairComponentStart  = 0x1F9B0;
constexpr SkUnichar kEmojiHairComponentEnd    = 0x1F9B3;

// Emoji Tag Characters (U+E0020..U+E007F):
// Used in subdivision flag sequences (e.g., Scotland flag: Black Flag + 'g''b''s''c''t' + Cancel)
constexpr SkUnichar kTagSpace                 = 0xE0020;
constexpr SkUnichar kTagCancel                = 0xE007F;

// Performs an O(log N) binary search over sorted, non-overlapping [first, second] ranges
template <size_t N>
static bool in_ranges(const std::pair<SkUnichar, SkUnichar> (&ranges)[N], SkUnichar u) {
    auto it = std::lower_bound(
        std::begin(ranges), std::end(ranges), u,
        [](const std::pair<SkUnichar, SkUnichar>& range, SkUnichar val) {
            return range.second < val;
        });
    return it != std::end(ranges) && u >= it->first && u <= it->second;
}

}  // namespace

bool SkUnicodeHardCodedCharProperties::isEmoji(SkUnichar u) {
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
    return in_ranges(kEmojiRanges, u);
}

bool SkUnicodeHardCodedCharProperties::isEmojiComponent(SkUnichar u) {
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

bool SkUnicodeHardCodedCharProperties::isEmojiModifier(SkUnichar u) {
    return (u >= kModifierFitzpatrickStart && u <= kModifierFitzpatrickEnd);
}

bool SkUnicodeHardCodedCharProperties::isEmojiModifierBase(SkUnichar u) {
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
    return in_ranges(kModifierBaseRanges, u);
}

bool SkUnicodeHardCodedCharProperties::isRegionalIndicator(SkUnichar u) {
    return (u >= kRegionalIndicatorStart && u <= kRegionalIndicatorEnd);
}

bool SkUnicodeHardCodedCharProperties::isIdeographic(SkUnichar unichar) {
    static constexpr std::array<std::pair<SkUnichar, SkUnichar>, 8> ranges {{
          {4352,   4607}, // Hangul Jamo
          {11904, 42191}, // CJK_Radicals
          {43072, 43135}, // Phags_Pa
          {44032, 55215}, // Hangul_Syllables
          {63744, 64255}, // CJK_Compatibility_Ideographs
          {65072, 65103}, // CJK_Compatibility_Forms
          {65381, 65500}, // Katakana_Hangul_Halfwidth
          {131072, 196607}// Supplementary_Ideographic_Plane
    }};
    for (auto range : ranges) {
        if (range.first <= unichar && range.second > unichar) {
            return true;
        }
    }
    return false;
}
