/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlTypes_DEFINED
#define skgpu_MtlTypes_DEFINED

#include "experimental/graphite/include/GraphiteTypes.h"
#include "include/ports/SkCFObject.h"

namespace skgpu::mtl {

/**
 * Declares typedefs for Metal types used in Ganesh cpp code
 */
using MTLPixelFormat = unsigned int;
using MTLTextureUsage = unsigned int;
using MTLStorageMode = unsigned int;
using MTLHandle = const void*;

///////////////////////////////////////////////////////////////////////////////

#ifdef __APPLE__

#include <TargetConditionals.h>

#if TARGET_OS_SIMULATOR
#define SK_API_AVAILABLE_CA_METAL_LAYER SK_API_AVAILABLE(macos(10.11), ios(13.0))
#else  // TARGET_OS_SIMULATOR
#define SK_API_AVAILABLE_CA_METAL_LAYER SK_API_AVAILABLE(macos(10.11), ios(8.0))
#endif  // TARGET_OS_SIMULATOR

/**
 * Types for interacting with Metal resources created externally to Skia.
 * This is used by BackendObjects.
 */
struct MtlTextureInfo {
public:
    MtlTextureInfo() {}

    sk_cfp<MTLHandle> fTexture;

    bool operator==(const MtlTextureInfo& that) const {
        return fTexture == that.fTexture;
    }
};

struct MtlSurfaceInfo {
    uint32_t fSampleCount = 1;
    uint32_t fLevelCount = 0;
    Protected fProtected = Protected::kNo;

    // Since we aren't in an Obj-C header we can't directly use Mtl types here. Each of these can
    // cast to their mapped Mtl types list below.
    MTLPixelFormat fFormat = 0;       // MTLPixelFormat fFormat = MTLPixelFormatInvalid;
    MTLTextureUsage fUsage = 0;       // MTLTextureUsage fUsage = MTLTextureUsageUnknown;
    MTLStorageMode fStorageMode = 0;  // MTLStorageMode fStorageMode = MTLStorageModeShared;
};

} // namespace skgpu

#endif // __APPLE__

#endif // skgpu_MtlTypes_DEFINED
