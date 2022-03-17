/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_PipelineDataCache_DEFINED
#define skgpu_PipelineDataCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/core/SkPipelineData.h"

#include <unordered_map>
#include <vector>

namespace skgpu {

class UniformDataCache {
public:
    static constexpr uint32_t kInvalidUniformDataID = 0;

    UniformDataCache();

    class UniformDataIndex {
    public:
        bool operator==(const UniformDataIndex& that) const { return fIndex == that.fIndex; }
        bool operator!=(const UniformDataIndex& that) const { return !(*this == that); }
        UniformDataIndex() : fIndex(kInvalidUniformDataID) {}
        explicit UniformDataIndex(uint32_t index) : fIndex(index) {}
        bool isValid() const { return fIndex != kInvalidUniformDataID; }
        uint32_t asUInt() const { return fIndex; }
        void update(uint32_t index) { fIndex = index; }
    private:
        uint32_t fIndex;
    };

    // TODO: Revisit the UniformDataCache::insert and UniformData::Make APIs:
    // 1. UniformData::Make requires knowing the data size up front, which involves two invocations
    //    of the UniformManager. Ideally, we could align uniforms on the fly into a dynamic buffer.
    // 2. UniformData stores the offsets for each uniform, but these aren't needed after we've
    //    filled out the buffer. If we remember layout offsets, it should be stored per Combination
    //    or RenderStep that defines the uniform set.
    // 3. UniformCache's ids are only fundamentally limited by the number of draws that can be
    //    recorded into a DrawPass, which means a very large recording with multiple passes could
    //    exceed uint32_t across all the passes.
    // 4. The check to know if a UniformData is present in the cache is practically the same for
    //    checking if the data needs to be uploaded to the GPU, so UniformCache could remember the
    //    associated BufferBindInfos as well.
    // 5. Because UniformCache only cares about the content byte hash/equality, and can memcpy to
    //    the GPU buffer, the cached data contents could all go into a shared byte array, instead of
    //    needing to extend SkRefCnt.
    // 6. insert() as a name can imply that the value is always added, so we may want a better one.
    //    It can be a little less generic if UniformCache returns id and bind buffer info. On the
    //    other hand unordered_map::insert has the same semantics as this insert, so maybe it's fine

    // Add the block of uniform data to the cache and return a unique ID that corresponds to its
    // contents. If an identical block of data is already in the cache, that unique ID is returned.
    UniformDataIndex insert1(const SkPipelineData::UniformDataBlock&);

    SkPipelineData::UniformDataBlock* lookup1(UniformDataIndex uniqueID);

    // The number of unique UniformDataBlock objects in the cache
    size_t count() const {
        SkASSERT(fUniformDataBlocks.size() == fUniformDataBlockIDs.size() && fUniformDataBlocks.size() > 0);
        return fUniformDataBlocks.size() - 1;
    }

private:
    struct Hash {
        // This hash operator de-references and hashes the data contents
        size_t operator()(SkPipelineData::UniformDataBlock*) const;
    };
    struct Eq {
        // This equality operator de-references and compares the actual data contents
        bool operator()(SkPipelineData::UniformDataBlock*, SkPipelineData::UniformDataBlock*) const;
    };

    // The UniformBlock's unique ID is only unique w/in a Recorder _not_ globally
    std::unordered_map<SkPipelineData::UniformDataBlock*, UniformDataIndex, Hash, Eq> fUniformDataBlockIDs;
    std::vector<std::unique_ptr<SkPipelineData::UniformDataBlock>> fUniformDataBlocks;

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif
};

class TextureDataCache {
public:
    static constexpr uint32_t kInvalidTextureDataID = 0;

    TextureDataCache() {}

    class TextureDataIndex {
    public:
        bool operator==(const TextureDataIndex& that) const { return fIndex == that.fIndex; }
        bool operator!=(const TextureDataIndex& that) const { return !(*this == that); }
        TextureDataIndex() : fIndex(kInvalidTextureDataID) {}
        explicit TextureDataIndex(uint32_t index) : fIndex(index) {}
        bool isValid() const { return fIndex != kInvalidTextureDataID; }
        uint32_t asUInt() const { return fIndex; }
        void update(uint32_t index) { fIndex = index; }
    private:
        uint32_t fIndex;
    };

    TextureDataIndex insert1(const SkPipelineData::TextureDataBlock&) {
        return TextureDataIndex();
    }

    SkPipelineData::TextureDataBlock* lookup1(TextureDataIndex uniqueID) {
        return nullptr;
    }

private:
};

} // namespace skgpu

#endif // skgpu_PipelineDataCache_DEFINED
