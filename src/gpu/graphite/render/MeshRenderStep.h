/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_MeshRenderStep_DEFINED
#define skgpu_graphite_render_MeshRenderStep_DEFINED

#include "include/core/SkMesh.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkMeshPriv.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/ShaderInfo.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/codegen/SkSLPipelineStageCodeGenerator.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#include <cstdint>
#include <unordered_map>

namespace skgpu::graphite {

class DrawParams;
class DrawWriter;
struct RootNodesInfo;
class PipelineDataGatherer;

class MeshRenderStep final : public RenderStep {
public:
    static constexpr char kMeshVSMainName[] = "drawMeshVSMain";
    static constexpr char kMeshFSMainName[] = "drawMeshFSMain";
    static constexpr char kMeshVaryingMangleSuffix[] = "_SkMeshSpecificationUniform";

    explicit MeshRenderStep(Layout, PrimitiveType);
    ~MeshRenderStep() override;

    std::string vertexSkSL(const RootNodesInfo&) const override;
    std::string fragmentColorSkSL(const RootNodesInfo&) const override;
    const char* fragmentColorSkSLLocalCoordsVariable() const override;

    void writeVertices(DrawWriter* writer,
                       const DrawParams& params,
                       uint32_t ssboIndex) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;
    size_t staticDataStride(const DrawParams& params) const override;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_MeshRenderStep_DEFINED

