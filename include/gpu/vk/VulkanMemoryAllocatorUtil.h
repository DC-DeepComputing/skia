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

struct GrVkBackendContext;

namespace skgpu {

struct VulkanBackendContext;
class VulkanExtensions;
class VulkanMemoryAllocator;

namespace VulkanMemoryAllocators {
// Returns a concrete implementation of a memory allocator.
SK_API sk_sp<VulkanMemoryAllocator> Make(const skgpu::VulkanBackendContext&, ThreadSafe);
// TODO(b/309785258) Delete when types are aligned.
SK_API sk_sp<VulkanMemoryAllocator> Make(const GrVkBackendContext&, ThreadSafe);

}  // namespace VulkanMemoryAllocators
}  // namespace skgpu

#endif  // skgpu_VulkanMemoryAllocatorUtil_DEFINED
