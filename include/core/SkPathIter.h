/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathIter_DEFINED
#define SkPathIter_DEFINED

#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSpan.h"

#include <array>
#include <cstddef>
#include <optional>

/*
 *  Iterates through a path's data: points, verbs, conicWeights.
 *
 *  Unlike SkPath::Iter, this does NOT create artifical line-verbs when a contour
 *  is closed. The Close record contains the two points that close the contour,
 *  but there is no "Line" verb returned for it.
 *
 *  A convenient way to invoke this...
 *
 *  for (auto iter = path.iter(); auto rec = iter.next();) {
 *      switch (rec->fVerb) {
 *          ... rec->fPoints, rec->conicWeight()
 *      }
a *  }
 */
class SK_API SkPathIter {
public:
    struct Rec {
        SkSpan<const SkPoint> fPoints;      // span of points for this verb
        float                 fConicWeight; // only valid when verb is kConic, else undefined
        SkPathVerb            fVerb;        // the verb for this iteration

        float conicWeight() const {
            SkASSERT(fVerb == SkPathVerb::kConic);
            return fConicWeight;
        }
    };

    SkPathIter(SkSpan<const SkPoint> pts, SkSpan<const SkPathVerb> vbs, SkSpan<const float> cns)
        : pIndex(0), vIndex(0), cIndex(0)
        , fPoints(pts), fVerbs(vbs), fConics(cns)
    {}

    /*  Holds the current verb, and its associated points (and possibly conicWeight())
     *  move:  pts[0]
     *  line:  pts[0..1]
     *  quad:  pts[0..2]
     *  conic: pts[0..2] fConicWeight
     *  cubic: pts[0..3]
     *  close: pts[0..1] ... as if close were a line from pts[0] to pts[1]
     */
    std::optional<Rec> next();

private:
    size_t                   pIndex, vIndex, cIndex;
    SkSpan<const SkPoint>    fPoints;
    SkSpan<const SkPathVerb> fVerbs;
    SkSpan<const float>      fConics;
    std::array<SkPoint, 2>   fClosePointStorage;
};

class SkPathContourIter {
public:
    struct Rec {
        SkSpan<const SkPoint>    fPoints;
        SkSpan<const SkPathVerb> fVerbs;
        SkSpan<const float>      fConics;
    };

    SkPathContourIter(SkSpan<const SkPoint> pts, SkSpan<const SkPathVerb> vbs,
                      SkSpan<const float> cns)
        : fPoints(pts), fVerbs(vbs), fConics(cns)
    {}

    std::optional<Rec> next();

private:
    SkSpan<const SkPoint>    fPoints;
    SkSpan<const SkPathVerb> fVerbs;
    SkSpan<const float>      fConics;
};

#endif
