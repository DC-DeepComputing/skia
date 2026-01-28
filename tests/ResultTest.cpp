/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkResult.h"
#include "tests/Test.h"

enum class EnumErrorClass {
    kInvalidInput,
    kOutOfMemory,
    kUnsupported,
};

static SkResult<SkBitmap, EnumErrorClass> get_bitmap(int w, int h) {
    if (w < 0 || h < 0) {
        return {EnumErrorClass::kInvalidInput};
    }
    SkBitmap bm;
    bm.setInfo(SkImageInfo::MakeN32Premul(w, h));
    if (!bm.tryAllocPixels()) {
        return {EnumErrorClass::kOutOfMemory};
    }
    return {bm};
}

DEF_TEST(SkResult_WithEnumClass_SuccessCaseReturnsBitmap, reporter) {
    auto result = get_bitmap(10, 13);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, result.has_value());

    SkBitmap bm = result.value();
    REPORTER_ASSERT(reporter, !bm.empty());
    REPORTER_ASSERT(reporter, result->height() == 13);
}

DEF_TEST(SkResult_WithEnumClass_ErrorCaseReturnsEnum, reporter) {
    auto result = get_bitmap(-10, 13);
    REPORTER_ASSERT(reporter, !result);
    REPORTER_ASSERT(reporter, !result.has_value());

    EnumErrorClass err = result.error();
    REPORTER_ASSERT(reporter, err == EnumErrorClass::kInvalidInput);
}

enum EnumError {
    kInvalidInput_Error,
    kOutOfMemory_Error,
    kUnsupported_Error,
};

static SkResult<SkBitmap, EnumError> get_bitmap2(int w, int h) {
    if (w < 0 || h < 0) {
        return {EnumError::kInvalidInput_Error};
    }
    SkBitmap bm;
    bm.setInfo(SkImageInfo::MakeN32Premul(w, h));
    if (!bm.tryAllocPixels()) {
        return {EnumError::kOutOfMemory_Error};
    }
    return {bm};
}

DEF_TEST(SkResult_WithEnum_ErrorCaseReturnsEnum, reporter) {
    auto result = get_bitmap2(-10, 13);
    REPORTER_ASSERT(reporter, !result);
    REPORTER_ASSERT(reporter, !result.has_value());

    EnumError err = result.error();
    REPORTER_ASSERT(reporter, err == EnumError::kInvalidInput_Error);
}
