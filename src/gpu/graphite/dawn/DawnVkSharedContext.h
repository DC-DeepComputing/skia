/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnVkSharedContext_DEFINED
#define skgpu_graphite_DawnVkSharedContext_DEFINED

#include "src/gpu/graphite/vk/VulkanSharedContext.h"

#include "include/gpu/graphite/dawn/DawnBackendContext.h"

namespace skgpu::graphite {
struct ContextOptions;
class DawnSharedContext;

class DawnVkSharedContext final : public VulkanSharedContext {
public:
    static sk_sp<SharedContext> Make(const VulkanBackendContext&,
                                     const DawnBackendContext&,
                                     const ContextOptions&);
    ~DawnVkSharedContext() override;

    std::unique_ptr<ResourceProvider> makeResourceProvider(SingleOwner*,
                                                           uint32_t recorderID,
                                                           size_t resourceBudget) override;

    const wgpu::Device& dawnDevice() const;

    DawnSharedContext* dawnSharedContext() const;

    void checkBackends();

    std::optional<BackendTexture>
    convertBackendTextureIfNeeded(const BackendTexture&) const override;

    std::optional<TextureInfo> convertTextureInfo(const TextureInfo&) const override;

    TextureInfo getDefaultSampledTextureInfo(SkColorType,
                                             Mipmapped,
                                             Protected,
                                             Renderable) const override;

    QueueSubmitResult backendQueueSubmit(VkSubmitInfo*, VkFence*) const override;

private:
    DawnVkSharedContext(const VulkanBackendContext&,
                        sk_sp<const skgpu::VulkanInterface>,
                        sk_sp<skgpu::VulkanMemoryAllocator>,
                        std::unique_ptr<const VulkanCaps>,
                        SkExecutor*,
                        PersistentPipelineStorage*,
                        SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects,
                        sk_sp<SharedContext> dawnSharedContext);

    sk_sp<SharedContext> fDawnSharedContext;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnVkSharedContext_DEFINED
