/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Renderer.h"

#include "experimental/graphite/src/ContextUtils.h"
#include "experimental/graphite/src/DrawWriter.h"
#include "experimental/graphite/src/UniformManager.h"
#include "experimental/graphite/src/geom/Shape.h"
#include "experimental/graphite/src/geom/Transform_graphite.h"
#include "src/gpu/BufferWriter.h"

namespace skgpu {

namespace {

// TODO: Hand off to csmartdalton, this should roughly correspond to the fStencilFanProgram and
// simple triangulator shader stage of the skgpu::v1::PathStencilCoverOp
/*
class StencilFanRenderStep : public RenderStep {
public:
    StencilFanRenderStep() {}

    ~StencilFanRenderStep() override {}

    const char* name()            const override { return "stencil-fan"; }
    bool        requiresStencil() const override { return true; }
    bool        requiresMSAA()    const override { return true; }
    bool        performsShading() const override { return false; }

private:
};
*/

// TODO: Hand off to csmartdalton, this should roughly correspond to the fStencilPathProgram stage
// of skgpu::v1::PathStencilCoverOp using the PathCurveTessellator
/*
class StencilCurvesRenderStep : public RenderStep {
public:
    StencilCurvesRenderStep() {}

    ~StencilCurvesRenderStep() override {}

    const char* name()            const override { return "stencil-curves"; }
    bool        requiresStencil() const override { return true;  }
    bool        requiresMSAA()    const override { return true;  }
    bool        performsShading() const override { return false; }

private:
};
*/

// TODO: Hand off to csmartdalton, this should roughly correspond to the fCoverBBoxProgram stage
// of skgpu::v1::PathStencilCoverOp.
class FillBoundsRenderStep final : public RenderStep {
public:
    // TODO: Will need to add kRequiresStencil when we support specifying stencil settings and
    // the Renderer includes the stenciling step first.
    FillBoundsRenderStep()
            : RenderStep(Flags::kPerformsShading,
                         /*uniforms=*/{{"localToDevice", SLType::kFloat4x4}},
                         PrimitiveType::kTriangleStrip,
                         /*vertexAttrs=*/{{"position", VertexAttribType::kFloat2, SLType::kFloat2}},
                         /*instanceAttrs=*/{}) {}

    ~FillBoundsRenderStep() override {}

    const char* name() const override { return "fill-bounds"; }

    const char* vertexMSL() const override {
        return "out.position = uniforms.localToDevice * float4(vtx.position, 0.0, 1.0);\n";
        // return "out.position.xy = vtx.position;\n"
        //        "out.position.zw = float2(0.0, 1.0);\n";
    }

    void writeVertices(DrawWriter* writer,  const Transform&, const Shape& shape) const override {
        // TODO: For now the transform is handled as a uniform so writeVertices ignores it, but
        // for something as simple as the bounding box, CPU transformation might be best.
        writer->appendVertices(4)
               .writeQuad(VertexWriter::TriStripFromRect(shape.bounds().asSkRect()));
        // Since we upload 4 dynamic verts as a triangle strip, we need to actually draw them
        // otherwise the next writeVertices() call would get connected to our verts.
        // TODO: Primitive restart? Just use indexed drawing? Just write 6 verts?
        writer->flush();
        // TODO: triangle strip/fan is actually tricky to merge correctly, since we want strips
        // for everything that was appended by the RenderStep, but no connection across RenderSteps,
        // so either we need a way to end it here, or switch to instance rendering w/o instance
        // data so that vertices are still clustered appripriately. But that would require updating
        // the DrawWriter to support appending both vertex and instance data simultaneously, which
        // would need to return 2 vertex writers?
    }

    sk_sp<UniformData> writeUniforms(Layout layout, const Transform& localToDevice, const Shape&) const override {
        // TODO: Given that a RenderStep has its own uniform binding slot, these offsets never change
        // so we could cache them per layout.
        UniformManager mgr(layout);
        size_t dataSize = mgr.writeUniforms(this->uniforms(), nullptr, nullptr, nullptr);
        sk_sp<UniformData> transformData = UniformData::Make((int) this->numUniforms(), this->uniforms().data(), dataSize);

        const void* transform[1] = {static_cast<const void*>(&localToDevice.matrix())};
        mgr.writeUniforms(this->uniforms(), transform, transformData->offsets(), transformData->data());
        return transformData;
    }
};

} // anonymous namespace

const Renderer& Renderer::StencilAndFillPath() {
    // TODO: Uncomment and include in kRenderer to draw flattened paths instead of bboxes
    // static const StencilFanRenderStep kStencilFan;
    // TODO: Uncomment and include in kRenderer to draw curved paths
    // static const StencilCurvesRenderStep kStencilCurves;
    // TODO: This could move into a header and be reused across renderers
    static const FillBoundsRenderStep kCover;
    static const Renderer kRenderer("stencil-and-fill",
                                    /*&kStencilFan,*/ /*&kStencilCurves,*/ &kCover);

    return kRenderer;
}

} // namespace skgpu
