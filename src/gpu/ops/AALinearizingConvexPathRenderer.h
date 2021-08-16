/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AALinearizingConvexPathRenderer_DEFINED
#define AALinearizingConvexPathRenderer_DEFINED

#include "src/gpu/GrPathRenderer.h"

namespace skgpu::v1 {

class AALinearizingConvexPathRenderer : public GrPathRenderer {
public:
    AALinearizingConvexPathRenderer() = default;

    const char* name() const final { return "AALinear"; }

private:
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const final;

    bool onDrawPath(const DrawPathArgs&) final;

    using INHERITED = GrPathRenderer;
};

} // namespace skgpu::v1

#endif // AALinearizingConvexPathRenderer_DEFINED
