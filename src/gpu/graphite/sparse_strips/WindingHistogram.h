/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_WindingHistogram_DEFINED
#define skgpu_graphite_WindingHistogram_DEFINED

#include "include/private/SkAssert.h"
#include "include/private/SkTDArray.h"

#include <cstring>

namespace skgpu::graphite {

class WindingHistogram {
public:
    WindingHistogram() = default;

    WindingHistogram(int maxRows) {
        this->reserve(maxRows);
    }

    void reserve(int maxRows) {
        fWindingData.reserve(maxRows);
        fActiveRows.reserve((maxRows + 31) >> 5);
    }

    void resize(int numRows) {
        fWindingData.resize(numRows);
        fActiveRows.resize((numRows + 31) >> 5);
    }

    void clear() {
        std::memset(fWindingData.data(), 0, fWindingData.size_bytes());
        std::memset(fActiveRows.data(), 0, fActiveRows.size_bytes());
    }

    int16_t operator[](int row) const {
        SkASSERT(row >= 0 && row < fWindingData.size());
        return fWindingData[row];
    }

    int16_t& operator[](int row) {
        SkASSERT(row >= 0 && row < fWindingData.size());
        return fWindingData[row];
    }

    SK_ALWAYS_INLINE void addWinding(int row, int16_t delta) {
        SkASSERT(row >= 0 && row < fWindingData.size());
        fWindingData[row] += delta;
        this->markActive(row);
    }

    SK_ALWAYS_INLINE void addWindingRange(int startRow, int endRow, int16_t delta) {
        SkASSERT(startRow >= 0 && endRow <= fWindingData.size());
        if (startRow >= endRow) return;

        for (int r = startRow; r < endRow; ++r) {
            fWindingData[r] += delta;
        }

        int startWord = startRow >> 5;
        int endWord = (endRow - 1) >> 5;

        if (startWord == endWord) {
            uint32_t mask = ((1u << (endRow - startRow)) - 1) << (startRow & 31);
            fActiveRows[startWord] |= mask;
        } else {
            fActiveRows[startWord] |= (~0u << (startRow & 31));
            for (int w = startWord + 1; w < endWord; ++w) {
                fActiveRows[w] = ~0u;
            }
            fActiveRows[endWord] |= ((1u << ((endRow - 1) & 31)) - 1) | (1u << ((endRow - 1) & 31));
        }
    }

    const SkTDArray<int16_t>& data()        const { return fWindingData;        }
    const SkTDArray<uint32_t>& activeRows() const { return fActiveRows;         }
    int32_t size()                          const { return fWindingData.size(); }

private:
    SK_ALWAYS_INLINE void markActive(int row) {
        SkASSERT(row >= 0 && row < fWindingData.size());
        fActiveRows[row >> 5] |= (1u << (row & 31));
    }

    SkTDArray<int16_t> fWindingData;
    SkTDArray<uint32_t> fActiveRows;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_WindingHistogram_DEFINED
