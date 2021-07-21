/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRuntimeEffectChild_DEFINED
#define SkRuntimeEffectChild_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkTileMode.h"

/** \class SkRuntimeEffectChild
 *
 *  A mix-in class for shaders and color filters; indicates support for Runtime Effect sample().
 */
class SK_API SkRuntimeEffectChild {
public:
    virtual ~SkRuntimeEffectChild() = default;

    enum class EffectType {
        kShader,
        kColorFilter,
    };

    virtual EffectType effectType() const = 0;
};

#endif
