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
             , fCompressionType(SkImage::CompressionType::kNone) {}
    GrMockTextureSpec(const GrMockSurfaceInfo& info)
            : fColorType(info.fColorType)
            , fCompressionType(info.fCompressionType) {}

    GrColorType fColorType = GrColorType::kUnknown;
    SkImage::CompressionType fCompressionType = SkImage::CompressionType::kNone;
};

static GrMockSurfaceInfo GrMockTextureSpecToSurfaceInfo(const GrMockTextureSpec& mockSpec,
                                                        const GrSharedSurfaceInfo& sharedInfo) {
    GrMockSurfaceInfo info;
    // Shared info
    info.fSampleCount = sharedInfo.fSampleCount;
    info.fLevelCount = sharedInfo.fLevelCount;
    info.fProtected = sharedInfo.fProtected;

    // Mock info
    info.fColorType = mockSpec.fColorType;
    info.fCompressionType = mockSpec.fCompressionType;

    return info;
}

#endif

