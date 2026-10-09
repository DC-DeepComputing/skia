/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_D3D11ResourceProvider_DEFINED
#define skgpu_graphite_D3D11ResourceProvider_DEFINED

#include "src/gpu/graphite/ResourceProvider.h"

namespace skgpu::graphite {

class D3D11SharedContext;

class D3D11ResourceProvider final : public ResourceProvider {
public:
    D3D11ResourceProvider(SharedContext* sharedContext,
                          SingleOwner* singleOwner,
                          uint32_t recorderID,
                          size_t resourceBudget);
    ~D3D11ResourceProvider() override;

    const D3D11SharedContext* d3d11SharedContext() const;

private:
    sk_sp<ComputePipeline> createComputePipeline(const ComputePipelineDesc&) override;
    sk_sp<Texture> createTexture(SkISize, const TextureInfo&, std::string_view label) override;
    sk_sp<Buffer> createBuffer(size_t, BufferType, AccessPattern, std::string_view label) override;
    sk_sp<Sampler> createSampler(const SamplerDesc&) override;

    sk_sp<Texture> onCreateWrappedTexture(const BackendTexture&, std::string_view label) override;
    BackendTexture onCreateBackendTexture(SkISize dimensions, const TextureInfo&) override;
    void onDeleteBackendTexture(const BackendTexture&) override;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_D3D11ResourceProvider_DEFINED
