/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Combinations.h"

int FirstCombinableTileMode(CombinableTileMode combinableTileMode) {
    static_assert(0x1 == static_cast<int>(CombinableTileMode::kFirst));

    int initialTM = static_cast<int>(CombinableTileMode::kFirst);
    for ( ; initialTM <= static_cast<int>(CombinableTileMode::kLast); initialTM <<= 1) {
        if (initialTM & static_cast<int>(combinableTileMode)) {
            break;
        }
    }

    SkASSERT(initialTM <= (int) CombinableTileMode::kLast);
    return initialTM;
}
