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
#include "src/gpu/graphite/Renderer.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/codegen/SkSLPipelineStageCodeGenerator.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#include <cstdint>
#include <string>

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

    static constexpr char kTransformMatrixUniformName[] = "transformMat";

    explicit MeshRenderStep(RenderStepID renderStepId, Layout layout,
                            sk_sp<SkMeshSpecification> spec, SkMesh::Mode mode,
                            std::vector<Attribute>, std::vector<Varying>,
                            std::vector<Uniform>);

    ~MeshRenderStep() override;

    std::string vertexSkSL() const override;
    std::string vertexSkSLTopLevel() const override;

    void writeVertices(DrawWriter* writer,
                       const DrawParams& params,
                       uint32_t ssboIndex) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;
    std::string fragmentColorSkSL() const override;
    std::string fragmentColorSkSLTopLevel() const override;

private:
    sk_sp<SkMeshSpecification> fSpec;
    const std::vector<Attribute> fAttributes;
    std::string fVSTopLevel;
    std::string fVSMain;
    std::string fFSTopLevel;
    std::string fFSMain;
    // True if the out color type of the main function is a float4 and needs to be coerced to a half4.
    bool        fFSColorConversion;
};

class MeshProgramCallbacks : public SkSL::PipelineStage::Callbacks {
public:
    MeshProgramCallbacks(std::string mainFn) : fMainFn(mainFn) {}

    std::string getMainName() override { return fMainFn; }
    void defineFunction(const char* declaration, const char* body, bool isMain) override {
        fPreamble = fPreamble.append(SkSL::String::printf("%s {\n %s }\n", declaration, body));
    }
    void declareFunction(const char* declaration) override {}
    void defineStruct(const char* definition) override {
        fPreamble = fPreamble.append(definition).append("\n");
    }
    void declareGlobal(const char* declaration) override {
        fPreamble = fPreamble.append(declaration).append("\n");
    }

    std::string declareUniform(const SkSL::VarDeclaration* decl) override {
        return std::string(decl->var()->name());
    }
    std::string sampleShader(int index, std::string coords) override {
        return "todoShaderEval" + std::to_string(index) + ".eval(" + coords + ')';
    }
    std::string sampleColorFilter(int index, std::string color) override {
        return color;
    }
    std::string sampleBlender(int index, std::string src, std::string dst) override {
        return src;
    }

    std::string toLinearSrgb(std::string color) override {
        return "toLinearSrgb(" + color + ')';
    }
    std::string fromLinearSrgb(std::string color) override {
        return "fromLinearSrgb(" + color + ')';
    }


    std::string fPreamble;
    std::string fMainFn;
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
