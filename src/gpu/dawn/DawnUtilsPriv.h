/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_DawnUtilsPriv_DEFINED
#define skgpu_DawnUtilsPriv_DEFINED

#include "include/private/gpu/dawn/SkiaWebGpuCpp.h"

namespace skgpu {

size_t DawnFormatBytesPerBlock(wgpu::TextureFormat format);

uint32_t DawnFormatChannels(wgpu::TextureFormat format);

} // namespace skgpu

#endif // skgpu_DawnUtilsPriv_DEFINED
