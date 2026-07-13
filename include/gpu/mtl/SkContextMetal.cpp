/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/mtl/SkContextMetal.h"

#include "include/gpu/graphite/mtl/MtlBackendContext.h"

namespace skcontext {
class SkContext;
class SkContextOptions;

sk_sp<SkContext> MakeGanesh(const skgpu::graphite::MtlBackendContext& mtlContext,
                            const SkContextOptions& options) {
    return nullptr;
}

sk_sp<SkContext> MakeGraphite(const skgpu::graphite::MtlBackendContext& mtlContext,
                              const SkContextOptions& options) {
    return nullptr;
}

}  // namespace skcontext
