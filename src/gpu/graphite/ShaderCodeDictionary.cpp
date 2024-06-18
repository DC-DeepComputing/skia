/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ShaderCodeDictionary.h"

#include "include/core/SkSamplingOptions.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/Context.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/BlendFormula.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/ReadSwizzle.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/codegen/SkSLPipelineStageCodeGenerator.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#include <initializer_list>
#include <new>

using namespace skia_private;
using namespace SkKnownRuntimeEffects;

namespace skgpu::graphite {

static_assert(static_cast<int>(BuiltInCodeSnippetID::kLast) < kSkiaBuiltInReservedCnt);

// The toLinearSrgb and fromLinearSrgb RuntimeEffect intrinsics need to be able to map to and
// from the dst color space and linearSRGB. These are the 10 uniforms needed to allow that.
// These boil down to two copies of the kColorSpaceTransformUniforms uniforms. The first set
// for mapping to LinearSRGB and the second set for mapping from LinearSRGB.
static constexpr Uniform kRuntimeEffectColorSpaceTransformUniforms[] = {
        // to LinearSRGB
        { "flags_toLinear",          SkSLType::kInt },
        { "srcKind_toLinear",        SkSLType::kInt },
        { "gamutTransform_toLinear", SkSLType::kHalf3x3 },
        { "dstKind_toLinear",        SkSLType::kInt },
        { "csXformCoeffs_toLinear",  SkSLType::kHalf4x4 },
        // from LinearSRGB
        { "flags_fromLinear",          SkSLType::kInt },
        { "srcKind_fromLinear",        SkSLType::kInt },
        { "gamutTransform_fromLinear", SkSLType::kHalf3x3 },
        { "dstKind_fromLinear",        SkSLType::kInt },
        { "csXformCoeffs_fromLinear",  SkSLType::kHalf4x4 },
};

namespace {

const char* get_known_rte_name(StableKey key) {
    switch (key) {
#define M(type) case StableKey::k##type : return "KnownRuntimeEffect_" #type;
#define M1(type)
#define M2(type, initializer) case StableKey::k##type : return "KnownRuntimeEffect_" #type;
        SK_ALL_STABLEKEYS(M, M1, M2)
#undef M2
#undef M1
#undef M
    }

    SkUNREACHABLE;
}

std::string get_mangled_name(const std::string& baseName, int manglingSuffix) {
    return baseName + "_" + std::to_string(manglingSuffix);
}

std::string get_mangled_uniform_name(const ShaderInfo& shaderInfo,
                                     const Uniform& uniform,
                                     int manglingSuffix) {
    std::string result;

    if (uniform.isPaintColor()) {
        // Due to deduplication there will only ever be one of these
        result = uniform.name();
    } else {
        result = uniform.name() + std::string("_") + std::to_string(manglingSuffix);
    }

    // FIXME!
    if (shaderInfo.ssboIndex()) {
        result = SkSL::String::printf("uniforms.%s", result.c_str());
        // result = EmitStorageBufferAccess("fs", shaderInfo.ssboIndex(), result.c_str());
    }
    return result;
}

std::string get_mangled_sampler_name(const TextureAndSampler& tex, int manglingSuffix) {
    return tex.name() + std::string("_") + std::to_string(manglingSuffix);
}

std::string stitch_csv(SkSpan<const std::string> args) {
    std::string code = "";
    const char* separator = "";
    for (const std::string& arg : args) {
        code += separator;
        code += arg;
        separator = ", ";
    }

    return code;
}

static const ShaderSnippet::Args kDefaultArgs{"inColor", "destColor", "pos"};

// If 'args' is null, the generated list is assumed to be for parameter declarations. If it's non
// null, it is assumed to be the expressions to invoke the default signature.
void append_defaults(TArray<std::string>& list,
                     bool useStorageBuffer,
                     const ShaderNode* node,
                     const ShaderSnippet::Args* args) {
    if (useStorageBuffer) {
        list.push_back(args ? "uniforms" : "const FSUniformData& uniforms");
    }
    // Use the node's aggregate required flags so that the provided dynamic variables propagate
    // to the child nodes that require them.
    if (node->requiredFlags() & SnippetRequirementFlags::kPriorStageOutput) {
        list.push_back(args ? args->fPriorStageOutput.c_str() : "half4 inColor");
    }
    if (node->requiredFlags() & SnippetRequirementFlags::kBlenderDstColor) {
        list.push_back(args ? args->fBlenderDstColor.c_str() : "half4 destColor");
    }
    if (node->requiredFlags() & SnippetRequirementFlags::kLocalCoords) {
        list.push_back(args ? args->fFragCoord.c_str() : "float2 pos");
    }
}

void append_uniforms(TArray<std::string>& list,
                     const ShaderInfo& shaderInfo,
                     const ShaderNode* node,
                     SkSpan<const std::string> childOutputs) {
    const ShaderSnippet* entry = node->entry();
    if (!shaderInfo.useStorageBuffer()) {
        // The uniforms are in the global scope, so just pass in the ones bound to 'node'
        for (int i = 0; i < entry->fUniforms.size(); ++i) {
            list.push_back(get_mangled_uniform_name(shaderInfo, entry->fUniforms[i], node->keyIndex()));
        }
    }

    // Append samplers (the same regardless of storing other data in storage buffers)
    for (int i = 0; i < entry->fTexturesAndSamplers.size(); ++i) {
        list.push_back(get_mangled_sampler_name(entry->fTexturesAndSamplers[i], node->keyIndex()));
    }

    // Append child output names.
    if (!childOutputs.empty()) {
        list.push_back_n(childOutputs.size(), childOutputs.data());
    }
}

// If we have no children, the default expression just calls a built-in snippet with the signature:
//     half4 BuiltinFunctionName(/* required variable inputs (e.g. float2 pos) */,
//                               /* all uniforms as parameters (bound to node's values */) { ... }
// If we do have children, we will have created a glue function in the preamble and that is called
// instead. Its signature looks like this:
//     half4 BuiltinFunctionName_N(/* uniform struct if using SSBOs */,
//                                 /* required variable inputs (e.g. float2 pos) */) { ... }
std::string invoke_default_expression(const ShaderInfo& shaderInfo,
                                      const ShaderNode* node,
                                      const ShaderSnippet::Args& args) {
    std::string fnName;
    // Use 4 to cover the fairly common case of 1 input, 1 ssbo struct, and either a texture or a
    // second input.
    STArray<3, std::string> params;

    if (node->numChildren() == 0) {
        // FIXME: if this is a runtime effect with no children we still need to use the mangled
        // name. Maybe we take the other branch if we don't have a static function name?

        // We didn't generate a helper function in the preamble, so add an globally scoped uniforms
        // to the parameter list and call the static function directly.
        fnName = node->entry()->fStaticFunctionName;
        // We pass in false for useStorageBuffer since we don't want to call the static function
        // with the struct.
        append_defaults(params, /*useStorageBuffer=*/false, node, &args);
        // Here, if useStorageBuffer is true, the uniform expressions will be bound to the node's
        // values within the struct assumed to be in the calling context.
        append_uniforms(params, shaderInfo, node, /*childOutputs=*/{});
    } else {
        // Invoke the generated helper function added to the preamble, which will handle invoking
        // any children and appending their values to the rest of the static fn's arguments.
        // FIXME and maybe we always use the generated helper names based on fName instead?
        fnName = get_mangled_name(node->entry()->fStaticFunctionName, node->keyIndex());
        append_defaults(params, shaderInfo.useStorageBuffer(), node, &args);
    }

    return SkSL::String::printf("%s(%s)", fnName.c_str(), stitch_csv(params).c_str());
}

// Returns an expression to invoke this entry.
std::string emit_expression_for_entry(const ShaderInfo& shaderInfo,
                                      const ShaderNode* node,
                                      ShaderSnippet::Args args) {
    if (node->entry()->fExpressionGenerator) {
        return node->entry()->fExpressionGenerator(shaderInfo, node, args);
    } else {
        return invoke_default_expression(shaderInfo, node, args);
    }
}

// Emit the glue code needed to invoke a single static helper isolated within its own scope.
// Glue code will assign the resulting color into a variable `half4 out%d`, where the %d is
// filled in with 'node->keyIndex()'.
std::string emit_glue_code_for_entry(const ShaderInfo& shaderInfo,
                                     const ShaderNode* node,
                                     const ShaderSnippet::Args& args,
                                     std::string* funcBody) {
    std::string expr = emit_expression_for_entry(shaderInfo, node, args);
    std::string outputVar = get_mangled_name("out", node->keyIndex());
    SkSL::String::appendf(funcBody,
                          "// [%d] %s\n"
                          "half4 %s = %s;",
                          node->keyIndex(),
                          node->entry()->fName,
                          outputVar.c_str(),
                          expr.c_str());
    return outputVar;
}

// Includes opening `{` for the helper function body
std::string emit_helper_declaration(const ShaderInfo& shaderInfo,
                                    const ShaderNode* node) {
    const ShaderSnippet* entry = node->entry();
    std::string helperFnName = get_mangled_name(
            entry->fStaticFunctionName ? entry->fStaticFunctionName : entry->fName, node->keyIndex());

    STArray<3, std::string> params;
    append_defaults(params, shaderInfo.useStorageBuffer(), node, /*args=*/nullptr); // null args emits declarations

    return SkSL::String::printf("half4 %s(%s) {",
            helperFnName.c_str(),
            stitch_csv(params).c_str());
}

// If we have no children, we don't need to add anything into the preamble.
// If we have child entries, we create a function in the preamble with a signature of:
//     half4 BuiltinFunctionName_N(/* uniform struct if using SSBOs */,
//                                 /* required variable inputs (e.g. float2 pos) */) { ... }
// This function invokes each child in sequence, and then calls the built-in function, passing all
// uniforms and child outputs along:
//     half4 BuiltinFunctionName(/* all uniforms as parameters (bound to node's values) */,
//                               /* all child output variable names as parameters */,
//                               /* required variable inputs */);
std::string emit_default_preamble(const ShaderInfo& shaderInfo,
                                  const ShaderNode* node) {
    if (node->numChildren() == 0) {
        // We don't need a helper function to wrap the snippet's static function
        return "";
    }

    std::string code = emit_helper_declaration(shaderInfo, node);

    // Invoke each child with unmodified input values and collect in a list of local variables
    STArray<2, std::string> childOutputVarNames;
    for (const ShaderNode* child : node->children()) {
        // Emit glue code into our helper function body (i.e. lifting the child execution up front
        // so their outputs can be passed to the static module function for the node's snippet).
        childOutputVarNames.push_back(emit_glue_code_for_entry(shaderInfo, child, kDefaultArgs, &code));
    }

    // Finally, invoke the snippet from the helper function, passing uniforms and child outputs.
    STArray<3, std::string> params;
    append_defaults(params, shaderInfo.useStorageBuffer(), node, &kDefaultArgs);
    append_uniforms(params, shaderInfo, node, childOutputVarNames);

    SkSL::String::appendf(&code,
                              "return %s(%s);"
                          "}",
                          node->entry()->fStaticFunctionName,
                          stitch_csv(params).c_str());
    return code;
}

// Walk the node tree and generate all preambles, accumulating into 'preamble'.
void emit_preambles(const ShaderInfo& shaderInfo,
                    SkSpan<const ShaderNode*> nodes,
                    std::string treeLabel,
                    std::string* preamble) {
    for (int i = 0; i < SkTo<int>(nodes.size()); ++i) {
        const ShaderNode* node = nodes[i];
        std::string nodeLabel = std::to_string(i);
        std::string nextLabel = treeLabel.empty() ? nodeLabel
                                                  : (treeLabel + "<-" + nodeLabel);

        if (node->numChildren() > 0) {
            emit_preambles(shaderInfo, node->children(), nextLabel, preamble);
        }

        std::string nodePreamble;
        if (node->entry()->fPreambleGenerator) {
            nodePreamble = node->entry()->fPreambleGenerator(shaderInfo, node);
        } else {
            nodePreamble = emit_default_preamble(shaderInfo, node);
        }
        if (!nodePreamble.empty()) {
            SkSL::String::appendf(preamble,
                                "// [%d]   %s: %s\n"
                                "%s\n",
                                node->keyIndex(), nextLabel.c_str(), node->entry()->fName,
                                nodePreamble.c_str());
        }
    }
}

constexpr skgpu::BlendInfo make_simple_blendInfo(skgpu::BlendCoeff srcCoeff,
                                                 skgpu::BlendCoeff dstCoeff) {
    return { skgpu::BlendEquation::kAdd,
             srcCoeff,
             dstCoeff,
             SK_PMColor4fTRANSPARENT,
             skgpu::BlendModifiesDst(skgpu::BlendEquation::kAdd, srcCoeff, dstCoeff) };
}

static constexpr int kNumCoeffModes = (int)SkBlendMode::kLastCoeffMode + 1;
static constexpr skgpu::BlendInfo gBlendTable[kNumCoeffModes] = {
        /* clear */      make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kZero),
        /* src */        make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kZero),
        /* dst */        make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kOne),
        /* src-over */   make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISA),
        /* dst-over */   make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kOne),
        /* src-in */     make_simple_blendInfo(skgpu::BlendCoeff::kDA,   skgpu::BlendCoeff::kZero),
        /* dst-in */     make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kSA),
        /* src-out */    make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kZero),
        /* dst-out */    make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kISA),
        /* src-atop */   make_simple_blendInfo(skgpu::BlendCoeff::kDA,   skgpu::BlendCoeff::kISA),
        /* dst-atop */   make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kSA),
        /* xor */        make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kISA),
        /* plus */       make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kOne),
        /* modulate */   make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kSC),
        /* screen */     make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISC)
};

} // anonymous namespace

//--------------------------------------------------------------------------------------------------
// ShaderInfo

ShaderInfo::ShaderInfo(UniquePaintParamsID id,
                       const ShaderCodeDictionary* dict,
                       const RuntimeEffectDictionary* rteDict,
                       const char* ssboIndex)
        : fRuntimeEffectDictionary(rteDict)
        , fSsboIndex(ssboIndex)
        , fSnippetRequirementFlags(SnippetRequirementFlags::kNone) {
    PaintParamsKey key = dict->lookup(id);
    SkASSERT(key.isValid()); // invalid keys should have been caught by invalid paint ID earlier

    fRootNodes = key.getRootNodes(dict, &fShaderNodeAlloc);
    // Aggregate snippet requirements across root nodes and look for fixed-function blend IDs in
    // the root to initialize the HW blend info.
    SkDEBUGCODE(bool fixedFuncBlendFound = false;)
    for (const ShaderNode* root : fRootNodes) {
        // TODO: This is brittle as it relies on PaintParams::toKey() putting the final fixed
        // function blend block at the root level. This can be improved with more structure to the
        // key creation.
        if (root->codeSnippetId() < kBuiltInCodeSnippetIDCount &&
            root->codeSnippetId() >= kFixedFunctionBlendModeIDOffset) {
            SkASSERT(root->numChildren() == 0);
            // This should occur at most once
            SkASSERT(!fixedFuncBlendFound);
            SkDEBUGCODE(fixedFuncBlendFound = true;)

            fBlendMode = static_cast<SkBlendMode>(root->codeSnippetId() -
                                                  kFixedFunctionBlendModeIDOffset);
            SkASSERT(static_cast<int>(fBlendMode) >= 0 &&
                     fBlendMode <= SkBlendMode::kLastCoeffMode);
            fBlendInfo = gBlendTable[static_cast<int>(fBlendMode)];
        } else {
            fSnippetRequirementFlags |= root->requiredFlags();
        }
    }
}

void append_color_output(std::string* mainBody,
                         BlendFormula::OutputType outputType,
                         const char* outColor,
                         const char* inColor) {
    switch (outputType) {
        case BlendFormula::kNone_OutputType:
            SkSL::String::appendf(mainBody, "%s = half4(0.0);", outColor);
            break;
        case BlendFormula::kCoverage_OutputType:
            SkSL::String::appendf(mainBody, "%s = outputCoverage;", outColor);
            break;
        case BlendFormula::kModulate_OutputType:
            SkSL::String::appendf(mainBody, "%s = %s * outputCoverage;", outColor, inColor);
            break;
        case BlendFormula::kSAModulate_OutputType:
            SkSL::String::appendf(mainBody, "%s = %s.a * outputCoverage;", outColor, inColor);
            break;
        case BlendFormula::kISAModulate_OutputType:
            SkSL::String::appendf(
                    mainBody, "%s = (1.0 - %s.a) * outputCoverage;", outColor, inColor);
            break;
        case BlendFormula::kISCModulate_OutputType:
            SkSL::String::appendf(
                    mainBody, "%s = (half4(1.0) - %s) * outputCoverage;", outColor, inColor);
            break;
        default:
            SkUNREACHABLE;
            break;
    }
}

// The current, incomplete, model for shader construction is:
//   - Static code snippets (which can have an arbitrary signature) live in the Graphite
//     pre-compiled modules, which are located at `src/sksl/sksl_graphite_frag.sksl` and
//     `src/sksl/sksl_graphite_frag_es2.sksl`.
//   - Glue code is generated in a `main` method which calls these static code snippets.
//     The glue code is responsible for:
//            1) gathering the correct (mangled) uniforms
//            2) passing the uniforms and any other parameters to the helper method
//   - The result of the final code snippet is then copied into "sk_FragColor".
//   Note: each entry's 'fStaticFunctionName' field is expected to match the name of a function
//   in the Graphite pre-compiled module.
std::string ShaderInfo::toSkSL(const Caps* caps,
                               const RenderStep* step,
                               bool useStorageBuffers,
                               int* numTexturesAndSamplersUsed,
                               int* numPaintUniforms,
                               int* renderStepUniformTotalBytes,
                               int* paintUniformsTotalBytes,
                               bool* hasGradientBuffer,
                               Swizzle writeSwizzle) {
    // If we're doing analytic coverage, we must also be doing shading.
    SkASSERT(step->coverage() == Coverage::kNone || step->performsShading());
    const bool hasStepUniforms = step->numUniforms() > 0 && step->coverage() != Coverage::kNone;
    const bool useStepStorageBuffer = useStorageBuffers && hasStepUniforms;
    const bool useShadingStorageBuffer = useStorageBuffers && step->performsShading();
    const bool useGradientStorageBuffer = useStorageBuffers && (fSnippetRequirementFlags
                                                    & SnippetRequirementFlags::kGradientBuffer);

    const bool defineLocalCoordsVarying = this->needsLocalCoords();
    std::string preamble = EmitVaryings(step,
                                        /*direction=*/"in",
                                        /*emitSsboIndicesVarying=*/useShadingStorageBuffer,
                                        defineLocalCoordsVarying);

    // The uniforms are mangled by having their index in 'fEntries' as a suffix (i.e., "_%d")
    // TODO: replace hard-coded bufferIDs with the backend's step and paint uniform-buffer indices.
    // TODO: The use of these indices is Metal-specific. We should replace these functions with
    // API-independent ones.
    const ResourceBindingRequirements& bindingReqs = caps->resourceBindingRequirements();
    if (hasStepUniforms) {
        if (useStepStorageBuffer) {
            preamble += EmitRenderStepStorageBuffer(/*bufferID=*/1, step->uniforms());
        } else {
            preamble += EmitRenderStepUniforms(/*bufferID=*/1,
                                               bindingReqs.fUniformBufferLayout,
                                               step->uniforms(),
                                               renderStepUniformTotalBytes);
        }
    }

    bool wrotePaintColor = false;
    if (useShadingStorageBuffer) {
        preamble += EmitPaintParamsStorageBuffer(/*bufferID=*/2,
                                                 fRootNodes,
                                                 numPaintUniforms,
                                                 &wrotePaintColor);
        // SkSL::String::appendf(&preamble, "uint %s;\n", this->ssboIndex());
    } else {
        preamble += EmitPaintParamsUniforms(/*bufferID=*/2,
                                            bindingReqs.fUniformBufferLayout,
                                            fRootNodes,
                                            numPaintUniforms,
                                            paintUniformsTotalBytes,
                                            &wrotePaintColor);
    }

    if (useGradientStorageBuffer) {
        SkASSERT(caps->storageBufferSupport());

        // In metal the vertex and instance buffer occupy slots 3 and 4 so we use slot 5 in that
        // case. In dawn and vulkan that is not the case so we can occupy slot 3, and those two
        // apis also do separate texture/sampler bindings.
        int binding = bindingReqs.fSeparateTextureAndSamplerBinding ? 3 : 5;
        SkSL::String::appendf(&preamble,
                              "layout (binding=%d) readonly buffer FSGradientBuffer {\n"
                              "    float fsGradientBuffer[];\n"
                              "};\n", binding);
        *hasGradientBuffer = true;
    }

    {
        int binding = 0;
        preamble += EmitTexturesAndSamplers(bindingReqs, fRootNodes, &binding);
        if (step->hasTextures()) {
            preamble += step->texturesAndSamplersSkSL(bindingReqs, &binding);
        }

        // Report back to the caller how many textures and samplers are used.
        if (numTexturesAndSamplersUsed) {
            *numTexturesAndSamplersUsed = binding;
        }
    }

    if (step->emitsPrimitiveColor()) {
        // TODO: Define this in the main body, and then pass it down into snippets like we do with
        // the local coordinates varying.
        preamble += "half4 primitiveColor;";
    }

    // Emit preamble declarations and helper functions required for snippets. In the default case
    // this adds functions that bind a node's specific mangled uniforms to the snippet's
    // implementation in the SkSL modules.
    emit_preambles(*this, fRootNodes, /*treeLabel=*/"", &preamble);

    std::string mainBody = "void main() {";

    if (useShadingStorageBuffer) {
    //     SkSL::String::appendf(&mainBody,
    //                           "%s = %s.y;\n",
    //                           this->ssboIndex(),
    //                           RenderStep::ssboIndicesVarying());
        SkSL::String::appendf(&mainBody,
                              "const FSUniformData& uniforms = fsUniformData[%s.y];",
                              RenderStep::ssboIndicesVarying());
    }

    if (step->emitsPrimitiveColor()) {
        mainBody += step->fragmentColorSkSL();
    }

    // While looping through root nodes to emit shader code, skip the clip shader node if it's found
    // and keep it to apply later during coverage calculation.
    const ShaderNode* clipShaderNode = nullptr;

    // Emit shader main body code, invoking each root node's expression, forwarding the previous
    // node's output to the next.
    ShaderSnippet::Args args = {/*src=*/"half4(0)",
                                /*dst=*/"half4(1)",
                                this->needsLocalCoords() ? "localCoordsVar" : "float2(0)"};
    for (const ShaderNode* node : fRootNodes) {
        if (node->codeSnippetId() == (int) BuiltInCodeSnippetID::kClipShader) {
            SkASSERT(!clipShaderNode);
            clipShaderNode = node;
            continue;
        }
        // This exclusion of the final Blend can be removed once we've resolved the final
        // blend parenting issue w/in the key
        if (node->codeSnippetId() >= kBuiltInCodeSnippetIDCount ||
            node->codeSnippetId() < kFixedFunctionBlendModeIDOffset) {
            args.fPriorStageOutput = emit_glue_code_for_entry(*this, node, args, &mainBody);
        }
    }

    if (writeSwizzle != Swizzle::RGBA()) {
        SkSL::String::appendf(&mainBody, "%s = %s.%s;", args.fPriorStageOutput.c_str(),
                                                        args.fPriorStageOutput.c_str(),
                                                        writeSwizzle.asString().c_str());
    }

    const char* outColor = args.fPriorStageOutput.c_str();
    const Coverage coverage = step->coverage();
    if (coverage != Coverage::kNone || clipShaderNode) {
        if (useStepStorageBuffer) {
            std::string ssboIndex = RenderStep::ssboIndicesVarying();
            ssboIndex += ".x";
            mainBody += EmitUniformsFromStorageBuffer("step", ssboIndex.c_str(), step->uniforms());
        }

        mainBody += "half4 outputCoverage = half4(1);";
        mainBody += step->fragmentCoverageSkSL();

        if (clipShaderNode) {
            std::string clipShaderOutput =
                    emit_glue_code_for_entry(*this, clipShaderNode, args, &mainBody);
            SkSL::String::appendf(&mainBody, "outputCoverage *= %s.a;", clipShaderOutput.c_str());
        }

        // TODO: Determine whether draw is opaque and pass that to GetBlendFormula.
        BlendFormula coverageBlendFormula =
                coverage == Coverage::kLCD
                        ? skgpu::GetLCDBlendFormula(fBlendMode)
                        : skgpu::GetBlendFormula(
                                  /*isOpaque=*/false, /*hasCoverage=*/true, fBlendMode);

        if (this->needsSurfaceColor()) {
            // If this draw uses a non-coherent dst read, we want to keep the existing dst color (or
            // whatever has been previously drawn) when there's no coverage. This helps for batching
            // text draws that need to read from a dst copy for blends. However, this only helps the
            // case where the outer bounding boxes of each letter overlap and not two actual parts
            // of the text.
            DstReadRequirement dstReadReq = caps->getDstReadRequirement();
            if (dstReadReq == DstReadRequirement::kTextureCopy ||
                dstReadReq == DstReadRequirement::kTextureSample) {
                // We don't think any shaders actually output negative coverage, but just as a
                // safety check for floating point precision errors, we compare with <= here. We
                // just check the RGB values of the coverage, since the alpha may not have been set
                // when using LCD. If we are using single-channel coverage, alpha will be equal to
                // RGB anyway.
                mainBody +=
                    "if (all(lessThanEqual(outputCoverage.rgb, half3(0)))) {"
                        "discard;"
                    "}";
            }

            // Use originally-specified BlendInfo and blend with dst manually.
            SkSL::String::appendf(
                    &mainBody,
                    "sk_FragColor = %s * outputCoverage + surfaceColor * (1.0 - outputCoverage);",
                    outColor);
            if (coverage == Coverage::kLCD) {
                SkSL::String::appendf(
                        &mainBody,
                        "half3 lerpRGB = mix(surfaceColor.aaa, %s.aaa, outputCoverage.rgb);"
                        "sk_FragColor.a = max(max(lerpRGB.r, lerpRGB.g), lerpRGB.b);",
                        outColor);
            }

        } else {
            fBlendInfo = {coverageBlendFormula.equation(),
                          coverageBlendFormula.srcCoeff(),
                          coverageBlendFormula.dstCoeff(),
                          SK_PMColor4fTRANSPARENT,
                          coverageBlendFormula.modifiesDst()};

            if (coverage == Coverage::kLCD) {
                mainBody += "outputCoverage.a = max(max(outputCoverage.r, "
                                                       "outputCoverage.g), "
                                                   "outputCoverage.b);";
            }
            append_color_output(
                    &mainBody, coverageBlendFormula.primaryOutput(), "sk_FragColor", outColor);
            if (coverageBlendFormula.hasSecondaryOutput()) {
                append_color_output(&mainBody,
                                    coverageBlendFormula.secondaryOutput(),
                                    "sk_SecondaryFragColor",
                                    outColor);
            }
        }

    } else {
        SkSL::String::appendf(&mainBody, "sk_FragColor = %s;", outColor);
    }
    mainBody += "}\n";

    return preamble + "\n" + mainBody;
}

//--------------------------------------------------------------------------------------------------
// ShaderCodeDictionary

UniquePaintParamsID ShaderCodeDictionary::findOrCreate(PaintParamsKeyBuilder* builder) {
    AutoLockBuilderAsKey keyView{builder};
    if (!keyView->isValid()) {
        return UniquePaintParamsID::InvalidID();
    }

    SkAutoSpinlock lock{fSpinLock};

    UniquePaintParamsID* existingEntry = fPaintKeyToID.find(*keyView);
    if (existingEntry) {
        SkASSERT(fIDToPaintKey[(*existingEntry).asUInt()] == *keyView);
        return *existingEntry;
    }

    // Detach from the builder and copy into the arena
    PaintParamsKey key = keyView->clone(&fArena);
    UniquePaintParamsID newID{SkTo<uint32_t>(fIDToPaintKey.size())};

    fPaintKeyToID.set(key, newID);
    fIDToPaintKey.push_back(key);
    return newID;
}

PaintParamsKey ShaderCodeDictionary::lookup(UniquePaintParamsID codeID) const {
    if (!codeID.isValid()) {
        return PaintParamsKey::Invalid();
    }

    SkAutoSpinlock lock{fSpinLock};
    SkASSERT(codeID.asUInt() < SkTo<uint32_t>(fIDToPaintKey.size()));
    return fIDToPaintKey[codeID.asUInt()];
}

SkSpan<const Uniform> ShaderCodeDictionary::getUniforms(BuiltInCodeSnippetID id) const {
    return fBuiltInCodeSnippets[(int) id].fUniforms;
}

const ShaderSnippet* ShaderCodeDictionary::getEntry(int codeSnippetID) const {
    if (codeSnippetID < 0) {
        return nullptr;
    }

    if (codeSnippetID < kBuiltInCodeSnippetIDCount) {
        return &fBuiltInCodeSnippets[codeSnippetID];
    }

    SkAutoSpinlock lock{fSpinLock};

    if (codeSnippetID >= kSkiaKnownRuntimeEffectsStart &&
        codeSnippetID < kSkiaKnownRuntimeEffectsStart + kStableKeyCnt) {
        int knownRTECodeSnippetID = codeSnippetID - kSkiaKnownRuntimeEffectsStart;

        // TODO(b/238759147): if the snippet hasn't been initialized, get the SkRuntimeEffect and
        // initialize it here
        SkASSERT(fKnownRuntimeEffectCodeSnippets[knownRTECodeSnippetID].fPreambleGenerator);
        return &fKnownRuntimeEffectCodeSnippets[knownRTECodeSnippetID];
    }

    // TODO(b/238759147): handle Android and chrome known runtime effects

    if (codeSnippetID >= kUnknownRuntimeEffectIDStart) {
        int userDefinedCodeSnippetID = codeSnippetID - kUnknownRuntimeEffectIDStart;
        if (userDefinedCodeSnippetID < SkTo<int>(fUserDefinedCodeSnippets.size())) {
            return &fUserDefinedCodeSnippets[userDefinedCodeSnippetID];
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
namespace {

//--------------------------------------------------------------------------------------------------

// TODO: These will be moved to be handled outside of the paint snippet stitching system, which will
// remove the need for the global variable.

// There should only ever be one node that produces the dst surface color, so these preambles
// use the same static name and program compilation will fail if the node tree has duplicates.
std::string GenerateDstReadSamplePreamble(const ShaderInfo& shaderInfo,
                                          const ShaderNode* node) {
    return SkSL::String::printf(
            "half4 surfaceColor;"  // we save off the original dstRead color to combine w/ coverage
            "half4 %s(float4 dstOffsetAndInvWH, sampler2D dstCopy) {"
                "surfaceColor = sample(dstCopy, (sk_FragCoord.xy - dstOffsetAndInvWH.xy) * dstOffsetAndInvWH.zw);"
                "return surfaceColor;"
            "}", node->entry()->fStaticFunctionName);
}

std::string GenerateDstReadFetchPreamble(const ShaderInfo& shaderInfo,
                                         const ShaderNode* node) {
    return SkSL::String::printf(
            "half4 surfaceColor;"  // we save off the original dstRead color to combine w/ coverage
            "half4 %s() {"
                "surfaceColor = sk_LastFragColor;"
                "return surfaceColor;"
            "}", node->entry()->fStaticFunctionName);
}

//--------------------------------------------------------------------------------------------------

static constexpr int kNumClipShaderChildren = 1;

std::string GenerateClipShaderExpression(const ShaderInfo& shaderInfo,
                                         const ShaderNode* node,
                                         const ShaderSnippet::Args& args) {
    SkASSERT(node->numChildren() == kNumClipShaderChildren);
    ShaderSnippet::Args clipArgs = args;
    clipArgs.fFragCoord = "sk_FragCoord.xy";
    return emit_expression_for_entry(shaderInfo, node->child(0), clipArgs);
}

std::string GenerateClipShaderPreamble(const ShaderInfo& shaderInfo, const ShaderNode* node) {
    // No preamble is used for clip shaders. The child shader is called directly with sk_FragCoord.
    return "";
}

//--------------------------------------------------------------------------------------------------

// These expression and preamble generators are only needed until we support passing unsized
// arrays into SkSL module functions (b/343510513).
// std::string GenerateGradientBufferExpression(const ShaderInfo& shaderInfo,
//                                              const ShaderNode* node,
//                                              const ShaderSnippet::Args& args) {
//     std::string helperFnName =
//             get_mangled_name(node->entry()->fStaticFunctionName, node->keyIndex());
//     return helperFnName + append_default_snippet_arguments(shaderInfo, node, args, {});
// }

std::string GenerateGradientBufferPreamble(const ShaderInfo& shaderInfo,
                                           const ShaderNode* node) {
    SkASSERT(node->codeSnippetId() == (int) BuiltInCodeSnippetID::kLinearGradientShaderBuffer ||
             node->codeSnippetId() == (int) BuiltInCodeSnippetID::kRadialGradientShaderBuffer ||
             node->codeSnippetId() == (int) BuiltInCodeSnippetID::kSweepGradientShaderBuffer ||
             node->codeSnippetId() == (int) BuiltInCodeSnippetID::kConicalGradientShaderBuffer);
    SkASSERT(node->numChildren() == 0);

    const char* gradArgs;
    const char* layoutFnCall;
    switch (node->codeSnippetId()) {
        case (int) BuiltInCodeSnippetID::kLinearGradientShaderBuffer:
            gradArgs = "float2 coords";
            layoutFnCall = "linear_grad_layout(coords)";
            break;
        case (int) BuiltInCodeSnippetID::kRadialGradientShaderBuffer:
            gradArgs = "float2 coords";
            layoutFnCall = "radial_grad_layout(coords)";
            break;
        case (int) BuiltInCodeSnippetID::kSweepGradientShaderBuffer:
            gradArgs = "float2 coords, float biasParam, float scaleParam";
            layoutFnCall = "sweep_grad_layout(biasParam, scaleParam, coords)";
            break;
        case (int) BuiltInCodeSnippetID::kConicalGradientShaderBuffer:
            gradArgs = "float2 coords, float radius0Param, float dRadiusParam, "
                            "float aParam, float invAParam";
            layoutFnCall = "conical_grad_layout(radius0Param, dRadiusParam, "
                                "aParam, invAParam, coords)";
            break;
    }

    // FIXME
    std::string helperFnName =
            get_mangled_name(node->entry()->fStaticFunctionName, node->keyIndex());
    return SkSL::String::printf(
                "half4 %s(%s, int numStops, int bufferOffset, int tileMode,"
                         "int colorSpace, int doUnpremul) {"
                    "float2 t = %s;"
                    "t = tile_grad(tileMode, t);"

                    // Colorize
                    "half4 color = half4(0);"
                    "if (t.y >= 0) {"
                        "if (t.x == 0) {"
                            // Start from 1 since bufferOffset + 0 is holding the
                            // color stop's offset value.
                            "color = half4(fsGradientBuffer[bufferOffset + 1],"
                                          "fsGradientBuffer[bufferOffset + 2],"
                                          "fsGradientBuffer[bufferOffset + 3],"
                                          "fsGradientBuffer[bufferOffset + 4]);"
                        "} else if (t.x == 1) {"
                            "int endBufferIdx = bufferOffset + numStops * 5;"
                            "color = half4(fsGradientBuffer[endBufferIdx - 4],"
                                          "fsGradientBuffer[endBufferIdx - 3],"
                                          "fsGradientBuffer[endBufferIdx - 2],"
                                          "fsGradientBuffer[endBufferIdx - 1]);"
                        "} else {"
                            // Binary search for the matching adjacent offsets
                            // running log(numStops).
                            "int low = 0;"
                            "int high = numStops - 1;"
                            "for (int i = 1; i < numStops; i += i) {"
                                "int mid = (low + high) / 2;"
                                "float offset = fsGradientBuffer[bufferOffset + mid * 5];"
                                "if (t.x < offset) {"
                                    "high = mid;"
                                "} else {"
                                    "low = mid;"
                                "}"
                            "}"
                            "int lowBufferIdx = bufferOffset + low * 5;"
                            "float lowOffset = fsGradientBuffer[lowBufferIdx];"
                            "half4 lowColor = half4(fsGradientBuffer[lowBufferIdx + 1],"
                                                   "fsGradientBuffer[lowBufferIdx + 2],"
                                                   "fsGradientBuffer[lowBufferIdx + 3],"
                                                   "fsGradientBuffer[lowBufferIdx + 4]);"

                            "int highBufferIdx = bufferOffset + high * 5;"
                            "float highOffset = fsGradientBuffer[highBufferIdx];"
                            "if (highOffset == lowOffset) {"
                                // If the t value falls exactly on a color stop, both lowOffset
                                // and highOffset will be exactly the same so we avoid having
                                // 0/0=NaN as our mix value.
                                "color = lowColor;"
                            "} else {"
                                "half4 highColor = half4(fsGradientBuffer[highBufferIdx + 1],"
                                                        "fsGradientBuffer[highBufferIdx + 2],"
                                                        "fsGradientBuffer[highBufferIdx + 3],"
                                                        "fsGradientBuffer[highBufferIdx + 4]);"

                                "color = half4(mix(lowColor,"
                                                  "highColor,"
                                                  "(t.x - lowOffset) /"
                                                  "(highOffset - lowOffset)));"
                            "}"
                        "}"
                    "}"

                    "return interpolated_to_rgb_unpremul(color,"
                                                        "colorSpace,"
                                                        "doUnpremul);"
                "}",
                helperFnName.c_str(),
                gradArgs,
                layoutFnCall);
}

//--------------------------------------------------------------------------------------------------

static constexpr int kNumLocalMatrixChildren = 1;

// Create a helper function that multiplies coordinates by a local matrix, invokes the child
// entry with those updated coordinates, and returns the result. This helper function meets the
// requirements for use with GenerateDefaultExpression, so there's no need to have a separate
// special GenerateLocalMatrixExpression.
std::string GenerateLocalMatrixPreamble(const ShaderInfo& shaderInfo,
                                        const ShaderNode* node) {
    SkASSERT(node->codeSnippetId() == (int) BuiltInCodeSnippetID::kLocalMatrixShader);
    SkASSERT(node->numChildren() == kNumLocalMatrixChildren);

    std::string code = emit_helper_declaration(shaderInfo, node);

    ShaderSnippet::Args localArgs = kDefaultArgs;
    if (node->child(0)->requiredFlags() & SnippetRequirementFlags::kLocalCoords) {
        std::string localMatrixUni =
                get_mangled_uniform_name(shaderInfo, node->entry()->fUniforms[0], node->keyIndex());
        localArgs.fFragCoord = SkSL::String::printf("(%s * %s.xy01).xy",
                                                    localMatrixUni.c_str(),
                                                    kDefaultArgs.fFragCoord.c_str());
    } // else this is a no-op

    std::string invokeChild = emit_expression_for_entry(shaderInfo, node->child(0), localArgs);
    SkSL::String::appendf(&code, "return %s; }", invokeChild.c_str());

    return code;
}

//--------------------------------------------------------------------------------------------------

static_assert(0 == static_cast<int>(SkTileMode::kClamp),  "ImageShader code depends on SkTileMode");
static_assert(1 == static_cast<int>(SkTileMode::kRepeat), "ImageShader code depends on SkTileMode");
static_assert(2 == static_cast<int>(SkTileMode::kMirror), "ImageShader code depends on SkTileMode");
static_assert(3 == static_cast<int>(SkTileMode::kDecal),  "ImageShader code depends on SkTileMode");

static_assert(0 == static_cast<int>(SkFilterMode::kNearest),
              "ImageShader code depends on SkFilterMode");
static_assert(1 == static_cast<int>(SkFilterMode::kLinear),
              "ImageShader code depends on SkFilterMode");

static_assert(0 == static_cast<int>(ReadSwizzle::kRGBA),
              "ImageShader code depends on ReadSwizzle");
static_assert(1 == static_cast<int>(ReadSwizzle::kRGB1),
              "ImageShader code depends on ReadSwizzle");
static_assert(2 == static_cast<int>(ReadSwizzle::kRRR1),
              "ImageShader code depends on ReadSwizzle");
static_assert(3 == static_cast<int>(ReadSwizzle::kBGRA),
              "ImageShader code depends on ReadSwizzle");
static_assert(4 == static_cast<int>(ReadSwizzle::k000R),
              "ImageShader code depends on ReadSwizzle");

//--------------------------------------------------------------------------------------------------

static constexpr int kNumCoordClampChildren = 1;

// Create a helper function that clamps the local coords to the subset, invokes the child
// entry with those updated coordinates, and returns the result. This helper function meets the
// requirements for use with GenerateDefaultExpression, so there's no need to have a separate
// special GenerateCoordClampExpression.
// TODO: this has a lot of overlap with GenerateLocalMatrixPreamble
std::string GenerateCoordClampPreamble(const ShaderInfo& shaderInfo,
                                       const ShaderNode* node) {
    SkASSERT(node->codeSnippetId() == (int) BuiltInCodeSnippetID::kCoordClampShader);
    SkASSERT(node->numChildren() == kNumCoordClampChildren);

    std::string code = emit_helper_declaration(shaderInfo, node);

    ShaderSnippet::Args clampArgs = kDefaultArgs;
    if (node->child(0)->requiredFlags() & SnippetRequirementFlags::kLocalCoords) {
        std::string subsetUni =
                get_mangled_uniform_name(shaderInfo, node->entry()->fUniforms[0], node->keyIndex());
        clampArgs.fFragCoord = SkSL::String::printf("clamp(%s, %s.LT, %s.RB)",
                kDefaultArgs.fFragCoord.c_str(), subsetUni.c_str(), subsetUni.c_str());
    } // else this is a no-op

    std::string invokeChild = emit_expression_for_entry(
            shaderInfo, node->child(0), clampArgs);
    SkSL::String::appendf(&code, "return %s; }", invokeChild.c_str());
    return code;
}

//--------------------------------------------------------------------------------------------------

std::string GenerateDitherExpression(const ShaderInfo& shaderInfo,
                                     const ShaderNode* node,
                                     const ShaderSnippet::Args& args) {
    SkASSERT(node->numChildren() == 0);
    ShaderSnippet::Args ditherArgs = args;
    ditherArgs.fFragCoord = "sk_FragCoord.xy";
    return invoke_default_expression(shaderInfo, node, ditherArgs);
}

//--------------------------------------------------------------------------------------------------

static constexpr int kNumBlendShaderChildren = 3;

std::string GenerateBlendShaderPreamble(const ShaderInfo& shaderInfo,
                                        const ShaderNode* node) {
    // Children are src, dst, and blender
    SkASSERT(node->numChildren() == kNumBlendShaderChildren);
    SkASSERT(node->codeSnippetId() == (int) BuiltInCodeSnippetID::kBlendShader);

    std::string code = emit_helper_declaration(shaderInfo, node);

    // Invoke first two children for src and dst colors
    ShaderSnippet::Args blendArgs = kDefaultArgs;
    blendArgs.fPriorStageOutput = emit_glue_code_for_entry(shaderInfo, node->child(0), kDefaultArgs, &code);
    blendArgs.fBlenderDstColor = emit_glue_code_for_entry(shaderInfo, node->child(1), kDefaultArgs, &code);

    // And then return the output of the third child
    std::string invokeBlend = emit_expression_for_entry(shaderInfo, node->child(2), blendArgs);

    SkSL::String::appendf(&code, "return %s; }", invokeBlend.c_str());
    return code;
}

//--------------------------------------------------------------------------------------------------
class GraphitePipelineCallbacks : public SkSL::PipelineStage::Callbacks {
public:
    GraphitePipelineCallbacks(const ShaderInfo& shaderInfo,
                              const ShaderNode* node,
                              std::string* preamble,
                              const SkRuntimeEffect* effect)
            : fShaderInfo(shaderInfo)
            , fNode(node)
            , fPreamble(preamble)
            , fEffect(effect) {}

    std::string declareUniform(const SkSL::VarDeclaration* decl) override {
        // this needs to match get_mangled_uniform_name except that it's based on SkSL::VarDeclaration
        // so we can't just reuse it.
        std::string result = get_mangled_name(std::string(decl->var()->name()), fNode->keyIndex());
        if (fShaderInfo.ssboIndex()) {
            // FIXME
            // result = EmitStorageBufferAccess("fs", fShaderInfo.ssboIndex(), result.c_str());
            result = SkSL::String::printf("uniforms.%s", result.c_str());
        }
        return result;
    }

    void defineFunction(const char* decl, const char* body, bool isMain) override {
        if (isMain) {
            SkSL::String::appendf(
                 fPreamble,
                 "%s %s }",
                 emit_helper_declaration(fShaderInfo, fNode).c_str(),
                 body);
        } else {
            // FIXME if this function references a "uniform" how does it get to the SSBO?
            // FIXME if we have embedded the effect multiple times, do we need to mangle it?
            SkSL::String::appendf(fPreamble, "%s {%s}\n", decl, body);
        }
    }

    void declareFunction(const char* decl) override {
        *fPreamble += std::string(decl);
    }

    void defineStruct(const char* definition) override {
        *fPreamble += std::string(definition);
    }

    void declareGlobal(const char* declaration) override {
        *fPreamble += std::string(declaration);
    }

    std::string sampleShader(int index, std::string coords) override {
        ShaderSnippet::Args sampleArgs = kDefaultArgs;
        sampleArgs.fFragCoord = coords;
        return emit_expression_for_entry(fShaderInfo, fNode->child(index), sampleArgs);
    }

    std::string sampleColorFilter(int index, std::string color) override {
        ShaderSnippet::Args filterArgs = kDefaultArgs;
        filterArgs.fPriorStageOutput = color;
        return emit_expression_for_entry(fShaderInfo, fNode->child(index), filterArgs);
    }

    std::string sampleBlender(int index, std::string src, std::string dst) override {
        ShaderSnippet::Args blendArgs = kDefaultArgs;
        blendArgs.fPriorStageOutput = src;
        blendArgs.fBlenderDstColor = dst;
        return emit_expression_for_entry(fShaderInfo, fNode->child(index), blendArgs);
    }

    std::string toLinearSrgb(std::string color) override {
        if (!SkRuntimeEffectPriv::UsesColorTransform(fEffect)) {
            return color;
        }

        color = SkSL::String::printf("(%s).rgb1", color.c_str());
        std::string helper = get_mangled_name("toLinearSRGB", fNode->keyIndex());
        std::string xformedColor = SkSL::String::printf("%s(%s)",
                                    helper.c_str(),
                                    color.c_str());
        return SkSL::String::printf("(%s).rgb", xformedColor.c_str());
    }


    std::string fromLinearSrgb(std::string color) override {
        if (!SkRuntimeEffectPriv::UsesColorTransform(fEffect)) {
            return color;
        }

        color = SkSL::String::printf("(%s).rgb1", color.c_str());
        std::string helper = get_mangled_name("fromLinearSRGB", fNode->keyIndex());
        std::string xformedColor = SkSL::String::printf("%s(%s)",
                                                        helper.c_str(),
                                                        color.c_str());
        return SkSL::String::printf("(%s).rgb", xformedColor.c_str());
    }

    std::string getMangledName(const char* name) override {
        return get_mangled_name(name, fNode->keyIndex());
    }

private:
    const ShaderInfo& fShaderInfo;
    const ShaderNode* fNode;
    std::string* fPreamble;
    const SkRuntimeEffect* fEffect;
};

std::string GenerateRuntimeShaderPreamble(const ShaderInfo& shaderInfo,
                                          const ShaderNode* node) {
    // Find this runtime effect in the runtime-effect dictionary.
    SkASSERT(node->codeSnippetId() >= kBuiltInCodeSnippetIDCount);
    const SkRuntimeEffect* effect;
    if (node->codeSnippetId() < kSkiaKnownRuntimeEffectsStart + kStableKeyCnt) {
        effect = GetKnownRuntimeEffect(static_cast<StableKey>(node->codeSnippetId()));
    } else {
        SkASSERT(node->codeSnippetId() >= kUnknownRuntimeEffectIDStart);
        effect = shaderInfo.runtimeEffectDictionary()->find(node->codeSnippetId());
    }
    SkASSERT(effect);

    const SkSL::Program& program = SkRuntimeEffectPriv::Program(*effect);

    std::string preamble;
    if (SkRuntimeEffectPriv::UsesColorTransform(effect)) {
    // FIXME: would be nice to de-mangle these somehow since the color transforms are static for
    // the given dst color spce
        SkSL::String::appendf(
                &preamble,
                "half4 %s(half4 inColor) {"
                    "return sk_color_space_transform(inColor, %s, %s, %s, %s, %s);"
                "}",
                get_mangled_name("toLinearSRGB", node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[0],
                                         node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[1],
                                         node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[2],
                                         node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[3],
                                         node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[4],
                                         node->keyIndex()).c_str());
        SkSL::String::appendf(
                &preamble,
                "half4 %s(half4 inColor) {"
                    "return sk_color_space_transform(inColor, %s, %s, %s, %s, %s);"
                "}",
                get_mangled_name("fromLinearSRGB", node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[5],
                                         node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[6],
                                         node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[7],
                                         node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[8],
                                         node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[9],
                                         node->keyIndex()).c_str());
    }

    GraphitePipelineCallbacks callbacks{shaderInfo, node, &preamble, effect};
    SkSL::PipelineStage::ConvertProgram(program, "coords", "inColor", "destColor", &callbacks);
    return preamble;
}

// std::string GenerateRuntimeShaderExpression(const ShaderInfo& shaderInfo,
//                                             const ShaderNode* node,
//                                             const ShaderSnippet::Args& args) {
//     return SkSL::String::printf(
//             "%s(%.*s, %.*s, %.*s)",
//             get_mangled_name(node->entry()->fName, node->keyIndex()).c_str(),
//             (int)args.fPriorStageOutput.size(), args.fPriorStageOutput.data(),
//             (int)args.fBlenderDstColor.size(),  args.fBlenderDstColor.data(),
//             (int)args.fFragCoord.size(),        args.fFragCoord.data());
// }

//--------------------------------------------------------------------------------------------------

static constexpr int kNumComposeChildren = 2;

// Compose two children, assuming the first child is the innermost.
std::string GenerateNestedChildrenPreamble(const ShaderInfo& shaderInfo,
                                           const ShaderNode* node) {
    // Children are inner, outer
    SkASSERT(node->numChildren() == kNumComposeChildren);
    SkASSERT(node->codeSnippetId() == (int) BuiltInCodeSnippetID::kCompose);

    std::string code = emit_helper_declaration(shaderInfo, node);

    // Invoke the inner with default args, and store its output as the input color to the outer
    ShaderSnippet::Args composeArgs = kDefaultArgs;
    composeArgs.fPriorStageOutput = emit_glue_code_for_entry(shaderInfo, node->child(0), kDefaultArgs, &code);

    // And then return the outer's result
    std::string outerResult = emit_expression_for_entry(shaderInfo, node->child(1), composeArgs);

    SkSL::String::appendf(&code, "return %s; }", outerResult.c_str());
    return code;
}

//--------------------------------------------------------------------------------------------------

static_assert(0 == static_cast<int>(skcms_TFType_Invalid),
              "ColorSpaceTransform code depends on skcms_TFType");
static_assert(1 == static_cast<int>(skcms_TFType_sRGBish),
              "ColorSpaceTransform code depends on skcms_TFType");
static_assert(2 == static_cast<int>(skcms_TFType_PQish),
              "ColorSpaceTransform code depends on skcms_TFType");
static_assert(3 == static_cast<int>(skcms_TFType_HLGish),
              "ColorSpaceTransform code depends on skcms_TFType");
static_assert(4 == static_cast<int>(skcms_TFType_HLGinvish),
              "ColorSpaceTransform code depends on skcms_TFType");

// TODO: We can meaningfully check these when we can use C++20 features.
// static_assert(0x1 == SkColorSpaceXformSteps::Flags{.unpremul = true}.mask(),
//               "ColorSpaceTransform code depends on SkColorSpaceXformSteps::Flags");
// static_assert(0x2 == SkColorSpaceXformSteps::Flags{.linearize = true}.mask(),
//               "ColorSpaceTransform code depends on SkColorSpaceXformSteps::Flags");
// static_assert(0x4 == SkColorSpaceXformSteps::Flags{.gamut_transform = true}.mask(),
//               "ColorSpaceTransform code depends on SkColorSpaceXformSteps::Flags");
// static_assert(0x8 == SkColorSpaceXformSteps::Flags{.encode = true}.mask(),
//               "ColorSpaceTransform code depends on SkColorSpaceXformSteps::Flags");
// static_assert(0x10 == SkColorSpaceXformSteps::Flags{.premul = true}.mask(),
//               "ColorSpaceTransform code depends on SkColorSpaceXformSteps::Flags");

//--------------------------------------------------------------------------------------------------

std::string GeneratePrimitiveColorExpression(const ShaderInfo&,
                                             const ShaderNode* node,
                                             const ShaderSnippet::Args&) {
    return "primitiveColor";
}

//--------------------------------------------------------------------------------------------------

} // anonymous namespace

#if defined(SK_DEBUG)
bool ShaderCodeDictionary::isValidID(int snippetID) const {
    if (snippetID < 0) {
        return false;
    }

    if (snippetID < kBuiltInCodeSnippetIDCount) {
        return true;
    }
    if (snippetID >= kSkiaKnownRuntimeEffectsStart && snippetID < kSkiaKnownRuntimeEffectsEnd) {
        return snippetID < kSkiaKnownRuntimeEffectsStart + kStableKeyCnt;
    }

    SkAutoSpinlock lock{fSpinLock};

    if (snippetID >= kUnknownRuntimeEffectIDStart) {
        int userDefinedCodeSnippetID = snippetID - kUnknownRuntimeEffectIDStart;
        return userDefinedCodeSnippetID < SkTo<int>(fUserDefinedCodeSnippets.size());
    }

    return false;
}

void ShaderCodeDictionary::dump(UniquePaintParamsID id) const {
    this->lookup(id).dump(this, id);
}
#endif

#if defined(GRAPHITE_TEST_UTILS)

int ShaderCodeDictionary::addRuntimeEffectSnippet(const char* functionName) {
    SkAutoSpinlock lock{fSpinLock};

    // fUserDefinedCodeSnippets.push_back(
    //         std::make_unique<ShaderSnippet>("UserDefined",
    //                                         SkSpan<const Uniform>(),            // no uniforms
    //                                         SnippetRequirementFlags::kNone,
    //                                         SkSpan<const TextureAndSampler>(),  // no samplers
    //                                         functionName,
    //                                         GenerateDefaultExpression,
    //                                         GenerateDefaultPreamble,
    //                                         kNoChildren));

    return kUnknownRuntimeEffectIDStart + fUserDefinedCodeSnippets.size() - 1;
}

#endif // GRAPHITE_TEST_UTILS

static SkSLType uniform_type_to_sksl_type(const SkRuntimeEffect::Uniform& u) {
    using Type = SkRuntimeEffect::Uniform::Type;
    if (u.flags & SkRuntimeEffect::Uniform::kHalfPrecision_Flag) {
        switch (u.type) {
            case Type::kFloat:    return SkSLType::kHalf;
            case Type::kFloat2:   return SkSLType::kHalf2;
            case Type::kFloat3:   return SkSLType::kHalf3;
            case Type::kFloat4:   return SkSLType::kHalf4;
            case Type::kFloat2x2: return SkSLType::kHalf2x2;
            case Type::kFloat3x3: return SkSLType::kHalf3x3;
            case Type::kFloat4x4: return SkSLType::kHalf4x4;
            // NOTE: shorts cannot be uniforms, so we shouldn't ever get here.
            // Defensively return the full precision integer type.
            case Type::kInt:      SkDEBUGFAIL("unsupported uniform type"); return SkSLType::kInt;
            case Type::kInt2:     SkDEBUGFAIL("unsupported uniform type"); return SkSLType::kInt2;
            case Type::kInt3:     SkDEBUGFAIL("unsupported uniform type"); return SkSLType::kInt3;
            case Type::kInt4:     SkDEBUGFAIL("unsupported uniform type"); return SkSLType::kInt4;
        }
    } else {
        switch (u.type) {
            case Type::kFloat:    return SkSLType::kFloat;
            case Type::kFloat2:   return SkSLType::kFloat2;
            case Type::kFloat3:   return SkSLType::kFloat3;
            case Type::kFloat4:   return SkSLType::kFloat4;
            case Type::kFloat2x2: return SkSLType::kFloat2x2;
            case Type::kFloat3x3: return SkSLType::kFloat3x3;
            case Type::kFloat4x4: return SkSLType::kFloat4x4;
            case Type::kInt:      return SkSLType::kInt;
            case Type::kInt2:     return SkSLType::kInt2;
            case Type::kInt3:     return SkSLType::kInt3;
            case Type::kInt4:     return SkSLType::kInt4;
        }
    }
    SkUNREACHABLE;
}

const char* ShaderCodeDictionary::addTextToArena(std::string_view text) {
    char* textInArena = fArena.makeArrayDefault<char>(text.size() + 1);
    memcpy(textInArena, text.data(), text.size());
    textInArena[text.size()] = '\0';
    return textInArena;
}

SkSpan<const Uniform> ShaderCodeDictionary::convertUniforms(const SkRuntimeEffect* effect) {
    using rteUniform = SkRuntimeEffect::Uniform;
    SkSpan<const rteUniform> uniforms = effect->uniforms();

    int numBaseUniforms = uniforms.size();
    int xtraUniforms = 0;
    if (SkRuntimeEffectPriv::UsesColorTransform(effect)) {
        xtraUniforms += std::size(kRuntimeEffectColorSpaceTransformUniforms);
    }

    // Convert the SkRuntimeEffect::Uniform array into its Uniform equivalent.
    int numUniforms = numBaseUniforms + xtraUniforms;
    Uniform* uniformArray = fArena.makeInitializedArray<Uniform>(numUniforms, [&](int index) {
        if (index >= numBaseUniforms) {
            return kRuntimeEffectColorSpaceTransformUniforms[index - numBaseUniforms];
        }

        const rteUniform* u;
        u = &uniforms[index];

        // The existing uniform names live in the passed-in SkRuntimeEffect and may eventually
        // disappear. Copy them into fArena. (It's safe to do this within makeInitializedArray; the
        // entire array is allocated in one big slab before any initialization calls are done.)
        const char* name = this->addTextToArena(u->name);

        // Add one Uniform to our array.
        SkSLType type = uniform_type_to_sksl_type(*u);
        return (u->flags & rteUniform::kArray_Flag) ? Uniform(name, type, u->count)
                                                    : Uniform(name, type);
    });

    return SkSpan<const Uniform>(uniformArray, numUniforms);
}

ShaderSnippet ShaderCodeDictionary::convertRuntimeEffect(const SkRuntimeEffect* effect,
                                                         const char* name) {
    SkEnumBitMask<SnippetRequirementFlags> snippetFlags = SnippetRequirementFlags::kNone;
    if (effect->allowShader()) {
        // Only add the local coord usage if the runtime effect actually uses it
        if (effect->usesSampleCoords()) {
            snippetFlags |= SnippetRequirementFlags::kLocalCoords;
        }
    } else if (effect->allowColorFilter()) {
        snippetFlags |= SnippetRequirementFlags::kPriorStageOutput;
    } else if (effect->allowBlender()) {
        snippetFlags |= SnippetRequirementFlags::kPriorStageOutput; // src
        snippetFlags |= SnippetRequirementFlags::kBlenderDstColor;  // dst
    }


    STArray<2, SkEnumBitMask<SnippetRequirementFlags>> childMasks((int) effect->children().size());
    for (size_t i = 0; i < effect->children().size(); ++i) {
        // Invoking children always requires the arguments to be provided explicitly within the
        // runtime sksl, so there's no need to propagate those requirements (if the expressions are
        // derived from the effect's inputs, `snippetFlags` will include them as needed).
        SkEnumBitMask<SnippetRequirementFlags> childMask =
                SnippetRequirementFlags::kLocalCoords |
                SnippetRequirementFlags::kPriorStageOutput |
                SnippetRequirementFlags::kBlenderDstColor;
        childMasks.push_back(childMask);
    }

    return ShaderSnippet(name,
                        /*function=*/name,
                        snippetFlags,
                        this->convertUniforms(effect),
                        /*textures=*/{},
                        nullptr,
                        GenerateRuntimeShaderPreamble,
                        childMasks);
}

int ShaderCodeDictionary::findOrCreateRuntimeEffectSnippet(const SkRuntimeEffect* effect) {
    SkAutoSpinlock lock{fSpinLock};

    if (int stableKey = SkRuntimeEffectPriv::StableKey(*effect)) {
        SkASSERT(stableKey >= kSkiaKnownRuntimeEffectsStart &&
                 stableKey < kSkiaKnownRuntimeEffectsStart + kStableKeyCnt);

        int index = stableKey - kSkiaKnownRuntimeEffectsStart;

        if (!fKnownRuntimeEffectCodeSnippets[index].fPreambleGenerator) {
            const char* name = get_known_rte_name(static_cast<StableKey>(stableKey));
            fKnownRuntimeEffectCodeSnippets[index] =
                    this->convertRuntimeEffect(effect, name);
        }

        return stableKey;
    }

    // Use the combination of {SkSL program hash, uniform size} as our key.
    // In the unfortunate event of a hash collision, at least we'll have the right amount of
    // uniform data available.
    RuntimeEffectKey key;
    key.fHash = SkRuntimeEffectPriv::Hash(*effect);
    key.fUniformSize = effect->uniformSize();

    int32_t* existingCodeSnippetID = fRuntimeEffectMap.find(key);
    if (existingCodeSnippetID) {
        return *existingCodeSnippetID;
    }

    // TODO: the memory for user-defined entries could go in the dictionary's arena but that
    // would have to be a thread safe allocation since the arena also stores entries for
    // 'fHash' and 'fEntryVector'
    fUserDefinedCodeSnippets.push_back(
            this->convertRuntimeEffect(effect, "RuntimeEffect"));

    int newCodeSnippetID = kUnknownRuntimeEffectIDStart + fUserDefinedCodeSnippets.size() - 1;

    fRuntimeEffectMap.set(key, newCodeSnippetID);
    return newCodeSnippetID;
}

ShaderCodeDictionary::ShaderCodeDictionary() {
    // The 0th index is reserved as invalid
    fIDToPaintKey.push_back(PaintParamsKey::Invalid());

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kError] = {
            /*name=*/"Error",
            /*function=*/"sk_error",
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPriorOutput] = {
            /*name=*/"PassthroughShader",
            /*function=*/"sk_passthrough",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kSolidColorShader] = {
            /*name=*/"SolidColor",
            /*function=*/"sk_solid_shader",
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{ { "color", SkSLType::kFloat4 } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kRGBPaintColor] = {
            /*name=*/"RGBPaintColor",
            /*function=*/"sk_rgb_opaque",
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{ Uniform::PaintColor() }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kAlphaOnlyPaintColor] = {
            /*name=*/"AlphaOnlyPaintColor",
            /*function=*/"sk_alpha_only",
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{ Uniform::PaintColor() }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLinearGradientShader4] = {
            /*name=*/"LinearGradient4",
            /*function=*/"sk_linear_grad_4_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "colors",      SkSLType::kFloat4, 4 },
                           { "offsets",     SkSLType::kFloat4 },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } },
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLinearGradientShader8] = {
            /*name=*/"LinearGradient8",
            /*function=*/"sk_linear_grad_8_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "colors",      SkSLType::kFloat4, 8 },
                           { "offsets",     SkSLType::kFloat4, 2 },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLinearGradientShaderTexture] = {
            /*name=*/"LinearGradientTexture",
            /*function=*/"sk_linear_grad_tex_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "numStops",    SkSLType::kInt },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } },
            /*textures=*/{"colorAndOffsetSampler"}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLinearGradientShaderBuffer] = {
            /*name=*/"LinearGradientBuffer",
            /*function=*/"sk_linear_grad_buf_shader",
            SnippetRequirementFlags::kLocalCoords | SnippetRequirementFlags::kGradientBuffer,
            /*uniforms=*/{ { "numStops",     SkSLType::kInt },
                           { "bufferOffset", SkSLType::kInt },
                           { "tilemode",     SkSLType::kInt },
                           { "colorSpace",   SkSLType::kInt },
                           { "doUnPremul",   SkSLType::kInt } },
            /*textures=*/{},
            /*expressionGenerator=*/nullptr,
            GenerateGradientBufferPreamble,
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kRadialGradientShader4] = {
            /*name=*/"RadialGradient4",
            /*function=*/ "sk_radial_grad_4_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "colors",      SkSLType::kFloat4, 4 },
                           { "offsets",     SkSLType::kFloat4 },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kRadialGradientShader8] = {
            /*name=*/"RadialGradient8",
            /*function=*/"sk_radial_grad_8_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "colors",      SkSLType::kFloat4, 8 },
                           { "offsets",     SkSLType::kFloat4, 2 },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kRadialGradientShaderTexture] = {
            /*name=*/"RadialGradientTexture",
            /*function=*/"sk_radial_grad_tex_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "numStops",    SkSLType::kInt },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } },
            /*textures=*/{"colorAndOffsetSampler"}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kRadialGradientShaderBuffer] = {
            /*name=*/"RadialGradientBuffer",
            /*function=*/"sk_radial_grad_buf_shader",
            SnippetRequirementFlags::kLocalCoords | SnippetRequirementFlags::kGradientBuffer,
            /*uniforms=*/{ { "numStops",     SkSLType::kInt },
                           { "bufferOffset", SkSLType::kInt },
                           { "tilemode",     SkSLType::kInt },
                           { "colorSpace",   SkSLType::kInt },
                           { "doUnPremul",   SkSLType::kInt } },
            /*textures=*/{},
            /*expressionGenerator=*/nullptr,
            GenerateGradientBufferPreamble,
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kSweepGradientShader4] = {
            /*name=*/"SweepGradient4",
            /*function=*/"sk_sweep_grad_4_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "colors",      SkSLType::kFloat4, 4 },
                           { "offsets",     SkSLType::kFloat4 },
                           { "bias",        SkSLType::kFloat },
                           { "scale",       SkSLType::kFloat },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kSweepGradientShader8] = {
            /*name=*/"SweepGradient8",
            /*function=*/"sk_sweep_grad_8_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "colors",      SkSLType::kFloat4, 8 },
                           { "offsets",     SkSLType::kFloat4, 2 },
                           { "bias",        SkSLType::kFloat },
                           { "scale",       SkSLType::kFloat },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kSweepGradientShaderTexture] = {
            /*name=*/"SweepGradientTexture",
            /*function=*/"sk_sweep_grad_tex_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "bias",        SkSLType::kFloat },
                            { "scale",      SkSLType::kFloat },
                            { "numStops",   SkSLType::kInt },
                            { "tilemode",   SkSLType::kInt },
                            { "colorSpace", SkSLType::kInt },
                            { "doUnPremul", SkSLType::kInt } },
            /*textures=*/{"colorAndOffsetSampler"}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kSweepGradientShaderBuffer] = {
            /*name=*/"SweepGradientBuffer",
            /*function=*/"sk_sweep_grad_buf_shader",
            SnippetRequirementFlags::kLocalCoords | SnippetRequirementFlags::kGradientBuffer,
            /*uniforms=*/{ { "bias",         SkSLType::kFloat },
                           { "scale",        SkSLType::kFloat },
                           { "numStops",     SkSLType::kInt },
                           { "bufferOffset", SkSLType::kInt },
                           { "tilemode",     SkSLType::kInt },
                           { "colorSpace",   SkSLType::kInt },
                           { "doUnPremul",   SkSLType::kInt } },
            /*textures=*/{},
            /*expressionGenerator=*/nullptr,
            GenerateGradientBufferPreamble,
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kConicalGradientShader4] = {
            /*name=*/"ConicalGradient4",
            /*function=*/"sk_conical_grad_4_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "colors",      SkSLType::kFloat4, 4 },
                           { "offsets",     SkSLType::kFloat4 },
                           { "radius0",     SkSLType::kFloat },
                           { "dRadius",     SkSLType::kFloat },
                           { "a",           SkSLType::kFloat },
                           { "invA",        SkSLType::kFloat },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kConicalGradientShader4] = {
            /*name=*/"ConicalGradient8",
            /*function=*/"sk_conical_grad_8_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "colors",      SkSLType::kFloat4, 8 },
                           { "offsets",     SkSLType::kFloat4, 2 },
                           { "radius0",     SkSLType::kFloat },
                           { "dRadius",     SkSLType::kFloat },
                           { "a",           SkSLType::kFloat },
                           { "invA",        SkSLType::kFloat },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kConicalGradientShaderTexture] = {
            /*name=*/"ConicalGradientTexture",
            /*function=*/"sk_conical_grad_tex_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "radius0",     SkSLType::kFloat },
                           { "dRadius",     SkSLType::kFloat },
                           { "a",           SkSLType::kFloat },
                           { "invA",        SkSLType::kFloat },
                           { "numStops",    SkSLType::kInt },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } },
            /*textures=*/{"colorAndOffsetSampler"}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kConicalGradientShaderBuffer] = {
            /*name=*/"ConicalGradientBuffer",
            /*function=*/"sk_conical_grad_buf_shader",
            SnippetRequirementFlags::kLocalCoords | SnippetRequirementFlags::kGradientBuffer,
            /*uniforms=*/{ { "radius0",      SkSLType::kFloat },
                           { "dRadius",      SkSLType::kFloat },
                           { "a",            SkSLType::kFloat },
                           { "invA",         SkSLType::kFloat },
                           { "numStops",     SkSLType::kInt },
                           { "bufferOffset", SkSLType::kInt },
                           { "tilemode",     SkSLType::kInt },
                           { "colorSpace",   SkSLType::kInt },
                           { "doUnPremul",   SkSLType::kInt } },
            /*textures=*/{},
            /*expressionGenerator=*/nullptr,
            GenerateGradientBufferPreamble,
    };

    // This snippet operates on local coords if the child requires local coords (hence why it does
    // not mask off the child's local coord requirement), but does nothing if the child does not
    // actually use coordinates.
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLocalMatrixShader] = {
            /*name=*/"LocalMatrixShader",
            /*function=*/"$local_matrix",
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{ { "localMatrix", SkSLType::kFloat4x4 } },
            /*textures=*/{},
            /*expressionGenerator=*/nullptr,
            GenerateLocalMatrixPreamble,
            /*childMasks=*/{SnippetRequirementFlags::kNone}
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kImageShader] = {
            /*name=*/"ImageShader",
            /*function=*/"sk_image_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "invImgSize",            SkSLType::kFloat2 },
                           { "subset",                SkSLType::kFloat4 },
                           { "tilemodeX",             SkSLType::kInt },
                           { "tilemodeY",             SkSLType::kInt },
                           { "filterMode",            SkSLType::kInt },
                           // The next 5 uniforms are for the color space transformation
                           { "csXformFlags",          SkSLType::kInt },
                           { "csXformSrcKind",        SkSLType::kInt },
                           { "csXformGamutTransform", SkSLType::kHalf3x3 },
                           { "csXformDstKind",        SkSLType::kInt },
                           { "csXformCoeffs",         SkSLType::kHalf4x4 } },
            /*textures=*/{"image"}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kCubicImageShader] = {
            /*name=*/"CubicImageShader",
            /*function=*/"sk_cubic_image_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "invImgSize",            SkSLType::kFloat2 },
                           { "subset",                SkSLType::kFloat4 },
                           { "tilemodeX",             SkSLType::kInt },
                           { "tilemodeY",             SkSLType::kInt },
                           { "cubicCoeffs",           SkSLType::kHalf4x4 },
                           // The next 5 uniforms are for the color space transformation
                           { "csXformFlags",          SkSLType::kInt },
                           { "csXformSrcKind",        SkSLType::kInt },
                           { "csXformGamutTransform", SkSLType::kHalf3x3 },
                           { "csXformDstKind",        SkSLType::kInt },
                           { "csXformCoeffs",         SkSLType::kHalf4x4 } },
            /*textures=*/{"image"}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kHWImageShader] = {
            /*name=*/"HardwareImageShader",
            /*function=*/"sk_hw_image_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "invImgSize",            SkSLType::kFloat2 },
                           // The next 5 uniforms are for the color space transformation
                           { "csXformFlags",          SkSLType::kInt },
                           { "csXformSrcKind",        SkSLType::kInt },
                           { "csXformGamutTransform", SkSLType::kHalf3x3 },
                           { "csXformDstKind",        SkSLType::kInt },
                           { "csXformCoeffs",         SkSLType::kHalf4x4 } },
            /*textures=*/{"image"}
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kYUVImageShader] = {
            /*name=*/"YUVImageShader",
            /*function=*/"sk_yuv_image_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "invImgSizeY",         SkSLType::kFloat2 },
                           { "invImgSizeUV",        SkSLType::kFloat2 },  // Relative to Y's texels
                           { "subset",              SkSLType::kFloat4 },
                           { "linearFilterUVInset", SkSLType::kFloat2 },
                           { "tilemodeX",           SkSLType::kInt },
                           { "tilemodeY",           SkSLType::kInt },
                           { "filterModeY",         SkSLType::kInt },
                           { "filterModeUV",        SkSLType::kInt },
                           { "channelSelectY",      SkSLType::kHalf4 },
                           { "channelSelectU",      SkSLType::kHalf4 },
                           { "channelSelectV",      SkSLType::kHalf4 },
                           { "channelSelectA",      SkSLType::kHalf4 },
                           { "yuvToRGBMatrix",      SkSLType::kHalf3x3 },
                           { "yuvToRGBTranslate",   SkSLType::kFloat3 } },
            /*textures=*/ {{ "samplerY" },
                           { "samplerU" },
                           { "samplerV" },
                           { "samplerA" }}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kCubicYUVImageShader] = {
            /*name=*/"CubicYUVImageShader",
            /*function=*/"sk_cubic_yuv_image_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "invImgSizeY",       SkSLType::kFloat2 },
                           { "invImgSizeUV",      SkSLType::kFloat2 },  // Relative to Y's texels
                           { "subset",            SkSLType::kFloat4 },
                           { "tilemodeX",         SkSLType::kInt },
                           { "tilemodeY",         SkSLType::kInt },
                           { "cubicCoeffs",       SkSLType::kHalf4x4 },
                           { "channelSelectY",    SkSLType::kHalf4 },
                           { "channelSelectU",    SkSLType::kHalf4 },
                           { "channelSelectV",    SkSLType::kHalf4 },
                           { "channelSelectA",    SkSLType::kHalf4 },
                           { "yuvToRGBMatrix",    SkSLType::kHalf3x3 },
                           { "yuvToRGBTranslate", SkSLType::kFloat3 } },
            /*textures=*/ {{ "samplerY" },
                           { "samplerU" },
                           { "samplerV" },
                           { "samplerA" }}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kHWYUVImageShader] = {
            /*name=*/"HWYUVImageShader",
            /*function=*/"sk_hw_yuv_image_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "invImgSizeY",           SkSLType::kFloat2 },
                           { "invImgSizeUV",          SkSLType::kFloat2 }, // Relative to Y's texels
                           { "channelSelectY",        SkSLType::kHalf4 },
                           { "channelSelectU",        SkSLType::kHalf4 },
                           { "channelSelectV",        SkSLType::kHalf4 },
                           { "channelSelectA",        SkSLType::kHalf4 },
                           { "yuvToRGBMatrix",        SkSLType::kHalf3x3 },
                           { "yuvToRGBTranslate",     SkSLType::kFloat3 } },
            /*textures=*/ {{ "samplerY" },
                           { "samplerU" },
                           { "samplerV" },
                           { "samplerA" }}
    };

    // Like the local matrix shader, this is a no-op if the child doesn't need coords
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kCoordClampShader] = {
            /*name=*/"CoordClampShader",
            /*function=*/"$coord_clamp",
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{ { "subset", SkSLType::kFloat4 } },
            /*textures=*/{},
            /*expressionGenerator=*/nullptr,
            GenerateCoordClampPreamble,
            /*childMasks=*/{SnippetRequirementFlags::kNone}
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kDitherShader] = {
            /*name=*/"DitherShader",
            /*function=*/"sk_dither_shader",
            SnippetRequirementFlags::kPriorStageOutput | SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "range", SkSLType::kHalf } },
            /*textures=*/{ { "ditherLUT" } },
            GenerateDitherExpression
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPerlinNoiseShader] = {
            /*name=*/"PerlinNoiseShader",
            /*function=*/"sk_perlin_noise_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "baseFrequency", SkSLType::kFloat2 },
                           { "stitchData",    SkSLType::kFloat2 },
                           { "noiseType",     SkSLType::kInt },
                           { "numOctaves",    SkSLType::kInt },
                           { "stitching",     SkSLType::kInt } },
            /*textures=*/{ { "permutationsSampler" },
                           { "noiseSampler" } }
    };

    // SkColorFilter snippets
    // TODO: investigate the implications of having separate hlsa and rgba matrix colorfilters. It
    // may be that having them separate will not contribute to combinatorial explosion.
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kMatrixColorFilter] = {
            /*name=*/"MatrixColorFilter",
            /*function=*/"sk_matrix_colorfilter",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{ { "matrix",    SkSLType::kFloat4x4 },
                           { "translate", SkSLType::kFloat4 },
                           { "inHSL",     SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kTableColorFilter] = {
            /*name=*/"TableColorFilter",
            /*function=*/"sk_table_colorfilter",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{},
            /*textures=*/{ {"table"} }};
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kGaussianColorFilter] = {
            /*name=*/"GaussianColorFilter",
            /*function=*/"sk_gaussian_colorfilter",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kColorSpaceXformColorFilter] = {
            /*name=*/"ColorSpaceTransform",
            /*function=*/"sk_color_space_transform",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{ { "flags",          SkSLType::kInt },
                           { "srcKind",        SkSLType::kInt },
                           { "gamutTransform", SkSLType::kHalf3x3 },
                           { "dstKind",        SkSLType::kInt },
                           { "csXformCoeffs",  SkSLType::kHalf4x4 } }
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kBlendShader] = {
            /*name=*/"BlendShader",
            /*function=*/"$blend_shader",
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{},
            /*textures=*/{},
            /*expressionGenerator=*/nullptr,
            GenerateBlendShaderPreamble,
            /*childMasks=*/{
                SnippetRequirementFlags::kNone, // src
                SnippetRequirementFlags::kNone, // dst
                SnippetRequirementFlags::kPriorStageOutput | SnippetRequirementFlags::kBlenderDstColor // blend func
            }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kCoeffBlender] = {
            /*name=*/"CoeffBlender",
            /*function=*/"sk_coeff_blend",
            SnippetRequirementFlags::kPriorStageOutput | SnippetRequirementFlags::kBlenderDstColor,
            /*uniforms=*/{ { "coeffs", SkSLType::kHalf4 } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kBlendModeBlender] = {
            /*name=*/"BlendModeBlender",
            /*function=*/"sk_blend",
            SnippetRequirementFlags::kPriorStageOutput | SnippetRequirementFlags::kBlenderDstColor,
            /*uniforms=*/{ { "blendMode", SkSLType::kInt } }
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPrimitiveColor] = {
            /*name=*/"PrimitiveColor",
            /*function=*/nullptr,
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{},
            /*textures=*/{},
            GeneratePrimitiveColorExpression
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kDstReadSample] = {
            /*name=*/"DstReadSample",
            /*function=*/"$read_surface_color",
            SnippetRequirementFlags::kSurfaceColor,
            /*uniforms=*/{ {"dstOffsetAndInvWH", SkSLType::kFloat4} },
            /*textures=*/{ {"dstCopy"} },
            /*expressionGenerator=*/nullptr,
            GenerateDstReadSamplePreamble,
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kDstReadFetch] = {
            /*name=*/"DstReadFetch",
            /*function=*/"$read_surface_color",
            SnippetRequirementFlags::kSurfaceColor,
            /*uniforms=*/{},
            /*textures=*/{},
            /*expressionGenerator=*/nullptr,
            GenerateDstReadFetchPreamble,
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kClipShader] = {
            /*name=*/"ClipShader",
            /*function=*/nullptr,
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{},
            /*textures=*/{},
            GenerateClipShaderExpression,
            GenerateClipShaderPreamble,
            /*childMasks=*/{{SnippetRequirementFlags::kLocalCoords}}
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kCompose] = {
            /*name=*/"Compose",
            /*function=*/"$compose",
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{},
            /*textures=*/{},
            /*expressionGenerator=*/nullptr,
            GenerateNestedChildrenPreamble,
            /*childMasks=*/{{
                SnippetRequirementFlags::kNone, // inner
                SnippetRequirementFlags::kPriorStageOutput // outer
            }}
    };

    // Fixed-function blend mode snippets are all the same, their functionality is entirely defined
    // by their unique code snippet IDs.
    for (int i = 0; i <= (int) SkBlendMode::kLastCoeffMode; ++i) {
        int ffBlendModeID = kFixedFunctionBlendModeIDOffset + i;
        fBuiltInCodeSnippets[ffBlendModeID] = {
                /*name=*/SkBlendMode_Name(static_cast<SkBlendMode>(i)),
                /*function=*/skgpu::BlendFuncName(static_cast<SkBlendMode>(i)),
                SnippetRequirementFlags::kPriorStageOutput |
                SnippetRequirementFlags::kBlenderDstColor,
                /*uniforms=*/{}
        };
    }
}

// Verify that the built-in code IDs for fixed function blending are consistent with SkBlendMode.
// clang-format off
static_assert((int)SkBlendMode::kClear    == (int)BuiltInCodeSnippetID::kFixedFunctionClearBlendMode    - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kSrc      == (int)BuiltInCodeSnippetID::kFixedFunctionSrcBlendMode      - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kDst      == (int)BuiltInCodeSnippetID::kFixedFunctionDstBlendMode      - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kSrcOver  == (int)BuiltInCodeSnippetID::kFixedFunctionSrcOverBlendMode  - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kDstOver  == (int)BuiltInCodeSnippetID::kFixedFunctionDstOverBlendMode  - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kSrcIn    == (int)BuiltInCodeSnippetID::kFixedFunctionSrcInBlendMode    - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kDstIn    == (int)BuiltInCodeSnippetID::kFixedFunctionDstInBlendMode    - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kSrcOut   == (int)BuiltInCodeSnippetID::kFixedFunctionSrcOutBlendMode   - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kDstOut   == (int)BuiltInCodeSnippetID::kFixedFunctionDstOutBlendMode   - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kSrcATop  == (int)BuiltInCodeSnippetID::kFixedFunctionSrcATopBlendMode  - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kDstATop  == (int)BuiltInCodeSnippetID::kFixedFunctionDstATopBlendMode  - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kXor      == (int)BuiltInCodeSnippetID::kFixedFunctionXorBlendMode      - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kPlus     == (int)BuiltInCodeSnippetID::kFixedFunctionPlusBlendMode     - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kModulate == (int)BuiltInCodeSnippetID::kFixedFunctionModulateBlendMode - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kScreen   == (int)BuiltInCodeSnippetID::kFixedFunctionScreenBlendMode   - kFixedFunctionBlendModeIDOffset);
// clang-format on

} // namespace skgpu::graphite
