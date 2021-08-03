/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/SurfaceFillContext.h"

#include "src/gpu/effects/GrMatrixEffect.h"

namespace skgpu {

std::unique_ptr<SurfaceFillContext> SurfaceFillContext::Make(GrRecordingContext* rContext,
                                                             GrImageInfo,
                                                             SkBackingFit,
                                                             int sampleCount,
                                                             GrMipmapped,
                                                             GrProtected,
                                                             GrSurfaceOrigin,
                                                             SkBudgeted) {
    return nullptr;
}

std::unique_ptr<SurfaceFillContext> SurfaceFillContext::MakeWithFallback(
        GrRecordingContext* rContext,
        GrImageInfo,
        SkBackingFit,
        int sampleCount,
        GrMipmapped,
        GrProtected,
        GrSurfaceOrigin,
        SkBudgeted) {
    return nullptr;
}

std::unique_ptr<SurfaceFillContext> SurfaceFillContext::Make(GrRecordingContext* rContext,
                                                             SkAlphaType,
                                                             sk_sp<SkColorSpace>,
                                                             SkISize dimensions,
                                                             SkBackingFit,
                                                             const GrBackendFormat&,
                                                             int sampleCount,
                                                             GrMipmapped,
                                                             GrProtected,
                                                             GrSwizzle readSwizzle,
                                                             GrSwizzle writeSwizzle,
                                                             GrSurfaceOrigin,
                                                             SkBudgeted) {
    return nullptr;
}

std::unique_ptr<SurfaceFillContext> SurfaceFillContext::MakeFromBackendTexture(
        GrRecordingContext* rContext,
        GrColorInfo,
        const GrBackendTexture&,
        int sampleCount,
        GrSurfaceOrigin,
        sk_sp<GrRefCntedCallback> releaseHelper) {
    return nullptr;
}

void SurfaceFillContext::fillRectWithFP(const SkIRect& dstRect,
                                        const SkMatrix& localMatrix,
                                        std::unique_ptr<GrFragmentProcessor> fp) {
    fp = GrMatrixEffect::Make(localMatrix, std::move(fp));
    this->fillRectWithFP(dstRect, std::move(fp));
}

} // namespace skgpu
