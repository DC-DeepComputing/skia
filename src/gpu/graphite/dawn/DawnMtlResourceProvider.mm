/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnMtlResourceProvider.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"
#include "src/gpu/graphite/dawn/DawnMtlSharedContext.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/gpu/graphite/dawn/DawnTexture.h"
//#include "src/gpu/graphite/mtl/MtlSharedContext.h"

namespace skgpu::graphite {

DawnMtlResourceProvider::DawnMtlResourceProvider(SharedContext* sharedContext,
                                                 SingleOwner* singleOwner,
                                                 uint32_t recorderID,
                                                 size_t resourceBudget)
        : MtlResourceProvider(sharedContext, singleOwner, recorderID, resourceBudget) {}

/*
sk_sp<Texture> DawnMtlResourceProvider::onCreateWrappedTexture(const BackendTexture& backendTex) {

}
 */

DawnSharedContext* DawnMtlResourceProvider::dawnSharedContext() const {
    DawnMtlSharedContext* dmsc = static_cast<DawnMtlSharedContext*>(fSharedContext);
    return dmsc->dawnSharedContext();
}

void DawnMtlResourceProvider::checkBackends() {
    DawnMtlSharedContext* dmsc = static_cast<DawnMtlSharedContext*>(fSharedContext);
    dmsc->checkBackends();
}

BackendTexture DawnMtlResourceProvider::onCreateBackendTexture(SkISize dimensions,
                                                               const TextureInfo& info) {
    wgpu::Texture texture = DawnTexture::MakeDawnTexture(this->dawnSharedContext(),
                                                         dimensions,
                                                         info);
    if (!texture) {
        return {};
    }

    return BackendTextures::MakeDawn(texture.MoveToCHandle());
}

void DawnMtlResourceProvider::onDeleteBackendTexture(const BackendTexture& texture) {
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

