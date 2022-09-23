/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkMalloc.h"
#include "include/private/SkTDArray.h"
#include "include/private/SkTo.h"

#include <utility>

namespace {
size_t mem_size(int n, size_t sizeOfT) {
    return SkToSizeT(n) * sizeOfT;
}
}  // namespace

// -- SkTDStorage ----------------------------------------------------------------------------------
SkTDStorage::SkTDStorage(SkTDStorage&& that) : fStorage{std::exchange(that.fStorage, nullptr)} {}

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

void SkTDStorage::shrink_to_fit(size_t storageSize) {
    // Because calling realloc with size of 0 is implementation defined, force to a good state by
    // freeing fStorage.
    if (storageSize != 0) {
        fStorage = static_cast<std::byte*>(sk_realloc_throw(fStorage, storageSize));
    } else {
        sk_free(fStorage);
        fStorage = nullptr;
    }
}

int SkTDStorage::reserve(int newReserve, size_t sizeOfT) {
    // Establish the maximum number of elements that includes a valid count for end. In the
    // largest case end() = &fArray[INT_MAX] which is 1 after the last indexable element.
    static constexpr int kMaxCount = INT_MAX;

    // Assume that the array will max out.
    int expandedReserve = kMaxCount;
    if (kMaxCount - newReserve > 4) {
        // Add 1/4 more than we need. Add 4 to ensure this grows by at least 1. Pin to
        // kMaxCount if no room for 1/4 growth.
        int growth = 4 + ((newReserve + 4) >> 2);
        // Read this line as: if (count + growth < kMaxCount) { ... }
        // It's rewritten to avoid signed integer overflow.
        if (kMaxCount - newReserve > growth) {
            expandedReserve = newReserve + growth;
        }
    }

    size_t newStorageSize = mem_size(expandedReserve, sizeOfT);
    this->shrink_to_fit(newStorageSize);
    return expandedReserve;
}
