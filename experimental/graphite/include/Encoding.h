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

enum class SkBackend : uint8_t;
class SkUniformBlock;

/**************************************************************************************************/
/* These logically attach to the SkTileMode enum */
//inline static constexpr int kTileModeBits = 2;
//inline static constexpr int kTileModeMask = 0x3;

#if 0
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

#endif

/**************************************************************************************************/
/* These logically attach to the SkBlendMode enum */
static constexpr int kSkBlendModeCount = static_cast<int>(SkBlendMode::kLastMode) + 1;
static const int kBlendModeBits = 5;
static const int kBlendModeMask =  (0x1 << kBlendModeBits) - 1;

class SkPaintParamsKey;
class SkShaderCodeDictionary;

namespace BlendMode {

    void addToKeyB(SkShaderCodeDictionary*,
                   SkBackend,
                   SkPaintParamsKey*,
                   SkUniformBlock*,
                   SkBlendMode);

} // namespace BlendMode

/**************************************************************************************************/

#endif // skgpu_Encoding_DEFINED
