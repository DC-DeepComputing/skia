/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_UniformManager_DEFINED
#define skgpu_UniformManager_DEFINED

#include "include/core/SkSpan.h"
#include "src/core/SkSLTypeShared.h"

class SkUniform;

namespace skgpu {

enum class Layout : uint8_t {
    kStd140,
    kStd430,
    kMetal, /** This is our own self-imposed layout we use for Metal. */
};

// This is only used in the SkPipelineDataGatherer - maybe hide it better.
class UniformManager {
public:
    UniformManager(Layout layout);

    void reset();
#ifdef SK_DEBUG
    void checkReset() const;
#endif
    /*
     * Use the uniform 'definitions' to write the data in 'srcs'.
     */
    void writeUniforms1(SkSpan<const SkUniform> definitions,
                        const void** srcs,
                        uint32_t* offsets);

private:
    SkSLType getUniformTypeForLayout(SkSLType type);

    Layout fLayout;
#ifdef SK_DEBUG
    uint32_t fCurUBOOffset;
    uint32_t fCurUBOMaxAlignment;
#endif // SK_DEBUG
    uint32_t fOffset;

    char* getMem(size_t needed) {
        if (fUsed + needed > fCapacity) {
            char* newMem = new char[fUsed + needed];
            memcpy(newMem, fDest, fUsed);
            delete [] fDest;
            fDest = newMem;
            fCapacity = fUsed + needed;
        }

        size_t old = fUsed;
        fUsed += needed;
        return &fDest[old];
    }

    char* fDest = nullptr;
    size_t fUsed = 0;
    size_t fCapacity = 0;
};

} // namespace skgpu

#endif // skgpu_UniformManager_DEFINED
