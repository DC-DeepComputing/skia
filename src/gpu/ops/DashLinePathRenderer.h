/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DashLinePathRenderer_DEFINED
#define DashLinePathRenderer_DEFINED

#include "src/gpu/GrPathRenderer.h"

class GrGpu;

namespace skgpu::v1 {

class DashLinePathRenderer : public GrPathRenderer {
public:
    DashLinePathRenderer() = default;

    const char* name() const final { return "DashLine"; }

private:
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const final;

    StencilSupport onGetStencilSupport(const GrStyledShape&) const final {
        return kNoSupport_StencilSupport;
    }

    bool onDrawPath(const DrawPathArgs&) final;

    sk_sp<GrGpu> fGpu;

    using INHERITED = GrPathRenderer;
};

} // namespace skgpu::v1

#endif // DashLinePathRenderer_DEFINED
