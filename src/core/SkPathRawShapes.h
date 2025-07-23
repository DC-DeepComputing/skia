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

struct SkPathRawShape_Rect : public SkPathRaw {
    SkPoint m_storage[4];   // move + 3 lines

    SkPathRawShape_Rect(const SkRect&,
                        SkPathDirection = SkPathDirection::kCW,
                        SkPathFillType = SkPathFillType::kWinding);
};

struct SkPathRawShape_Oval : public SkPathRaw {
    SkPoint m_storage[9];   // move + 4 conics

    SkPathRawShape_Oval(const SkRect&,
                        SkPathDirection = SkPathDirection::kCW,
                        SkPathFillType = SkPathFillType::kWinding);
};

#endif
