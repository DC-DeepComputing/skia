/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrMtlTypesPriv.h"

GrMtlSurfaceInfo GrMtlTextureSpecToSurfaceInfo(const GrMtlTextureSpec& mtlSpec,
                                               const GrSharedSurfaceInfo& sharedInfo) {
    GrMtlSurfaceInfo info;
    // Shared info
    info.fSampleCount = sharedInfo.fSampleCount;
    info.fLevelCount = sharedInfo.fLevelCount;
    info.fProtected = sharedInfo.fProtected;

    // Mtl info
    info.fFormat = mtlSpec.fFormat;
    info.fUsage = mtlSpec.fUsage;
    info.fStorageMode = mtlSpec.fStorageMode

    return info;
}
