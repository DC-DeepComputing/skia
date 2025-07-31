/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStrokeAndFillPathEffect_DEFINED
#define SkStrokeAndFillPathEffect_DEFINED

#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"

class SkPathEffect;

class SK_API SkStrokeAndFillPathEffect {
public:
    static sk_sp<SkPathEffect> Make(float strokeWidth, SkPaint::Cap, SkPaint::Join,
                                    float miterLimit = -1);

    static void RegisterFlattenables();
};

#endif
