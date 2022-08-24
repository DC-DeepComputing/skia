/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RendererProvider.h"

#include "include/core/SkPathTypes.h"
#include "include/core/SkVertices.h"
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
    auto addSingleStep = [&](Renderer* target, std::unique_ptr<RenderStep> singleStep) {
        std::string name = "SimpleRenderer[";
        name += singleStep->name();
        name += "]";

        *target = Renderer(name, singleStep.get());
        fRenderSteps.push_back(std::move(singleStep));
        fRenderers.push_back(target);
    };

    addSingleStep(&fConvexTessellatedWedges,
                 std::make_unique<TessellateWedgesRenderStep>("convex", kDirectDepthGreaterPass));
    addSingleStep(&fTessellatedStrokes,
                 std::make_unique<TessellateStrokesRenderStep>());
    for (bool a8 : {false, true}) {
        addSingleStep(&fBitmapText[a8], std::make_unique<BitmapTextRenderStep>(a8));
    }
    for (bool lcd : {false, true}) {
        addSingleStep(&fSDFText[lcd], std::make_unique<SDFTextRenderStep>(lcd));
    }
    for (PrimitiveType primType : {PrimitiveType::kTriangles, PrimitiveType::kTriangleStrip}) {
        for (bool color : {false, true}) {
            for (bool texCoords : {false, true}) {
                addSingleStep(&fVertices[4*((int) primType) + 2*color + texCoords],
                             std::make_unique<VerticesRenderStep>(primType, color, texCoords));
            }
        }
    }

    // The tessellating path renderers that use stencil can share the cover steps.
    auto coverFill = std::make_unique<CoverBoundsRenderStep>(false);
    auto coverInverse = std::make_unique<CoverBoundsRenderStep>(true);

    static const char* kTessVariants[4] =
            {"[winding]", "[evenodd]", "[inverse-winding]", "[inverse-evenodd]"};

    for (bool evenOdd : {false, true}) {
        // These steps can be shared by regular and inverse fills
        auto stencilFan = std::make_unique<MiddleOutFanRenderStep>(evenOdd);
        auto stencilCurve = std::make_unique<TessellateCurvesRenderStep>(evenOdd);
        auto stencilWedge = evenOdd
                ? std::make_unique<TessellateWedgesRenderStep>("evenodd", kEvenOddStencilPass)
                : std::make_unique<TessellateWedgesRenderStep>("winding", kWindingStencilPass);

        for (bool inverse : {false, true}) {
            int index = 2*inverse + evenOdd; // matches SkPathFillType
            const RenderStep* coverStep = inverse ? coverInverse.get() : coverFill.get();
            std::string variant = kTessVariants[index];

            fStencilTessellatedCurves[index] =
                    Renderer("StencilTessellatedCurvesAndTris" + variant,
                             stencilFan.get(), stencilCurve.get(), coverStep);
            fRenderers.push_back(&fStencilTessellatedCurves[index]);

            fStencilTessellatedWedges[index] =
                    Renderer("StencilTessellatedWedges" + variant,
                             stencilWedge.get(), coverStep);
            fRenderers.push_back(&fStencilTessellatedWedges[index]);
        }

        fRenderSteps.push_back(std::move(stencilFan));
        fRenderSteps.push_back(std::move(stencilCurve));
        fRenderSteps.push_back(std::move(stencilWedge));
    }

    fRenderSteps.push_back(std::move(coverInverse));
    fRenderSteps.push_back(std::move(coverFill));
}

} // namespace skgpu::graphite
