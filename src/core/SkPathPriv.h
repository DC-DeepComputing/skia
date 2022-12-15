/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * The SkPathPriv class allows Skia internals to use non-public SkPath methods and functions
 * without needing to add a lot of friend classes (which removes a lot of encapsulation).
 */

#ifndef SkPathPriv_DEFINED
#define SkPathPriv_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/SkIDChangeListener.h"
#include "include/private/SkPathEnums.h"
#include "include/private/SkPathRef.h"

#include <cstdint>
#include <utility>

class SkRRect;

static_assert(0 == static_cast<int>(SkPathFillType::kWinding), "fill_type_mismatch");
static_assert(1 == static_cast<int>(SkPathFillType::kEvenOdd), "fill_type_mismatch");
static_assert(2 == static_cast<int>(SkPathFillType::kInverseWinding), "fill_type_mismatch");
static_assert(3 == static_cast<int>(SkPathFillType::kInverseEvenOdd), "fill_type_mismatch");

class SkPathPriv {
public:

    static SkPathFirstDirection AsFirstDirection(SkPathDirection dir) {
        // since we agree numerically for the values in Direction, we can just cast.
        return (SkPathFirstDirection)dir;
    }

    static SkPathFirstDirection ComputeFirstDirection(const SkPath& path) {
        return path.computeFirstDirection();
    }

    static bool IsClosedSingleContour(const SkPath& path) {
        int verbCount = path.countVerbs();
        if (verbCount == 0)
            return false;
        int moveCount = 0;
        auto verbs = path.fPathRef->verbsBegin();
        for (int i = 0; i < verbCount; i++) {
            switch (verbs[i]) {
                case SkPath::Verb::kMove_Verb:
                    moveCount += 1;
                    if (moveCount > 1) {
                        return false;
                    }
                    break;
                case SkPath::Verb::kClose_Verb:
                    if (i == verbCount - 1) {
                        return true;
                    }
                    return false;
                default: break;
            }
        }
        return false;
    }

    // In some scenarios (e.g. fill or convexity checking all but the last leading move to are
    // irrelevant to behavior). SkPath::injectMoveToIfNeeded should ensure that this is always at
    // least 1.
    static int LeadingMoveToCount(const SkPath& path) {
        return path.leadingMoveToCount();
    }

    static void AddGenIDChangeListener(const SkPath& path, sk_sp<SkIDChangeListener> listener) {
        path.fPathRef->addGenIDChangeListener(std::move(listener));
    }

    static bool IsSimpleRect(const SkPath& path, bool isSimpleFill, SkRect* rect,
                             SkPathDirection* direction, unsigned* start) {
        return path.isSimpleRect(isSimpleFill, rect, direction, start);
    }

    /**
     * Creates a path from arc params using the semantics of SkCanvas::drawArc. This function
     * assumes empty ovals and zero sweeps have already been filtered out.
     */
    static void CreateDrawArcPath(SkPath* path, const SkRect& oval, SkScalar startAngle,
                                  SkScalar sweepAngle, bool useCenter, bool isFillNoPathEffect) {
        return SkPath::CreateDrawArcPath(path, oval, startAngle, sweepAngle,
                                         useCenter, isFillNoPathEffect);
    }


    static bool DrawArcIsConvex(SkScalar sweepAngle, bool useCenter, bool isFillNoPathEffect) {
        return SkPath::DrawArcIsConvex(sweepAngle, useCenter, isFillNoPathEffect);
    }

    static void ShrinkToFit(SkPath* path) {
        path->shrinkToFit();
    }

    /**
     * Returns a C++11-iterable object that traverses a path's verbs in order. e.g:
     *
     *   for (SkPath::Verb verb : SkPathPriv::Verbs(path)) {
     *       ...
     *   }
     */
    struct Verbs {
    public:
        Verbs(const SkPath& path) : fPathRef(path.fPathRef.get()) {}
        struct Iter {
            void operator++() { fVerb++; }
            bool operator!=(const Iter& b) { return fVerb != b.fVerb; }
            SkPath::Verb operator*() { return static_cast<SkPath::Verb>(*fVerb); }
            const uint8_t* fVerb;
        };
        Iter begin() { return Iter{fPathRef->verbsBegin()}; }
        Iter end() { return Iter{fPathRef->verbsEnd()}; }
    private:
        Verbs(const Verbs&) = delete;
        Verbs& operator=(const Verbs&) = delete;
        SkPathRef* fPathRef;
    };

    /**
      * Iterates through a raw range of path verbs, points, and conics. All values are returned
      * unaltered.
    */
    using RangeIter = SkPath::RangeIter;
    using Iterate = SkPath::Iterate;


    /**
     * Returns a pointer to the verb data.
     */
    static const uint8_t* VerbData(const SkPath& path) {
        return path.fPathRef->verbsBegin();
    }

    /** Returns a raw pointer to the path points */
    static const SkPoint* PointData(const SkPath& path) {
        return path.fPathRef->points();
    }

    /** Returns the number of conic weights in the path */
    static int ConicWeightCnt(const SkPath& path) {
        return path.fPathRef->countWeights();
    }

    /** Returns a raw pointer to the path conic weights. */
    static const SkScalar* ConicWeightData(const SkPath& path) {
        return path.fPathRef->conicWeights();
    }

    /** Returns true if the underlying SkPathRef has one single owner. */
    static bool TestingOnly_unique(const SkPath& path) {
        return path.fPathRef->unique();
    }

    // Won't be needed once we can make path's immutable (with their bounds always computed)
    static bool HasComputedBounds(const SkPath& path) {
        return path.hasComputedBounds();
    }

    static bool IsOval(const SkPath& path, SkRect* rect, SkPathDirection* dir, unsigned* start) {
        return path.isOval(rect, dir, start);
    }

    static bool IsRRect(const SkPath& path, SkRRect* rrect, SkPathDirection* dir,
                        unsigned* start) {
        return path.isRRect(rrect, dir, start);
    }

    /**
     *  Sometimes in the drawing pipeline, we have to perform math on path coordinates, even after
     *  the path is in device-coordinates. Tessellation and clipping are two examples. Usually this
     *  is pretty modest, but it can involve subtracting/adding coordinates, or multiplying by
     *  small constants (e.g. 2,3,4). To try to preflight issues where these optionations could turn
     *  finite path values into infinities (or NaNs), we allow the upper drawing code to reject
     *  the path if its bounds (in device coordinates) is too close to max float.
     */
    static bool TooBigForMath(const SkRect& bounds) {
        // This value is just a guess. smaller is safer, but we don't want to reject largish paths
        // that we don't have to.
        constexpr SkScalar scale_down_to_allow_for_small_multiplies = 0.25f;
        constexpr SkScalar max = SK_ScalarMax * scale_down_to_allow_for_small_multiplies;

        // use ! expression so we return true if bounds contains NaN
        return !(bounds.fLeft >= -max && bounds.fTop >= -max &&
                 bounds.fRight <= max && bounds.fBottom <= max);
    }
    static bool TooBigForMath(const SkPath& path) {
        return TooBigForMath(path.getBounds());
    }

    // Returns number of valid points for each SkPath::Iter verb
    static int PtsInIter(unsigned verb) {
        return SkPath::PtsInIter(verb);
    }

    static bool IsAxisAligned(const SkPath& path) {
        return path.isAxisAligned();
    }

    static bool AllPointsEq(const SkPoint pts[], int count) {
        return SkPath::AllPointsEq(pts, count);
    }

    static int LastMoveToIndex(const SkPath& path) { return path.fLastMoveToIndex; }

    static bool IsNestedFillRects(const SkPath& path, SkRect rect[2],
                                  SkPathDirection dirs[2] = nullptr) {
        return path.isNestedFillRects(rect, dirs);
    }

    static bool IsInverseFillType(SkPathFillType fill) {
        return (static_cast<int>(fill) & 2) != 0;
    }

    /** Returns equivalent SkPath::FillType representing SkPath fill inside its bounds.
     .

     @param fill  one of: kWinding_FillType, kEvenOdd_FillType,
     kInverseWinding_FillType, kInverseEvenOdd_FillType
     @return      fill, or kWinding_FillType or kEvenOdd_FillType if fill is inverted
     */
    static SkPathFillType ConvertToNonInverseFillType(SkPathFillType fill) {
        return (SkPathFillType)(static_cast<int>(fill) & 1);
    }

    static int GenIDChangeListenersCount(const SkPath& path) {
        return path.genIDChangeListenersCount();
    }

    static void UpdatePathPoint(SkPath* path, int index, const SkPoint& pt) {
        SkASSERT(index < path->countPoints());
        SkPathRef::Editor ed(&path->fPathRef);
        ed.writablePoints()[index] = pt;
        path->dirtyAfterEdit();
    }

    static SkPathConvexity GetConvexity(const SkPath& path) {
        return path.getConvexity();
    }
    static SkPathConvexity GetConvexityOrUnknown(const SkPath& path) {
        return path.getConvexityOrUnknown();
    }
    static void SetConvexity(const SkPath& path, SkPathConvexity c) {
        path.setConvexity(c);
    }
    static void ForceComputeConvexity(const SkPath& path) {
        path.setConvexity(SkPathConvexity::kUnknown);
        (void)path.isConvex();
    }

    static SkPath& ReversePathTo(SkPath& path, const SkPath& contour) {
        return path.reversePathTo(contour);
    }

    static void ReverseAddPath(SkPathBuilder* builder, const SkPath& reverseMe) {
        builder->privateReverseAddPath(reverseMe);
    }

    using EdgeIter = SkPath::EdgeIter;
};

#endif
