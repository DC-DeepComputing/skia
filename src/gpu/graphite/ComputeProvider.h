/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ComputeProvider_DEFINED
#define skgpu_graphite_ComputeProvider_DEFINED

#include <memory>

namespace skgpu::graphite {

class Caps;
class GaussianBlurStep;

class ComputeProvider {
public:
    ~ComputeProvider();

    // The step that blurs with compute, or null if blurs should use the shader blur. This is null
    // unless Caps::useComputeBlur() is set and the device supports the step.
    const GaussianBlurStep* gaussianBlur() const { return fGaussianBlur.get(); }

private:
    friend class Context; // for constructor access

    ComputeProvider(const Caps* caps);

    ComputeProvider(const ComputeProvider&) = delete;
    ComputeProvider(ComputeProvider&&) = delete;

    std::unique_ptr<GaussianBlurStep> fGaussianBlur;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ComputeProvider_DEFINED
