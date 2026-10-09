/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/d3d11/D3D11ResourceProvider.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/d3d11/D3D11GraphiteTypes.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/d3d11/D3D11Buffer.h"
#include "src/gpu/graphite/d3d11/D3D11Sampler.h"
#include "src/gpu/graphite/d3d11/D3D11SharedContext.h"
#include "src/gpu/graphite/d3d11/D3D11Texture.h"

namespace skgpu::graphite {

D3D11ResourceProvider::D3D11ResourceProvider(SharedContext* sharedContext,
                                             SingleOwner* singleOwner,
                                             uint32_t recorderID,
                                             size_t resourceBudget)
        : ResourceProvider(sharedContext, singleOwner, recorderID, resourceBudget) {}

D3D11ResourceProvider::~D3D11ResourceProvider() = default;

const D3D11SharedContext* D3D11ResourceProvider::d3d11SharedContext() const {
    return static_cast<const D3D11SharedContext*>(fSharedContext);
}

sk_sp<ComputePipeline> D3D11ResourceProvider::createComputePipeline(
        const ComputePipelineDesc&) {
    return nullptr;
}

sk_sp<Texture> D3D11ResourceProvider::createTexture(SkISize dimensions,
                                                    const TextureInfo& info,
                                                    std::string_view label) {
    return D3D11Texture::Make(this->d3d11SharedContext(), dimensions, info, label);
}

sk_sp<Buffer> D3D11ResourceProvider::createBuffer(size_t size,
                                                  BufferType type,
                                                  AccessPattern accessPattern,
                                                  std::string_view label) {
    return D3D11Buffer::Make(this->d3d11SharedContext(), size, type, accessPattern, label);
}

sk_sp<Sampler> D3D11ResourceProvider::createSampler(const SamplerDesc& samplerDesc) {
    return D3D11Sampler::Make(this->d3d11SharedContext(), samplerDesc);
}

sk_sp<Texture> D3D11ResourceProvider::onCreateWrappedTexture(const BackendTexture& backendTexture,
                                                            std::string_view label) {
    ID3D11Texture2D* texture = BackendTextures::GetD3D11Texture2D(backendTexture);
    return D3D11Texture::MakeWrapped(this->d3d11SharedContext(),
                                     backendTexture.dimensions(),
                                     backendTexture.info(),
                                     texture,
                                     label);
}

BackendTexture D3D11ResourceProvider::onCreateBackendTexture(SkISize dimensions,
                                                            const TextureInfo& info) {
    sk_sp<D3D11Texture> texture =
            D3D11Texture::Make(this->d3d11SharedContext(), dimensions, info, "BackendTexture");
    if (!texture) {
        return {};
    }

    return BackendTextures::MakeD3D11(
            dimensions, TextureInfoPriv::Get<D3D11TextureInfo>(info), texture->d3d11Texture());
}

void D3D11ResourceProvider::onDeleteBackendTexture(const BackendTexture&) {}

} // namespace skgpu::graphite
