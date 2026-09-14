/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/pathops/SkPathOps.h"
#include "include/private/SkMacros.h"
#include "src/core/SkPathPriv.h"
#include "src/pathops/SkPathOpsConic.h"
#include "src/pathops/SkPathOpsCubic.h"
#include "src/pathops/SkPathOpsCurve.h"
#include "src/pathops/SkPathOpsPoint.h"
#include "src/pathops/SkPathOpsQuad.h"
#include "src/pathops/SkPathOpsTypes.h"

#include <algorithm>
#include <array>
#include <vector>

using std::vector;

struct Contour {
    enum class Direction {  // SkPathDirection doesn't have 'none' state
        kCCW = -1,
        kNone,
        kCW,
    };

    Contour(const SkRect& bounds, int lastStart, int verbStart)
        : fBounds(bounds)
        , fVerbStart(lastStart)
        , fVerbEnd(verbStart) {
    }

    vector<Contour*> fChildren;
    const SkRect fBounds;
    SkPoint fMinXY{SK_ScalarMax, SK_ScalarMax};
    const int fVerbStart;
    const int fVerbEnd;
    Direction fDirection{Direction::kNone};
    bool fContained{false};
    bool fReverse{false};
};

static unsigned VerbPtCount(SkPathVerb verb) {
    static const auto kPtCount = std::to_array<uint8_t>({ 1, 1, 2, 2, 3, 0 });
    unsigned index = static_cast<unsigned>(verb);
    SkASSERT(index < std::size(kPtCount));
    return kPtCount[index];
}

static int VerbPtIndex(SkPathVerb verb) {
    static const auto kPtIndex = std::to_array<int>({ 0, 1, 1, 1, 1, 0 });
    unsigned index = static_cast<unsigned>(verb);
    SkASSERT(index < std::size(kPtIndex));
    return kPtIndex[index];
}

static int contains_edge(const SkPoint pts[4],
                         SkPathVerb verb,
                         SkScalar weight,
                         const SkPoint& edge,
                         int* onCurveCount) {
    // Use the same Y-monotone crossings and endpoint convention as path contains.
    // Preserve AsWinding's sign convention. This count is not contour direction.
    return -SkPathPriv::Winding(pts, verb, weight, edge, onCurveCount);
}

static float conic_weight(const SkPath::IterRec& rec) {
    return rec.fVerb == SkPathVerb::kConic ? rec.conicWeight() : 1;
}

static SkPoint left_edge(const SkPoint pts[4], SkPathVerb verb, SkScalar weight) {
    SkASSERT(SkPathVerb::kLine <= verb && verb <= SkPathVerb::kCubic);
    SkPoint result;
    double t SK_INIT_TO_AVOID_WARNING;
    int roots = 0;
    if (SkPathVerb::kLine == verb) {
        result = pts[0].fX < pts[1].fX ? pts[0] : pts[1];
    } else if (SkPathVerb::kQuad == verb) {
        SkDQuad quad;
        quad.set(pts);
        if (!quad.monotonicInX()) {
            roots = SkDQuad::FindExtrema(&quad[0].fX, &t);
        }
        if (roots) {
            result = quad.ptAtT(t).asSkPoint();
        } else {
            result = pts[0].fX < pts[2].fX ? pts[0] : pts[2];
        }
    } else if (SkPathVerb::kConic == verb) {
        SkDConic conic;
        conic.set(pts, weight);
        if (!conic.monotonicInX()) {
            roots = SkDConic::FindExtrema(&conic[0].fX, weight, &t);
        }
        if (roots) {
            result = conic.ptAtT(t).asSkPoint();
        } else {
            result = pts[0].fX < pts[2].fX ? pts[0] : pts[2];
        }
    } else {
        SkASSERT(SkPathVerb::kCubic == verb);
        SkDCubic cubic;
        cubic.set(pts);
        if (!cubic.monotonicInX()) {
            double tValues[2];
            roots = SkDCubic::FindExtrema(&cubic[0].fX, tValues);
            SkASSERT(roots <= 2);
            for (int index = 0; index < roots; ++index) {
                SkPoint temp = cubic.ptAtT(tValues[index]).asSkPoint();
                if (0 == index || result.fX > temp.fX) {
                    result = temp;
                }
            }
        }
        if (roots) {
            result = cubic.ptAtT(t).asSkPoint();
        } else {
            result = pts[0].fX < pts[3].fX ? pts[0] : pts[3];
        }
    }
    return result;
}

class OpAsWinding {
public:
    enum class Edge {
        kInitial,
        kCompare,
    };

    OpAsWinding(const SkPath& path)
        : fPath(path) {
    }

    void contourBounds(vector<Contour>* containers) {
        SkRect bounds;
        bounds.setEmpty();
        int lastStart = 0;
        int verbStart = 0;
        // Use the same verb sequence as nextEdge() and getDirection(), including
        // implicit closing edges. Raw verb offsets drift when contours are open.
        SkPath::Iter iter(fPath, true);
        while (auto rec = iter.next()) {
            auto verb = rec->fVerb;
            auto pts = rec->fPoints;
            if (SkPathVerb::kMove == verb) {
                if (!bounds.isEmpty()) {
                    containers->emplace_back(bounds, lastStart, verbStart);
                    lastStart = verbStart;
               }
                bounds.setBounds({&pts[VerbPtIndex(SkPathVerb::kMove)],
                                  VerbPtCount(SkPathVerb::kMove)});
            }
            if (SkPathVerb::kLine <= verb && verb <= SkPathVerb::kCubic) {
                SkRect verbBounds;
                verbBounds.setBounds({&pts[VerbPtIndex(verb)], VerbPtCount(verb)});
                bounds.joinPossiblyEmptyRect(verbBounds);
            }
            ++verbStart;
        }
        if (!bounds.isEmpty()) {
            containers->emplace_back(bounds, lastStart, verbStart);
        }
    }

    Contour::Direction getDirection(Contour& contour) {
        SkPath::Iter iter(fPath, true);
        int verbCount = -1;

        SkScalar total_signed_area = 0;
        while (auto rec = iter.next()) {
            if (++verbCount < contour.fVerbStart) {
                continue;
            }
            if (verbCount >= contour.fVerbEnd) {
                continue;
            }
            if (SkPathVerb::kLine > rec->fVerb || rec->fVerb > SkPathVerb::kCubic) {
                continue;
            }

            SkSpan<const SkPoint> pts = rec->fPoints;
            switch (rec->fVerb) {
                case SkPathVerb::kLine:
                    total_signed_area += (pts[0].fY - pts[1].fY) * (pts[0].fX + pts[1].fX);
                    break;
                case SkPathVerb::kQuad:
                case SkPathVerb::kConic:
                    total_signed_area += (pts[0].fY - pts[2].fY) * (pts[0].fX + pts[2].fX);
                    break;
                case SkPathVerb::kCubic:
                    total_signed_area += (pts[0].fY - pts[3].fY) * (pts[0].fX + pts[3].fX);
                    break;
                default:
                    break;
            }
        }

        return total_signed_area < 0 ? Contour::Direction::kCCW: Contour::Direction::kCW;
    }

    int nextEdge(Contour& contour, Edge edge, int* onCurveCount = nullptr) {
        SkPath::Iter iter(fPath, true);
        int verbCount = -1;
        int winding = 0;
        while (auto rec = iter.next()) {
            const SkPathVerb verb = rec->fVerb;
            if (++verbCount < contour.fVerbStart) {
                continue;
            }
            if (verbCount >= contour.fVerbEnd) {
                continue;
            }
            if (SkPathVerb::kLine > verb || verb > SkPathVerb::kCubic) {
                continue;
            }

            SkSpan<const SkPoint> pts = rec->fPoints;
            if (edge == Edge::kCompare) {
                winding += contains_edge(
                        pts.data(), verb, conic_weight(*rec), contour.fMinXY, onCurveCount);
                continue;
            }
            bool horizontal = true;
            for (size_t index = 1; index <= VerbPtCount(verb); ++index) {
                if (pts[0].fY != pts[index].fY) {
                    horizontal = false;
                    break;
                }
            }
            if (horizontal) {
                continue;
            }
            SkASSERT(edge == Edge::kInitial);
            SkPoint minXY = left_edge(pts.data(), verb, conic_weight(*rec));
            if (minXY.fX > contour.fMinXY.fX) {
                continue;
            }
            if (minXY.fX == contour.fMinXY.fX) {
                if (minXY.fY != contour.fMinXY.fY) {
                    continue;
                }
            }
            contour.fMinXY = minXY;
        }
        return winding;
    }

    bool containerContains(Contour& contour, Contour& test) {
        // find outside point on lesser contour
        // arbitrarily, choose non-horizontal edge where point <= bounds left
        // note that if leftmost point is control point, may need tight bounds
        // to find edge with minimum-x
        if (SK_ScalarMax == test.fMinXY.fX) {
            this->nextEdge(test, Edge::kInitial);
        }
        int onCurveCount = 0;
        auto windingAt = [&](SkPoint p) {
            contour.fMinXY = p;
            onCurveCount = 0;
            return this->nextEdge(contour, Edge::kCompare, &onCurveCount);
        };
        int winding = windingAt(test.fMinXY);
        if (onCurveCount) {
            // Touching contours can share the initial query point. For contours
            // that do not cross, any point off the parent's boundary determines
            // containment. Try vertices and edge midpoints before giving up.
            SkPath::Iter iter(fPath, true);
            int verbCount = -1;
            while (auto rec = iter.next()) {
                if (++verbCount < test.fVerbStart) {
                    continue;
                }
                if (verbCount >= test.fVerbEnd) {
                    break;
                }
                auto verb = rec->fVerb;
                if (verb < SkPathVerb::kLine || verb > SkPathVerb::kCubic) {
                    continue;
                }
                auto pts = rec->fPoints;
                for (double t : {0.0, 0.5}) {
                    SkPoint p = (*CurvePointAtT[(int)verb])(pts.data(), conic_weight(*rec), t);
                    winding = windingAt(p);
                    if (!onCurveCount) {
                        test.fMinXY = p;
                        break;
                    }
                }
                if (!onCurveCount) {
                    break;
                }
            }
        }
        // A boundary point alone cannot determine strict containment.
        test.fContained = winding != 0;
        return onCurveCount == 0 && -1 <= winding && winding <= 1;
    }

    void inParent(Contour& contour, Contour& parent) {
        // move contour into sibling list contained by parent
        for (auto test : parent.fChildren) {
            if (test->fBounds.contains(contour.fBounds)) {
                inParent(contour, *test);
                return;
            }
        }
        // move parent's children into contour's children if contained by contour
        for (auto iter = parent.fChildren.begin(); iter != parent.fChildren.end(); ) {
            if (contour.fBounds.contains((*iter)->fBounds)) {
                contour.fChildren.push_back(*iter);
                iter = parent.fChildren.erase(iter);
                continue;
            }
            ++iter;
        }
        parent.fChildren.push_back(&contour);
    }

    bool checkContainerChildren(Contour* parent, Contour* child) {
        for (auto grandChild : child->fChildren) {
            if (!checkContainerChildren(child, grandChild)) {
                return false;
            }
        }
        if (parent) {
            if (!containerContains(*parent, *child)) {
                return false;
            }
        }
        return true;
    }

    bool markReverse(Contour* parent, Contour* child) {
        // Descendants must see the effective direction after this contour is reversed.
        child->fDirection = getDirection(*child);
        child->fReverse = parent && parent->fDirection == child->fDirection;
        if (child->fReverse) {
            child->fDirection = (Contour::Direction) -(int) child->fDirection;
        }
        bool reversed = child->fReverse;
        for (auto grandChild : child->fChildren) {
            reversed |= markReverse(grandChild->fContained ? child : parent, grandChild);
        }
        return reversed;
    }

    SkPath reverseMarkedContours(vector<Contour>& contours, SkPathFillType fillType) {
        SkPath::Iter iter(fPath, true);
        int verbCount = 0;

        SkPathBuilder result;
        result.setFillType(fillType);
        for (const Contour& contour : contours) {
            SkPathBuilder reverse;
            SkPathBuilder* temp = contour.fReverse ? &reverse : &result;
            for (; verbCount < contour.fVerbEnd; ++verbCount) {
                auto rec = iter.next();
                SkASSERT(rec);
                auto verb = rec->fVerb;
                auto pts = rec->fPoints;
                switch (verb) {
                    case SkPathVerb::kMove:
                        temp->moveTo(pts[0]);
                        break;
                    case SkPathVerb::kLine:
                        // close() below will supply this edge; preserve the usual
                        // representation for contours with an implicit closing line.
                        if (!iter.isCloseLine()) {
                            temp->lineTo(pts[1]);
                        }
                        break;
                    case SkPathVerb::kQuad:
                        temp->quadTo(pts[1], pts[2]);
                        break;
                    case SkPathVerb::kConic:
                        temp->conicTo(pts[1], pts[2], rec->conicWeight());
                        break;
                    case SkPathVerb::kCubic:
                        temp->cubicTo(pts[1], pts[2], pts[3]);
                        break;
                    case SkPathVerb::kClose:
                        temp->close();
                        break;
                }
            }
            if (contour.fReverse) {
                SkASSERT(temp == &reverse);
                SkPathPriv::ReverseAddPath(&result, reverse.detach());
            }
        }
        return result.detach();
    }

private:
    const SkPath& fPath;
};

std::optional<SkPath> AsWinding(const SkPath& path) {
    if (!path.isFinite()) {
        return {};
    }
    SkPathFillType fillType = path.getFillType();
    if (fillType == SkPathFillType::kWinding
            || fillType == SkPathFillType::kInverseWinding ) {
        return path;
    }
    fillType = path.isInverseFillType() ? SkPathFillType::kInverseWinding :
            SkPathFillType::kWinding;
    if (path.isEmpty() || path.isConvex()) {
        return path.makeFillType(fillType);
    }
    // count contours
    vector<Contour> contours;   // one per contour
    OpAsWinding winder(path);
    winder.contourBounds(&contours);
    if (contours.size() <= 1) {
        return path.makeFillType(fillType);
    }
    // create contour bounding box tree
    Contour sorted(SkRect(), 0, 0);
    for (auto& contour : contours) {
        winder.inParent(contour, sorted);
    }
    // if sorted has no grandchildren, no child has to fix its children's winding
    if (std::all_of(sorted.fChildren.begin(), sorted.fChildren.end(),
            [](const Contour* contour) -> bool { return contour->fChildren.empty(); } )) {
        return path.makeFillType(fillType);
    }
    // starting with outermost and moving inward, see if one path contains another
    for (auto contour : sorted.fChildren) {
        winder.nextEdge(*contour, OpAsWinding::Edge::kInitial);
        if (!winder.checkContainerChildren(nullptr, contour)) {
            return {};
        }
    }
    // starting with outermost and moving inward, mark paths to reverse
    bool reversed = false;
    for (auto contour : sorted.fChildren) {
        reversed |= winder.markReverse(nullptr, contour);
    }
    if (!reversed) {
        return path.makeFillType(fillType);
    }
    return winder.reverseMarkedContours(contours, fillType);
}
