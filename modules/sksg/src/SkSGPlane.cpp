/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGPlane.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkAssert.h"

class SkMatrix;
struct SkPoint;

namespace sksg {
Plane::Plane() = default;

void Plane::onClip(SkCanvas*, bool) const {}

void Plane::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    canvas->drawPaint(paint);
}

bool Plane::onContains(const SkPoint&) const { return true; }

SkRect Plane::onRevalidate(InvalidationController*, const SkMatrix&) {
    SkASSERT(this->hasInval());

    return SkRect::MakeLTRB(SK_ScalarMin, SK_ScalarMin, SK_ScalarMax, SK_ScalarMax);
}

SkPath Plane::onAsPath() const {
    SkPathBuilder path;
    path.setFillType(SkPathFillType::kInverseWinding);
    return path.detach();
}

}  // namespace sksg
