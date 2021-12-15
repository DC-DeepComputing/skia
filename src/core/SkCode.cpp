/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkCode.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/Encoding.h"
#include "include/core/SkPaint.h"

#include "src/shaders/SkComposeShader.h"
#include "src/utils/SkShaderUtils.h"

CodeSnippetID ExtractCodeSnippetID(uint32_t code) {
    code &= kCodeSnippetIDMask;
    SkASSERT(code < kCodeSnippetIDCount);
    return static_cast<CodeSnippetID>(code);
}

const char* gNames[] = {
        "kNoneShader",
        "kImageShader",
        "kGradientShader",
        "kBlendShader",

        "kBlendMode",
};

typedef void (*Dumper)(const uint32_t* data, int maxIndex);

Dumper gDumpers[] = {
        skgpu::NoShaderCombo::Dump,
        SkImageShader::Dump,
        skgpu::GradientCombinations::Dump,
        SkShader_Blend::Dump,

        BlendMode::Dump,
};

// We're guaranteed that data[0] contains a YYY code and a data size. Dump the data for that
// YYY code and return the number of ints consumed. This entry point may be called recursively.
uint32_t SkCode::DumpFrom(const uint32_t* data) {
    auto[id, numIntsOfData] = ReadCodeSnippetID(data[0]);

    SkDebugf("%s %d: ", gNames[(int) id], numIntsOfData);
    (gDumpers[(int) id])(data, numIntsOfData+1);
    SkDebugf("\n");

    return numIntsOfData + 1;
}

// This just iterates over the top-level YYY codes calling DumpFrom.
void SkCode::dump() const {
    SkDebugf("Combo %d: ", this->numInts());

    const uint32_t* data = static_cast<const uint32_t *>(this->data());
    uint32_t curIndex = 0;
    while (curIndex < this->numInts()) {

        uint32_t numIntsConsumed = DumpFrom(&data[curIndex]);
        SkASSERT(curIndex + numIntsConsumed <= this->numInts());

        curIndex += numIntsConsumed;
    }
}

SkMagicCodeDictionary gDict;

uint32_t SkCode::ToSkSLFrom(const uint32_t* data, SkSL::String* result) {
    auto[id, numIntsOfData] = ReadCodeSnippetID(data[0]);

    result->appendf("%s %d\n", gNames[(int) id], numIntsOfData);

    return numIntsOfData + 1;
}

SkSL::String SkCode::toSkSL() const {
    SkSL::String result;

    const uint32_t* data = static_cast<const uint32_t *>(this->data());
    uint32_t curIndex = 0;
    while (curIndex < this->numInts()) {

        uint32_t numIntsConsumed = ToSkSLFrom(&data[curIndex], &result);
        SkASSERT(curIndex + numIntsConsumed <= this->numInts());

        curIndex += numIntsConsumed;
    }

    return result;
}

void AddCodeToDictionary(const SkPaint& p) {

    std::unique_ptr<SkMagicCodeDictionary::FullCodeEntry> entry(new SkMagicCodeDictionary::FullCodeEntry);
    p.toCode(&gDict, Mapping::kGanesh, &entry->fCode, nullptr);
    entry->fSkSL = entry->fCode.toSkSL();

    entry->fCode.dump();

    gDict.findOrCreateFull(std::move(entry));

}

void CheckCodeHasBeenSeen(const SkSL::String& code) {
    gDict.exhaustiveSearch(code);
}

#include "SkOpts.h"

size_t SkMagicCodeDictionary::Hash::operator()(SkCode code) const {
    return SkOpts::hash_fn(code.data(), code.sizeInBytes(), 0);
}

const SkMagicCodeDictionary::FullCodeEntry* SkMagicCodeDictionary::findOrCreateFull(
        std::unique_ptr<FullCodeEntry> newEntry) {
    auto iter = fFullCodeHash.find(newEntry->fCode);
    if (iter != fFullCodeHash.end()) {
        return iter->second.get();
    }

    auto iter2 = fFullCodeHash.insert(std::make_pair(newEntry->fCode, std::move(newEntry)));
    return iter2.first->second.get();
}

const SkMagicCodeDictionary::FullCodeEntry* SkMagicCodeDictionary::exhaustiveSearch(const SkSL::String& code) const {

    for (const auto& e : fFullCodeHash) {
        if (code == e.second->fSkSL) {
            return e.second.get();
        }
    }

    SkDebugf("------------------------------\n");
    SkDebugf("Didn't find this code:\n");
    SkShaderUtils::PrintLineByLine(SkShaderUtils::PrettyPrint(code));

    SkDebugf("------------------------------\n");
    SkDebugf("in this %d list:\n", (int) fFullCodeHash.size());

    int index = 0;
    for (const auto& e : fFullCodeHash) {
        SkDebugf("%d ------------------------------\n", index++);
        SkShaderUtils::PrintLineByLine(SkShaderUtils::PrettyPrint(e.second->fSkSL));
    }

    return nullptr;
}
