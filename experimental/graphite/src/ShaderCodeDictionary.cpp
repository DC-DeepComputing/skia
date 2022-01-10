/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ShaderCodeDictionary.h"
#include "experimental/graphite/src/Uniform.h"
#include "include/private/SkSLString.h"
#include "src/core/SkOpts.h"

namespace skgpu {

//--------------------------------------------------------------------------------------------------
SkSL::String ShaderCodeDictionary::Entry::toSkSL(bool *writesColor) const {
    SkSL::String sksl;

    int curHeaderOffset = 0;
    while (curHeaderOffset < this->fPaintParamsKey.sizeInBytes()) {
        auto [codeSnippetID, blockSize] = this->fPaintParamsKey.readCodeSnippetID(curHeaderOffset);

        // Typedefs needed for painting
        auto paintUniforms = GetUniforms(codeSnippetID);
        if (!paintUniforms.empty()) {
            sksl += EmitSKSLUniforms(2, "FS", paintUniforms);
        }

        sksl += "layout(location = 0, index = 0) out half4 sk_FragColor;\n";
        sksl += "void main() {\n"
                "    half4 outColor;\n";
        sksl += GetShaderSkSL(codeSnippetID);
        sksl += "    sk_FragColor = outColor;\n"
                "}\n";

        *writesColor = codeSnippetID != CodeSnippetID::kStencilOnlyDraw;

        curHeaderOffset += blockSize;
    }

    return sksl;
}

//--------------------------------------------------------------------------------------------------
ShaderCodeDictionary::ShaderCodeDictionary() {
    // The 0th index is reserved as invalid
    fEntryVector.push_back(nullptr);
}

ShaderCodeDictionary::Entry* ShaderCodeDictionary::makeEntry(SkPaintParamsKey key) {
    return fArena.make([&](void *ptr) { return new(ptr) Entry(key); });
}

// TODO: const& ??
size_t ShaderCodeDictionary::Hash::operator()(SkPaintParamsKey key) const {
    return SkOpts::hash_fn(key.data(), key.sizeInBytes(), 0);
}

const ShaderCodeDictionary::Entry* ShaderCodeDictionary::findOrCreate(SkPaintParamsKey key) {
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

const ShaderCodeDictionary::Entry* ShaderCodeDictionary::lookup(UniquePaintParamsID codeID) const {
    if (!codeID.isValid()) {
        return nullptr;
    }

    SkAutoSpinlock lock{fSpinLock};

    SkASSERT(codeID.asUInt() < fEntryVector.size());

    return fEntryVector[codeID.asUInt()];
}

} // namespace skgpu
