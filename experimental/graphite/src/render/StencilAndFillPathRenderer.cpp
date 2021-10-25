/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Renderer.h"

namespace skgpu {

namespace {

// TODO: Hand off to csmartdalton, this should roughly correspond to the fStencilPathProgram stage
// of the skgpu::v1::PathStencilCoverOp, using the PathWedgeTessellator.
class StencilWedgesRenderStep : public RenderStep {
public:
    StencilWedgesRenderStep() {}

    ~StencilWedgesRenderStep() override {}

    const char* name()            const override { return "stencil-wedges"; }
    bool        requiresStencil() const override { return true; }
    bool        requiresMSAA()    const override { return true; }

private:
};

// TODO: Hand off to csmartdalton, this should roughly correspond to the fCoverBBoxProgram stage
// of skgpu::v1::PathStencilCoverOp.
class FillBoundsRenderStep : public RenderStep {
public:
    FillBoundsRenderStep() {}

    ~FillBoundsRenderStep() override {}

    const char* name()            const override { return "fill-bounds"; }
    bool        requiresStencil() const override { return true; }
    bool        requiresMSAA()    const override { return false; }

private:
};

} // anonymous namespace

const Renderer& Renderer::StencilAndFillPath() {
    static const StencilWedgesRenderStep kStencil;
    // TODO: This could move into a header and be reused across renderers
    static const FillBoundsRenderStep kCover;
    static const Renderer kRenderer("stencil-and-fill", kStencil, kCover);

    return kRenderer;
}

} // namespace skgpu
