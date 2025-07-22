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
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>

struct SkArc;
struct SkPathRaw;
class SkRRect;

/* Idea for new SkPath

 class SkPath {
    sk_sp<SkPath>   fData;      // immutable
    SkPathFillTYpe  fFillType;
    bool            fIsVolatile;
 ...
 };

 */

class SK_API SkPathData : public SkRefCnt {
public:
    /**
     *  Create a new pathdata with the specified segments.
     *
     *  The points and weights arrays are read in order, based on the sequence of verbs.
     *
     *  Move    1 point
     *  Line    1 point
     *  Quad    2 points
     *  Conic   2 points and 1 weight
     *  Cubic   3 points
     *  Close   0 points
     *
     *  If an illegal sequence of verbs is encountered, or the specified number of points
     *  or weights is not sufficient given the verbs, null is returned.
     *
     *  A legal sequence of verbs consists of any number of Contours. A contour always begins
     *  with a Move verb, followed by 0 or more segments: Line, Quad, Conic, Cubic, followed
     *  by an optional Close.
     */
    static sk_sp<SkPathData> Make(SkSpan<const SkPoint> pts,
                                  SkSpan<const SkPathVerb> verbs,
                                  SkSpan<const float> conics = {});

    static sk_sp<SkPathData> Empty();
    static sk_sp<SkPathData> Rect(const SkRect&,
                                  SkPathDirection = SkPathDirection::kCW,
                                  unsigned startIndex = 0);
    static sk_sp<SkPathData> Oval(const SkRect&,
                                  SkPathDirection = SkPathDirection::kCW,
                                  unsigned startIndex = 0);
    static sk_sp<SkPathData> RRect(const SkRRect&,
                                   SkPathDirection dir = SkPathDirection::kCW,
                                   unsigned startIndex = 0);
    static sk_sp<SkPathData> Polygon(SkSpan<const SkPoint> pts, bool isClosed);
    static sk_sp<SkPathData> Line(SkPoint a, SkPoint b) {
        return Polygon({a, b}, false);
    }

    /**
     * Compares a and b; returns true if all their values are equal..
     */
    friend SK_API bool operator==(const SkPathData& a, const SkPathData& b);
    friend bool operator!=(const SkPathData& a, const SkPathData& b) {
        return !(a == b);
    }

    SkSpan<const SkPoint> points() const;
    SkSpan<const SkPathVerb> verbs() const;
    SkSpan<const float> conics() const;
    SkRect bounds() const;

    uint8_t getSegmentMask() const; // SkPathSegmentMask

    SkPathRaw raw(SkPathFillType) const;

    /**
     * Return true if the path contains no points or verbs
     */
    bool isEmpty() const;

    /** Returns true if the pathdata is convex. If necessary, it will first compute the convexity.
     */
    bool isConvex() const;

    /**
     * If the path is recconized as an oval, return its bounds. If not, return {}.
     */
    std::optional<SkRect> isOval() const;

    /**
     * If the path is recconized as a round-rect, return it. If not, return {}.
     */
    std::optional<SkRRect> isRRect() const;

    /**
     * If the path is recconized as an arc, return it. If not, return {}.
     */
    std::optional<SkArc> isArc() const;

    /**
     * Returns two points if SkPath contains only one line. If not, return {}.
     */
    std::optional<std::array<SkPoint, 2>> isLine() const;

    /**
     * If the pathdata is recognized as a rect, return it and its direction and open/closed.
     * If not, return {}
     */
    struct RectInfo {
        SkRect fRect;
        SkPathDirection fDirection;
        bool fIsClosed;
    };
    std::optional<RectInfo> isRect() const;

    /**
     * Returns the approximate byte size of the SkPathData in memory.
     */
    size_t approximateBytesUsed() const;

    sk_sp<SkPathData> transform(const SkMatrix&) const;
    sk_sp<SkPathData> offset(SkVector) const;

private:
    SkSpan<const SkPoint>    fPoints;
    SkSpan<const float>      fConics;
    SkSpan<const SkPathVerb> fVerbs;
    SkRect                   fBounds;

    enum class IsA : uint8_t {
        kGeneral,
        kOval,
        kRRect,
        kArc,
    };

    IsA                          fIsA;
    uint8_t                      fSegmentMask;  // SkPathSegmentMask
    mutable std::atomic<uint8_t> fConvexity;    // SkPathConvexity

    // storage for Arc/RRect/etc.
    // storage for points, verbs, conicsweights

    SkPathData(size_t npts, size_t nvbs, size_t ncns);
    bool finishInit(const SkRect* bounds);
    static sk_sp<SkPathData> Alloc(size_t npts, size_t nvbs, size_t ncns);
    static sk_sp<SkPathData> Make(SkSpan<const SkPoint> pts,
                                  SkSpan<const SkPathVerb> verbs,
                                  SkSpan<const float> conics,
                                  const SkRect* boundsOrNull);
};

#endif
