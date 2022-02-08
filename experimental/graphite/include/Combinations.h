/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Combinations_DEFINED
#define skgpu_Combinations_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkShader.h"
#include "include/gpu/GrTypes.h"

#include "experimental/graphite/include/Encoding.h"

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

static constexpr SkShader::GradientType CombinableGTToGT(CombinableGradientType type) {
    static_assert((0x1 << (kCombinableGradientTypeCnt-1)) == (int) CombinableGradientType::kLast);
    static_assert((0x1 << (kCombinableGradientTypeCnt))-1 == (int) kAllCombinableGradientTypes);
    SkASSERT((type & kAllCombinableGradientTypes)); // should have 1 & only 1 Type

    switch (type) {
        case CombinableGradientType::kLinear:  return SkShader::kLinear_GradientType;
        case CombinableGradientType::kRadial:  return SkShader::kRadial_GradientType;
        case CombinableGradientType::kConical: return SkShader::kConical_GradientType;
        case CombinableGradientType::kSweep:   return SkShader::kSweep_GradientType;
    }

    SkASSERT(0); // should have 1 & only 1 Type
    return SkShader::kLinear_GradientType;
}

int FirstCombinableGradientType(CombinableGradientType combinableGradientType);

/**************************************************************************************************/

#endif // skgpu_Combinations_DEFINED
