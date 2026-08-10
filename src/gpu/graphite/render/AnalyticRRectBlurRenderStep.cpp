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
                      {"blurData", SkSLType::kHalf2},
                      {"radii", SkSLType::kFloat2, 4},
                      {"radius", SkSLType::kInt2},
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
                                                            "blurData, "
                                                            "radii, "
                                                            "radius);";
}

void AnalyticRRectBlurRenderStep::writeVertices(DrawWriter* writer,
                                                const DrawParams& params,
                                                uint32_t ssboIndex) const {
    constexpr uint8_t kInside = 128;
    constexpr uint8_t kOn     = 255;

    const AnalyticRRectBlurMask& blur = params.geometry().analyticRRectBlurMask();

    // 1. Recover local-space sigmas from blurData
    float sigmaX = (1.f/SK_FloatSqrt2) / blur.blurData().x;
    float sigmaY = (1.f/SK_FloatSqrt2) / blur.blurData().y;

    float drawPadX = std::ceil(3.f * sigmaX);
    float drawPadY = std::ceil(3.f * sigmaY);

    // 2. Recover local-space rect of the rounded rect
    const Rect& drawBounds = blur.drawBounds();
    SkRect rect = SkRect::MakeLTRB(drawBounds.left() + drawPadX,
                                   drawBounds.top()  + drawPadY,
                                   drawBounds.right() - drawPadX,
                                   drawBounds.bot()   - drawPadY);

    float xMin = drawBounds.left();
    float xMax = drawBounds.right();
    float yMin = drawBounds.top();
    float yMax = drawBounds.bot();

    // 3. Retrieve radii
    SkVector radii[4];
    for (int i = 0; i < 4; ++i) {
        radii[i] = {blur.radii()[i].x, blur.radii()[i].y};
    }

    // 4. Safe offsets & safe bounds
    float safeOffsetX = std::ceil(3.f * sigmaX);
    float safeOffsetY = std::ceil(3.f * sigmaY);

    float ySafe[] = {
        rect.fTop    + radii[0].fY + safeOffsetY,  // TL
        rect.fTop    + radii[1].fY + safeOffsetY,  // TR
        rect.fBottom - radii[2].fY - safeOffsetY,  // BR
        rect.fBottom - radii[3].fY - safeOffsetY,  // BL
    };
    float xSafe[] = {
        rect.fLeft  + radii[0].fX + safeOffsetX,   // TL
        rect.fRight - radii[1].fX - safeOffsetX,   // TR
        rect.fRight - radii[2].fX - safeOffsetX,   // BR
        rect.fLeft  + radii[3].fX + safeOffsetX,   // BL
    };

    // Stack-allocated sorted, unique boundary arrays
    float xBounds[8];
    int xBoundsCount = 0;
    float yBounds[8];
    int yBoundsCount = 0;

    auto addBound = [](float* array, int& count, float val, float lo, float hi) {
        float clamped = std::clamp(val, lo, hi);
        int insertPos = 0;
        while (insertPos < count && array[insertPos] < clamped) {
            insertPos++;
        }
        if (insertPos < count && array[insertPos] == clamped) {
            return;
        }
        for (int i = count; i > insertPos; i--) {
            array[i] = array[i - 1];
        }
        array[insertPos] = clamped;
        count++;
    };

    float insInsetX = std::ceil(3.5f * sigmaX);
    float insInsetY = std::ceil(3.5f * sigmaY);
    SkRRect insideRRect = SkRRect::MakeRectRadii(rect, radii);
    insideRRect.inset(insInsetX, insInsetY);
    SkRect insBounds = insideRRect.getBounds();

    addBound(xBounds, xBoundsCount, xMin, xMin, xMax);
    addBound(xBounds, xBoundsCount, xSafe[0], xMin, xMax);
    addBound(xBounds, xBoundsCount, xSafe[1], xMin, xMax);
    addBound(xBounds, xBoundsCount, xSafe[2], xMin, xMax);
    addBound(xBounds, xBoundsCount, xSafe[3], xMin, xMax);
    addBound(xBounds, xBoundsCount, insBounds.fLeft, xMin, xMax);
    addBound(xBounds, xBoundsCount, insBounds.fRight, xMin, xMax);
    addBound(xBounds, xBoundsCount, xMax, xMin, xMax);

    addBound(yBounds, yBoundsCount, yMin, yMin, yMax);
    addBound(yBounds, yBoundsCount, ySafe[0], yMin, yMax);
    addBound(yBounds, yBoundsCount, ySafe[1], yMin, yMax);
    addBound(yBounds, yBoundsCount, ySafe[2], yMin, yMax);
    addBound(yBounds, yBoundsCount, ySafe[3], yMin, yMax);
    addBound(yBounds, yBoundsCount, insBounds.fTop, yMin, yMax);
    addBound(yBounds, yBoundsCount, insBounds.fBottom, yMin, yMax);
    addBound(yBounds, yBoundsCount, yMax, yMin, yMax);

    constexpr float kBevelFrac = 2.f - float(SK_FloatSqrt2);
    SkVector bevel[4];
    for (int i = 0; i < 4; i++) {
        bevel[i] = (radii[i] + SkVector{drawPadX, drawPadY}) * kBevelFrac;
    }

    skia_private::STArray<128, MeshVertex> vertices;
    skia_private::STArray<384, uint16_t> indices;

    auto addQuadTo = [&](skia_private::STArray<384, uint16_t>& idxs,
                         float x0, float y0, float x1, float y1,
                         uint8_t tl, uint8_t tr, uint8_t br, uint8_t bl) {
        uint16_t base = static_cast<uint16_t>(vertices.size());
        uint8_t f[4] = {tl, tr, br, bl};
        vertices.push_back({{x0, y0}, {f[0], f[1], f[2], f[3]}});
        vertices.push_back({{x1, y0}, {f[0], f[1], f[2], f[3]}});
        vertices.push_back({{x1, y1}, {f[0], f[1], f[2], f[3]}});
        vertices.push_back({{x0, y1}, {f[0], f[1], f[2], f[3]}});
        idxs.push_back(base);
        idxs.push_back(base + 1);
        idxs.push_back(base + 3);
        idxs.push_back(base + 1);
        idxs.push_back(base + 2);
        idxs.push_back(base + 3);
    };
    auto addQuad = [&](float x0, float y0, float x1, float y1,
                       uint8_t tl, uint8_t tr, uint8_t br, uint8_t bl) {
        addQuadTo(indices, x0, y0, x1, y1, tl, tr, br, bl);
    };

    auto addBeveledQuad = [&](float xlo, float ylo, float xhi, float yhi,
                              float cx, float cy, int corner,
                              uint8_t tl, uint8_t tr, uint8_t br, uint8_t bl) {
        uint16_t base = static_cast<uint16_t>(vertices.size());
        uint8_t f[4] = {tl, tr, br, bl};
        auto v = [&](float x, float y) {
            vertices.push_back({{x, y}, {f[0], f[1], f[2], f[3]}});
        };
        auto tri = [&](int a, int b, int c) {
            indices.push_back(base + a);
            indices.push_back(base + b);
            indices.push_back(base + c);
        };
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

    int xiLast = xBoundsCount - 2;
    int yiLast = yBoundsCount - 2;

    for (int yi = 0; yi + 1 < yBoundsCount; yi++) {
        float ylo = yBounds[yi], yhi = yBounds[yi + 1];
        if (yhi <= ylo) {
            continue;
        }

        for (int xi = 0; xi + 1 < xBoundsCount; xi++) {
            float xlo = xBounds[xi], xhi = xBounds[xi + 1];
            if (xhi <= xlo) {
                continue;
            }

            uint8_t doTL = (yhi <= ySafe[0] && xhi <= xSafe[0]) ? kOn : 0;
            uint8_t doTR = (yhi <= ySafe[1] && xlo >= xSafe[1]) ? kOn : 0;
            uint8_t doBR = (ylo >= ySafe[2] && xlo >= xSafe[2]) ? kOn : 0;
            uint8_t doBL = (ylo >= ySafe[3] && xhi <= xSafe[3]) ? kOn : 0;

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
                addQuadTo(indices, xlo, ylo, xhi, yhi,
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
    gatherer->write(blur.shapeData().asSkRect());
    gatherer->writeHalf(blur.blurData());
    gatherer->writeArray(blur.radii());
    gatherer->write(blur.radius());
    gatherer->write(params.order().depthAsFloat());
}

}  // namespace skgpu::graphite
