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
#include "include/private/base/SkTArray.h"
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/graphite/vk/VulkanSampler.h"

namespace SkSL {
    class Compiler;
}

namespace skgpu::graphite {

class Attribute;
class GraphicsPipelineDesc;
class RuntimeEffectDictionary;
class VulkanResourceProvider;
class VulkanSharedContext;
struct RenderPassDesc;
class TextureInfo;
class VulkanRenderPass;

// This does not own the underlying vulkan objects, they must be explicitly cleaned up by
// calling destroy(sharedContext).
struct VulkanProgramInfo {
    VkShaderModule fVS = VK_NULL_HANDLE;
    VkShaderModule fFS = VK_NULL_HANDLE;
    VkPipelineLayout fLayout = VK_NULL_HANDLE;

    void destroy(const VulkanSharedContext*);
};

class VulkanGraphicsPipeline final : public GraphicsPipeline {
public:
    inline static constexpr unsigned int kRenderStepUniformBufferIndex = 0;
    inline static constexpr unsigned int kPaintUniformBufferIndex = 1;
    inline static constexpr unsigned int kGradientBufferIndex = 2;
    inline static constexpr unsigned int kNumUniformBuffers = 3;

    // For now, rigidly assign all descriptor types to be at statically-defined set indices.
    // TODO(b/274762935): Make the bindings and descriptor set organization more flexible.
    inline static constexpr unsigned int kDstAsInputDescSetIndex = 0;
    inline static constexpr unsigned int kUniformBufferDescSetIndex = 1;
    inline static constexpr unsigned int kTextureBindDescSetIndex = 2;
    inline static constexpr unsigned int kLoadMsaaFromResolveInputDescSetIndex = 3;
    inline static constexpr unsigned int kMaxNumDescSets = 4;

    inline static constexpr unsigned int kVertexBufferIndex = 0;
    inline static constexpr unsigned int kInstanceBufferIndex = 1;
    inline static constexpr unsigned int kNumInputBuffers = 2;

    // Define a static DescriptorData to represent input attachments which have the same values
    // across all pipelines (we currently only ever use one input attachment within a set).
    inline static const DescriptorData kInputAttachmentDescriptor = {
            DescriptorType::kInputAttachment, /*count=*/1,
            /*bindingIdx=*/0, // We only expect to encounter one input attachment
            PipelineStageFlags::kFragmentShader};

    static sk_sp<VulkanGraphicsPipeline> Make(VulkanResourceProvider*,
                                              const RuntimeEffectDictionary*,
                                              const UniqueKey&,
                                              const GraphicsPipelineDesc&,
                                              const RenderPassDesc&,
                                              SkEnumBitMask<PipelineCreationFlags>,
                                              uint32_t compilationID);

    // The struct will either have all non-null handles on success, or all null handles on failure.
    // The created program info can be provided to MakeLoadMSAAPipeline for reuse with different
    // render pass descriptions.
    static VulkanProgramInfo CreateLoadMSAAProgram(const VulkanSharedContext*);

    static sk_sp<VulkanGraphicsPipeline> MakeLoadMSAAPipeline(
            VulkanResourceProvider*,
            const VulkanProgramInfo& loadMSAAProgram,
            const RenderPassDesc&);

    ~VulkanGraphicsPipeline() override {}

    VkPipelineLayout layout() const {
        SkASSERT(fPipelineLayout != VK_NULL_HANDLE);
        return fPipelineLayout;
    }

    VkPipeline pipeline() const {
        SkASSERT(fPipeline != VK_NULL_HANDLE);
        return fPipeline;
    }

private:
    VulkanGraphicsPipeline(const VulkanSharedContext* sharedContext,
                           const PipelineInfo& pipelineInfo,
                           VkPipelineLayout,
                           VkPipeline,
                           bool ownsPipelineLayout,
                           skia_private::TArray<sk_sp<VulkanSampler>>&& immutableSamplers);

    void freeGpuData() override;

    // The fragment shader can be null if no shading is performed by the pipeline.
    // This function does not cleanup any of the VulkanProgramInfo's objects on success or failure.
    static VkPipeline MakePipeline(VulkanResourceProvider*,
                                   const VulkanProgramInfo&,
                                   int subpassIndex,
                                   PrimitiveType,
                                   SkSpan<const Attribute> vertexAttrs,
                                   SkSpan<const Attribute> instanceAttrs,
                                   const DepthStencilSettings&,
                                   const BlendInfo&,
                                   const RenderPassDesc&);

    VkPipelineLayout fPipelineLayout = VK_NULL_HANDLE;
    VkPipeline fPipeline = VK_NULL_HANDLE;
    bool fOwnsPipelineLayout = true;

    // Hold a ref to immutable samplers used such that their lifetime is properly managed.
    const skia_private::TArray<sk_sp<VulkanSampler>> fImmutableSamplers;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlGraphicsPipeline_DEFINED
