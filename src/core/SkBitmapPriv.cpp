/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkBitmapPriv.h"

#include "src/core/SkPixmapPriv.h"

bool SkBitmapPriv::setColor4f(SkBitmap* bm, int x, int y, SkColor4f c, SkColorSpace* colorSpace) {
    if (kUnknown_SkColorType == bm->colorType()) {
        return false;
    }

    SkPixmap pm;
    if (!bm->peekPixels(&pm)) {
        return false;
    }

    if (SkPixmapPriv::setColor4f(&pm, x, y, c, colorSpace)) {
        bm->notifyPixelsChanged();
        return true;
    }

    return false;
}

bool SkBitmapPriv::setColor(SkBitmap* bm, int x, int y, SkColor c, SkColorSpace* colorspace) {
    return setColor4f(bm, x, y, SkColor4f::FromColor(c), colorspace);
}

bool SkBitmapPriv::setPMColor4f(SkBitmap* bm, int x, int y, SkPMColor4f pmC,
                                SkColorSpace* colorSpace) {
    if (kUnknown_SkColorType == bm->colorType()) {
        return false;
    }

    SkPixmap pm;
    if (!bm->peekPixels(&pm)) {
        return false;
    }

    if (SkPixmapPriv::setPMColor4f(&pm, x, y, pmC, colorSpace)) {
        bm->notifyPixelsChanged();
        return true;
    }

    return false;
}
