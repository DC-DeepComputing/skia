/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrMtlBackendSurface_DEFINED
#define GrMtlBackendSurface_DEFINED

#include "include/private/base/SkAPI.h"
#include "include/gpu/ganesh/mtl/GrMtlTypes.h"

#include <string_view>

class GrBackendFormat;
class GrBackendTexture;
class GrBackendRenderTarget;

namespace GrBackendFormats {
SK_API GrBackendFormat MakeMetal(GrMTLPixelFormat format);

SK_API GrMTLPixelFormat AsMetalFormat(const GrBackendFormat&);

}  // namespace GrBackendFormats

namespace GrBackendTextures {
SK_API GrBackendTexture MakeMetal(int width, int height, skgpu::Mipmapped,
                                  const GrMtlTextureInfo& mtlInfo, std::string_view label = {});

SK_API bool GetMetalTextureInfo(const GrBackendTexture&, GrMtlTextureInfo*);

}  // namespace GrBackendTextures

namespace GrBackendRenderTargets {
SK_API GrBackendRenderTarget MakeMetal(int width, int height, const GrMtlTextureInfo& mtlInfo);

SK_API bool GetMetalTextureInfo(const GrBackendRenderTarget&, GrMtlTextureInfo*);

}  // namespace GrBackendRenderTargets

#endif