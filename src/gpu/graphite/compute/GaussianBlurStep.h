/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_compute_GaussianBlurStep_DEFINED
#define skgpu_graphite_compute_GaussianBlurStep_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"
#include "src/core/SkBlurEngine.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "src/gpu/graphite/compute/ComputeTuning/GaussianBlurTuning.h"

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
 * GaussianBlurStep performs a separable Gaussian blur in a single dispatch. Each workgroup blurs one
 * tile of the output, whose size is set by the step's GaussianBlurTuning. It runs the vertical pass
 * first, streaming the columns of the tile and its horizontal halo from the source into sums held
 * in registers, and stores those to workgroup memory. After one barrier, it runs the horizontal
 * pass from workgroup memory into the output. Unlike the shader blur, it needs no intermediate
 * texture or render passes, and its dispatches are batched through the Recorder's
 * ComputeScheduler.
 *
 * The tuning is composed into the SkSL when the step is created, so that it can differ per GPU
 * architecture without changing the results.
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
    static constexpr float kMaxSigma = SkShaderBlurAlgorithm::kMaxLinearSigma;
    // SkBlurEngine::SigmaToRadius(kMaxSigma), which isn't constexpr.
    static constexpr int kMaxRadius = 12;

    // Returns whether the step can run with `tuning` on a device with `caps`. If it can't because
    // `tuning` is invalid or exceeds the device's compute limits (rather than because the device
    // can't run the step at all), `error` (if not null) is set to the reason.
    static bool IsSupported(const Caps*,
                            const GaussianBlurTuning& tuning = {},
                            std::string* error = nullptr);

    // `tuning` must be valid, which IsSupported() checks.
    explicit GaussianBlurStep(const GaussianBlurTuning& tuning = {});
    ~GaussianBlurStep() override;

    const GaussianBlurTuning& tuning() const { return fTuning; }

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

private:
    // Returns the radius of the smallest bucket that covers `radius`.
    int bucketRadius(int radius) const;
    // Returns the index of entry `i` of a row of vertical sums, with the tuning's padding.
    int paddedIndex(int i) const;

    void appendBlurVertical(std::string* sksl, int radius) const;
    void appendBlurHorizontal(std::string* sksl, int radius) const;
    void appendBucketCalls(std::string* sksl,
                           const char* radius,
                           const char* function,
                           const char* args) const;

    const GaussianBlurTuning fTuning;
    // Derived from fTuning.
    const int fTileWidth;
    const int fTileHeight;
    // The number of blocks of rows that the vertical pass splits the tile into.
    const int fVerticalBlocks;
    // The number of invocations that blur each row in the horizontal pass.
    const int fHorizontalStrips;
    const int fWorkgroupSize;
    // The entries between consecutive rows of vertical sums in workgroup memory.
    const int fRowStride;
    skia_private::STArray<kMaxRadius + 1, int> fBucketRadii;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_compute_GaussianBlurStep_DEFINED
