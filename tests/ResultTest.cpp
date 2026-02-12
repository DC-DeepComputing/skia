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

    SkBitmap bm = result.take();
    REPORTER_ASSERT(reporter, !bm.empty());
    REPORTER_ASSERT(reporter, bm.height() == 13);
    REPORTER_ASSERT(reporter, !result.has_value());
    REPORTER_ASSERT(reporter, result.error() == SkError::kEmptyResult);
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

DEF_TEST(SkResultV_WithCustomError_ErrorCaseReturnsCustomError, reporter) {
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
    SkImageInfo ii = SkImageInfo::MakeN32Premul(w, h);
    SkBitmap bm;
    bm.setInfo(ii);
    if (!bm.tryAllocPixels()) {
        return {SkError::kOutOfMemory};
    }
    // If we update this call to return an SkResultP we can return
    // it directly.
    auto img = SkImage_Raster::MakeFromBitmap(bm, SkCopyPixelsMode::kNever);
    if (!img) {
        return {SkError::kInternalError};
    }
    return {img.release()};
}

DEF_TEST(SkResultP_WithError_SuccessCaseReturnsNonNullImage, reporter) {
    {
        auto result = get_image(10, 13);
        REPORTER_ASSERT(reporter, result);
        REPORTER_ASSERT(reporter, result.has_value());

        sk_sp<SkImage> img = result.take();
        REPORTER_ASSERT(reporter, img);
        REPORTER_ASSERT(reporter, !result.has_value());
        REPORTER_ASSERT(reporter, result.error() == SkError::kEmptyResult);
    }
    {
        auto result = get_image(10, 13);
        REPORTER_ASSERT(reporter, result);
        REPORTER_ASSERT(reporter, result.has_value());

        std::unique_ptr<SkImage> img = result.takeUnique();
        REPORTER_ASSERT(reporter, img);
        REPORTER_ASSERT(reporter, !result.has_value());
        REPORTER_ASSERT(reporter, result.error() == SkError::kEmptyResult);
    }
}

DEF_TEST(SkResultP_WithError_ErrorCaseReturnsEnum, reporter) {
    auto result = get_image(-10, 13);
    REPORTER_ASSERT(reporter, !result);
    REPORTER_ASSERT(reporter, !result.has_value());

    SkError err = result.error();
    REPORTER_ASSERT(reporter, err == SkError::kInvalidInput);
}

struct MoveOnly {
    MoveOnly(int* moveCounter) : fMoveCounter(moveCounter) {}
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&& that) : fMoveCounter(that.fMoveCounter) {
        SkASSERT_RELEASE(fMoveCounter);
        (*fMoveCounter)++;
    }
    MoveOnly& operator=(MoveOnly&& that) {
        fMoveCounter = that.fMoveCounter;
        SkASSERT_RELEASE(fMoveCounter);
        (*fMoveCounter)++;
        return *this;
    }
    int* fMoveCounter;
};

DEF_TEST(SkResultV_MoveSemantics, reporter) {
    int moveCount = 0;
    SkResultV<MoveOnly> res{MoveOnly(&moveCount)};
    REPORTER_ASSERT(reporter, res.has_value());
    REPORTER_ASSERT(reporter, moveCount == 1);

    SkResultV<MoveOnly> intermediate = std::move(res);
    REPORTER_ASSERT(reporter, moveCount == 2);
    REPORTER_ASSERT(reporter, intermediate.has_value());
    REPORTER_ASSERT(reporter, !res.has_value());
    REPORTER_ASSERT(reporter, res.error() == SkError::kEmptyResult);

    MoveOnly moved = intermediate.take();
    REPORTER_ASSERT(reporter, moveCount == 3);
    REPORTER_ASSERT(reporter, moved.fMoveCounter == &moveCount);
}

struct DeleteCounter {
    DeleteCounter(int* deleteCounter) : fDeleteCounter(deleteCounter) {}
    ~DeleteCounter() {
        SkASSERT_RELEASE(fDeleteCounter);
        (*fDeleteCounter)++;
    }
    int* fDeleteCounter;
};

DEF_TEST(SkResultP_CleanupRawPointer, reporter) {
    int deleteCount = 0;
    {
        SkResultP<DeleteCounter> res(new DeleteCounter(&deleteCount));
        REPORTER_ASSERT(reporter, res.has_value());
    }
    REPORTER_ASSERT(reporter, deleteCount == 1);
}
