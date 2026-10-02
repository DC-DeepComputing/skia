/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUnicode_apple_DEFINED
#define SkUnicode_apple_DEFINED

#include "modules/skunicode/include/SkUnicode.h"

#include <memory>

namespace SkUnicodes::Apple {
SKUNICODE_API sk_sp<SkUnicode> Make();

// Critical Unicode code points used in line breaking, emoji shaping, and text segmentation.
namespace CodePoints {
    // =========================================================================
    // Mandatory (Hard) Line Breaks (Unicode Standard Annex #14: UAX #14)
    // =========================================================================
    // Line Feed ('\n', U+000A): Standard Unix newline; ends a paragraph.
    inline constexpr SkUnichar kLineFeed                 = 0x000A;
    // Line Tabulation / Vertical Tab ('\v', U+000B): C0 control code for line advance.
    inline constexpr SkUnichar kVerticalTab              = 0x000B;
    // Form Feed ('\f', U+000C): C0 control code for page/form break.
    inline constexpr SkUnichar kFormFeed                 = 0x000C;
    // Carriage Return ('\r', U+000D): Standard classic Mac newline / CRLF lead byte.
    inline constexpr SkUnichar kCarriageReturn           = 0x000D;
    // Next Line (NEL, U+0085): C1 control code indicating a newline in EBCDIC and ISO 8859.
    inline constexpr SkUnichar kNextLine                 = 0x0085;
    // Line Separator (LS, U+2028): Unconditionally breaks a line without ending a paragraph.
    inline constexpr SkUnichar kLineSeparator            = 0x2028;
    // Paragraph Separator (PS, U+2029): Unconditionally breaks a line and ends a paragraph.
    inline constexpr SkUnichar kParagraphSeparator       = 0x2029;

    // =========================================================================
    // Emoji Syntax & Structure Code Points (Unicode Technical Report #51: TR51)
    // =========================================================================
    // Zero Width Joiner (ZWJ, U+200D): Glues multiple emojis into a single glyph sequence
    // (e.g., Man + ZWJ + Woman + ZWJ + Boy = Family emoji).
    inline constexpr SkUnichar kZeroWidthJoiner          = 0x200D;
    // Combining Enclosing Keycap (U+20E3): Applied after '0'-'9', '#', or '*' to produce keycaps (e.g. 1️⃣).
    inline constexpr SkUnichar kCombiningEnclosingKeycap = 0x20E3;
    // Variation Selector-16 (VS-16, U+FE0F): Specifies that the preceding character must be
    // displayed with colorful emoji presentation rather than text/monochrome presentation (e.g. ⚠️ vs ⚠).
    inline constexpr SkUnichar kVariationSelector16      = 0xFE0F;

    // Regional Indicator Symbols (U+1F1E6..U+1F1FF):
    // Pairs of regional indicators encode ISO 3166-1 alpha-2 two-letter country codes (e.g., 🇯🇵 = JP).
    inline constexpr SkUnichar kRegionalIndicatorStart   = 0x1F1E6; // Regional Indicator Symbol Letter A
    inline constexpr SkUnichar kRegionalIndicatorEnd     = 0x1F1FF; // Regional Indicator Symbol Letter Z

    // Fitzpatrick Skin Tone Modifiers (U+1F3FB..U+1F3FF):
    // Applied to modifier-base emojis to select human skin tones from the Fitzpatrick scale.
    inline constexpr SkUnichar kModifierFitzpatrickStart = 0x1F3FB; // Type-1-2 (Light skin tone)
    inline constexpr SkUnichar kModifierFitzpatrickEnd   = 0x1F3FF; // Type-6 (Dark skin tone)

    // Emoji Hair Components (U+1F9B0..U+1F9B3):
    // Combined via ZWJ to specify hair color/style (e.g. Woman + ZWJ + Red Hair).
    inline constexpr SkUnichar kEmojiHairComponentStart  = 0x1F9B0; // Red hair
    inline constexpr SkUnichar kEmojiHairComponentEnd    = 0x1F9B3; // Bald

    // Emoji Tag Characters (U+E0020..U+E007F):
    // Used in subdivision flag sequences (e.g., Black Flag + 'g''b''s''c''t' + Cancel Tag = 🏴󠁧󠁢󠁳󠁣󠁴󠁿 Scotland).
    inline constexpr SkUnichar kTagSpace                 = 0xE0020; // First emoji tag character (tag space)
    inline constexpr SkUnichar kTagTilde                 = 0xE007E; // Last ASCII-mirroring emoji tag character
    inline constexpr SkUnichar kTagCancel                = 0xE007F; // Emoji cancel tag (terminator for tag sequence)
}
}

#endif // SkUnicode_apple_DEFINED
