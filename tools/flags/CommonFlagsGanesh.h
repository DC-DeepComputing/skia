/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#pragma once

#include "include/core/SkString.h"
#include "include/private/base/SkTArray.h"
#include "tools/flags/CommandLineFlags.h"

namespace skgpu::graphite {
struct ContextOptions;
};

namespace CommonFlags {
/**
 *  Helper to set GrContextOptions from common GPU flags, including
 *     --gpuThreads
 *     --cachePathMasks
 *     --allPathsVolatile
 *     --(no)gs
 *     --(no)ts
 *     --pr
 *     --internalSamples
 *     --disableDriverCorrectnessWorkarounds
 *     --reduceOpsTaskSplitting
 *     --dontReduceOpsTaskSplitting
 *     --allowMSAAOnNewIntel
 */
void SetCtxOptions(struct GrContextOptions*);

}  // namespace CommonFlags
