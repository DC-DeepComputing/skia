/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkRasterContext_Base_DEFINED
#define SkRasterContext_Base_DEFINED

#include "include/core/SkRasterContext.h"
#include "include/core/SkSurfaceProps.h"
#include "src/core/SkResourceCache.h"

class SkRasterContext_Base final : public SkRasterContext {
public:
    SkRasterContext_Base() = default;

    static const SkRasterContext_Base* TODO();

private:
};

#endif
