/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnMtlSharedContext.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes_cpp.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"
#include "src/gpu/graphite/dawn/DawnMtlResourceProvider.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/gpu/graphite/mtl/MtlGraphiteUtils.h"
#include "src/gpu/mtl/MtlMemoryAllocatorImpl.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {
struct ContextOptions;

sk_sp<SharedContext> DawnMtlSharedContext::Make(const MtlBackendContext& mtlBackendContext,
                                                const DawnBackendContext& dawnBackendContext,
                                                const ContextOptions& options) {
    if (@available(macOS 10.15, iOS 13.0, tvOS 13.0, *)) {
        // no warning needed
    } else {
        SKGPU_LOG_E("Skia's Graphite backend no longer supports this OS version.");
#ifdef SK_BUILD_FOR_IOS
        SKGPU_LOG_E("Minimum supported version is iOS/tvOS 13.0.");
#else
        SKGPU_LOG_E("Minimum supported version is MacOS 10.15.");
#endif
        return nullptr;
    }

    sk_sp<SharedContext> dawnSharedContext = DawnSharedContext::Make(dawnBackendContext, options);
    if (!dawnSharedContext) {
        return nullptr;
    }

    sk_cfp<id<MTLDevice>> device = sk_ret_cfp((id<MTLDevice>)(mtlBackendContext.fDevice.get()));

    std::unique_ptr<const MtlCaps> caps(new MtlCaps(device.get(), options));

    // TODO: Add memory allocator to context once we figure out synchronization
    sk_sp<MtlMemoryAllocator> memoryAllocator = skgpu::MtlMemoryAllocatorImpl::Make(device.get());
    if (!memoryAllocator) {
        SkDEBUGFAIL("No supplied Metal memory allocator and unable to create one internally.");
        return nullptr;
    }

    return sk_sp<SharedContext>(new DawnMtlSharedContext(std::move(device),
                                                         std::move(memoryAllocator),
                                                         std::move(caps),
                                                         options.fExecutor,
                                                         options.fUserDefinedKnownRuntimeEffects,
                                                         std::move(dawnSharedContext)));
}

DawnMtlSharedContext::DawnMtlSharedContext(
        sk_cfp<id<MTLDevice>> device,
        sk_sp<skgpu::MtlMemoryAllocator> memoryAllocator,
        std::unique_ptr<const MtlCaps> caps,
        SkExecutor* executor,
        SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects,
        sk_sp<SharedContext> dawnSharedContext)
        : MtlSharedContext(std::move(device),
                           std::move(memoryAllocator),
                           std::move(caps),
                           executor,
                           userDefinedKnownRuntimeEffects)
        , fDawnSharedContext(std::move(dawnSharedContext)) {
}

DawnMtlSharedContext::~DawnMtlSharedContext() {}


void DawnMtlSharedContext::checkBackends() {
/*
    id<MTLDevice> metalDevice = (__bridge id<MTLDevice>)this->dawnDevice().GetMetalDeviceId();
    id<MTLCommandQueue> metalQueue = (__bridge id<MTLCommandQueue>)this->dawnDevice().GetMetalQueueId();

    SkDebugf("Device name in checkBackends: %s, pointer: %p\n", metalDevice.name.UTF8String, metalDevice);
    SkDebugf("Queue pointer in checkBackends: %p\n", metalQueue);
    */
}

std::unique_ptr<ResourceProvider> DawnMtlSharedContext::makeResourceProvider(
        SingleOwner* singleOwner,
        uint32_t recorderID,
        size_t resourceBudget) {
    return std::unique_ptr<ResourceProvider>(new DawnMtlResourceProvider(this,
                                                                         singleOwner,
                                                                         recorderID,
                                                                         resourceBudget));
}

const wgpu::Device& DawnMtlSharedContext::dawnDevice() const {
    return this->dawnSharedContext()->device();
}

DawnSharedContext* DawnMtlSharedContext::dawnSharedContext() const {
    return static_cast<DawnSharedContext*>(fDawnSharedContext.get());
}

std::optional<BackendTexture>
DawnMtlSharedContext::convertBackendTextureIfNeeded(const BackendTexture& dawnBeTex) const {
    if (!dawnBeTex.isValid() || dawnBeTex.backend() != BackendApi::kDawn) {
        return {};
    }

    wgpu::Texture dawnTexture = BackendTextures::GetDawnTexturePtr(dawnBeTex);
    id<MTLTexture> mtlTexture = (id<MTLTexture>)(dawnTexture.GetMetalTextureId());
    return BackendTextures::MakeMetal(dawnBeTex.dimensions(), (CFTypeRef)mtlTexture);
}

std::optional<TextureInfo> DawnMtlSharedContext::convertTextureInfo(const TextureInfo& info) const {
    if (!info.isValid() || info.backend() != BackendApi::kDawn) {
        return {};
    }

    const auto& dawnInfo = TextureInfoPriv::Get<DawnTextureInfo>(info);

    MTLPixelFormat mtlFormat = TextureFormatToMTLPixelFormat(DawnFormatToTextureFormat(dawnInfo.fFormat));

    MtlTextureInfo mtlTextureInfo;
    mtlTextureInfo.fFormat = mtlFormat;
    mtlTextureInfo.fUsage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
    mtlTextureInfo.fStorageMode = MTLStorageModeManaged;

    return TextureInfos::MakeMetal(mtlTextureInfo);
}

} // namespace skgpu::graphite

