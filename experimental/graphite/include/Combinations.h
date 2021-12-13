/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Combinations_DEFINED
#define skgpu_Combinations_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkTileMode.h"
#include "include/gpu/GrTypes.h"

/**************************************************************************************************/
/* These logically attach to the SkTileMode enum */
inline static constexpr int kTileModeBits = 2;
inline static constexpr int kTileModeMask = 0x3;

#if defined(SK_DEBUG) || GR_TEST_UTILS || defined(SK_ENABLE_DUMP_GPU)
static constexpr const char* TileModeToStr(SkTileMode tm) {
    static_assert(kSkTileModeCount == (0x1 << (kTileModeBits)));
    static_assert(kTileModeMask == kSkTileModeCount-1);

    static_assert((int) SkTileMode::kClamp  == 0);
    static_assert((int) SkTileMode::kRepeat == 1);
    static_assert((int) SkTileMode::kMirror == 2);
    static_assert((int) SkTileMode::kDecal  == 3);
    static_assert(kSkTileModeCount == 4);

    const char* kStrings[kSkTileModeCount] = { "kClamp", "kRepeat", "kMirror", "kDecal" };
    return kStrings[(int) tm];
}
#endif

/**************************************************************************************************/
enum class CombinableTileMode : uint8_t {
    kClamp  = 0x1,
    kRepeat = 0x2,
    kMirror = 0x4,
    kDecal  = 0x8,

    kFirst = kClamp,
    kLast = kDecal,
};
static const int kCombinableTileModeCnt = 4;

GR_MAKE_BITFIELD_CLASS_OPS(CombinableTileMode)

inline static constexpr CombinableTileMode kAllCombinableTileModes =
        (CombinableTileMode) (CombinableTileMode::kClamp  |
                              CombinableTileMode::kRepeat |
                              CombinableTileMode::kMirror |
                              CombinableTileMode::kDecal);

static constexpr SkTileMode CombinableTMToTM(CombinableTileMode tm) {
    static_assert((0x1 << (kCombinableTileModeCnt-1)) == (int) CombinableTileMode::kLast);
    static_assert((0x1 << (kCombinableTileModeCnt))-1 == (int) kAllCombinableTileModes);
    SkASSERT((tm & kAllCombinableTileModes)); // should have 1 & only 1 TileMode

    switch (tm) {
        case CombinableTileMode::kClamp:  return SkTileMode::kClamp;
        case CombinableTileMode::kRepeat: return SkTileMode::kRepeat;
        case CombinableTileMode::kMirror: return SkTileMode::kMirror;
        case CombinableTileMode::kDecal:  return SkTileMode::kDecal;
    }

    SkASSERT(0); // should have 1 & only 1 TileMode
    return SkTileMode::kRepeat;
}

int FirstCombinableTileMode(CombinableTileMode combinableTileMode);

/**************************************************************************************************/
enum class GradientCode : uint8_t {
    kLinear,
    kRadial,
    kConical,
    kSweep,

    kLast = kSweep,
};
static constexpr int kGradientCodeCount = static_cast<int>(GradientCode::kLast) + 1;

inline static constexpr int kGradientCodeBits = 2;
inline static constexpr int kGradientCodeMask = (0x1 << kGradientCodeBits) - 1;

#if defined(SK_DEBUG) || GR_TEST_UTILS || defined(SK_ENABLE_DUMP_GPU)
static constexpr const char* GradientTypeToStr(GradientCode gc) {
    static_assert(kGradientCodeCount <= (0x1 << (kGradientCodeBits)));

    static_assert((int) GradientCode::kLinear  == 0);
    static_assert((int) GradientCode::kRadial == 1);
    static_assert((int) GradientCode::kConical == 2);
    static_assert((int) GradientCode::kSweep  == 3);
    static_assert(kGradientCodeCount == 4);

    const char* kStrings[kGradientCodeCount] = { "kLinear", "kRadial", "kConical", "kSweep" };
    return kStrings[(int) gc];
}
#endif

/**************************************************************************************************/
enum class CombinableGradientType : uint8_t {
    kLinear  = 0x1,
    kRadial  = 0x2,
    kConical = 0x4,
    kSweep   = 0x8,

    kFirst = kLinear,
    kLast = kSweep,
};
static const int kCombinableGradientTypeCnt = 4;

GR_MAKE_BITFIELD_CLASS_OPS(CombinableGradientType)

static const constexpr CombinableGradientType kAllCombinableGradientTypes =
        (CombinableGradientType) (CombinableGradientType::kLinear  |
                                  CombinableGradientType::kRadial  |
                                  CombinableGradientType::kConical |
                                  CombinableGradientType::kSweep);

static constexpr GradientCode CombinableGTToGT(CombinableGradientType type) {
    static_assert((0x1 << (kCombinableGradientTypeCnt-1)) == (int) CombinableGradientType::kLast);
    static_assert((0x1 << (kCombinableGradientTypeCnt))-1 == (int) kAllCombinableGradientTypes);
    SkASSERT((type & kAllCombinableGradientTypes)); // should have 1 & only 1 Type

    switch (type) {
        case CombinableGradientType::kLinear:  return GradientCode::kLinear;
        case CombinableGradientType::kRadial:  return GradientCode::kRadial;
        case CombinableGradientType::kConical: return GradientCode::kConical;
        case CombinableGradientType::kSweep:   return GradientCode::kSweep;
    }

    SkASSERT(0); // should have 1 & only 1 Type
    return GradientCode::kLinear;
}

int FirstCombinableGradientType(CombinableGradientType combinableGradientType);

/**************************************************************************************************/
/* These logically attach to the SkBlendMode enum */
static constexpr int kSkBlendModeCount = static_cast<int>(SkBlendMode::kLastMode) + 1;
static const int kBlendModeBits = 5;
static const int kBlendModeMask =  (0x1 << kBlendModeBits) - 1;

#if defined(SK_DEBUG) || GR_TEST_UTILS || defined(SK_ENABLE_DUMP_GPU)
static constexpr const char* BlendModeToStr(SkBlendMode bm) {
    static_assert(kSkBlendModeCount <= (0x1 << (kBlendModeBits)));
    static_assert(kSkBlendModeCount == 29);

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

/**************************************************************************************************/

#endif // skgpu_Combinations_DEFINED
