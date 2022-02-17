/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintParamsKey_DEFINED
#define SkPaintParamsKey_DEFINED

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"
#include "src/core/SkBuiltInCodeSnippetID.h"

#include <array>
#include <limits>
#include <vector>

enum class SkBackend : uint8_t {
    kGanesh,
    kGraphite,
    kSkVM
};
class SkPaintParamsKey;
class SkShaderCodeDictionary;
class SkShaderInfo;

class SkPaintParamsKeyPersistentData {
public:
    ~SkPaintParamsKeyPersistentData() {
        SkASSERT(!this->isLocked());
    }

private:
    friend class SkPaintParamsKeyBuilder;
    friend class SkPaintParamsKey;

    // TODO: templatize these
    uint8_t& operator[] (int i) {
        SkASSERT(!this->isLocked());
        return fData[i];
    }

    uint8_t operator[] (int i) const {
        return fData[i];
    }

    void push_back_n(int n, const uint8_t* data) {
        SkASSERT(!this->isLocked());
        fData.push_back_n(n, data);
    }

    void reserve_back(int n) {
        SkASSERT(!this->isLocked());
        fData.reserve_back(n);
    }

    void reset() {
        SkASSERT(!this->isLocked());
        fData.reset();
    }

    int count() const { return fData.count(); }
    const uint8_t* data() const { return fData.data(); }

#ifdef SK_DEBUG
    void lock() { SkASSERT(!fLocked); fLocked = true; }
    void unlock() {
        SkASSERT(fLocked);
        fData.reset();
        fLocked = false;
    }
    bool isLocked() const { return fLocked; }

    bool fLocked = false;
#endif

    // TODO: this could be an SkArena
    SkTArray<uint8_t, true> fData;
};

// TODO: Make the builder carry the 'backend' rather than passing it everywhere separately
class SkPaintParamsKeyBuilder {
public:
    SkPaintParamsKeyBuilder(const SkShaderCodeDictionary*,
                            SkPaintParamsKeyPersistentData*);

    void beginBlock(int codeSnippetID);
    void beginBlock(SkBuiltInCodeSnippetID id) { this->beginBlock(static_cast<int>(id)); }
    void endBlock();

    void addBytes(uint32_t numBytes, const uint8_t* data);
    void addByte(uint8_t data) {
        this->addBytes(1, &data);
    }

#ifdef SK_DEBUG
    void checkReset();
    uint8_t byte(int offset) const { return (*fPersistentData)[offset]; }
#endif

    SkPaintParamsKey snap();

    int sizeInBytes() const { return fPersistentData->count(); }

    bool isValid() const { return fIsValid; }

private:
    void makeInvalid();

    struct StackFrame {
        int fCodeSnippetID;
        int fHeaderOffset;
    };

    bool fIsValid = true;
    const SkShaderCodeDictionary* fDict;
    std::vector<StackFrame> fStack;

    // TODO: It is probably overkill but we could encode the SkBackend in the first byte of
    // the key.
    SkPaintParamsKeyPersistentData* fPersistentData;
};

// This class is a compact representation of the shader needed to implement a given
// PaintParams. Its structure is a series of blocks where each block has a
// header that consists of 2-bytes:
//   a 1-byte code-snippet ID
//   a 1-byte number-of-bytes-in-the-block field (incl. the space for the header)
// The rest of the data in the block is dependent on the individual code snippet.
// If a given block has child blocks, they appear in the key right after their
// parent block's header.
class SkPaintParamsKey {
public:
    static const int kBlockHeaderSizeInBytes = 2;
    static const int kBlockSizeOffsetInBytes = 1; // offset to the block size w/in the header
    static const int kMaxBlockSize = std::numeric_limits<uint8_t>::max();

    ~SkPaintParamsKey();

    std::pair<SkBuiltInCodeSnippetID, uint8_t> readCodeSnippetID(int headerOffset) const {
        SkASSERT(headerOffset < this->sizeInBytes() - kBlockHeaderSizeInBytes);

        SkBuiltInCodeSnippetID id = static_cast<SkBuiltInCodeSnippetID>(fData[headerOffset]);
        uint8_t blockSize = fData[headerOffset+1];
        SkASSERT(headerOffset + blockSize <= this->sizeInBytes());

        return { id, blockSize };
    }

#ifdef SK_DEBUG
    uint8_t byte(int offset) const {
        SkASSERT(offset < this->sizeInBytes());
        return fData[offset];
    }
    static int DumpBlock(const SkPaintParamsKey&, int headerOffset);
    void dump() const;
#endif
    void toShaderInfo(SkShaderCodeDictionary*, SkShaderInfo*) const;

    SkSpan<const uint8_t> asSpan() const { return fData; }
    const uint8_t* data() const { return fData.data(); }
    int sizeInBytes() const { return SkTo<int>(fData.size()); }

    bool operator==(const SkPaintParamsKey& that) const;
    bool operator!=(const SkPaintParamsKey& that) const { return !(*this == that); }

#if GR_TEST_UTILS
    bool isErrorKey() const;
#endif

private:
    friend class SkPaintParamsKeyBuilder;
    friend class SkShaderCodeDictionary;

    // This ctor is to be used when paintparam keys are being consecutively generated
    // by paintparam key builders. The persistent data is shared between the builder and
    // its keys.
    SkPaintParamsKey(SkPaintParamsKeyPersistentData*);

    // This ctor is used when there is no persistentData object associated with the key
    // (i.e., when the key is in the dictionary)
    SkPaintParamsKey(SkSpan<const uint8_t> rawData);

    static int AddBlockToShaderInfo(SkShaderCodeDictionary*,
                                    const SkPaintParamsKey&,
                                    int headerOffset,
                                    SkShaderInfo*);

    SkSpan<const uint8_t> fData;
    SkPaintParamsKeyPersistentData* fPersistentData1 = nullptr;
};

#endif // SkPaintParamsKey_DEFINED
