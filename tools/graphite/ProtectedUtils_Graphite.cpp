/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "tools/gpu/BackendSurfaceFactory.h"
#include "tools/gpu/BackendTextureImageFactory.h"
#include "tools/gpu/ProtectedUtils.h"
//#include "include/gpu/ganesh/SkImageGanesh.h"
//#include "include/gpu/ganesh/SkSurfaceGanesh.h"
//#include "src/gpu/ganesh/GrDirectContextPriv.h"

//#include "src/gpu/graphite/Image_Graphite.h"
//#include "src/gpu/graphite/Surface_Graphite.h"
//#include "src/gpu/graphite/Texture.h"
//#include "src/image/SkImage_Base.h"
//#include "src/image/SkSurface_Base.h"

using namespace skgpu;
using namespace skgpu::graphite;

namespace ProtectedUtils {

sk_sp<SkSurface> CreateProtectedSkSurface(Recorder* recorder,
                                          SkISize size,
                                          Protected isProtected) {
    SkImageInfo ii = SkImageInfo::Make(size, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkSurfaceProps surfaceProps = SkSurfaceProps(0, kRGB_H_SkPixelGeometry);

    sk_sp<SkSurface> surface = sk_gpu_test::MakeBackendTextureSurface(recorder,
                                                                      ii,
                                                                      Origin::kTopLeft,
                                                                      /* sampleCount=*/ 1,
                                                                      Mipmapped::kNo,
                                                                      isProtected,
                                                                      &surfaceProps);
    if (!surface) {
        SK_ABORT("Could not create %s surface.",
                 isProtected == Protected::kYes ? "protected" : "unprotected");
        return nullptr;
    }

    SkCanvas* canvas = surface->getCanvas();

    canvas->clear(SkColors::kBlue);

    return surface;
}

sk_sp<SkImage> CreateProtectedSkImage(Recorder* recorder,
                                      SkISize size,
                                      SkColor4f color,
                                      Protected isProtected) {
    SkImageInfo ii = SkImageInfo::Make(size, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkImage> image = sk_gpu_test::MakeBackendTextureImage2(recorder,
                                                                ii,
                                                                color,
                                                                Mipmapped::kNo,
                                                                GrRenderable::kNo,
                                                                Origin::kTopLeft,
                                                                isProtected);
    if (!image) {
        SK_ABORT("Could not create %s image.",
                 isProtected == Protected::kYes ? "protected" : "unprotected");
        return nullptr;
    }

    return image;
}

}  // namespace ProtectedUtils
