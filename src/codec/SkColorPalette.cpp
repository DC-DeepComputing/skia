/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkColorPalette.h"

#include "include/private/SkMalloc.h"

#include <cstring>

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-format-attr-call"
#endif

SkColorPalette::SkColorPalette(const SkPMColor colors[], int count) {
    SkASSERT(0 == count || colors);
    SkASSERT(count >= 0 && count <= 256);

    fCount = count;
    fColors = reinterpret_cast<SkPMColor*>(sk_malloc_throw(count * sizeof(SkPMColor)));

    memcpy(fColors, colors, count * sizeof(SkPMColor));
}

SkColorPalette::~SkColorPalette() {
    sk_free(fColors);
}
