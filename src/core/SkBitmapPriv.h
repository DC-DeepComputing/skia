/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapPriv_DEFINED
#define SkBitmapPriv_DEFINED

#include "include/core/SkBitmap.h"

#include "include/private/SkColorData.h"

class SkBitmapPriv {
public:
    static bool setColor4f(SkBitmap*, int x, int y, SkColor4f, SkColorSpace* = nullptr);
    static bool setColor(SkBitmap*, int x, int y, SkColor, SkColorSpace* = nullptr);

    static bool setPMColor4f(SkBitmap*, int x, int y, SkPMColor4f, SkColorSpace* = nullptr);
};

#endif // SkBitmapPriv_DEFINED
