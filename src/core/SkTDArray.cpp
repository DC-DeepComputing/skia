/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkTDArray.h"
#include <utility>
#include "include/private/SkTo.h"

namespace {
size_t offset(int n, size_t sizeOfT) {
    return SkToSizeT(n) * sizeOfT;
}

size_t size(int n, size_t sizeOfT) {
    return SkToSizeT(n) * sizeOfT;
}
}  // namespace

SkTDStorage::SkTDStorage(SkTDStorage&& that)
        : fStorage{std::move(std::exchange(that.fStorage, nullptr))}
        , fReserve{std::exchange(that.fReserve, 0)}
        , fCount{std::exchange(that.fCount, 0)} {}

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

void SkTDStorage::assign(const void* src, int count, size_t sizeOfT) {
    if (count > 0) {
        fCount = count;
        size_t byteSize = this->size_bytes(sizeOfT);
        if (fCount > fReserve) {
            fStorage = static_cast<std::byte*>(sk_realloc_throw(fStorage, byteSize));
            fReserve = fCount;
        }
        memcpy(fStorage, src, byteSize);
    }
}

void SkTDStorage::resize(int newCount, size_t sizeOfT) {
    if (fReserve < newCount) {
        this->reserveAtLeast(newCount, sizeOfT);
    }
    fCount = newCount;
}

size_t SkTDStorage::size_bytes(size_t sizeOfT) const {
    return offset(fCount, sizeOfT);
}

void SkTDStorage::reserve(int newReserve, size_t sizeOfT) {
    SkASSERT(0 <= newReserve);
    fReserve = newReserve;
    // Because using a size of zero implementation defined force to defined behavior.
    if (0 < fReserve) {
        fStorage = static_cast<std::byte*>(sk_realloc_throw(fStorage, size(fReserve, sizeOfT)));
    } else {
        sk_free(fStorage);
        fStorage = nullptr;
    }
}

void SkTDStorage::shrinkToFit(size_t sizeOfT) {
    this->reserve(fCount, sizeOfT);
}

void* SkTDStorage::erase(int index, int count, size_t sizeOfT) {
    SkASSERT(index + count <= fCount);

    // Check that the resulting size is Ok. This will throw if bad.
    const int newCount = this->safeCountChange(-count);

    const int tailBegin = index + count;
    const size_t tailBeginOffset = offset(tailBegin, sizeOfT);
    if (0 < count) {
        // Move the tail down if there is one.
        if (tailBegin < fCount) {
            const size_t headEndOffset = offset(index, sizeOfT);
            const size_t gapSize = this->size_bytes(sizeOfT) - tailBeginOffset;
            memmove(fStorage + headEndOffset, fStorage + tailBeginOffset, gapSize);
        }
        this->resize(newCount, sizeOfT);
    }

    return fStorage + tailBeginOffset;
}

void* SkTDStorage::prepend(size_t sizeOfT) {
    return this->insert(0, nullptr, 1, sizeOfT);
}

void* SkTDStorage::append(size_t sizeOfT) {
    return this->append(nullptr, 1, sizeOfT);
}

void* SkTDStorage::append(const void* src, int count, size_t sizeOfT) {
    return this->insert(fCount, src, count, sizeOfT);
}

void* SkTDStorage::insert(int index, const void* src, int count, size_t sizeOfT) {
    SkASSERT(0 <= index && index <= fCount);
    SkASSERT(0 <= count);
    const size_t indexOffset = offset(index, sizeOfT);

    if (0 < count) {
        const int oldCount = fCount;

        size_t oldCountOffset = this->size_bytes(sizeOfT);

        const int newCount = this->safeCountChange(count);
        this->resize(newCount, sizeOfT);

        // Shift memory to make space.
        if (index < oldCount) {
            // Safe because index <= oldCount and oldCount + count is safe.
            size_t shiftOffset = offset(index + count, sizeOfT);

            // Move the tail of data from index to oldCount.
            memmove(fStorage + shiftOffset, fStorage + indexOffset, oldCountOffset - indexOffset);
        }

        if (src != nullptr) {
            memcpy(fStorage + indexOffset, src, size(count, sizeOfT));
        }
    }

    return fStorage + indexOffset;
}

int SkTDStorage::safeCountChange(int delta) const {
    // Check that count will not go negative.
    SkASSERT_RELEASE(-fCount <= delta);

    // We take care to avoid overflow here.
    // The sum of fCount and delta is at most 4294967294, which fits fine in uint32_t.
    uint32_t count = (uint32_t)this->count() + (uint32_t)delta;
    SkASSERT_RELEASE(SkTFitsIn<int>(count));
    return SkToInt(count);
}

void SkTDStorage::reserveAtLeast(int count, size_t sizeOfT) {
    SkASSERT(fReserve < count);
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

    this->reserve(newReserve, sizeOfT);
}
