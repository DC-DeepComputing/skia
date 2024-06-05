/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PaintParamsKey.h"

#include "src/base/SkArenaAlloc.h"
#include "src/base/SkStringView.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"

using namespace skia_private;

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
// PaintParamsKeyBuilder

#ifdef SK_DEBUG

void PaintParamsKeyBuilder::checkReset() {
    SkASSERT(!fLocked);
    SkASSERT(fData.empty());
    SkASSERT(fStack.empty());
}

void PaintParamsKeyBuilder::pushStack(uint32_t codeSnippetID) {
    SkASSERT(fDict->isValidID(codeSnippetID));

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

void PaintParamsKeyBuilder::addImmutableSamplerDataIfApplicable(const Caps* caps,
                                                                const SkSamplingOptions& sampling,
                                                                const SkTileMode tileModes[2],
                                                                const TextureProxy* proxy) {
    if (!proxy || !caps) {
        return;
    }
    ImmutableSamplerInfo info = caps->getImmutableSamplerInfo(proxy);

    if (!(info.fNonFormatYcbcrConversionInfo || info.fFormat)) {
        // Means we have a regular dynamic sampler for which no data needs to be appended.
        // Call addData() regardless w/ an empty span such that a data legnth of '0' is added.
        this->addData({});
        return;
    }

    SamplerDesc desc{sampling, tileModes, info};
    SkASSERT(desc.isImmutable());

    skia_private::TArray<uint32_t> samplerData;
    samplerData.push_back(desc.desc());
    if (desc.usesExternalFormat()) {
        samplerData.push_back(desc.externalFormatMSBs());
    }
    samplerData.push_back(desc.format());

    this->addData({samplerData});
}

//--------------------------------------------------------------------------------------------------
// PaintParamsKey

PaintParamsKey PaintParamsKey::clone(SkArenaAlloc* arena) const {
    uint32_t* newData = arena->makeArrayDefault<uint32_t>(fData.size());
    memcpy(newData, fData.data(), fData.size_bytes());
    return PaintParamsKey({newData, fData.size()});
}


const ShaderNode* PaintParamsKey::createNode(const ShaderCodeDictionary* dict,
                                             int* currentIndex,
                                             SkArenaAlloc* arena) const {
    SkASSERT(*currentIndex < SkTo<int>(fData.size()));
    const int32_t index = (*currentIndex)++;
    const uint32_t id = fData[index];

    const ShaderSnippet* entry = dict->getEntry(id);
    if (!entry) {
        SKGPU_LOG_E("Unknown snippet ID in key: %d", id);
        return nullptr;
    }

    if (index == 0) {
        SkDebugf("data: ");
        for (const auto dataMember : fData) {
            SkDebugf(" %d ", dataMember);
        }
        SkDebugf("\n");
    }


    SkSpan<const uint32_t> dataSpan = {};
    if (entry->fSnippetRequirementFlags & SnippetRequirementFlags::kStoresData) {
        // Gather any additional sampler data that should be passed into ShaderNode creation. If the
        // next entry is 0, that indicates there is no immutable sampler data to store.
        const int storedDataLengthIdx = index + 1;
        if (storedDataLengthIdx < SkTo<int>(fData.size()) && fData[storedDataLengthIdx]) {
            dataSpan = fData.subspan(storedDataLengthIdx + 1, fData[storedDataLengthIdx]);
            // Iterate past the data length index, the data length itself, and one more to account
            // for the fact that we are checking the index prior to fIdxOfStoredSamplerData.
            *currentIndex = index + fData[storedDataLengthIdx] + 2;
        } else {
            // Iterate past data length index of 0.
            *currentIndex += 1;
        }
    }

    const ShaderNode** childArray = arena->makeArray<const ShaderNode*>(entry->fNumChildren);
    for (int i = 0; i < entry->fNumChildren; ++i) {
        const ShaderNode* child = this->createNode(dict, currentIndex, arena);
        if (!child) {
            return nullptr;
        }
        childArray[i] = child;
    }

    return arena->make<ShaderNode>(entry,
                                   SkSpan(childArray, entry->fNumChildren),
                                   id,
                                   index,
                                   dataSpan);
}

SkSpan<const ShaderNode*> PaintParamsKey::getRootNodes(const ShaderCodeDictionary* dict,
                                                       SkArenaAlloc* arena) const {
    // TODO: Once the PaintParamsKey creation is organized to represent a single tree starting at
    // the final blend, there will only be a single root node and this can be simplified.
    // For now, we don't know how many roots there are, so collect them into a local array before
    // copying into the arena.
    const int keySize = SkTo<int>(fData.size());

    // Normal PaintParams creation will have up to 7 roots for the different stages.
    STArray<7, const ShaderNode*> roots;
    int currentIndex = 0;
    while (currentIndex < keySize) {
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

static int key_to_string(SkString* str,
                         const ShaderCodeDictionary* dict,
                         SkSpan<const uint32_t> keyData,
                         int currentIndex) {
    SkASSERT(currentIndex < SkTo<int>(keyData.size()));

    uint32_t id = keyData[currentIndex++];
    auto entry = dict->getEntry(id);
    if (!entry) {
        str->append("UnknownCodeSnippetID:");
        str->appendS32(id);
        str->append(" ");
        return currentIndex;
    }

    std::string_view name = entry->fName;
    if (skstd::ends_with(name, "Shader")) {
        name.remove_suffix(6);
    }
    str->append(name);

    if (entry->fNumChildren > 0) {
        str->append(" [ ");
        for (int i = 0; i < entry->fNumChildren; ++i) {
            currentIndex = key_to_string(str, dict, keyData, currentIndex);
        }
        str->append("]");
    }

    str->append(" ");
    return currentIndex;
}

SkString PaintParamsKey::toString(const ShaderCodeDictionary* dict) const {
    SkString str;
    const int keySize = SkTo<int>(fData.size());
    for (int currentIndex = 0; currentIndex < keySize;) {
        currentIndex = key_to_string(&str, dict, fData, currentIndex);
    }
    return str.isEmpty() ? SkString("(empty)") : str;
}

#ifdef SK_DEBUG

static int dump_node(const ShaderCodeDictionary* dict,
                     SkSpan<const uint32_t> keyData,
                     int currentIndex,
                     int indent) {
    SkASSERT(currentIndex < SkTo<int>(keyData.size()));

    SkDebugf("%*c", 2 * indent, ' ');

    uint32_t id = keyData[currentIndex++];
    auto entry = dict->getEntry(id);
    if (!entry) {
        SkDebugf("[%d] unknown block!\n", id);
        return currentIndex;
    }

    SkDebugf("[%d] %s\n", id, entry->fStaticFunctionName);
    for (int i = 0; i < entry->fNumChildren; ++i) {
        currentIndex = dump_node(dict, keyData, currentIndex, indent + 1);
    }
    return currentIndex;
}

void PaintParamsKey::dump(const ShaderCodeDictionary* dict, UniquePaintParamsID id) const {
    const int keySize = SkTo<int>(fData.size());

    SkDebugf("--------------------------------------\n");
    SkDebugf("%u PaintParamsKey (keySize: %d):\n", id.asUInt(), keySize);

    int currentIndex = 0;
    while (currentIndex < keySize) {
        currentIndex = dump_node(dict, fData, currentIndex, 1);
    }
}

#endif // SK_DEBUG

} // namespace skgpu::graphite
