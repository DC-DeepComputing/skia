/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBrotli_DEFINED
#define SkBrotli_DEFINED

#include "include/private/base/SkAPI.h"
#include <cstdint>
#include <stddef.h>

class SK_API SkBrotli {
public:
    static constexpr size_t DictionarySize = 122784;
    static bool SetDictionaryData(const uint8_t* dictionaryData, size_t dictionaryLength);
    static size_t GetDictionaryData(const uint8_t** dictionaryData);
};

#endif
