/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_PodArray_DEFINED
#define skgpu_graphite_sparse_strips_PodArray_DEFINED

#include "include/private/SkAssert.h"
#include "include/private/SkAttributes.h"
#include "include/private/SkMalloc.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>
#include <utility>

namespace skgpu::graphite {

/*
 * A growable array of trivially copyable elements for the sparse strips hot loops. Like SkTDArray,
 * elements are moved with memcpy and new elements are left uninitialized. Unlike SkTDArray, whose
 * push_back(), append() and resize() are out-of-line calls into SkTDStorage, only growing the
 * allocation is out-of-line, and the capacity doubles (SkTDArray grows by ~1.25x).
 */
template <typename T>
class PodArray {
    static_assert(std::is_trivially_copyable_v<T>);

public:
    PodArray() = default;
    ~PodArray() { sk_free(fData); }

    PodArray(const PodArray& that) { this->copyFrom(that); }
    PodArray& operator=(const PodArray& that) {
        if (this != &that) {
            this->copyFrom(that);
        }
        return *this;
    }
    PodArray(PodArray&& that) noexcept
            : fData(std::exchange(that.fData, nullptr))
            , fSize(std::exchange(that.fSize, 0))
            , fCapacity(std::exchange(that.fCapacity, 0)) {}
    PodArray& operator=(PodArray&& that) noexcept {
        if (this != &that) {
            sk_free(fData);
            fData = std::exchange(that.fData, nullptr);
            fSize = std::exchange(that.fSize, 0);
            fCapacity = std::exchange(that.fCapacity, 0);
        }
        return *this;
    }

    int size() const { return fSize; }
    int capacity() const { return fCapacity; }
    bool empty() const { return fSize == 0; }

    T* data() { return fData; }
    const T* data() const { return fData; }
    T* begin() { return fData; }
    const T* begin() const { return fData; }
    T* end() { return fData + fSize; }
    const T* end() const { return fData + fSize; }

    T& operator[](int i) {
        SkASSERT(i >= 0 && i < fSize);
        return fData[i];
    }
    const T& operator[](int i) const {
        SkASSERT(i >= 0 && i < fSize);
        return fData[i];
    }
    T& back() {
        SkASSERT(fSize > 0);
        return fData[fSize - 1];
    }
    const T& back() const {
        SkASSERT(fSize > 0);
        return fData[fSize - 1];
    }

    // Keeps the allocation.
    void clear() { fSize = 0; }

    // Ensures room for 'minCapacity' elements in total.
    void reserve(int minCapacity) {
        if (minCapacity > fCapacity) {
            this->grow(minCapacity);
        }
    }

    // Like reserve(), but grows to exactly 'minCapacity', for buffers that are sized up front.
    void reserveExact(int minCapacity) {
        if (minCapacity > fCapacity) {
            this->reallocTo(minCapacity);
        }
    }

    // Growing leaves the new elements uninitialized.
    void resize(int newSize) {
        SkASSERT_RELEASE(newSize >= 0);
        this->reserve(newSize);
        fSize = newSize;
    }

    // Appends 'count' uninitialized elements and returns the first of them.
    T* append(int count) {
        SkASSERT_RELEASE(count >= 0 && count <= std::numeric_limits<int>::max() - fSize);
        this->reserve(fSize + count);
        T* first = fData + fSize;
        fSize += count;
        return first;
    }

    // Takes 'value' by value, so pushing an element of this array is safe.
    SK_ALWAYS_INLINE void push_back(T value) {
        if (fSize == fCapacity) {
            this->grow(fSize + 1);
        }
        fData[fSize++] = value;
    }

    void swap(PodArray& that) {
        std::swap(fData, that.fData);
        std::swap(fSize, that.fSize);
        std::swap(fCapacity, that.fCapacity);
    }

private:
    void copyFrom(const PodArray& that) {
        this->resize(that.fSize);
        if (that.fSize > 0) {
            memcpy(fData, that.fData, sizeof(T) * static_cast<size_t>(that.fSize));
        }
    }

    SK_NEVER_INLINE void grow(int minCapacity) {
        // Doubles, but not past kMaxCapacity.
        const int doubled = fCapacity <= kMaxCapacity / 2 ? 2 * fCapacity : kMaxCapacity;
        this->reallocTo(std::max({minCapacity, doubled, 32}));
    }

    // Allocation failures abort in sk_realloc_throw().
    void reallocTo(int newCapacity) {
        SkASSERT_RELEASE(newCapacity >= 0 && newCapacity <= kMaxCapacity);
        fData = static_cast<T*>(
                sk_realloc_throw(fData, sizeof(T) * static_cast<size_t>(newCapacity)));
        fCapacity = newCapacity;
    }

    // The largest capacity whose size in bytes fits in a size_t.
    static constexpr int kMaxCapacity = static_cast<int>(
            std::min(static_cast<size_t>(std::numeric_limits<int>::max()), SIZE_MAX / sizeof(T)));

    T* fData = nullptr;
    int fSize = 0;
    int fCapacity = 0;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_PodArray_DEFINED
