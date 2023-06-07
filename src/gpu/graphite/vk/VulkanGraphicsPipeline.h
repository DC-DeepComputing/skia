/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanGraphicsPipeline_DEFINED
#define skgpu_graphite_VulkanGraphicsPipeline_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/GraphicsPipeline.h"

namespace skgpu::graphite {

class VulkanSharedContext;
class Attribute;

class VulkanGraphicsPipeline final : public GraphicsPipeline {
public:
    inline static constexpr unsigned int kIntrinsicUniformBufferIndex = 0;
    inline static constexpr unsigned int kRenderStepUniformBufferIndex = 1;
    inline static constexpr unsigned int kPaintUniformBufferIndex = 2;
    inline static constexpr unsigned int kNumUniformBuffers = 3;

    // For now, rigidly assign all uniform buffer descriptors to be in one descriptor set in binding
    // 0 and all texture/samplers to be in binding 1.
    // TODO(b/274762935): Make the bindings and descriptor set organization more flexible.
    inline static constexpr unsigned int kUniformBufferDescSetIndex = 0;
    inline static constexpr unsigned int kTextureBindDescSetIndex = 1;

    inline static constexpr unsigned int kVertexBufferIndex = 0;
    inline static constexpr unsigned int kInstanceBufferIndex = 1;
    inline static constexpr unsigned int kNumInputBuffers = 2;

    static sk_sp<VulkanGraphicsPipeline> Make(const VulkanSharedContext* sharedContext,
                                              VkShaderModule vertexShader,
                                              SkSpan<const Attribute> vertexAttrs,
                                              SkSpan<const Attribute> instanceAttrs,
                                              VkShaderModule fragShader,
                                              DepthStencilSettings,
                                              PrimitiveType,
                                              int numTexturesAndSamplers,
                                              const BlendInfo&,
                                              bool hasRenderStepUniforms,
                                              uint32_t numSamples);


    ~VulkanGraphicsPipeline() override {}

    VkPipelineLayout layout() const {
        SkASSERT(fPipelineLayout != VK_NULL_HANDLE);
        return fPipelineLayout;
    }

    bool hasStepUniforms() const { return fHasStepUniforms; }
    bool hasFragment() const { return fHasFragment; }

    uint32_t numTextureSamplers() const { return fNumTextureSamplers; }

private:
    VulkanGraphicsPipeline(const skgpu::graphite::SharedContext* sharedContext,
                           VkPipelineLayout,
                           bool hasStepUniforms,
                           bool hasFragmentShader,
                           uint32_t numTextureSamplers);

    void freeGpuData() override;

    VkPipelineLayout  fPipelineLayout = VK_NULL_HANDLE;

    // #ifdef SK_DEBUG

    // #endif

    bool fHasStepUniforms;
    bool fHasFragment;
    uint32_t fNumTextureSamplers;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlGraphicsPipeline_DEFINED
