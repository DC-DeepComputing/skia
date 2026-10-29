// Copyright 2019 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"

#include <cinttypes>

REG_FIDDLE(Bitmap_getGenerationID, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    SkDebugf("empty id %" PRIu64 "\n", bitmap.getGenerationID());
    bitmap.allocPixels(SkImageInfo::MakeN32(64, 64, kOpaque_SkAlphaType));
    SkDebugf("alloc id %" PRIu64 "\n", bitmap.getGenerationID());
    bitmap.eraseColor(SK_ColorRED);
    SkDebugf("erase id %" PRIu64 "\n", bitmap.getGenerationID());
}
}  // END FIDDLE
