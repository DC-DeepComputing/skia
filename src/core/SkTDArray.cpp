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

SkTDStorage::SkTDStorage(const void* src, int count, int sizeOfT) : fSizeOfT{sizeOfT} {
    SkASSERT(src || count == 0);
    this->assign(count, src);
}

SkTDStorage::SkTDStorage(const SkTDStorage& that)
        : SkTDStorage{that.data(), that.size(), that.fSizeOfT} {}

SkTDStorage& SkTDStorage::operator=(const SkTDStorage& that) {
    if (this != &that) {
        this->assign(that.size(), that.data());
    }
    return *this;
}

SkTDStorage::SkTDStorage(SkTDStorage&& that)
        : fSizeOfT{that.fSizeOfT}
        , fEnd(that.fEnd)
        , fCapacity{that.fCapacity}
        , fSize{that.fSize} {
    that.fEnd = nullptr;
    that.fSize = 0;
}

SkTDStorage& SkTDStorage::operator=(SkTDStorage&& that) {
    if (this != &that) {
        this->~SkTDStorage();
        new (this) SkTDStorage{std::move(that)};
    }
    return *this;
}

SkTDStorage::~SkTDStorage() {
    sk_free(this->data());
}

void SkTDStorage::reset() {
    const int sizeOfT = fSizeOfT;
    this->~SkTDStorage();
    new (this) SkTDStorage{sizeOfT};
}

void SkTDStorage::swap(SkTDStorage& that) {
    SkASSERT(fSizeOfT == that.fSizeOfT);
    using std::swap;
    swap(fEnd, that.fEnd);
    swap(fCapacity, that.fCapacity);
    swap(fSize, that.fSize);
}

void SkTDStorage::resize(int size) {
    SkASSERT(size >= 0);
    if (size > fCapacity) {
        this->reserve(size);
    }
    fEnd += this->bytes(size - fSize);
    fSize = size;
}

void SkTDStorage::reserve(int capacity) {
    SkASSERT(capacity >= 0);
    if (capacity > fCapacity) {
        // Establish the maximum number of elements that includes a valid count for end. In the
        // largest case end() = &fArray[INT_MAX] which is 1 after the last indexable element.
        static constexpr int kMaxCount = INT_MAX;

        // Assume that the array will max out.
        int expandedCapacity = kMaxCount;
        if (kMaxCount - capacity > 4) {
            // Add 1/4 more than we need. Add 4 to ensure this grows by at least 1. Pin to
            // kMaxCount if no room for 1/4 growth.
            int growth = 4 + ((capacity + 4) >> 2);
            // Read this line as: if (count + growth < kMaxCount) { ... }
            // It's rewritten to avoid signed integer overflow.
            if (kMaxCount - capacity > growth) {
                expandedCapacity = capacity + growth;
            }
        }

        this->realloc(expandedCapacity);
    }
}

void SkTDStorage::shrink_to_fit() {
    if (fCapacity != fSize) {
        // Because calling realloc with size of 0 is implementation defined, force to a good state
        // by freeing fStorage.
        if (fSize > 0) {
            this->realloc(fSize);
        } else {
            sk_free(this->data());
            fEnd = nullptr;
            fCapacity = 0;
            fSize = 0;
        }
    }
}

void SkTDStorage::erase(int index, int count) {
    SkASSERT(count >= 0);
    SkASSERT(fSize >= count);
    SkASSERT(0 <= index && index <= fSize);

    if (count > 0) {
        // Check that the resulting size fits in an int. This will abort if not.
        const int newCount = this->calculateSizeOrDie(-count);
        this->moveTail(index, index + count, fSize);
        this->resize(newCount);
    }
}

void SkTDStorage::removeShuffle(int index) {
    SkASSERT(fSize > 0);
    SkASSERT(0 <= index && index < fSize);
    // Check that the new count is valid.
    const int newCount = this->calculateSizeOrDie(-1);
    this->moveTail(index, fSize - 1, fSize);
    this->resize(newCount);
}

void SkTDStorage::assign(int count, const void* src) {
    if (fCapacity < count) {
        this->realloc(count);
    }
    if (count > 0 && src != nullptr) {
        this->copySrc(/*dstIndex=*/0, src, count);
    }
    fEnd += this->bytes(count - fSize);
    fSize = count;
}

void* SkTDStorage::prepend() {
    return this->insert(/*index=*/0);
}

void* SkTDStorage::append(int count, const void* src) {
    return this->insert(fSize, count, src);
}

void* SkTDStorage::insert(int index) {
    return this->insert(index, /*count=*/1, nullptr);
}

void* SkTDStorage::insert(int index, int count, const void* src) {
    SkASSERT(0 <= index && index <= fSize);
    SkASSERT(count >= 0);

    if (count > 0) {
        const int oldCount = fSize;
        const int newCount = this->calculateSizeOrDie(count);
        this->resize(newCount);
        this->moveTail(index + count, index, oldCount);

        if (src != nullptr) {
            this->copySrc(index, src, count);
        }
    }

    return this->address(index);
}

bool operator==(const SkTDStorage& a, const SkTDStorage& b) {
    return a.size() == b.size() &&
           (a.size() == 0 || !memcmp(a.data(), b.data(), a.bytes(a.size())));
}

void SkTDStorage::realloc(int capacity) {
    fCapacity = capacity;
    size_t newStorageSize = this->bytes(capacity);
    std::byte* begin = static_cast<std::byte*>(sk_realloc_throw(this->data(), newStorageSize));
    fEnd = begin + this->bytes(fSize);
}

int SkTDStorage::calculateSizeOrDie(int delta) {
    // Check that count will not go negative.
    SkASSERT_RELEASE(-fSize <= delta);

    // We take care to avoid overflow here.
    // Because count and delta are both signed 32-bit ints, the sum of count and delta is at
    // most 4294967294, which fits fine in uint32_t. Proof follows in assert.
    static_assert(UINT32_MAX >= (uint32_t)INT_MAX + (uint32_t)INT_MAX);
    uint32_t testCount = (uint32_t)fSize + (uint32_t)delta;
    SkASSERT_RELEASE(SkTFitsIn<int>(testCount));
    return SkToInt(testCount);
}

void SkTDStorage::moveTail(int to, int tailStart, int tailEnd) {
    SkASSERT(0 <= to && to <= fSize);
    SkASSERT(0 <= tailStart && tailStart <= tailEnd && tailEnd <= fSize);
    if (to != tailStart && tailStart != tailEnd) {
        this->copySrc(to, this->address(tailStart), tailEnd - tailStart);
    }
}

void SkTDStorage::copySrc(int dstIndex, const void* src, int count) {
    SkASSERT(count > 0);
    memmove(this->address(dstIndex), src, this->bytes(count));
}
