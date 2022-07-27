/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/TextDirectRenderStep.h"

#include "src/core/SkPipelineData.h"

#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkSLString.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/text/AtlasManager.h"
#include "src/text/gpu/SubRunContainer.h"

using AtlasSubRun = sktext::gpu::AtlasSubRun;

namespace skgpu::graphite {

namespace {
static constexpr DepthStencilSettings kDirectShadingPass = {
        /*frontStencil=*/{},
        /*backStencil=*/ {},
        /*refValue=*/    0,
        /*stencilTest=*/ false,
        /*depthCompare=*/CompareOp::kGEqual,
        /*depthTest=*/   true,
        /*depthWrite=*/  true
};

constexpr size_t kQuadVertexBufferSize = sizeof(float)*8;
void write_quad_vertex_buffer(VertexWriter vertexWriter, size_t bufferSize) {
    SkASSERT(bufferSize == kQuadVertexBufferSize);
    vertexWriter << 0.0f << 0.0f;
    vertexWriter << 1.0f << 0.0f;
    vertexWriter << 0.0f << 1.0f;
    vertexWriter << 1.0f << 1.0f;
}

// We are expecting to sample from up to 4 textures
constexpr int kNumTextAtlasTextures = 4;
}  // namespace


TextDirectRenderStep::TextDirectRenderStep()
        : RenderStep("TextDirectRenderStep",
                     "",
                     Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage,
                     /*uniforms=*/{{"atlasSizeInv", SkSLType::kFloat2}},
                     PrimitiveType::kTriangles,
                     kDirectShadingPass,
                     /*vertexAttrs=*/
                     {{"baseCoords", VertexAttribType::kFloat2, SkSLType::kFloat2}},
                     /*instanceAttrs=*/
                     {{"instancePos", VertexAttribType::kFloat4, SkSLType::kFloat4},
                      {"instanceUV", VertexAttribType::kUShort4, SkSLType::kUShort4}},
                      {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                      {"index", VertexAttribType::kFloat, SkSLType::kFloat}},
                     /*varyings=*/
                     {{"textureCoords", SkSLType::kFloat2},
                      {"texIndex", SkSLType::kFloat}}){}

TextDirectRenderStep::~TextDirectRenderStep() {}

const char* TextDirectRenderStep::vertexSkSL() const {
    return R"(
        int2 coords = int2(texCoords.x, texCoords.y);

        float2 position = baseCoords*instancePos.xy + instancePos.zw;
        float2 unormTexCoords = baseCoords*instanceUV.xy + instanceUV.zw;

        textureCoords = unormTexCoords * atlasSizeInv;
        texIndex = index;

        float4 devPosition = float4(position, depth, 1);
    )";
}

std::string TextDirectRenderStep::texturesAndSamplersSkSL(int binding) const {
    std::string result;

    for (unsigned int i = 0; i < kNumTextAtlasTextures; ++i) {
        SkSL::String::appendf(&result,
                              "layout(binding=%d) uniform sampler2D text_atlas_%d;\n", binding, i);
        binding++;
    }

    return result;
}

const char* TextDirectRenderStep::fragmentCoverageSkSL() const {
    // TODO: handle color textures
    return R"(
        half4 texColor;
        if (texIndex == 0) {
           texColor = sample(text_atlas_0, textureCoords).rrrr;
        } else if (texIndex == 1) {
           texColor = sample(text_atlas_1, textureCoords).rrrr;
        } else if (texIndex == 2) {
           texColor = sample(text_atlas_2, textureCoords).rrrr;
        } else if (texIndex == 3) {
           texColor = sample(text_atlas_3, textureCoords).rrrr;
        } else {
           texColor = sample(text_atlas_0, textureCoords).rrrr;
        }
        outputCoverage = texColor;
    )";
}

void TextDirectRenderStep::writeVertices(DrawWriter* dw, const DrawParams& params) const {
    BindBufferInfo fixedVertexBuffer = dw->bufferManager()->getStaticBuffer(
            BufferType::kVertex,
            write_quad_vertex_buffer,
            kQuadVertexBufferSize);

    const SubRunData& subRunData = params.geometry().subRunData();
    // TODO: pass through the color from the SkPaint via the SubRunData
    subRunData.subRun()->fillInstanceData(dw, subRunData.startGlyphIndex(), subRunData.glyphCount(),
                                          params.order().depthAsFloat(),
                                          params.transform());
}

void TextDirectRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                    SkPipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    const SubRunData& subRunData = params.geometry().subRunData();
    unsigned int numProxies;
    Recorder* recorder = subRunData.recorder();
    const sk_sp<TextureProxy>* proxies =
            recorder->priv().atlasManager()->getProxies(subRunData.subRun()->maskFormat(),
                                                        &numProxies);
    SkASSERT(proxies && numProxies > 0);

    // write uniforms
    skvx::float2 atlasDimensionsInverse = {1.f/proxies[0]->dimensions().width(),
                                           1.f/proxies[0]->dimensions().height()};
    gatherer->write(atlasDimensionsInverse);

    // write textures and samplers
    const SkSamplingOptions kSamplingOptions(SkFilterMode::kNearest);
    constexpr SkTileMode kTileModes[2] = { SkTileMode::kClamp, SkTileMode::kClamp };
    for (unsigned int i = 0; i < numProxies; ++i) {
        gatherer->add(kSamplingOptions, kTileModes, proxies[i]);
    }
    // If the atlas has less than 4 active proxies we still need to set up samplers for the shader.
    for (unsigned int i = numProxies; i < kNumTextAtlasTextures; ++i) {
        gatherer->add(kSamplingOptions, kTileModes, proxies[0]);
    }
}

}  // namespace skgpu::graphite
