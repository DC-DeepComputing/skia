/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_Polyline_DEFINED
#define skgpu_graphite_sparse_strips_Polyline_DEFINED

#include "include/private/base/SkTDArray.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"

#include <cmath>
#include <utility>

namespace skgpu::graphite {

/*
 * Helper class that uses the watertight property of flattened paths to deduplicate resulting
 * points. Since any two adjacent points imply a continuous line segment, NaN values are injected
 * into the underlying array as sentinels to signal the end of a sub-path. During tile creation, the
 * raw index into the `fPoints` array is stored on the tile, allowing the coverage generation phase
 * to retrieve the correct parent line independent of the deduplication.
 */
class Polyline {
public:
    void reset() {
        fPoints.clear();
    }

    void reserve(int count) {
        fPoints.reserve(count);
    }

    void appendPoint(SkPoint pt) {
        SkASSERT(!std::isnan(pt.fX) && !std::isnan(pt.fY));
        if (fPoints.empty() || fPoints.back() != pt) {
            fPoints.push_back(pt);
        }
    }

    void appendSentinel() {
        if (!fPoints.empty() && !std::isnan(fPoints.back().fX)) {
            fPoints.push_back({SK_ScalarNaN, SK_ScalarNaN});
        }
    }

    class LineIterator {
    public:
        LineIterator(const SkPoint* pts, int index, int count)
            : fPts(pts), fIndex(index), fCount(count) {

            while (fIndex < fCount && std::isnan(fPts[fIndex].fX)) {
                fIndex++;
            }
            advanceToValidLine();
        }

        std::pair<Line, int> operator*() const {
            SkASSERT(fIndex + 1 < fCount);
            return { {fPts[fIndex], fPts[fIndex + 1]}, fIndex };
        }

        LineIterator& operator++() {
            fIndex++;
            advanceToValidLine();
            return *this;
        }

        bool operator!=(const LineIterator& other) const {
            return fIndex != other.fIndex;
        }

    private:
        void advanceToValidLine() {
            while (fIndex + 1 < fCount) {
                if (std::isnan(fPts[fIndex + 1].fX)) {
                    fIndex += 2;
                    while (fIndex < fCount && std::isnan(fPts[fIndex].fX)) {
                        fIndex++;
                    }
                    continue;
                }
                return;
            }
            fIndex = fCount;
        }

        const SkPoint* fPts;
        int fIndex;
        int fCount;
    };

    LineIterator begin() const {
        return LineIterator(fPoints.begin(), 0, fPoints.size());
    }

    LineIterator end() const {
        return LineIterator(fPoints.begin(), fPoints.size(), fPoints.size());
    }

    bool empty() const { return fPoints.empty(); }
    size_t count() const { return fPoints.size(); }

    const SkTDArray<SkPoint>& points() const { return fPoints; }

private:
    SkTDArray<SkPoint> fPoints;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_Polyline_DEFINED
