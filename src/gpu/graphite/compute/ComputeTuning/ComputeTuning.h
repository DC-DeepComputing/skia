/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_compute_ComputeTuning_DEFINED
#define skgpu_graphite_compute_ComputeTuning_DEFINED

#include "src/gpu/graphite/compute/ComputeTuning/DownsampleTuning.h"
#include "src/gpu/graphite/compute/ComputeTuning/GaussianBlurTuning.h"
#include "src/gpu/graphite/compute/ComputeTuning/GpuArchitecture.h"

namespace skgpu::graphite {

/**
 * The tunings of the compute steps whose SkSL is composed when the Context is created (see
 * ComputeProvider). Each step's tuning has its own header in this directory, with the step's
 * defaults and the changes for each GpuArchitecture whose best tuning differs from them.
 *
 * Caps selects the tuning of the device's architecture, which tests can override through
 * ContextOptionsPriv.
 */
struct ComputeTuning {
    GaussianBlurTuning fGaussianBlur;
    DownsampleTuning fDownsample;

    // Returns the defaults, with the changes for `architecture`.
    static constexpr ComputeTuning For(GpuArchitecture architecture) {
        return {GaussianBlurTuning::For(architecture), DownsampleTuning::For(architecture)};
    }
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_compute_ComputeTuning_DEFINED
