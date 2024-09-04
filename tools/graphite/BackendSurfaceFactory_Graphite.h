/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef BackendSurfaceFactory_Graphite_DEFINED
#define BackendSurfaceFactory_Graphite_DEFINED

#include "include/core/SkColorSpace.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/gpu/GpuTypes.h"
#include "include/private/SkColorData.h"

class SkSurface;
class SkSurfaceProps;
enum SkColorType : int;
struct SkImageInfo;

namespace skgpu::graphite {
    class Recorder;
}

namespace sk_gpu_test {

/*
 * Graphite version of MakeBackendTextureSurface
 */
sk_sp<SkSurface> MakeBackendTextureSurface(skgpu::graphite::Recorder*,
                                           const SkImageInfo&,
                                           skgpu::Mipmapped = skgpu::Mipmapped::kNo,
                                           skgpu::Protected = skgpu::Protected::kNo,
                                           const SkSurfaceProps* = nullptr);
/*
 * Variation that wraps a WGPUTextureView. Only supported on Dawn backend.
 */
sk_sp<SkSurface> MakeBackendTextureViewSurface(skgpu::graphite::Recorder*,
                                               const SkImageInfo&,
                                               skgpu::Mipmapped = skgpu::Mipmapped::kNo,
                                               skgpu::Protected = skgpu::Protected::kNo,
                                               const SkSurfaceProps* = nullptr);

}  // namespace sk_gpu_test

#endif // BackendSurfaceFactory_Graphite_DEFINED
