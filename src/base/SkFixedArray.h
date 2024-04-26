/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTFixedArray_DEFINED
#define SkTFixedArray_DEFINED

#include "include/core/SkTypes.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <utility>

namespace skia_private {

/**
 * Represents an array of `T` (must be a trivial type) that cannot grow past a fixed size `N`.
 * The fixed-size restriction allows for tighter codegen and a smaller memory footprint.
 * Missing methods from TArray (e.g. `push_back_n`) can be added on demand.
 *
 * The trivial-type restriction is only to simplify implementation; if there is a need, we can
 * adopt proper move/copy semantics in this class as well.
 */
template <int N, typename T>
class FixedArray {
public:
    FixedArray() = default;

    FixedArray(std::initializer_list<T> values) {
        for (int8_t value : values) {
            fData[fSize++] = value;
        }
    }

    T& operator[](size_t index) {
        SkASSERT(index < fSize);
        return fData[index];
    }

    const T& operator[](size_t index) const {
        SkASSERT(index < fSize);
        return fData[index];
    }

    bool operator==(const FixedArray<N, T>& that) const {
        return fSize == that.fSize && (std::mismatch(this->begin(), this->end(), that.begin()) ==
                                       std::pair(this->end(), that.end()));
    }

    bool operator!=(const FixedArray<N, T>& that) const {
        return !this->operator==(that);
    }

    int size() const {
        return fSize;
    }

    bool empty() const {
        return fSize == 0;
    }

    void clear() {
        fSize = 0;
    }

    void resize(int newSize) {
        if (fSize > newSize) {
            fSize = newSize;
        } else {
            while (newSize > fSize) {
                this->push_back(T());
            }
        }
    }

    void push_back(T x) {
        SkASSERT(fSize < N);
        fData[fSize++] = x;
    }

    void pop_back() {
        SkASSERT(fSize > 0);
        --fSize;
    }

    T* data() {
        return fData;
    }

    const T* data() const {
        return fData;
    }

    T* begin() {
        return fData;
    }

    const T* begin() const {
        return fData;
    }

    T* end() {
        return fData + fSize;
    }

    const T* end() const {
        return fData + fSize;
    }

    T& front() {
        SkASSERT(fSize > 0);
        return fData[0];
    }

    const T& front() const {
        SkASSERT(fSize > 0);
        return fData[0];
    }

    T& back() {
        SkASSERT(fSize > 0);
        return fData[fSize - 1];
    }

    const T& back() const {
        SkASSERT(fSize > 0);
        return fData[fSize - 1];
    }

private:
    static_assert(std::is_trivial_v<T>);
    static_assert(N > 0);
    static_assert(N < 256);  // restricted by `uint8_t fSize`

    T fData[N];
    uint8_t fSize = 0;
};

}  // namespace skia_private

#endif
