/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Gpu.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/GpuWorkSubmission.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/sksl/SkSLCompiler.h"

namespace skgpu::graphite {

// This constant determines how many OutstandingSubmissions are allocated together as a block in
// the deque. As such it needs to balance allocating too much memory vs. incurring
// allocation/deallocation thrashing. It should roughly correspond to the max number of outstanding
// submissions we expect to see.
static constexpr int kDefaultOutstandingAllocCnt = 8;

Gpu::Gpu(sk_sp<const Caps> caps)
    : fCaps(std::move(caps)) {
}

Gpu::~Gpu() {
    // TODO: add disconnect?

    // TODO: destroyResources instead?
}

void Gpu::initCompiler() {
    fCompiler = std::make_unique<SkSL::Compiler>(fCaps->shaderCaps());
}

sk_sp<const Caps> Gpu::refCaps() const {
    return fCaps;
}

BackendTexture Gpu::createBackendTexture(SkISize dimensions, const TextureInfo& info) {
    if (dimensions.isEmpty() || dimensions.width()  > this->caps()->maxTextureSize() ||
                                dimensions.height() > this->caps()->maxTextureSize()) {
        return {};
    }

    return this->onCreateBackendTexture(dimensions, info);
}

void Gpu::deleteBackendTexture(BackendTexture& texture) {
    this->onDeleteBackendTexture(texture);
    // Invalidate the texture;
    texture = BackendTexture();
}


} // namespace skgpu::graphite
