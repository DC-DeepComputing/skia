/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnMtlSharedContext_DEFINED
#define skgpu_graphite_DawnMtlSharedContext_DEFINED

#include "src/gpu/graphite/mtl/MtlSharedContext.h"

#include "include/gpu/graphite/dawn/DawnBackendContext.h"

namespace skgpu::graphite {
struct ContextOptions;
class DawnSharedContext;

class DawnMtlSharedContext final : public MtlSharedContext {
public:
    static sk_sp<SharedContext> Make(const MtlBackendContext&,
                                     const DawnBackendContext&,
                                     const ContextOptions&);
    ~DawnMtlSharedContext() override;

    std::unique_ptr<ResourceProvider> makeResourceProvider(SingleOwner*,
                                                           uint32_t recorderID,
                                                           size_t resourceBudget) override;

    const wgpu::Device& dawnDevice() const;

    DawnSharedContext* dawnSharedContext() const;

    void checkBackends();

    std::optional<BackendTexture>
    convertBackendTextureIfNeeded(const BackendTexture&) const override;

    std::optional<TextureInfo> convertTextureInfo(const TextureInfo&) const override;

private:
    DawnMtlSharedContext(sk_cfp<id<MTLDevice>>,
                         sk_sp<skgpu::MtlMemoryAllocator>,
                         std::unique_ptr<const MtlCaps>,
                         SkExecutor*,
                         SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects,
                         sk_sp<SharedContext> dawnSharedContext);

    sk_sp<SharedContext> fDawnSharedContext;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnMtlSharedContext_DEFINED
