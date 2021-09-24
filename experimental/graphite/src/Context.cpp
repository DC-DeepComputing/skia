/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Context.h"

#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/Gpu.h"

namespace skgpu {

Context::Context(sk_sp<Gpu> gpu) : fGpu(std::move(gpu)) {}
Context::~Context() {}

#ifdef SK_METAL
sk_sp<Context> Context::MakeMetal(const GrMtlBackendContext& backendContext) {
    sk_sp<Gpu> gpu = MtlGpu::Make(backendContext);
    if (!gpu) {
        return nullptr;
    }

    return sk_sp<Context>(new Context(std::move(gpu)));
}
#endif

#ifdef SK_VULKAN
sk_sp<Context> Context::MakeVulkan(const GrVkBackendContext& backendContext) {
    sk_sp<Gpu> gpu; // = VkGpu::Make(backendContext);
    if (!gpu) {
        return nullptr;
    }

    return sk_sp<Context>(new Context(std::move(gpu)));
}
#endif

} // namespace skgpu
