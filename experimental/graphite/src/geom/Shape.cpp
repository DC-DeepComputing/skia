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
// SkPoint* v2_as_points(SkV2* verts) {
//     return reinterpret_cast<SkPoint*>(verts);
// }

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

} // anonymous namespace

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
        // Not actually convex or too big, so switch to a path
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
