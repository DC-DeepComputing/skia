/*
* Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ThreadSafeResourceProvider_DEFINED
#define skgpu_graphite_ThreadSafeResourceProvider_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkSpinlock.h"

#include <memory>

namespace skgpu {
class UniqueKey;
}

namespace skgpu::graphite {

class GraphicsPipeline;
class GraphicsPipelineDesc;
enum class PipelineCreationFlags : uint8_t;
struct RenderPassDesc;
class ResourceProvider;
class RuntimeEffectDictionary;
class Sampler;
struct SamplerDesc;
class VulkanRenderPass;

// The ThreadSafeResourceProvider wraps a backend-specific ResourceProvider and guards its
// usage via a SpinLock. It exposes the smallest API surface required.
// Note that once ResourceCaches are thread safe this could be replaced with the Context's
// ResourceProvider.
class ThreadSafeResourceProvider {
public:
    ThreadSafeResourceProvider(std::unique_ptr<ResourceProvider>);

    sk_sp<Sampler> findOrCreateCompatibleSampler(const SamplerDesc&) SK_EXCLUDES(fSpinLock);

    sk_sp<VulkanRenderPass> findOrCreateRenderPass(const RenderPassDesc&, bool compatibleOnly);

private:
    mutable SkSpinlock fSpinLock;

    std::unique_ptr<ResourceProvider> fWrappedProvider SK_GUARDED_BY(fSpinLock);
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ThreadSafeResourceProvider_DEFINED
