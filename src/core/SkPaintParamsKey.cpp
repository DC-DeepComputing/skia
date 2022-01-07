/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkPaintParamsKey.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/Encoding.h"
#include "include/core/SkPaint.h"

#include "src/shaders/SkComposeShader.h"

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
uint32_t SkPaintParamsKey::DumpFrom(const uint32_t* data) {
    auto[id, numIntsOfData] = ReadCodeSnippetID(data[0]);

    SkDebugf("%s %d: ", gNames[(int) id], numIntsOfData);
    (gDumpers[(int) id])(data, numIntsOfData+1);
    SkDebugf("\n");

    return numIntsOfData + 1;
}

// This just iterates over the top-level YYY codes calling DumpFrom.
void SkPaintParamsKey::dump() const {
    SkDebugf("Combo %d: ", this->numInts());

    const uint32_t* data = static_cast<const uint32_t *>(this->data());
    uint32_t curIndex = 0;
    while (curIndex < this->numInts()) {

        uint32_t numIntsConsumed = DumpFrom(&data[curIndex]);
        SkASSERT(curIndex + numIntsConsumed <= this->numInts());

        curIndex += numIntsConsumed;
    }
}

uint32_t SkPaintParamsKey::ToSkSLFrom(const uint32_t* data, SkSL::String* result) {
    auto[id, numIntsOfData] = ReadCodeSnippetID(data[0]);

    result->appendf("%s %d\n", gNames[(int) id], numIntsOfData);

    return numIntsOfData + 1;
}

SkSL::String SkPaintParamsKey::toSkSL() const {
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
