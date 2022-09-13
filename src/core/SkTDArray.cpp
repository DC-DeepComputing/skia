/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkTDArray.h"

SkTDStorage::SkTDStorage(SkTDStorage&& that)
        : fStorage{std::move(that.fStorage)} { that.fStorage = nullptr; }

SkTDStorage& SkTDStorage::operator=(SkTDStorage&& that) {
    if (this != &that) {
        this->~SkTDStorage();
        new (this) SkTDStorage{std::move(that)};
    }
    return *this;
}

SkTDStorage::~SkTDStorage() {
    sk_free(fStorage);
}

int SkTDStorage::copy(const void* src, int count, size_t TSize) {
    int reserve = 0;
    if (count > 0) {
        reserve = this->resizeStorageToAtLeast(count, TSize);
        memcpy(fStorage, src, SkToSizeT(count) * TSize);
    }
    return reserve;
}

std::tuple<SkTDStorage, int> SkTDStorage::copy(int count, size_t TSize) const {
    SkTDStorage out;
    int reserve = out.copy(fStorage, count, TSize);
    return {std::move(out), reserve};
}

int SkTDStorage::resizeStorageToAtLeast(int count, size_t TSize) {
    SkASSERT(count > 0);
    // Establish the maximum number of elements that includes a valid count for end. In the
    // largest case end() = &fArray[INT_MAX] which is 1 after the last indexable element.
    static constexpr int kMaxCount = INT_MAX;

    // Assume that the array will max out.
    int newReserve = kMaxCount;
    if (kMaxCount - count > 4) {
        // Add 1/4 more than we need. Add 4 to ensure this grows by at least 1. Pin to
        // kMaxCount if no room for 1/4 growth.
        int growth = 4 + ((count + 4) >> 2);
        // Read this line as: if (count + growth < kMaxCount) { ... }
        // It's rewritten to avoid signed integer overflow.
        if (kMaxCount - count > growth) {
            newReserve = count + growth;
        }
    }

    fStorage = sk_realloc_throw(fStorage, SkToSizeT(newReserve) * TSize);
    return newReserve;
}

int SkTDStorage::shrinkToFit(int count, size_t TSize) {
    fStorage = sk_realloc_throw(fStorage, SkToSizeT(count) * TSize);
    return count;
}

