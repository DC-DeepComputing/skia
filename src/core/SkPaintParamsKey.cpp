/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkPaintParamsKey.h"

#include "experimental/graphite/include/Context.h"
#include "include/core/SkPaint.h"

#include "src/shaders/SkComposeShader.h"

static const char* gCodeSnippetNames[kCodeSnippetIDCount] = {
        "kNoneShader",
        "kLinearGradientShader",
        "kRadialGradientShader",
        "kSweepGradientShader",
        "kConicalGradientShader",

        "kSimpleBlendMode",
};

typedef void (*Dumper)(const uint32_t* data, int maxIndex);

Dumper gDumpers[] = {
        skgpu::NoShaderCombo::Dump,
        SkImageShader::Dump,
        skgpu::GradientCombinations::Dump,
        SkShader_Blend::Dump,

        BlendMode::Dump,
};

// We're guaranteed that data[0] contains a CodeSnippetID and a data size. Dump the data for that
// CodeSnippetID and return the number of ints consumed. This entry point may be called recursively.
uint32_t SkPaintParamsKey::DumpBlock(const uint32_t* data) {
    auto[id, numIntsOfData] = ReadCodeSnippetID(data[0]);

    SkDebugf("%s %d: ", gNames[(int) id], numIntsOfData);
    (gDumpers[(int) id])(data, numIntsOfData+1);
    SkDebugf("\n");

    return numIntsOfData + 1;
}

// This just iterates over the top-level blocks calling DumpBlock.
void SkPaintParamsKey::dump() const {
    SkDebugf("SkPaintParamsKey %d: ", this->numInts());

    const uint32_t* data = static_cast<const uint32_t *>(this->data());
    uint32_t curIndex = 0;
    while (curIndex < this->numInts()) {

        uint32_t numIntsConsumed = DumpBlock(&data[curIndex]);
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
