/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGradientFlags_DEFINED
#define SkGradientFlags_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkMacros.h"

enum class SkGradientFlags : uint32_t {
    kNone                        = 0x0,
    kInterpolateColorsInPremul   = 0x1,
};
SK_MAKE_BITFIELD_OPS(SkGradientFlags)

#endif // SkGradientFlags_DEFINED
