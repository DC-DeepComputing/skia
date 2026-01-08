/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

#include "include/android/SkImageAndroid.h"
#include "include/core/SkBitmap.h"
#include "src/image/SkImage_Raster.h"

namespace SkImages {

sk_sp<SkImage> RasterFromBitmapNoCopy(const SkBitmap& bm) {
    return SkImage_Raster::MakeFromBitmap(bm, SkCopyPixelsMode::kNever);
}

}  // namespace SkImages

#endif  // defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26
