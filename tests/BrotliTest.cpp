/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkBrotli.h"
#include "tests/Test.h"
#include <stddef.h>

DEF_TEST(SetBrotliDictionaryData_Null, reporter) {
    auto result = SkBrotli::SetDictionaryData(nullptr);
    REPORTER_ASSERT(reporter, !result);
}

DEF_TEST(SetBrotliDictionaryData_Some, reporter) {
    uint8_t array[122784];
    auto result = SkBrotli::SetDictionaryData( &(array[0]));
    REPORTER_ASSERT(reporter, result);
    const uint8_t* dict;
    auto size = SkBrotli::GetDictionaryData(&dict);
    REPORTER_ASSERT(reporter, size == SkBrotli::GetDictionarySize());
    REPORTER_ASSERT(reporter, dict == &(array[0]));
}
