/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_PipelineDesc_DEFINED
#define skgpu_PipelineDesc_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"

namespace skgpu {

class RenderPipelineDesc {
public:
    RenderPipelineDesc();

    // Returns this as a uint32_t array to be used as a key in the program cache.
    const uint32_t* asKey() const {
        return fKey.data();
    }

    // Gets the number of bytes in asKey(). It will be a 4-byte aligned value.
    uint32_t keyLength() const {
        return fKey.size() * sizeof(uint32_t);
    }

    bool operator== (const RenderPipelineDesc& that) const {
        return this->fKey == that.fKey;
    }

    bool operator!= (const RenderPipelineDesc& other) const {
        return !(*this == other);
    }

    enum {
        kPreAllocSize = 1,  // TODO: flesh this out
    };

private:
    SkSTArray<kPreAllocSize, uint32_t, true> fKey;
};

// TODO: add ComputePipelineDesc

} // namespace skgpu

#endif // skgpu_PipelineDesc_DEFINED
