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

class GrDirectContext;
class SkImage;
class SkSurface;
class SkSurfaceProps;
struct SkISize;

namespace ProtectedUtils {

/*
 * These factories create Surfaces and Images with an explicitly specified protected status.
 * If the Surface/Image cannot be created with the specified protected status nullptr will
 * be returned.
 */

sk_sp<SkSurface> CreateProtectedSkSurface(GrDirectContext*,
                                          SkISize size,
                                          bool textureable,
                                          bool isProtected,
                                          const SkSurfaceProps* = nullptr);

void CheckImageBEProtection(SkImage*, bool expectingProtected);

sk_sp<SkImage> CreateProtectedSkImage(GrDirectContext*,
                                      SkISize size,
                                      SkColor4f color,
                                      bool isProtected);

}  // namespace ProtectedUtils

#endif  // ProtectedUtils_DEFINED
