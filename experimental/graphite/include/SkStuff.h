/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStuff_DEFINED
#define SkStuff_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"

class SkSurface;
class SkSurfaceProps;

namespace skgpu {
    class BackendTexture;
    class Recorder;
}

// TODO: Should be in SkSurface.h
sk_sp<SkSurface> MakeGraphite(sk_sp<skgpu::Recorder>, const SkImageInfo&);

sk_sp<SkSurface> MakeGraphiteFromBackendTexture(sk_sp<skgpu::Recorder>,
                                                const skgpu::BackendTexture&,
                                                SkColorType colorType,
                                                sk_sp<SkColorSpace> colorSpace,
                                                const SkSurfaceProps* props);

#endif // SkStuff_DEFINED
