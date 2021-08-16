/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SmallPathRenderer_DEFINED
#define SmallPathRenderer_DEFINED

#include "src/gpu/GrPathRenderer.h"
#include "src/gpu/ops/GrOp.h"

class GrDrawOp;
class GrRecordingContext;
class GrStyledShape;

namespace skgpu::v1 {

class SmallPathRenderer : public GrPathRenderer {
public:
    SmallPathRenderer();
    ~SmallPathRenderer() override;

    const char* name() const final { return "Small"; }

    static GrOp::Owner createOp_TestingOnly(GrRecordingContext*,
                                            GrPaint&&,
                                            const GrStyledShape&,
                                            const SkMatrix& viewMatrix,
                                            bool gammaCorrect,
                                            const GrUserStencilSettings*);

private:
    class SmallPathOp;

    StencilSupport onGetStencilSupport(const GrStyledShape&) const override {
        return GrPathRenderer::kNoSupport_StencilSupport;
    }

    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;

    using INHERITED = GrPathRenderer;
};

} // namespace skgpu::v1

#endif // SmallPathRenderer_DEFINED
