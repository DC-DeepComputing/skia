/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkKnownRuntimeEffects_DEFINED
#define SkKnownRuntimeEffects_DEFINED

#include "include/core/SkTypes.h"
#include <cstdint>

class SkRuntimeEffect;

// We allocate the keys in blocks in the order:
//     Skia builtins
//     Skia known runtime effects
//     Android known runtime effects
//     Chrome known runtime effects
//     unknown runtime effects (on a first come, first served basis -> unstable)
static constexpr int kSkiaBuiltInReservedCnt = 100;
static constexpr int kSkiaKnownRuntimeEffectsReservedCnt = 100;
static constexpr int kAndroidKnownRuntimeEffectsReservedCnt = 100;
static constexpr int kChromeKnownRuntimeEffectsReservedCnt = 100;

static constexpr int kSkiaKnownRuntimeEffectsStart = kSkiaBuiltInReservedCnt;
static constexpr int kSkiaKnownRuntimeEffectsEnd = kSkiaKnownRuntimeEffectsStart +
                                                   kSkiaKnownRuntimeEffectsReservedCnt;

static constexpr int kAndroidKnownRuntimeEffectsStart = kSkiaKnownRuntimeEffectsEnd;
static constexpr int kAndroidKnownRuntimeEffectsEnd = kAndroidKnownRuntimeEffectsStart +
                                                      kAndroidKnownRuntimeEffectsReservedCnt;

static constexpr int kChromeKnownRuntimeEffectsStart = kAndroidKnownRuntimeEffectsEnd;
static constexpr int kChromeKnownRuntimeEffectsEnd = kChromeKnownRuntimeEffectsStart +
                                                     kChromeKnownRuntimeEffectsReservedCnt;

static constexpr int kUnknownRuntimeEffectIDStart = kChromeKnownRuntimeEffectsEnd;


enum class StableKey : int32_t {
    kStart =   kSkiaKnownRuntimeEffectsStart,

    kInvalid = kStart,
    kBlend,

    kLast =    kBlend,
};

static const int kStableKeyCnt = static_cast<int>(StableKey::kLast) -
                                 static_cast<int>(StableKey::kStart) + 1;

static_assert(static_cast<int>(StableKey::kLast) < kSkiaKnownRuntimeEffectsEnd);

SkRuntimeEffect* GetKnownRuntimeEffect(StableKey);

#endif // SkKnownRuntimeEffects_DEFINED
