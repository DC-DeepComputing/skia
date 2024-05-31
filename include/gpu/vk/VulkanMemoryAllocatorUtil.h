/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanMemoryAllocatorUtil_DEFINED
#define skgpu_VulkanMemoryAllocatorUtil_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkAPI.h"

namespace skgpu {

struct VulkanBackendContext;
class VulkanExtensions;
class VulkanMemoryAllocator;

namespace VulkanMemoryAllocators {
// Returns a concrete implementation of a memory allocator. Note that the
// fVkExtensions must be set on the context or this will fail.
SK_API sk_sp<VulkanMemoryAllocator> Make(const skgpu::VulkanBackendContext&, ThreadSafe);

}  // namespace VulkanMemoryAllocators
}  // namespace skgpu

#endif  // skgpu_VulkanMemoryAllocatorUtil_DEFINED
