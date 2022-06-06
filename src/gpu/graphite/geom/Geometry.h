/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_Geometry_DEFINED
#define skgpu_graphite_geom_Geometry_DEFINED

#include "include/core/SkVertices.h"
#include "src/gpu/graphite/geom/Shape.h"

namespace skgpu::graphite {

/**
 * Geometry is a container that can house Shapes, Vertices, and eventually text subruns.
 */
class Geometry {
public:

enum class GeometryType : uint8_t {
  Empty, Shape, Vertices
};

Geometry() {}
Geometry(const Shape& shape) {
  setShape(shape);
}
~Geometry() { setGeometryType(GeometryType::Empty); }

void setShape(const Shape& shape) {
  setGeometryType(GeometryType::Shape);
  fShape = shape;
}
// void setVertices(const SkVertices&);

const Shape& getShape() const { return fShape; }
// const SkVertices& getVertices() const;

bool isShape() const { return fGeometryType == GeometryType::Shape; }
bool isVertices() const { return fGeometryType == GeometryType::Vertices; }

// Not currently referenced by anything, but could be useful in the future.
GeometryType getGeometryType() const { return fGeometryType; }

// Query bounds?

private:

GeometryType fGeometryType = GeometryType::Empty;
void setGeometryType(GeometryType geometryType) { fGeometryType = geometryType; }

union {
    Shape fShape;
    // SkVertices fVertices;
};

};
} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_Geometry_DEFINED

/** Questions:
 * Should Geometry have one private variable that is a union?
 * Or an enum value that stores what type it contains?
 * I think.... both?
 * Does Geometry needto house 1 static member variable, similar to Shape?
 */

// TODO - Add text subruns, vertices functionality.
// As of right now, this is really just a wrapper for Shape.
