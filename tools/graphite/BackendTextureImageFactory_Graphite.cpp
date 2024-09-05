/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/BackendTextureImageFactory_Graphite.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "src/core/SkAutoPixmapStorage.h"

#include "include/core/SkBitmap.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "tools/graphite/ManagedBackendTexture_Graphite.h"

namespace sk_gpu_test {

using Recorder = skgpu::graphite::Recorder;
sk_sp<SkImage> MakeBackendTextureImage(Recorder* recorder,
                                       const SkPixmap& pixmap,
                                       skgpu::Mipmapped isMipmapped,
                                       Renderable isRenderable,
                                       Origin origin,
                                       Protected isProtected) {
    auto mbet = ManagedGraphiteTexture::MakeFromPixmap(recorder,
                                                       pixmap,
                                                       isMipmapped,
                                                       isRenderable,
                                                       isProtected);
    if (!mbet) {
        return nullptr;
    }

    return SkImages::WrapTexture(recorder,
                                 mbet->texture(),
                                 pixmap.colorType(),
                                 pixmap.alphaType(),
                                 pixmap.refColorSpace(),
                                 origin,
                                 sk_gpu_test::ManagedGraphiteTexture::ImageReleaseProc,
                                 mbet->releaseContext());
}

sk_sp<SkImage> MakeBackendTextureImage(Recorder* recorder,
                                       const SkImageInfo& ii,
                                       SkColor4f color,
                                       skgpu::Mipmapped isMipmapped,
                                       Renderable isRenderable,
                                       Origin origin,
                                       Protected isProtected) {
    if (ii.alphaType() == kOpaque_SkAlphaType) {
        color = color.makeOpaque();
    }

    SkBitmap bitmap;
    bitmap.allocPixels(ii);

    bitmap.eraseColor(color);

    return MakeBackendTextureImage(recorder, bitmap.pixmap(), isMipmapped, isRenderable,
                                   origin, isProtected);
}

}  // namespace sk_gpu_test
