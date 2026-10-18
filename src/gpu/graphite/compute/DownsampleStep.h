/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_compute_DownsampleStep_DEFINED
#define skgpu_graphite_compute_DownsampleStep_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "src/gpu/graphite/compute/ComputeTuning/DownsampleTuning.h"

#include <string>
#include <tuple>

enum class SkTileMode;
struct SkIPoint;
struct SkIRect;
struct SkISize;

namespace skgpu::graphite {

class Caps;
class Image;
class Recorder;

/**
 * DownsampleStep downscales a source by power-of-two factors in a single dispatch, for the sigmas
 * above GaussianBlurStep::kMaxSigma. GaussianBlurStep then blurs its output in a second dispatch of
 * the same batch (see ComputeBlurAlgorithm in TextureUtils.cpp). It reads the source through a
 * bilinear sampler, with each sample placed between texels so that it averages up to 2 of them
 * along each axis.
 *
 * The samples of each output texel are split across a cluster of consecutive lanes of a subgroup.
 * Each lane sums its share in registers, and the cluster then adds those up with subgroup
 * shuffles. A cluster has at most 1 << DownsampleTuning::fMaxClusterSizeLog lanes, which assumes
 * that subgroups have at least that many. In a smaller subgroup, the first lane of each cluster
 * takes all of its samples instead.
 *
 * The step's DownsampleTuning is composed into the SkSL when the step is created, so that it can
 * differ per GPU architecture without changing the results.
 *
 * It has GaussianBlurStep's assumptions about the source, and these:
 *  - The factors are powers of two up to kMaxFactor along each axis.
 *  - The tile mode maps each pair of source texels that a sample averages to the same texel or to
 *    adjacent ones. That holds for every tile mode except repeat, where it only holds if the pairs
 *    align with the edges of the source rect.
 *  - Subgroups are full, since lanes are indexed through their subgroup. That holds for a
 *    workgroup that's a multiple of the subgroup size, which is a power of two on every backend.
 */
class DownsampleStep final : public ComputeStep {
public:
    // The largest downscale factor along each axis.
    static constexpr int kMaxFactor = 64;

    // Returns whether the step can run with `tuning` on a device with `caps`. If it can't because
    // `tuning` is invalid or exceeds the device's compute limits (rather than because the device
    // can't run the step at all), `error` (if not null) is set to the reason.
    static bool IsSupported(const Caps*,
                            const DownsampleTuning& tuning = {},
                            std::string* error = nullptr);

    // `tuning` must be valid, which IsSupported() checks.
    explicit DownsampleStep(const DownsampleTuning& tuning = {});
    ~DownsampleStep() override;

    const DownsampleTuning& tuning() const { return fTuning; }

    // Queues a downsample on `recorder`'s ComputeScheduler. Texel (u, v) of the output is the
    // average of the source texels at `gridOrigin` + [i*fx, (i+1)*fx) x [j*fy, (j+1)*fy), with the
    // tile mode applied to them against `srcRect`, where (i, j) is `dstRect.topLeft()` + (u, v) and
    // {fx, fy} is `factors`. `srcRect` and `gridOrigin` are relative to the texture of `src`. The
    // returned image is approx-fit, with the downsample in its top-left `dstRect.size()` pixels.
    // Returns null if the step doesn't support the downsample (see above).
    sk_sp<Image> downsample(Recorder*,
                            SkISize factors,
                            const Image& src,
                            const SkIRect& srcRect,
                            SkTileMode,
                            SkIPoint gridOrigin,
                            const SkIRect& dstRect) const;

    std::string computeSkSL() const override;

    // Only called for the output, which is always assigned by ComputeScheduler.
    std::tuple<SkISize, SkColorType> calculateTextureParameters(int resourceIndex,
                                                                const ResourceDesc&) const override;

    // The source is sampled bilinearly, between texels that are within the source rect.
    SamplerDesc calculateSamplerParameters(int resourceIndex, const ResourceDesc&) const override;

private:
    const DownsampleTuning fTuning;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_compute_DownsampleStep_DEFINED
