/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/AnalyticRRectBlurRenderStep.h"

#include "include/core/SkM44.h"
#include "include/core/SkRRect.h"
#include "include/private/SkDebug.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/geom/AnalyticRRectBlurMask.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

namespace skgpu::graphite {

struct Vertex {
    // x and y determine our index into xBounds and yBounds respectively.
    // z and w determine if we should apply our bevel and the direction.
    int32_t fGridAndBevel[4];
    int32_t fCellIndex;
};

static constexpr int kVertexCount = 104;
static constexpr int kIndexCount = 162;

// We construct a 25 cell grid (5x5) for our rrect, with 6 distinct boundaries for each axis:
// - Outset blur padding: the provided rrect's bounding box outset by 3 sigma.
// - Inset edge: the provided rrect's bounding box inset by 3.5 sigma.
// - Safe bounds: The furthest point from the edge which is inset by the deepest corner radii on
//                for that edge, then inset by an additional 3 sigma.
//
// For each corner cell, we use the following template of 5 vertices and 3 triangles:
//
//      v0-----v1
//     / \  f0  |
//    /   \__   |
//   v4  f1  \  |
//   | \____  \ |
//   |  f2  \__\|
//   v3--------v2
//
// In the case that the sigma is very large or the radii leaves no room for a straight edge on an
// axis, the inset and safe bound are snapped to the middle that axis and will result in degenerate
// triangles which can be ignored.
static void write_vertex_buffer(VertexWriter writer) {
    if (!writer) return;

    // Corner 0: TL, cell 0.
    writer << Vertex{{0, 0,  1,  0}, 0}   // v0
           << Vertex{{1, 0,  0,  0}, 0}   // v1
           << Vertex{{1, 1,  0,  0}, 0}   // v2
           << Vertex{{0, 1,  0,  0}, 0}   // v3
           << Vertex{{0, 0,  0,  1}, 0};  // v4

    // Corner 1: TR, cell 4.
    writer << Vertex{{4, 0,  0,  0}, 4}   // v5
           << Vertex{{5, 0, -1,  0}, 4}   // v6
           << Vertex{{5, 0,  0,  1}, 4}   // v7
           << Vertex{{5, 1,  0,  0}, 4}   // v8
           << Vertex{{4, 1,  0,  0}, 4};  // v9

    // Corner 2: BR, cell 24.
    writer << Vertex{{4, 4,  0,  0}, 24}  // v10
           << Vertex{{5, 4,  0,  0}, 24}  // v11
           << Vertex{{5, 5,  0, -1}, 24}  // v12
           << Vertex{{5, 5, -1,  0}, 24}  // v13
           << Vertex{{4, 5,  0,  0}, 24}; // v14

    // Corner 3: BL, cell 20.
    writer << Vertex{{0, 4,  0,  0}, 20}  // v15
           << Vertex{{1, 4,  0,  0}, 20}  // v16
           << Vertex{{1, 5,  0,  0}, 20}  // v17
           << Vertex{{0, 5,  1,  0}, 20}  // v18
           << Vertex{{0, 5,  0, -1}, 20}; // v19

    // 21 quads for the remaining cells.
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            // Skip corners.
            if ((row == 0 && col == 0) || (row == 0 && col == 4) ||
                (row == 4 && col == 0) || (row == 4 && col == 4)) {
                continue;
            }

            int cId = row * 5 + col;
            int x0 = col;
            int x1 = col + 1;
            int y0 = row;
            int y1 = row + 1;

            writer << Vertex{{x0, y0, 0, 0}, cId}
                   << Vertex{{x1, y0, 0, 0}, cId}
                   << Vertex{{x1, y1, 0, 0}, cId}
                   << Vertex{{x0, y1, 0, 0}, cId};
        }
    }
}

static void write_index_buffer(VertexWriter writer) {
    if (!writer) return;

    // Corner 0: TL Corner, fan from v2.
    writer << uint16_t(2) << uint16_t(0) << uint16_t(1)
           << uint16_t(2) << uint16_t(4) << uint16_t(0)
           << uint16_t(2) << uint16_t(3) << uint16_t(4);

    // Corner 1: TR Corner, fan from v9.
    writer << uint16_t(9) << uint16_t(5) << uint16_t(6)
           << uint16_t(9) << uint16_t(6) << uint16_t(7)
           << uint16_t(9) << uint16_t(7) << uint16_t(8);

    // Corner 2: BR Corner, fan from v10.
    writer << uint16_t(10) << uint16_t(11) << uint16_t(12)
           << uint16_t(10) << uint16_t(12) << uint16_t(13)
           << uint16_t(10) << uint16_t(13) << uint16_t(14);

    // Corner 3: BL Corner, fan from v16.
    writer << uint16_t(16) << uint16_t(17) << uint16_t(18)
           << uint16_t(16) << uint16_t(18) << uint16_t(19)
           << uint16_t(16) << uint16_t(19) << uint16_t(15);

    // Create remaining quads.
    uint16_t base = 20;
    for (int i = 0; i < 21; i++) {
        writer << uint16_t(base + 0) << uint16_t(base + 1) << uint16_t(base + 3)
               << uint16_t(base + 1) << uint16_t(base + 2) << uint16_t(base + 3);
        base += 4;
    }
}

AnalyticRRectBlurRenderStep::AnalyticRRectBlurRenderStep(Layout layout,
                                                         StaticBufferManager* bufferManager)
        : RenderStep(layout,
                     RenderStepID::kAnalyticRRectBlur,
                     Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage |
                     Flags::kAppendInstances,
                     /*uniforms=*/
                      {{"localToDevice", SkSLType::kFloat4x4},
                       {"rect", SkSLType::kFloat4},
                       {"xBounds", SkSLType::kFloat, 6},
                       {"yBounds", SkSLType::kFloat, 6},
                       {"safeLimits", SkSLType::kFloat4},
                       {"drawPad", SkSLType::kFloat2},
                       {"sqrtHalfOverSigma", SkSLType::kHalf2},
                       {"radii", SkSLType::kFloat2, 4},
                       {"radius", SkSLType::kFloat2},
                       {"depth", SkSLType::kFloat}},
                     PrimitiveType::kTriangles,
                     kDirectDepthLessPass,
                     /*staticAttrs=*/
                     {{"gridAndBevel", VertexAttribType::kInt4, SkSLType::kInt4},
                      {"cellIndex", VertexAttribType::kInt, SkSLType::kInt}},
                     /*appendAttrs=*/
                     {{"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt}},
                     /*storageUniforms=*/{},
                     /*varyings=*/
                     {{"scaledShapeCoords", SkSLType::kFloat2},
                      {"vFlags", SkSLType::kHalf4}}) {
    write_vertex_buffer(bufferManager->getVertexWriter(kVertexCount, sizeof(Vertex),
                                                       &fVertexBuffer));
    write_index_buffer(bufferManager->getIndexWriter(sizeof(uint16_t) * kIndexCount,
                                                     &fIndexBuffer));
}

std::string AnalyticRRectBlurRenderStep::vertexSkSL(const RootNodesInfo&) const {
    return "float4 devPosition = analytic_rrect_blur_vertex_fn("
                   "gridAndBevel, cellIndex, "
                   "localToDevice, rect, xBounds, yBounds, safeLimits, "
                   "drawPad, radii, depth, "
                   "scaledShapeCoords, vFlags, stepLocalCoords);\n";
}

std::string AnalyticRRectBlurRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    return EmitSamplerLayout(bindingReqs, nextBindingIndex) + " sampler2D cdfLut;";
}

const char* AnalyticRRectBlurRenderStep::fragmentCoverageSkSL() const {
    return "outputCoverage = analytic_rrect_blur_coverage_fn(scaledShapeCoords, "
                                                            "vFlags, "
                                                            "rect, "
                                                            "sqrtHalfOverSigma, "
                                                            "radii, "
                                                            "radius, "
                                                            "cdfLut);";
}

void AnalyticRRectBlurRenderStep::writeVertices(DrawWriter* writer,
                                                StorageContext* /*storageContext*/,
                                                const DrawParams& params,
                                                uint32_t ssboIndex) const {
    DrawWriter::Instances instances{*writer, fVertexBuffer, fIndexBuffer, kIndexCount};
    instances.append(1) << ssboIndex;
}

void AnalyticRRectBlurRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                          PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    const AnalyticRRectBlurMask& blur = params.geometry().analyticRRectBlurMask();
    const SkRect& rect = blur.rrect().getBounds();
    float drawPadX = blur.drawPadX();
    float drawPadY = blur.drawPadY();

    float satPadX = std::ceil(3.5f * blur.localSigma().x);
    float satPadY = std::ceil(3.5f * blur.localSigma().y);

    // Calculate distance from edge where the corner curvature has ended and is saturated.
    float safeOffsetXMin = std::max(blur.radii()[0].x, blur.radii()[3].x) + drawPadX;
    float safeOffsetXMax = std::max(blur.radii()[1].x, blur.radii()[2].x) + drawPadX;
    float safeOffsetYMin = std::max(blur.radii()[0].y, blur.radii()[1].y) + drawPadY;
    float safeOffsetYMax = std::max(blur.radii()[2].y, blur.radii()[3].y) + drawPadY;

    float xlo = rect.fLeft + safeOffsetXMin;
    float xhi = rect.fRight - safeOffsetXMax;
    float ylo = rect.fTop + safeOffsetYMin;
    float yhi = rect.fBottom - safeOffsetYMax;
    SkRect cornerSafeLimits{xlo, ylo, xhi, yhi};

    // Inner-most safe bounds that we define as our fully saturated bounds.
    float insXMin = rect.fLeft + std::max(satPadX, safeOffsetXMin);
    float insXMax = rect.fRight - std::max(satPadX, safeOffsetXMax);
    float insYMin = rect.fTop + std::max(satPadY, safeOffsetYMin);
    float insYMax = rect.fBottom - std::max(satPadY, safeOffsetYMax);

    // Snap inner-most inset bounds to the center if they are overlapping.
    if (insXMin >= insXMax) {
        insXMin = insXMax = (rect.fLeft + rect.fRight) * 0.5f;
    }
    if (insYMin >= insYMax) {
        insYMin = insYMax = (rect.fTop + rect.fBottom) * 0.5f;
    }

    // Our outer-most edge safe inset bounds. This allows us to assume full coverage when we are
    // far enough from an edge on one axis and within the corner radius safe limits on the other
    // axis.
    float edgeInsXMin = std::min(insXMin, rect.fLeft + std::min(satPadX, safeOffsetXMin));
    float edgeInsXMax = std::max(insXMax, rect.fRight - std::min(satPadX, safeOffsetXMax));
    float edgeInsYMin = std::min(insYMin, rect.fTop + std::min(satPadY, safeOffsetYMin));
    float edgeInsYMax = std::max(insYMax, rect.fBottom - std::min(satPadY, safeOffsetYMax));

    // Bounds mapping to each row and columns of our 5x5 vertex grid.
    const float xBounds[6] = {rect.fLeft - drawPadX, edgeInsXMin, insXMin,
                              insXMax, edgeInsXMax, rect.fRight + drawPadX};
    const float yBounds[6] = {rect.fTop - drawPadY,  edgeInsYMin, insYMin,
                              insYMax, edgeInsYMax, rect.fBottom + drawPadY};

    gatherer->write(params.transform().matrix());
    gatherer->write(rect);
    gatherer->writeArray(SkSpan<const float>(xBounds));
    gatherer->writeArray(SkSpan<const float>(yBounds));
    gatherer->write(cornerSafeLimits);
    gatherer->write(SkV2{drawPadX, drawPadY});
    gatherer->writeHalf(blur.sqrtHalfOverSigma());
    gatherer->writeArray(blur.radii());
    gatherer->write(SkSize::Make(blur.radius()));
    gatherer->write(params.order().depthAsFloat());

    gatherer->add(blur.refCdfProxy(), {SkFilterMode::kLinear, SkTileMode::kClamp});
}

}  // namespace skgpu::graphite
