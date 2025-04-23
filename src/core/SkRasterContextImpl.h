/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkRasterContextImpl_DEFINED
#define SkRasterContextImpl_DEFINED

#include "include/core/SkRasterContext.h"
#include "include/core/SkSurfaceProps.h"
#include "src/core/SkResourceCache.h"

class SkRasterContextImpl final : public SkRasterContext {
public:
    SkRasterContextImpl() = default;

    static const SkRasterContextImpl* TODO();
};

#endif
