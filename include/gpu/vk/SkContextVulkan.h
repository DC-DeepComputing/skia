/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkContextVulkan_DEFINED
#define SkContextVulkan_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu {
struct VulkanBackendContext;
}
namespace skcontext {
class SkContext;
class SkContextOptions;

// Creates a context wrapping a Ganesh GPU backend with Vulkan
sk_sp<SkContext> MakeGanesh(const skgpu::VulkanBackendContext& vkContext,
                            const SkContextOptions& options);

//  Creates a context wrapping a Graphite GPU backend using Vulkan
sk_sp<SkContext> MakeGraphite(const skgpu::VulkanBackendContext& vkContext,
                              const SkContextOptions& options);
}  // namespace skcontext

#endif  // SkContextVulkan_DEFINED
