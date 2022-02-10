/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintParamsKey_DEFINED
#define SkPaintParamsKey_DEFINED

#include <array>
#include <limits>
#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"
#include "src/core/SkBuiltInCodeSnippetID.h"
#include <vector>

enum class SkBackend : uint8_t {
    kGanesh,
    kGraphite,
    kSkVM
};
class SkPaintParamsKey;
class SkShaderCodeDictionary;
class SkShaderInfo;

class SkPaintParamsKeyBuilder {
public:
    SkPaintParamsKeyBuilder(SkShaderCodeDictionary*);

    void beginBlock(int codeSnippetID);
    void beginBlock(SkBuiltInCodeSnippetID id) { this->beginBlock(static_cast<int>(id)); }
    void endBlock();

    void addBytes(uint32_t numBytes, const uint8_t* data) {
        fData.push_back_n(numBytes, data);
    }
    void addByte(uint8_t data) {
        this->addBytes(1, &data);
    }

#ifdef SK_DEBUG
    uint8_t byte(int offset) const { return fData[offset]; }
#endif

    std::unique_ptr<SkPaintParamsKey> snap();

    size_t sizeInBytes() const { return fData.count(); }

private:
    SkShaderCodeDictionary* fDict;
    std::vector<std::pair<int, int>> fStack;
    SkTArray<uint8_t, true> fData;
};

// This class is a compact representation of the shader needed to implement a given
// PaintParams. Its structure is a series of blocks where each block has a
// header that consists of 2-bytes - a 1-byte code-snippet ID and a 1-byte number-of-bytes-in-the-
// block field. The rest of the data in the block is dependent on the individual code snippet.
class SkPaintParamsKey {
public:
    static const int kBlockHeaderSizeInBytes = 2;
    static const int kBlockSizeOffsetInBytes = 1; // offset to the block size w/in the header

#if 0
    int beginBlock(SkBuiltInCodeSnippetID codeSnippetID) {
        SkASSERT(fNumBytes < kMaxKeySize);

        this->addByte((uint8_t) codeSnippetID);
        this->addByte(0); // this needs to be patched up with a call to endBlock
        return fNumBytes - kBlockHeaderSizeInBytes;
    }

    // Update the size byte of a block header
    void endBlock(int headerOffset, SkBuiltInCodeSnippetID codeSnippetID) {
        SkASSERT(fData[headerOffset] == (uint32_t) codeSnippetID);
        int blockSize = fNumBytes - headerOffset;
        SkASSERT(blockSize <= kMaxBlockSize);
        fData[headerOffset+1] = blockSize;
    }
#endif

    std::pair<SkBuiltInCodeSnippetID, uint8_t> readCodeSnippetID(int headerOffset) const {
        SkASSERT(headerOffset < kMaxKeySize - kBlockHeaderSizeInBytes);

        SkBuiltInCodeSnippetID id = static_cast<SkBuiltInCodeSnippetID>(fData[headerOffset]);
        uint8_t blockSize = fData[headerOffset+1];
        SkASSERT(headerOffset + blockSize <= this->sizeInBytes());

        return { id, blockSize };
    }

#if 0
    void addByte(uint8_t byte) {
        SkASSERT(fNumBytes < kMaxKeySize);

        fData[fNumBytes++] = byte;
    }
#endif

#ifdef SK_DEBUG
    static int DumpBlock(const SkPaintParamsKey&, int headerOffset);
    void dump() const;
#endif
    void toShaderInfo(SkShaderCodeDictionary*, SkShaderInfo*) const;

#ifdef SK_DEBUG
    uint8_t byte(int offset) const { SkASSERT(offset < fNumBytes); return fData[offset]; }
#endif

    const void* data() const { return fData.data(); }
    int sizeInBytes() const { return fNumBytes; }

    bool operator==(const SkPaintParamsKey& that) const;
    bool operator!=(const SkPaintParamsKey& that) const { return !(*this == that); }

private:
    friend class SkPaintParamsKeyBuilder;

    SkPaintParamsKey();

    static int AddBlockToShaderInfo(SkShaderCodeDictionary*,
                                    const SkPaintParamsKey&,
                                    int headerOffset,
                                    SkShaderInfo*);

    // TODO: need to make it so the key can can dynamically grow
    static const int kMaxKeySize = 32;
    static const int kMaxBlockSize = std::numeric_limits<uint8_t>::max();

    // TODO: It is probably overkill but we could encode the SkBackend in the first byte of
    // the key.
    int fNumBytes = 0;
    std::array<uint8_t, kMaxKeySize> fData;
};

#endif // SkPaintParamsKey_DEFINED
