/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCustomMesh.h"

#include "src/gpu/GrShaderCaps.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLSharedCompiler.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#include <locale>

using Attribute = SkCustomMeshSpecification::Attribute;
using Varying   = SkCustomMeshSpecification::Varying;

std::atomic<int> SkCustomMeshSpecification::gCurrID{0};

#define RETURN_FAILURE(...) return Result{nullptr, SkStringPrintf(__VA_ARGS__)}

#define RETURN_ERROR(...) return std::make_tuple(false, SkStringPrintf(__VA_ARGS__))

#define RETURN_SUCCESS return std::make_tuple(true, SkString{})

static const char* child_type_name(const SkSL::Type& type) {
    switch (type.typeKind()) {
        case SkSL::Type::TypeKind::kBlender:     return "blender";
        case SkSL::Type::TypeKind::kColorFilter: return "colorfilter";
        case SkSL::Type::TypeKind::kShader:      return "shader";
        default: SkUNREACHABLE;
    }
}

static std::tuple<bool, SkString> check_program_for_disallowed_types(const SkSL::Program& p) {
    for (const SkSL::ProgramElement* elem : p.elements()) {
        if (elem->is<SkSL::GlobalVarDeclaration>()) {
            const SkSL::GlobalVarDeclaration& global =
                    elem->as<SkSL::GlobalVarDeclaration>();
            const SkSL::VarDeclaration& varDecl =
                    global.declaration()->as<SkSL::VarDeclaration>();

            const SkSL::Variable& var = varDecl.var();
            const SkSL::Type& varType = var.type();

            if (var.modifiers().fFlags & SkSL::Modifiers::kUniform_Flag) {
                RETURN_ERROR("Uniforms not allowed (%.*s on line %d).",
                             static_cast<int>(var.name().length()),
                             var.name().data(),
                             var.fLine);
            }
            if (varType.isEffectChild()) {
                RETURN_ERROR("%s not allowed (%.*s on line %d).",
                             child_type_name(varType),
                             static_cast<int>(var.name().length()),
                             var.name().data(),
                             var.fLine);
            }
        }
    }
    RETURN_SUCCESS;
};

static bool has_color(const SkSL::Program& fsProgram) {
    for (const SkSL::ProgramElement* elem : fsProgram.elements()) {
        if (elem->is<SkSL::FunctionDefinition>()) {
            const SkSL::FunctionDefinition& defn = elem->as<SkSL::FunctionDefinition>();
            const SkSL::FunctionDeclaration& decl = defn.declaration();
            if (decl.isMain()) {
                SkASSERT(decl.parameters().size() == 1 || decl.parameters().size() == 2);
                return decl.parameters().size() == 2;
            }
        }
    }
    SkUNREACHABLE;
}

// This is a non-exhaustive check for the validity of a variable name. The SkSL compiler will
// actually process the name. We're just guarding against having multiple tokens embedded in the
// name before we put it into a struct definition.
static bool check_name(const SkString& name) {
    for (size_t i = 0; i < name.size(); ++i) {
        if (name[i] != '_' && !std::isalnum(name[i], std::locale::classic())) {
            return false;
        }
    }
    return true;
}

static size_t attribute_type_size(Attribute::Type type) {
    switch (type) {
        case Attribute::Type::kFloat4:        return 4*4;
        case Attribute::Type::kUByte4_unorm:  return 4;
    }
    SkUNREACHABLE;
};

const char* attribute_type_string(Attribute::Type type) {
    switch (type) {
        case Attribute::Type::kFloat4:        return "float4";
        case Attribute::Type::kUByte4_unorm:  return "half4";
    }
    SkUNREACHABLE;
};

const char* varying_type_string(Varying::Type type) {
    switch (type) {
        case Varying::Type::kFloat:  return "float";
        case Varying::Type::kFloat2: return "float2";
        case Varying::Type::kFloat3: return "float3";
        case Varying::Type::kFloat4: return "float4";
        case Varying::Type::kHalf:   return "half";
        case Varying::Type::kHalf2:  return "half2";
        case Varying::Type::kHalf3:  return "half3";
        case Varying::Type::kHalf4:  return "half4";
    }
    SkUNREACHABLE;
};

std::tuple<bool, SkString>
check_vertex_offsets_and_stride(SkSpan<const Attribute> attributes,
                                size_t                  stride) {
    // Vulkan 1.0 has a minimum maximum vertex stride of 2048.
    static constexpr size_t kMaxStride     = 1024;
    // Vulkan 1.0 has a minimum maximum attribute count of 16.
    static constexpr size_t kMaxAttributes =  16;
    // Required by Metal
    static constexpr size_t kStrideAlignment = 4;
    // Required by Metal
    static constexpr size_t kOffsetAlignment = 4;
    if (attributes.empty()) {
        RETURN_ERROR("At least 1 attribute is required.");
    }
    if (attributes.size() > kMaxAttributes) {
        RETURN_ERROR("A maximum of %zu attributes is allowed.", kMaxAttributes);
    }
    static_assert(SkIsPow2(kStrideAlignment));
    if (stride == 0 || stride & (kStrideAlignment - 1)) {
        RETURN_ERROR("Vertex stride must be a non-zero multiple of %zu.", kStrideAlignment);
    }
    if (stride > kMaxStride) {
        RETURN_ERROR("Stride cannot exceed %zu.", kMaxStride);
    }
    for (const auto& a : attributes) {
        if (a.offset & (kOffsetAlignment - 1)) {
            RETURN_ERROR("Attribute offset must be a multiple of %zu.", kOffsetAlignment);
        }
        // This equivalent to vertexAttributeAccessBeyondStride==VK_FALSE in
        // VK_KHR_portability_subset. First check is to avoid overflow in second check.
        if (a.offset > stride || a.offset + attribute_type_size(a.type) > stride) {
            RETURN_ERROR("Attribute offset plus size cannot exceed stride.");
        }
    }
    RETURN_SUCCESS;
}

SkCustomMeshSpecification::Result SkCustomMeshSpecification::Make(
        SkSpan<const Attribute> attributes,
        size_t                  stride,
        SkSpan<const Varying>   varyings,
        const SkString&         vs,
        const SkString&         fs) {
    if (auto [ok, error] = check_vertex_offsets_and_stride(attributes, stride); !ok) {
        return {nullptr, error};
    }

    SkString attributesStruct("struct Attributes {\n");
    for (const auto& a : attributes) {
        if (!check_name(a.name)) {
            RETURN_FAILURE("\"%s\" is not a valid attribute name.", a.name.c_str());
        }
        attributesStruct.appendf("  %s %s;\n", attribute_type_string(a.type), a.name.c_str());
    }
    attributesStruct.append("};\n");

    SkString varyingStruct("struct Varyings {\n");
    for (const auto& v : varyings) {
        if (!check_name(v.name)) {
            return {nullptr, SkStringPrintf("\"%s\" is not a valid varying name.", v.name.c_str())};
        }
        varyingStruct.appendf("  %s %s;\n", varying_type_string(v.type), v.name.c_str());
    }
    varyingStruct.append("};\n");

    std::unique_ptr<SkSL::Program> vsProgram;
    std::unique_ptr<SkSL::Program> fsProgram;
    {
        // We keep this SharedCompiler in a separate scope to make sure it's destroyed before
        // calling the Make overload at the end, which creates its own (non-reentrant)
        // SharedCompiler instance
        SkSL::SharedCompiler compiler;
        SkSL::Program::Settings settings;
        settings.fInlineThreshold = 0;
        settings.fForceNoInline = true;
        settings.fEnforceES2Restrictions = true;
        SkSL::String fullVS;
        fullVS.append(varyingStruct.c_str());
        fullVS.append(attributesStruct.c_str());
        fullVS.append(vs.c_str());

        vsProgram = compiler->convertProgram(SkSL::ProgramKind::kRuntimeCustomMeshVertex,
                                             fullVS,
                                             settings);
        if (!vsProgram) {
            RETURN_FAILURE("VS: %s", compiler->errorText().c_str());
        }
        if (auto [ok, error] = check_program_for_disallowed_types(*vsProgram); !ok) {
            RETURN_FAILURE("VS: %s", error.c_str());
        }

        SkSL::String fullFS;
        fullFS.append(varyingStruct.c_str());
        fullFS.append(fs.c_str());
        fsProgram = compiler->convertProgram(SkSL::ProgramKind::kRuntimeCustomMeshFragment,
                                             fullFS,
                                             settings);
        if (!fsProgram) {
            RETURN_FAILURE("FS: %s", compiler->errorText().c_str());
        }
        if (auto [ok, error] = check_program_for_disallowed_types(*fsProgram); !ok) {
            RETURN_FAILURE("FS: %s", error.c_str());
        }
    }

    // TODO: Validate that attr offset alignments and stride are valid.
    return {sk_sp<SkCustomMeshSpecification>(new SkCustomMeshSpecification(attributes,
                                                                           stride,
                                                                           varyings,
                                                                           std::move(vsProgram),
                                                                           std::move(fsProgram))),
            /*error*/{}};
}

SkCustomMeshSpecification::~SkCustomMeshSpecification() = default;

SkCustomMeshSpecification::SkCustomMeshSpecification(
        SkSpan<const Attribute>         attributes,
        size_t                          stride,
        SkSpan<const Varying>           varyings,
        std::unique_ptr<SkSL::Program>  vs,
        std::unique_ptr<SkSL::Program>  fs)
        : fAttributes(attributes.begin(), attributes.end())
        , fVaryings(varyings.begin(), varyings.end())
        , fVS(std::move(vs))
        , fFS(std::move(fs))
        , fStride(stride)
        , fHasColors(has_color(*fFS))
        , fID(++gCurrID) {}
