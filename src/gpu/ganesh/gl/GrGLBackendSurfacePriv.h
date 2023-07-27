/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLBackendSurfacePriv_DEFINED
#define GrGLBackendSurfacePriv_DEFINED

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/private/gpu/ganesh/GrGLTypesPriv.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"

namespace GrBackendTextures {
// The GrGLTextureInfo must have a valid fFormat.
GrBackendTexture MakeGL(int width,
                        int height,
                        skgpu::Mipmapped,
                        const GrGLTextureInfo& glInfo,
                        sk_sp<GrGLTextureParameters> params,
                        std::string_view label = {});
}

class final GrGLBackendTextureData : public GrBackendTextureData {
public:
    GrGLBackendTextureData(GrGLBackendTextureInfo info): fGLInfo(info) {}

    const GrGLBackendTextureInfo& info() const { return fGLInfo; }

private:
    void cleanup() override;
    void assign(const GrBackendTextureData* that, bool cleanupPrevData) override;
    bool isProtected() const override;
    bool equal(const GrBackendTextureData* that) const override;
    bool isSameTexture(const GrBackendTextureData* that) const override;
    GrBackendFormat getBackendFormat() const override;

    GrGLBackendTextureInfo fGLInfo;
};

#endif