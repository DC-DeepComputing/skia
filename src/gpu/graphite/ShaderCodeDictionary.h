/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ShaderCodeDictionary_DEFINED
#define skgpu_graphite_ShaderCodeDictionary_DEFINED

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkSpinlock.h"
#include "include/private/base/SkMacros.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkThreadAnnotations.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkEnumBitMask.h"
#include "src/core/SkTHash.h"
#include "src/gpu/graphite/BuiltInCodeSnippetID.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/Uniform.h"
#include "src/gpu/graphite/UniquePaintParamsID.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

class SkRuntimeEffect;

namespace skgpu {
class Swizzle;
}

namespace skgpu::graphite {

class RenderStep;
class RuntimeEffectDictionary;

struct ResourceBindingRequirements;

// TODO: How to represent the type (e.g., 2D) of texture being sampled?
class TextureAndSampler {
public:
    constexpr TextureAndSampler(const char* name) : fName(name) {}

    const char* name() const { return fName; }

private:
    const char* fName;
};

enum class SnippetRequirementFlags : uint32_t {
    // kNone = 0x0,
    // kLocalCoords = 0x1,
    // kPriorStageOutput = 0x2,  // AKA the "input" color, or the "source" color for a blender
    // kRuntimeShaderDstColor = 0x4,
    // kBlendAgainstPrimitiveColor = 0x8,

    // FIXME I think we need prior stage output still for compose operations..
    kNone,
    kLocalCoords,
    kDstColor,
    kPrimitiveColor,
    kPaintColor, // FIXME or is this just solidcolor block?
};
SK_MAKE_BITMASK_OPS(SnippetRequirementFlags);

class ShaderNode;

// ShaderSnippets define the "ABI" of a SkSL module function and its required uniform data, as
// well as functions for generating the invoking SkSL. Snippets are composed into an effect tree
// using ShaderNodes.
struct ShaderSnippet {
    using GeneratePreambleForSnippetFn = std::string (*)(const ShaderInfo& shaderInfo,
                                                         const ShaderNode* node);

    // FIXME hide?
    struct Args {
        std::string_view fPriorStageOutput;
        std::string_view fRuntimeShaderDstColor;
        std::string_view fFragCoord;
    };

    ShaderSnippet() = default;

    ShaderSnippet(const char* functionName,
                  SkSpan<const Uniform> uniforms,
                  SkSpan<const TextureAndSampler> texturesAndSamplers,
                  SkEnumBitMask<SnippetRequirementFlags> snippetRequirementFlags,
                  int numChildren = 0,
                  GeneratePreambleForSnippetFn preambleGenerator = nullptr)
        : fFunctionName(functionName)
        , fUniforms(uniforms)
        , fSnippetRequirementFlags(snippetRequirementFlags)
        , fTexturesAndSamplers(texturesAndSamplers)
        , fPreambleGenerator(preambleGenerator)
        , fNumChildren(numChildren) {}

    const char* fFunctionName = nullptr;
    SkSpan<const Uniform> fUniforms;
    SkEnumBitMask<SnippetRequirementFlags> fSnippetRequirementFlags{SnippetRequirementFlags::kNone};
    SkSpan<const TextureAndSampler> fTexturesAndSamplers;
    GeneratePreambleForSnippetFn fPreambleGenerator = nullptr;
    int fNumChildren = 0;
};

// ShaderNodes organize snippets into an effect tree, and provide random access to the dynamically
// bound child snippets. Each node has a fixed number of children defined by its code ID
// (either a BuiltInCodeSnippetID or a runtime effect's assigned ID). All children are non-null.
// A ShaderNode tree represents a decompressed PaintParamsKey.
class ShaderNode {
public:
    // ShaderNodes should be created in conjunction with an SkArenaAlloc that owns all nodes.
    ShaderNode(const ShaderSnippet* snippet,
               SkSpan<const ShaderNode*> children,
               int codeID,
               int keyIndex)
            : fEntry(snippet)
            , fChildren(children)
            , fCodeID(codeID)
            , fKeyIndex(keyIndex)
            , fRequiredFlags(snippet->fSnippetRequirementFlags) {
        SkASSERT(children.size() == fEntry->fNumChildren);
        for (const ShaderNode* child : children) {
            fRequiredFlags |= child->entry()->fSnippetRequirementFlags;
        }
    }

    int32_t codeSnippetId() const { return fCodeID; }
    int32_t keyIndex() const { return fKeyIndex; }
    const ShaderSnippet* entry() const { return fEntry; }

    SkEnumBitMask<SnippetRequirementFlags> requiredFlags() const { return fRequiredFlags; }

    int numChildren() const { return fEntry->fNumChildren; }
    SkSpan<const ShaderNode*> children() const { return fChildren; }
    const ShaderNode* child(int childIndex) const { return fChildren[childIndex]; }

private:
    const ShaderSnippet* fEntry;
    SkSpan<const ShaderNode*> fChildren;

    int32_t fCodeID;
    int32_t fKeyIndex; // index back to PaintParamsKey, unique across nodes within a ShaderInfo

    SkEnumBitMask<SnippetRequirementFlags> fRequiredFlags;
};

// ShaderInfo holds all root ShaderNodes defined for a PaintParams as well as the extracted fixed
// function blending parameters and other aggregate requirements for the effect trees that have
// been linked into a single fragment program (sans any RenderStep fragment work and fixed SkSL
// logic required for all rendering in Graphite).
class ShaderInfo {
public:
    ShaderInfo(UniquePaintParamsID id,
               const ShaderCodeDictionary* dict,
               const RuntimeEffectDictionary* rteDict,
               const char* ssboIndex);

    bool needsLocalCoords() const {
        return fSnippetRequirementFlags & SnippetRequirementFlags::kLocalCoords;
    }

    const RuntimeEffectDictionary* runtimeEffectDictionary() const {
        return fRuntimeEffectDictionary;
    }
    const char* ssboIndex() const { return fSsboIndex; }

    void setBlendInfo(const skgpu::BlendInfo& blendInfo) {
        fBlendInfo = blendInfo;
    }
    const skgpu::BlendInfo& blendInfo() const { return fBlendInfo; }

    std::string toSkSL(const ResourceBindingRequirements& bindingReqs,
                       const RenderStep* step,
                       const bool useStorageBuffers,
                       const bool defineLocalCoordsVarying,
                       int* numTexturesAndSamplersUsed,
                       Swizzle writeSwizzle) const;

private:
    // All shader nodes and arrays of children pointers are held in this arena
    SkArenaAlloc fShaderNodeAlloc{256};

    const RuntimeEffectDictionary* fRuntimeEffectDictionary;
    const char* fSsboIndex;

    // De-compressed shader tree from a PaintParamsKey with accumulated blend info and requirements.
    // The blendInfo doesn't contribute to the program's SkSL but contains the fixed-function state
    // required to function correctly, which the program's caller is responsible for configuring.
    // TODO: There should really only be one root node representing the final blend, which has a
    // child defining how the src color is calculated.
    SkSpan<const ShaderNode*> fRootNodes;
    skgpu::BlendInfo fBlendInfo;
    SkEnumBitMask<SnippetRequirementFlags> fSnippetRequirementFlags
};

// ShaderCodeDictionary is a thread-safe dictionary of ShaderSnippets to code IDs for use with
// creating PaintParamKeys, as well as assigning unique IDs to each encountered PaintParamKey.
// It defines ShaderSnippets for every BuiltInCodeSnippetID and maintains records for IDs per
// SkRuntimeEffect, including de-duplicating equivalent SkRuntimeEffect objects.
class ShaderCodeDictionary {
public:
    ShaderCodeDictionary();

    struct Entry {
    public:
        UniquePaintParamsID uniqueID() const {
            SkASSERT(fUniqueID.isValid());
            return fUniqueID;
        }
        const PaintParamsKey& paintParamsKey() const { return fKey; }

    private:
        friend class ShaderCodeDictionary;

        Entry(const PaintParamsKey& key, uint32_t newID) : fUniqueID(newID), fKey(key.asSpan()) {}

        UniquePaintParamsID fUniqueID;  // fixed-size (uint32_t) unique ID assigned to a key
        PaintParamsKey fKey; // variable-length paint key descriptor
    };

    const Entry* findOrCreate(PaintParamsKeyBuilder*) SK_EXCLUDES(fSpinLock);

    const Entry* lookup(UniquePaintParamsID) const SK_EXCLUDES(fSpinLock);

    SkSpan<const Uniform> getUniforms(BuiltInCodeSnippetID) const;
    SkEnumBitMask<SnippetRequirementFlags> getSnippetRequirementFlags(
            BuiltInCodeSnippetID id) const {
        return fBuiltInCodeSnippets[(int) id].fSnippetRequirementFlags;
    }

    bool isValidID(int snippetID) const;

    // This method can return nullptr
    const ShaderSnippet* getEntry(int codeSnippetID) const;
    const ShaderSnippet* getEntry(BuiltInCodeSnippetID codeSnippetID) const {
        return this->getEntry(SkTo<int>(codeSnippetID));
    }

    // void getShaderInfo(UniquePaintParamsID, ShaderInfo*) const;

    int findOrCreateRuntimeEffectSnippet(const SkRuntimeEffect* effect);

    // TODO: Remove or make testing-only
    int addUserDefinedSnippet(const char* name);

private:
    // TODO: this is still experimental but, most likely, it will need to be made thread-safe
    // It returns the code snippet ID to use to identify the supplied user-defined code.
    // TODO: Rename to addRuntimeEffectSnippet().
    int addUserDefinedSnippet(
        const char* functionName,
        SkSpan<const Uniform> uniforms,
        SkSpan<const TextureAndSampler> texturesAndSamplers,
        SkEnumBitMask<SnippetRequirementFlags> snippetRequirementFlags,
        ShaderSnippet::GeneratePreambleForSnippetFn preambleGenerator,
        int numChildren);

    const char* addTextToArena(std::string_view text);

    SkSpan<const Uniform> convertUniforms(const SkRuntimeEffect* effect);

    std::array<ShaderSnippet, kBuiltInCodeSnippetIDCount> fBuiltInCodeSnippets;

    // The value returned from 'getEntry' must be stable so, hold the user-defined code snippet
    // entries as pointers.
    skia_private::TArray<std::unique_ptr<ShaderSnippet>> fUserDefinedCodeSnippets;

    // TODO: can we do something better given this should have write-seldom/read-often behavior?
    mutable SkSpinlock fSpinLock;

    struct PaintParamsKeyPtr {
        const PaintParamsKey* fKey;

        bool operator==(PaintParamsKeyPtr rhs) const {
            return *fKey == *rhs.fKey;
        }
        struct Hash {
            size_t operator()(PaintParamsKeyPtr) const;
        };
    };

    using PaintHashMap = SkTHashMap<PaintParamsKeyPtr, Entry*, PaintParamsKeyPtr::Hash>;

    PaintHashMap fHash SK_GUARDED_BY(fSpinLock);
    skia_private::TArray<Entry*> fEntryVector SK_GUARDED_BY(fSpinLock);

    SK_BEGIN_REQUIRE_DENSE
    struct RuntimeEffectKey {
        uint32_t fHash;
        uint32_t fUniformSize;

        bool operator==(RuntimeEffectKey rhs) const {
            return fHash == rhs.fHash && fUniformSize == rhs.fUniformSize;
        }
        struct Hash {
            size_t operator()(RuntimeEffectKey) const;
        };
    };
    SK_END_REQUIRE_DENSE

    // A map from RuntimeEffectKeys (hash plus uniforms) to code-snippet IDs. RuntimeEffectKeys
    // don't track the lifetime of a runtime effect at all; they live forever, and a newly-
    // instantiated runtime effect with the same program as a previously-discarded effect will reuse
    // an existing ID. Entries in the runtime-effect map are never removed; they only disappear when
    // the context is discarded, which takes the ShaderCodeDictionary along with it. However, they
    // are extremely small (< 20 bytes) so the memory footprint should be unnoticeable.
    using RuntimeEffectMap = SkTHashMap<RuntimeEffectKey, int32_t>;
    RuntimeEffectMap fRuntimeEffectMap SK_GUARDED_BY(fSpinLock);

    // This arena holds:
    //   - the Entries held in `fHash` and `fEntryVector`
    //   - Uniform data created by `findOrCreateRuntimeEffectSnippet`
    // and in all cases is guarded by `fSpinLock`
    SkArenaAlloc fArena{256};
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ShaderCodeDictionary_DEFINED
