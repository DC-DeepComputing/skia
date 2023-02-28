/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrTypes.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"

namespace skgpu::ganesh {

struct PinnedData {
    GrSurfaceProxyView fPinnedView;
    int32_t fPinnedCount = 0;
    uint32_t fPinnedUniqueID = SK_InvalidUniqueID;
    uint32_t fPinnedContextID = SK_InvalidUniqueID;
    GrColorType fPinnedColorType = GrColorType::kUnknown;
};

}