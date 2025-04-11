/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkRasterContext.h"

#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "src/core/SkRasterContextPriv.h"
#include "src/core/SkRasterRecorderPriv.h"
#include "src/core/SkResourceCache.h"

#include <utility>

SkRasterRecorder* SkRasterRecorder::TODO() {
    static std::unique_ptr<SkRasterRecorder> gRecorder;
    static SkOnce once;
    once([]() { gRecorder = SkRasterContextPriv::TODO()->makeRecorder(); });
    return gRecorder.get();
}
