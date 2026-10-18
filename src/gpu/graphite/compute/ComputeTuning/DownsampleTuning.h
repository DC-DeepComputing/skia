/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_compute_DownsampleTuning_DEFINED
#define skgpu_graphite_compute_DownsampleTuning_DEFINED

#include "src/gpu/graphite/compute/ComputeTuning/GpuArchitecture.h"

namespace skgpu::graphite {

/**
 * The tuning of DownsampleStep, which is composed into its SkSL when the step is created (see
 * ComputeProvider). It changes how the work is split across invocations and workgroups, but not
 * the results. DownsampleStep::IsSupported() checks a tuning against the step's requirements and
 * the device's compute limits.
 *
 * The member initializers are the defaults, which were tuned on an Apple M-series GPU. For() adds
 * the changes for each architecture whose best tuning differs from them.
 */
struct DownsampleTuning {
    // The workgroup has 1 << fWorkgroupSizeLog invocations.
    int fWorkgroupSizeLog = 8;
    // Each lane of a cluster takes at least 1 << fMinSamplesPerLaneLog samples, since the shuffles
    // cost more than they save with fewer.
    int fMinSamplesPerLaneLog = 2;
    // A cluster has at most 1 << fMaxClusterSizeLog lanes.
    int fMaxClusterSizeLog = 4;

    // Returns the defaults, with the changes for `architecture`.
    static constexpr DownsampleTuning For(GpuArchitecture architecture) {
        DownsampleTuning tuning;
        switch (architecture) {
            case GpuArchitecture::kOther:
                break;
            case GpuArchitecture::kMaliValhall:
                // TODO: Tune on a Pixel 7 Pro (Mali-G710).
                break;
        }
        return tuning;
    }
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_compute_DownsampleTuning_DEFINED
