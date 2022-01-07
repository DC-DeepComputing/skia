/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintParamsKey_DEFINED
#define SkPaintParamsKey_DEFINED

#include <array>
#include "include/core/SkTypes.h"

namespace SkSL {
    class String;
}

// TODO: this needs to be expanded into a more flexible dictionary (esp. for user-supplied SkSL)
enum class CodeSnippetID {
    // SkShader code snippets
    kNoneShader,
    kLinearGradientShader,
    kRadialGradientShader,
    kSweepGradientShader,
    kConicalGradientShader,

    // BlendMode code snippets
    kSimpleBlendMode,

    kLast = kSimpleBlendMode
};
static constexpr int kCodeSnippetIDCount = static_cast<int>(CodeSnippetID::kLast) + 1;

// This class is a compact representation of the shader needed to implement a given
// PaintParams. Its structure is a series of blocks where each block has a
// header that consists of 2-bytes - a 1-byte code-snippet ID and a 1-byte number-of-ints-in-the-
// block field. The rest of the data in the block is dependent on the individual code snippet.
class SkPaintParamsKey {
public:
    // Block headers have the following structure:
    //  1st byte: codeSnippetID
    //  2nd byte: total blockSize in bytes
    // Returns the header's offset in the key - to be passed back into patchBlockHeader
    int writeBlockHeader(CodeSnippetID codeSnippetID) {
        SkASSERT(fNumChars < kMaxKeySize);
        SkASSERT(((int)codeSnippetID) < 256); // TODO: make this a static assert

        this->addByte((uint8_t) codeSnippetID);
        this->addByte(0); // this needs to be patched up with a call to patchBlockHeader
    }

    // Update the size byte of a block header
    void patchBlockHeader(int headerOffset, CodeSnippetID codeSnippetID) {
        SkASSERT(fData[headerOffset] == (uint32_t) codeSnippetID);
        int blockSize = fNumChars - headerOffset;
        SkASSERT(blockSize < kMaxBlockSize);
        fData[headerOffset+1] = blockSize;
    }

    std::pair<CodeSnippetID, uint8_t> readCodeSnippetID(int headerOffset) {
        SkASSERT(headerOffset < kMaxKeySize - 2);

        CodeSnippetID id = static_cast<CodeSnippetID>(fData[headerOffset]);
        uint8_t blockSize = fData[headerOffset+1];
        return { id, blockSize };
    }

#if 0
    void addInt1(int iFoo) {
        SkASSERT(fNumInts < kMaxInts);
        fData[fNumInts] = iFoo;
        ++fNumInts;
    }
#endif

    void addByte(uint8_t byte) {
        SkASSERT(fNumChars < kMaxKeySize);

        fData[fNumChars++] = byte;
    }

    void dump() const;

    SkSL::String toSkSL() const;

    uint8_t byte(uint32_t offset) const { SkASSERT(offset < fNumChars); return fData[offset]; }
    const void* data() const { return fData.data(); }
    int sizeInBytes() const { return fNumChars; }

    bool operator==(const SkPaintParamsKey& rhs) const {
        return fNumChars == rhs.fNumChars &&
               !memcmp(fData.data(), rhs.fData.data(), this->sizeInBytes());
    }

private:
    static uint32_t DumpBlock(const uint32_t* data);
    static uint32_t BlockToSkSL(const uint32_t* data, SkSL::String* result);

    // TODO: need to make it so the key can can dynamically grow
    static const int kMaxKeySize = 32;
    static const uint32_t kMaxBlockSize = 256; // needs to fit in a uint8_t

    int fNumChars = 0;
    std::array<uint8_t, kMaxKeySize> fData;
};

#endif // SkPaintParamsKey_DEFINED
