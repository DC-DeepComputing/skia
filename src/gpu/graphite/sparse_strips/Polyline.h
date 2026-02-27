/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_Polyline_DEFINED
#define skgpu_graphite_sparse_strips_Polyline_DEFINED

#include "src/gpu/graphite/sparse_strips/SparseStripTypes.h"
#include <cmath>
#include <utility>

namespace skgpu::graphite {

class Polyline {
public:
    void reset() {
        fPoints.clear();
    }

    void appendPoint(SkPoint pt) {
        if (fPoints.empty() || fPoints.back() != pt) {
            fPoints.push_back(pt);
        }
    }

    void appendSentinel() {
        if (!fPoints.empty() && !std::isnan(fPoints.back().fX)) {
            fPoints.push_back({SK_ScalarNaN, SK_ScalarNaN});
        }
    }

    class EdgeIterator {
    public:
        EdgeIterator(const SkPoint* pts, int index, int count)
            : fPts(pts), fIndex(index), fCount(count) {

            while (fIndex < fCount && std::isnan(fPts[fIndex].fX)) {
                fIndex++;
            }
            advanceToValidEdge();
        }

        std::pair<Edge, int> operator*() const {
            return { {fPts[fIndex], fPts[fIndex + 1]}, fIndex };
        }

        EdgeIterator& operator++() {
            fIndex++;
            advanceToValidEdge();
            return *this;
        }

        bool operator!=(const EdgeIterator& other) const {
            return fIndex != other.fIndex;
        }

    private:
        void advanceToValidEdge() {
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

    EdgeIterator begin() const {
        return EdgeIterator(fPoints.begin(), 0, fPoints.size());
    }

    EdgeIterator end() const {
        return EdgeIterator(fPoints.begin(), fPoints.size(), fPoints.size());
    }

    bool empty() const { return fPoints.empty(); }
    size_t count() const { return fPoints.size(); }

    const SkTDArray<SkPoint>& points() const { return fPoints; }

private:
    SkTDArray<SkPoint> fPoints;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_Polyline_DEFINED
