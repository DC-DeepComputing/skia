/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImageEncoder.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <string.h>
#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <thread>

#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/src/SkUnicode_icu4x.h"

UNIX_ONLY_TEST(SkUnicode_IsBreakingWhitespace, reporter) {

    UChar32 whitespaces [] = {
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
        0x3000, // ideographic space
    };
    UChar32 letters[] = { 'a', 'b', 'c', 'd' };

    auto icu4x = SkUnicode::MakeIcu4xBasedUnicode();

    for (auto& uc : whitespaces) {
        REPORTER_ASSERT(reporter, icu4x->isBreakingWhitespace(uc));
    }

    for (auto& uc : letters) {
        REPORTER_ASSERT(reporter, !icu4x->isBreakingWhitespace(uc));
    }
};

UNIX_ONLY_TEST(SkUnicode_ToUpper, reporter) {

    SkString lower("abcdefghijklmnopqrstuvwxyz");
    SkString upper("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    auto icu4x = SkUnicode::MakeIcu4xBasedUnicode();

    auto test = icu4x->toUpper(lower);

    REPORTER_ASSERT(reporter, upper.equals(test));
    REPORTER_ASSERT(reporter, !lower.equals(test));
};



