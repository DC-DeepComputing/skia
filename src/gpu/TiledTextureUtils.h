/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_TiledTextureUtils_DEFINED
#define skgpu_TiledTextureUtils_DEFINED

#include "include/core/SkTypes.h"

class GrClip;
class GrRecordingContext;
struct SkIRect;
struct SkISize;
class SkMatrix;
struct SkRect;

namespace skgpu {

bool ShouldTileImage(GrRecordingContext* context,
                     SkISize rtSize,
                     const GrClip* clip,
                     uint32_t /* imageID */,
                     const SkISize& imageSize,
                     const SkMatrix& ctm,
                     const SkMatrix& srcToDst,
                     const SkRect* src,
                     int maxTileSize,
                     int* tileSize,
                     SkIRect* clippedSubset);

} // namespace skgpu

#endif // skgpu_TiledTextureUtils_DEFINED
