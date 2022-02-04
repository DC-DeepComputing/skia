/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkShaderCodeDictionary.h"

#include "include/private/SkSLString.h"
#include "src/core/SkOpts.h"

// TODO: SkShaderInfo::flatten needs to work outside of both just graphite and metal. To do
// so we'll need to switch over to using SkSL's uniform capabilities.
#if defined(SK_GRAPHITE_ENABLED) && defined(SK_METAL)

// TODO: switch this over to using SkSL's uniform system
namespace skgpu::mtl {
std::string GetMtlUniforms(int bufferID,
                           const char* name,
                           const std::vector<SkSpan<const SkUniform>>& uniforms);
}

std::string SkShaderInfo::flatten() const {
    std::string result = skgpu::mtl::GetMtlUniforms(2, "FS", fUniforms);

    for (auto c : fPreamble) {
        result += c;
    }

    result += "layout(location = 0, index = 0) out half4 sk_FragColor;\n";
    result += "void main() {\n";

    for (size_t i = 0; i < fNames.size(); ++i) {
        SkSL::String::appendf(&result, "half4 outColor%d = %s();\n", (int) i, fNames[i]);
    }
    SkSL::String::appendf(&result, "sk_FragColor = outColor%d;\n", (int) fNames.size()-1);
    result += "}\n";

    return result;
}
#endif

//--------------------------------------------------------------------------------------------------
SkShaderCodeDictionary::SkShaderCodeDictionary() {
    // The 0th index is reserved as invalid
    fEntryVector.push_back(nullptr);
}

SkShaderCodeDictionary::Entry* SkShaderCodeDictionary::makeEntry(const SkPaintParamsKey& key) {
    return fArena.make([&](void *ptr) { return new(ptr) Entry(key); });
}

size_t SkShaderCodeDictionary::Hash::operator()(const SkPaintParamsKey& key) const {
    return SkOpts::hash_fn(key.data(), key.sizeInBytes(), 0);
}

const SkShaderCodeDictionary::Entry* SkShaderCodeDictionary::findOrCreate(const SkPaintParamsKey& key) {
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

void SkShaderCodeDictionary::getShaderInfo(SkUniquePaintParamsID uniqueID, SkShaderInfo* info) {
    auto entry = this->lookup(uniqueID);

    entry->paintParamsKey().toShaderInfo(info);
}
