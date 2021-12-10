/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkCode.h"

#include "include/core/SkPaint.h"
#include "experimental/graphite/include/Context.h"

void SkCode::dump() const {
    SkASSERT(this->numInts() == 2);

    SkDebugf("Combo: ");
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
}

SkMagicCodeDictionary gDict;

std::string SkCode::toSkSL() const {



    return "";
}

void foo(const SkPaint& p) {

    std::unique_ptr<SkMagicCodeDictionary::FullCodeEntry> entry(new SkMagicCodeDictionary::FullCodeEntry);
    p.toCode(&gDict, Mapping::kGanesh, &entry->fCode);
    entry->fSkSL = entry->fCode.toSkSL();

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
