/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/SkGaneshRecorder.h"

#include "include/gpu/ganesh/SkSurfaceGanesh.h"


sk_sp<SkSurface> SkGaneshRecorder::makeSurface(const SkImageInfo& ii, const SkSurfaceProps& props) {
    if (fGaneshCtx) {
        auto isBudgeted = skgpu::Budgeted::kNo;  // Assuming we're a one-shot surface
        return SkSurfaces::RenderTarget(fGaneshCtx, isBudgeted, ii, 0, &props);
    }
    return nullptr;
}