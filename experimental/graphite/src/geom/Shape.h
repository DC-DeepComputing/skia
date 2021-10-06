/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_geom_Shape_DEFINED
#define skgpu_geom_Shape_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkSpan.h"

#include <array>

namespace skgpu::geom {

/**
 * Shape is effectively a std::variant over different geometric shapes, with the most complex
 * being an SkPath. The simpler shapes can either be constructed directly or extracted by analyzing
 * the path. It provides a canonical set of simplification routines and API for convexity, bounds,
 * containment, etc.
 */
class Shape {
    // NOTE: Points and lines are represented as polygons with a vertex count of 1 or 2 respectively
    // Shape can only store convex polygons up to 6 vertices and is implicitly closed.
    enum class Type : uint8_t {
        kEmpty, kRect, kRRect, kPolygon, kPath
    };
    static constexpr int kTypeCount = static_cast<int>(Type::kPath) + 1;

    // The polygon vertex count is chosen so that sizeof(Shape) == sizeof(SkRRect)+4.
    static constexpr size_t kMaxVertexCount = 6;

    Shape() {}
    Shape(const Shape& shape)            { *this = shape; }
    Shape(Shape&&) = delete;

    explicit Shape(const SkRect& rect)   { this->setRect(rect); }
    explicit Shape(const SkRRect& rrect) { this->setRRect(rrect); }
    explicit Shape(const SkPath& path)   { this->setPath(path); }

    explicit Shape(std::initializer_list<SkV2> vertices) { this->setPolygon(vertices); }
    explicit Shape(SkSpan<const SkV2> vertices)          { this->setPolygon(vertices); }

    ~Shape() { this->reset(); }

    // NOTE: None of the geometry types benefit from move semantics, so we don't bother
    // defining a move assignment operator for Shape.
    Shape& operator=(Shape&&) = delete;
    Shape& operator=(const Shape&);

    // Return the type of the data last stored in the Shape, which does not incorporate any possible
    // simplifications that could be applied to it (e.g. a degenerate round rect with 0 radius
    // corners is kRRect and not kRect).
    Type type() const { return fType; }

    bool isEmpty()    const { return fType == Type::kEmpty; }
    bool isRect()     const { return fType == Type::kRect; }
    bool isRRect()    const { return fType == Type::kRRect; }
    bool isPath()     const { return fType == Type::kPath; }
    bool isPolygon()  const { return fType == Type::kPolygon; }

    bool isPoint()    const { return this->isPolygon() && fVertexCount == 1; }
    bool isLine()     const { return this->isPolygon() && fVertexCount == 2; }
    bool isTriangle() const { return this->isPolygon() && fVertexCount == 3; }
    bool isQuad()     const { return this->isPolygon() && fVertexCount == 4; }

    bool inverted() const {
        SkASSERT(fType != Type::kPath || fInverted == fPath.isInverseFillType());
        return fInverted;
    }

    void setInverted(bool inverted) {
        if (fType == Type::kPath && inverted != fPath.isInverseFillType()) {
            fPath.toggleInverseFillType();
        }
        fInverted = inverted;
    }

    SkPathFillType fillType() const {
        if (fType == Type::kPath) {
            return fPath.getFillType(); // already incorporates invertedness
        } else {
            return fInverted ? SkPathFillType::kInverseEvenOdd : SkPathFillType::kEvenOdd;
        }
    }

    // True if the given bounding box is completely inside the shape, if it's conservatively treated
    // as a filled, closed shape.
    bool conservativeContains(const SkRect& rect) const;
    bool conservativeContains(const SkV2& point) const;

    // True if the underlying geometry represents a closed shape, without the need for an
    // implicit close.
    bool closed() const;

    // True if the underlying shape is known to be convex, assuming no other styles. If 'simpleFill'
    // is true, it is assumed the contours will be implicitly closed when drawn or used.
    bool convex(bool simpleFill = true) const;

    // The bounding box of the shape.
    SkRect bounds() const;

    // Convert the shape into a path that describes the same geometry.
    SkPath asPath() const;

    // Access the actual geometric description of the shape. May only access the appropriate type
    // based on what was last set.
    const SkRect&      rect()        const { SkASSERT(this->isRect()); return fRect; }
    const SkRRect&     rrect()       const { SkASSERT(this->isRRect()); return fRRect; }
    const SkPath&      path()        const { SkASSERT(this->isPath()); return fPath; }
    SkSpan<const SkV2> polygon()     const { return { this->vertices(), fVertexCount }; }
    const SkV2*        vertices()    const { SkASSERT(this->isPolygon()); return fPolygon.data(); }
    int                vertexCount() const { SkASSERT(this->isPolygon()); return fVertexCount; }

    // Update the geometry stored in the Shape and update its associated type to match. This
    // performs no simplification, so calling setRRect() with a round rect that has isRect() return
    // true will still be considered an rrect by Shape.
    //
    // These reset inversion to the default for the geometric type.
    void setRect(const SkRect& rect) {
        this->setType(Type::kRect);
        fRect = rect;
        fInverted = false;
    }
    void setRRect(const SkRRect& rrect) {
        this->setType(Type::kRRect);
        fRRect = rrect;
        fInverted = false;
    }
    void setPath(const SkPath& path) {
        if (fType == Type::kPath) {
            // Assign directly
            fPath = path;
        } else {
            // In-place initialize
            this->setType(Type::kPath);
            new (&fPath) SkPath(path);
        }
        fInverted = path.isInverseFillType();
    }

    // Returns false if 'vertices' is not convex or too large to fit inline, in which case the Shape
    // stores an SkPath representing the polygon formed by the vertices (and has kPath type).
    // Otherwise the vertex data is copied into the Shape and it has kPolygon type.
    bool setPolygon(SkSpan<const SkV2> vertices);
    bool setPolygon(std::initializer_list<SkV2> vertices) {
        return this->setPolygon({vertices.begin(), vertices.size()});
    }
    void setPoint(const SkV2& point) { SkAssertResult(this->setPolygon({point})); }
    void setLine(const SkV2& p0, const SkV2& p1) { SkAssertResult(this->setPolygon({p0, p1})); }
    void setTriangle(const SkV2& p0, const SkV2& p1, const SkV2& p2) {
        SkAssertResult(this->setPolygon({p0, p1, p2}));
    }
    bool setQuad(const SkV2& p0, const SkV2& p1, const SkV2& p2, const SkV2& p3) {
        return this->setPolygon({p0, p1, p2, p3});
    }

    void reset() {
        this->setType(Type::kEmpty);
        fInverted = false;
    }

private:
    void setType(Type type) {
        if (this->isPath() && type != Type::kPath) {
            fPath.~SkPath();
        }
        fType = type;
    }

    union {
        SkRect  fRect;
        SkRRect fRRect;
        SkPath  fPath;
        // TBD: Do we want this to be SkV3/4s so that z or w can be specified? It would either
        // decrease the polygon size or make Shape larger than an SkRRect
        std::array<SkV2, kMaxVertexCount> fPolygon;
    };

    Type    fType        = Type::kEmpty;
    uint8_t fVertexCount = 0; // only used when fType == kPolygon
    bool    fInverted    = false;
};

} // namespace skgpu::geom

#endif // skgpu_geom_Shape_DEFINED
