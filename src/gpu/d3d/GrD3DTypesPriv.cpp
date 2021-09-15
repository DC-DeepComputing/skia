/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DTypesPriv.h"

GrD3DSurfaceInfo GrD3DTextureResourceSpecToSurfaceInfo(const GrD3DTextureResourceSpec& d3dSpec,
                                                       const GrSharedSurfaceInfo& sharedInfo) {
    GrD3DSurfaceInfo info;
    // Shared info
    info.fSampleCount = sharedInfo.fSampleCount;
    info.fLevelCount = sharedInfo.fLevelCount;
    info.fProtected = sharedInfo.fProtected;

    // D3D info
    info.fFormat = d3dSpec.fFormat;
    info.fSampleQualityPattern = d3dSpec.fSampleQualityPattern;

    return info;
}
