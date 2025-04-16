/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkRasterRecorder.h"

#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkOnce.h"
#include "src/core/SkRasterContext_Base.h"

#include <memory>

SkRasterRecorder* SkRasterRecorder::TODO() {
    static std::unique_ptr<SkRasterRecorder> gRecorder;
    static SkOnce once;
    once([]() { gRecorder = SkRasterContext_Base::TODO()->makeRecorder(); });
    return gRecorder.get();
}

sk_sp<SkSurface> SkRasterRecorder::makeSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    constexpr size_t kDefaultRowBytes = 0;
    return SkSurfaces::Raster(info, kDefaultRowBytes, &props);
}

