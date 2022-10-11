/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanCaps_DEFINED
#define skgpu_graphite_VulkanCaps_DEFINED

#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/vk/VulkanUtils.h"

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
    enum VkVendor {
        kAMD_VkVendor = 4098,
        kARM_VkVendor = 5045,
        kImagination_VkVendor = 4112,
        kIntel_VkVendor = 32902,
        kNvidia_VkVendor = 4318,
        kQualcomm_VkVendor = 20803,
    };

    enum class IntelGPUType {
        // 11th gen
        kIceLake,

        // 12th gen
        kRocketLake,
        kTigerLake,
        kAlderLake,

        kOther
    };

    static IntelGPUType GetIntelGPUType(uint32_t deviceID);
    static int GetIntelGen(IntelGPUType type) {
        switch (type) {
            case IntelGPUType::kIceLake:
                return 11;
            case IntelGPUType::kRocketLake: // fall through
            case IntelGPUType::kTigerLake:  // fall through
            case IntelGPUType::kAlderLake:
                return 12;
            case IntelGPUType::kOther:
                // For now all our workaround checks are in the form of "if gen > some_value". So
                // we can return 0 for kOther which means we won't put in the new workaround for
                // older gens which is fine. If we stay on top of adding support for new gen
                // intel devices we shouldn't hit cases where we'd need to change this pattern.
                return 0;
        }
        SkUNREACHABLE;
    }

    const ColorTypeInfo* getColorTypeInfo(SkColorType, const TextureInfo&) const override;

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

    struct FormatInfo {
        uint32_t colorTypeFlags(SkColorType colorType) const {
            for (int i = 0; i < fColorTypeInfoCount; ++i) {
                if (fColorTypeInfos[i].fColorType == colorType) {
                    return fColorTypeInfos[i].fFlags;
                }
            }
            return 0;
        }

        void init(const skgpu::VulkanInterface*,
                  VkPhysicalDevice,
                  const VkPhysicalDeviceProperties&,
                  VkFormat);
        static void InitFormatFlags(VkFormatFeatureFlags, uint16_t* flags);
        void initSampleCounts(const skgpu::VulkanInterface*,
                              VkPhysicalDevice,
                              const VkPhysicalDeviceProperties&,
                              VkFormat);

        enum {
            kTexturable_Flag = 0x1,
            kRenderable_Flag = 0x2,
            kBlitSrc_Flag    = 0x4,
            kBlitDst_Flag    = 0x8,
        };

        uint16_t fOptimalFlags = 0;
        uint16_t fLinearFlags = 0;

        SkTDArray<int> fColorSampleCounts;

        std::unique_ptr<ColorTypeInfo[]> fColorTypeInfos;
        int fColorTypeInfoCount = 0;
    };
    static const size_t kNumVkFormats = 22;
    FormatInfo fFormatTable[kNumVkFormats];

    // FormatInfo& getFormatInfo(VkFormat); TODO - Do we need to support this?
    const FormatInfo& getFormatInfo(VkFormat) const;

    VkFormat fColorTypeToFormatTable[kSkColorTypeCnt];
    void setColorType(SkColorType, std::initializer_list<VkFormat> formats);

    VkFormat fPreferredStencilFormat;

    // SkSTArray<1, GrVkYcbcrConversionInfo> fYcbcrInfos;

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

