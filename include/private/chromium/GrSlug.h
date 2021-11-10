/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSlug_DEFINED
#define GrSlug_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"

class SkCanvas;
class SkPaint;
class SkTextBlob;

class GrSlug : public SkRefCnt {
public:
    ~GrSlug() override;
    // Return nullptr if the blob would not draw. This is not because of clipping, but because of
    // some paint optimization.
    static sk_sp<GrSlug> ConvertBlob(
            SkCanvas* canvas, const SkTextBlob& blob, SkPoint origin, const SkPaint& paint);
    void draw(SkCanvas* canvas);
    virtual SkRect sourceBounds() const = 0;
    virtual const SkPaint& paint() const = 0;
};
#endif  // GrSlug_DEFINED
