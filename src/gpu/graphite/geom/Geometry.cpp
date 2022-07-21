/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/geom/Geometry.h"

namespace skgpu::graphite {

Geometry& Geometry::operator=(const Geometry& geometry) {
    switch (geometry.type()) {
        case Type::kEmpty: this->setType(Type::kEmpty); break;
        case Type::kShape: this->setShape(geometry.shape()); break;
        case default: break;
    }
    return *this;
}

} // namespace skgpu::graphite
