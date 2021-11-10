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

class GrAtlasSubRun;
class SkCanvas;
class SkGlyphRunList;
class SkMatrix;
class SkMatrixProvider;
class SkPaint;
class SkTextBlob;

namespace skgpu {
namespace v1 {
class SurfaceDrawContext;
}
}

class GrSlug : public SkRefCnt {
public:
    // Return nullptr if would not draw.
    static sk_sp<GrSlug> ConvertBlob(
            SkCanvas* canvas, const SkTextBlob& blob, const SkPaint& paint);
    ~GrSlug() override;
    void draw(SkCanvas* canvas);
    virtual SkRect sourceBounds() const = 0;
    virtual const SkPaint& paint() const = 0;
};
#endif  // GrSlug_DEFINED
