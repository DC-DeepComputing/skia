/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanMemoryAllocatorUtil_DEFINED
#define skgpu_VulkanMemoryAllocatorUtil_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

class VulkanMemoryAllocator;

namespace skgpu {

enum class ThreadSafe : bool {
    kNo = false,
    kYes = true,
};

struct VulkanBackendContext;

}

namespace skgpu::VulkanMemoryAllocators {
// Returns a concrete implementation of a memory allocator.
SK_API sk_sp<VulkanMemoryAllocator> Make(const skgpu::VulkanBackendContext&,
									     const skgpu::VulkanExtensions* extOverride,
                                         ThreadSafe);

} // namespace skgpu::VulkanMemoryAllocators

#endif // skgpu_VulkanMemoryAllocatorUtil_DEFINED
