/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkTDArray.h"
#include "include/private/SkTo.h"

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
    if (count > 0) {
        fStorage = sk_realloc_throw(fStorage, SkToSizeT(count) * TSize);
        memcpy(fStorage, src, SkToSizeT(count) * TSize);
    }
    return count;
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

std::tuple<int, int> SkTDStorage::append(
        const void* src, int count, size_t TSize, int reserve, int oldCount) {
    SkASSERT(count >= 0);
    int newCount = oldCount;
    int newReserve = reserve;
    if (count > 0) {
        // We take care to avoid overflow here.
        // The sum of fCount and delta is at most 4294967294, which fits fine in uint32_t.
        uint32_t testCount = (uint32_t)oldCount + (uint32_t)count;
        SkASSERT_RELEASE(SkTFitsIn<int>(testCount));
        newCount = testCount;
        if (newCount > reserve) {
            newReserve = this->resizeStorageToAtLeast(newCount, TSize);
        }
        if (src != nullptr) {
            memcpy(this->array<char>() + TSize*SkToSizeT(oldCount), src, TSize*SkToSizeT(count));
        }
    }
    return {newCount, newReserve};
}

