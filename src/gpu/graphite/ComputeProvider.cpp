/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ComputeProvider.h"

#if defined(SK_ENABLE_SPARSE_STRIPS)
#include "include/private/SkAssert.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#endif

namespace skgpu::graphite {

ComputeProvider::ComputeProvider(const Caps* caps,
                                 StaticBufferManager* bufferManager,
                                 const GlobalCache* globalCache,
                                 PathRendererStrategy pathRendererStrategy) {
#if defined(SK_ENABLE_SPARSE_STRIPS)
    // Only the GPU sparse strips coverage pass reads the LUT. Its strategy is never supported in
    // protected contexts, where the upload would fail Context creation: Vulkan makes their GPU-only
    // storage buffers protected, but static buffers are filled by unprotected copies.
    if (pathRendererStrategy == PathRendererStrategy::kGPUSparseStripsMSAA8) {
        SkASSERT(caps->computeSupport() && !caps->protectedSupport());
        const SparseStripConfig::LUTArray& lut = globalCache->getLUTArray();
        BufferWriter writer = bufferManager->getStorageWriter(sizeof(lut), &fSparseStripsMaskLUT);
        if (writer) {
            writer.write(lut.data(), sizeof(lut));
        }
    }
#endif
}

ComputeProvider::~ComputeProvider() = default;

} // namespace skgpu::graphite
