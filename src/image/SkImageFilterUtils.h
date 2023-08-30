/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ImageFilterUtils_DEFINED
#define skgpu_ImageFilterUtils_DEFINED

#include "include/core/SkRefCnt.h"

#include <functional>

class SkImage;
class SkImageFilter;
struct SkIPoint;
struct SkIRect;

namespace skif {
struct Functors;

sk_sp<SkImage> MakeWithFilter_Shared(
        const skif::Functors& functors,
        sk_sp<SkImage> src,
        const SkImageFilter* filter,
        const SkIRect& subset,
        const SkIRect& clipBounds,
        SkIRect* outSubset,
        SkIPoint* offset);

} // namespace skif

#endif // skgpu_ImageFilterUtils_DEFINED
