/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathRawShapes_DEFINED
#define SkPathRawShapes_DEFINED

#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "src/core/SkPathRaw.h"

struct SkRect;

namespace SkPathRawShapes {

struct Rect : public SkPathRaw {
    SkPoint fStorage[4];   // move + 3 lines

    Rect(const SkRect&, SkPathDirection = SkPathDirection::kCW,
         SkPathFillType = SkPathFillType::kWinding);
};

struct Oval : public SkPathRaw {
    SkPoint fStorage[9];   // move + 4 conics

    Oval(const SkRect&, SkPathDirection = SkPathDirection::kCW,
         SkPathFillType = SkPathFillType::kWinding);
};

} // namespace

#endif
