/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ComputeProvider.h"

#include "include/private/SkLog.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/compute/ComputeTuning/ComputeTuning.h"
#include "src/gpu/graphite/compute/DownsampleStep.h"
#include "src/gpu/graphite/compute/GaussianBlurStep.h"

#include <string>

namespace skgpu::graphite {

ComputeProvider::ComputeProvider(const Caps* caps) {
    // The compute blur is opt-in until it's been benchmarked against the shader blur, which also
    // decides whether small blurs should keep using the shader blur.
    if (!caps->useComputeBlur()) {
        return;
    }
    // The steps compose their tunings into their SkSL here, once per Context. A tuning that's
    // invalid or exceeds the device's limits is logged, since it can only come from a test option.
    const ComputeTuning& tuning = caps->computeTuning();
    std::string error;
    if (GaussianBlurStep::IsSupported(caps, tuning.fGaussianBlur, &error)) {
        fGaussianBlur = std::make_unique<GaussianBlurStep>(tuning.fGaussianBlur);
    } else if (!error.empty()) {
        SKIA_LOG_E("Not using GaussianBlurStep: %s", error.c_str());
    }
    error.clear();
    if (DownsampleStep::IsSupported(caps, tuning.fDownsample, &error)) {
        fDownsample = std::make_unique<DownsampleStep>(tuning.fDownsample);
    } else if (!error.empty()) {
        SKIA_LOG_E("Not using DownsampleStep: %s", error.c_str());
    }
}

ComputeProvider::~ComputeProvider() = default;

} // namespace skgpu::graphite
