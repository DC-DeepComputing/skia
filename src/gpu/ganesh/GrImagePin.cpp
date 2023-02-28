/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrImagePin.h"

#include "include/android/SkImageAndroid.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Raster.h"

#include <cstdlib>
#include <cstdint>
#include <tuple>

class SkImage;

namespace skgpu::ganesh {

bool PinAsTexture(GrRecordingContext* rContext, SkImage* img) {
    auto ib = as_IB(img);
    if (ib->type() != ImageType::kRaster) {
        // Cannot pin non-raster images
        return false;
    }
    auto raster = static_cast<SkImage_Raster*>(ib);
    PinnedData* data = raster->pinnedData();
    if (data == nullptr) {
        data = new(PinnedData);
        std::tie(data->fPinnedView, data->fPinnedColorType) =
                GrMakeCachedBitmapProxyView(rContext,
                                            raster->bitmap(),
                                            /*label=*/"ganesh_PinAsTexture",
                                            GrMipmapped::kNo);
        if (!data->fPinnedView) {
            return false;
        }
        data->fPinnedUniqueID = raster->bitmap().getGenerationID();
        data->fPinnedContextID = rContext->priv().contextID();
        raster->setPinnedData(data);
    } else {
        SkASSERT(data->fPinnedCount > 0);
        SkASSERT(data->fPinnedUniqueID != 0);
        if (rContext->priv().contextID() != data->fPinnedContextID) {
            return false;
        }
    }
    // Note: we only increment if the texture was successfully pinned
    data->fPinnedCount++;
    return true;
}

void UnpinTexture(GrRecordingContext*, SkImage* img) {
    auto ib = as_IB(img);
    if (ib->type() != ImageType::kRaster) {
        // Cannot unpin non-raster images
        return;
    }
    auto raster = static_cast<SkImage_Raster*>(ib);
    PinnedData* data = raster->pinnedData();
    if (data == nullptr) {
        SkASSERT(false);
        return;
    }

    SkASSERT(data->fPinnedCount > 0);
    SkASSERT(data->fPinnedUniqueID != 0);
    // It would be good to check rContext->priv().contextID() != fPinnedContextID
    // but Android used to (maybe still does) call Unpin with a freed context ptr

    data->fPinnedCount--;
    if (data->fPinnedCount <= 0) {
        free(data);
        raster->setPinnedData(nullptr);
    }
}

} // namespace skgpu::ganesh
