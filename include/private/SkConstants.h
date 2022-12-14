/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkConstants_DEFINED
#define SkConstants_DEFINED

#include "include/private/SkScalar_impl.h"

// skbug.com/9906: Not a perfect solution for W plane clipping, but 1/16384 is a
// reasonable limit (roughly 5e-5)
constexpr SkScalar kW0PlaneDistance = 1.f / (1 << 14);

#endif
