/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"

#include "include/core/SkSamplingOptions.h"
#include "include/gpu/GrTypes.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h" // IWYU pragma: keep
#include "src/gpu/ganesh/SkGr.h"

#include <memory>
#include <tuple>

class GrFragmentProcessor;
class GrRecordingContext;
class SkImage;
class SkMatrix;
enum class GrColorType;
enum class SkTileMode;
struct SkRect;

namespace skgpu::ganesh {
    // Returns a GrSurfaceProxyView representation of the image, if possible. This also returns
    // a color type. This may be different than the image's color type when the image is not
    // texture-backed and the capabilities of the GPU require a data type conversion to put
    // the data in a texture.
    std::tuple<GrSurfaceProxyView, GrColorType> AsView(
            GrRecordingContext*, const SkImage*, GrMipmapped,
            GrImageTexGenPolicy = GrImageTexGenPolicy::kDraw);

    inline std::tuple<GrSurfaceProxyView, GrColorType> AsView(
            GrRecordingContext* ctx, sk_sp<const SkImage> img, GrMipmapped mm,
            GrImageTexGenPolicy policy = GrImageTexGenPolicy::kDraw) {
        return AsView(ctx, img.get(), mm, policy);
    }

    /**
     * Returns a GrFragmentProcessor that can be used with the passed GrRecordingContext to
     * draw the image. SkSamplingOptions indicates the filter and SkTileMode[] indicates the x and
     * y tile modes. The passed matrix is applied to the coordinates before sampling the image.
     * Optional 'subset' indicates whether the tile modes should be applied to a subset of the image
     * Optional 'domain' is a bound on the coordinates of the image that will be required and can be
     * used to optimize the shader if 'subset' is also specified.
     */
    std::unique_ptr<GrFragmentProcessor> AsFragmentProcessor(GrRecordingContext*,
                                                             sk_sp<const SkImage>,
                                                             SkSamplingOptions,
                                                             const SkTileMode[2],
                                                             const SkMatrix&,
                                                             const SkRect* subset = nullptr,
                                                             const SkRect* domain = nullptr);
}
