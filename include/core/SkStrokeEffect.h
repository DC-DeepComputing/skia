/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStrokeEffect_DEFINED
#define SkStrokeEffect_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"
#include "include/private/base/SkAssert.h"

class SK_API SkStrokeEffect : public SkRefCnt {
public:
    // virtual ~SkStrokeEffect() {}
    virtual float strokeMultiplierAt(SkScalar idx) {
        return 1.0;
    }
    int t_vals = 100;
};

#endif
