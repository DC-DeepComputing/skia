/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanRenderPass.h"

#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

sk_sp<VulkanRenderPass> VulkanRenderPass::Make(const VulkanSharedContext* context) {
    return sk_sp<VulkanRenderPass>(new VulkanRenderPass(context));
}

VulkanRenderPass::VulkanRenderPass(const VulkanSharedContext* context)
        : Resource(context, Ownership::kOwned, skgpu::Budgeted::kYes, sizeof(VkRenderPass)) {
    // TODO: Verify Resource arg correctness
}

} // namespace skgpu::graphite
