/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkShaderCodeDictionary.h"

#include "include/private/SkSLString.h"
#include "src/core/SkOpts.h"

namespace {

void add_indent(std::string* result, int indent) {
    result->append(4*indent, ' ');
}

} // anonymous namespace

// TODO: SkShaderInfo::toSkSL needs to work outside of both just graphite and metal. To do
// so we'll need to switch over to using SkSL's uniform capabilities.
#if SK_SUPPORT_GPU && defined(SK_GRAPHITE_ENABLED) && defined(SK_METAL)

#include <set>

// TODO: switch this over to using SkSL's uniform system
namespace skgpu::mtl {
std::string GetMtlUniforms(int bufferID,
                           const char* name,
                           const std::vector<SkShaderInfo::SnippetEntry>&);
} // namespace skgpu::mtl

// Emit the glue code needed to invoke a single static helper isolated w/in its own scope.
// The structure of this will be:
//
//     half4 outColor%d;
//     {
//         half4 child-outColor%d;  // for each child
//         {
//             /* emitted snippet sksl assigns to child-outColor%d */
//         }
//
//         /* emitted snippet sksl assigns to outColor%d - taking a vector of child var names */
//     }
// Where the %d is filled in with 'entryIndex'.
std::string SkShaderInfo::emitGlueCodeForEntry(int* entryIndex,
                                               std::string* result,
                                               int indent) const {
    const SkShaderInfo::SnippetEntry& entry = fEntries[*entryIndex];
    int curEntryIndex = *entryIndex;

    std::string scopeOutputVar(std::string("outColor") + std::to_string(curEntryIndex));

    add_indent(result, indent);
    SkSL::String::appendf(result, "half4 %s;\n", scopeOutputVar.c_str());
    add_indent(result, indent);
    *result += "{\n";

    // Although the children appear after the parent in the shader info they are emitted
    // before the parent
    std::vector<std::string> childNames;
    for (int j = 0; j < entry.fNumChildren; ++j) {
        *entryIndex += 1;
        std::string childOutputVar = this->emitGlueCodeForEntry(entryIndex, result, indent+1);
        childNames.push_back(childOutputVar);
    }

    *result += (entry.fGlueCodeGenerator)(scopeOutputVar, curEntryIndex,
                                          entry, childNames, indent+1);
    add_indent(result, indent);
    *result += "}\n";

    return scopeOutputVar;
}

// The current, incomplete, model for shader construction is:
//   each static code snippet (which can have an arbitrary signature) gets emitted once as a
//            preamble
//   glue code is then generated w/in the "main" method. The glue code is responsible for:
//            1) gathering the correct (mangled) uniforms
//            2) passing the uniforms and any other parameters to the helper method
//   The result of the last code snippet is then copied into "sk_FragColor".
// Note: that each entry's 'fStaticFunctionName' field must match the name of the method defined
// in the 'fStaticSkSL' field.
std::string SkShaderInfo::toSkSL() const {
    // The uniforms are mangled by having their index in 'fEntries' as a suffix (i.e., "_%d")
    std::string result = skgpu::mtl::GetMtlUniforms(2, "FS", fEntries);

    std::set<const char*> emittedStaticSnippets;
    for (auto c : fEntries) {
        if (emittedStaticSnippets.find(c.fStaticFunctionName) == emittedStaticSnippets.end()) {
            result += c.fStaticSkSL;
            emittedStaticSnippets.insert(c.fStaticFunctionName);
        }
    }

    result += "layout(location = 0, index = 0) out half4 sk_FragColor;\n";
    result += "void main() {\n";

    // TODO: for some effects (e.g., SW blending) we will need to feed the output variable
    // name from the prior step into the current step's glue code (and deal with the
    // initial color issue).
    std::string lastOutputVar;
    for (int entryIndex = 0; entryIndex < (int) fEntries.size(); ++entryIndex) {
        lastOutputVar = this->emitGlueCodeForEntry(&entryIndex, &result, 1);
    }

    SkSL::String::appendf(&result, "    sk_FragColor = %s;\n", lastOutputVar.c_str());
    result += "}\n";

    return result;
}
#endif

SkShaderCodeDictionary::Entry* SkShaderCodeDictionary::makeEntry(const SkPaintParamsKey& key) {
    return fArena.make([&](void *ptr) { return new(ptr) Entry(key); });
}

size_t SkShaderCodeDictionary::Hash::operator()(const SkPaintParamsKey& key) const {
    return SkOpts::hash_fn(key.data(), key.sizeInBytes(), 0);
}

const SkShaderCodeDictionary::Entry* SkShaderCodeDictionary::findOrCreate(
        const SkPaintParamsKey& key) {
    SkAutoSpinlock lock{fSpinLock};

    auto iter = fHash.find(key);
    if (iter != fHash.end()) {
        SkASSERT(fEntryVector[iter->second->uniqueID().asUInt()] == iter->second);
        return iter->second;
    }

    Entry* newEntry = this->makeEntry(key);
    newEntry->setUniqueID(fEntryVector.size());
    fHash.insert(std::make_pair(newEntry->paintParamsKey(), newEntry));
    fEntryVector.push_back(newEntry);

    return newEntry;
}

const SkShaderCodeDictionary::Entry* SkShaderCodeDictionary::lookup(
        SkUniquePaintParamsID codeID) const {

    if (!codeID.isValid()) {
        return nullptr;
    }

    SkAutoSpinlock lock{fSpinLock};

    SkASSERT(codeID.asUInt() < fEntryVector.size());

    return fEntryVector[codeID.asUInt()];
}

SkSpan<const SkUniform> SkShaderCodeDictionary::getUniforms(SkBuiltInCodeSnippetID id) const {
    return fCodeSnippets[(int) id].fUniforms;
}

const SkShaderInfo::SnippetEntry* SkShaderCodeDictionary::getEntry(SkBuiltInCodeSnippetID id) const {
    return &fCodeSnippets[(int) id];
}

void SkShaderCodeDictionary::getShaderInfo(SkUniquePaintParamsID uniqueID, SkShaderInfo* info) {
    auto entry = this->lookup(uniqueID);

    entry->paintParamsKey().toShaderInfo(this, info);
}
//--------------------------------------------------------------------------------------------------
namespace {

// The default glue code just calls a helper function with the signature:
//    half4 fStaticFunctionName(/* all uniforms as parameters */);
// and stores the result in a variable named "resultName".
std::string GenerateDefaultGlueCode(const std::string& resultName,
                                    int entryIndex,
                                    const SkShaderInfo::SnippetEntry& entry,
                                    const std::vector<std::string>& childNames,
                                    int indent) {
    SkASSERT(childNames.empty());

    std::string result;

    add_indent(&result, indent);
    SkSL::String::appendf(&result, "%s = %s(", resultName.c_str(), entry.fStaticFunctionName);
    for (size_t i = 0; i < entry.fUniforms.size(); ++i) {
        // The uniform names are mangled w/ the entry's index as a suffix
        result += entry.fUniforms[i].name() + std::string("_") + std::to_string(entryIndex);

        if (i+1 < entry.fUniforms.size()) {
            result += ", ";
        }
    }
    result += ");\n";

    return result;
}

//--------------------------------------------------------------------------------------------------
static constexpr int kFourStopGradient = 4;

// TODO: For the sprint we unify all the gradient uniforms into a standard set of 6:
//   kMaxStops colors
//   kMaxStops offsets
//   2 points
//   2 radii
static constexpr int kNumGradientUniforms = 7;
static constexpr SkUniform kGradientUniforms[kNumGradientUniforms] = {
        { "colors",  SkSLType::kFloat4, kFourStopGradient },
        { "offsets", SkSLType::kFloat, kFourStopGradient },
        { "point0",  SkSLType::kFloat2 },
        { "point1",  SkSLType::kFloat2 },
        { "radius0", SkSLType::kFloat },
        { "radius1", SkSLType::kFloat },
        { "padding", SkSLType::kFloat2 } // TODO: add automatic uniform padding
};

static const char *kLinearGradient4Name = "linear_grad_4_shader";
static const char *kLinearGradient4SkSL =
        // TODO: This should use local coords
        "half4 linear_grad_4_shader(float4 colorsParam[4],\n"
        "                           float offsetsParam[4],\n"
        "                           float2 point0Param,\n"
        "                           float2 point1Param,\n"
        "                           float radius0Param,\n"
        "                           float radius1Param,\n"
        "                           float2 padding) {\n"
        "    float2 pos = sk_FragCoord.xy;\n"
        "    float2 delta = point1Param - point0Param;\n"
        "    float2 pt = pos - point0Param;\n"
        "    float t = dot(pt, delta) / dot(delta, delta);\n"
        "    float4 result = colorsParam[0];\n"
        "    result = mix(result, colorsParam[1],\n"
        "                 clamp((t-offsetsParam[0])/(offsetsParam[1]-offsetsParam[0]),\n"
        "                       0, 1));\n"
        "    result = mix(result, colorsParam[2],\n"
        "                 clamp((t-offsetsParam[1])/(offsetsParam[2]-offsetsParam[1]),\n"
        "                       0, 1));\n"
        "    result = mix(result, colorsParam[3],\n"
        "                 clamp((t-offsetsParam[2])/(offsetsParam[3]-offsetsParam[2]),\n"
        "                 0, 1));\n"
        "    return half4(result);\n"
        "}\n";

//--------------------------------------------------------------------------------------------------
static constexpr int kNumSolidShaderUniforms = 1;
static constexpr SkUniform kSolidShaderUniforms[kNumSolidShaderUniforms] = {
        { "color", SkSLType::kFloat4 }
};

static const char* kSolidShaderName = "solid_shader";
static const char* kSolidShaderSkSL =
        "half4 solid_shader(float4 colorParam) {\n"
        "    return half4(colorParam);\n"
        "}\n";

//--------------------------------------------------------------------------------------------------
static constexpr int kNumImageShaderUniforms = 0;

static const char* kImageShaderName = "image_shader";
static const char* kImageShaderSkSL =
        "half4 image_shader() {\n"
        "    float c = fract(abs(sk_FragCoord.x/10.0));\n"
        "    return half4(c, c, c, 1.0);\n"
        "}\n";

//--------------------------------------------------------------------------------------------------
static constexpr int kNumBlendShaderUniforms = 0;
static constexpr int kNumBlendShaderChildren = 2;

// Note: we're counting on the compiler to inline this code and trim it down to just the used
// branch(es).
static const char* kBlendShaderName = "blend_shader";
static const char* kBlendShaderSkSL =
        "const int kClear    = 0;\n"
        "const int kSrc      = 1;\n"
        "const int kDst      = 2;\n"
        "const int kSrcOver  = 3;\n"
        "const int kDstOver  = 4;\n"
        "const int kSrcIn    = 5;\n"
        "const int kDstIn    = 6;\n"
        "const int kSrcOut   = 7;\n"
        "const int kDstOut   = 8;\n"
        "const int kSrcATop  = 9;\n"
        "const int kDstATop  = 10;\n"
        "const int kXor      = 11;\n"
        "const int kPlus     = 12;\n"
        "const int kModulate = 13;\n"
        "const int kScreen   = 14;\n"
        "half4 blend_shader(half4 src, half4 dst, int mode) {\n"
        "    switch (mode) {\n"
        "        case kClear: return half4(0.0, 0.0, 0.0, 0.0);\n"
        "        case kSrc: return src;\n"
        "        case kDst: return dst;\n"
        "        case kSrcOver: return src + (1.0-src.a) * dst;\n"
        "        case kDstOver: return dst + (1.0-dst.a) * src;\n"
        "        case kSrcIn: return src * dst.a;\n"
        "        case kDstIn: return dst * src.a;\n"
        "        case kSrcOut: return src * (1.0 - dst.a);\n"
        "        case kDstOut: return dst * (1.0 - src.a);\n"
        "        case kSrcATop: return src * dst.a + dst * (1.0 - src.a);\n"
        "        case kDstATop: return dst * src.a + src * (1.0 - dst.a);\n"
        "        case kXor: return src * (1.0 - dst.a) + dst * (1.0 - src.a);\n"
        "        case kPlus: return min(src + dst, 1.0);\n"
        "        case kModulate: return src * dst;\n"
        "        case kScreen: return src + dst - src * dst;\n"
        "    }\n"
        "    return half4(0.0, 0.0, 0.0, 1.0);\n"
        "}\n";

std::string GenerateBlendShaderGlueCode(const std::string& resultName,
                                        int entryIndex,
                                        const SkShaderInfo::SnippetEntry& entry,
                                        const std::vector<std::string>& childNames,
                                        int indent) {
    SkASSERT(childNames.size() == kNumBlendShaderChildren);

    std::string result;

    add_indent(&result, indent);
    // TODO: actually feed in the blend mode either through a uniform or, somehow, from the
    // SkPaintParamsKey
    SkSL::String::appendf(&result, "%s = blend_shader(%s, %s, kModulate);\n",
                          resultName.c_str(),
                          childNames[1].c_str(),
                          childNames[0].c_str());

    return result;
}

//--------------------------------------------------------------------------------------------------
static constexpr int kNumErrorUniforms = 0;
static const char* kErrorName = "error";
static const char* kErrorSkSL =
        "half4 error() {\n"
        "    return half4(1.0, 0.0, 1.0, 1.0);\n"
        "}\n";

} // anonymous namespace

static constexpr int kNoChildren = 0;

SkShaderCodeDictionary::SkShaderCodeDictionary() {
    // The 0th index is reserved as invalid
    fEntryVector.push_back(nullptr);

    fCodeSnippets[(int) SkBuiltInCodeSnippetID::kDepthStencilOnlyDraw] = {
            { nullptr, kNumErrorUniforms },
            kErrorName, kErrorSkSL,
            GenerateDefaultGlueCode,
            kNoChildren
    };
    fCodeSnippets[(int) SkBuiltInCodeSnippetID::kSolidColorShader] = {
            SkMakeSpan(kSolidShaderUniforms, kNumSolidShaderUniforms),
            kSolidShaderName, kSolidShaderSkSL,
            GenerateDefaultGlueCode,
            kNoChildren
    };
    fCodeSnippets[(int) SkBuiltInCodeSnippetID::kLinearGradientShader] = {
            SkMakeSpan(kGradientUniforms, kNumGradientUniforms),
            kLinearGradient4Name, kLinearGradient4SkSL,
            GenerateDefaultGlueCode,
            kNoChildren
    };
    fCodeSnippets[(int) SkBuiltInCodeSnippetID::kRadialGradientShader] = {
            SkMakeSpan(kGradientUniforms, kNumGradientUniforms),
            kLinearGradient4Name, kLinearGradient4SkSL,
            GenerateDefaultGlueCode,
            kNoChildren
    };
    fCodeSnippets[(int) SkBuiltInCodeSnippetID::kSweepGradientShader] = {
            SkMakeSpan(kGradientUniforms, kNumGradientUniforms),
            kLinearGradient4Name, kLinearGradient4SkSL,
            GenerateDefaultGlueCode,
            kNoChildren
    };
    fCodeSnippets[(int) SkBuiltInCodeSnippetID::kConicalGradientShader] = {
            SkMakeSpan(kGradientUniforms, kNumGradientUniforms),
            kLinearGradient4Name, kLinearGradient4SkSL,
            GenerateDefaultGlueCode,
            kNoChildren
    };
    fCodeSnippets[(int) SkBuiltInCodeSnippetID::kImageShader] = {
            { nullptr, kNumImageShaderUniforms },
            kImageShaderName, kImageShaderSkSL,
            GenerateDefaultGlueCode,
            kNoChildren
    };
    fCodeSnippets[(int) SkBuiltInCodeSnippetID::kBlendShader] = {
            { nullptr, kNumBlendShaderUniforms },
            kBlendShaderName, kBlendShaderSkSL,
            GenerateBlendShaderGlueCode,
            kNumBlendShaderChildren
    };
    fCodeSnippets[(int) SkBuiltInCodeSnippetID::kSimpleBlendMode] = {
            { nullptr, kNumErrorUniforms },
            kErrorName, kErrorSkSL,
            GenerateDefaultGlueCode,
            kNoChildren
    };
}
