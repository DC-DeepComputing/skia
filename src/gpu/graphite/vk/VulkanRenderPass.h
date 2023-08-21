/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanRenderPass_DEFINED
#define skgpu_graphite_VulkanRenderPass_DEFINED

#include "src/gpu/graphite/Resource.h"

#include "src/gpu/graphite/AttachmentTypes.h"

namespace skgpu::graphite {

class VulkanSharedContext;

/**
 * Wrapper around VkRenderPass.
*/
class VulkanRenderPass : public Resource {
public:

    static GraphiteResourceKey GenerateRenderPassKey(const RenderPassDesc&);

    static sk_sp<VulkanRenderPass> Make(const VulkanSharedContext*);

private:
    void freeGpuData() override;

    VulkanRenderPass(const VulkanSharedContext*);

    const VulkanSharedContext* fSharedContext;
    VkRenderPass fRenderPass;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanRenderPass_DEFINED
