/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_compute_GpuArchitecture_DEFINED
#define skgpu_graphite_compute_GpuArchitecture_DEFINED

namespace skgpu::graphite {

/**
 * The GPU architectures whose compute tunings differ from the defaults (see ComputeTuning). Each
 * backend's Caps identifies the device's architecture. Add an architecture when a step's best
 * tuning on it differs from the step's defaults.
 */
enum class GpuArchitecture {
    // Every architecture without a tuning of its own, which uses the defaults.
    kOther,
    // Arm Mali GPUs of the Valhall architecture, e.g. the Mali-G710 of the Pixel 7 Pro.
    kMaliValhall,
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_compute_GpuArchitecture_DEFINED
