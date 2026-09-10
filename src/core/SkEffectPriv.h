/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEffectPriv_DEFINED
#define SkEffectPriv_DEFINED

#include "src/partition_alloc/raw_ptr.h"
#include "src/partition_alloc/raw_ref.h"

#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkRect.h"

class SkArenaAlloc;
class SkColorSpace;
class SkRasterPipeline;
class SkSurfaceProps;

// Passed to effects that will add stages to rasterpipeline
struct SkStageRec {
    bisect_raw_ptr<SkRasterPipeline> fPipeline;
    bisect_raw_ptr<SkArenaAlloc> fAlloc;
    SkColorType fDstColorType;
    bisect_raw_ptr<SkColorSpace> fDstCS;  // may be nullptr
    SkColor4f fPaintColor;
    const bisect_raw_ref<const SkSurfaceProps> fSurfaceProps;
    // The device-space bounding box of the geometry being drawn.
    // An empty value can be used when it is expensive to compute,
    // in which case a heuristic will be used if necessary.
    SkRect fDstBounds;
};

#endif // SkEffectPriv_DEFINED
