/*
* Copyright 2022 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkTestCanvas_DEFINED
#define SkTestCanvas_DEFINED

#include "include/core/SkSize.h"
#include "include/utils/SkNWayCanvas.h"
#include "src/core/SkDevice.h"
#include "src/core/SkGlyphRun.h"

template <typename Key> class SkTestCanvas;

struct SkSlugTestKey {};

template <>
class SkTestCanvas<SkSlugTestKey> : public SkNWayCanvas {
public:
    SkTestCanvas(SkISize size, SkCanvas* convertCanvas);
    void onDrawGlyphRunList(const SkGlyphRunList& glyphRunList, const SkPaint& paint) override;
private:
    SkCanvas* fConvertCanvas;
};

#endif  // SkTestCanvas_DEFINED
