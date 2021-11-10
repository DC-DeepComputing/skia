/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/chromium/GrSlug.h"

#include "include/core/SkCanvas.h"

GrSlug::~GrSlug() = default;
sk_sp<GrSlug> GrSlug::ConvertBlob(
        SkCanvas* canvas, const SkTextBlob& blob, const SkPaint& paint) {
    return canvas->convertBlobToSlug(blob, paint);
}
void GrSlug::draw(SkCanvas* canvas) {
    canvas->drawSlug(this);
}

// Most of GrSlug's implementation is in GrTextBlob.cpp to share common code.



