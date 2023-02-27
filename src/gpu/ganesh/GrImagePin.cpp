#include "src/image/SkImage_Raster.h"

#include "include/core/SkBitmap.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/SkGr.h"

namespace skgpu::ganesh {

struct PinnedData {
    GrSurfaceProxyView fPinnedView;
    int32_t fPinnedCount = 0;
    uint32_t fPinnedUniqueID = SK_InvalidUniqueID;
    uint32_t fPinnedContextID = SK_InvalidUniqueID;
    GrColorType fPinnedColorType = GrColorType::kUnknown;
};

bool PinAsTexture(GrRecordingContext*, const SkImage*) {
    return false;
}

/**
 *  The balancing call to a successful invocation of SkImage_pinAsTexture. When a balanced
 *  number of calls have been made, then the "pinned" texture is free to be purged, etc. This
 *  also means that a subsequent "pin" call will look at the original content again, and if
 *  its uniqueID/generationID has changed, then a newer texture will be uploaded/pinned.
 *
 *  The context passed to unpin must match the one passed to pin.
 */
void UnpinTexture(GrRecordingContext*, const SkImage*) {

}
}