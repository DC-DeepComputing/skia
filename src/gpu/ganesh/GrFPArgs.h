/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFPArgs_DEFINED
#define GrFPArgs_DEFINED

#include "include/private/SkAssert.h"
#include "src/partition_alloc/raw_ptr_exclusion.h"

class GrColorInfo;
class SkSurfaceProps;
namespace skgpu::ganesh { class SurfaceDrawContext; }

struct GrFPArgs {
    enum class Scope {
        kDefault,
        kRuntimeEffect,
    };

    GrFPArgs(skgpu::ganesh::SurfaceDrawContext* sdc,
             const GrColorInfo* dstColorInfo,
             const SkSurfaceProps& surfaceProps,
             Scope scope)
            : fSurfaceDrawContext(sdc)
            , fDstColorInfo(dstColorInfo)
            , fSurfaceProps(surfaceProps)
            , fScope(scope) {
        SkASSERT(fSurfaceDrawContext);
    }

    // RAW_PTR_EXCLUSION: Ephemeral argument pack for FP construction.
    RAW_PTR_EXCLUSION skgpu::ganesh::SurfaceDrawContext* fSurfaceDrawContext;
    RAW_PTR_EXCLUSION const GrColorInfo* fDstColorInfo;
    RAW_PTR_EXCLUSION const SkSurfaceProps& fSurfaceProps;

    Scope fScope;
};

#endif
