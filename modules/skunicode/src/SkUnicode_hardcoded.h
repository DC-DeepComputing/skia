/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicode_hardcoded_DEFINED
#define SkUnicode_hardcoded_DEFINED

#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkBitmaskEnum.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "modules/skunicode/include/SkUnicode.h"
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

class SkUnicodeHardCodedCharProperties : public SkUnicode {
public:
    ~SkUnicodeHardCodedCharProperties() override;

    bool isControl(SkUnichar utf8) override;
    bool isWhitespace(SkUnichar utf8) override;
    bool isSpace(SkUnichar utf8) override;
    bool isTabulation(SkUnichar utf8) override;
    bool isHardBreak(SkUnichar utf8) override;
    bool isEmoji(SkUnichar utf8) override;
    bool isIdeographic(SkUnichar utf8) override;
};

#endif // SkUnicode_hardcoded_DEFINED
