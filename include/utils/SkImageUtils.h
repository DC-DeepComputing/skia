/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageUtils_DEFINED
#define SkImageUtils_DEFINED

#include "include/core/SkImage.h"

namespace SkImageUtils {

/**
 * Creates a new image matching the specified Image Info, keeping the same backend type as the original image.
 *
 * If any scaling is necessary (i.e. info has different dimensions) then the scaling will be done using the specified sampling options.
 * If the image is lazy generated, then the returned image will be software raster backed.
 *
 * As an optimization, if the image info exactly matches the image's, then the original image is returned.
 *
 * If any part of the image info is not supported for the image's backend, null is returned.
 */
sk_sp<SkImage> ScaleImage(const SkImage* image, const SkImageInfo& info,
                          const SkSamplingOptions& sampling);

} // namespace

#endif

