/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUniformData_DEFINED
#define SkUniformData_DEFINED

#include "include/core/SkRefCnt.h"

class SkUniform;

/*
 * TODO: Here is the plan of record for SkUniformData
 *    allocate them (and their ancillary data (offsets & data)) in an arena - and rm SkRefCnt
 *    remove the offsets - these should be recomputable as/if needed
 *    ensure that only one copy of the uniformData is stored in the cache
 *       - if a duplicate is found it can be removed from the arena via a reset/return call
 *    if possible, clear out the cache (and delete the arena) once the DrawList is created/the
 *               uniforms are copied into a buffer
 *       - this will mean we'll get w/in a Recording reuse but not cross-Recording reuse
 *    have the SkUniformData's data layout match what is required by the DrawList so we can just
 *               copy it into the buffer
 */

class SkUniformData : public SkRefCnt {
public:

    // TODO: should we require a name (e.g., "gradient_uniforms") for each uniform block so
    // we can better name the Metal FS uniform struct?
    static sk_sp<SkUniformData> Make(int count,
                                     const SkUniform* uniforms,
                                     size_t dataSize);

    ~SkUniformData() override {
        // TODO: fOffsets and fData should just be allocated right after UniformData in an arena
        delete [] fOffsets;
        delete [] fData;
    }

    int count() const { return fCount; }
    const SkUniform* uniforms() const { return fUniforms; }
    uint32_t* offsets() { return fOffsets; }
    uint32_t offset(int index) {
        SkASSERT(index >= 0 && index < fCount);
        return fOffsets[index];
    }
    char* data() { return fData; }
    size_t dataSize() const { return fDataSize; }

private:
    SkUniformData(int count,
                  const SkUniform* uniforms,
                  uint32_t* offsets,
                  char* data,
                  size_t dataSize)
            : fCount(count)
            , fUniforms(uniforms)
            , fOffsets(offsets)
            , fData(data)
            , fDataSize(dataSize) {
    }

    const int fCount;
    const SkUniform* fUniforms;
    uint32_t* fOffsets; // offset of each uniform in 'fData'
    char* fData;
    const size_t fDataSize;
};

#endif // SkUniformData_DEFINED
