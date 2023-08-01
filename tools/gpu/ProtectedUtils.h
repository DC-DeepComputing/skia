/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ProtectedUtils_DEFINED
#define ProtectedUtils_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"

class GrDirectContext;
class SkImage;
class SkSurface;

namespace ProtectedUtils {

bool ContextSupportsProtected(GrDirectContext* dContext);

sk_sp<SkSurface> CreateProtectedSkSurface(GrDirectContext* dContext,
                                          bool textureable,
                                          bool isProtected);

void CheckImageBEProtection(SkImage* image,
                            bool expectingProtected);

sk_sp<SkImage> CreateProtectedSkImage(GrDirectContext* dContext,
                                      SkISize size,
                                      SkColor4f color,
                                      bool isProtected);

}  // namespace ProtectedUtils

#endif  // ProtectedUtils_DEFINED
