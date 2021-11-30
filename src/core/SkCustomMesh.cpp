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
#include "src/sksl/ir/SkSLProgram.h"

std::atomic<int> SkCustomMeshSpecification::gCurrID{0};

#define RETURN_FAILURE(...) return Result{nullptr, SkStringPrintf(__VA_ARGS__)}

SkCustomMeshSpecification::Result SkCustomMeshSpecification::Make(
        SkSpan<const Attribute> attributes,
        size_t                  stride,
        SkSpan<const Varying>   varyings,
        const SkString&         vs,
        const SkString&         fs) {
    auto attributeTypeString = [](Attribute::Type type) {
        switch (type) {
            case Attribute::Type::kFloat4:        return "float4";
            case Attribute::Type::kUByte4_unorm:  return "half4";
        }
        SkUNREACHABLE;
    };
    SkString attributesStruct("struct Attributes {\n");
    for (const auto& a : attributes) {
        // TODO: Validate a name is a valid name here?
        attributesStruct.appendf("  %s %s;\n", attributeTypeString(a.type), a.name.c_str());
    }
    attributesStruct.append("};\n");

    auto varyingTypeString = [](Varying::Type type) {
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
    SkString varyingStruct("struct Varyings {\n");
    for (const auto& v : varyings) {
        // TODO: Validate varying name is a valid name here?
        varyingStruct.appendf("  %s %s;\n", varyingTypeString(v.type), v.name.c_str());
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

        SkSL::String fullFS;
        fullFS.append(varyingStruct.c_str());
        fullFS.append(fs.c_str());
        fsProgram = compiler->convertProgram(SkSL::ProgramKind::kRuntimeCustomMeshFragment,
                                             fullFS,
                                             settings);
        if (!fsProgram) {
            RETURN_FAILURE("FS: %s", compiler->errorText().c_str());
        }
    }
    // TODO: Check that user code doesn't declare anything we don't know how to bind to, e.g.
    // children or uniforms.

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
        , fID(++gCurrID) {}
