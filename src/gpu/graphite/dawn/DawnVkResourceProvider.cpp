/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnVkResourceProvider.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "include/gpu/vk/VulkanMutableTextureState.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/gpu/graphite/dawn/DawnTexture.h"
#include "src/gpu/graphite/dawn/DawnVkSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"
#include "src/gpu/graphite/vk/VulkanYcbcrConversion.h"

namespace skgpu::graphite {

DawnVkResourceProvider::DawnVkResourceProvider(SharedContext* sharedContext,
                                               SingleOwner* singleOwner,
                                               uint32_t recorderID,
                                               size_t resourceBudget)
        : VulkanResourceProvider(sharedContext, singleOwner, recorderID, resourceBudget) {}


sk_sp<Texture> DawnVkResourceProvider::onCreateWrappedTexture(const BackendTexture& texture) {
    return VulkanResourceProvider::internalCreateWrappedTexture(texture, /*saveOriginalLayout=*/true);
}

DawnSharedContext* DawnVkResourceProvider::dawnSharedContext() const {
    DawnVkSharedContext* dvsc = static_cast<DawnVkSharedContext*>(fSharedContext);
    return dvsc->dawnSharedContext();
}

void DawnVkResourceProvider::checkBackends() {
    DawnVkSharedContext* dmsc = static_cast<DawnVkSharedContext*>(fSharedContext);
    dmsc->checkBackends();
}

BackendTexture DawnVkResourceProvider::onCreateBackendTexture(SkISize dimensions,
                                                              const TextureInfo& info) {
  //  SkDebugf("onCreateBackendTex 1. info backend: %d\n", info.backend());
    wgpu::Texture texture = DawnTexture::MakeDawnTexture(this->dawnSharedContext(),
                                                         dimensions,
                                                         info);
   // SkDebugf("onCreateBackendTex 2\n");
    if (!texture) {
    //    SkDebugf("onCreateBackendTex 3\n");
        return {};
    }
   //SkDebugf("onCreateBackendTex 4\n");

    return BackendTextures::MakeDawn(texture.MoveToCHandle());
}

void DawnVkResourceProvider::onDeleteBackendTexture(const BackendTexture& texture) {
    SkASSERT(texture.isValid());
    SkASSERT(texture.backend() == BackendApi::kDawn);

    // Automatically release the pointers in wgpu::TextureView & wgpu::Texture's dtor.
    // Acquire() won't increment the ref count.
    wgpu::TextureView::Acquire(BackendTextures::GetDawnTextureViewPtr(texture));
    // We need to explicitly call Destroy() here since since that is the recommended way to delete
    // a Dawn texture predictably versus just dropping a ref and relying on garbage collection.
    //
    // Additionally this helps to work around an issue where Skia may have cached a BindGroup that
    // references the underlying texture. Skia currently doesn't destroy BindGroups when its use of
    // the texture goes away, thus a ref to the texture remains on the BindGroup and memory is never
    // cleared up unless we call Destroy() here.
    wgpu::Texture::Acquire(BackendTextures::GetDawnTexturePtr(texture)).Destroy();
}

} // namespace skgpu::graphite

