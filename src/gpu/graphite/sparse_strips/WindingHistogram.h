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

        // Note: Both shift amounts are in [0, 31]. Do not compute the mask as
        // `(1u << (endRow - startRow)) - 1`, which is undefined when the range spans an entire word
        // (e.g. [0, 32)) and in practice produces an empty mask.
        uint32_t startMask = ~0u << (startRow & 31);             // Bits [startRow & 31, 31]
        uint32_t endMask = ~0u >> (31 - ((endRow - 1) & 31));    // Bits [0, (endRow - 1) & 31]

        if (startWord == endWord) {
            fActiveRows[startWord] |= startMask & endMask;
        } else {
            fActiveRows[startWord] |= startMask;
            for (int w = startWord + 1; w < endWord; ++w) {
                fActiveRows[w] = ~0u;
            }
            fActiveRows[endWord] |= endMask;
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
