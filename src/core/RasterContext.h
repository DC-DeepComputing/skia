/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef RasterContext_DEFINED
#define RasterContext_DEFINED

#include "include/core/SkRefCnt.h"

class SkContext;
struct SkContextOptions;

namespace SkUnifiedContext {

// Creates a context for SkContext with only Software Rasterization
sk_sp<SkContext> MakeRaster(const SkContextOptions& options);

}  // namespace SkUnifiedContext

#endif  // RasterContext_DEFINED
