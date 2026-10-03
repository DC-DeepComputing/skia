/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_compute_GaussianBlurStep_DEFINED
#define skgpu_graphite_compute_GaussianBlurStep_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/core/SkBlurEngine.h"
#include "src/gpu/graphite/compute/ComputeStep.h"

#include <string>
#include <tuple>

enum class SkTileMode;
struct SkIRect;
struct SkISize;
struct SkSize;

namespace skgpu::graphite {

class Caps;
class Image;
class Recorder;

/**
 * GaussianBlurStep performs a separable Gaussian blur in a single dispatch. Each workgroup loads its
 * kTileSize x kTileSize tile of the source, plus a halo of the blur radius, into workgroup memory.
 * It runs the horizontal pass over that into a second workgroup array, and then the vertical pass
 * into the output. Unlike the shader blur, it needs no intermediate texture or render passes, and
 * its dispatches are batched through the Recorder's ComputeScheduler.
 *
 * It implements SkBlurEngine::Algorithm::blur() (see ComputeBlurAlgorithm in TextureUtils.cpp)
 * under these assumptions:
 *  - The maximum sigma is the shader blur's (SkShaderBlurAlgorithm::kMaxLinearSigma), so callers
 *    rescale the same way whichever algorithm blurs. Larger sigmas would avoid the rescale passes,
 *    but whether that's a win depends on the downscaling tradeoffs, which haven't been measured.
 *  - Only premultiplied (or opaque) RGBA8 sources are supported. The output must keep the source's
 *    color type, and RGBA8 is the only 8-bit color format that every backend can write to as a
 *    storage texture: BGRA8 needs bgra8unorm-storage on Dawn, A8 needs R8 storage, and F16 needs a
 *    smaller tile for its workgroup arrays to fit in Caps::kMinComputeWorkgroupStorageSize.
 *    Callers use the shader blur for other sources.
 *  - The source's read swizzle is RGBA, since ComputeScheduler binds its texture directly.
 */
class GaussianBlurStep final : public ComputeStep {
public:
    // The output tile of each workgroup is kTileSize x kTileSize.
    static constexpr int kTileSize = 16;
    static constexpr float kMaxSigma = SkShaderBlurAlgorithm::kMaxLinearSigma;
    // SkBlurEngine::SigmaToRadius(kMaxSigma), which isn't constexpr.
    static constexpr int kMaxRadius = 12;

    // Returns whether the step can run on a device with `caps`.
    static bool IsSupported(const Caps*);

    GaussianBlurStep();
    ~GaussianBlurStep() override;

    // Queues a blur on `recorder`'s ComputeScheduler, with the semantics of
    // SkBlurEngine::Algorithm::blur(), except that `srcRect` and `dstRect` are relative to the
    // texture of `src`. The returned image is approx-fit, with the blur in its top-left
    // `dstRect.size()` pixels. Returns null if the step doesn't support the blur (see above), in
    // which case the caller should use the shader blur instead.
    sk_sp<Image> blur(Recorder*,
                      SkSize sigma,
                      const Image& src,
                      const SkIRect& srcRect,
                      SkTileMode,
                      const SkIRect& dstRect) const;

    std::string computeSkSL() const override;

    // Only called for the output, which is always assigned by ComputeScheduler.
    std::tuple<SkISize, SkColorType> calculateTextureParameters(int resourceIndex,
                                                                const ResourceDesc&) const override;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_compute_GaussianBlurStep_DEFINED
