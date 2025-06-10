/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathRaw_DEFINED
#define SkPathRaw_DEFINED

#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSpan.h"

#include <array>

struct SkPathRaw {
    SkSpan<const SkPoint>  fPoints;
    SkSpan<const uint8_t>  fVerbs;
    SkSpan<const SkScalar> fConics;
    SkRect                 fBounds;
    SkPathFillType         fFillType;
    bool                   fIsConvex;

    SkSpan<const SkPoint>  points() const { return fPoints; }
    SkSpan<const uint8_t>  verbs() const { return fVerbs; }
    SkSpan<const SkScalar> conics() const { return fConics; }
    SkRect bounds() const { return fBounds; }
    SkPathFillType fillType() const { return fFillType; }
    bool isConvex() const { return fIsConvex; }

    bool empty() const { return fVerbs.empty(); }

#ifdef DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

    struct IterRec {
        SkSpan<const SkPoint> pts;
        SkScalar              w;
        SkPathVerb            vrb;
    };

    class Iter {
        size_t                 vIndex, pIndex, cIndex;
        SkSpan<const SkPoint>  fPoints;
        SkSpan<const uint8_t>  fVerbs;
        SkSpan<const SkScalar> fConics;
        std::array<SkPoint, 2> fPointStorage;

    public:
        Iter(const SkPathRaw& p) : fPoints(p.points()), fVerbs(p.verbs()), fConics(p.conics()) {
            vIndex = pIndex = cIndex = 0;
        }
        Iter(SkSpan<const SkPoint> pts, SkSpan<const uint8_t> vbs, SkSpan<const SkScalar> cns)
            : fPoints(pts), fVerbs(vbs), fConics(cns)
        {
            vIndex = pIndex = cIndex = 0;
        }

        /*  Holds the current verb, and its associated points
         *  move:  pts[0]
         *  line:  pts[0..1]
         *  quad:  pts[0..2]
         *  conic: pts[0..2] w
         *  cubic: pts[0..3]
         *  close: pts[0..1] ... as if close were a line from pts[0] to pts[1]
         */
        std::optional<IterRec> next();
    };

    Iter iter() const {
        return Iter(*this);
    }

#if 0
    // 1) auto-closes each contour
    // 2) always returns an edge: line | quad | cubic
    class Edger {
        Iter                 m_iter;
        std::array<Point, 2> m_twoPts;
        PathVerb             m_lastVb;
        bool                 m_finished;
    public:
        Edger(const SkPathRaw& raw);
        std::optional<SkSpan<const Point>> next();
    };
#endif

    struct ContourRec {
        SkSpan<const SkPoint>  fPoints;
        SkSpan<const uint8_t>  fVerbs;
        SkSpan<const SkScalar> fConics;
    };

    class ContourIter {
        SkSpan<const SkPoint>  fPoints;
        SkSpan<const uint8_t>  fVerbs;
        SkSpan<const SkScalar> fConics;
    public:
        ContourIter(const SkPathRaw&);
        std::optional<ContourRec> next();
    };
};

#if 0
static inline int points_for_verb(PathVerb v) {
    static const uint8_t gVerbPointCount[] = { 1, 1, 2, 3, 0 };
    static_assert(std::size(gVerbPointCount) == (unsigned)PathVerb::_last_value+1);

    return gVerbPointCount[(unsigned)v];
}
#endif

#endif
