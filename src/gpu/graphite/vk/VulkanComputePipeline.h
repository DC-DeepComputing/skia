/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanComputePipeline_DEFINED
#define skgpu_graphite_VulkanComputePipeline_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "include/private/SkTArray.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/DescriptorData.h"

namespace skgpu::graphite {

class ComputePipelineDesc;
class VulkanSharedContext;

class VulkanComputePipeline final : public ComputePipeline {
public:
    static sk_sp<VulkanComputePipeline> Make(const VulkanSharedContext*,
                                             VkPipeline,
                                             VkPipelineLayout,
                                             skia_private::TArray<DescriptorData> descriptorData);
    static sk_sp<VulkanComputePipeline> Make(const VulkanSharedContext*,
                                             const ComputePipelineDesc&);

    ~VulkanComputePipeline() override = default;

    VkPipeline vkPipeline() const { return fPipeline; }
    VkPipelineLayout vkPipelineLayout() const { return fPipelineLayout; }
    SkSpan<DescriptorData> descriptorData() const {
        return SkSpan<DescriptorData>(const_cast<DescriptorData*>(fDescriptorData.data()),
                                      fDescriptorData.size());
    }

private:
    VulkanComputePipeline(const SharedContext*,
                          VkPipeline,
                          VkPipelineLayout,
                          skia_private::TArray<DescriptorData> descriptorData);

    void freeGpuData() override;

    VkPipeline fPipeline;
    VkPipelineLayout fPipelineLayout;
    skia_private::TArray<DescriptorData> fDescriptorData;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_VulkanComputePipeline_DEFINED
