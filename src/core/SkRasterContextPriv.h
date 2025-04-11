/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterContextPriv_DEFINED
#define SkRasterContextPriv_DEFINED

#include "include/core/SkContext.h"
#include "include/core/SkRasterContext.h"
#include "include/core/SkSurfaceProps.h"
#include "src/core/SkResourceCache.h"

class SkRasterContextPriv final : public SkRasterContext {
public:
    SkRasterContextPriv() = default;

    static const SkRasterContextPriv* TODO();

private:
};

#endif
