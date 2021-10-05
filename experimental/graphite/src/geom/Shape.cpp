/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/geom/Shape.h"

#include "src/core/SkPathPriv.h"
#include "src/core/SkRRectPriv.h"
#include "src/utils/SkPolyUtils.h"

namespace skgpu::geom {

namespace {

// Returns true if 'path' can be represented exactly as a polygon with at most 'count' vertices.
// Extraneous moveTos are ignored, but the path can only be a single contour and no curves. If
// the path would require more than 'count' vertices, false is returned; otherwise 'count' is
// updated to be number of points written to 'vertices'.
//
// If 'autoClose' is true, the path does not need to have a close verb or a line back to the start
// of the contour. If it's false, the path is only a polygon if it was already closed.
bool path_to_vertices(const SkPath& path, bool autoClose, SkV2* vertices, size_t* count) {
    // TODO: implement this, see for instance GrConvexPolyEffect
    return false;
}

void vertices_to_path(SkSpan<const SkV2> vertices, SkPathBuilder* builder) {
    if (vertices.empty()) {
        return;
    }

    builder->moveTo(vertices[0].x, vertices[0].y);
    if (vertices.size() == 1) {
        // A plain moveTo() or moveTo+close() does not match the expected path for a
        // point that is being dashed (see SkDashPath's handling of 0-length segments).
        builder->lineTo(vertices[0].x, vertices[0].y);
    } else {
        for (size_t i = 1; i < vertices.size(); ++i) {
            builder->lineTo(vertices[i].x, vertices[i].y);
        }
        if (vertices.size() > 2) {
            builder->close();
        }
    }
}

// SkPoint and SkV2 are laid out the same: (x,y) as two packed floats
const SkPoint* v2_as_points(const SkV2* verts) {
    return reinterpret_cast<const SkPoint*>(verts);
}
SkPoint* v2_as_points(SkV2* verts) {
    return reinterpret_cast<SkPoint*>(verts);
}

bool is_convex(SkSpan<const SkV2> vertices) {
    return vertices.size() <= 3 ||
           SkIsConvexPolygon(v2_as_points(vertices.begin()), static_cast<int>(vertices.size()));
}

SkRect poly_bounds(SkSpan<const SkV2> vertices) {
    SkRect bounds;
    bounds.setBounds(v2_as_points(vertices.begin()), static_cast<int>(vertices.size()));
    return bounds;
}

bool convex_poly_contains_point(SkSpan<const SkV2> vertices, const SkV2& point) {
    // TODO: implement this
    return false;
}

} // namespace

Shape& Shape::operator=(const Shape& shape) {
    switch (shape.type()) {
        case Type::kEmpty:
            this->reset();
            break;
        case Type::kRect:
            this->setRect(shape.rect());
            break;
        case Type::kRRect:
            this->setRRect(shape.rrect());
            break;
        case Type::kPolygon:
            this->setPolygon(shape.polygon());
            break;
        case Type::kPath:
            this->setPath(shape.path());
            break;
    }

    fInverted = shape.fInverted;
    return *this;
}

bool Shape::setPolygon(SkSpan<const SkV2> vertices) {
    if (vertices.size() > kMaxVertexCount || !is_convex(vertices)) {
        // Not actually convex, so switch to a path
        SkPathBuilder builder;
        vertices_to_path(vertices, &builder);
        this->setPath(builder.detach());
        return false;
    }

    this->setType(Type::kPolygon);
    for (size_t i = 0; i < vertices.size(); ++i) {
        fPolygon[i] = vertices[i];
    }
    fVertexCount = static_cast<uint8_t>(vertices.size());
    fInverted = false;
    return true;
}

bool Shape::simplifyPath(bool simpleFill) {
    SkASSERT(fType == Type::kPath);

    // The direction and start index used when a shape does not have a representable winding, so a
    // path cannot be simplified to such a representation if it's point/verb ordering does't match
    static constexpr SkPathDirection kDefaultDir   = SkPathDirection::kCW;
    static constexpr unsigned        kDefaultStart = 0;

    SkRect rect;
    SkRRect rrect;
    SkV2 pts[2];

    SkPathDirection dir;
    unsigned start;

    if (fPath.isEmpty()) {
        this->setType(Type::kEmpty);
        return true;
    } else if (fPath.isLine(v2_as_points(pts))) {
        return this->simplifyLine(pts[0], pts[1], simpleFill);
    } else if (SkPathPriv::IsRRect(fPath, &rrect, &dir, &start)) {
        if (simpleFill || (dir == kDefaultDir && start == kDefaultStart)) {
            return this->simplifyRRect(rrect, simpleFill);
        }
    } else if (SkPathPriv::IsOval(fPath, &rect, &dir, &start)) {
        if (simpleFill || (dir == kDefaultDir && start == kDefaultStart)) {
            return this->simplifyRRect(SkRRect::MakeOval(rect), simpleFill);
        }
    } else if (simpleFill) {
        // Use isRect() since it doesn't care about winding or if it's explicitly closed
        bool closed;
        if (fPath.isRect(&rect, &closed)) {
            return this->simplifyRect(rect, true);
        }
    } else if (SkPathPriv::IsSimpleRect(fPath, simpleFill, &rect, &dir, &start)) {
        if (dir == kDefaultDir && start == kDefaultStart) {
            return this->simplifyRect(rect, false);
        }
    } else if (fPath.getSegmentMasks() == SkPath::kLine_SegmentMask && fPath.isConvex()) {
        // A convex polygon that can be inlined (unless it has too many vertices or is not closed)
        SkV2 vertices[kMaxVertexCount];
        size_t count = kMaxVertexCount;
        if (path_to_vertices(fPath, simpleFill, vertices, &count)) {
            return this->simplifyPolygon(SkMakeSpan(vertices, count), simpleFill);
        }
    }

    // No further simplification for a path.
    return false;
}

bool Shape::simplifyRRect(const SkRRect& rrect, bool simpleFill) {
    if (rrect.isEmpty() || rrect.isRect()) {
        return this->simplifyRect(rrect.rect(), simpleFill);
    } else if (fType != Type::kRRect) {
        this->setType(Type::kRRect);
        fRRect = rrect;
        return true;
    } else {
        SkASSERT(fRRect == rrect); // shouldn't have changed
        return false;
    }
}

bool Shape::simplifyRect(const SkRect& rect, bool simpleFill) {
    if (!rect.width() || !rect.height()) {
        if (simpleFill) {
            // A zero area, filled shape so go straight to empty
            this->setType(Type::kEmpty);
            return true;
        } else {
            // A line from (l,t) to (r,b), which may actually be a point
            return this->simplifyLine({rect.fLeft, rect.fTop}, {rect.fRight, rect.fBottom}, false);
        }
    } else if (fType != Type::kRect) {
        this->setType(Type::kRect);
        fRect = rect;
        return true;
    } else {
        SkASSERT(fRect == rect); // shouldn't have changed
        return false;
    }
}

bool Shape::simplifyLine(const SkV2& p1, const SkV2& p2, bool simpleFill) {
    if (simpleFill) {
        this->setType(Type::kEmpty);
        return true;
    } else if (p1 == p2) {
        return this->simplifyPoint(p1, false);
    } else if (fType != Type::kPolygon) {
        this->setType(Type::kPolygon);
        fVertexCount = 2;
        fPolygon[0] = p1;
        fPolygon[1] = p2;
        return true;
    } else {
        // If starting as a line, the provided points should already be set
        SkASSERT(fVertexCount == 2 && fPolygon[0] == p1 && fPolygon[1] == p2);
        return false;
    }
}

bool Shape::simplifyPoint(const SkV2& point, bool simpleFill) {
    if (simpleFill) {
        this->setType(Type::kEmpty);
        return true;
    } else if (fType != Type::kPolygon) {
        this->setType(Type::kPolygon);
        fVertexCount = 1;
        fPolygon[0] = point;
        return true;
    } else {
        // If starting as a point, the provided position should already be set
        SkASSERT(fVertexCount == 1 && fPolygon[0] == point);
        return false;
    }
}

bool Shape::simplifyPolygon(SkSpan<SkV2> vertices, bool simpleFill) {
    SkASSERT(vertices.size() < kMaxVertexCount && is_convex(vertices));
    if (vertices.empty()) {
        this->setType(Type::kEmpty);
        return true;
    } else if (vertices.size() == 1) {
        return this->simplifyPoint(vertices[0], simpleFill);
    } else if (vertices.size() == 2) {
        return this->simplifyLine(vertices[0], vertices[1], simpleFill);
    } else if (fType != Type::kPolygon) {
        this->setType(Type::kPolygon);
        fVertexCount = static_cast<uint8_t>(vertices.size());
        for (size_t i = 0; i < fVertexCount; ++i) {
            fPolygon[i] = vertices[i];
        }
        return true;
    } else {
        // TBD - We could iterate the vertices and look for duplicates, but may not be worth it
        SkASSERT(fVertexCount == vertices.size());
        return false;
    }
}

bool Shape::simplify(bool simpleFill) {
    switch (fType) {
        case Type::kEmpty:   return false;
        case Type::kRect:    return this->simplifyRect(fRect, simpleFill);
        case Type::kRRect:   return this->simplifyRRect(fRRect, simpleFill);
        case Type::kPolygon: return this->simplifyPolygon(this->polygon(), simpleFill);
        case Type::kPath:    return this->simplifyPath(simpleFill);
    }
    SkUNREACHABLE;
}

bool Shape::conservativeContains(const SkRect& rect) const {
    switch (fType) {
        case Type::kEmpty:   return false;
        case Type::kRect:    return fRect.contains(rect);
        case Type::kRRect:   return fRRect.contains(rect);
        case Type::kPolygon:
            return convex_poly_contains_point(this->polygon(), {rect.fLeft,  rect.fTop})    &&
                   convex_poly_contains_point(this->polygon(), {rect.fLeft,  rect.fBottom}) &&
                   convex_poly_contains_point(this->polygon(), {rect.fRight, rect.fTop})    &&
                   convex_poly_contains_point(this->polygon(), {rect.fRight, rect.fBottom});
        case Type::kPath:    return fPath.conservativelyContainsRect(rect);
    }
    SkUNREACHABLE;
}

bool Shape::conservativeContains(const SkV2& point) const {
    switch (fType) {
        case Type::kEmpty:   return false;
        case Type::kRect:    return fRect.contains(point.x, point.y);
        case Type::kRRect:   return SkRRectPriv::ContainsPoint(fRRect, {point.x, point.y});
        case Type::kPolygon: return convex_poly_contains_point(this->polygon(), point);
        case Type::kPath:    return fPath.contains(point.x, point.y);
    }
    SkUNREACHABLE;
}

bool Shape::closed() const {
    switch (fType) {
        case Type::kEmpty:   return true;
        case Type::kRect:    return true;
        case Type::kRRect:   return true;
        case Type::kPolygon: return fVertexCount > 2;
        case Type::kPath:    return SkPathPriv::IsClosedSingleContour(fPath);
    }
    SkUNREACHABLE;
}

bool Shape::convex(bool simpleFill) const {
    if (this->isPath()) {
        // SkPath.isConvex() really means "is this path convex were it to be closed".
        return (simpleFill || fPath.isLastContourClosed()) && fPath.isConvex();
    } else {
        // Every other shape type is convex by construction (since non-convex polygons are
        // forced over to the kPath type). This does consider points and lines as convex.
        return true;
    }
}

SkRect Shape::bounds() const {
    // Bounds where left == bottom or top == right can indicate a line or point shape. We return
    // inverted bounds for a truly empty shape.
    static constexpr SkRect kInverted = SkRect::MakeLTRB(1, 1, -1, -1);
    switch (fType) {
        case Type::kEmpty:   return kInverted;
        case Type::kRect:    return fRect.makeSorted();
        case Type::kRRect:   return fRRect.getBounds();
        case Type::kPolygon: return fVertexCount ? poly_bounds(this->polygon()) : kInverted;
        case Type::kPath:    return fPath.getBounds();
    }
    SkUNREACHABLE;
}

SkPath Shape::asPath() const {
    if (fType == Type::kPath) {
        return fPath;
    }

    SkPathBuilder builder(this->fillType());
    switch (fType) {
        case Type::kEmpty:   /* do nothing */                             break;
        case Type::kRect:    builder.addRect(fRect);                      break;
        case Type::kRRect:   builder.addRRect(fRRect);                    break;
        case Type::kPolygon: vertices_to_path(this->polygon(), &builder); break;
        case Type::kPath:    SkUNREACHABLE;
    }
    return builder.detach();
}

} // namespace skgpu::geom
