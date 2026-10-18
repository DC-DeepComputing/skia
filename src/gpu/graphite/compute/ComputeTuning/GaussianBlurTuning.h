/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_compute_GaussianBlurTuning_DEFINED
#define skgpu_graphite_compute_GaussianBlurTuning_DEFINED

#include "src/gpu/graphite/compute/ComputeTuning/GpuArchitecture.h"

namespace skgpu::graphite {

/**
 * The tuning of GaussianBlurStep, which is composed into its SkSL when the step is created (see
 * ComputeProvider). It changes how the work is split across invocations and workgroups, but not
 * the results. GaussianBlurStep::IsSupported() checks a tuning against the step's requirements and
 * the device's compute limits.
 *
 * The member initializers are the defaults, which were tuned on an Apple M-series GPU. For() adds
 * the changes for each architecture whose best tuning differs from them.
 */
struct GaussianBlurTuning {
    // The output tile of each workgroup is (1 << fTileWidthLog) x (1 << fTileHeightLog).
    int fTileWidthLog = 6;
    int fTileHeightLog = 4;
    // Each invocation of the vertical pass sums this many consecutive rows of one column in
    // registers, so the tile's rows are split into (tile height / this) blocks. A power of two
    // that's at most the tile height.
    int fVerticalSumsPerThread = 8;
    // Each invocation of the horizontal pass blurs this many consecutive outputs of one row, so
    // each row is split into (tile width / this) strips, and the workgroup has an invocation for
    // each strip of each row. A power of two that's at most the tile width, and at least 4 if
    // fPadSums is set.
    int fHorizontalSumsPerThread = 4;
    // The radii of the kernel's buckets are the multiples of this up to
    // GaussianBlurStep::kMaxRadius, which it must divide. A smaller step wastes fewer taps on radii
    // between buckets, but generates more code.
    int fBucketStep = 2;
    // Whether the vertical sums in workgroup memory have a pad entry after every 4 entries, which
    // keeps the horizontal pass's strided reads from conflicting on banked workgroup memory.
    bool fPadSums = true;

    // Returns the defaults, with the changes for `architecture`.
    static constexpr GaussianBlurTuning For(GpuArchitecture architecture) {
        GaussianBlurTuning tuning;
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

#endif  // skgpu_graphite_compute_GaussianBlurTuning_DEFINED
