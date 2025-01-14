/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_GraphicsPipelineDesc_DEFINED
#define skgpu_graphite_GraphicsPipelineDesc_DEFINED

#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/UniquePaintParamsID.h"

namespace skgpu::graphite {

/**
 * GraphicsPipelineDesc represents the state needed to create a backend specific GraphicsPipeline,
 * minus the target-specific properties that can be inferred from the DrawPass and RenderPassTask.
 */
class GraphicsPipelineDesc {
public:
    GraphicsPipelineDesc() : fRenderStepID1(RenderStep::RenderStepID::kInvalid)
                           , fPaintID(UniquePaintParamsID::InvalidID()) {}
    GraphicsPipelineDesc(RenderStep::RenderStepID renderStepID, UniquePaintParamsID paintID)
        : fRenderStepID1(renderStepID)
        , fPaintID(paintID) {}
    bool isValid() const {
        return fRenderStepID1 != RenderStep::RenderStepID::kInvalid &&
               fPaintID != UniquePaintParamsID::InvalidID();
    }

    bool operator==(const GraphicsPipelineDesc& that) const {
        return fRenderStepID1 == that.fRenderStepID1 && fPaintID == that.fPaintID;
    }

    bool operator!=(const GraphicsPipelineDesc& other) const {
        return !(*this == other);
    }

    // Describes the geometric portion of the pipeline's program and the pipeline's fixed state
    // (except for renderpass-level state that will never change between draws).
    RenderStep::RenderStepID renderStepID1() const { return fRenderStepID1; }
    // UniqueID of the required PaintParams
    UniquePaintParamsID paintParamsID() const { return fPaintID; }

private:
    // Each RenderStep defines a fixed set of attributes and rasterization state, as well as the
    // shader fragments that control the geometry and coverage calculations. The RenderStep's shader
    // is combined with the rest of the shader generated from the PaintParams. Because each
    // RenderStep is fixed, its pointer can be used as a proxy for everything that it specifies in
    // the GraphicsPipeline.
    RenderStep::RenderStepID fRenderStepID1;
    UniquePaintParamsID fPaintID;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_GraphicsPipelineDesc_DEFINED
