/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/vk/SkContextVulkan.h"

#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/gpu/vk/VulkanTypes.h"

namespace skcontext {
class SkContext;
class SkContextOptions;

sk_sp<SkContext> MakeGanesh(const skgpu::VulkanBackendContext& vkContext,
                            const SkContextOptions& options) {
    return nullptr;
}

sk_sp<SkContext> MakeGraphite(const skgpu::VulkanBackendContext& vkContext,
                              const SkContextOptions& options) {
    return nullptr;
}
}  // namespace skcontext
