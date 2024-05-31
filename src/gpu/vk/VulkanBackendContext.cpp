/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/vk/VulkanBackendContext.h"

#include <functional>
#include "include/gpu/vk/VulkanMemoryAllocator.h"

namespace skgpu {
VulkanBackendContext::VulkanBackendContext() = default;
VulkanBackendContext::~VulkanBackendContext() = default;
VulkanBackendContext::VulkanBackendContext(const VulkanBackendContext& other) = default;
VulkanBackendContext& VulkanBackendContext::operator=(const VulkanBackendContext& other) = default;
}  // namespace skgpu
