/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/codec/SkColorPalette.h"


#include <cstring>

SkColorPalette::SkColorPalette(const SkPMColor colors[], int count)
        : fColors(std::make_unique<SkPMColor[]>(count))
        , fCount(count) {
    SkASSERT(0 == count || colors);
    SkASSERT(count >= 0 && count <= 256);

    if (count > 0) {
        memcpy(fColors.get(), colors, count * sizeof(SkPMColor));
    }
}

SkColorPalette::~SkColorPalette() = default;
