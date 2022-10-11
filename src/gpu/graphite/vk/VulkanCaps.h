/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanCaps_DEFINED
#define skgpu_graphite_VulkanCaps_DEFINED

#include "src/gpu/graphite/Caps.h"

namespace skgpu::graphite {
struct ContextOptions;

/**
 * Stores some capabilities of a Vk backend.
 */
class VulkanCaps final : public Caps {
public:
    /**
     * Creates a VkCaps where nothing is supported. The init function should be called to fill out
     * the caps.
     */
    VulkanCaps();
    ~VulkanCaps() override;

    TextureInfo getDefaultSampledTextureInfo(SkColorType,
                                             uint32_t levelCount,
                                             Protected,
                                             Renderable) const override;

    TextureInfo getDefaultMSAATextureInfo(const TextureInfo& singleSampledInfo,
                                          Discardable discardable) const override;

    TextureInfo getDefaultDepthStencilTextureInfo(SkEnumBitMask<DepthStencilFlags>,
                                                  uint32_t sampleCount,
                                                  Protected) const override;

    UniqueKey makeGraphicsPipelineKey(const GraphicsPipelineDesc&,
                                      const RenderPassDesc&) const override { return {}; }
    UniqueKey makeComputePipelineKey(const ComputePipelineDesc&) const override { return {}; }

    bool isRenderable(const TextureInfo&) const override { return false; }

    void buildKeyForTexture(SkISize dimensions,
                            const TextureInfo&,
                            ResourceType,
                            Shareable,
                            GraphiteResourceKey*) const override {}

private:
    const ColorTypeInfo* getColorTypeInfo(SkColorType, const TextureInfo&) const override {
        return nullptr;
    }

    bool onIsTexturable(const TextureInfo&) const override { return false; }

    size_t getTransferBufferAlignment(size_t bytesPerPixel) const override { return 0; }

    bool supportsWritePixels(const TextureInfo&) const override { return false; }
    bool supportsReadPixels(const TextureInfo&) const override { return false; }

    SkColorType supportedWritePixelsColorType(SkColorType dstColorType,
                                              const TextureInfo& dstTextureInfo,
                                              SkColorType srcColorType) const override {
        return kUnknown_SkColorType;
    }
    SkColorType supportedReadPixelsColorType(SkColorType srcColorType,
                                             const TextureInfo& srcTextureInfo,
                                             SkColorType dstColorType) const override {
        return kUnknown_SkColorType;
    }

    bool fMustSyncCommandBuffersWithQueue = false;
    bool fShouldAlwaysUseDedicatedImageMemory = false;

    bool fAvoidUpdateBuffers = false;

    bool fSupportsSwapchain = false;

    bool fSupportsPhysicalDeviceProperties2 = false;
    bool fSupportsMemoryRequirements2 = false;
    bool fSupportsBindMemory2 = false;
    bool fSupportsMaintenance1 = false;
    bool fSupportsMaintenance2 = false;
    bool fSupportsMaintenance3 = false;

    bool fSupportsDedicatedAllocation = false;
    bool fSupportsExternalMemory = false;
    bool fSupportsAndroidHWBExternalMemory = false;

    bool fSupportsYcbcrConversion = false;

    bool fSupportsProtectedMemory = false;

    bool fSupportsDRMFormatModifiers = false;

    bool fPreferPrimaryOverSecondaryCommandBuffers = true;
    bool fMustInvalidatePrimaryCmdBufferStateAfterClearAttachments = false;

    bool fMustUseCoherentHostVisibleMemory = false;
    bool fGpuOnlyBuffersMorePerformant = false;
    bool fShouldPersistentlyMapCpuToGpuBuffers = true;

    /**
     * TODO - Do we still cap the max render tasks at 100 before doing a submission? Meaning, should
     * we still default this to 100?
     */
    int fMaxPerPoolCachedSecondaryCommandBuffers = 100;

    uint32_t fMaxInputAttachmentDescriptors = 0;

    float fMaxSamplerAnisotropy = 1.f;

    bool fMustLoadFullImageWithDiscardableMSAA = false;
    bool fSupportsDiscardableMSAAForDMSAA = true;
    bool fSupportsMemorylessAttachments = false;

    uint32_t fMaxDrawIndirectDrawCount = 0;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanCaps_DEFINED

