/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RecorderOptionsPriv_DEFINED
#define skgpu_graphite_RecorderOptionsPriv_DEFINED

#include "src/gpu/graphite/BufferManager.h"

#include <cstddef>
#include <optional>

namespace skgpu::graphite {

/**
 * Private options that are only meant for testing within Skia's tools.
 */

struct RecorderOptionsPriv {
    // Override the default buffer sizes of the DrawBufferManager using this option.
    std::optional<DrawBufferManager::Options> fDbmOptions = std::nullopt;

    // Override the batch budget of the Recorder's ComputeScheduler. 0 adds every compute dispatch
    // to the task graph as soon as it's queued, and SIZE_MAX only does so when a dependency
    // requires it. See ComputeScheduler.
    std::optional<size_t> fComputeBatchBudget = std::nullopt;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_RecorderOptionsPriv_DEFINED
