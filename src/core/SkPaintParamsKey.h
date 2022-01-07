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
static const int kCodeSnippetIDBits = 16;
static const uint32_t kCodeSnippetIDMask = 0xFFFF;

static const uint32_t kMaxBlockSize = 256;

CodeSnippetID ExtractCodeSnippetID(uint32_t code);

// This class is a compact representation of the SkSL code needed to implement a given
// PaintParams. Its structure is a series of blocks where each block has a 4-byte aligned
// header that consists of 2-bytes - a 1-byte code-snippet ID and a 1-byte number-of-ints
// in the block field. The rest of the data in the block is dependent on the individual
// code snippet. For example, a
class SkPaintParamsKey {
public:
    static const int kMaxInts = 8;

    // Block headers are always 4 byte aligned with the following structure:
    //  1st byte: codeSnippetID
    //  2nd byte: codeSnippetSize
    //  TODO: maybe jam the encoding in here? That should go at the start of the entire key though.
    int writeBlockHeader(CodeSnippetID codeSnippetID) {
        SkASSERT(fNumInts < kMaxInts);
        fData[fNumInts] = (int) codeSnippetID;
        return fNumInts++;
    }

    // Update the size byte of a block header
    void patchBlockHeader(int index, CodeSnippetID codeSnippetID, uint32_t blockSize) {
        SkASSERT(blockSize < kMaxBlockSize);
        SkASSERT(fData[index] == (uint32_t) codeSnippetID);
        fData[index] = ((int) codeSnippetID) | (blockSize << kCodeSnippetIDBits);
    }

    static std::pair<CodeSnippetID, uint32_t> ReadCodeSnippetID(uint32_t header) {
        CodeSnippetID id = ExtractCodeSnippetID(header);
        uint32_t blockSize = header >> kCodeSnippetIDBits;

        SkASSERT(blockSize < kMaxBlockSize);
        return { id, blockSize };
    }

    void addInt1(int iFoo) {
        SkASSERT(fNumInts < kMaxInts);
        fData[fNumInts] = iFoo;
        ++fNumInts;
    }

    uint32_t numInts() const { return fNumInts; }

    static uint32_t DumpBlock(const uint32_t* data);
    void dump() const;

    static uint32_t ToSkSLFrom(const uint32_t* data, SkSL::String* result);
    SkSL::String toSkSL() const;

    const void* data() const { return fData.data(); }
    int sizeInBytes() const { return fNumInts * sizeof(uint32_t); }

    bool operator==(const SkPaintParamsKey& rhs) const {
        return fNumInts == rhs.fNumInts &&
               !memcmp(fData.data(), rhs.fData.data(), this->sizeInBytes());
    }

private:
    int fNumInts = 0;
    std::array<uint32_t, kMaxInts> fData;
};

#endif // SkPaintParamsKey_DEFINED
