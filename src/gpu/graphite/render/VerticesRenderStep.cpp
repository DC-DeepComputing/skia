/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/VerticesRenderStep.h"

#include "src/core/SkPipelineData.h"
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

// Define different combinations of vertex attributes based upon whether the vertices have color
// and/or texture coordinates.
static constexpr Attribute positionAttribute = {"position", VertexAttribType::kFloat2,
                                                SkSLType::kFloat2};
// TODO: Use smaller data type for tex coordinates; VertexAttribType::kUShort2, SkSLType::kUShort2?
static constexpr Attribute textureAttribute = {"texCoords", VertexAttribType::kFloat2,
                                               SkSLType::kFloat2};
static constexpr Attribute colorAttribute = {"vertColor", VertexAttribType::kFloat4,
                                             SkSLType::kFloat4};

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

// Define different combinations of varyings based upon whether the vertices have color and/or
// texture coordinates.
static constexpr std::initializer_list<Varying> noVarying = {};
// DrawVertices only needs the built-in localCoords varying.
static constexpr std::initializer_list<Varying> textureVarying = noVarying;
static constexpr std::initializer_list<Varying> colorVarying = {{"color", SkSLType::kFloat4}};
static constexpr std::initializer_list<Varying> textureAndColorVarying = colorVarying;
static const std::initializer_list<Varying> varyingArray [4] = { noVarying, colorVarying,
                                                                 textureVarying,
                                                                 textureAndColorVarying};

// Define different combinations of flags based upon whether the vertices have color and/or
// texture coordinates. TODO: Add Flags::kRequiresMSAA?
using Flags = VerticesRenderStep::Flags;
static const SkEnumBitMask<Flags> shadingOnlyFlag {Flags::kPerformsShading};
static const SkEnumBitMask<Flags> textureFlags {Flags::kPerformsShading | Flags::kHasTextures};
static const SkEnumBitMask<Flags> colorFlags {Flags::kPerformsShading |
                                              Flags::kEmitsPrimitiveColor};
static const SkEnumBitMask<Flags> colorAndTextureFlags {Flags::kPerformsShading |
                                                        Flags::kEmitsPrimitiveColor |
                                                        Flags::kHasTextures};
static const SkEnumBitMask<Flags> flagArray [4] = {shadingOnlyFlag, colorFlags, textureFlags,
                                                   colorAndTextureFlags};

}  // namespace

VerticesRenderStep::VerticesRenderStep(PrimitiveType type, std::string_view variantName,
                                       bool hasColor, bool hasTexture)
        : RenderStep("VerticesRenderStep",
                     variantName,
                     (hasTexture && !hasColor) ? flagArray[2]
                                               : flagArray[hasColor + hasTexture],
                     /*uniforms=*/{{"depth", SkSLType::kFloat},
                                   {"skipColorXForm", SkSLType::kInt}},
                     type,
                     kDirectShadingPass,
                     (hasTexture && !hasColor) ? attributeArray[2]
                                               : attributeArray[hasColor + hasTexture],
                     /*instanceAttrs=*/{},
                     /*varyings=*/ (hasTexture && !hasColor) ? varyingArray[2]
                                                             : varyingArray[hasColor + hasTexture])
        , fHasColor (hasColor), fHasTexture (hasTexture) {}

VerticesRenderStep::~VerticesRenderStep() {}

const char* VerticesRenderStep::vertexSkSL() const {
    // TODO - Have ALL render step implementations return local coordinates.
    // TODO - remove added texture varying from this class once the varying is added for all steps.
    if (fHasColor && fHasTexture) {
        return R"(
            color = vertColor;
            float4 devPosition = float4(position, 1.0, 1.0);\n);
            localCoords = texCoords;
        )";
    } else if (fHasTexture && !fHasColor) {
        return R"(
            float4 devPosition = float4(position, 1.0, 1.0);
            localCoords = texCoords;
        )";
    } else if (fHasColor && !fHasTexture) {
        return R"(
            color = vertColor;
            float4 devPosition = float4(position, 1.0, 1.0);
        )";
    } else {
        return "float4 devPosition = float4(position, 1.0, 1.0);\n";
    }
}

void VerticesRenderStep::writeVertices(DrawWriter* writer, const DrawParams& params) const {
    // TODO: Instead of fHasColor and fHasTexture, use info() and assert
    if (fHasColor && fHasTexture) {
        writeVerticesColorAndTexture(writer, params);
    } else if (fHasColor) {
        writeVerticesColor(writer, params);
    } else if (fHasTexture) {
        writeVerticesTexture(writer, params);
    } else {
        SkVerticesPriv info(params.geometry().verticesData().vertices()->priv());
        const int vertexCount = info.vertexCount();
        const int indexCount = info.indexCount();
        const SkPoint* positions = info.positions();
        const uint16_t* indices = info.indices();

        DrawWriter::Vertices verts{*writer};
        verts.reserve(indexCount);

        VertState state(vertexCount, indices, indexCount);
        VertState::Proc vertProc = state.chooseProc(info.mode());
        while (vertProc(&state)) {
            SkV2 p[3] = {{positions[state.f0].x(), positions[state.f0].y()},
                        {positions[state.f1].x(), positions[state.f1].y()},
                        {positions[state.f2].x(), positions[state.f2].y()}};

            SkV4 devPoints[3];
            params.transform().mapPoints(p, devPoints, 3);
            verts.append(3) << devPoints[0].x << devPoints[0].y
                            << devPoints[1].x << devPoints[1].y
                            << devPoints[2].x << devPoints[2].y;
        }
    }
}

void VerticesRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                  SkPipelineDataGatherer* gatherer) const {
    // Vertices are currently pre-transformed to device space on the CPU so no transformation matrix
    // is needed. Store PaintDepth as a uniform to avoid copying the same depth for each vertex.
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)
    gatherer->write(params.order().depthAsFloat());
    gatherer->write(params.geometry().verticesData().skipColorXForm());

    // TODO check geometry within params instead of mem. attrib.s
    // if (fHasColor) {
        // SkVerticesPriv info(params.geometry().vertices()->priv());
        // const SkColor* colors = info.colors();
         // TODO: For now, only record one color as a uniform (first index's color; black)
         // SkPMColor4f::
        // gatherer->write(SK_PMColor4fBLACK);
    //         void write(const SkColor4f* colors, int numColors) { fUniformManager.write(colors, numColors); }
    // void write(const SkPMColor4f& premulColor) { fUniformManager.write(&premulColor, 1); }
    // }
    // if (fHasTexture) {
        // SkVerticesPriv info(params.geometry().vertices()->priv());
        // const SkPoint* texCoords = info.texCoords();

        // Recorder* recorder = params.geometry().vertices()->priv()
        // Get recorderPriv, textureDataCache()
        // pipelineDataCache<std::unique_ptr<SkTextureDataBlock>, SkTextureDataBlock>)
        // SkPipelineData::SkTextureDataBlock textureInfo() sampler key

        // For each texture, gatherer->add(samplingOption, tileMode, textureInfo.proxy)
    // }
}

const char* VerticesRenderStep::fragmentColorSkSL() const {
    if (!fHasColor) { return R"()"; }
    else {
        return R"(
        outputColor = half4(color);
        )";
    }
}

std::string VerticesRenderStep::texturesAndSamplersSkSL(int startBinding) const {
  // if (!fHasTexture) { return R"()"; }
  // else {
  //     return R"(
  //     outputColor = half4(color);
  //     )";
  // }
  // return "layout(binding=%d) uniform sampler2D texture;\n", binding);
    return R"()";
}

void VerticesRenderStep::writeVerticesColorAndTexture(DrawWriter* writer, const DrawParams& params) const {
    SkVerticesPriv info(params.geometry().verticesData().vertices()->priv());
    const int vertexCount = info.vertexCount();
    const int indexCount = info.indexCount();
    const SkPoint* positions = info.positions();
    const uint16_t* indices = info.indices();
    const SkColor* colors = info.colors();
    const SkPoint* texCoords = info.texCoords();

    DrawWriter::Vertices verts{*writer};

    VertState state(vertexCount, indices, indexCount);
    VertState::Proc vertProc = state.chooseProc(info.mode());

    while (vertProc(&state)) {
        SkV2 p[3] = {{positions[state.f0].x(), positions[state.f0].y()},
                     {positions[state.f1].x(), positions[state.f1].y()},
                     {positions[state.f2].x(), positions[state.f2].y()}};

        SkV4 devPoints[3];
        params.transform().mapPoints(p, devPoints, 3);

        SkColor4f color0 = SkColor4f::FromColor(colors[state.f0]);
        SkColor4f color1 = SkColor4f::FromColor(colors[state.f1]);
        SkColor4f color2 = SkColor4f::FromColor(colors[state.f2]);

        SkV4 c[3] = {{color0.fR, color0.fG, color0.fB, color0.fA},
                     {color1.fR, color1.fG, color1.fB, color1.fA},
                     {color2.fR, color2.fG, color2.fB, color2.fA}};

        SkV2 t[3] = {{texCoords[state.f0].x(), texCoords[state.f0].y()},
                     {texCoords[state.f1].x(), texCoords[state.f1].y()},
                     {texCoords[state.f2].x(), texCoords[state.f2].y()}};

        verts.append(3) << devPoints[0].x << devPoints[0].y << c[0] << SkPoint{t[0].x, t[0].y}
                        << devPoints[1].x << devPoints[1].y << c[1] << SkPoint{t[1].x, t[1].y}
                        << devPoints[2].x << devPoints[2].y << c[2] << SkPoint{t[2].x, t[2].y};

    }
}

void VerticesRenderStep::writeVerticesColor(DrawWriter* writer, const DrawParams& params) const {
    SkVerticesPriv info(params.geometry().verticesData().vertices()->priv());
    const int vertexCount = info.vertexCount();
    const int indexCount = info.indexCount();
    const SkPoint* positions = info.positions();
    const uint16_t* indices = info.indices();
    const SkColor* colors = info.colors();

    DrawWriter::Vertices verts{*writer};
    VertState state(vertexCount, indices, indexCount);
    VertState::Proc vertProc = state.chooseProc(info.mode());

    while (vertProc(&state)) {
        SkV2 p[3] = {{positions[state.f0].x(), positions[state.f0].y()},
                     {positions[state.f1].x(), positions[state.f1].y()},
                     {positions[state.f2].x(), positions[state.f2].y()}};

        SkV4 devPoints[3];
        params.transform().mapPoints(p, devPoints, 3);

        SkColor4f color0 = SkColor4f::FromColor(colors[state.f0]);
        SkColor4f color1 = SkColor4f::FromColor(colors[state.f1]);
        SkColor4f color2 = SkColor4f::FromColor(colors[state.f2]);

        SkV4 c[3] = {{color0.fR, color0.fG, color0.fB, color0.fA},
                     {color1.fR, color1.fG, color1.fB, color1.fA},
                     {color2.fR, color2.fG, color2.fB, color2.fA}};
        verts.append(3) << SkPoint{devPoints[0].x, devPoints[0].y} << c[0]
                        << SkPoint{devPoints[1].x, devPoints[1].y} << c[1]
                        << SkPoint{devPoints[2].x, devPoints[2].y} << c[2];
    }
}

void VerticesRenderStep::writeVerticesTexture(DrawWriter* writer, const DrawParams& params) const {
    SkVerticesPriv info(params.geometry().verticesData().vertices()->priv());
    const int vertexCount = info.vertexCount();
    const int indexCount = info.indexCount();
    const SkPoint* positions = info.positions();
    const uint16_t* indices = info.indices();
    const SkPoint* texCoords = info.texCoords();

    DrawWriter::Vertices verts{*writer};

    VertState state(vertexCount, indices, indexCount);
    VertState::Proc vertProc = state.chooseProc(info.mode());
    while (vertProc(&state)) {
        SkV2 p[3] = {{positions[state.f0].x(), positions[state.f0].y()},
                     {positions[state.f1].x(), positions[state.f1].y()},
                     {positions[state.f2].x(), positions[state.f2].y()}};

        SkV4 devPoints[3];
        params.transform().mapPoints(p, devPoints, 3);

        SkV2 t[3] = {{texCoords[state.f0].x(), texCoords[state.f0].y()},
                      {texCoords[state.f1].x(), texCoords[state.f1].y()},
                      {texCoords[state.f2].x(), texCoords[state.f2].y()}};
        verts.append(3) << SkPoint{devPoints[0].x, devPoints[0].y} << SkPoint{t[0].x, t[0].y}
                        << SkPoint{devPoints[1].x, devPoints[1].y} << SkPoint{t[1].x, t[1].y}
                        << SkPoint{devPoints[2].x, devPoints[2].y} << SkPoint{t[2].x, t[2].y};
    }
}

}  // namespace skgpu::graphite
