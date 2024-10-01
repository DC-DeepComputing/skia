/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkBrotli.h"
#include "include/private/base/SkAssert.h"
#include "third_party/externals/brotli/c/common/dictionary.h"

bool SkBrotli::SetDictionaryData(const uint8_t* dictionaryData, size_t dictionaryLength) {
    if (dictionaryData != nullptr) {
        auto dictionary = BrotliGetDictionary();
        SkASSERT(dictionary != nullptr);
        if (dictionary->data == nullptr) {
            BrotliSetDictionaryData(dictionaryData);
            dictionary = BrotliGetDictionary();
            SkASSERT(dictionary != nullptr && dictionary->data == dictionaryData);
        }
        SkASSERT(SkBrotli::DictionarySize == dictionaryLength);
        return true;
    } else {
        return false;
    }
}

size_t SkBrotli::GetDictionaryData(const uint8_t** dictionaryData) {
  auto dictionary = BrotliGetDictionary();
  SkASSERT(dictionary != nullptr);
  *dictionaryData = dictionary->data;
  return dictionary->data_size;
}