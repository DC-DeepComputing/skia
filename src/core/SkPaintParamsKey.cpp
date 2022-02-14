/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkPaintParamsKey.h"

#include <cstring>
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkShaderCodeDictionary.h"

//--------------------------------------------------------------------------------------------------
SkPaintParamsKeyBuilder::SkPaintParamsKeyBuilder(SkShaderCodeDictionary* dict)
        : fDict(dict) {
}

// Block headers have the following structure:
//  1st byte: codeSnippetID
//  2nd byte: total blockSize in bytes
// This call stores the header's offset in the key on the stack to be used in 'endBlock'
void SkPaintParamsKeyBuilder::beginBlock(int codeSnippetID) {
    if (codeSnippetID < 0 || codeSnippetID > fDict->maxCodeSnippetID()) {
        this->markInvalid();
        codeSnippetID = static_cast<int>(SkBuiltInCodeSnippetID::kError);   // fall through
    }

    // TODO: add assert that the id fits in a uint8_t

    fData.reserve_back(SkPaintParamsKey::kBlockHeaderSizeInBytes);
    fStack.push_back({ codeSnippetID, this->sizeInBytes() });

#ifdef SK_DEBUG
    static const SkDataPayloadInfo kHeader[2] = {
        {"snippetID", DataPayloadType::kByte, 1},
        {"blockSize", DataPayloadType::kByte, 1},
    };

    static const SkSpan<const SkDataPayloadInfo> kHeaderExpectations(kHeader, 2);

    fDataPayloadExpectations = kHeaderExpectations;
    fCurDataPayloadEntry = 0;
#endif

    this->addByte((uint8_t) codeSnippetID);
    this->addByte(0);  // this needs to be patched up with a call to endBlock

#ifdef SK_DEBUG
    fDataPayloadExpectations = fDict->getDataPayloadExpectations(codeSnippetID);
    fCurDataPayloadEntry = 0;
#endif
}

// Update the size byte of a block header
void SkPaintParamsKeyBuilder::endBlock() {
    if (fStack.empty()) {
        SkASSERT(0);
        this->markInvalid();
        return;
    }

#ifdef SK_DEBUG
    if (fCurDataPayloadEntry != static_cast<int>(fDataPayloadExpectations.size())) {
        // Data payload expectations were not met
        SkASSERT(0);
        this->markInvalid();
        return;
    }
#endif

    int headerOffset = fStack.back().fHeaderOffset;

    SkASSERT(fData[headerOffset] == fStack.back().fCodeSnippetID);
    SkASSERT(fData[headerOffset+SkPaintParamsKey::kBlockSizeOffsetInBytes] == 0);

    int blockSize = this->sizeInBytes() - headerOffset;
    if (blockSize > SkPaintParamsKey::kMaxBlockSize) {
        this->markInvalid();
        blockSize = 0;         // fall through
    }

    fData[headerOffset+SkPaintParamsKey::kBlockSizeOffsetInBytes] = blockSize;

    fStack.pop_back();
}

void SkPaintParamsKeyBuilder::addBytes(uint32_t numBytes, const uint8_t* data) {
    if (fStack.empty()) {
        // Missing call to 'beginBlock'
        SkASSERT(0);
        this->markInvalid();
        return;
    }

#ifdef SK_DEBUG
    if (fCurDataPayloadEntry > static_cast<int>(fDataPayloadExpectations.size()) ||
        fDataPayloadExpectations[fCurDataPayloadEntry].fType != DataPayloadType::kByte ||
        fDataPayloadExpectations[fCurDataPayloadEntry].fCount != numBytes) {
        // Expectation mismatch
        SkASSERT(0);
        this->markInvalid();
        return;
    }

    fCurDataPayloadEntry++;
#endif

    fData.push_back_n(numBytes, data);
}

std::unique_ptr<SkPaintParamsKey> SkPaintParamsKeyBuilder::snap() {
    if (!fStack.empty()) {
        this->markInvalid();  // fall through
    }

    if (!this->isValid()) {
        fData.reset();
        this->beginBlock(SkBuiltInCodeSnippetID::kError);
        this->endBlock();     // fall through
    }

    return std::unique_ptr<SkPaintParamsKey>(new SkPaintParamsKey(&fData));
}

//--------------------------------------------------------------------------------------------------
SkPaintParamsKey::SkPaintParamsKey(SkTArray<uint8_t, true>* data) {
    fData.swap(*data);
}

bool SkPaintParamsKey::operator==(const SkPaintParamsKey& that) const {
    return fData == that.fData;
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

DumpMethod get_dump_method(SkBuiltInCodeSnippetID id) {
    switch (id) {
        case SkBuiltInCodeSnippetID::kDepthStencilOnlyDraw:  return DepthStencilOnlyBlock::Dump;

        // SkShader code snippets
        case SkBuiltInCodeSnippetID::kSolidColorShader:      return SolidColorShaderBlock::Dump;

        case SkBuiltInCodeSnippetID::kLinearGradientShader:  [[fallthrough]];
        case SkBuiltInCodeSnippetID::kRadialGradientShader:  [[fallthrough]];
        case SkBuiltInCodeSnippetID::kSweepGradientShader:   [[fallthrough]];
        case SkBuiltInCodeSnippetID::kConicalGradientShader: return GradientShaderBlocks::Dump;

        case SkBuiltInCodeSnippetID::kImageShader:           return ImageShaderBlock::Dump;
        case SkBuiltInCodeSnippetID::kBlendShader:           return BlendShaderBlock::Dump;

        // BlendMode code snippets
        case SkBuiltInCodeSnippetID::kSimpleBlendMode:       return BlendModeBlock::Dump;

        default:                                             return dump_unknown_block;
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

int SkPaintParamsKey::AddBlockToShaderInfo(SkShaderCodeDictionary* dict,
                                           const SkPaintParamsKey& key,
                                           int headerOffset,
                                           SkShaderInfo* result) {
    auto [codeSnippetID, blockSize] = key.readCodeSnippetID(headerOffset);

    if (codeSnippetID != SkBuiltInCodeSnippetID::kSimpleBlendMode) {
        auto entry = dict->getEntry(codeSnippetID);

        result->add(*entry);

        if (codeSnippetID != SkBuiltInCodeSnippetID::kDepthStencilOnlyDraw) {
            result->setWritesColor();
        }
    }

    return blockSize;
}

void SkPaintParamsKey::toShaderInfo(SkShaderCodeDictionary* dict, SkShaderInfo* result) const {

    int curHeaderOffset = 0;
    while (curHeaderOffset < this->sizeInBytes()) {
        int blockSize = AddBlockToShaderInfo(dict, *this, curHeaderOffset, result);
        curHeaderOffset += blockSize;
    }
}
