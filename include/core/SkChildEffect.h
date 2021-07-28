/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkChildEffect_DEFINED
#define SkChildEffect_DEFINED

#include "include/core/SkFlattenable.h"

/** \class SkChildEffect
 *
 *  A base class for shaders and color filters; indicates support for Runtime Effect sample().
 */
class SK_API SkChildEffect : public SkFlattenable {
public:
    enum class Type {
        kShader,
        kColorFilter,
        kBlender,
    };

    virtual Type effectType() const = 0;
};

#endif
