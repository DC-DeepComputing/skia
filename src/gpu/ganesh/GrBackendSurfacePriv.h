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
                                               sk_sp<const GrBackendFormatData> data) {
        return GrBackendFormat(textureType, api, std::move(data));
    }

    static sk_sp<const GrBackendFormatData> GetBackendData(const GrBackendFormat& format) {
         return format.backendData();
    }

    static GrBackendTexture MakeGrBackendTexture(int width,
                                                 int height,
                                                 std::string_view label,
                                                 skgpu::Mipmapped mipped,
                                                 GrBackendApi backend,
                                                 GrTextureType texture,
                                                 sk_sp<GrBackendTextureData> data) {
        return GrBackendTexture(width, height, label, mipped, backend, texture, data);
    }

    static sk_sp<GrBackendTextureData> GetBackendData(GrBackendTexture& tex) {
         return tex.fTextureData;
    }

    static sk_sp<const GrBackendTextureData> GetBackendData(const GrBackendTexture& tex) {
         return tex.fTextureData;
    }

    static GrBackendRenderTarget MakeGrBackendRenderTarget(int width,
                                                           int height,
                                                           int sampleCnt,
                                                           int stencilBits,
                                                           GrBackendApi backend,
                                                           bool framebufferOnly,
                                                           sk_sp<GrBackendRenderTargetData> data) {
    return GrBackendRenderTarget(width, height, sampleCnt, stencilBits, backend, framebufferOnly,
                                data);
    }

    static sk_sp<const GrBackendRenderTargetData> GetBackendData(const GrBackendRenderTarget& rt) {
         return rt.fRTData;
    }
};

#endif
