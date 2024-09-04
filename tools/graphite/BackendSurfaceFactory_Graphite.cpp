/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/BackendSurfaceFactory_Graphite.h"

#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "tools/gpu/ManagedBackendTexture.h"

#include "include/gpu/graphite/Surface.h"
#if defined(SK_DAWN)
#include "include/gpu/graphite/dawn/DawnTypes.h"
#include "src/gpu/graphite/dawn/DawnGraphiteTypesPriv.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE
#endif

namespace sk_gpu_test {

sk_sp<SkSurface> MakeBackendTextureSurface(skgpu::graphite::Recorder* recorder,
                                           const SkImageInfo& ii,
                                           skgpu::Mipmapped mipmapped,
                                           skgpu::Protected isProtected,
                                           const SkSurfaceProps* props) {
    if (ii.alphaType() == kUnpremul_SkAlphaType) {
        return nullptr;
    }
    sk_sp<ManagedGraphiteTexture> mbet = ManagedGraphiteTexture::MakeUnInit(recorder,
                                                                            ii,
                                                                            mipmapped,
                                                                            skgpu::Renderable::kYes,
                                                                            isProtected);
    if (!mbet) {
        return nullptr;
    }
    return SkSurfaces::WrapBackendTexture(recorder,
                                          mbet->texture(),
                                          ii.colorType(),
                                          ii.refColorSpace(),
                                          props,
                                          ManagedGraphiteTexture::ReleaseProc,
                                          mbet->releaseContext());
}

sk_sp<SkSurface> MakeBackendTextureViewSurface(skgpu::graphite::Recorder* recorder,
                                               const SkImageInfo& ii,
                                               skgpu::Mipmapped mipmapped,
                                               skgpu::Protected isProtected,
                                               const SkSurfaceProps* props) {
#ifdef SK_DAWN
    if (recorder->backend() != skgpu::BackendApi::kDawn) {
        return nullptr;
    }

    if (ii.alphaType() == kUnpremul_SkAlphaType) {
        return nullptr;
    }

    auto mbet = ManagedGraphiteTexture::MakeUnInit(recorder,
                                                   ii,
                                                   mipmapped,
                                                   skgpu::Renderable::kYes,
                                                   isProtected);
    if (!mbet) {
        return nullptr;
    }

    wgpu::Texture texture(skgpu::graphite::BackendTextures::GetDawnTexturePtr(mbet->texture()));
    SkASSERT(texture);

    wgpu::TextureView view = texture.CreateView();
    SkASSERT(view);

    skgpu::graphite::DawnTextureInfo textureInfo;
    textureInfo.fAspect      = wgpu::TextureAspect::All;
    textureInfo.fFormat      = texture.GetFormat();
    textureInfo.fMipmapped   = mipmapped;
    textureInfo.fSampleCount = texture.GetSampleCount();
    textureInfo.fUsage       = texture.GetUsage();

    skgpu::graphite::BackendTexture betFromView =
            skgpu::graphite::BackendTextures::MakeDawn(ii.dimensions(), textureInfo, view.Get());

    auto release = [](void* ctx) { static_cast<ManagedGraphiteTexture*>(ctx)->unref(); };

    return SkSurfaces::WrapBackendTexture(recorder,
                                          betFromView,
                                          ii.colorType(),
                                          ii.refColorSpace(),
                                          props,
                                          release,
                                          mbet.release());
#endif // SK_DAWN
    return nullptr;
}

}  // namespace sk_gpu_test
