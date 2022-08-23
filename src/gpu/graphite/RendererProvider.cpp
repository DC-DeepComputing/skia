/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RendererProvider.h"
#include <memory>
#include <string_view>

#include "include/core/SkPathTypes.h"
#include "include/core/SkVertices.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/render/BitmapTextRenderStep.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/graphite/render/CoverBoundsRenderStep.h"
#include "src/gpu/graphite/render/MiddleOutFanRenderStep.h"
#include "src/gpu/graphite/render/SDFTextRenderStep.h"
#include "src/gpu/graphite/render/TessellateCurvesRenderStep.h"
#include "src/gpu/graphite/render/TessellateStrokesRenderStep.h"
#include "src/gpu/graphite/render/TessellateWedgesRenderStep.h"
#include "src/gpu/graphite/render/VerticesRenderStep.h"

namespace skgpu::graphite {

RendererProvider::RendererProvider() {
    // Single-step renderers don't share RenderSteps
    this->initSingleStepRenderer(
            &fConvexTessellatedWedges,
            std::make_unique<TessellateWedgesRenderStep>("convex", kDirectDepthGreaterPass));
    this->initSingleStepRenderer(&fTessellatedStrokes,
                                 std::make_unique<TessellateStrokesRenderStep>());
    for (bool a8 : {false, true}) {
        this->initSingleStepRenderer(&fBitmapText[BoolIndex(a8)],
                                     std::make_unique<BitmapTextRenderStep>(a8));
    }
    for (bool lcd : {false, true}) {
        this->initSingleStepRenderer(&fSDFText[BoolIndex(lcd)],
                                     std::make_unique<SDFTextRenderStep>(lcd));
    }
    for (PrimitiveType primType : {PrimitiveType::kTriangles, PrimitiveType::kTriangleStrip}) {
        for (bool color : {false, true}) {
            for (bool texCoords : {false, true}) {
                SkVertices::VertexMode mode =  primType == PrimitiveType::kTriangles ?
                        SkVertices::kTriangles_VertexMode : SkVertices::kTriangleStrip_VertexMode;
                this->initSingleStepRenderer
                        (&fVertices[VerticesIndex(mode, color, texCoords)],
                        std::make_unique<VerticesRenderStep>(primType, color, texCoords));
            }
        }
    }

    // The tessellating path renderers that use stencil can share the cover steps.
    std::unique_ptr<RenderStep> coverFill = std::make_unique<CoverBoundsRenderStep>(false);
    std::unique_ptr<RenderStep> coverInverse = std::make_unique<CoverBoundsRenderStep>(true);

    static const char* kTessVariants[4] =
            {"[winding]", "[evenodd]", "[inverse-winding]", "[inverse-evenodd]"};

    for (bool evenOdd : {false, true}) {
        // These steps can be shared by regular and inverse fills
        std::unique_ptr<RenderStep> stencilFan = std::make_unique<MiddleOutFanRenderStep>(evenOdd);
        std::unique_ptr<RenderStep> stencilCurve = std::make_unique<TessellateCurvesRenderStep>(evenOdd);
        std::unique_ptr<RenderStep> stencilWedge = evenOdd
                ? std::make_unique<TessellateWedgesRenderStep>("evenodd", kEvenOddStencilPass)
                : std::make_unique<TessellateWedgesRenderStep>("winding", kWindingStencilPass);

        for (bool inverse : {false, true}) {
            int typeIndex = 2 * inverse + evenOdd;
            SkASSERT(typeIndex == PathTypeIndex(static_cast<SkPathFillType>(typeIndex)));

            const RenderStep* coverStep = inverse ? coverInverse.get() : coverFill.get();

            std::string name = "StencilTessellatedCurvesAndTris";
            name += kTessVariants[typeIndex];
            fRenderers.push_back(new (&fStencilTessellatedCurves[typeIndex]) Renderer(
                    name, stencilFan.get(), stencilCurve.get(), coverStep));

            name = "StencilTessellatedWedges";
            name += kTessVariants[typeIndex];
            fRenderers.push_back(new (&fStencilTessellatedWedges[typeIndex]) Renderer(
                    name, stencilWedge.get(), coverStep));
        }

        fRenderSteps.push_back(std::move(stencilFan));
        fRenderSteps.push_back(std::move(stencilCurve));
        fRenderSteps.push_back(std::move(stencilWedge));
    }

    fRenderSteps.push_back(std::move(coverInverse));
    fRenderSteps.push_back(std::move(coverFill));
}

void RendererProvider::initSingleStepRenderer(void* field, std::unique_ptr<RenderStep> singleStep) {
    std::string name = "SimpleRenderer[";
    name += singleStep->name();
    name += "]";

    Renderer* renderer = new (field) Renderer(name, singleStep.get());
    fRenderSteps.push_back(std::move(singleStep));
    fRenderers.push_back(renderer);
}

} // namespace skgpu::graphite
