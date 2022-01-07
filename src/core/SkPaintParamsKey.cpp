/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkPaintParamsKey.h"

#include "src/core/SkKeyHelpers.h"

#ifdef SK_DEBUG
typedef void (*Dumper)(const SkPaintParamsKey&, uint32_t headerOffset);

static Dumper gDumpers[kCodeSnippetIDCount] = {
        NoShaderBlock::Dump,
        LinearGradientShaderBlock::Dump,
        RadialGradientShaderBlock::Dump,
        SweepGradientShaderBlock::Dump,
        ConicalGradientShaderBlock::Dump,

        BlendModeBlock::Dump,
};

// This just iterates over the top-level blocks calling DumpBlock.
void SkPaintParamsKey::dump() const {
    SkDebugf("SkPaintParamsKey %dB:\n", this->sizeInBytes());

    uint32_t curHeaderOffset = 0;
    while (curHeaderOffset < this->sizeInBytes()) {
        auto [codeSnippetID, blockSize] = this->readCodeSnippetID(curHeaderOffset);

        (gDumpers[(int) codeSnippetID])(*this, curHeaderOffset);

        curHeaderOffset += blockSize;
    }
}
#endif

SkSL::String SkPaintParamsKey::toSkSL() const {
    SkSL::String result;

    uint32_t curHeaderOffset = 0;
    while (curHeaderOffset < this->sizeInBytes()) {
        auto [codeSnippetID, blockSize] = this->readCodeSnippetID(curHeaderOffset);

        // Call the block's toSkSL function

        curHeaderOffset += blockSize;
    }

    return result;
}
