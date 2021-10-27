/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ContextUtils_DEFINED
#define skgpu_ContextUtils_DEFINED

#include "experimental/graphite/include/Context.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTileMode.h"

class SkPaint;

namespace skgpu {

class Uniform;

// A single, fully specified combination resulting from a PaintCombo (i.e., it corresponds to a
// specific SkPaint)
struct Combination {
    bool operator==(const Combination& other) const {
        return fShaderType == other.fShaderType &&
               fTileMode == other.fTileMode &&
               fBlendMode == other.fBlendMode;
    }

    ShaderCombo::ShaderType fShaderType = ShaderCombo::ShaderType::kNone;
    SkTileMode fTileMode = SkTileMode::kRepeat;
    SkBlendMode fBlendMode = SkBlendMode::kSrc;
};

class UniformData : public SkRefCnt {
public:
    static constexpr int kInvalidUniformID = 0;

    static sk_sp<UniformData> Make(int count,
                                   const Uniform* uniforms,
                                   size_t dataSize);

    ~UniformData() override {
        // TODO: fOffsets and fData should just be allocated right after UniformData in an arena
        delete [] fOffsets;
        delete [] fData;
    }

    void setID(int id) {   // TODO: maybe make priveleged for only UniformCache
        SkASSERT(fID == kInvalidUniformID);
        fID = id;
    }
    int id() const { return fID; }
    int count() const { return fCount; }
    const Uniform* uniforms() const { return fUniforms; }
    uint32_t* offsets() { return fOffsets; }
    uint32_t offset(int index) {
        SkASSERT(index > 0 && index < fCount);
        return fOffsets[index];
    }
    char* data() { return fData; }
    size_t dataSize() const { return fDataSize; }

private:
    UniformData(int count,
                const Uniform* uniforms,
                uint32_t* offsets,
                char* data,
                size_t dataSize)
            : fCount(count)
            , fUniforms(uniforms)
            , fOffsets(offsets)
            , fData(data)
            , fDataSize(dataSize) {
    }

    int fID = kInvalidUniformID;
    const int fCount;
    const Uniform* fUniforms;
    uint32_t* fOffsets; // offset of each uniform in 'fData'
    char* fData;
    const size_t fDataSize;
};

std::tuple<Combination, sk_sp<UniformData>> ExtractCombo(const SkPaint&);

} // namespace skgpu

#endif // skgpu_ContextUtils_DEFINED
