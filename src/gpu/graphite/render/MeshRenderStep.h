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
class PipelineDataGatherer;

class MeshRenderStep final : public RenderStep {
public:
    class HashKey {
    public:
        explicit HashKey(const SkMesh& mesh);

        bool operator==(const HashKey& other) const {
            const SkSL::Program* vs = SkMeshSpecificationPriv::VS(*fMeshSpec);
            const SkSL::Program* fs = SkMeshSpecificationPriv::FS(*fMeshSpec);
            const SkSL::Program* otherVS = SkMeshSpecificationPriv::VS(*other.fMeshSpec);
            const SkSL::Program* otherFS = SkMeshSpecificationPriv::FS(*other.fMeshSpec);

            SkSpan<const SkMeshSpecification::Attribute> attrs = fMeshSpec->attributes();
            SkSpan<const SkMeshSpecification::Attribute> otherAttrs = other.fMeshSpec->attributes();
            if (attrs.size() != otherAttrs.size()) {
                return false;
            }

            for (size_t i = 0; i < attrs.size(); i++) {
                if (attrs[i].type != otherAttrs[i].type) {
                    return false;
                }
                if (attrs[i].offset != otherAttrs[i].offset) {
                    return false;
                }
            }

            return fPrimitiveType == other.fPrimitiveType &&
                   vs->fSource == otherVS->fSource &&
                   fs->fSource == otherFS->fSource &&
                   fMeshSpec->stride() == other.fMeshSpec->stride();
        }

    private:
        friend std::hash<HashKey>;

        PrimitiveType fPrimitiveType;
        sk_sp<SkMeshSpecification> fMeshSpec;
    };

    static constexpr char kMeshVSMainName[] = "drawMeshVSMain";
    static constexpr char kMeshFSMainName[] = "drawMeshFSMain";

    ~MeshRenderStep() override;

    static std::unique_ptr<MeshRenderStep> Make(RenderStepID, Layout, const SkMesh&);

    std::string vertexSkSL() const override;
    std::string vertexSkSLTopLevel(const ShaderInfo*) const override;

    void writeVertices(DrawWriter* writer,
                       const DrawParams& params,
                       uint32_t ssboIndex) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;
    std::string fragmentColorSkSL() const override;
    std::string fragmentColorSkSLTopLevel(const ShaderInfo*) const override;
    const char* fragmentColorSkSLLocalCoordsVariable() const override;

private:
    explicit MeshRenderStep(RenderStepID, Layout,
                            sk_sp<SkMeshSpecification>, SkMesh::Mode,
                            std::vector<Attribute>, std::vector<Varying>,
                            std::unordered_map<std::string, std::string> mangledVaryingsMap,
                            std::vector<Uniform>, bool hasColorOutput);

    sk_sp<SkMeshSpecification> fSpec;
    const std::vector<Attribute> fAttributes;
    // Map storing mangled varying name -> unmangled name. Since the user can declare
    // the name of their attributes and varyings, it is possible they use the same name for
    // both causing a SkSL variable name collision, so we mangle the varyings names to prevent that.
    // To satisfy their custom VS and FS, we need to know the mapping to the user-defined names when
    // populating the Varyings struct.
    std::unordered_map<std::string, std::string> fMangledVaryingsMap;

    bool fHasColorOutput;
};

}  // namespace skgpu::graphite

namespace std {
    template<> struct hash<skgpu::graphite::MeshRenderStep::HashKey> {
        size_t operator()(const skgpu::graphite::MeshRenderStep::HashKey& key) const {
            SkMeshSpecification* spec = key.fMeshSpec.get();

            size_t hash = SkChecksum::Hash64(&key.fPrimitiveType, sizeof(key.fPrimitiveType));

            const SkSL::Program* vs = SkMeshSpecificationPriv::VS(*spec);
            const SkSL::Program* fs = SkMeshSpecificationPriv::FS(*spec);
            hash = SkChecksum::Hash64(vs->fSource->c_str(), vs->fSource->size(), hash);
            hash = SkChecksum::Hash64(fs->fSource->c_str(), fs->fSource->size(), hash);

            // Hash the attributes since they contain offset and CPU types which are not captured
            // in the VS and FS SkSL hash.
            for (const auto& a : spec->attributes()) {
                hash = SkChecksum::Hash32(&a.offset, sizeof(a.offset), hash);
                hash = SkChecksum::Hash32(&a.type,   sizeof(a.type),   hash);
            }

            size_t stride = spec->stride();
            hash = SkChecksum::Hash32(&stride, sizeof(stride), hash);

            return hash;
        }
    };
}

#endif // skgpu_graphite_render_MeshRenderStep_DEFINED

