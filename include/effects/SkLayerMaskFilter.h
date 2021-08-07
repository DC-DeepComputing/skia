/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLayerMaskFilter_DEFINED
#define SkLayerMaskFilter_DEFINED

#include "include/core/SkPathEffect.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPoint.h"
#include "include/core/SkBlendMode.h"

class SK_API SkLayerMaskFilter {
public:
    struct Layer {
        sk_sp<SkPathEffect> patheffect;
        sk_sp<SkMaskFilter> maskfilter;
        SkVector            offset      = {0, 0};
        SkBlendMode         blend       = SkBlendMode::kSrcOver;
        SkAlpha             alpha       = 0xFF;
    };

    static sk_sp<SkMaskFilter> Make(const Layer[], int count);
};

#endif
