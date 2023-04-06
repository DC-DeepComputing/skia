/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PaintParamsKey.h"

#include <cstring>
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
// PaintParamsKeyBuilder

#ifdef SK_DEBUG

void PaintParamsKeyBuilder::checkReset() {
    SkASSERT(!fLocked);
    SkASSERT(fData.empty());
    SkASSERT(fStack.empty());
}

void PaintParamsKeyBuilder::pushStack(int32_t codeSnippetID) {
    if (!fStack.empty()) {
        fStack.back().fNumActualChildren++;
        SkASSERT(fStack.back().fNumActualChildren <= fStack.back().fNumExpectedChildren);
    }

    const ShaderSnippet* snippet = fDict->getEntry(codeSnippetID);
    fStack.push_back({codeSnippetID, snippet->fNumChildren});
}

void PaintParamsKeyBuilder::popStack() {
    SkASSERT(!fStack.empty());
    SkASSERT(fStack.back().fNumActualChildren == fStack.back().fNumExpectedChildren);
    fStack.pop_back();
}

#endif // SK_DEBUG

//--------------------------------------------------------------------------------------------------
// PaintParamsKey

// FIXME needed? move to header?
bool PaintParamsKey::operator==(const PaintParamsKey& that) const {
    return fData.size() == that.fData.size() &&
           !memcmp(fData.data(), that.fData.data(), fData.size());
}

PaintParamsKey PaintParamsKey::clone(SkArenaAlloc* arena) const {
    int32_t* newData = arena->makeArrayDefault<int32_t>(fData.size());
    memcpy(newData, fData.data(), fData.size_bytes());
    return PaintParamsKey({newData, fData.size()});
}

const ShaderNode* PaintParamsKey::createNode(const ShaderCodeDictionary* dict,
                                             int* currentIndex,
                                             SkArenaAlloc* arena) const {
    SkASSERT(*currentIndex < fData.size());
    int32_t index = (*currentIndex)++;
    int32_t id = fData[index];

    auto entry = dict->getEntry(id);
    if (!entry) {
        SKGPU_LOG_E("Unknown snippet ID in key: %d", id);
        return nullptr;
    }

    const ShaderNode** childArray = arena->makeArray<const ShaderNode*>(entry->fNumChildren);
    for (int i = 0; i < entry->fNumChildren; ++i) {
        const ShaderNode* child = this->createNode(dict, currentIndex, arena);
        if (!child) {
            return nullptr;
        }
        childArray[i] = child;
    }

    return arena->make<ShaderNode>(entry, SkSpan(childArray, entry->fNumChildren), id, index);
}

SkSpan<const ShaderNode*> PaintParamsKey::getRootNodes(const ShaderCodeDictionary* dict,
                                                       SkArenaAlloc* arena) const {
    // TODO: Once the PaintParamsKey creation is organized to represent a single tree starting at
    // the final blend, there will only be a single root node and this can be simplified.
    // For now, we don't know how many roots there are, so collect them into a local vector before
    // copying into the arena.
    SkTDArray<const ShaderNode*> roots;
    int currentIndex = 0;
    while (currentIndex < fData.size()) {
        const ShaderNode* root = this->createNode(dict, &currentIndex, arena);
        if (!root) {
            return {}; // a bad key
        }
        roots.push_back(root);
    }

    // Copy the accumulated roots into a span stored in the arena
    const ShaderNode** rootSpan = arena->makeArray<const ShaderNode*>(roots.size());
    memcpy(rootSpan, roots.data(), roots.size_bytes());
    return SkSpan(rootSpan, roots.size());
}

#ifdef SK_DEBUG

static void output_indent(int indent) {
    SkDebugf("%*c", 2 * indent, ' ');
}

int dump_block(const ShaderCodeDictionary* dict,
               SkSpan<const int32_t> keyData,
               int currentIndex,
               int indent) {
    SkASSERT(currentIndex < keyData.size());
    output_indent(indent);
    int32_t id = keyData[currentIndex++];

    auto entry = dict->getEntry(id);
    if (!entry) {
        SkDebugf("[%d] unknown block!\n", id);
        return currentIndex;
    }

    SkDebugf("[%d] %s\n", id, entry->fStaticFunctionName);
    for (int i = 0; i < entry->fNumChildren; ++i) {
        currentIndex = dump_block(dict, keyData, currentIndex, indent + 1);
    }
    return currentIndex;
}

void PaintParamsKey::dump(const ShaderCodeDictionary* dict) const {
    SkDebugf("--------------------------------------\n");
    SkDebugf("PaintParamsKey (%dB):\n", fData.size_bytes());

    int currentIndex = 0;
    while (currentIndex < fData.size()) {
        currentIndex = dump_block(dict, fData, currentIndex, 1);
    }
}

#endif // SK_DEBUG

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

static void add_block_to_shader_info(const ShaderCodeDictionary* dict,
                                     const PaintParamsKey::BlockReader& reader,
                                     ShaderInfo* result) {
    result->add(reader);
    result->addFlags(dict->getEntry(reader.codeSnippetId())->fSnippetRequirementFlags);

    if (reader.codeSnippetId() < kBuiltInCodeSnippetIDCount &&
        reader.codeSnippetId() >= kFixedFunctionBlendModeIDOffset) {
        SkASSERT(reader.numChildren() == 0);

        // Set SkBlendInfo on the ShaderInfo if the block represents a fixed function blend. The
        // key and paint are defined in such a way that this can occur at most once.
        int coeffBlendMode = reader.codeSnippetId() - kFixedFunctionBlendModeIDOffset;
        SkASSERT(coeffBlendMode >= 0 &&
                 static_cast<SkBlendMode>(coeffBlendMode) <= SkBlendMode::kLastCoeffMode);
        result->setBlendInfo(gBlendTable[coeffBlendMode]);
    } else {
        // The child blocks appear right after the parent block's header in the key and go
        // right after the parent's SnippetEntry in the shader info
        for (int i = 0; i < reader.numChildren(); ++i) {
            add_block_to_shader_info(dict, reader.child(dict, i), result);
        }
    }
}

} // namespace skgpu::graphite
