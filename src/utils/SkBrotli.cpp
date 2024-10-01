/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkBrotli.h"
#include "include/private/base/SkAssert.h"
#include <dictionary.h>

size_t SkBrotli::GetDictionarySize() {
    auto dictionary = BrotliGetDictionary();
    SkASSERT(dictionary != nullptr);
    return dictionary->data_size;
}

bool SkBrotli::HasDictionaryData() {
  auto dictionary = BrotliGetDictionary();
  SkASSERT(dictionary != nullptr);
  return dictionary->data != nullptr;
}

bool SkBrotli::SetDictionaryData(const uint8_t* dictionaryData) {
    if (dictionaryData != nullptr) {
        auto dictionary = BrotliGetDictionary();
        SkASSERT(dictionary != nullptr);
        if (dictionary->data == nullptr) {
            BrotliSetDictionaryData(dictionaryData);
            dictionary = BrotliGetDictionary();
            SkASSERT(dictionary != nullptr && dictionary->data == dictionaryData);
        }
        SkASSERT(SkBrotli::GetDictionarySize() == dictionary->data_size);
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