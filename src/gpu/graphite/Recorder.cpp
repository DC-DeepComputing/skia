/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Recorder.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/ImageProvider.h"
#include "include/gpu/graphite/Recording.h"

#include "src/core/SkConvertPixels.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/CopyTask.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PathAtlas.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/PipelineDataCache.h"
#include "src/gpu/graphite/ProxyCache.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/TaskGraph.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/UploadBufferManager.h"
#include "src/gpu/graphite/UploadTask.h"
#include "src/gpu/graphite/text/TextAtlasManager.h"
#include "src/image/SkImage_Base.h"
#include "src/text/gpu/StrikeCache.h"
#include "src/text/gpu/TextBlobRedrawCoordinator.h"

 // TODO: Move these includes.
#ifdef SK_BUILD_FOR_ANDROID
#ifdef SK_VULKAN
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "src/gpu/graphite/vk/VulkanCaps.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#endif // SK_VULKAN
#endif // SK_BUILD_FOR_ANDROID

namespace skgpu::graphite {

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(this->singleOwner())
#define ASSERT_SINGLE_OWNER_PRIV SKGPU_ASSERT_SINGLE_OWNER(fRecorder->singleOwner())

/*
 * The default image provider doesn't perform any conversion so, by default, Graphite won't
 * draw any non-Graphite-backed images.
 */
class DefaultImageProvider final : public ImageProvider {
public:
    static sk_sp<DefaultImageProvider> Make() {
        return sk_ref_sp(new DefaultImageProvider);
    }

    sk_sp<SkImage> findOrCreate(Recorder* recorder,
                                const SkImage* image,
                                SkImage::RequiredProperties) override {
        SkASSERT(!as_IB(image)->isGraphiteBacked());

        return nullptr;
    }

private:
    DefaultImageProvider() {}
};

/**************************************************************************************************/
RecorderOptions::RecorderOptions() = default;
RecorderOptions::RecorderOptions(const RecorderOptions&) = default;
RecorderOptions::~RecorderOptions() = default;

/**************************************************************************************************/
static int32_t next_id() {
    static std::atomic<int32_t> nextID{1};
    int32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidGenID);
    return id;
}

Recorder::Recorder(sk_sp<SharedContext> sharedContext, const RecorderOptions& options)
        : fSharedContext(std::move(sharedContext))
        , fRuntimeEffectDict(std::make_unique<RuntimeEffectDictionary>())
        , fGraph(new TaskGraph)
        , fUniformDataCache(new UniformDataCache)
        , fTextureDataCache(new TextureDataCache)
        , fRecorderID(next_id())
        , fAtlasProvider(std::make_unique<AtlasProvider>(this))
        , fTokenTracker(std::make_unique<TokenTracker>())
        , fStrikeCache(std::make_unique<sktext::gpu::StrikeCache>())
        , fTextBlobCache(std::make_unique<sktext::gpu::TextBlobRedrawCoordinator>(fRecorderID)) {
    fClientImageProvider = options.fImageProvider;
    if (!fClientImageProvider) {
        fClientImageProvider = DefaultImageProvider::Make();
    }

    fResourceProvider = fSharedContext->makeResourceProvider(this->singleOwner(),
                                                             fRecorderID,
                                                             options.fGpuBudgetInBytes);
    fDrawBufferManager.reset( new DrawBufferManager(fResourceProvider.get(),
                                                    fSharedContext->caps()));
    fUploadBufferManager.reset(new UploadBufferManager(fResourceProvider.get(),
                                                       fSharedContext->caps()));
    SkASSERT(fResourceProvider);
}

Recorder::~Recorder() {
    ASSERT_SINGLE_OWNER
    // Any finished procs that haven't been passed to a Recording fail
    for (int i = 0; i < fFinishedProcs.size(); ++i) {
        fFinishedProcs[i]->setFailureResult();
    }
    for (auto& device : fTrackedDevices) {
        device->abandonRecorder();
    }
#if defined(GRAPHITE_TEST_UTILS)
    if (fContext) {
        fContext->priv().deregisterRecorder(this);
    }
#endif

    // TODO: needed?
    fStrikeCache->freeAll();
}

BackendApi Recorder::backend() const { return fSharedContext->backend(); }

std::unique_ptr<Recording> Recorder::snap() {
    ASSERT_SINGLE_OWNER
    for (auto& device : fTrackedDevices) {
        device->flushPendingWorkToRecorder();
    }

    std::unordered_set<sk_sp<TextureProxy>, Recording::ProxyHash> nonVolatileLazyProxies;
    std::unordered_set<sk_sp<TextureProxy>, Recording::ProxyHash> volatileLazyProxies;
    fTextureDataCache->foreach([&](const TextureDataBlock* block) {
        for (int j = 0; j < block->numTextures(); ++j) {
            const TextureDataBlock::SampledTexture& tex = block->texture(j);

            if (tex.first->isLazy()) {
                if (tex.first->isVolatile()) {
                    volatileLazyProxies.insert(tex.first);
                } else {
                    nonVolatileLazyProxies.insert(tex.first);
                }
            }
        }
    });

    // TODO: fulfill all promise images in the TextureDataCache here
    // TODO: create all the samplers needed in the TextureDataCache here

    if (!fGraph->prepareResources(fResourceProvider.get(), fRuntimeEffectDict.get())) {
        // Leaving 'fTrackedDevices' alone since they were flushed earlier and could still be
        // attached to extant SkSurfaces.
        fDrawBufferManager.reset(new DrawBufferManager(fResourceProvider.get(),
                                                       fSharedContext->caps()));
        fTextureDataCache = std::make_unique<TextureDataCache>();
        fUniformDataCache = std::make_unique<UniformDataCache>();
        fGraph->reset();
        fRuntimeEffectDict->reset();
        return nullptr;
    }

    std::unique_ptr<Recording::LazyProxyData> targetProxyData;
    if (fTargetProxyData) {
        targetProxyData = std::move(fTargetProxyData);
        fTargetProxyDevice.reset();
        fTargetProxyCanvas.reset();
    }
    std::unique_ptr<Recording> recording(new Recording(std::move(fGraph),
                                                       std::move(nonVolatileLazyProxies),
                                                       std::move(volatileLazyProxies),
                                                       std::move(targetProxyData),
                                                       std::move(fFinishedProcs)));

    fDrawBufferManager->transferToRecording(recording.get());
    fUploadBufferManager->transferToRecording(recording.get());

    fGraph = std::make_unique<TaskGraph>();
    fRuntimeEffectDict->reset();
    fTextureDataCache = std::make_unique<TextureDataCache>();
    fUniformDataCache = std::make_unique<UniformDataCache>();

    // inject an initial task to maintain atlas state for next Recording
    auto uploads = std::make_unique<UploadList>();
    fAtlasProvider->textAtlasManager()->recordUploads(uploads.get(), /*useCachedUploads=*/true);
    if (uploads->size() > 0) {
        sk_sp<Task> uploadTask = UploadTask::Make(uploads.get());
        this->priv().add(std::move(uploadTask));
    }

    return recording;
}

SkCanvas* Recorder::makeDeferredCanvas(const SkImageInfo& imageInfo,
                                       const TextureInfo& textureInfo) {
    if (fTargetProxyCanvas) {
        // Require snapping before requesting another canvas.
        SKGPU_LOG_W("Requested a new deferred canvas before snapping the previous one");
        return nullptr;
    }

    fTargetProxyData = std::make_unique<Recording::LazyProxyData>(textureInfo);
    fTargetProxyDevice = Device::Make(this,
                                      fTargetProxyData->refLazyProxy(),
                                      imageInfo.dimensions(),
                                      imageInfo.colorInfo(),
                                      {},
                                      false);
    fTargetProxyCanvas = std::make_unique<SkCanvas>(fTargetProxyDevice);
    return fTargetProxyCanvas.get();
}

void Recorder::registerDevice(Device* device) {
    ASSERT_SINGLE_OWNER
    fTrackedDevices.push_back(device);
}

void Recorder::deregisterDevice(const Device* device) {
    ASSERT_SINGLE_OWNER
    for (auto it = fTrackedDevices.begin(); it != fTrackedDevices.end(); it++) {
        if (*it == device) {
            fTrackedDevices.erase(it);
            return;
        }
    }
}

BackendTexture Recorder::createBackendTexture(SkISize dimensions, const TextureInfo& info) {
    ASSERT_SINGLE_OWNER

    if (!info.isValid() || info.backend() != this->backend()) {
        return {};
    }
    return fResourceProvider->createBackendTexture(dimensions, info);
}

#ifdef SK_BUILD_FOR_ANDROID
TextureInfo Recorder::getTextureInfoFromAHardwareBuffer(
        AHardwareBuffer* hardwareBuffer, uint32_t bufferFormat, bool requireKnownFormat,
        bool isRenderable) {
    if (fSharedContext->backend() != BackendApi::kVulkan) {
        return {};
    }
#ifdef SK_VULKAN
    auto vkContext = static_cast<VulkanSharedContext*>(fSharedContext);
    VkDevice device = vkContext->device();
    VkFormat format;
    switch (bufferFormat) {
        case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM: {
            format = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        }
        case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT: {
            format = VK_FORMAT_R16G16B16A16_SFLOAT;
            break;
        }
        case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM: {
            format = VK_FORMAT_R5G6B5_UNORM_PACK16;
            break;
        }
        case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM: {
            format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
            break;
        }
        case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM: {
            format = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        }
        case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM: {
            format = VK_FORMAT_R8G8B8_UNORM;
            break;
        }
#if __ANDROID_API__ >= 33
        case AHARDWAREBUFFER_FORMAT_R8_UNORM: {
            format = VK_FORMAT_R8_UNORM;
            break;
        }
#endif
        default: {
            if (requireKnownFormat) {
                return {};
            } else {
                VkAndroidHardwareBufferFormatPropertiesANDROID hwbFormatProps;
                hwbFormatProps.sType =
                        VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
                hwbFormatProps.pNext = nullptr;
                VkAndroidHardwareBufferPropertiesANDROID hwbProps;
                hwbProps.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
                hwbProps.pNext = &hwbFormatProps;

                VkResult result = VULKAN_CALL(GetAndroidHardwareBufferProperties(device,
                                                                                 hardwareBuffer,
                                                                                 &hwbProps));
                if (result != VK_SUCCESS) {
                    return {};
                } else if (hwbFormatProps.format != VK_FORMAT_UNDEFINED) {
                    return {};
                }

                // TODO: Use fetched  hardware properties to determine Ycbcr conversion information/
                // Pass the ycbcr conversion information into VulkanTextureInfo.
                // GrVkYcbcrConversionInfo ycbcrConversion;
                // ycbcrConversion.fYcbcrModel = hwbFormatProps.suggestedYcbcrModel;
                // ycbcrConversion.fYcbcrRange = hwbFormatProps.suggestedYcbcrRange;
                // ycbcrConversion.fXChromaOffset = hwbFormatProps.suggestedXChromaOffset;
                // ycbcrConversion.fYChromaOffset = hwbFormatProps.suggestedYChromaOffset;
                // ycbcrConversion.fForceExplicitReconstruction = VK_FALSE;
                // ycbcrConversion.fExternalFormat = hwbFormatProps.externalFormat;
                // ycbcrConversion.fFormatFeatures = hwbFormatProps.formatFeatures;
                // if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT &
                //     hwbFormatProps.formatFeatures) {
                //     ycbcrConversion.fChromaFilter = VK_FILTER_LINEAR;
                // } else {
                //     ycbcrConversion.fChromaFilter = VK_FILTER_NEAREST;
                // }

                VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
                if (format != VK_FORMAT_UNDEFINED) {
                    usageFlags = usageFlags |
                                VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                    if (isRenderable) {
                        usageFlags = usageFlags | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                    }
                }
                VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL; // TODO: Query for tiling?
                VkImageCreateFlags imgCreateflags =
                        isProtectedContent ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;

                // TODO: Determine VkImageAspectFlags. If the VkImage is a Ycbcr format, the client
                //       can select a plane. For now, use the default VK_IMAGE_ASPECT_COLOR_BIT.
                VulkanTextureInfo vulkanTextureInfo {VK_SAMPLE_COUNT_1_BIT,
                                                     Mipmapped::kNo,
                                                     imgCreateflags,
                                                     format,
                                                     tiling,
                                                     usageFlags,
                                                     VK_SHARING_MODE_EXCLUSIVE,
                                                     VK_IMAGE_ASPECT_COLOR_BIT};
                return {vulkanTextureInfo};
            }
        }
    }
#endif // #ifdef SK_VULKAN
    return {};
}

BackendTexture Recorder::createBackendTexture(AHardwareBuffer*,
                                              int width, int height,
                                              bool isProtectedContent,
                                              const TextureInfo& backendTextureInfo,
                                              bool isRenderable,
                                              bool fromAndroidWindow = false) {
    // TODO: Remove isRenderable bool since we can now simply query TextureInfo?
    if (fSharedContext->backend() != BackendApi::kVulkan) {
        return {};
    }
#ifdef SK_VULKAN
    VulkanTextureInfo vkTexInfo;
    if (!backendTextureInfo.getVulkanTextureInfo(&vkTexInfo)) {
        // TODO: Add message
        return;
    }

    auto vkContext = static_cast<VulkanSharedContext*>(fSharedContext);
    SkASSERT(!isProtectedContent || sharedContext->isProtected() == Protected::kYes);

    VkDevice device = vkContext->device();
    VkFormat format = vkTexInfo.fFormat;// backendTextureInfo.fFormat;

    VkResult result;
    VkAndroidHardwareBufferFormatPropertiesANDROID hwbFormatProps;
    hwbFormatProps.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    hwbFormatProps.pNext = nullptr;

    VkAndroidHardwareBufferPropertiesANDROID hwbProps;
    hwbProps.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    hwbProps.pNext = &hwbFormatProps;

    result = VULKAN_CALL(GetAndroidHardwareBufferProperties(device, hardwareBuffer, &hwbProps));
    if (result != VK_SUCCESS) {
        return {};
    }
    if (hwbFormatProps.format != format) {
        SkDebugf("Queried format not consistent with expected format; got: %d, expected: %d",
                 hwbFormatProps.format,
                 format);
        return {};
    }

    VkExternalFormatANDROID externalFormat;
    externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    externalFormat.pNext = nullptr;
    externalFormat.externalFormat = 0;  // If this is zero it is as if we aren't using this struct.
    // TODO: Add YCBCR support
    // const GrVkYcbcrConversionInfo* ycbcrConversion =
    //         GrBackendFormats::GetVkYcbcrConversionInfo(backendFormat);
    // if (!ycbcrConversion) {
    //     return {};
    // }

    if (hwbFormatProps.format != VK_FORMAT_UNDEFINED) {
        SkASSERT(SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & hwbFormatProps.formatFeatures) &&
                 SkToBool(VK_FORMAT_FEATURE_TRANSFER_SRC_BIT & hwbFormatProps.formatFeatures) &&
                 SkToBool(VK_FORMAT_FEATURE_TRANSFER_DST_BIT & hwbFormatProps.formatFeatures));
        // SkASSERT(!ycbcrConversion->isValid());
    } else {
        // SkASSERT(ycbcrConversion->isValid());
        // We have an external only format
        SkASSERT(SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & hwbFormatProps.formatFeatures));
        SkASSERT(format == VK_FORMAT_UNDEFINED);
        //SkASSERT(hwbFormatProps.externalFormat == ycbcrConversion->fExternalFormat);
        externalFormat.externalFormat = hwbFormatProps.externalFormat;
    }

    const VkExternalMemoryImageCreateInfo externalMemoryImageInfo{
            VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,                 // sType
            &externalFormat,                                                     // pNext
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,  // handleTypes
    };
    VkImageUsageFlags usageFlags = vkTexInfo.fImageUsageFlags;
    // VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    // if (format != VK_FORMAT_UNDEFINED) {
    //     usageFlags = usageFlags |
    //             VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
    //             VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    //     if (isRenderable) {
    //         usageFlags = usageFlags | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    //     }
    // }
    VkImageTiling tiling = vkTexInfo.fImageTiling;//VK_IMAGE_TILING_OPTIMAL; // TODO: Query for tiling?
    VkImageCreateFlags flags = vkTexInfo.fFlags;//isProtectedContent ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;
    VkSharingMode sharingMode = vkTexInfo.fSharingMode;
    const VkImageCreateInfo imageCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,         // sType
        &externalMemoryImageInfo,                    // pNext
        flags,                                       // VkImageCreateFlags
        VK_IMAGE_TYPE_2D,                            // VkImageType
        format,                                      // VkFormat
        { (uint32_t)width, (uint32_t)height, 1 },    // VkExtent3D
        1,                                           // mipLevels
        1,                                           // arrayLayers
        VK_SAMPLE_COUNT_1_BIT,                       // samples
        tiling,                                      // VkImageTiling
        usageFlags,                                  // VkImageUsageFlags
        sharingMode,                                 // VkSharingMode
        0,                                           // queueFamilyCount
        nullptr,                                     // pQueueFamilyIndices
        VK_IMAGE_LAYOUT_UNDEFINED,                   // initialLayout
    };

    VkImage image;
    result = VULKAN_CALL(CreateImage(device, &imageCreateInfo, nullptr, &image));
    if (result != VK_SUCCESS) {
        return {};
    }

    uint32_t typeIndex = 0;
    bool foundHeap = false;
    VkPhysicalDeviceMemoryProperties2 phyDevMemProps =
            vkContext->vulkanCaps()->physicalDeviceMemoryProperties2();
    uint32_t memTypeCnt = phyDevMemProps.memoryProperties.memoryTypeCount;
    for (uint32_t i = 0; i < memTypeCnt && !foundHeap; ++i) {
        if (hwbProps.memoryTypeBits & (1 << i)) {
            const VkPhysicalDeviceMemoryProperties& pdmp = phyDevMemProps.memoryProperties;
            uint32_t supportedFlags = pdmp.memoryTypes[i].propertyFlags &
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            if (supportedFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                typeIndex = i;
                foundHeap = true;
            }
        }
    }

    // Fallback to use any available memory type for AHB
    //
    // For external memory import, compatible memory types are decided by the Vulkan driver since
    // the memory has been allocated externally. There are usually special requirements against
    // external memory. e.g. AHB allocated with CPU R/W often usage bits is only importable for
    // non-device-local heap on some AMD systems.
    if (!foundHeap && hwbProps.memoryTypeBits) {
        typeIndex = ffs(hwbProps.memoryTypeBits) - 1;
        foundHeap = true;
    }

    if (!foundHeap) {
        VULKAN_CALL(DestroyImage(device, image, nullptr));
        return {};
    }

    VkImportAndroidHardwareBufferInfoANDROID hwbImportInfo;
    hwbImportInfo.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
    hwbImportInfo.pNext = nullptr;
    hwbImportInfo.buffer = hardwareBuffer;

    VkMemoryDedicatedAllocateInfo dedicatedAllocInfo;
    dedicatedAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    dedicatedAllocInfo.pNext = &hwbImportInfo;
    dedicatedAllocInfo.image = image;
    dedicatedAllocInfo.buffer = VK_NULL_HANDLE;

    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,      // sType
        &dedicatedAllocInfo,                         // pNext
        hwbProps.allocationSize,                     // allocationSize
        typeIndex,                                   // memoryTypeIndex
    };

    VkDeviceMemory memory;
    result = VULKAN_CALL(AllocateMemory(device, &allocInfo, nullptr, &memory));
    if (VK_SUCCESS != err) {
        VULKAN_CALL(DestroyImage(device, image, nullptr));
        return {};
    }

    VkBindImageMemoryInfo bindImageInfo;
    bindImageInfo.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
    bindImageInfo.pNext = nullptr;
    bindImageInfo.image = image;
    bindImageInfo.memory = memory;
    bindImageInfo.memoryOffset = 0;

    result = VULKAN_CALL(BindImageMemory2(device, 1, &bindImageInfo));
    if (VK_SUCCESS != err) {
        VULKAN_CALL(DestroyImage(device, image, nullptr));
        VULKAN_CALL(FreeMemory(device, memory, nullptr));
        return {};
    }

    skgpu::VulkanAlloc alloc;
    alloc.fMemory = memory;
    alloc.fOffset = 0;
    alloc.fSize = hwbProps.allocationSize;
    alloc.fFlags = 0;

    return {{width, height}, vkTexInfo, VK_IMAGE_LAYOUT_UNDEFINED, VK_QUEUE_FAMILY_EXTERNAL, image};
#endif // SK_VULKAN
    return {};
}
#endif // SK_BUILD_FOR_ANDROID

bool Recorder::updateBackendTexture(const BackendTexture& backendTex,
                                    const SkPixmap srcData[],
                                    int numLevels) {
    ASSERT_SINGLE_OWNER

    if (!backendTex.isValid() || backendTex.backend() != this->backend()) {
        return false;
    }

    if (!srcData || numLevels <= 0) {
        return false;
    }

    // If the texture has MIP levels then we require that the full set is overwritten.
    int numExpectedLevels = 1;
    if (backendTex.info().mipmapped() == Mipmapped::kYes) {
        numExpectedLevels = SkMipmap::ComputeLevelCount(backendTex.dimensions().width(),
                                                        backendTex.dimensions().height()) + 1;
    }
    if (numLevels != numExpectedLevels) {
        return false;
    }

    SkColorType ct = srcData[0].colorType();

    if (!this->priv().caps()->areColorTypeAndTextureInfoCompatible(ct, backendTex.info())) {
        return false;
    }

    sk_sp<Texture> texture = this->priv().resourceProvider()->createWrappedTexture(backendTex);
    if (!texture) {
        return false;
    }

    sk_sp<TextureProxy> proxy = TextureProxy::Wrap(std::move(texture));

    std::vector<MipLevel> mipLevels;
    mipLevels.resize(numLevels);

    for (int i = 0; i < numLevels; ++i) {
        SkASSERT(srcData[i].addr());
        SkASSERT(srcData[i].info().colorInfo() == srcData[0].info().colorInfo());

        mipLevels[i].fPixels = srcData[i].addr();
        mipLevels[i].fRowBytes = srcData[i].rowBytes();
    }

    // Src and dst colorInfo are the same
    const SkColorInfo& colorInfo = srcData[0].info().colorInfo();
    // Add UploadTask to Recorder
    UploadInstance upload = UploadInstance::Make(this,
                                                 std::move(proxy),
                                                 colorInfo, colorInfo,
                                                 mipLevels,
                                                 SkIRect::MakeSize(backendTex.dimensions()),
                                                 std::make_unique<ImageUploadContext>());
    if (!upload.isValid()) {
        SKGPU_LOG_E("Recorder::updateBackendTexture: Could not create UploadInstance");
        return false;
    }
    sk_sp<Task> uploadTask = UploadTask::Make(std::move(upload));

    // Need to flush any pending work in case it depends on this texture
    this->priv().flushTrackedDevices();

    this->priv().add(std::move(uploadTask));

    return true;
}

void Recorder::deleteBackendTexture(BackendTexture& texture) {
    ASSERT_SINGLE_OWNER

    if (!texture.isValid() || texture.backend() != this->backend()) {
        return;
    }
    fResourceProvider->deleteBackendTexture(texture);
}

void Recorder::addFinishInfo(const InsertFinishInfo& info) {
    if (info.fFinishedProc) {
        sk_sp<RefCntedCallback> callback =
                RefCntedCallback::Make(info.fFinishedProc, info.fFinishedContext);
        fFinishedProcs.push_back(std::move(callback));
    }
}

void Recorder::freeGpuResources() {
    ASSERT_SINGLE_OWNER

    // We don't want to free the Uniform/TextureDataCaches or the Draw/UploadBufferManagers since
    // all their resources need to be held on to until a Recording is snapped. And once snapped, all
    // their held resources are released. The StrikeCache and TextBlobCache don't hold onto any Gpu
    // resources.

    // The AtlasProvider gives out refs to TextureProxies so it should be safe to clear its pool
    // in the middle of Recording since those using the previous TextureProxies will have refs on
    // them.
    fAtlasProvider->clearTexturePool();

    fResourceProvider->freeGpuResources();
}

void Recorder::performDeferredCleanup(std::chrono::milliseconds msNotUsed) {
    ASSERT_SINGLE_OWNER

    auto purgeTime = skgpu::StdSteadyClock::now() - msNotUsed;
    fResourceProvider->purgeResourcesNotUsedSince(purgeTime);
}

void RecorderPriv::add(sk_sp<Task> task) {
    ASSERT_SINGLE_OWNER_PRIV
    fRecorder->fGraph->add(std::move(task));
}

void RecorderPriv::flushTrackedDevices() {
    ASSERT_SINGLE_OWNER_PRIV
    for (Device* device : fRecorder->fTrackedDevices) {
        device->flushPendingWorkToRecorder();
    }
}

sk_sp<TextureProxy> RecorderPriv::CreateCachedProxy(Recorder* recorder,
                                                    const SkBitmap& bitmap,
                                                    Mipmapped mipmapped) {
    SkASSERT(!bitmap.isNull());
    return recorder->priv().proxyCache()->findOrCreateCachedProxy(recorder, bitmap, mipmapped);
}

size_t RecorderPriv::getResourceCacheLimit() const {
    return fRecorder->fResourceProvider->getResourceCacheLimit();
}

#if defined(GRAPHITE_TEST_UTILS)
bool RecorderPriv::deviceIsRegistered(Device* device) {
    ASSERT_SINGLE_OWNER_PRIV
    for (auto& currentDevice : fRecorder->fTrackedDevices) {
        if (device == currentDevice) {
            return true;
        }
    }
    return false;
}

// used by the Context that created this Recorder to set a back pointer
void RecorderPriv::setContext(Context* context) {
    fRecorder->fContext = context;
}
#endif


} // namespace skgpu::graphite
