/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AndroidVulkanMemoryAllocator_DEFINED
#define AndroidVulkanMemoryAllocator_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

namespace skgpu {
struct VulkanBackendContext;
class VulkanMemoryAllocator;
}

namespace SkiaVMA {

// Returns a concrete implementation of a memory allocator using some hard-coded settings.
// If Android Framework ever wants to make their own, they can stop using this.
sk_sp<skgpu::VulkanMemoryAllocator> Make(const skgpu::VulkanBackendContext&);

}  // namespace SkiaVMA

#endif
