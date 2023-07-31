/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendSurfacePriv_DEFINED
#define GrBackendSurfacePriv_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrTypes.h"

class GrBackendSurfacePriv final {
public:
    static GrBackendFormat MakeGrBackendFormat(GrTextureType textureType,
                                               GrBackendApi api,
                                               std::unique_ptr<const GrBackendFormatData> data) {
        return GrBackendFormat(textureType, api, std::move(data));
    }

    static const GrBackendFormatData* GetBackendData(const GrBackendFormat& format) {
         return format.fFormatData.get();
    }

    static GrBackendTexture MakeGrBackendTexture(int width,
                                                 int height,
                                                 std::string_view label,
                                                 skgpu::Mipmapped mipped,
                                                 GrBackendApi backend,
                                                 GrTextureType texture,
                                                 std::unique_ptr<GrBackendTextureData> data) {
        return GrBackendTexture(width, height, label, mipped, backend, texture, std::move(data));
    }

    static GrBackendTextureData* GetBackendData(const GrBackendTexture& tex) {
        return tex.fTextureData.get();
    }

    static GrBackendTextureData* GetBackendData(GrBackendTexture* tex) {
        SkASSERT(tex);
        return tex->fTextureData.get();
    }

    static GrBackendRenderTarget MakeGrBackendRenderTarget(int width,
                                                           int height,
                                                           int sampleCnt,
                                                           int stencilBits,
                                                           GrBackendApi backend,
                                                           bool framebufferOnly,
                                                           std::unique_ptr<GrBackendRenderTargetData> data) {
    return GrBackendRenderTarget(width, height, sampleCnt, stencilBits, backend, framebufferOnly,
                                 std::move(data));
    }

    static const GrBackendRenderTargetData* GetBackendData(const GrBackendRenderTarget& rt) {
         return rt.fRTData.get();
    }
};

#endif
