/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrMockBackendSurface_DEFINED
#define GrMockBackendSurface_DEFINED

#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/mock/GrMockTypes.h"
#include "include/private/base/SkAPI.h"

#include <string_view>

class GrBackendFormat;
class GrBackendTexture;
class GrBackendRenderTarget;
enum class SkTextureCompressionType;

namespace GrBackendFormats {

SK_API GrBackendFormat MakeMock(GrColorType colorType,
                                SkTextureCompressionType compression,
                                bool isStencilFormat = false);

SK_API GrColorType AsMockColorType(const GrBackendFormat&);
SK_API SkTextureCompressionType AsMockCompressionType(const GrBackendFormat&);
SK_API bool IsMockStencilFormat(const GrBackendFormat&);

}  // namespace GrBackendFormats

namespace GrBackendTextures {

SK_API GrBackendTexture MakeMock(int width,
                                 int height,
                                 skgpu::Mipmapped,
                                 const GrMockTextureInfo& mockInfo,
                                 std::string_view label = {});

SK_API bool GetMockTextureInfo(const GrBackendTexture&, GrMockTextureInfo*);

}  // namespace GrBackendTextures

namespace GrBackendRenderTargets {

SK_API GrBackendRenderTarget MakeMock(int width,
                                      int height,
                                      int sampleCnt,
                                      int stencilBits,
                                      const GrMockRenderTargetInfo& mockInfo);

SK_API bool GetMockRenderTargetInfo(const GrBackendRenderTarget&, GrMockRenderTargetInfo*);

}  // namespace GrBackendRenderTargets

#endif
