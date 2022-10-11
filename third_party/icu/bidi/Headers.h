// Copyright 2022 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "unicode/ubidi.h"

namespace Bidi {
    typedef uint8_t UBiDiLevel;
    static void reorderVisual(const UBiDiLevel* levels, int32_t length, int32_t* indexMap) {
        ubidi_reorderVisual(levels, length, indexMap);
    }
};
