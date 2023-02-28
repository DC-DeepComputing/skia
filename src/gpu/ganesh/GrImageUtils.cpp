/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrImageUtils.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkMath.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkImagePriv.h"
#include "src/image/SkImage_Base.h"

namespace skgpu::ganesh {

    std::tuple<GrSurfaceProxyView, GrColorType> AsView(GrRecordingContext* rContext,
                                                       const SkImage* img,
                                                       GrMipmapped mipmapped,
                                                       GrImageTexGenPolicy policy) {
        return {}; // FIXME(kjlubick)
    }

    /**
     * Returns a GrFragmentProcessor that can be used with the passed GrRecordingContext to
     * draw the image. SkSamplingOptions indicates the filter and SkTileMode[] indicates the x and
     * y tile modes. The passed matrix is applied to the coordinates before sampling the image.
     * Optional 'subset' indicates whether the tile modes should be applied to a subset of the image
     * Optional 'domain' is a bound on the coordinates of the image that will be required and can be
     * used to optimize the shader if 'subset' is also specified.
     */
    std::unique_ptr<GrFragmentProcessor> AsFragmentProcessor(GrRecordingContext* rContext,
                                                             sk_sp<const SkImage> img,
                                                             SkSamplingOptions,
                                                             const SkTileMode tm[2],
                                                             const SkMatrix& m,
                                                             const SkRect* subset,
                                                             const SkRect* domain) {
        return nullptr;
    };
}
