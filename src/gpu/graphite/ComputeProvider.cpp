/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ComputeProvider.h"

#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/compute/GaussianBlurStep.h"

namespace skgpu::graphite {

ComputeProvider::ComputeProvider(const Caps* caps) {
    // The compute blur is opt-in until it's been benchmarked against the shader blur, which also
    // decides whether small blurs should keep using the shader blur.
    if (caps->useComputeBlur() && GaussianBlurStep::IsSupported(caps)) {
        fGaussianBlur = std::make_unique<GaussianBlurStep>();
    }
}

ComputeProvider::~ComputeProvider() = default;

} // namespace skgpu::graphite
