/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockTypesPriv_DEFINED
#define GrMockTypesPriv_DEFINED

#include "include/gpu/mock/GrMockTypes.h"

struct GrMockTextureSpec {
    GrMockTextureSpec()
            : fColorType(GrColorType::kUnknown)
             , fCompressionType(sk_image_factory::CompressionType::kNone) {}
    GrMockTextureSpec(const GrMockSurfaceInfo& info)
            : fColorType(info.fColorType)
            , fCompressionType(info.fCompressionType) {}

    GrColorType fColorType = GrColorType::kUnknown;
    sk_image_factory::CompressionType fCompressionType = sk_image_factory::CompressionType::kNone;
};

GrMockSurfaceInfo GrMockTextureSpecToSurfaceInfo(const GrMockTextureSpec& mockSpec,
                                                 uint32_t sampleCount,
                                                 uint32_t levelCount,
                                                 GrProtected isProtected);

#endif

