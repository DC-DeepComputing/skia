/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "gm/vias/Draw.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/private/base/SkAssert.h"

#include <string>

// This draw() implementation draws the GM directly on the surface under test (that is, without
// applying a "via").
GMOutput draw(skiagm::GM* gm, SkSurface* surface, std::string via) {
    if (via != "") {
        SK_ABORT("unknown --via flag value: %s", via.c_str());
    }

    // Run the GM.
    SkString msg;
    skiagm::GM::DrawResult result = gm->draw(surface->getCanvas(), &msg);
    if (result != skiagm::DrawResult::kOk) {
        return {result, msg.c_str()};
    }

    // Extract a bitmap from the surface.
    SkBitmap bitmap;
    bitmap.allocPixelsFlags(surface->getCanvas()->imageInfo(), SkBitmap::kZeroPixels_AllocFlag);
    if (!surface->readPixels(bitmap, 0, 0)) {
        return {skiagm::DrawResult::kFail, "Could not read pixels from surface"};
    }
    return {result, msg.c_str(), bitmap};
}
