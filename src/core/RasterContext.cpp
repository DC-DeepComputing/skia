/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/RasterContext.h"
#include "src/core/SkContext.h"
#include "src/core/SkContextOptions.h"

namespace SkUnifiedContext {

// Creates a context for SkContext with only Software Rasterization
sk_sp<SkContext> MakeRaster(const SkContextOptions& options) {
    return nullptr;
}

}  // namespace SkUnifiedContext
