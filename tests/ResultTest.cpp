/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkResult.h"
#include "include/core/SkBitmap.h"
#include "tests/Test.h"

enum class EnumErrorClass {
    kInvalidInput,
    kOutOfMemory,
    kUnsupported,
}

static SkResult<SkBitmap, EnumErrorClass> get_bitmap(int w, int h) {
    if (w < 0 || h < 0) {
        return SkResult(EnumErrorClass::kInvalidInput);
    }
    SkBitmap bm;
    bm.setInfo(SkImageInfo::MakeN32Premul(w, h));
    if (!bm.tryAllocPixels()) {
        return SkResult(EnumErrorClass::kOutOfMemory);
    }
    return SkResult(bm);
}

DEF_TEST(Result_WithEnumClass, reporter) {
    auto result = get_bitmap(10, 13);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, result.has_value());

    SkBitmap bm = result.value();
    result->height();
}