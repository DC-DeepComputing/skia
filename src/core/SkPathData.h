/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathData_DEFINED
#define SkPathData_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"

#include "include/private/SkPathRef.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>

struct SkPathRaw;

/* Idea for new SkPath

 class SkPath {
    // immutable
    sk_sp<SkPath>   fData;

    // mutable fields
    SkPathFillTYpe  fFillType;
    bool            fIsVolatile;
    ...
 };

 */

/*
 *  Immutable container for path geometry data: points, verbs, bounds, segment masks.
 *
 *  All of the public factories check for valid input
 *  - valid verb sequenc
 *  - corresponding # points
 *  - finite point and conic values
 *
 *  If any of these checks fail, null is returned.
 *
 *  A valid sequence of verbs (and corresponding points/conics) is:
 *
 *  Any number of contours (0 or more)
 *  ... Each contour must begin with a single Move verb
 *      ... followed by any number of segments: lines, quads, conics, cubics (0 or more)
 *      ... followed by 0 or 1 Close verb
 *
 *  A minor exception to these rules, is that the last contour may end with a single
 *  Move verb -- this will be ignored in the resulting PathData.
 *
 *  Given a valid verb sequence, there must be the corresponding number of points and
 *  conics to match. If there are more or fewer, null is returned.
 *
 *  Verb    points  conic_weights
 *  -----------------------------
 *  Move    1       0
 *  Line    1       0
 *  Quad    2       0
 *  Conic   2       1
 *  Cubic   3       0
 *  Close   0       0
 */
class SK_API SkPathData : public SkRefCnt {
public:
    static sk_sp<SkPathData> Make(SkSpan<const SkPoint> pts,
                                  SkSpan<const SkPathVerb> verbs,
                                  SkSpan<const float> conics = {});

    static sk_sp<SkPathData> Empty();
    static sk_sp<SkPathData> Rect(const SkRect&,
                                  SkPathDirection = SkPathDirection::kDefault,
                                  unsigned startIndex = 0);
    static sk_sp<SkPathData> Oval(const SkRect&,
                                  SkPathDirection = SkPathDirection::kDefault,
                                  unsigned startIndex = 1);

    static sk_sp<SkPathData> RRect(const SkRRect&, SkPathDirection, unsigned startIndex);
    static sk_sp<SkPathData> RRect(const SkRRect& rrect,
                                   SkPathDirection dir = SkPathDirection::kDefault) {
        return RRect(rrect, dir, dir == SkPathDirection::kCW ? 6 : 7);
    }
    static sk_sp<SkPathData> Polygon(SkSpan<const SkPoint> pts, bool isClosed);
    static sk_sp<SkPathData> Line(SkPoint a, SkPoint b) {
        return Polygon({a, b}, false);
    }

    friend SK_API bool operator==(const SkPathData& a, const SkPathData& b);
    friend bool operator!=(const SkPathData& a, const SkPathData& b) {
        return !(a == b);
    }

    SkSpan<const SkPoint> points() const { return fPoints; }
    SkSpan<const SkPathVerb> verbs() const { return fVerbs; }
    SkSpan<const float> conics() const { return fConics; }
    SkRect bounds() const { return fBounds; }
    uint8_t getSegmentMask() const { return fSegmentMask; }

    SkPathRaw raw(SkPathFillType) const;

    /**
     * Return true if the path contains no points or verbs
     */
    bool empty() const { return fVerbs.empty(); }

    /**
     * Returns true if the pathdata is convex. If necessary, it will first compute the convexity.
     */
    bool isConvex() const;

    /**
     * Returns two points if SkPath contains only one line. If not, return {}.
     */
    std::optional<std::array<SkPoint, 2>> asLine() const;

    /**
     * If the pathdata is recognized as a rect, return it and its direction and open/closed.
     * If not, return {}
     */
    std::optional<SkPathRectInfo> asRect() const;

    /**
     * If the path is recconized as an oval, return its bounds. If not, return {}.
     */
    std::optional<SkPathOvalInfo> asOval() const;

    /**
     * If the path is recconized as a round-rect, return it. If not, return {}.
     */
    std::optional<SkPathRRectInfo> asRRect() const;

    /**
     * Returns the approximate byte size of the SkPathData in memory.
     */
    size_t approximateBytesUsed() const;

    sk_sp<SkPathData> transform(const SkMatrix&) const;
    sk_sp<SkPathData> offset(SkVector) const;

private:
    friend class SkPathBuilder;

    SkSpan<SkPoint>    fPoints;
    SkSpan<float>      fConics;
    SkSpan<SkPathVerb> fVerbs;
    SkRect             fBounds;

    uint8_t                      fSegmentMask;  // SkPathSegmentMask
    mutable std::atomic<uint8_t> fConvexity;    // SkPathConvexity
    SkPathIsAType                fType;
    SkPathIsAData                fIsA {};

    //
    // Memory layout after this (assuming we're not empty)
    //
    //  [point data]
    //  [conic data]
    //  [verb  data]
    //

    SkPathData(size_t npts, size_t nvbs, size_t ncns);

    // internal finisher when building a PathData.
    // If the optional value is not present, it will be computed (else checked in debug mode).
    //
    // In particular, if bounds is not provided, it will be computed, and if it proves
    // to be non-finite, nullptr will be returned.
    sk_sp<SkPathData> finishInit(std::optional<SkRect>, std::optional<uint8_t> segmentMask);

    // If we know we're a special shape, call this after the normal initialization
    void setupIsA(SkPathIsAType, SkPathDirection dir, unsigned startIndex);

    static sk_sp<SkPathData> Alloc(size_t npts, size_t nvbs, size_t ncns);

    static sk_sp<SkPathData> MakeNoCheck(SkSpan<const SkPoint> pts,
                                         SkSpan<const SkPathVerb> verbs,
                                         SkSpan<const float> conics,
                                         std::optional<SkRect> bounds,
                                         std::optional<unsigned> segmentMask);
    static sk_sp<SkPathData> MakeNoCheck(const SkPathRaw&);
};

#endif
