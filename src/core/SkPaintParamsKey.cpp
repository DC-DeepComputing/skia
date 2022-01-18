/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkPaintParamsKey.h"

#include <cstring>
#include "src/core/SkKeyHelpers.h"
#include "src/shaders/SkComposeShader.h"
//#include "src/shaders/SkImageShader.h"

bool SkPaintParamsKey::operator==(const SkPaintParamsKey& that) const {
    return fNumBytes == that.fNumBytes &&
           !memcmp(fData.data(), that.fData.data(), fNumBytes);
}

#ifdef SK_DEBUG
typedef void (*DumpMethod)(const SkPaintParamsKey&, int headerOffset);

namespace {

void dump_unknown_block(const SkPaintParamsKey& key, int headerOffset) {
    uint8_t id = key.byte(headerOffset);
    uint8_t blockSize = key.byte(headerOffset+1);
    SkASSERT(blockSize >= 2 && headerOffset+blockSize <= key.sizeInBytes());

    SkDebugf("Unknown block - id: %d size: %dB\n", id, blockSize);
}

DumpMethod get_dump_method(CodeSnippetID id) {
    switch (id) {
        case CodeSnippetID::kDepthStencilOnlyDraw:  return DepthStencilOnlyBlock::Dump;

        // SkShader code snippets
        case CodeSnippetID::kSolidColorShader:      return SolidColorShaderBlock::Dump;

        case CodeSnippetID::kLinearGradientShader:  [[fallthrough]];
        case CodeSnippetID::kRadialGradientShader:  [[fallthrough]];
        case CodeSnippetID::kSweepGradientShader:   [[fallthrough]];
        case CodeSnippetID::kConicalGradientShader: return GradientShaderBlocks::Dump;

        case CodeSnippetID::kImageShader:           return ImageShaderBlock::Dump;
        case CodeSnippetID::kBlendShader:           return SkShader_Blend::Dump;

        // BlendMode code snippets
        case CodeSnippetID::kSimpleBlendMode:       return BlendModeBlock::Dump;

        default:                                    return dump_unknown_block;
    }
}

} // anonymous namespace

int SkPaintParamsKey::DumpBlock(const SkPaintParamsKey& key, int headerOffset) {
    auto [codeSnippetID, blockSize] = key.readCodeSnippetID(headerOffset);

    get_dump_method(codeSnippetID)(key, headerOffset);

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
