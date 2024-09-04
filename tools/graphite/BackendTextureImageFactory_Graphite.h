/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/GpuTypes.h"

class SkColorSpace;
class SkImage;
class SkPixmap;
struct SkISize;

namespace skgpu::graphite {
    class Recorder;
}
using Origin = skgpu::Origin; // TODO: Can we migrate Ganesh to use this?

namespace sk_gpu_test {

using Mipmapped = skgpu::Mipmapped;
using Protected = skgpu::Protected;
using Renderable = skgpu::Renderable;

/*
 * Graphite versions of MakeBackendTextureImage
 */
sk_sp<SkImage> MakeBackendTextureImage(skgpu::graphite::Recorder*,
                                       const SkPixmap&,
                                       Mipmapped,
                                       Renderable,
                                       Origin,
                                       Protected = Protected::kNo);

/** Creates a solid color image. */
sk_sp<SkImage> MakeBackendTextureImage(skgpu::graphite::Recorder*,
                                       const SkImageInfo&,
                                       SkColor4f,
                                       Mipmapped,
                                       Renderable,
                                       Origin,
                                       Protected = Protected::kNo);

}  // namespace sk_gpu_test
