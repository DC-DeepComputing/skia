/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnVkResourceProvider_DEFINED
#define skgpu_graphite_DawnVkResourceProvider_DEFINED

#include "src/gpu/graphite/vk/VulkanResourceProvider.h"

namespace skgpu::graphite {

class CommandBuffer;
class DawnSharedContext;
class MtlSharedContext;

class DawnVkResourceProvider final : public VulkanResourceProvider {
public:
    DawnVkResourceProvider(SharedContext* sharedContext,
                           SingleOwner*,
                           uint32_t recorderID,
                           size_t resourceBudget);
    ~DawnVkResourceProvider() override {}

    void checkBackends() override;

private:
    DawnSharedContext* dawnSharedContext() const;
    sk_sp<Texture> onCreateWrappedTexture(const BackendTexture&) override;

    BackendTexture onCreateBackendTexture(SkISize dimensions, const TextureInfo&) override;
    void onDeleteBackendTexture(const BackendTexture&) override;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VkResourceProvider_DEFINED

