/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef vulkan_context_helper_DEFINED
#define vulkan_context_helper_DEFINED

#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "src/gpu/vk/VulkanInterface.h"

struct VulkanContext {
    skgpu::VulkanBackendContext fBackendContext;
    skgpu::VulkanExtensions fExtensions;
    sk_sp<const skgpu::VulkanInterface> fInterface;
    void* fLibVulkan = nullptr;

    VulkanContext() = default;
    ~VulkanContext();
};

bool CreateVulkanContext(VulkanContext* ctx);

#endif
