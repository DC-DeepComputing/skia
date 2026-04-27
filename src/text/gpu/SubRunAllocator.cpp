/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/gpu/SubRunAllocator.h"

#include "include/private/base/SkMath.h"
#include "include/private/base/SkTFitsIn.h"

#include <cstddef>
#include <memory>
#include <new>

namespace sktext::gpu {

// -- BagOfBytes ---------------------------------------------------------------------------------
BagOfBytes::BagOfBytes(char* bytes, size_t size, size_t firstHeapAllocation)
        : fFibProgression(static_cast<uint32_t>(size), static_cast<uint32_t>(firstHeapAllocation)) {
    SkASSERT_RELEASE(size < kMaxByteSize);
    SkASSERT_RELEASE(firstHeapAllocation < kMaxByteSize);

    std::size_t space = size;
    void* ptr = bytes;
    if (bytes && std::align(kMaxAlignment, sizeof(Block), ptr, space)) {
        this->setupBytesAndCapacity(bytes, size);
        new (fEndByte) Block(nullptr, nullptr);
    }
}

BagOfBytes::BagOfBytes(size_t firstHeapAllocation)
        : BagOfBytes(nullptr, 0, firstHeapAllocation) {}

BagOfBytes::~BagOfBytes() {
    Block* cursor = reinterpret_cast<Block*>(fEndByte);
    while (cursor != nullptr) {
        char* toDelete = cursor->fBlockStart;
        cursor = cursor->fPrevious;
        delete [] toDelete;
    }
}

BagOfBytes::Block::Block(char* previous, char* startOfBlock)
        : fBlockStart{startOfBlock}
        , fPrevious{reinterpret_cast<Block*>(previous)} {}

void* BagOfBytes::alignedBytes(size_t size, size_t alignment) {
    SkASSERT_RELEASE(0 < size && size < kMaxByteSize);
    SkASSERT_RELEASE(0 < alignment && alignment <= kMaxAlignment);
    SkASSERT_RELEASE(SkIsPow2(alignment));

    return this->allocateBytes(size, alignment);
}

void BagOfBytes::setupBytesAndCapacity(char* bytes, size_t size) {
    // endByte must be aligned to the maximum alignment to allow tracking alignment using capacity;
    // capacity and endByte are both aligned to max alignment.
    uintptr_t endByte = reinterpret_cast<uintptr_t>(bytes + size - sizeof(Block)) & ~(kMaxAlignment - 1);
    fEndByte  = reinterpret_cast<char*>(endByte);
    fCapacity = static_cast<size_t>(fEndByte - bytes);
}

void BagOfBytes::needMoreBytes(size_t requestedSize, size_t alignment) {
    uint32_t nextBlockSize = fFibProgression.nextBlockSize();
    const size_t size = PlatformMinimumSizeWithOverhead(
            std::max<size_t>(requestedSize, nextBlockSize), kAllocationAlignment);
    char* const bytes = new char[size];
    // fEndByte is changed by setupBytesAndCapacity. Remember it to link back to.
    char* const previousBlock = fEndByte;
    this->setupBytesAndCapacity(bytes, size);

    // Make a block to delete these bytes, and points to the previous block.
    new (fEndByte) Block{previousBlock, bytes};

    // Make fCapacity the alignment for the requested object.
    fCapacity = fCapacity & ~(alignment - 1);
    SkASSERT(fCapacity >= requestedSize);
}

// -- SubRunAllocator ----------------------------------------------------------------------------
SubRunAllocator::SubRunAllocator(char* bytes, size_t size, size_t firstHeapAllocation)
        : fAlloc{bytes, size, firstHeapAllocation} { }

SubRunAllocator::SubRunAllocator(size_t firstHeapAllocation)
        : SubRunAllocator(nullptr, 0, firstHeapAllocation) { }

void* SubRunAllocator::alignedBytes(size_t unsafeSize, size_t unsafeAlignment) {
    return fAlloc.alignedBytes(unsafeSize, unsafeAlignment);
}

}  // namespace sktext::gpu
