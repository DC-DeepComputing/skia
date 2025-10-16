/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanTypes_DEFINED
#define skgpu_VulkanTypes_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/vk/SkiaVulkan.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#ifndef VK_VERSION_1_1
#error Skia requires the use of Vulkan 1.1 headers
#endif

namespace skgpu {

namespace graphite {
class VulkanYcbcrConversion;
}

using VulkanGetProc = std::function<PFN_vkVoidFunction(
        const char*, // function name
        VkInstance,  // instance or VK_NULL_HANDLE
        VkDevice     // device or VK_NULL_HANDLE
        )>;

typedef intptr_t VulkanBackendMemory;

/**
 * Types for interacting with Vulkan resources created externally to Skia.
 */
struct VulkanAlloc {
    // can be VK_NULL_HANDLE iff is an RT and is borrowed
    VkDeviceMemory      fMemory = VK_NULL_HANDLE;
    VkDeviceSize        fOffset = 0;
    VkDeviceSize        fSize = 0;  // this can be indeterminate iff Tex uses borrow semantics
    uint32_t            fFlags = 0;
    // handle to memory allocated via skgpu::VulkanMemoryAllocator.
    VulkanBackendMemory fBackendMemory = 0;

    enum Flag {
        kNoncoherent_Flag     = 0x1,   // memory must be flushed to device after mapping
        kMappable_Flag        = 0x2,   // memory is able to be mapped.
        kLazilyAllocated_Flag = 0x4,   // memory was created with lazy allocation
    };

    bool operator==(const VulkanAlloc& that) const {
        return fMemory == that.fMemory && fOffset == that.fOffset && fSize == that.fSize &&
               fFlags == that.fFlags && fUsesSystemHeap == that.fUsesSystemHeap;
    }

private:
    bool fUsesSystemHeap = false;
};

// Used to pass in the necessary information to create a VkSamplerYcbcrConversion object for an
// VkExternalFormatANDROID.
struct VulkanYcbcrConversionInfo {
public:
    // Makes an invalid VulkanYcbcrConversionInfo
    VulkanYcbcrConversionInfo() = default;

    // For external images format features here should be those returned by a call to
    // vkAndroidHardwareBufferFormatPropertiesANDROID
    VulkanYcbcrConversionInfo(uint64_t externalFormat,
                              VkSamplerYcbcrModelConversion ycbcrModel,
                              VkSamplerYcbcrRange ycbcrRange,
                              VkChromaLocation xChromaOffset,
                              VkChromaLocation yChromaOffset,
                              VkFilter chromaFilter,
                              VkBool32 forceExplicitReconstruction,
                              VkComponentMapping components,
                              VkFormatFeatureFlags formatFeatures)
            : VulkanYcbcrConversionInfo(VK_FORMAT_UNDEFINED,
                                        externalFormat,
                                        ycbcrModel,
                                        ycbcrRange,
                                        xChromaOffset,
                                        yChromaOffset,
                                        chromaFilter,
                                        forceExplicitReconstruction,
                                        components,
                                        formatFeatures) {}

    VulkanYcbcrConversionInfo(VkFormat format,
                              VkSamplerYcbcrModelConversion ycbcrModel,
                              VkSamplerYcbcrRange ycbcrRange,
                              VkChromaLocation xChromaOffset,
                              VkChromaLocation yChromaOffset,
                              VkFilter chromaFilter,
                              VkBool32 forceExplicitReconstruction,
                              VkComponentMapping components,
                              VkFormatFeatureFlags formatFeatures)
            : VulkanYcbcrConversionInfo(format,
                                        0,
                                        ycbcrModel,
                                        ycbcrRange,
                                        xChromaOffset,
                                        yChromaOffset,
                                        chromaFilter,
                                        forceExplicitReconstruction,
                                        components,
                                        formatFeatures) {}

    bool operator==(const VulkanYcbcrConversionInfo& that) const {
        // Invalid objects are not required to have all other fields initialized or matching.
        if (!this->isValid() && !that.isValid()) {
            return true;
        }

        // Note that we do not need to check for fSamplerFilterMustMatchChromaFilter equality. This
        // is because the Vulkan spec dictates that Android hardware buffers with the same external
        // format must have the same support for key features. See
        // https://docs.vulkan.org/spec/latest/chapters/memory.html#_android_hardware_buffer_external_memory
        // for more details.
        return this->fFormat                      == that.fFormat                      &&
               this->fExternalFormat              == that.fExternalFormat              &&
               this->fYcbcrModel                  == that.fYcbcrModel                  &&
               this->fYcbcrRange                  == that.fYcbcrRange                  &&
               this->fXChromaOffset               == that.fXChromaOffset               &&
               this->fYChromaOffset               == that.fYChromaOffset               &&
               this->fChromaFilter                == that.fChromaFilter                &&
               this->fForceExplicitReconstruction == that.fForceExplicitReconstruction &&
               this->fComponents.r                == that.fComponents.r                &&
               this->fComponents.g                == that.fComponents.g                &&
               this->fComponents.b                == that.fComponents.b                &&
               this->fComponents.a                == that.fComponents.a;
    }
    bool operator!=(const VulkanYcbcrConversionInfo& that) const { return !(*this == that); }

    bool isValid() const {
        return fYcbcrModel != VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY ||
               this->hasExternalFormat();
    }

    VkFormat format() const { return fFormat; }

    bool hasExternalFormat() const { return fExternalFormat != 0; }
    bool externalFormat() const { return fExternalFormat; }

    VkSamplerYcbcrModelConversion model() const { return fYcbcrModel; }
    VkSamplerYcbcrRange range() const { return fYcbcrRange; }
    VkChromaLocation xChromaOffset() const { return fXChromaOffset; }
    VkChromaLocation yChromaOffset() const { return fYChromaOffset; }
    VkFilter chromaFilter() const { return fChromaFilter; }
    VkBool32 forceExplicitReconstruction() const { return fForceExplicitReconstruction; }
    VkComponentMapping components() const { return fComponents; }

    bool supportsLinearFilter() const { return fSupportsLinearFilter; }

    // Returns a populated VkSamplerYcbcrConversionCreateInfo object based on
    // VulkanYcbcrConversionInfo
    void toVkSamplerYcbcrConversionCreateInfo(
            VkSamplerYcbcrConversionCreateInfo* outInfo,
            std::optional<VkFilter>* requiredSamplerFilter) const {
        outInfo->sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
        outInfo->pNext = nullptr;
        outInfo->format = fFormat;
        outInfo->ycbcrModel = fYcbcrModel;
        outInfo->ycbcrRange = fYcbcrRange;
        outInfo->components = fComponents;
        outInfo->xChromaOffset = fXChromaOffset;
        outInfo->yChromaOffset = fYChromaOffset;
        outInfo->chromaFilter = fChromaFilter;
        outInfo->forceExplicitReconstruction = fForceExplicitReconstruction;

        // TODO: Remove this first if block for format features once clients are updated to use the
        // ctor and not setting fFormatFeature
        if (fFormatFeatures != VK_FORMAT_FEATURE_FLAG_BITS_MAX_ENUM) {
            VkFilter chromaFilter = fChromaFilter;
            if (!(fFormatFeatures &
                  VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT)) {
                if (!(fFormatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
                    // Because we don't have have separate reconstruction filter, the min, mag and
                    // chroma filter must all match. However, we also don't support linear sampling so
                    // the min/mag filter have to be nearest. Therefore, we force the chroma filter to
                    // be nearest regardless of support for the feature
                    // VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT.
                    chromaFilter = VK_FILTER_NEAREST;
                }
                // Let the caller know that it must match min and mag filters with the chroma filter.
                *requiredSamplerFilter = chromaFilter;
            } else {
                if (!(formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
                    *requiredSamplerFilter = VK_FILTER_NEAREST;
                }
            }
        } else if (fSamplerFilterMustMatchChromaFilter) {
            *requiredSamplerFilter = fChromaFilter;
        } else if (!fSupportsLinearFilter) {
            *requiredSamplerFilter = VK_FILTER_NEAREST;
        }
    }
private:
    VulkanYcbcrConversionInfo(VkFormat format,
                              uint64_t externalFormat,
                              VkSamplerYcbcrModelConversion ycbcrModel,
                              VkSamplerYcbcrRange ycbcrRange,
                              VkChromaLocation xChromaOffset,
                              VkChromaLocation yChromaOffset,
                              VkFilter chromaFilter,
                              VkBool32 forceExplicitReconstruction,
                              VkComponentMapping components,
                              VkFormatFeatureFlags formatFeatures)
            : fFormat(format)
            , fExternalFormat(externalFormat)
            , fYcbcrModel(ycbcrModel)
            , fYcbcrRange(ycbcrRange)
            , fXChromaOffset(xChromaOffset)
            , fYChromaOffset(yChromaOffset)
            , fChromaFilter(chromaFilter)
            , fForceExplicitReconstruction(forceExplicitReconstruction)
            , fComponents(components) {
#ifdef SK_DEBUG
        // Format feature flags are only representative of an external format's capabilities, so skip
        // these checks in the case of using a known format or if the featureFlags were clearly not
        // filled in.
        if (fFormat == VK_FORMAT_UNDEFINED && formatFeatures) {
            if (fXChromaOffset == VK_CHROMA_LOCATION_MIDPOINT ||
                fYChromaOffset == VK_CHROMA_LOCATION_MIDPOINT) {
                SkASSERT(formatFeatures & VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT);
            }
            if (fXChromaOffset == VK_CHROMA_LOCATION_COSITED_EVEN ||
                fYChromaOffset == VK_CHROMA_LOCATION_COSITED_EVEN) {
                SkASSERT(formatFeatures & VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT);
            }
            if (fChromaFilter == VK_FILTER_LINEAR) {
                SkASSERT(formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT);
            }
            if (fForceExplicitReconstruction) {
                SkASSERT(formatFeatures &
                        VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT);
            }
        }
#endif

        if (!(formatFeatures &
              VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT)) {
            if (!(formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
                // Because we don't have have separate reconstruction filter, the min, mag and
                // chroma filter must all match. However, we also don't support linear sampling so
                // the min/mag filter have to be nearest. Therefore, we force the chroma filter to
                // be nearest regardless of support for the feature
                // VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT.
                fChromaFilter = VK_FILTER_NEAREST;
            }
            fSamplerFilterMustMatchChromaFilter = true;
        }
        fSupportsLinearFilter = SkToBool(formatFeatures &
                                         VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);
    }


    // For use internally to recreate from an ImmutableSamplerInfo. We don't have feature flags
    // and assume we've updated all the fields as required by the feature flags previously.
    friend class graphite::VulkanYcbcrConversion;

    VulkanYcbcrConversionInfo(VkFormat format,
                              uint64_t externalFormat,
                              VkSamplerYcbcrModelConversion ycbcrModel,
                              VkSamplerYcbcrRange ycbcrRange,
                              VkChromaLocation xChromaOffset,
                              VkChromaLocation yChromaOffset,
                              VkFilter chromaFilter,
                              VkBool32 forceExplicitReconstruction,
                              VkComponentMapping components,
                              bool mustMatchChromaFilter,
                              bool supportsLinearFilter)
            : fFormat(format)
            , fExternalFormat(externalFormat)
            , fYcbcrModel(ycbcrModel)
            , fYcbcrRange(ycbcrRange)
            , fXChromaOffset(xChromaOffset)
            , fYChromaOffset(yChromaOffset)
            , fChromaFilter(chromaFilter)
            , fForceExplicitReconstruction(forceExplicitReconstruction)
            , fComponents(components)
            , fSamplerFilterMustMatchChromaFilter(mustMatchChromaFilter)
            , fSupportsLinearFilter(supportsLinearFilter) {}

// Temporarily make these fields public until we can move clients to using ctor
public:
    // Format of the source image. Must be set to VK_FORMAT_UNDEFINED for external images or
    // a valid image format otherwise.
    VkFormat fFormat = VK_FORMAT_UNDEFINED;

    // The external format. Must be non-zero for external images, zero otherwise.
    // Should be compatible to be used in a VkExternalFormatANDROID struct.
    uint64_t fExternalFormat = 0;

    VkSamplerYcbcrModelConversion fYcbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    VkSamplerYcbcrRange fYcbcrRange           = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    VkChromaLocation fXChromaOffset           = VK_CHROMA_LOCATION_COSITED_EVEN;
    VkChromaLocation fYChromaOffset           = VK_CHROMA_LOCATION_COSITED_EVEN;
    VkFilter fChromaFilter                    = VK_FILTER_NEAREST;
    VkBool32 fForceExplicitReconstruction     = false;

    // This is ignored when fExternalFormat is non-zero.
    VkComponentMapping fComponents            = {VK_COMPONENT_SWIZZLE_IDENTITY,
                                                 VK_COMPONENT_SWIZZLE_IDENTITY,
                                                 VK_COMPONENT_SWIZZLE_IDENTITY,
                                                 VK_COMPONENT_SWIZZLE_IDENTITY};

    // TODO: Remove once clients are updated. This field isn't actually used but clients who are
    // not using the ctor are still manually setting it.
    VkFormatFeatureFlags fFormatFeatures = VK_FORMAT_FEATURE_FLAG_BITS_MAX_ENUM;

private;
    bool fSamplerFilterMustMatchChromaFilter = false;
    bool fSupportsLinearFilter = false;
};

typedef void* VulkanDeviceLostContext;
typedef void (*VulkanDeviceLostProc)(VulkanDeviceLostContext faultContext,
                                     const std::string& description,
                                     const std::vector<VkDeviceFaultAddressInfoEXT>& addressInfos,
                                     const std::vector<VkDeviceFaultVendorInfoEXT>& vendorInfos,
                                     const std::vector<std::byte>& vendorBinaryData);

} // namespace skgpu

#endif // skgpu_VulkanTypes_DEFINED
