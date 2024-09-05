/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/BackendTextureImageFactory.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "src/core/SkAutoPixmapStorage.h"

#ifdef SK_GANESH
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "tools/gpu/ManagedBackendTexture.h"
#endif

namespace sk_gpu_test {
#ifdef SK_GANESH
sk_sp<SkImage> MakeBackendTextureImage(GrDirectContext* dContext,
                                       const SkPixmap& pixmap,
                                       Renderable renderable,
                                       GrSurfaceOrigin origin,
                                       Protected isProtected) {
    auto mbet = ManagedBackendTexture::MakeWithData(dContext,
                                                    pixmap,
                                                    origin,
                                                    renderable,
                                                    isProtected);
    if (!mbet) {
        return nullptr;
    }
    return SkImages::BorrowTextureFrom(dContext,
                                       mbet->texture(),
                                       origin,
                                       pixmap.colorType(),
                                       pixmap.alphaType(),
                                       pixmap.refColorSpace(),
                                       ManagedBackendTexture::ReleaseProc,
                                       mbet->releaseContext());
}

sk_sp<SkImage> MakeBackendTextureImage(GrDirectContext* dContext,
                                       const SkImageInfo& info,
                                       SkColor4f color,
                                       Mipmapped mipmapped,
                                       Renderable renderable,
                                       GrSurfaceOrigin origin,
                                       Protected isProtected) {
    if (info.alphaType() == kOpaque_SkAlphaType) {
        color = color.makeOpaque();
    } else if (info.alphaType() == kPremul_SkAlphaType) {
        auto pmColor = color.premul();
        color = {pmColor.fR, pmColor.fG, pmColor.fB, pmColor.fA};
    }
    auto mbet = ManagedBackendTexture::MakeWithData(dContext,
                                                    info.width(),
                                                    info.height(),
                                                    info.colorType(),
                                                    color,
                                                    mipmapped,
                                                    renderable,
                                                    isProtected);
    if (!mbet) {
        return nullptr;
    }
    return SkImages::BorrowTextureFrom(dContext,
                                       mbet->texture(),
                                       origin,
                                       info.colorType(),
                                       info.alphaType(),
                                       info.refColorSpace(),
                                       ManagedBackendTexture::ReleaseProc,
                                       mbet->releaseContext());
}
#endif  // SK_GANESH

}  // namespace sk_gpu_test
