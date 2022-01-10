/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkDebugUtils.h"

#if defined(SK_DEBUG) || GR_TEST_UTILS
static constexpr const char* SkTileModeToStr(SkTileMode tm) {
    static_assert((int) SkTileMode::kClamp  == 0);
    static_assert((int) SkTileMode::kRepeat == 1);
    static_assert((int) SkTileMode::kMirror == 2);
    static_assert((int) SkTileMode::kDecal  == 3);
    static_assert(kSkTileModeCount == 4);

    const char* kStrings[kSkTileModeCount] = { "kClamp", "kRepeat", "kMirror", "kDecal" };
    return kStrings[(int) tm];
}

static constexpr const char* SkBlendModeToStr(SkBlendMode bm) {
    static_assert(static_cast<int>(SkBlendMode::kClear)       == 0);
    static_assert(static_cast<int>(SkBlendMode::kSrc)         == 1);
    static_assert(static_cast<int>(SkBlendMode::kDst)         == 2);
    static_assert(static_cast<int>(SkBlendMode::kSrcOver)     == 3);
    static_assert(static_cast<int>(SkBlendMode::kDstOver)     == 4);
    static_assert(static_cast<int>(SkBlendMode::kSrcIn)       == 5);
    static_assert(static_cast<int>(SkBlendMode::kDstIn)       == 6);
    static_assert(static_cast<int>(SkBlendMode::kSrcOut)      == 7);
    static_assert(static_cast<int>(SkBlendMode::kDstOut)      == 8);
    static_assert(static_cast<int>(SkBlendMode::kSrcATop)     == 9);
    static_assert(static_cast<int>(SkBlendMode::kDstATop)     == 10);
    static_assert(static_cast<int>(SkBlendMode::kXor)         == 11);
    static_assert(static_cast<int>(SkBlendMode::kPlus)        == 12);
    static_assert(static_cast<int>(SkBlendMode::kModulate)    == 13);
    static_assert(static_cast<int>(SkBlendMode::kScreen)      == 14);

    static_assert(static_cast<int>(SkBlendMode::kOverlay)     == 15);
    static_assert(static_cast<int>(SkBlendMode::kDarken)      == 16);
    static_assert(static_cast<int>(SkBlendMode::kLighten)     == 17);
    static_assert(static_cast<int>(SkBlendMode::kColorDodge)  == 18);
    static_assert(static_cast<int>(SkBlendMode::kColorBurn)   == 19);
    static_assert(static_cast<int>(SkBlendMode::kHardLight)   == 20);
    static_assert(static_cast<int>(SkBlendMode::kSoftLight)   == 21);
    static_assert(static_cast<int>(SkBlendMode::kDifference)  == 22);
    static_assert(static_cast<int>(SkBlendMode::kExclusion)   == 23);
    static_assert(static_cast<int>(SkBlendMode::kMultiply)    == 24);

    static_assert(static_cast<int>(SkBlendMode::kHue)         == 25);
    static_assert(static_cast<int>(SkBlendMode::kSaturation)  == 26);
    static_assert(static_cast<int>(SkBlendMode::kColor)       == 27);
    static_assert(static_cast<int>(SkBlendMode::kLuminosity)  == 28);

    const char* kStrings[kSkBlendModeCount] = {
            "kClear",     "kSrc",        "kDst",        "kSrcOver",    "kDstOver",
            "kSrcIn",     "kDstIn",      "kSrcOut",     "kDstOut",     "kSrcATop",
            "kDstATop",   "kXor",        "kPlus",       "kModulate",   "kScreen",

            "kOverlay",   "kDarken",     "kLighten",    "kColorDodge", "kColorBurn",
            "kHardLight", "kSoftLight",  "kDifference", "kExclusion",  "kMultiply",

            "kHue",       "kSaturation", "kColor",      "kLuminosity"
    };

    return kStrings[(int) bm];
}
#endif
