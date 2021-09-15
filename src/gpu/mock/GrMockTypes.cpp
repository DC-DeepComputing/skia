/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/mock/GrMockTypes.h"

#include "include/gpu/GrBackendSurface.h"

GrBackendFormat GrMockRenderTargetInfo::getBackendFormat() const {
    return GrBackendFormat::MakeMock(fColorType, SkImage::CompressionType::kNone);
}

GrBackendFormat GrMockTextureInfo::getBackendFormat() const {
    return GrBackendFormat::MakeMock(fColorType, fCompressionType);
}

GrMockSurfaceInfo GrMockTextureSpecToSurfaceInfo(const GrMockTextureSpec& mockSpec,
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
