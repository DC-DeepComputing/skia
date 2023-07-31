/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLBackendSurfacePriv_DEFINED
#define GrGLBackendSurfacePriv_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/private/gpu/ganesh/GrGLTypesPriv.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"

#include <string_view>

struct GrGLTextureInfo;
namespace skgpu { enum class Mipmapped : bool; }

namespace GrBackendTextures {
// The GrGLTextureInfo must have a valid fFormat.
GrBackendTexture MakeGL(int width,
                        int height,
                        skgpu::Mipmapped,
                        const GrGLTextureInfo& glInfo,
                        sk_sp<GrGLTextureParameters> params,
                        std::string_view label = {});
}  // namespace GrBackendTextures

class GrGLBackendTextureData final : public GrBackendTextureData {
public:
    GrGLBackendTextureData(const GrGLTextureInfo& info, GrGLTextureParameters* params);

    const GrGLBackendTextureInfo& info() const { return fGLInfo; }

private:
    void cleanup() override;
    GrBackendTextureData* copy() const override;
    bool isProtected() const override;
    bool equal(const GrBackendTextureData* that) const override;
    bool isSameTexture(const GrBackendTextureData* that) const override;
    GrBackendFormat getBackendFormat() const override;

    GrGLBackendTextureInfo fGLInfo;
};

#endif
