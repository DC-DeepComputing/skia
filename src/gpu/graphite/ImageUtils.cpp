/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ImageUtils.h"

#include "include/gpu/graphite/ImageProvider.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/image/SkImage_Base.h"



namespace skgpu::graphite {

std::pair<sk_sp<SkImage>, SkSamplingOptions> GetGraphiteBacked(Recorder* recorder,
                                                               const SkImage* imageIn,
                                                               SkSamplingOptions sampling) {
    skgpu::graphite::Mipmapped mipmapped = (sampling.mipmap != SkMipmapMode::kNone)
                                               ? skgpu::graphite::Mipmapped::kYes
                                               : skgpu::graphite::Mipmapped::kNo;

    if (imageIn->dimensions().area() <= 1 && mipmapped == skgpu::graphite::Mipmapped::kYes) {
        mipmapped = skgpu::graphite::Mipmapped::kNo;
        sampling = SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
    }

    sk_sp<SkImage> result = imageIn->makeTextureImage(recorder, { mipmapped });
    if (!result) {
        return {};
    }

    // If the Graphite-backed image doesn't have the required mipmaps we downgrade the sampling
    if (mipmapped == skgpu::graphite::Mipmapped::kYes && !result->hasMipmaps()) {
        mipmapped = skgpu::graphite::Mipmapped::kNo;
        sampling = SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
    }

    return { result, sampling };
}

} // namespace skgpu::graphite
