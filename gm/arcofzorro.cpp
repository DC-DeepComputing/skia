/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/base/SkRandom.h"

#include <algorithm>
#include <vector>
#include <set>
#include <unordered_set>

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString getName() const override { return SkString("arcofzorro"); }

    SkISize getISize() override { return SkISize::Make(1000, 1000); }

    struct Point2D {
        bool operator==(Point2D other) const {
            return fX == other.fX && fY == other.fY;
        }

        int fX = 0;
        int fY = 0;
    };

    int area(std::array<Point2D, 4>& pts, int& numRects) {
        struct {
            bool operator()(Point2D a, Point2D b) const {
                if (a.fY == b.fY) { return a.fX < b.fX; }
                return a.fY < b.fY;
            }
        } customLess;
        std::sort(pts.begin(), pts.end(), customLess);

        if (pts[0].fY != pts[1].fY ||
            pts[0].fX != pts[2].fX ||
            pts[3].fY != pts[2].fY ||
            pts[3].fX != pts[1].fX) {
            return -1;
        }

        ++numRects;
        return (pts[1].fX - pts[0].fX) *  (pts[2].fY - pts[1].fY);
    }

    void doitBruteForce(const std::vector<Point2D>& pts) {
        int numPts = pts.size();
        std::array<Point2D, 4> quad;
        int maxArea = 0;
        int numRects = 0;
        for (int i = 0; i < numPts; ++i) {
            for (int j = i+1; j < numPts; ++j) {
                for (int k = j+1; k < numPts; ++k) {
                    for (int l = k+1; l < numPts; ++l) {
                        quad[0] = pts[i]; quad[1] = pts[j]; quad[2] = pts[k]; quad[3] = pts[l];
                        int t = area(quad, numRects);
                        if (maxArea < t) {
                            maxArea = t;
                        }
                    }
                }
            }
        }

        SkDebugf("max %d out of %d rects\n", maxArea, numRects);
    }

    void doit1(const std::vector<Point2D>& pts) {
        struct Foo {
            bool operator()(Point2D a, Point2D b) const {
                if (a.fY == b.fY) { return a.fX < b.fX; }
                return a.fY < b.fY;
            }
        } customLess;

        struct Hash {
            size_t operator() (Point2D p) const {
                return p.fY * 10000 + p.fX;
            }
        };

//        std::set<Point2D, Foo> lookup(pts.begin(), pts.end());

        int numPts = pts.size();

        std::set<Point2D, Foo> lookup;
        for (int i = 0; i < numPts; ++i) {
            lookup.insert(pts[i]);
        }

//        std::unordered_set<Point2D, Hash> lookup(pts.begin(), pts.end());

        int maxArea = 0;
        int numRects = 0;

        for (int i = 0; i < numPts; ++i) {
            for (int j = i+1; j < numPts; ++j) {
                Point2D p0 = pts[i];
                Point2D p1 = pts[j];

                if (p0.fX == p1.fX || p0.fY == p1.fY) {
                    continue;
                }

                if (lookup.find({ p0.fX, p1.fY }) == lookup.end() ||
                    lookup.find({ p1.fX, p0.fY }) == lookup.end()) {
                    continue;
                }

                if ((customLess)(p0, p1)) {
                    std::swap(p0, p1);
                }

                ++numRects;
                int area = (p1.fY - p0.fY) * (p1.fX - p0.fX);

                if (maxArea < area) {
                    maxArea = area;
                }
            }
        }

        SkDebugf("max %d out of %d rects\n", maxArea, numRects);

    }

    void onDraw(SkCanvas* canvas) override {

        const std::vector<Point2D> kPts {
                { 0, 0, },
                { 1, 0, },
                { 0, 1, },
                { 1, 1, },
                { 2, 2, },
                { 0, 2, },
                { 2, 0, },
        };
        const std::vector<Point2D> kPts2 {
                { 2,  1 },
                { 8,  1 },
                { 2,  4 },
                { 8,  4 },
                {-8,  1 },
                { 2, -2 },
                {-8, -2 },
                {-2,  8 },
                { 3,  3 },
                {-7,  3 },
                {-2, -2 },
                { 2,  6 },
                {-6,  0 },
        };

        //doitBruteForce(kPts2);
        doit1(kPts);

    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
