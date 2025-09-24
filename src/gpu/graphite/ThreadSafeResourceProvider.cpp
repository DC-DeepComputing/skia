/*
* Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ThreadSafeResourceProvider.h"

#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Sampler.h"

namespace skgpu::graphite {

ThreadSafeResourceProvider::ThreadSafeResourceProvider(
        std::unique_ptr<ResourceProvider> resourceProvider)
    : fWrappedProvider(std::move(resourceProvider)) {}

sk_sp<Sampler> ThreadSafeResourceProvider::findOrCreateCompatibleSampler(const SamplerDesc& desc) {
    SkAutoSpinlock lock{fSpinLock};

    sk_sp<Sampler> sampler = fWrappedProvider->findOrCreateCompatibleSampler(desc);
    SkAssertResult(sampler->gpuMemorySize() == 0);
    return sampler;
}


} // namespace skgpu::graphite

