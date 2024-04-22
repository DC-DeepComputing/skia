/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TINYARRAY
#define SKSL_TINYARRAY

#include "include/core/SkTypes.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>

namespace SkSL {

/**
 * Represents a tiny array of `T` (must be trivial) that cannot grow past a fixed size `N`.
 */
template <int N, typename T>
class TinyArray {
public:
    TinyArray() = default;

    TinyArray(std::initializer_list<T> values) {
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

    bool operator==(const TinyArray<N, T>& that) const {
        return fSize == that.fSize && (0 == memcmp(fData, that.fData, fSize));
    }

    int size() const {
        return fSize;
    }

    bool empty() const {
        return fSize == 0;
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
        SkASSERT(fSize < 4);
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
    static_assert(N < 256);

    T fData[N];
    uint8_t fSize = 0;
};

}  // namespace SkSL

#endif
