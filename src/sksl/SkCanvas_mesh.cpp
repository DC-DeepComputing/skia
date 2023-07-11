/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlender.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMesh.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkCanvasPriv.h" // IWYU pragma: keep
#include "src/core/SkDevice.h"

#include <optional>
#include <utility>

static SkPaint clean_paint(SkPaint paint) {
    paint.setStyle(SkPaint::kFill_Style);
    paint.setMaskFilter(nullptr);
    paint.setPathEffect(nullptr);
    return paint;
}

void SkCanvas::onDrawMesh(const SkMesh& mesh, sk_sp<SkBlender> blender, const SkPaint& paint) {
    if (!mesh.isValid()) {
        return;
    }
    SkPaint simplePaint = clean_paint(paint);
    if (this->internalQuickReject(mesh.bounds(), simplePaint)) {
        return;
    }

    auto layer = this->aboutToDraw(this, simplePaint, nullptr);
    if (layer) {
        this->topDevice()->drawMesh(mesh, std::move(blender), paint);
    }
}
