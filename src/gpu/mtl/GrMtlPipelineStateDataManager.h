/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlPipelineStateDataManager_DEFINED
#define GrMtlPipelineStateDataManager_DEFINED

#include "src/core/SkAutoMalloc.h"
#include "src/gpu/GrUniformDataManager.h"
#include "src/gpu/mtl/GrMtlUniformHandler.h"

#import <Metal/Metal.h>

class GrMtlBuffer;
class GrMtlGpu;
class GrMtlRenderCommandEncoder;

class GrMtlPipelineStateDataManager : public GrUniformDataManager {
public:
    typedef GrMtlUniformHandler::UniformInfoArray UniformInfoArray;

    GrMtlPipelineStateDataManager(const UniformInfoArray&,
                                  uint32_t uniformSize);

    void set1i(UniformHandle, int32_t) const override;
    void set1iv(UniformHandle, int arrayCount, const int32_t v[]) const override;
    void set1f(UniformHandle, float v0) const override;
    void set1fv(UniformHandle, int arrayCount, const float v[]) const override;
    void set2i(UniformHandle, int32_t, int32_t) const override;
    void set2iv(UniformHandle, int arrayCount, const int32_t v[]) const override;
    void set2f(UniformHandle, float, float) const override;
    void set2fv(UniformHandle, int arrayCount, const float v[]) const override;
    void set3i(UniformHandle, int32_t, int32_t, int32_t) const override;
    void set3iv(UniformHandle, int arrayCount, const int32_t v[]) const override;
    void set3f(UniformHandle, float, float, float) const override;
    void set3fv(UniformHandle, int arrayCount, const float v[]) const override;
    void set4i(UniformHandle, int32_t, int32_t, int32_t, int32_t) const override;
    void set4iv(UniformHandle, int arrayCount, const int32_t v[]) const override;
    void set4f(UniformHandle, float, float, float, float) const override;
    void set4fv(UniformHandle, int arrayCount, const float v[]) const override;
    // matrices are column-major, the first one uploads a single matrix, the latter uploads
    // arrayCount matrices into a uniform array.
    template <int N, GrSLType kFullType, GrSLType kHalfType>
    inline void set(UniformHandle u, const void* v) const;
    template <int N, GrSLType kFullType, GrSLType kHalfType>
    inline void setv(UniformHandle u, int arrayCount, const void* v) const;

    void setMatrix2f(UniformHandle, const float matrix[]) const override;
    void setMatrix2fv(UniformHandle, int arrayCount, const float matrices[]) const override;


    void uploadAndBindUniformBuffers(GrMtlGpu* gpu,
                                     GrMtlRenderCommandEncoder* renderCmdEncoder) const;
    void resetDirtyBits();

private:
    typedef GrUniformDataManager INHERITED;
};

#endif
