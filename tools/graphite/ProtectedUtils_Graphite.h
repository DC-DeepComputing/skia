/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ProtectedUtils_Graphite_DEFINED
#define ProtectedUtils_Graphite_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkRefCnt.h"

class SkImage;
class SkSurface;
struct SkISize;

namespace skgpu {
    enum class Protected : bool;
}
namespace skgpu::graphite {
    class Recorder;
}

namespace ProtectedUtils {

/*
 * These factories create Surfaces and Images with an explicitly specified protected status.
 * If the Surface/Image cannot be created with the specified protected status nullptr will
 * be returned.
 */

sk_sp<SkSurface> CreateProtectedSkSurface(skgpu::graphite::Recorder*,
                                          SkISize size,
                                          skgpu::Protected);

sk_sp<SkImage> CreateProtectedSkImage(skgpu::graphite::Recorder*,
                                      SkISize size,
                                      SkColor4f color,
                                      skgpu::Protected);

}  // namespace ProtectedUtils

#endif  // ProtectedUtils_Graphite_DEFINED
