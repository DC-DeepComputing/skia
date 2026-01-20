/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/android/vk/AndroidVulkanMemoryAllocator.h"

#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/vk/vulkanmemoryallocator/VulkanMemoryAllocatorPriv.h"

#include <optional>

namespace SkiaVMA {
    sk_sp<skgpu::VulkanMemoryAllocator> Make(const skgpu::VulkanBackendContext& ctx) {
        return skgpu::VulkanMemoryAllocators::Make(ctx, skgpu::ThreadSafe::kNo, std::nullopt);
    }
}
