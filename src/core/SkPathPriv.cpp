/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathTypes.h"
#include "include/private/SkPathRef.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkPoint_impl.h"
#include "src/core/SkPathPriv.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <optional>

/*
 Determines if path is a rect by keeping track of changes in direction
 and looking for a loop either clockwise or counterclockwise.

 The direction is computed such that:
  0: vertical up
  1: horizontal left
  2: vertical down
  3: horizontal right

A rectangle cycles up/right/down/left or up/left/down/right.

The test fails if:
  The path is closed, and followed by a line.
  A second move creates a new endpoint.
  A diagonal line is parsed.
  There's more than four changes of direction.
  There's a discontinuity on the line (e.g., a move in the middle)
  The line reverses direction.
  The path contains a quadratic or cubic.
  The path contains fewer than four points.
 *The rectangle doesn't complete a cycle.
 *The final point isn't equal to the first point.

  *These last two conditions we relax if we have a 3-edge path that would
   form a rectangle if it were closed (as we do when we fill a path)

It's OK if the path has:
  Several colinear line segments composing a rectangle side.
  Single points on the rectangle side.

The direction takes advantage of the corners found since opposite sides
must travel in opposite directions.

FIXME: Allow colinear quads and cubics to be treated like lines.
FIXME: If the API passes fill-only, return true if the filled stroke
       is a rectangle, though the caller failed to close the path.

 directions values:
    0x1 is set if the segment is horizontal
    0x2 is set if the segment is moving to the right or down
 thus:
    two directions are opposites iff (dirA ^ dirB) == 0x2
    two directions are perpendicular iff (dirA ^ dirB) == 0x1

 */
static int rect_make_dir(SkScalar dx, SkScalar dy) {
    return ((0 != dx) << 0) | ((dx > 0 || dy > 0) << 1);
}

// Quick check for a "trivial" rect, i.e. a rect created via SkPath::Rect(),
// SkPathBuilder::addRect(), etc.
static std::optional<SkPathPriv::RectContour> trivial_rect(SkSpan<const SkPoint> pts,
                                                           SkSpan<const SkPathVerb> vbs) {
    static constexpr std::array<SkPathVerb, 5> gTrivialVerbs = {
        SkPathVerb::kMove,
        SkPathVerb::kLine,
        SkPathVerb::kLine,
        SkPathVerb::kLine,
        SkPathVerb::kClose,
    };
    if (pts.size() != 4 ||
        vbs.size() != std::size(gTrivialVerbs) ||
        !std::equal(vbs.begin(), vbs.end(), std::begin(gTrivialVerbs))) {
        return std::nullopt;
    }

    // True if the vectors are axis-aligned and perpendicular.
    const auto right_angle_corner = [](const SkVector& a, const SkVector& b) {
        return a.fX == 0 ^ b.fX == 0 &&
               a.fY == 0 ^ b.fY == 0;
    };

    const SkVector v0 = pts[1] - pts[0],
                   v1 = pts[2] - pts[1],
                   v2 = pts[3] - pts[2],
                   v3 = pts[0] - pts[3];
    if (!right_angle_corner(v0, v1) ||
        !right_angle_corner(v1, v2) ||
        !right_angle_corner(v2, v3)) {
        return std::nullopt;
    }

    // Right angles + right verbs => trivial rect
    const SkRect rect = SkRect::MakeLTRB(pts[0].fX, pts[0].fY, pts[2].fX, pts[2].fY).makeSorted();
    const SkPathDirection dir = SkPoint::CrossProduct(v0, v1) > 0
        ? SkPathDirection::kCW
        : SkPathDirection::kCCW;

    return {{
        rect,
        true,
        dir,
        pts.size(),
        vbs.size(),
    }};
}

std::optional<SkPathPriv::RectContour> SkPathPriv::IsRectContour(SkSpan<const SkPoint> ptSpan,
                                                                 SkSpan<const SkPathVerb> vbSpan,
                                                                 uint32_t segmentMask,
                                                                 bool allowPartial) {
    if (segmentMask != kLine_SkPathSegmentMask ||
        ptSpan.size() < 4 ||
        vbSpan.size() < 4) {
        return {};
    }

    if (!allowPartial) {
        if (auto rc = trivial_rect(ptSpan, vbSpan)) {
            return rc;
        }
    }

    size_t currVerb = 0;
    const size_t verbCnt = vbSpan.size();

    int corners = 0;
    SkPoint closeXY;  // used to determine if final line falls on a diagonal
    SkPoint lineStart;  // used to construct line from previous point
    const SkPoint* firstPt = nullptr; // first point in the rect (last of first moves)
    const SkPoint* lastPt = nullptr;  // last point in the rect (last of lines or first if closed)
    SkPoint firstCorner;
    SkPoint thirdCorner;
    const SkPoint* pts = ptSpan.data();
    const SkPoint* savePts = nullptr; // used to allow caller to iterate through a pair of rects
    lineStart.set(0, 0);
    signed char directions[] = {-1, -1, -1, -1, -1};  // -1 to 3; -1 is uninitialized
    bool closedOrMoved = false;
    bool autoClose = false;
    bool insertClose = false;
    while (currVerb < verbCnt && (!allowPartial || !autoClose)) {
        SkPathVerb verb = insertClose ? SkPathVerb::kClose : vbSpan[currVerb];
        switch (verb) {
            case SkPathVerb::kClose:
                savePts = pts;
                autoClose = true;
                insertClose = false;
                [[fallthrough]];
            case SkPathVerb::kLine: {
                if (SkPathVerb::kClose != verb) {
                    lastPt = pts;
                }
                SkPoint lineEnd = SkPathVerb::kClose == verb ? *firstPt : *pts++;
                SkVector lineDelta = lineEnd - lineStart;
                if (lineDelta.fX && lineDelta.fY) {
                    return {}; // diagonal
                }
                if (!lineDelta.isFinite()) {
                    return {}; // path contains infinity or NaN
                }
                if (lineStart == lineEnd) {
                    break; // single point on side OK
                }
                int nextDirection = rect_make_dir(lineDelta.fX, lineDelta.fY); // 0 to 3
                if (0 == corners) {
                    directions[0] = nextDirection;
                    corners = 1;
                    closedOrMoved = false;
                    lineStart = lineEnd;
                    break;
                }
                if (closedOrMoved) {
                    return {}; // closed followed by a line
                }
                if (autoClose && nextDirection == directions[0]) {
                    break; // colinear with first
                }
                closedOrMoved = autoClose;
                if (directions[corners - 1] == nextDirection) {
                    if (3 == corners && SkPathVerb::kLine == verb) {
                        thirdCorner = lineEnd;
                    }
                    lineStart = lineEnd;
                    break; // colinear segment
                }
                directions[corners++] = nextDirection;
                // opposite lines must point in opposite directions; xoring them should equal 2
                switch (corners) {
                    case 2:
                        firstCorner = lineStart;
                        break;
                    case 3:
                        if ((directions[0] ^ directions[2]) != 2) {
                            return {};
                        }
                        thirdCorner = lineEnd;
                        break;
                    case 4:
                        if ((directions[1] ^ directions[3]) != 2) {
                            return {};
                        }
                        break;
                    default:
                        return {}; // too many direction changes
                }
                lineStart = lineEnd;
                break;
            }
            case SkPathVerb::kQuad:
            case SkPathVerb::kConic:
            case SkPathVerb::kCubic:
                return {}; // quadratic, cubic not allowed
            case SkPathVerb::kMove:
                if (allowPartial && !autoClose && directions[0] >= 0) {
                    insertClose = true;
                    currVerb -= 1;  // try move again afterwards
                    goto addMissingClose;
                }
                if (!corners) {
                    firstPt = pts;
                } else {
                    closeXY = *firstPt - *lastPt;
                    if (closeXY.fX && closeXY.fY) {
                        return {};   // we're diagonal, abort
                    }
                }
                lineStart = *pts++;
                closedOrMoved = true;
                break;
            default:
                SkDEBUGFAIL("unexpected verb");
                break;
        }
        currVerb += 1;
    addMissingClose:
        ;
    }
    // Success if 4 corners and first point equals last
    if (corners < 3 || corners > 4) {
        return {};
    }
    // check if close generates diagonal
    closeXY = *firstPt - *lastPt;
    if (closeXY.fX && closeXY.fY) {
        return {};
    }

    auto bounds = [](SkPoint a, SkPoint b) {
        SkRect r;
        r.set(a, b);
        return r;
    };

    return {{
        bounds(firstCorner, thirdCorner),
        autoClose,
        directions[0] == ((directions[1] + 1) & 3) ? SkPathDirection::kCW
                                                   : SkPathDirection::kCCW,
        savePts ? size_t(savePts - ptSpan.data()) : 0,
        currVerb,
    }};
}

bool SkPathPriv::IsNestedFillRects(const SkPathRaw& raw, SkRect rects[2], SkPathDirection dirs[2]) {
    SkPathDirection testDirs[2];
    SkRect testRects[2];

    SkSpan<const SkPoint> pts = raw.points();
    SkSpan<const SkPathVerb> vbs = raw.verbs();

    auto rc = IsRectContour(pts, vbs, raw.fSegmentMask, true);
    if (!rc) {
        return false;
    }

    testDirs[0] = rc->fDirection;
    testRects[0] = rc->fRect;
    pts = pts.subspan(rc->fPointsConsumed);
    vbs = vbs.subspan(rc->fVerbsConsumed);

    rc = IsRectContour(pts, vbs, raw.fSegmentMask, false);
    if (rc) {
        testDirs[1] = rc->fDirection;
        testRects[1] = rc->fRect;
        if (testRects[0].contains(testRects[1])) {
            if (rects) {
                rects[0] = testRects[0];
                rects[1] = testRects[1];
            }
            if (dirs) {
                dirs[0] = testDirs[0];
                dirs[1] = testDirs[1];
            }
            return true;
        }
        if (testRects[1].contains(testRects[0])) {
            if (rects) {
                rects[0] = testRects[1];
                rects[1] = testRects[0];
            }
            if (dirs) {
                dirs[0] = testDirs[1];
                dirs[1] = testDirs[0];
            }
            return true;
        }
    }
    return false;
}
