/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/VerticesRenderStep.h"

#include "src/core/SkVertState.h"
#include "src/core/SkVerticesPriv.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/render/StencilAndCoverDSS.h"

namespace skgpu::graphite {

namespace {
static constexpr DepthStencilSettings kDirectShadingPass = {
        /*frontStencil=*/{},
        /*backStencil=*/ {},
        /*refValue=*/    0,
        /*stencilTest=*/ false,
        /*depthCompare=*/CompareOp::kGreater,
        /*depthTest=*/   true,
        /*depthWrite=*/  true
};

static constexpr Attribute positionAttribute = {"position", VertexAttribType::kFloat2,
                                                SkSLType::kFloat2};
// TODO: Use smaller data type for tex coordinates; VertexAttribType::kUShort2, SkSLType::kUShort2?
static constexpr Attribute textureAttribute = {"texCoords", VertexAttribType::kFloat2,
                                               SkSLType::kFloat2};
static constexpr Attribute colorAttribute = {"color", VertexAttribType::kFloat4, SkSLType::kFloat4};

static constexpr std::initializer_list<Attribute> positionOnly = {positionAttribute};
static constexpr std::initializer_list<Attribute> withColor = {positionAttribute, colorAttribute};
static constexpr std::initializer_list<Attribute> withTexture = {positionAttribute,
                                                                 textureAttribute};
static constexpr std::initializer_list<Attribute> withTextureAndColor = {positionAttribute,
                                                                         colorAttribute,
                                                                         textureAttribute};

static constexpr std::initializer_list<Attribute> attributeArray [4] = { positionOnly, withColor,
                                                                         withTexture,
                                                                         withTextureAndColor };
}  // namespace

VerticesRenderStep::VerticesRenderStep(PrimitiveType type, std::string_view variantName,
                                       bool hasColor, bool hasTexture)
        : RenderStep("VerticesRenderStep",
                     variantName,
                     Flags::kPerformsShading, //Add Flags::kRequiresMSAA?
                     /*uniforms=*/{},
                     type,
                     kDirectShadingPass,
                     hasTexture ? attributeArray[2] : attributeArray[hasColor + hasTexture],
                     /*instanceAttrs=*/{}),
        fHasColor (hasColor), fHasTexture (hasTexture) {}

VerticesRenderStep::~VerticesRenderStep() {}

const char* VerticesRenderStep::vertexSkSL() const {
    // TODO: Add SkSl for color and/or texture information
    if (fHasColor && !fHasTexture) {
        return R"(
            float4 color = color;
            float2 textureCoords = texCoords;
            float4 devPosition = float4(position, 1.0, 1.0);\n)";
    } else if (fHasTexture && !fHasColor) {
        return R"(
            float2 textureCoords = texCoords;
            float4 devPosition = float4(position, 1.0, 1.0);)";
    } else if (fHasColor && fHasTexture) {
        return R"(
            float4 color = color;
            float4 devPosition = float4(position, 1.0, 1.0);)";
    } else {
        return "float4 devPosition = float4(position, 1.0, 1.0);\n";
    }
}

void VerticesRenderStep::writeVertices(DrawWriter* writer, const DrawParams& params) const {
    SkVerticesPriv info(params.geometry().vertices()->priv());
    const int vertexCount = info.vertexCount();
    const int indexCount = info.indexCount();
    const SkPoint* positions = info.positions();
    const uint16_t* indices = info.indices();

    DrawWriter::Vertices verts{*writer};

    VertState state(vertexCount, indices, indexCount);
    VertState::Proc vertProc = state.chooseProc(info.mode());
    while (vertProc(&state)) {

        SkV2 p[3] = {{positions[state.f0].x(), positions[state.f0].y()},
                     {positions[state.f1].x(), positions[state.f1].y()},
                     {positions[state.f2].x(), positions[state.f2].y()}};

        SkV4 devPoints[3];
        params.transform().mapPoints(p, devPoints, 3);
        // TODO: Transform texture coordinates?
        // TODO: Handle color, make helpers

        if (fHasColor && fHasTexture) {
            const SkColor* colors = info.colors();
            SkColor4f color0 = SkColor4f::FromColor(colors[state.f0]);
            SkColor4f color1 = SkColor4f::FromColor(colors[state.f1]);
            SkColor4f color2 = SkColor4f::FromColor(colors[state.f2]);

            SkV4 c[3] = {{color0.fR, color0.fG, color0.fB, color0.fA},
                         {color1.fR, color1.fG, color1.fB, color1.fA},
                         {color2.fR, color2.fG, color2.fB, color2.fA}};

            const SkPoint* texCoords = info.texCoords();
            SkV2 t[3] = {{texCoords[state.f0].x(), texCoords[state.f0].y()},
                         {texCoords[state.f1].x(), texCoords[state.f1].y()},
                         {texCoords[state.f2].x(), texCoords[state.f2].y()}};
            verts.append(3) << devPoints[0].x << devPoints[0].y << c[0] << SkPoint{t[0].x, t[0].y}
                            << devPoints[1].x << devPoints[1].y << c[1] << SkPoint{t[1].x, t[1].y}
                            << devPoints[2].x << devPoints[2].y << c[2] << SkPoint{t[2].x, t[2].y};
        } else if (fHasColor) {
            const SkColor* colors = info.colors();
            SkColor4f color0 = SkColor4f::FromColor(colors[state.f0]);
            SkColor4f color1 = SkColor4f::FromColor(colors[state.f1]);
            SkColor4f color2 = SkColor4f::FromColor(colors[state.f2]);

            SkV4 c[3] = {{color0.fR, color0.fG, color0.fB, color0.fA},
                         {color1.fR, color1.fG, color1.fB, color1.fA},
                         {color2.fR, color2.fG, color2.fB, color2.fA}};
            verts.append(3) << SkPoint{devPoints[0].x, devPoints[0].y} << c[0]
                            << SkPoint{devPoints[1].x, devPoints[1].y} << c[1]
                            << SkPoint{devPoints[2].x, devPoints[2].y} << c[2];
        } else if (fHasTexture) {
            const SkPoint* texCoords = info.texCoords();
            SkV2 t[3] = {{texCoords[state.f0].x(), texCoords[state.f0].y()},
                         {texCoords[state.f1].x(), texCoords[state.f1].y()},
                         {texCoords[state.f2].x(), texCoords[state.f2].y()}};
            verts.append(3) << SkPoint{devPoints[0].x, devPoints[0].y} << SkPoint{t[0].x, t[0].y}
                            << SkPoint{devPoints[1].x, devPoints[1].y} << SkPoint{t[1].x, t[1].y}
                            << SkPoint{devPoints[2].x, devPoints[2].y} << SkPoint{t[2].x, t[2].y};
        } else {
            verts.append(3) << devPoints[0].x << devPoints[0].y
                            << devPoints[1].x << devPoints[1].y
                            << devPoints[2].x << devPoints[2].y;
        }
    }
}

void VerticesRenderStep::writeUniforms(const DrawParams&, SkPipelineDataGatherer*) const {
    // Vertices are currently pre-transformed to device space on the CPU, so no uniforms needed.
}

}  // namespace skgpu::graphite
