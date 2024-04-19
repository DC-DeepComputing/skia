/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArc_DEFINED
#define SkArc_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"

// Represents an arc along an oval boundary, or a closed wedge of the oval.
struct SkArc {
    SkRect   fOval;
    SkScalar fStartAngle;
    SkScalar fSweepAngle;
    bool     fUseCenter;
};

#endif
