/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Encoding_DEFINED
#define skgpu_Encoding_DEFINED

#include <vector>
#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTileMode.h"

/**************************************************************************************************/
enum class Mapping : uint8_t {
    kGanesh,
    kGraphite,
    kSkVM
};

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
/* These logically attach to the SkBlendMode enum */
static constexpr int kSkBlendModeCount = static_cast<int>(SkBlendMode::kLastMode) + 1;
static const int kBlendModeBits = 5;
static const int kBlendModeMask =  (0x1 << kBlendModeBits) - 1;

SkBlendMode ExtractBlendMode(uint32_t code);

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

class SkPaintParamsKey;

namespace skgpu {
    class ShaderCodeDictionary;
    class UniformData;
};

namespace BlendMode {

    int ToCodeB(skgpu::ShaderCodeDictionary*, Mapping, SkPaintParamsKey*, std::vector<sk_sp<skgpu::UniformData>>*, SkBlendMode);

    int Encode2(skgpu::ShaderCodeDictionary*, Mapping, SkPaintParamsKey*, SkBlendMode);
    void Dump(const uint32_t* data, int maxIndex);

} // namespace BlendMode

/**************************************************************************************************/

#endif // skgpu_Encoding_DEFINED
