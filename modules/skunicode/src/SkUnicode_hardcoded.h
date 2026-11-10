/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicode_hardcoded_DEFINED
#define SkUnicode_hardcoded_DEFINED

#include "include/core/SkFourByteTag.h"
#include "include/core/SkTypes.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/core/SkUTF.h"

class SKUNICODE_API SkUnicodeHardCodedCharProperties : public SkUnicode {
public:
    static bool IsDefaultIgnorable(SkUnichar utf8);
    static bool IsControl(SkUnichar utf8);
    static bool IsWhitespace(SkUnichar utf8);
    static bool IsSpace(SkUnichar utf8);
    static bool IsIdeographic(SkUnichar utf8);
    // Returns the ISO 15924 UAX #24 Script tag of utf8 from the generated UCD table, or 'Zzzz'
    // when utf8 is unassigned or outside U+0000..U+10FFFF
    static SkFourByteTag GetScript(SkUnichar utf8);
    // Returns true when utf8 is in General_Category Mn or Me in the generated UCD table
    static bool IsNonSpacingMark(SkUnichar utf8);

    bool isControl(SkUnichar utf8) override;
    bool isWhitespace(SkUnichar utf8) override;
    bool isSpace(SkUnichar utf8) override;
    bool isTabulation(SkUnichar utf8) override;
    bool isHardBreak(SkUnichar utf8) override;
    bool isDefaultIgnorable(SkUnichar utf8) override;
    bool isEmoji(SkUnichar utf8) override;
    bool isEmojiComponent(SkUnichar utf8) override;
    bool isEmojiModifierBase(SkUnichar utf8) override;
    bool isEmojiModifier(SkUnichar utf8) override;
    bool isRegionalIndicator(SkUnichar utf8) override;
    bool isIdeographic(SkUnichar utf8) override;
    SkFourByteTag getScript(SkUnichar utf8) override;
    bool isNonSpacingMark(SkUnichar utf8) override;
};

#endif // SkUnicode_hardcoded_DEFINED
