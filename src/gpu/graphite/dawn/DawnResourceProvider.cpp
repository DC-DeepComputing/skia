/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnResourceProvider.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "src/gpu/graphite/dawn/DawnBuffer.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/gpu/graphite/dawn/DawnUtils.h"


namespace skgpu::graphite {

DawnResourceProvider::DawnResourceProvider(SharedContext* sharedContext,
                                           SingleOwner* singleOwner)
        : ResourceProvider(sharedContext, singleOwner) {}

DawnResourceProvider::~DawnResourceProvider() = default;

sk_sp<Texture> DawnResourceProvider::createWrappedTexture(const BackendTexture& texture) {
    SkASSERT(false);
    return {};
}

sk_sp<GraphicsPipeline> DawnResourceProvider::createGraphicsPipeline(
        const SkRuntimeEffectDictionary* runtimeDict,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc) {
    SkASSERT(false);
    return {};
}

sk_sp<ComputePipeline> DawnResourceProvider::createComputePipeline(const ComputePipelineDesc&) {
    SkASSERT(false);
    return nullptr;
}

sk_sp<Texture> DawnResourceProvider::createTexture(SkISize dimensions,
                                                   const TextureInfo& info,
                                                   SkBudgeted budgeted) {
    SkASSERT(false);
    return nullptr;
}

sk_sp<Buffer> DawnResourceProvider::createBuffer(size_t size,
                                                 BufferType type,
                                                 PrioritizeGpuReads prioritizeGpuReads) {

    return DawnBuffer::Make(dawnSharedContext(), size, type, prioritizeGpuReads);
}

sk_sp<Sampler> DawnResourceProvider::createSampler(const SkSamplingOptions& options,
                                                   SkTileMode xTileMode,
                                                   SkTileMode yTileMode) {
    SkASSERT(false);
    return nullptr;
}

BackendTexture DawnResourceProvider::onCreateBackendTexture(SkISize dimensions,
                                                            const TextureInfo& info) {
    SkASSERT(false);
    return {};
}

void DawnResourceProvider::onDeleteBackendTexture(BackendTexture& texture) {
    SkASSERT(texture.isValid());
    SkASSERT(texture.backend() == BackendApi::kDawn);
    SkASSERT(false);
}

const DawnSharedContext* DawnResourceProvider::dawnSharedContext() const {
    return static_cast<const DawnSharedContext*>(fSharedContext);
}

} // namespace skgpu::graphite
