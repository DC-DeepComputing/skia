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
#include "include/private/SkTArray.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/geom/AnalyticRRectBlurMask.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

namespace skgpu::graphite {

struct MeshVertex {
    SkPoint pos;
    uint8_t flags[4];
};

AnalyticRRectBlurRenderStep::AnalyticRRectBlurRenderStep(Layout layout)
        : RenderStep(layout,
                     RenderStepID::kAnalyticRRectBlur,
                     Flags::kPerformsShading | Flags::kEmitsCoverage | Flags::kAppendVertices,
                     /*uniforms=*/
                     {{"localToDevice", SkSLType::kFloat4x4},
                      {"deviceToScaledShape", SkSLType::kFloat3x3},
                      {"shapeData", SkSLType::kFloat4},
                      {"sqrtHalfOverSigma", SkSLType::kHalf},
                      {"radii", SkSLType::kFloat2, 4},
                      {"radius", SkSLType::kInt},
                      {"depth", SkSLType::kFloat}},
                     PrimitiveType::kTriangles,
                     kDirectDepthLessPass,
                     /*staticAttrs=*/ {},
                     /*appendAttrs=*/
                     {{"position", VertexAttribType::kFloat2, SkSLType::kFloat2},
                      {"flags", VertexAttribType::kUByte4_norm, SkSLType::kHalf4},
                      {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt}},
                     /*varyings=*/
                     {{"scaledShapeCoords", SkSLType::kFloat2},
                      {"vFlags", SkSLType::kHalf4}}) {}

std::string AnalyticRRectBlurRenderStep::vertexSkSL(const RootNodesInfo&) const {
    return
        "float4 devPosition = localToDevice * float4(position, depth, 1.0);\n"
        "stepLocalCoords = position;\n"
        "vFlags = flags;\n"
        "scaledShapeCoords = (deviceToScaledShape * devPosition.xy1).xy;\n";
}

const char* AnalyticRRectBlurRenderStep::fragmentCoverageSkSL() const {
    return "outputCoverage = analytic_rrect_blur_coverage_fn(scaledShapeCoords, "
                                                            "vFlags, "
                                                            "shapeData, "
                                                            "sqrtHalfOverSigma, "
                                                            "radii, "
                                                            "radius);";
}

void AnalyticRRectBlurRenderStep::writeVertices(DrawWriter* writer,
                                                const DrawParams& params,
                                                uint32_t ssboIndex) const {
    constexpr uint8_t kInside = 128;
    constexpr uint8_t kOn     = 255;

    const AnalyticRRectBlurMask& blur = params.geometry().analyticRRectBlurMask();

    float sigmaX = blur.localSigma();
    float sigmaY = blur.localSigma();

    SkRect rect = blur.rrect().getBounds();
    float drawPadX = std::ceil(3.f * sigmaX);
    float drawPadY = std::ceil(3.f * sigmaY);
    float xMin = rect.fLeft   - drawPadX;
    float xMax = rect.fRight  + drawPadX;
    float yMin = rect.fTop    - drawPadY;
    float yMax = rect.fBottom + drawPadY;

    SkSpan<const SkV2> radii = blur.radii();
    // Safe offset: the distance from a straight edge at which the 1D CDF saturates.
    // Used for per-corner safe bounds.
    float safeOffsetX = std::ceil(3.f * sigmaX);
    float safeOffsetY = std::ceil(3.f * sigmaY);

    // Per-corner safe boundaries. TL=0, TR=1, BR=2, BL=3.
    // Top corners active when y < ySafe, bottom corners when y > ySafe.
    // Left corners active when x < xSafe, right corners when x > xSafe.
    float ySafe[] = {
        rect.fTop    + radii[0].y + safeOffsetY,  // TL
        rect.fTop    + radii[1].y + safeOffsetY,  // TR
        rect.fBottom - radii[2].y - safeOffsetY,  // BR
        rect.fBottom - radii[3].y - safeOffsetY,  // BL
    };
    float xSafe[] = {
        rect.fLeft  + radii[0].x + safeOffsetX,   // TL
        rect.fRight - radii[1].x - safeOffsetX,   // TR
        rect.fRight - radii[2].x - safeOffsetX,   // BR
        rect.fLeft  + radii[3].x + safeOffsetX,   // BL
    };

    // Build sorted, unique boundary arrays from per-corner safe values.
    auto makeBounds = [](std::initializer_list<float> vals, float lo, float hi) {
        std::vector<float> v;
        for (float f : vals) {
            v.push_back(std::clamp(f, lo, hi));
        }
        std::sort(v.begin(), v.end());
        v.erase(std::unique(v.begin(), v.end()), v.end());
        return v;
    };

    // Inset rrect for fully-inside detection. Uses 3.5*sigma so that
    // cells near curved corners are correctly classified.
    float insInsetX = std::ceil(3.5f * sigmaX);
    float insInsetY = std::ceil(3.5f * sigmaY);
    SkRRect insideRRect = blur.rrect();
    insideRRect.inset(insInsetX, insInsetY);

    // Include inside-saturation bounds in the grid so the inside
    // optimization doesn't require per-cell sub-splitting (which
    // creates T-junctions on shared edges with non-sub-split neighbors).
    SkRect insBounds = insideRRect.getBounds();
    auto yBounds = makeBounds(
            {yMin, ySafe[0], ySafe[1], ySafe[2], ySafe[3],
             insBounds.fTop, insBounds.fBottom, yMax}, yMin, yMax);
    auto xBounds = makeBounds(
            {xMin, xSafe[0], xSafe[1], xSafe[2], xSafe[3],
             insBounds.fLeft, insBounds.fRight, xMax}, xMin, xMax);

    constexpr float kBevelFrac = 2.f - float(SK_FloatSqrt2);
    SkVector bevel[4];
    for (int i = 0; i < 4; i++) {
        bevel[i] = (SkVector{radii[i].x + drawPadX, radii[i].y + drawPadY}) * kBevelFrac;
    }

    std::vector<MeshVertex> vertices;
    std::vector<uint16_t> indices;

    auto addQuadTo = [&](std::vector<uint16_t>& idxs,
                         float x0, float y0, float x1, float y1,
                         uint8_t tl, uint8_t tr, uint8_t br, uint8_t bl) {
        uint16_t base = static_cast<uint16_t>(vertices.size());
        uint8_t f[4] = {tl, tr, br, bl};
        vertices.push_back({{x0, y0}, {f[0], f[1], f[2], f[3]}});
        vertices.push_back({{x1, y0}, {f[0], f[1], f[2], f[3]}});
        vertices.push_back({{x1, y1}, {f[0], f[1], f[2], f[3]}});
        vertices.push_back({{x0, y1}, {f[0], f[1], f[2], f[3]}});
        idxs.insert(idxs.end(), {uint16_t(base),     uint16_t(base + 1), uint16_t(base + 3),
                                 uint16_t(base + 1), uint16_t(base + 2), uint16_t(base + 3)});
    };
    auto addQuad = [&](float x0, float y0, float x1, float y1,
                       uint8_t tl, uint8_t tr, uint8_t br, uint8_t bl) {
        addQuadTo(indices, x0, y0, x1, y1, tl, tr, br, bl);
    };

    // Pentagon: quad with one corner beveled. 5 vertices, 3 triangles.
    // corner: 0=TL, 1=TR, 2=BR, 3=BL identifies which corner to clip.
    auto addBeveledQuad = [&](float xlo, float ylo, float xhi, float yhi,
                              float cx, float cy, int corner,
                              uint8_t tl, uint8_t tr, uint8_t br, uint8_t bl) {
        uint16_t base = static_cast<uint16_t>(vertices.size());
        uint8_t f[4] = {tl, tr, br, bl};
        auto v = [&](float x, float y) {
            vertices.push_back({{x, y}, {f[0], f[1], f[2], f[3]}});
        };
        auto tri = [&](int a, int b, int c) {
            indices.insert(indices.end(),
                           {uint16_t(base+a), uint16_t(base+b), uint16_t(base+c)});
        };
        // Vertices listed CW (y-down), fan from the vertex opposite the bevel.
        switch (corner) {
            case 0:  // TL
                v(xlo+cx, ylo); v(xhi, ylo); v(xhi, yhi); v(xlo, yhi); v(xlo, ylo+cy);
                tri(2,0,1); tri(2,4,0); tri(2,3,4);
                break;
            case 1:  // TR
                v(xlo, ylo); v(xhi-cx, ylo); v(xhi, ylo+cy); v(xhi, yhi); v(xlo, yhi);
                tri(4,0,1); tri(4,1,2); tri(4,2,3);
                break;
            case 2:  // BR
                v(xlo, ylo); v(xhi, ylo); v(xhi, yhi-cy); v(xhi-cx, yhi); v(xlo, yhi);
                tri(0,1,2); tri(0,2,3); tri(0,3,4);
                break;
            case 3:  // BL
                v(xlo, ylo); v(xhi, ylo); v(xhi, yhi); v(xlo+cx, yhi); v(xlo, yhi-cy);
                tri(1,2,3); tri(1,3,4); tri(1,4,0);
                break;
        }
    };

    size_t xiLast = xBounds.size() - 2;
    size_t yiLast = yBounds.size() - 2;

    for (size_t yi = 0; yi + 1 < yBounds.size(); yi++) {
        float ylo = yBounds[yi], yhi = yBounds[yi + 1];
        if (yhi <= ylo) {
            continue;
        }

        for (size_t xi = 0; xi + 1 < xBounds.size(); xi++) {
            float xlo = xBounds[xi], xhi = xBounds[xi + 1];
            if (xhi <= xlo) {
                continue;
            }

            // Per-corner active check. Since safe values are grid split points,
            // each cell is entirely inside or outside each corner's region.
            uint8_t doTL = (yhi <= ySafe[0] && xhi <= xSafe[0]) ? kOn : 0;
            uint8_t doTR = (yhi <= ySafe[1] && xlo >= xSafe[1]) ? kOn : 0;
            uint8_t doBR = (ylo >= ySafe[2] && xlo >= xSafe[2]) ? kOn : 0;
            uint8_t doBL = (ylo >= ySafe[3] && xhi <= xSafe[3]) ? kOn : 0;

            // Octagonal bevel on the 4 outer corner cells.
            int bevelCorner = -1;
            float cx = 0, cy = 0;
            if (yi == 0       && xi == 0)      { bevelCorner = 0; }
            if (yi == 0       && xi == xiLast) { bevelCorner = 1; }
            if (yi == yiLast && xi == xiLast)  { bevelCorner = 2; }
            if (yi == yiLast && xi == 0)       { bevelCorner = 3; }
            if (bevelCorner >= 0) {
                cx = std::min(bevel[bevelCorner].fX, xhi - xlo);
                cy = std::min(bevel[bevelCorner].fY, yhi - ylo);
            }

            if (bevelCorner >= 0 && cx > 0 && cy > 0) {
                addBeveledQuad(xlo, ylo, xhi, yhi, cx, cy, bevelCorner,
                                 doTL, doTR, doBR, doBL);
            } else if (insideRRect.contains({xlo, ylo, xhi, yhi})) {
                // Fully inside the inset rrect: CDF is saturated.
                auto& dst = indices;
                addQuadTo(dst, xlo, ylo, xhi, yhi,
                          kInside, kInside, kInside, kInside);
            } else {
                addQuad(xlo, ylo, xhi, yhi, doTL, doTR, doBR, doBL);
            }
        }
    }

    DrawWriter::Vertices verts{*writer};
    verts.reserve(indices.size());
    for (uint16_t idx : indices) {
        const auto& v = vertices[idx];
        verts.append(1) << skvx::float2(v.pos.fX, v.pos.fY)
                        << skvx::byte4{v.flags[0], v.flags[1], v.flags[2], v.flags[3]}
                        << ssboIndex;
    }
}

void AnalyticRRectBlurRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                          PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    const AnalyticRRectBlurMask& blur = params.geometry().analyticRRectBlurMask();

    gatherer->write(params.transform().matrix());
    gatherer->write(blur.devToScaledShape().asM33());
    gatherer->write(blur.rrect().getBounds());
    gatherer->writeHalf(blur.sqrtHalfOverSigma());
    gatherer->writeArray(blur.radii());
    gatherer->write(blur.radius());
    gatherer->write(params.order().depthAsFloat());
}

}  // namespace skgpu::graphite
