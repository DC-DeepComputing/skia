/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkCode.h"

#include "experimental/graphite/include/Context.h"
#include "include/core/SkPaint.h"

const char* gNames[] = {
        "kNoneShader",
        "kImageShader",
        "kGradientShader",
        "kBlendShader",

        "kBlendMode",
};

typedef void (*Dumper)(const void* data, int maxIndex);

Dumper gDumpers[] = {
//        skgpu::NoShaderCombo::Dump,
//        skgpu::ImageShaderCombinations::Dump,
        //skgpu::GradientCombinations::Dump,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
};

void SkCode::dump() const {
    SkDebugf("Combo: ");

    uint32_t curIndex = 0;
    while (curIndex <= this->numInts()) {
        auto[foo, numIntsOfData] = this->readFoo(curIndex);

        SkASSERT(curIndex + numIntsOfData < this->numInts());
        const uint32_t* data = static_cast<const uint32_t *>(this->data());
        SkDebugf("%s %d", gNames[(int) foo], numIntsOfData);
        (gDumpers[(int) foo])(&data[curIndex+1], this->numInts()-curIndex-1);
        curIndex += numIntsOfData+1;
    }

#if 0
    int iFoo = fData[0] & skgpu::kShaderCodeMask;
    switch (iFoo) {
        case skgpu::ShaderCode::kNone_ShaderCode:
            skgpu::NoShaderCombo::Dump(*this);
            break;
        case skgpu::ShaderCode::kImage_ShaderCode:
            skgpu::ImageShaderCombinations::Dump(*this);
            break;
        case skgpu::ShaderCode::kGradient_ShaderCode:
            skgpu::GradientCombinations::Dump(*this);
            break;
    }

    SkBlendMode bm = (SkBlendMode) fData[1];
    SkDebugf(" %s\n", skgpu::BMToStr(bm));
#endif
}

SkMagicCodeDictionary gDict;

std::string SkCode::toSkSL() const {
    SkSL::String sksl;

    uint32_t curIndex = 0;
    while (curIndex <= this->numInts()) {
        auto[foo, numIntsOfData] = this->readFoo(curIndex);

        SkASSERT(curIndex + numIntsOfData < this->numInts());
        const uint32_t* data = static_cast<const uint32_t *>(this->data());
        sksl.appendf("%s %d", gNames[(int) foo], numIntsOfData);
//        (gDumpers[(int) foo])(&data[curIndex+1], this->numInts()-curIndex-1);
        curIndex += numIntsOfData+1;
    }

    return sksl;
}

void foo(const SkPaint& p) {

    std::unique_ptr<SkMagicCodeDictionary::FullCodeEntry> entry(new SkMagicCodeDictionary::FullCodeEntry);
    p.toCode(&gDict, Mapping::kGanesh, &entry->fCode);
    entry->fSkSL = entry->fCode.toSkSL();

    entry->fCode.dump();

    gDict.findOrCreateFull(std::move(entry));;

}

#include "SkOpts.h"

size_t SkMagicCodeDictionary::Hash::operator()(SkCode code) const {
    return SkOpts::hash_fn(code.data(), code.sizeInBytes(), 0);
}

SkMagicCodeDictionary::FullCodeEntry* SkMagicCodeDictionary::findOrCreateFull(
        std::unique_ptr<FullCodeEntry> newEntry) {
    auto iter = fFullCodeHash.find(newEntry->fCode);
    if (iter != fFullCodeHash.end()) {
        return iter->second.get();
    }

    auto iter2 = fFullCodeHash.insert(std::make_pair(newEntry->fCode, std::move(newEntry)));
    return iter2.first->second.get();
}
