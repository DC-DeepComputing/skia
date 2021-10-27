/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ContextPriv.h"

namespace skgpu {

Gpu* ContextPriv::gpu() {
    return fContext->fGpu.get();
}

ProgramCache* ContextPriv::programCache() {
    return fContext->fProgramCache.get();
}

UniformCache* ContextPriv::uniformCache() {
    return fContext->fUniformCache.get();
}

} // namespace skgpu
