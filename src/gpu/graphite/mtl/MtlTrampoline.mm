/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlTrampoline.h"

#include "src/gpu/graphite/mtl/MtlGpu.h"
#include "src/gpu/graphite/mtl/MtlQueueManager.h"

namespace skgpu::graphite {
sk_sp<Gpu> MtlTrampoline::MakeGpu(const MtlBackendContext& backendContext) {
    return MtlGpu::Make(backendContext);
}

std::unique_ptr<QueueManager> MtlTrampoline::MakeQueueManager() {
    return std::make_unique<MtlQueueManager>();
}

}  // namespace skgpu::graphite
