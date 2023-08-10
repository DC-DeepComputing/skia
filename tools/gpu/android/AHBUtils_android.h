/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AHBUtils_DEFINED
#define AHBUtils_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrTypes.h"

class GrDirectContext;
class SkBitmap;
class SkImage;
class SkSurface;

sk_sp<SkSurface> CreateAHBBackedSurface(GrDirectContext* dContext,
                                        int width, int height,
                                        GrSurfaceOrigin surfaceOrigin,
                                        bool isProtected);

sk_sp<SkImage> CreateAHBBackedImage(GrDirectContext* dContext,
                                    const SkBitmap& bitmap,
                                    GrSurfaceOrigin surfaceOrigin,
                                    bool isProtected);

#endif  // AHBUtils_DEFINED
