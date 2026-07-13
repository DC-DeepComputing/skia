/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkContextMetal_DEFINED
#define SkContextMetal_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu::graphite {
struct MtlBackendContext;
}

namespace skcontext {

// Creates a context wrapping a Ganesh GPU backend with Metal
sk_sp<SkContext> MakeGanesh(const skgpu::graphite::MtlBackendContext& mtlContext,
                            const SkContextOptions& options);

// Creates a context wrapping a Graphite GPU backend using Metal
sk_sp<SkContext> MakeGraphite(const skgpu::graphite::MtlBackendContext& mtlContext,
                              const SkContextOptions& options);

}  // namespace skcontext

#endif  // SkContextMetal_DEFINED
