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
