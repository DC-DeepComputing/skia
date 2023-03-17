/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixmapPriv_DEFINED
#define SkPixmapPriv_DEFINED

#include "include/core/SkPixmap.h"

#include "include/private/SkColorData.h"

class SkPixmapPriv {
public:
    static bool setColor4f(SkPixmap*, int x, int y, SkColor4f, const SkColorSpace*);

    static bool setPMColor4f(SkPixmap*, int x, int y, SkPMColor4f, const SkColorSpace*);
};

#endif // SkPixmapPriv_DEFINED
