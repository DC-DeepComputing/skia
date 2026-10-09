/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/d3d11/D3D11SharedContext.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/ThreadSafeResourceProvider.h"
#include "src/gpu/graphite/d3d11/D3D11GraphicsPipeline.h"
#include "src/gpu/graphite/d3d11/D3D11QueueManager.h"
#include "src/gpu/graphite/d3d11/D3D11ResourceProvider.h"

namespace skgpu::graphite {

namespace ContextFactory {
std::unique_ptr<Context> MakeD3D11(const D3D11BackendContext& backendContext,
                                   const ContextOptions& options) {
    sk_sp<SharedContext> sharedContext = D3D11SharedContext::Make(backendContext, options);
    if (!sharedContext) {
        return nullptr;
    }

    auto queueManager = std::make_unique<D3D11QueueManager>(backendContext.fDeviceContext.Get(),
                                                            sharedContext.get());
    if (!queueManager) {
        return nullptr;
    }

    auto context = ContextCtorAccessor::MakeContext(std::move(sharedContext),
                                                    std::move(queueManager),
                                                    options);
    SkASSERT(context);
    return context;
}
} // namespace ContextFactory

sk_sp<SharedContext> D3D11SharedContext::Make(const D3D11BackendContext& backendContext,
                                              const ContextOptions& options) {
    if (!backendContext.fDevice || !backendContext.fDeviceContext) {
        return nullptr;
    }

    auto caps = std::make_unique<const D3D11Caps>(backendContext, options);

    return sk_sp<SharedContext>(new D3D11SharedContext(backendContext,
                                                      std::move(caps),
                                                      options.fExecutor,
                                                      options.fUserDefinedKnownRuntimeEffects));
}

D3D11SharedContext::D3D11SharedContext(
        const D3D11BackendContext& backendContext,
        std::unique_ptr<const D3D11Caps> caps,
        SkExecutor* executor,
        SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects)
        : SharedContext(std::move(caps),
                        BackendApi::kD3D11,
                        executor,
                        userDefinedKnownRuntimeEffects)
        , fAdapter(backendContext.fAdapter)
        , fDevice(backendContext.fDevice)
        , fDeviceContext(backendContext.fDeviceContext) {
    fThreadSafeResourceProvider = std::make_unique<ThreadSafeResourceProvider>(
            this->makeResourceProvider(&fSingleOwner,
                                       SK_InvalidGenID,
                                       kThreadedSafeResourceBudget));
}

D3D11SharedContext::~D3D11SharedContext() {
    fThreadSafeResourceProvider.reset();
    this->globalCache()->deleteResources();
}

std::unique_ptr<ResourceProvider> D3D11SharedContext::makeResourceProvider(
        SingleOwner* singleOwner, uint32_t recorderID, size_t resourceBudget) {
    return std::make_unique<D3D11ResourceProvider>(this, singleOwner, recorderID, resourceBudget);
}

sk_sp<GraphicsPipeline> D3D11SharedContext::createGraphicsPipeline(
        const RuntimeEffectDictionary* runtimeDict,
        const UniqueKey& pipelineKey,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> flags,
        uint32_t compilationID) {
    return D3D11GraphicsPipeline::Make(this,
                                       runtimeDict,
                                       pipelineKey,
                                       pipelineDesc,
                                       renderPassDesc,
                                       flags,
                                       compilationID);
}

} // namespace skgpu::graphite
