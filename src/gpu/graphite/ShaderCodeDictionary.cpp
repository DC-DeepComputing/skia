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
#include "include/private/SkOpts_spi.h"
#include "include/private/SkSLString.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/ReadSwizzle.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/sksl/codegen/SkSLPipelineStageCodeGenerator.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#include <new>

namespace skgpu::graphite {

namespace {

std::string get_mangled_name(const std::string& baseName, int manglingSuffix) {
    return baseName + "_" + std::to_string(manglingSuffix);
}

std::string get_mangled_uniform_name(const ShaderInfo& shaderInfo,
                                     const Uniform& uniform,
                                     int manglingSuffix) {
    std::string result;
    result = uniform.name() + std::string("_") + std::to_string(manglingSuffix);
    if (shaderInfo.ssboIndex()) {
        result = EmitStorageBufferAccess("fs", shaderInfo.ssboIndex(), result.c_str());
    }
    return result;
}

std::string get_mangled_sampler_name(const TextureAndSampler& tex, int manglingSuffix) {
    return tex.name() + std::string("_") + std::to_string(manglingSuffix);
}

void add_param(std::string_view param, std::string* argList) {
    if (!argList->empty()) {
        *argList += ", ";
    }
    *argList += param;
}

std::string snippet_args_list(const ShaderSnippet::Args& args,
                              SkEnumBitMask<SnippetRequirementFlags> requiredFlags) {
    std::string params = "";
    if (requiredFlags & SnippetRequirementFlags::kPriorStageOutput) {
        add_param(args.fPriorStageOutput, &params);
    }
    if (requiredFlags & SnippetRequirementFlags::kRuntimeShaderDstColor) {
        add_param(args.fRuntimeShaderDstColor, &params);
    }
    if (requiredFlags & SnippetRequirementFlags::kLocalCoords) {
        add_param(args.fFragCoord, &params);
    }
    return params;
}

// Returns an expression to invoke the ShaderNode with the given args.
std::string invoke_node(const ShaderInfo& shaderInfo,
                        const ShaderNode* node,
                        ShaderSnippet::Args args) {
    return SkSL::String::printf("%s(%s)",
            get_mangled_name(node->entry()->fFunctionName, node->keyIndex()),
            snippet_args_list(args, node->requiredFlags()));
}

// Emit the glue code needed to invoke a single static helper isolated within its own scope.
// Glue code will assign the resulting color into a variable `half4 outColor%d`, where the %d is
// filled in with 'entryIndex'.
// Utility to add the invocation of the ShaderNode with the given args to the function body being
// built in 'funcBody', assigning the output to a local variable name, which is returned.
std::string assign_node_to_var(const ShaderInfo& shaderInfo,
                               const ShaderNode* node,
                               const ShaderSnippet::Args& args,
                               std::string* funcBody) {
    std::string expr = invoke_node(shaderInfo, node, args);
    std::string outputVar = get_mangled_name("outColor", node->keyIndex());
    SkSL::String::appendf(funcBody,
                          "// %s\n"
                          "half4 %s = %s;",
                          node->entry()->fFunctionName,
                          outputVar.c_str(),
                          expr.c_str());
    return outputVar;
}

// Return an SkSL expression that invokes the node's snippet's static function with the provided
// parameters. This assumes the static function is structured to take the ShaderSnippet::Args first,
// based on its requirement flags, followed by snippet's N "uniforms" and samplers. These are bound
// to the mangled/unique variables for 'node'. Lastly, the M outputs from already invoked child
// nodes are passed in.
//
// This form of invocation is only possible when the snippet does not modify the inputs into its
// children and only requires processing their outputs. The primary benefit is increased reuse
// of heavy static functions that can be referenced by multiple ShaderNodes in the effect tree.
std::string invoke_bound_static_fn(const ShaderInfo& shaderInfo,
                                   const ShaderNode* node,
                                   const ShaderSnippet::Args& args,
                                   SkSpan<const std::string> childOutputs) {
    // ShaderSnippet::Args that are required come first in 'params'
    std::string params = snippet_args_list(args, node->requiredFlags());

    // Append uniforms and then samplers specific to 'node'.
    const ShaderSnippet* entry = node->entry();
    for (size_t i = 0; i < entry->fUniforms.size(); ++i) {
        add_param(get_mangled_uniform_name(shaderInfo, entry->fUniforms[i], node->keyIndex()),
                  &params);
    }
    for (size_t i = 0; i < entry->fTexturesAndSamplers.size(); ++i) {
        add_param(get_mangled_sampler_name(entry->fTexturesAndSamplers[i], node->keyIndex()),
                  &params);
    }

    // Append child output names.
    for (const std::string& childOutputVar : childOutputs) {
        add_param(childOutputVar, &params);
    }

    return SkSL::String::printf("%s(%s)", entry->fFunctionName, params);
}

std::string emit_node_function(const ShaderInfo& shaderInfo,
                               const ShaderNode* node,
                               const ShaderSnippet::Args& args,
                               std::string_view body) {
    std::string priorStageOutput = SkSL::String::printf("half4 %s", args.fPriorStageOutput);
    std::string destColor = SkSL::String::printf("half4 %s", args.fRuntimeShaderDstColor);
    std::string localCoord = SkSL::String::printf("float2 %s", args.fFragCoord);
    return SkSL::String::printf(
        "half4 %s(%s) {"
            "%s"
        "}",
        get_mangled_name(node->entry()->fFunctionName, node->keyIndex()).c_str(),
        snippet_args_list({priorStageOutput, destColor, localCoord}, node->requiredFlags()).c_str(),
        body.c_str()
    );
}

std::string emit_default_node_function(const ShaderInfo& shaderInfo,
                                       const ShaderNode* node,
                                       const ShaderSnippet::Args& args,
                                       std::string_view preChildSkSL) {
    // After 'preChildSkSL' is executed, the default node function invokes each of the node's
    // children with the provided args, and then forwards their outputs and the node's uniform
    // values to the static helper function.
    std::string body{preChildSkSL};
    std::vector<std::string> childOutputVarNames;
    for (const ShaderNode* child : node->children()) {
        // Invoke each child node and store in a local variable before invoking the entry.
        childOutputVarNames.push_back(assign_node_to_var(shaderInfo, child, args, &body));
    }
    body += "return " + invoke_bound_static_fn(shaderInfo, node, args, childOutputVarNames) + ";";
    return emit_node_function(shaderInfo, node, args, body);
}

// Walk the node tree and generate all preambles, accumulating into 'preamble'.
void emit_preambles(const ShaderInfo& shaderInfo,
                    SkSpan<const ShaderNode*> nodes,
                    std::string treeLabel,
                    std::string* preamble) {
    int i = 0;
    for (const ShaderNode* node : nodes) {
        std::string nextLabel = treeLabel + std::to_string(i++);
        if (node->numChildren() > 0) {
            emit_preambles(shaderInfo, node->children(), treeLabel + "->", preamble);
        }
        SkSL::String::appendf(preamble, "// %s (%d): %s\n%s\n",
                              nextLabel.c_str(), node->keyIndex(), node->entry()->fName,
                              node->entry()->fPreambleGenerator(shaderInfo, node));
    }
}

} // anonymous namespace

// The current, incomplete, model for shader construction is:
//   - Static code snippets (which can have an arbitrary signature) live in the Graphite
//     pre-compiled module, which is located at `src/sksl/sksl_graphite_frag.sksl`.
//   - Glue code is generated in a `main` method which calls these static code snippets.
//     The glue code is responsible for:
//            1) gathering the correct (mangled) uniforms
//            2) passing the uniforms and any other parameters to the helper method
//   - The result of the final code snippet is then copied into "sk_FragColor".
//   Note: each entry's 'fStaticFunctionName' field is expected to match the name of a function
//   in the Graphite pre-compiled module.
std::string ShaderInfo::toSkSL(const ResourceBindingRequirements& bindingReqs,
                               const RenderStep* step,
                               const bool useStorageBuffers,
                               const bool defineLocalCoordsVarying,
                               int* numTexturesAndSamplersUsed,
                               Swizzle writeSwizzle) const {
    std::string preamble = EmitVaryings(step,
                                        /*direction=*/"in",
                                        /*emitShadingSsboIndexVarying=*/useStorageBuffers,
                                        defineLocalCoordsVarying);

    // The uniforms and samplers for a snippet are mangled to be unique based on the depth-first
    // traversal index of the ShaderNodes that reference each snippet, as a suffix (i.e., "_%d").
    // TODO: replace hard-coded bufferIDs with the backend's step and paint uniform-buffer indices.
    if (step->numUniforms() > 0) {
        preamble += EmitRenderStepUniforms(
                /*bufferID=*/1, "Step", bindingReqs.fUniformBufferLayout, step->uniforms());
    }
    if (this->ssboIndex()) {
        preamble += EmitPaintParamsStorageBuffer(/*bufferID=*/2, "FS", "fs", fRootNodes);
    } else {
        preamble += EmitPaintParamsUniforms(
                /*bufferID=*/2,
                "FS",
                useStorageBuffers ? bindingReqs.fStorageBufferLayout
                                  : bindingReqs.fUniformBufferLayout,
                fRootNodes);
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

    // Emit preamble declarations and helper functions required for snippets. In the default case
    // this adds functions that bind a node's specific mangled uniforms to the snippet's
    // implementation in the SkSL modules.
    emit_preambles(*this, fRootNodes, "", &preamble);

    std::string mainBody = "void main() {";
    // Set initial color. This will typically be optimized out by SkSL in favor of the paint
    // specifying a color with a solid color shader.
    std::string lastOutputVar = "initialColor";
    mainBody += "half4 initialColor = half4(0);";

    if (step->emitsPrimitiveColor()) {
        mainBody += "half4 primitiveColor;";
        mainBody += step->fragmentColorSkSL();
    }

    // Emit shader main body code, invoking each root node's expression, forwarding the previous
    // node's output to the next.
    static constexpr char kUnusedDstColor[] = "half4(1)";
    static constexpr char kUnusedLocalCoords[] = "float2(0)";
    ShaderSnippet::Args args = {"initialColor",
                                kUnusedDstColor,
                                this->needsLocalCoords() ? "localCoordsVar" : kUnusedLocalCoords};
    for (const ShaderNode* node : fRootNodes) {
        args.fPriorStageOutput = assign_node_to_var(*this, node, args, &mainBody);
    }

    if (writeSwizzle != Swizzle::RGBA()) {
        SkSL::String::appendf(&mainBody, "%s = %s.%s;", lastOutputVar.c_str(),
                                                        lastOutputVar.c_str(),
                                                        writeSwizzle.asString().c_str());
    }
    if (step->emitsCoverage()) {
        mainBody += "half4 outputCoverage;";
        mainBody += step->fragmentCoverageSkSL();
        SkSL::String::appendf(&mainBody, "sk_FragColor = %s * outputCoverage;",
                              lastOutputVar.c_str());
    } else {
        SkSL::String::appendf(&mainBody, "sk_FragColor = %s;", lastOutputVar.c_str());
    }
    mainBody += "}\n";

    return preamble + "\n" + mainBody;
}

size_t ShaderCodeDictionary::PaintParamsKeyPtr::Hash::operator()(PaintParamsKeyPtr p) const {
    return SkOpts::hash_fn(p.fKey->data(), p.fKey->sizeInBytes(), 0);
}

size_t ShaderCodeDictionary::RuntimeEffectKey::Hash::operator()(RuntimeEffectKey k) const {
    return SkOpts::hash_fn(&k, sizeof(k), 0);
}

const ShaderCodeDictionary::Entry* ShaderCodeDictionary::findOrCreate(
        PaintParamsKeyBuilder* builder) {
    PaintParamsKey key = builder->lockAsKey();

    SkAutoSpinlock lock{fSpinLock};

    Entry** existingEntry = fHash.find(PaintParamsKeyPtr{&key});
    if (existingEntry) {
        SkASSERT(fEntryVector[(*existingEntry)->uniqueID().asUInt()] == *existingEntry);
        return *existingEntry;
    }

    uint32_t newID = fEntryVector.size();
    PaintParamsKey newKey = key.clone(&fArena);
    Entry* newEntry = fArena.make([&](void *ptr) { return new(ptr) Entry(newKey, newID); });

    fHash.set(PaintParamsKeyPtr{&newEntry->paintParamsKey()}, newEntry);
    fEntryVector.push_back(newEntry);

    return newEntry;
}

const ShaderCodeDictionary::Entry* ShaderCodeDictionary::lookup(
        UniquePaintParamsID codeID) const {

    if (!codeID.isValid()) {
        return nullptr;
    }

    SkAutoSpinlock lock{fSpinLock};

    SkASSERT(codeID.asUInt() < fEntryVector.size());

    return fEntryVector[codeID.asUInt()];
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

    int userDefinedCodeSnippetID = codeSnippetID - kBuiltInCodeSnippetIDCount;
    if (userDefinedCodeSnippetID < SkTo<int>(fUserDefinedCodeSnippets.size())) {
        return fUserDefinedCodeSnippets[userDefinedCodeSnippetID].get();
    }

    return nullptr;
}

void ShaderCodeDictionary::getShaderInfo(UniquePaintParamsID uniqueID,
                                         ShaderInfo* info) const {
    auto entry = this->lookup(uniqueID);

    entry->paintParamsKey().toShaderInfo(this, info);
}

//--------------------------------------------------------------------------------------------------
namespace {

std::string GenerateDefaultPreamble(const ShaderInfo& shaderInfo,
                                    const ShaderNode* node) {
    const ShaderSnippet::Args args = {"inColor", "destColor", "pos"};
    return emit_default_node_function(shaderInfo, node, args, "");
}

//--------------------------------------------------------------------------------------------------

static constexpr Uniform kLinearGradientUniformsTexture[] = {
        { "point0",      SkSLType::kFloat2 },
        { "point1",      SkSLType::kFloat2 },
        { "numStops",    SkSLType::kInt },
        { "tilemode",    SkSLType::kInt },
        { "colorSpace",  SkSLType::kInt },
        { "doUnPremul",  SkSLType::kInt },
};

static constexpr TextureAndSampler kTextureGradientTexturesAndSamplers[] = {
        {"colorAndOffsetSampler"},
};

static constexpr Uniform kRadialGradientUniformsTexture[] = {
        { "center",      SkSLType::kFloat2 },
        { "radius",      SkSLType::kFloat },
        { "numStops",    SkSLType::kInt },
        { "tilemode",    SkSLType::kInt },
        { "colorSpace",  SkSLType::kInt },
        { "doUnPremul",  SkSLType::kInt },
};

static constexpr Uniform kSweepGradientUniformsTexture[] = {
        { "center",      SkSLType::kFloat2 },
        { "bias",        SkSLType::kFloat },
        { "scale",       SkSLType::kFloat },
        { "numStops",    SkSLType::kInt },
        { "tilemode",    SkSLType::kInt },
        { "colorSpace",  SkSLType::kInt },
        { "doUnPremul",  SkSLType::kInt },
};

static constexpr Uniform kConicalGradientUniformsTexture[] = {
        { "point0",      SkSLType::kFloat2 },
        { "point1",      SkSLType::kFloat2 },
        { "radius0",     SkSLType::kFloat },
        { "radius1",     SkSLType::kFloat },
        { "numStops",    SkSLType::kInt },
        { "tilemode",    SkSLType::kInt },
        { "colorSpace",  SkSLType::kInt },
        { "doUnPremul",  SkSLType::kInt },
};

//--------------------------------------------------------------------------------------------------

std::string GenerateLocalMatrixPreamble(const ShaderInfo& shaderInfo,
                                        const ShaderNode* node) {
    SkASSERT(node->codeSnippetId() == (int) BuiltInCodeSnippetID::kLocalMatrixShader);
    SkASSERT(node->numChildren() == 1);
    const ShaderSnippet::Args args = {"inColor", "destColor", "coords"};
    std::string localMatrix = get_mangled_uniform_name(shaderInfo, node->entry()->fUniforms[0], node->keyIndex());
    std::string modifyCoords = SkSL::String::printf(
            "coords = (%s * coords.xy01).xy;"
            "return %s;",
            localMatrix.c_str(),
            invoke_node(shaderInfo, node->child(0), args).c_str());
    return emit_node_function(shaderInfo, node, args, modifyCoords);
}

//--------------------------------------------------------------------------------------------------
static constexpr int kNumXferFnCoeffs = 7;

static constexpr Uniform kImageShaderUniforms[] = {
        { "imgSize",               SkSLType::kFloat2 },
        { "subset",                SkSLType::kFloat4 },
        { "tilemodeX",             SkSLType::kInt },
        { "tilemodeY",             SkSLType::kInt },
        { "filterMode",            SkSLType::kInt },
        { "useCubic",              SkSLType::kInt },
        { "cubicCoeffs",           SkSLType::kFloat4x4 },
        { "readSwizzle",           SkSLType::kInt },
        // The next 6 uniforms are for the color space transformation
        { "csXformFlags",          SkSLType::kInt },
        { "csXformSrcKind",        SkSLType::kInt },
        { "csXformSrcCoeffs",      SkSLType::kHalf, kNumXferFnCoeffs },
        { "csXformGamutTransform", SkSLType::kHalf3x3 },
        { "csXformDstKind",        SkSLType::kInt },
        { "csXformDstCoeffs",      SkSLType::kHalf, kNumXferFnCoeffs },
};

static constexpr TextureAndSampler kISTexturesAndSamplers[] = {
        {"sampler"},
};

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
static_assert(2 == static_cast<int>(ReadSwizzle::kRRRR),
              "ImageShader code depends on ReadSwizzle");
static_assert(3 == static_cast<int>(ReadSwizzle::kRRR1),
              "ImageShader code depends on ReadSwizzle");
static_assert(4 == static_cast<int>(ReadSwizzle::kBGRA),
              "ImageShader code depends on ReadSwizzle");

static constexpr char kImageShaderName[] = "sk_image_shader";

//--------------------------------------------------------------------------------------------------

std::string GenerateCoordClampPreamble(const ShaderInfo& shaderInfo,
                                       const ShaderNode* node) {
    SkASSERT(node->codeSnippetId() == (int) BuiltInCodeSnippetID::kCoordClampShader);
    SkASSERT(node->numChildren() == 1);
    const ShaderSnippet::Args args = {"inColor", "destColor", "coords"};
    std::string subset = get_mangled_uniform_name(shaderInfo, node->entry()->fUniforms[0], node->keyIndex());
    std::string modifyCoords = SkSL::String::printf(
            "coords = clamp(coords, %s.LT, %s.RB);"
            "return %s;",
            subset.c_str(), subset.c_str(),
            invoke_node(shaderInfo, node->child(0), args).c_str());
    return emit_node_function(shaderInfo, node, args, modifyCoords);
}

//--------------------------------------------------------------------------------------------------
static constexpr Uniform kDitherShaderUniforms[] = {
        { "range", SkSLType::kHalf },
};

static constexpr TextureAndSampler kDitherTexturesAndSamplers[] = {
        {"sampler"},
};

static constexpr char kDitherShaderName[] = "sk_dither_shader";

//--------------------------------------------------------------------------------------------------
static constexpr char kRuntimeShaderName[] = "RuntimeEffect";

class GraphitePipelineCallbacks : public SkSL::PipelineStage::Callbacks {
public:
    GraphitePipelineCallbacks(const ShaderInfo& shaderInfo,
                              const ShaderNode* node,
                              std::string* preamble)
            : fShaderInfo(shaderInfo)
            , fNode(node)
            , fPreamble(preamble) {}

    std::string declareUniform(const SkSL::VarDeclaration* decl) override {
        std::string result = get_mangled_name(std::string(decl->var()->name()), fNode->keyIndex());
        if (fShaderInfo.ssboIndex()) {
            result = EmitStorageBufferAccess("fs", fShaderInfo.ssboIndex(), result.c_str());
        }
        return result;
    }

    void defineFunction(const char* decl, const char* body, bool isMain) override {
        if (isMain) {
            *fPreamble += emit_node_function(fShaderInfo, fNode, {"inColor", "destColor", "pos"}, body);
        } else {
            SkSL::String::appendf(fPreamble, "%s {%s}\n", decl, body);
        }
    }

    void declareFunction(const char* decl) override {
        *fPreamble += std::string(decl) + ";";
    }

    void defineStruct(const char* definition) override {
        *fPreamble += std::string(definition) + ";";
    }

    void declareGlobal(const char* declaration) override {
        *fPreamble += std::string(declaration) + ";";
    }

    std::string sampleShader(int index, std::string coords) override {
        SkASSERT(index >= 0 && index < fNode->numChildren());
        return invoke_node(fShaderInfo, fNode->child(index), {"", "", coords});
    }

    std::string sampleColorFilter(int index, std::string color) override {
        SkASSERT(index >= 0 && index < fNode->numChildren());
        return invoke_node(fShaderInfo, fNode->child(index), {color, "", ""});
    }

    std::string sampleBlender(int index, std::string src, std::string dst) override {
        SkASSERT(index >= 0 && index < fNode->numChildren());
        return invoke_node(fShaderInfo, fNode->child(index), {src, dst, ""});
    }

    std::string toLinearSrgb(std::string color) override {
        // TODO(skia:13508): implement to-linear-SRGB child effect
        return color;
    }
    std::string fromLinearSrgb(std::string color) override {
        // TODO(skia:13508): implement from-linear-SRGB child effect
        return color;
    }

    std::string getMangledName(const char* name) override {
        return get_mangled_name(name, fNode->keyIndex());
    }

private:
    const ShaderInfo& fShaderInfo;
    const ShaderNode* fNode;
    std::string* fPreamble;
};

std::string GenerateRuntimeShaderPreamble(const ShaderInfo& shaderInfo,
                                          const ShaderNode* node) {
    // Find this runtime effect in the runtime-effect dictionary which is sole location that
    // defines the SkSL of the runtime effect (the ShaderCodeDictionary does not).
    const SkRuntimeEffect* effect =
            shaderInfo.runtimeEffectDictionary()->find(node->codeSnippetId());
    SkASSERT(effect);
    const SkSL::Program& program = SkRuntimeEffectPriv::Program(*effect);

    std::string preamble;
    GraphitePipelineCallbacks callbacks{shaderInfo, node, &preamble};
    SkSL::PipelineStage::ConvertProgram(program, "pos", "inColor", "destColor", &callbacks);
    return preamble;
}

//--------------------------------------------------------------------------------------------------
static constexpr Uniform kBlendColorFilterUniforms[] = {
        { "blendMode", SkSLType::kInt },
        { "color",     SkSLType::kFloat4 }
};

static constexpr char kBlendColorFilterName[] = "sk_blend_colorfilter";

//--------------------------------------------------------------------------------------------------

// FIXME I think this can go away if we just construct the composed children directly?
// I think it gets trickier when both branches are a chain
// and hypothetically we have this child issue for regular color filters because you could create
//  a complex color filter tree under one root CF and it could reference the SkShader's output
//  multiple times
// In that case it really is that the CF nodes should be pointing to a lightweight reference node
//  (e.g. priorStageOutput) otherwise we would end up replicating the shader ocmponent many times.
// ugh that is a little clunky. it is much easier to fit with Greg's approach of discrete stages
// that each have a tree, but how would we represent that?
// PriorStage block that is placed every time one of the Sk objects is null?
std::string GenerateNestedChildrenPreamble(const ShaderInfo& shaderInfo,
                                           const ShaderNode* node) {
    SkASSERT(node->codeSnippetId() == (int) BuiltInCodeSnippetID::kComposeColorFilter);
    SkASSERT(node->numChildren() == 2);

    const ShaderSnippet::Args args = {"inColor", "", ""};
    std::string body = "";

    // Evaluate inner child and save result to a local
    std::string innerColor = assign_node_to_var(shaderInfo, node->child(1), args, &body);
    // Override the input color for evaluation of the outer child
    const ShaderSnippet::Args outerArgs = {innerColor, "", ""};
    SkSL::String::appendf(&body, "return %s;", invoke_node(shaderInfo, node->child(0), outerArgs));
    return emit_node_function(shaderInfo, node, args, body);
}

//--------------------------------------------------------------------------------------------------
static constexpr Uniform kColorSpaceTransformUniforms[] = {
        { "flags",          SkSLType::kInt },
        { "srcKind",        SkSLType::kInt },
        { "srcCoeffs",      SkSLType::kHalf, kNumXferFnCoeffs },
        { "gamutTransform", SkSLType::kHalf3x3 },
        { "dstKind",        SkSLType::kInt },
        { "dstCoeffs",      SkSLType::kHalf, kNumXferFnCoeffs },
};

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

static constexpr char kColorSpaceTransformName[] = "sk_color_space_transform";

//--------------------------------------------------------------------------------------------------

// This method generates the glue code for the case where the SkBlendMode-based blending is
// handled with fixed function blending.
// FIXME: -> default expression doesn't make sense for this case
std::string GenerateFixedFunctionBlenderExpression(const ShaderInfo&,
                                                   int entryIndex,
                                                   const PaintParamsKey::BlockReader& reader,
                                                   const ShaderSnippet::Args& args) {
    SkASSERT(reader.entry()->fUniforms.empty());

    // The actual blending is set up via the fixed function pipeline so we don't actually
    // need to access the blend mode in the glue code.
    return std::string(args.fPriorStageOutput);
}

//--------------------------------------------------------------------------------------------------
static constexpr Uniform kShaderBasedBlenderUniforms[] = {
        { "blendMode", SkSLType::kInt },
};

static constexpr char kBlendHelperName[] = "sk_blend";

// This method generates the glue code for the case where the SkBlendMode-based blending must occur
// in the shader (i.e., fixed function blending isn't possible).
// It exists as custom glue code so that we can deal with the dest reads. If that can be
// standardized (e.g., via a snippets requirement flag) this could be removed.
// std::string GenerateShaderBasedBlenderExpression(const ShaderInfo& shaderInfo,
//                                                  int entryIndex,
//                                                  const PaintParamsKey::BlockReader& reader,
//                                                  const ShaderSnippet::Args& args) {
//     const bool usePrimitiveColorAsDst = reader.entry()->blendAgainstPrimitiveColor();

//     SkASSERT(reader.entry()->fUniforms.size() == 1);

//     std::string uniformName = reader.entry()->getMangledUniformName(shaderInfo, 0, entryIndex);

//     // FIXME hmmm, how to consolidate this? is it used for both shader blends and primitive blends?
//     // presumably that makes for two snippet definitions

//     // TODO: emit function to perform dest read into preamble, and replace half4(1) with that call
//     // (The `args.destColor` variable might seem tempting here, but this is used for programmatic
//     // src+dest blends within the shader, not for blends against the destination surface.)
//     const char * destColor = usePrimitiveColorAsDst ? "primitiveColor" : "half4(1)";

//     return SkSL::String::printf("%s(%s, %.*s, %s)",
//                                 reader.entry()->fStaticFunctionName,
//                                 uniformName.c_str(),
//                                 (int)args.fPriorStageOutput.size(), args.fPriorStageOutput.data(),
//                                 destColor);
// }
// FIXME need special blocks for paint color and primitive color and dst surface color that can be
// wired up for children to blend blocks so that the blends are just default generated.

//--------------------------------------------------------------------------------------------------

} // anonymous namespace

bool ShaderCodeDictionary::isValidID(int snippetID) const {
    if (snippetID < 0) {
        return false;
    }

    if (snippetID < kBuiltInCodeSnippetIDCount) {
        return true;
    }

    int userDefinedCodeSnippetID = snippetID - kBuiltInCodeSnippetIDCount;
    return userDefinedCodeSnippetID < SkTo<int>(fUserDefinedCodeSnippets.size());
}

int ShaderCodeDictionary::addUserDefinedSnippet(
        const char* functionName,
        SkSpan<const Uniform> uniforms,
        SkSpan<const TextureAndSampler> texturesAndSamplers,
        SkEnumBitMask<SnippetRequirementFlags> snippetRequirementFlags,
        ShaderSnippet::GeneratePreambleForSnippetFn preambleGenerator,
        int numChildren) {
    // TODO: the memory for user-defined entries could go in the dictionary's arena but that
    // would have to be a thread safe allocation since the arena also stores entries for
    // 'fHash' and 'fEntryVector'
    fUserDefinedCodeSnippets.push_back(std::make_unique<ShaderSnippet>(functionName,
                                                                       uniforms,
                                                                       texturesAndSamplers,
                                                                       snippetRequirementFlags,
                                                                       preambleGenerator,
                                                                       numChildren));

    return kBuiltInCodeSnippetIDCount + fUserDefinedCodeSnippets.size() - 1;
}

// TODO: this version needs to be removed
int ShaderCodeDictionary::addUserDefinedSnippet(const char* name) {
    return this->addUserDefinedSnippet(name,
                                       /*uniforms=*/{},
                                       /*texturesAndSamplers=*/{},
                                       SnippetRequirementFlags::kNone,
                                       GenerateDefaultPreamble,
                                       /*numChildren=*/0);
}

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
            case Type::kInt:      return SkSLType::kShort;
            case Type::kInt2:     return SkSLType::kShort2;
            case Type::kInt3:     return SkSLType::kShort3;
            case Type::kInt4:     return SkSLType::kShort4;
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

    // Convert the SkRuntimeEffect::Uniform array into its Uniform equivalent.
    int numUniforms = uniforms.size();
    Uniform* uniformArray = fArena.makeInitializedArray<Uniform>(numUniforms, [&](int index) {
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

int ShaderCodeDictionary::findOrCreateRuntimeEffectSnippet(const SkRuntimeEffect* effect) {
    // Use the combination of {SkSL program hash, uniform size} as our key.
    // In the unfortunate event of a hash collision, at least we'll have the right amount of
    // uniform data available.
    RuntimeEffectKey key;
    key.fHash = SkRuntimeEffectPriv::Hash(*effect);
    key.fUniformSize = effect->uniformSize();

    SkAutoSpinlock lock{fSpinLock};

    int32_t* existingCodeSnippetID = fRuntimeEffectMap.find(key);
    if (existingCodeSnippetID) {
        return *existingCodeSnippetID;
    }

    SkEnumBitMask<SnippetRequirementFlags> snippetFlags = SnippetRequirementFlags::kNone;
    if (effect->allowShader()) {
        snippetFlags |= SnippetRequirementFlags::kLocalCoords;
    }
    if (effect->allowBlender()) {
        snippetFlags |= SnippetRequirementFlags::kRuntimeShaderDstColor;
    }
    int newCodeSnippetID = this->addUserDefinedSnippet("RuntimeEffect",
                                                       this->convertUniforms(effect),
                                                       /*texturesAndSamplers=*/{},
                                                       snippetFlags,
                                                       GenerateRuntimeShaderPreamble,
                                                       (int)effect->children().size());
    fRuntimeEffectMap.set(key, newCodeSnippetID);
    return newCodeSnippetID;
}

ShaderCodeDictionary::ShaderCodeDictionary() {
    // The 0th index is reserved as invalid
    fEntryVector.push_back(nullptr);

    // FIXME more useful as PriorStageOutput block?
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPassthroughShader] = {
            "sk_passthrough",
            /*uniforms=*/{ },
            /*samplers=*/{ },
            SnippetRequirementFlags::kPriorStageOutput
    };
    // FIXME what does this even mean?
    // SkBlenders are just functions that take src/dst values; they can be invoked arbitrarily byu
    // runtime sksl, so there isn't a parent/child relationship in the effect graph between the
    // parent runtime effect and the inputs to the blender, so if the blender is null bound andthe
    // runtime effect still has a means to calculate src/dst it is expecting a single value back
    //  But the annoying thing here is that if we do this by exposing the requirement flags to have (src,dst)
    //  be in the signature, requirements bubble up when in the case of a runtime effect, they really shouldn't?
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPassthroughBlender] = {
            "blend_src_over",
            /*uniforms=*/{ },
            /*samplers=*/{ },
            SnippetRequirementFlags::kPriorStageOutput |
                    SnippetRequirementFlags::kRuntimeShaderDstColor,
            {},      // no samplers
            kPassthroughBlenderName,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kSolidColorShader] = {
            "sk_solid_shader",
            /*uniforms=*/{{ "color", SkSLType::kFloat4 }},
            /*samplers=*/{ },
            SnippetRequirementFlags::kNone
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLinearGradientShader4] = {
            "sk_linear_grad_4_shader",
            /*uniforms=*/{{ "colors",     SkSLType::kFloat4, 4 },
                          { "offsets",    SkSLType::kFloat,  4 },
                          { "point0",     SkSLType::kFloat2 },
                          { "point1",     SkSLType::kFloat2 },
                          { "tilemode",   SkSLType::kInt },
                          { "colorSpace", SkSLType::kInt },
                          { "doUnPremul", SkSLType::kInt }},
            /*samplers=*/{ },
            SnippetRequirementFlags::kLocalCoords
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLinearGradientShader8] = {
            "sk_linear_grad_8_shader",
            /*uniforms=*/{{ "colors",     SkSLType::kFloat4, 8 },
                          { "offsets",    SkSLType::kFloat,  8 },
                          { "point0",     SkSLType::kFloat2 },
                          { "point1",     SkSLType::kFloat2 },
                          { "tilemode",   SkSLType::kInt },
                          { "colorSpace", SkSLType::kInt },
                          { "doUnPremul", SkSLType::kInt }},
            /*samplers=*/{ },
            SnippetRequirementFlags::kLocalCoords
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLinearGradientShaderTexture] = {
            "sk_linear_grad_tex_shader",
            SkSpan(kLinearGradientUniformsTexture),
            SnippetRequirementFlags::kLocalCoords,
            SkSpan(kTextureGradientTexturesAndSamplers)
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kRadialGradientShader4] = {
            "sk_radial_grad_4_shader",
            /*uniforms=*/{{ "colors",     SkSLType::kFloat4, 4 },
                          { "offsets",    SkSLType::kFloat,  4 },
                          { "center",     SkSLType::kFloat2 },
                          { "radius",     SkSLType::kFloat },
                          { "tilemode",   SkSLType::kInt },
                          { "colorSpace", SkSLType::kInt },
                          { "doUnPremul", SkSLType::kInt }},
            /*samplers=*/{ },
            SnippetRequirementFlags::kLocalCoords
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kRadialGradientShader8] = {
            "sk_radial_grad_8_shader",
            /*uniforms=*/{{ "colors",     SkSLType::kFloat4, 8 },
                          { "offsets",    SkSLType::kFloat,  8 },
                          { "center",     SkSLType::kFloat2 },
                          { "radius",     SkSLType::kFloat },
                          { "tilemode",   SkSLType::kInt },
                          { "colorSpace", SkSLType::kInt },
                          { "doUnPremul", SkSLType::kInt }},
            /*samplers=*/{ },
            SnippetRequirementFlags::kLocalCoords
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kRadialGradientShaderTexture] = {
            "sk_radial_grad_tex_shader",
            SkSpan(kRadialGradientUniformsTexture),
            /*samplers=*/{ },
            SnippetRequirementFlags::kLocalCoords,
            SkSpan(kTextureGradientTexturesAndSamplers),
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kSweepGradientShader4] = {
            "sk_sweep_grad_4_shader",
            /*uniforms=*/{{ "colors",     SkSLType::kFloat4, 4 },
                          { "offsets",    SkSLType::kFloat,  4 },
                          { "center",     SkSLType::kFloat2 },
                          { "bias",       SkSLType::kFloat },
                          { "scale",      SkSLType::kFloat },
                          { "tilemode",   SkSLType::kInt },
                          { "colorSpace", SkSLType::kInt },
                          { "doUnPremul", SkSLType::kInt }},
            /*samplers=*/{ },
            SnippetRequirementFlags::kLocalCoords
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kSweepGradientShader8] = {
            "sk_sweep_grad_8_shader",
            /*uniforms=*/{{ "colors",     SkSLType::kFloat4, 8 },
                          { "offsets",    SkSLType::kFloat,  8 },
                          { "center",     SkSLType::kFloat2 },
                          { "bias",       SkSLType::kFloat },
                          { "scale",      SkSLType::kFloat },
                          { "tilemode",   SkSLType::kInt },
                          { "colorSpace", SkSLType::kInt },
                          { "doUnPremul", SkSLType::kInt }},
            /*samplers=*/{ },
            SnippetRequirementFlags::kLocalCoords
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kSweepGradientShaderTexture] = {
            "sk_sweep_grad_tex_shader",
            SkSpan(kSweepGradientUniformsTexture),
            /*samplers=*/{ },
            SnippetRequirementFlags::kLocalCoords,
            SkSpan(kTextureGradientTexturesAndSamplers),
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kConicalGradientShader4] = {
            "sk_conical_grad_4_shader",
            /*uniforms=*/{{ "colors",     SkSLType::kFloat4, 4 },
                          { "offsets",    SkSLType::kFloat,  4 },
                          { "point0",     SkSLType::kFloat2 },
                          { "point1",     SkSLType::kFloat2 },
                          { "radius0",    SkSLType::kFloat },
                          { "radius1",    SkSLType::kFloat },
                          { "tilemode",   SkSLType::kInt },
                          { "colorSpace", SkSLType::kInt },
                          { "doUnPremul", SkSLType::kInt }},
            /*samplers=*/{ },
            SnippetRequirementFlags::kLocalCoords
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kConicalGradientShader8] = {
            "sk_conical_grad_8_shader",
            /*uniforms=*/{{ "colors",     SkSLType::kFloat4, 8 },
                          { "offsets",    SkSLType::kFloat,  8 },
                          { "point0",     SkSLType::kFloat2 },
                          { "point1",     SkSLType::kFloat2 },
                          { "radius0",    SkSLType::kFloat },
                          { "radius1",    SkSLType::kFloat },
                          { "tilemode",   SkSLType::kInt },
                          { "colorSpace", SkSLType::kInt },
                          { "doUnPremul", SkSLType::kInt }},
            /*samplers=*/{ },
            SnippetRequirementFlags::kLocalCoords
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kConicalGradientShaderTexture] = {
            "sk_conical_grad_tex_shader",
            SkSpan(kConicalGradientUniformsTexture),
            SnippetRequirementFlags::kLocalCoords,
            SkSpan(kTextureGradientTexturesAndSamplers),
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLocalMatrixShader] = {
            "LocalMatrixShader", // FIXME not a module function, but useful in comments and helper func name
            /*uniforms=*/{{ "localMatrix", SkSLType::kFloat4x4 }},
            /*samplers=*/{ },
            SnippetRequirementFlags::kLocalCoords,
            /*numChildren=*/1,
            GenerateLocalMatrixPreamble
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kImageShader] = {
            "ImageShader",
            SkSpan(kImageShaderUniforms),
            SnippetRequirementFlags::kLocalCoords,
            SkSpan(kISTexturesAndSamplers),
            kImageShaderName,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kCoordClampShader] = {
            "CoordClampShader", // FIXME not a module function, but useful in comments and helper func name
            /*uniforms=*/{{ "subset", SkSLType::kFloat4 }},
            /*samplers=*/{ },
            SnippetRequirementFlags::kLocalCoords,
            /*numChildren=*/1,
            GenerateCoordClampPreamble
    };
    // FIXME this should be set up as a child relationship to avoid use of priorStageOutput
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kDitherShader] = {
            "DitherShader",
            SkSpan(kDitherShaderUniforms),
            (SnippetRequirementFlags::kPriorStageOutput | SnippetRequirementFlags::kLocalCoords),
            SkSpan(kDitherTexturesAndSamplers),
            kDitherShaderName,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPerlinNoiseShader] = {
            "perlin_noise_shader",
            /*uniforms=*/{{ "baseFrequency", SkSLType::kFloat2 },
                          { "stitchData",    SkSLType::kFloat2 },
                          { "noiseType",     SkSLType::kInt },
                          { "numOctaves",    SkSLType::kInt },
                          { "stitching",     SkSLType::kInt }},
            /*samplers=*/{{ "permutationsSampler" },
                          { "noiseSampler" }},
            SnippetRequirementFlags::kLocalCoords};
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPorterDuffBlendShader] = {
            "porter_duff_blend_shader",
            /*uniforms=*/{{ "blendConstants", SkSLType::kHalf4 }},
            /*samplers=*/{ },
            SnippetRequirementFlags::kNone,
            /*numChildren=*/2
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kBlendShader] = {
            "sk_blend_shader",
            /*uniforms=*/{{ "blendMode", SkSLType::kInt }},
            /*samplers=*/{ },
            SnippetRequirementFlags::kNone,
            /*numChildren=*/2
    };

    // FIXME equivalent to compose color filter color filter, right? well not quite because of where
    // the input to the first step comes from (this one doesn't need it, CF compose does)
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kColorFilterShader] = {
            "ColorFilterShader", // FIXME not a module function, but useful in comments and helper func name
            /*uniforms=*/{ },
            /*samplers=*/{ },
            SnippetRequirementFlags::kNone,
            /*numChildren=*/2,
            GenerateNestedChildrenPreamble
    };

    // SkColorFilter snippets

    // TODO: investigate the implications of having separate hlsa and rgba matrix colorfilters. It
    // may be that having them separate will not contribute to combinatorial explosion.
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kMatrixColorFilter] = {
            "sk_matrix_colorfilter",
            /*uniforms=*/{{ "matrix",    SkSLType::kFloat4x4 },
                          { "translate", SkSLType::kFloat4 },
                          { "inHSL",     SkSLType::kInt }},
            /*samplers=*/{ },
            SnippetRequirementFlags::kPriorStageOutput};
    // FIXME remove -> in-shader blend with a solid-color snippet AND the input color filter as a child
    // but notably this means that the blend block has to have 2 children, it doesn't have requirements
    // so invoking it only has 'priorStageOutput' if one of its nested children would need to return that
    // -> so the blend block's helper function is really going to be zero-args by default
    //    and then it invokes each child, and then calls sk_blend(bm, src, dst)
    //
    // -> the only case that doesn't match this is runtime effect blenders, for both passthrough and
    //    regular, then want to pass in dynamic arguments, not have children bindings.
    //    they also have this issue with shaders and color filters
    //
    //    really, it has the proper model of function invocation and its the children model that
    //    is a little less flexible, although not unreasonable in terms of gluing the different
    //    parts together. But if we want the blending logic to be a child-less function (well I suppose
    //    it can have its own internal functions), how do we structure the tree blend system on top
    //    of that?
    //     -> TreeBlendBlock has 3 children -> src, dst, blender and then puts the src/dst child outputs
    //        as dynamic args to the blender itself.
    //
    //        which is valid but annoying because we then have this extra block running around
    //
    // So if we have blend(src, dst) as the SkBlender signature.
    //
    //  but if we have a blend block wired up with children, it'd be blend() -> blend(unis, src, dst)
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kBlendColorFilter] = {
            "BlendColorFilter",
            SkSpan(kBlendColorFilterUniforms),
            SnippetRequirementFlags::kPriorStageOutput,
            { },     // no samplers
            kBlendColorFilterName,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kComposeColorFilter] = {
            "ComposeColorFilter", // FIXME not a module function, but useful in comments and helper func name
            /*uniforms=*/{ },
            /*samplers=*/{ },
            SnippetRequirementFlags::kPriorStageOutput,
            /*numChildren=*/2,
            GenerateNestedChildrenPreamble
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kTableColorFilter] = {
            "sk_table_colorfilter",
            /*uniforms=*/{ },
            /*samplers=*/{{ "tableSampler" }},
            SnippetRequirementFlags::kPriorStageOutput};
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kGaussianColorFilter] = {
            "sk_gaussian_colorfilter",
            /*uniforms=*/{ },
            /*samplers=*/{ },
            SnippetRequirementFlags::kPriorStageOutput};
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kColorSpaceXformColorFilter] = {
            "ColorSpaceTransform",
            SkSpan(kColorSpaceTransformUniforms),
            SnippetRequirementFlags::kPriorStageOutput,
            { },     // no samplers
            kColorSpaceTransformName,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren
    };
    // FIXME This is a shader blend block with the rest of the paint as one child and a dst-color block as the other child
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kShaderBasedBlender] = {
            "ShaderBasedBlender",
            SkSpan(kShaderBasedBlenderUniforms),
            SnippetRequirementFlags::kNone,
            { },     // no samplers
            kBlendHelperName,
            GenerateShaderBasedBlenderExpression,
            GenerateDefaultPreamble,
            kNoChildren
    };
    // FIXME this is a shader blend block with the shader as one child and a primitive-color block as the other
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPrimitiveColorShaderBasedBlender] = {
            "PrimitiveColorShaderBasedBlender",
            SkSpan(kShaderBasedBlenderUniforms),
            SnippetRequirementFlags::kBlendAgainstPrimitiveColor,
            { },     // no samplers
            kBlendHelperName,
            GenerateShaderBasedBlenderExpression,
            GenerateDefaultPreamble,
            kNoChildren
    };

    // FIXME these are going away? or single child variant of identity function
    // Fixed-function blend mode snippets are all the same, their functionality is entirely defined
    // by their unique code snippet IDs.
    for (int i = 0; i <= (int) SkBlendMode::kLastCoeffMode; ++i) {
        int ffBlendModeID = kFixedFunctionBlendModeIDOffset + i;
        fBuiltInCodeSnippets[ffBlendModeID] = {
                SkBlendMode_Name(static_cast<SkBlendMode>(i)),
                { },     // no uniforms
                SnippetRequirementFlags::kNone,
                { },     // no samplers
                "",  // fixed function blending doesn't use static SkSL
                GenerateFixedFunctionBlenderExpression,
                GenerateDefaultPreamble,
                kNoChildren
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
