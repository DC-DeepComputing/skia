/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkPaintParamsKey.h"

#include <cstring>
#include "src/core/SkKeyHelpers.h"

#if 0
#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/Encoding.h"
#include "include/core/SkPaint.h"

#endif

#include "src/shaders/SkComposeShader.h"

bool SkPaintParamsKey::operator==(const SkPaintParamsKey& that) const {
    return fNumBytes == that.fNumBytes &&
           !memcmp(fData.data(), that.fData.data(), fNumBytes);
}

#ifdef SK_DEBUG
typedef void (*Dumper)(const SkPaintParamsKey&, uint32_t headerOffset);

static Dumper gDumpers[kCodeSnippetIDCount] = {
    DepthStencilOnlyBlock::Dump,  // kDepthStencilOnlyDraw

    SolidColorShaderBlock::Dump,  // kSolidColorShader
    GradientShaderBlocks::Dump,   // kLinearGradientShader
    GradientShaderBlocks::Dump,   // kRadialGradientShader
    GradientShaderBlocks::Dump,   // kSweepGradientShader
    GradientShaderBlocks::Dump,   // kConicalGradientShader

    nullptr, // SkImageShader::Dump,
    SkShader_Blend::Dump,

    BlendModeBlock::Dump,         // kSimpleBlendMode
};

int SkPaintParamsKey::DumpBlock(const SkPaintParamsKey& key, int headerOffset) {
    auto [id, blockSize] = key.readCodeSnippetID(headerOffset);

    (gDumpers[(int) id])(key, headerOffset);

    return blockSize;
}

// This just iterates over the top-level blocks calling block-specific dump methods.
void SkPaintParamsKey::dump() const {
    SkDebugf("SkPaintParamsKey %dB:\n", this->sizeInBytes());

    int curHeaderOffset = 0;
    while (curHeaderOffset < this->sizeInBytes()) {
        int blockSize = DumpBlock(*this, curHeaderOffset);

        curHeaderOffset += blockSize;
    }
}
#endif

void SkPaintParamsKey::BlockToSkSL(const SkPaintParamsKey& key,
                                   uint32_t headerOffset,
                                   SkSL::String* result) {
//    auto[id, numIntsOfData] = ReadCodeSnippetID(data[0]);

//    result->appendf("%s %d\n", gNames[(int) id], numIntsOfData);
}

SkSL::String SkPaintParamsKey::toSkSL() const {
    SkSL::String result;

    int curHeaderOffset = 0;
    while (curHeaderOffset < this->sizeInBytes()) {
        auto [codeSnippetID, blockSize] = this->readCodeSnippetID(curHeaderOffset);

        BlockToSkSL(*this, curHeaderOffset, &result);

//        uint32_t numIntsConsumed = ToSkSLFrom(&data[curIndex], &result);
//        SkASSERT(curIndex + numIntsConsumed <= this->numInts());

        curHeaderOffset += blockSize;
    }

    return result;
}
