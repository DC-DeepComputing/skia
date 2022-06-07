/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/geom/Geometry.h"

namespace skgpu::graphite {

Geometry& Geometry::operator=(const Geometry& geometry) {
    switch (geometry.getGeometryType()) {
        case GeometryType::Empty: setGeometryType(GeometryType::Empty); break;                       break;
        case GeometryType::Shape: setShape(geometry.getShape()); break;
        case default: break;
    }
    return *this;
}

} // namespace skgpu::graphite
