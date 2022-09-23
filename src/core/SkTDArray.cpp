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

SkSizeMultiplexer::SkSizeMultiplexer(const void* src, int count, int sizeOfT) : fSizeOfT{sizeOfT} {
    SkASSERT(src || count == 0);
    this->assign(src, count);
}
SkSizeMultiplexer::SkSizeMultiplexer(const SkSizeMultiplexer& that)
        : SkSizeMultiplexer{that.fStorage, that.fCount, that.fSizeOfT} {}
SkSizeMultiplexer& SkSizeMultiplexer::operator=(const SkSizeMultiplexer& that) {
    if (this != &that) {
        this->assign(that.fStorage, that.fCount);
    }
    return *this;
}
SkSizeMultiplexer::SkSizeMultiplexer(SkSizeMultiplexer&& that)
        : fSizeOfT{that.fSizeOfT}
        , fStorage(std::exchange(that.fStorage, nullptr))
        , fReserve{std::exchange(that.fReserve, 0)}
        , fCount{std::exchange(that.fCount, 0)} {}
SkSizeMultiplexer& SkSizeMultiplexer::operator=(SkSizeMultiplexer&& that) {
    if (this != &that) {
        this->~SkSizeMultiplexer();
        new (this) SkSizeMultiplexer{std::move(that)};
    }
    return *this;
}
SkSizeMultiplexer::~SkSizeMultiplexer() {
    sk_free(fStorage);
}
void SkSizeMultiplexer::reset() {
    const int sizeOfT = fSizeOfT;
    this->~SkSizeMultiplexer();
    new (this) SkSizeMultiplexer{sizeOfT};
}
void SkSizeMultiplexer::resize(int newCount) {
    SkASSERT(newCount >= 0);
    if (newCount > fReserve) {
        this->reserve(newCount);
    }
    fCount = newCount;
}
void SkSizeMultiplexer::reserve(int newReserve) {
    SkASSERT(newReserve >= 0);
    if (newReserve > fReserve) {
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

        fReserve = expandedReserve;
        size_t newStorageSize = this->bytes(fReserve);
        fStorage = static_cast<std::byte*>(sk_realloc_throw(fStorage, newStorageSize));
    }
}
void SkSizeMultiplexer::shrink_to_fit() {
    fReserve = fCount;
    // Because calling realloc with size of 0 is implementation defined, force to a good state
    // by freeing fStorage.
    if (fReserve > 0) {
        fStorage = static_cast<std::byte*>(sk_realloc_throw(fStorage, this->bytes(fReserve)));
    } else {
        sk_free(fStorage);
        fStorage = nullptr;
    }
}
void SkSizeMultiplexer::swap(SkSizeMultiplexer& that) {
    using std::swap;
    swap(fStorage, that.fStorage);
    swap(fReserve, that.fReserve);
    swap(fCount, that.fCount);
}
void* SkSizeMultiplexer::erase(int index, int count) {
    SkASSERT(index >= 0);
    SkASSERT(count >= 0);
    SkASSERT(index + count <= fCount);

    // Check that the resulting size fits in an int. This will abort if not.
    const int newCount = this->calculateSizeOrDie(-count);

    const int tailBegin = index + count;
    if (count > 0) {
        // Move the tail down if there is one.
        if (fCount > tailBegin) {
            this->internalMove(index, tailBegin, fCount - tailBegin);
        }
        this->resize(newCount);
    }

    return this->address(tailBegin);
}
void* SkSizeMultiplexer::removeShuffle(int index) {
    SkASSERT(0 <= index && index < fCount);
    // Check that the new count is valid.
    const int newCount = this->calculateSizeOrDie(-1);

    // Fill the index if not the last element.
    if (index < newCount) {
        this->internalMove(index, newCount, /*count=*/1);
    }
    this->resize(newCount);
    return this->address(newCount);
}
void SkSizeMultiplexer::assign(const void* src, int count) {
    fCount = count;
    this->shrink_to_fit();
    if (count > 0) {
        this->internalCopySrc(/*dstIndex=*/0, src, count);
    }
}
void* SkSizeMultiplexer::prepend() {
    return this->insert(/*index=*/0);
}
void* SkSizeMultiplexer::append() {
    return this->insert(fCount);
}
void* SkSizeMultiplexer::append(const void* src, int count) {
    return this->insert(fCount, src, count);
}
void* SkSizeMultiplexer::insert(int index) {
    return this->insert(index, nullptr, /*count=*/1);
}
void* SkSizeMultiplexer::insert(int index, const void* src, int count) {
    SkASSERT(0 <= index && index <= fCount);
    SkASSERT(count >= 0);

    if (count > 0) {
        const int oldCount = fCount;

        const int newCount = this->calculateSizeOrDie(count);
        this->resize(newCount);

        // Shift memory to make space.
        if (oldCount > index) {
            // Safe because index <= oldCount and oldCount + count is safe.
            this->internalMove(index + count, index, oldCount - index);
        }

        if (src != nullptr) {
            this->internalCopySrc(index, src, count);
        }
    }

    return this->address(index);
}
int SkSizeMultiplexer::calculateSizeOrDie(int delta) {
    // Check that count will not go negative.
    SkASSERT_RELEASE(-fCount <= delta);

    // We take care to avoid overflow here.
    // Because count and delta are both signed 32-bit ints, the sum of count and delta is at
    // most 4294967294, which fits fine in uint32_t. Proof follows in assert.
    static_assert(UINT32_MAX >= (uint32_t)INT_MAX + (uint32_t)INT_MAX);
    uint32_t testCount = (uint32_t)fCount + (uint32_t)delta;
    SkASSERT_RELEASE(SkTFitsIn<int>(testCount));
    return SkToInt(testCount);
}
void SkSizeMultiplexer::internalMove(int dstIndex, int srcIndex, int count) {
    this->internalCopySrc(dstIndex, this->address(srcIndex), count);
}
void SkSizeMultiplexer::internalCopySrc(int dstIndex, const void* src, int count) {
    SkASSERT(count > 0);
    memmove(this->address(dstIndex), src, this->bytes(count));
}
