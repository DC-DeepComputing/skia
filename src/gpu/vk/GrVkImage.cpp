/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkImage.h"

#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImageView.h"
#include "src/gpu/vk/GrVkMemory.h"
#include "src/gpu/vk/GrVkTexture.h"
#include "src/gpu/vk/GrVkUtil.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

sk_sp<GrVkImage> GrVkImage::MakeStencil(GrVkGpu* gpu,
                                        SkISize dimensions,
                                        int sampleCnt,
                                        VkFormat format) {
    VkImageUsageFlags vkUsageFlags =
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    return GrVkImage::Make(gpu,
                           dimensions,
                           UsageFlags::kStencilAttachment,
                           sampleCnt,
                           format,
                           /*mipLevels=*/1,
                           vkUsageFlags,
                           GrProtected::kNo,
                           SkBudgeted::kYes);
}

sk_sp<GrVkImage> GrVkImage::MakeMSAA(GrVkGpu* gpu,
                                     SkISize dimensions,
                                     int numSamples,
                                     VkFormat format,
                                     GrProtected isProtected) {
    SkASSERT(numSamples > 1);

    VkImageUsageFlags vkUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    return GrVkImage::Make(gpu,
                           dimensions,
                           UsageFlags::kColorAttachment,
                           numSamples,
                           format,
                           /*mipLevels=*/1,
                           vkUsageFlags,
                           isProtected,
                           SkBudgeted::kYes);
}

sk_sp<GrVkImage> GrVkImage::MakeTexture(GrVkGpu* gpu,
                                        SkISize dimensions,
                                        VkFormat format,
                                        uint32_t mipLevels,
                                        GrRenderable renderable,
                                        int numSamples,
                                        SkBudgeted budgeted,
                                        GrProtected isProtected) {
    UsageFlags usageFlags = UsageFlags::kTexture;
    VkImageUsageFlags vkUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (renderable == GrRenderable::kYes) {
        usageFlags |= UsageFlags::kColorAttachment;
        vkUsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        // We always make our render targets support being used as input attachments
        vkUsageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }

    return GrVkImage::Make(gpu,
                           dimensions,
                           usageFlags,
                           numSamples,
                           format,
                           mipLevels,
                           vkUsageFlags,
                           isProtected,
                           budgeted);
}

static bool make_views(GrVkGpu* gpu,
                       VkImage image,
                       const GrVkSurfaceInfo& info,
                       GrAttachment::UsageFlags attachmentUsages,
                       sk_sp<const GrVkImageView>* framebufferView,
                       sk_sp<const GrVkImageView>* textureView) {
    GrVkImageView::Type viewType;
    if (attachmentUsages & GrAttachment::UsageFlags::kStencilAttachment) {
        // If we have stencil usage then we shouldn't have any other usages
        SkASSERT(attachmentUsages == GrAttachment::UsageFlags::kStencilAttachment);
        viewType = GrVkImageView::kStencil_Type;
    } else {
        viewType = GrVkImageView::kColor_Type;
    }

    if (SkToBool(attachmentUsages & GrAttachment::UsageFlags::kStencilAttachment) ||
        SkToBool(attachmentUsages & GrAttachment::UsageFlags::kColorAttachment)) {
        // Attachments can only have a mip level of 1
        *framebufferView = GrVkImageView::Make(
                gpu, image, info.fFormat, viewType, 1, info.fYcbcrConversionInfo);
        if (!*framebufferView) {
            return false;
        }
    }

    if (attachmentUsages & GrAttachment::UsageFlags::kTexture) {
        *textureView = GrVkImageView::Make(gpu,
                                           image,
                                           info.fFormat,
                                           viewType,
                                           info.fLevelCount,
                                           info.fYcbcrConversionInfo);
        if (!*textureView) {
            return false;
        }
    }
    return true;
}

sk_sp<GrVkImage> GrVkImage::Make(GrVkGpu* gpu,
                                 SkISize dimensions,
                                 UsageFlags attachmentUsages,
                                 int sampleCnt,
                                 VkFormat format,
                                 uint32_t mipLevels,
                                 VkImageUsageFlags vkUsageFlags,
                                 GrProtected isProtected,
                                 SkBudgeted budgeted) {
    GrVkSurfaceInfo vkInfo;
    vkInfo.fSampleCount = sampleCnt;
    vkInfo.fLevelCount = mipLevels;
    vkInfo.fProtected = isProtected;
    vkInfo.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    vkInfo.fFormat = format;
    vkInfo.fImageUsageFlags = vkUsageFlags;
    vkInfo.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    ImageObjects imageObjs;
    if (!GrVkImage::CreateImage(gpu, dimensions, vkInfo, &imageObjs)) {
        return nullptr;
    }

    sk_sp<const GrVkImageView> framebufferView;
    sk_sp<const GrVkImageView> textureView;
    if (!make_views(gpu,
                    imageObjs.fImage,
                    vkInfo,
                    attachmentUsages,
                    &framebufferView,
                    &textureView)) {
        GrVkImage::DestroyImage(gpu, imageObjs.fImage, imageObjs.fAlloc);
        return nullptr;
    }

    sk_sp<GrBackendSurfaceMutableStateImpl> mutableState(new GrBackendSurfaceMutableStateImpl(
            imageObjs.fImageLayout, imageObjs.fCurrentQueueFamily));
    return sk_sp<GrVkImage>(new GrVkImage(gpu,
                                          dimensions,
                                          GrSurfaceInfo(vkInfo),
                                          attachmentUsages,
                                          imageObjs.fImage,
                                          imageObjs.fAlloc,
                                          std::move(mutableState),
                                          std::move(framebufferView),
                                          std::move(textureView),
                                          budgeted));
}

sk_sp<GrVkImage> GrVkImage::MakeWrapped(GrVkGpu* gpu,
                                        SkISize dimensions,
                                        const GrVkImageInfo& info,
                                        sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                                        UsageFlags attachmentUsages,
                                        GrWrapOwnership ownership,
                                        GrWrapCacheable cacheable,
                                        bool forSecondaryCB) {

    GrVkSurfaceInfo vkInfo;
    vkInfo.fSampleCount = info.fSampleCount;
    vkInfo.fLevelCount = info.fLevelCount;
    vkInfo.fProtected = info.fProtected;
    vkInfo.fImageTiling = info.fImageTiling;
    vkInfo.fFormat = info.fFormat;
    vkInfo.fImageUsageFlags = info.fImageUsageFlags;
    vkInfo.fSharingMode = info.fSharingMode;

    sk_sp<const GrVkImageView> framebufferView;
    sk_sp<const GrVkImageView> textureView;
    if (!forSecondaryCB) {
        if (!make_views(gpu,
                        info.fImage,
                        vkInfo,
                        attachmentUsages,
                        &framebufferView,
                        &textureView)) {
            return nullptr;
        }
    }

    GrBackendObjectOwnership backendOwnership = kBorrow_GrWrapOwnership == ownership
                                                        ? GrBackendObjectOwnership::kBorrowed
                                                        : GrBackendObjectOwnership::kOwned;

    return sk_sp<GrVkImage>(new GrVkImage(gpu,
                                          dimensions,
                                          GrSurfaceInfo(vkInfo),
                                          attachmentUsages,
                                          info.fImage,
                                          info.fAlloc,
                                          std::move(mutableState),
                                          std::move(framebufferView),
                                          std::move(textureView),
                                          backendOwnership,
                                          cacheable,
                                          forSecondaryCB));
}

GrVkImage::GrVkImage(GrVkGpu* gpu,
                     SkISize dimensions,
                     const GrSurfaceInfo& surfaceInfo,
                     UsageFlags supportedUsages,
                     VkImage image,
                     const GrVkAlloc& alloc,
                     sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                     sk_sp<const GrVkImageView> framebufferView,
                     sk_sp<const GrVkImageView> textureView,
                     SkBudgeted budgeted)
        : GrAttachment(gpu, dimensions, surfaceInfo, supportedUsages)
        , fInitialQueueFamily(mutableState->getVkSharedImageInfo().getQueueFamilyIndex())
        , fMutableState(std::move(mutableState))
        , fFramebufferView(std::move(framebufferView))
        , fTextureView(std::move(textureView))
        , fIsBorrowed(false) {
    this->init(gpu, image, alloc, false);
    this->registerWithCache(budgeted);
}

GrVkImage::GrVkImage(GrVkGpu* gpu,
                     SkISize dimensions,
                     const GrSurfaceInfo& surfaceInfo,
                     UsageFlags supportedUsages,
                     VkImage image,
                     const GrVkAlloc& alloc,
                     sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                     sk_sp<const GrVkImageView> framebufferView,
                     sk_sp<const GrVkImageView> textureView,
                     GrBackendObjectOwnership ownership,
                     GrWrapCacheable cacheable,
                     bool forSecondaryCB)
        : GrAttachment(gpu, dimensions, surfaceInfo, supportedUsages)
        , fInitialQueueFamily(mutableState->getVkSharedImageInfo().getQueueFamilyIndex())
        , fMutableState(std::move(mutableState))
        , fFramebufferView(std::move(framebufferView))
        , fTextureView(std::move(textureView))
        , fIsBorrowed(GrBackendObjectOwnership::kBorrowed == ownership) {
    this->init(gpu, image, alloc, forSecondaryCB);
    this->registerWithCacheWrapped(cacheable);
}

void GrVkImage::init(GrVkGpu* gpu, VkImage image, const GrVkAlloc& alloc, bool forSecondaryCB) {
#ifdef SK_DEBUG
    if (this->spec().fImageUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        SkASSERT(SkToBool(this->spec().fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT));
    } else {
        SkASSERT(SkToBool(this->spec().fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) &&
                 SkToBool(this->spec().fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT));
    }
    // We can't transfer from the non graphics queue to the graphics queue since we can't
    // release the image from the original queue without having that queue. This limits us in terms
    // of the types of queue indices we can handle.
    uint32_t currentQueueFamilyIndex = this->currentQueueFamilyIndex();
    if (currentQueueFamilyIndex != VK_QUEUE_FAMILY_IGNORED &&
        currentQueueFamilyIndex != VK_QUEUE_FAMILY_EXTERNAL &&
        currentQueueFamilyIndex != VK_QUEUE_FAMILY_FOREIGN_EXT) {
        if (this->spec().fSharingMode == VK_SHARING_MODE_EXCLUSIVE) {
            if (currentQueueFamilyIndex != gpu->queueIndex()) {
                SkASSERT(false);
            }
        } else {
            SkASSERT(false);
        }
    }
#endif
    if (forSecondaryCB) {
        fResource = nullptr;
    } else if (fIsBorrowed) {
        fResource = new BorrowedResource(gpu, image, alloc, this->spec().fImageTiling);
    } else {
        SkASSERT(VK_NULL_HANDLE != alloc.fMemory);
        fResource = new Resource(gpu, image, alloc, this->spec().fImageTiling);
    }
}

VkPipelineStageFlags GrVkImage::LayoutToPipelineSrcStageFlags(const VkImageLayout layout) {
    if (VK_IMAGE_LAYOUT_GENERAL == layout) {
        return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    } else if (VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == layout ||
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == layout) {
        return VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL == layout) {
        return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    } else if (VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL == layout ||
               VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL == layout) {
        return VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    } else if (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == layout) {
        return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (VK_IMAGE_LAYOUT_PREINITIALIZED == layout) {
        return VK_PIPELINE_STAGE_HOST_BIT;
    } else if (VK_IMAGE_LAYOUT_PRESENT_SRC_KHR == layout) {
        return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }

    SkASSERT(VK_IMAGE_LAYOUT_UNDEFINED == layout);
    return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
}

VkAccessFlags GrVkImage::LayoutToSrcAccessMask(const VkImageLayout layout) {
    // Currently we assume we will never being doing any explict shader writes (this doesn't include
    // color attachment or depth/stencil writes). So we will ignore the
    // VK_MEMORY_OUTPUT_SHADER_WRITE_BIT.

    // We can only directly access the host memory if we are in preinitialized or general layout,
    // and the image is linear.
    // TODO: Add check for linear here so we are not always adding host to general, and we should
    //       only be in preinitialized if we are linear
    VkAccessFlags flags = 0;
    if (VK_IMAGE_LAYOUT_GENERAL == layout) {
        flags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                VK_ACCESS_TRANSFER_WRITE_BIT |
                VK_ACCESS_HOST_WRITE_BIT;
    } else if (VK_IMAGE_LAYOUT_PREINITIALIZED == layout) {
        flags = VK_ACCESS_HOST_WRITE_BIT;
    } else if (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL == layout) {
        flags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    } else if (VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL == layout) {
        flags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    } else if (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == layout) {
        flags = VK_ACCESS_TRANSFER_WRITE_BIT;
    } else if (VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == layout ||
               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == layout ||
               VK_IMAGE_LAYOUT_PRESENT_SRC_KHR == layout) {
        // There are no writes that need to be made available
        flags = 0;
    }
    return flags;
}

VkImageAspectFlags vk_format_to_aspect_flags(VkFormat format) {
    switch (format) {
        case VK_FORMAT_S8_UINT:
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        case VK_FORMAT_D24_UNORM_S8_UINT: // fallthrough
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

void GrVkImage::setImageLayoutAndQueueIndex(const GrVkGpu* gpu,
                                            VkImageLayout newLayout,
                                            VkAccessFlags dstAccessMask,
                                            VkPipelineStageFlags dstStageMask,
                                            bool byRegion,
                                            uint32_t newQueueFamilyIndex) {
    SkASSERT(!gpu->isDeviceLost());
    SkASSERT(newLayout == this->currentLayout() ||
             (VK_IMAGE_LAYOUT_UNDEFINED != newLayout &&
              VK_IMAGE_LAYOUT_PREINITIALIZED != newLayout));
    VkImageLayout currentLayout = this->currentLayout();
    uint32_t currentQueueIndex = this->currentQueueFamilyIndex();

#ifdef SK_DEBUG
    if (this->spec().fSharingMode == VK_SHARING_MODE_CONCURRENT) {
        if (newQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED) {
            SkASSERT(currentQueueIndex == VK_QUEUE_FAMILY_IGNORED ||
                     currentQueueIndex == VK_QUEUE_FAMILY_EXTERNAL ||
                     currentQueueIndex == VK_QUEUE_FAMILY_FOREIGN_EXT);
        } else {
            SkASSERT(newQueueFamilyIndex == VK_QUEUE_FAMILY_EXTERNAL ||
                     newQueueFamilyIndex == VK_QUEUE_FAMILY_FOREIGN_EXT);
            SkASSERT(currentQueueIndex == VK_QUEUE_FAMILY_IGNORED);
        }
    } else {
        SkASSERT(this->spec().fSharingMode == VK_SHARING_MODE_EXCLUSIVE);
        if (newQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED ||
            currentQueueIndex == gpu->queueIndex()) {
            SkASSERT(currentQueueIndex == VK_QUEUE_FAMILY_IGNORED ||
                     currentQueueIndex == VK_QUEUE_FAMILY_EXTERNAL ||
                     currentQueueIndex == VK_QUEUE_FAMILY_FOREIGN_EXT ||
                     currentQueueIndex == gpu->queueIndex());
        } else if (newQueueFamilyIndex == VK_QUEUE_FAMILY_EXTERNAL ||
                   newQueueFamilyIndex == VK_QUEUE_FAMILY_FOREIGN_EXT) {
            SkASSERT(currentQueueIndex == VK_QUEUE_FAMILY_IGNORED ||
                     currentQueueIndex == gpu->queueIndex());
        }
    }
#endif

    if (this->spec().fSharingMode == VK_SHARING_MODE_EXCLUSIVE) {
        if (newQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED) {
            newQueueFamilyIndex = gpu->queueIndex();
        }
        if (currentQueueIndex == VK_QUEUE_FAMILY_IGNORED) {
            currentQueueIndex = gpu->queueIndex();
        }
    }

    // If the old and new layout are the same and the layout is a read only layout, there is no need
    // to put in a barrier unless we also need to switch queues.
    if (newLayout == currentLayout && currentQueueIndex == newQueueFamilyIndex &&
        (VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL == currentLayout ||
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == currentLayout ||
         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == currentLayout)) {
        return;
    }

    VkAccessFlags srcAccessMask = GrVkImage::LayoutToSrcAccessMask(currentLayout);
    VkPipelineStageFlags srcStageMask = GrVkImage::LayoutToPipelineSrcStageFlags(currentLayout);

    VkImageAspectFlags aspectFlags = vk_format_to_aspect_flags(this->spec().fFormat);

    VkImageMemoryBarrier imageMemoryBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,          // sType
        nullptr,                                         // pNext
        srcAccessMask,                                   // srcAccessMask
        dstAccessMask,                                   // dstAccessMask
        currentLayout,                                   // oldLayout
        newLayout,                                       // newLayout
        currentQueueIndex,                               // srcQueueFamilyIndex
        newQueueFamilyIndex,                             // dstQueueFamilyIndex
        this->image(),                                   // image
        { aspectFlags, 0, this->mipLevels(), 0, 1 }      // subresourceRange
    };
    SkASSERT(srcAccessMask == imageMemoryBarrier.srcAccessMask);
    gpu->addImageMemoryBarrier(this->resource(), srcStageMask, dstStageMask, byRegion,
                               &imageMemoryBarrier);

    this->updateImageLayout(newLayout);
    this->setQueueFamilyIndex(newQueueFamilyIndex);
}

bool GrVkImage::CreateImage(GrVkGpu* gpu,
                            SkISize dimensions,
                            const GrVkSurfaceInfo& surfaceInfo,
                            ImageObjects* outObjects) {
    if (!dimensions.fWidth || !dimensions.fHeight) {
        return false;
    }
    if ((surfaceInfo.fProtected == GrProtected::kYes) && !gpu->vkCaps().supportsProtectedMemory()) {
        return false;
    }
    VkImage image = VK_NULL_HANDLE;
    GrVkAlloc alloc;

    bool isLinear = VK_IMAGE_TILING_LINEAR == surfaceInfo.fImageTiling;
    VkImageLayout initialLayout = isLinear ? VK_IMAGE_LAYOUT_PREINITIALIZED
                                           : VK_IMAGE_LAYOUT_UNDEFINED;

    // Create Image
    VkSampleCountFlagBits vkSamples;
    if (!GrSampleCountToVkSampleCount(surfaceInfo.fSampleCount, &vkSamples)) {
        return false;
    }

    SkASSERT(VK_IMAGE_TILING_OPTIMAL == surfaceInfo.fImageTiling ||
             VK_SAMPLE_COUNT_1_BIT == vkSamples);

    VkImageCreateFlags createflags = 0;
    if (surfaceInfo.fProtected == GrProtected::kYes || gpu->protectedContext()) {
        createflags |= VK_IMAGE_CREATE_PROTECTED_BIT;
    }
    const VkImageCreateInfo imageCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,                               // sType
        nullptr,                                                           // pNext
        createflags,                                                       // VkImageCreateFlags
        VK_IMAGE_TYPE_2D,                                                  // VkImageType
        surfaceInfo.fFormat,                                               // VkFormat
        {(uint32_t)dimensions.width(), (uint32_t)dimensions.height(), 1},  // VkExtent3D
        surfaceInfo.fLevelCount,                                           // mipLevels
        1,                                                                 // arrayLayers
        vkSamples,                                                         // samples
        surfaceInfo.fImageTiling,                                          // VkImageTiling
        surfaceInfo.fImageUsageFlags,                                      // VkImageUsageFlags
        surfaceInfo.fSharingMode,                                          // VkSharingMode
        0,                                                                 // queueFamilyCount
        nullptr,                                                           // pQueueFamilyIndices
        initialLayout                                                      // initialLayout
    };

    VkResult result;
    GR_VK_CALL_RESULT(gpu, result, CreateImage(gpu->device(), &imageCreateInfo, nullptr, &image));
    if (result != VK_SUCCESS) {
        return false;
    }

    if (!GrVkMemory::AllocAndBindImageMemory(gpu, image, &alloc)) {
        VK_CALL(gpu, DestroyImage(gpu->device(), image, nullptr));
        return false;
    }

    outObjects->fImage = image;
    outObjects->fAlloc = alloc;
    outObjects->fImageLayout = initialLayout;
    outObjects->fCurrentQueueFamily = VK_QUEUE_FAMILY_IGNORED;

    return true;
}

void GrVkImage::DestroyImage(const GrVkGpu* gpu, VkImage image, const GrVkAlloc& alloc) {
    VK_CALL(gpu, DestroyImage(gpu->device(), image, nullptr));
    GrVkMemory::FreeImageMemory(gpu, alloc);
}

GrVkImage::~GrVkImage() {
    // should have been released first
    SkASSERT(!fResource);
    SkASSERT(!fFramebufferView);
    SkASSERT(!fTextureView);
}

void GrVkImage::prepareForPresent(GrVkGpu* gpu) {
    VkImageLayout layout = this->currentLayout();
    if (fInitialQueueFamily != VK_QUEUE_FAMILY_EXTERNAL &&
        fInitialQueueFamily != VK_QUEUE_FAMILY_FOREIGN_EXT) {
        if (gpu->vkCaps().supportsSwapchain()) {
            layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }
    }
    this->setImageLayoutAndQueueIndex(gpu, layout, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, false,
                                      fInitialQueueFamily);
}

void GrVkImage::prepareForExternal(GrVkGpu* gpu) {
    this->setImageLayoutAndQueueIndex(gpu, this->currentLayout(), 0,
                                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, false,
                                     fInitialQueueFamily);
}

void GrVkImage::releaseImage() {
    if (fResource) {
        fResource->unref();
        fResource = nullptr;
    }
    fFramebufferView.reset();
    fTextureView.reset();
    fCachedBlendingInputDescSet.reset();
    fCachedMSAALoadInputDescSet.reset();
}

void GrVkImage::onRelease() {
    this->releaseImage();
    GrAttachment::onRelease();
}

void GrVkImage::onAbandon() {
    this->releaseImage();
    GrAttachment::onAbandon();
}

void GrVkImage::setResourceRelease(sk_sp<GrRefCntedCallback> releaseHelper) {
    SkASSERT(fResource);
    // Forward the release proc on to GrVkImage::Resource
    fResource->setRelease(std::move(releaseHelper));
}

void GrVkImage::Resource::freeGPUData() const {
    this->invokeReleaseProc();
    VK_CALL(fGpu, DestroyImage(fGpu->device(), fImage, nullptr));
    GrVkMemory::FreeImageMemory(fGpu, fAlloc);
}

void GrVkImage::BorrowedResource::freeGPUData() const {
    this->invokeReleaseProc();
}

static void write_input_desc_set(GrVkGpu* gpu,
                                 VkImageView view,
                                 VkImageLayout layout,
                                 VkDescriptorSet descSet) {
    VkDescriptorImageInfo imageInfo;
    memset(&imageInfo, 0, sizeof(VkDescriptorImageInfo));
    imageInfo.sampler = VK_NULL_HANDLE;
    imageInfo.imageView = view;
    imageInfo.imageLayout = layout;

    VkWriteDescriptorSet writeInfo;
    memset(&writeInfo, 0, sizeof(VkWriteDescriptorSet));
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.pNext = nullptr;
    writeInfo.dstSet = descSet;
    writeInfo.dstBinding = GrVkUniformHandler::kInputBinding;
    writeInfo.dstArrayElement = 0;
    writeInfo.descriptorCount = 1;
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    writeInfo.pImageInfo = &imageInfo;
    writeInfo.pBufferInfo = nullptr;
    writeInfo.pTexelBufferView = nullptr;

    GR_VK_CALL(gpu->vkInterface(), UpdateDescriptorSets(gpu->device(), 1, &writeInfo, 0, nullptr));
}

gr_rp<const GrVkDescriptorSet> GrVkImage::inputDescSetForBlending(GrVkGpu* gpu) {
    if (!this->supportsInputAttachmentUsage()) {
        return nullptr;
    }
    if (fCachedBlendingInputDescSet) {
        return fCachedBlendingInputDescSet;
    }

    fCachedBlendingInputDescSet.reset(gpu->resourceProvider().getInputDescriptorSet());
    if (!fCachedBlendingInputDescSet) {
        return nullptr;
    }

    write_input_desc_set(gpu,
                         this->framebufferView()->imageView(),
                         VK_IMAGE_LAYOUT_GENERAL,
                         *fCachedBlendingInputDescSet->descriptorSet());

    return fCachedBlendingInputDescSet;
}

gr_rp<const GrVkDescriptorSet> GrVkImage::inputDescSetForMSAALoad(GrVkGpu* gpu) {
    if (!this->supportsInputAttachmentUsage()) {
        return nullptr;
    }
    if (fCachedMSAALoadInputDescSet) {
        return fCachedMSAALoadInputDescSet;
    }

    fCachedMSAALoadInputDescSet.reset(gpu->resourceProvider().getInputDescriptorSet());
    if (!fCachedMSAALoadInputDescSet) {
        return nullptr;
    }

    write_input_desc_set(gpu,
                         this->framebufferView()->imageView(),
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                         *fCachedMSAALoadInputDescSet->descriptorSet());

    return fCachedMSAALoadInputDescSet;
}

GrVkImageInfo GrVkImage::imageInfo() const {
    GrVkImageInfo info;
    info.fImage = this->image();
    info.fAlloc = this->alloc();
    info.fImageTiling = this->spec().fImageTiling;
    info.fImageLayout = this->currentLayout();
    info.fFormat = this->spec().fFormat;
    info.fImageUsageFlags = this->spec().fImageUsageFlags;
    info.fSampleCount = this->numSamples();
    info.fLevelCount = this->mipLevels();
    info.fCurrentQueueFamily = this->currentQueueFamilyIndex();
    info.fProtected = GrProtected(this->isProtected());
    info.fSharingMode = this->spec().fSharingMode;
    return info;
}

GrVkGpu* GrVkImage::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}

#if GR_TEST_UTILS
void GrVkImage::setCurrentQueueFamilyToGraphicsQueue(GrVkGpu* gpu) {
    fMutableState->setQueueFamilyIndex(gpu->queueIndex());
}
#endif

