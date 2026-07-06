/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/partition_alloc/raw_ref.h"

#ifndef GrFPArgs_DEFINED

#define GrFPArgs_DEFINED

#include "src/partition_alloc/raw_ptr.h"

#include "include/private/SkAssert.h"

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

    raw_ptr<skgpu::ganesh::SurfaceDrawContext> fSurfaceDrawContext;

    raw_ptr<const GrColorInfo> fDstColorInfo;

    const raw_ref<const SkSurfaceProps> fSurfaceProps;

    Scope fScope;
};

#endif
