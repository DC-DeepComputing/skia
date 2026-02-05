/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkResult.h"
#include "src/image/SkImage_Raster.h"
#include "tests/Test.h"

static SkResultV<SkBitmap> get_bitmap(int w, int h) {
    if (w < 0 || h < 0) {
        return {SkError::kInvalidInput};
    }
    SkBitmap bm;
    bm.setInfo(SkImageInfo::MakeN32Premul(w, h));
    if (!bm.tryAllocPixels()) {
        return {SkError::kOutOfMemory};
    }
    return {bm};
}

DEF_TEST(SkResultV_WithError_SuccessCaseReturnsBitmap, reporter) {
    auto result = get_bitmap(10, 13);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, result.has_value());

    SkBitmap bm = result.value();
    REPORTER_ASSERT(reporter, !bm.empty());
    REPORTER_ASSERT(reporter, result->height() == 13);
}

DEF_TEST(SkResultV_WithError_ErrorCaseReturnsEnum, reporter) {
    auto result = get_bitmap(-10, 13);
    REPORTER_ASSERT(reporter, !result);
    REPORTER_ASSERT(reporter, !result.has_value());

    SkError err = result.error();
    REPORTER_ASSERT(reporter, err == SkError::kInvalidInput);
}

enum EnumError {
    kInvalidInput_Error,
    kOutOfMemory_Error,
    kUnsupported_Error,
};

static SkResultV<SkBitmap, EnumError> get_bitmap2(int w, int h) {
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

DEF_TEST(SkResultV_WithEnum_ErrorCaseReturnsEnum, reporter) {
    auto result = get_bitmap2(-10, 13);
    REPORTER_ASSERT(reporter, !result);
    REPORTER_ASSERT(reporter, !result.has_value());

    EnumError err = result.error();
    REPORTER_ASSERT(reporter, err == EnumError::kInvalidInput_Error);
}

static SkResultP<SkImage> get_image(int w, int h) {
    if (w < 0 || h < 0) {
        //  return {nullptr} is a compile error
        return {SkError::kInvalidInput};
    }
    SkBitmap bm;
    bm.setInfo(SkImageInfo::MakeN32Premul(w, h));
    if (!bm.tryAllocPixels()) {
        return {SkError::kOutOfMemory};
    }
    return {new SkImage_Raster({}, nullptr, 0, nullptr, 0)};
}

DEF_TEST(SkResultP_WithError_SuccessCaseReturnsNonNullImage, reporter) {
    auto result = get_image(10, 13);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, result.has_value());

    sk_sp<SkImage> img = result.take();
    REPORTER_ASSERT(reporter, img);
}
