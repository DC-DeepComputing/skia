/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkImage_Picture.h"

#include "include/core/SkImage.h"
#include "include/gpu/graphite/Image.h"

sk_sp<SkImage> SkImage_Picture::onMakeSubset(skgpu::graphite::Recorder* recorder,
                                             const SkIRect& subset,
                                             RequiredProperties props) const {
    // The Ganesh version doesn't make use of GrDirectContext so we can use it to
    // generate our initial subset.
    sk_sp<SkImage> image = this->onMakeSubset(nullptr, subset);
    if (props.fMipmapped) {
        return SkImages::TextureFromImage(recorder, image, props);
    }
    return image;
}
