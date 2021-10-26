/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlPipelineStateDataManager.h"

#include "include/private/SkHalf.h"
#include "src/gpu/mtl/GrMtlBuffer.h"
#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlRenderCommandEncoder.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

// ensure that these types are the sizes the uniform data is expecting
static_assert(sizeof(int32_t) == 4);
static_assert(sizeof(float) == 4);
static_assert(sizeof(short) == 2);
static_assert(sizeof(SkHalf) == 2);

GrMtlPipelineStateDataManager::GrMtlPipelineStateDataManager(const UniformInfoArray& uniforms,
                                                             uint32_t uniformSize)
        : INHERITED(uniforms.count(), uniformSize) {
    // We must add uniforms in same order is the UniformInfoArray so that UniformHandles already
    // owned by other objects will still match up here.
    int i = 0;
    for (const auto& uniformInfo : uniforms.items()) {
        Uniform& uniform = fUniforms[i];
        SkASSERT(GrShaderVar::kNonArray == uniformInfo.fVariable.getArrayCount() ||
                 uniformInfo.fVariable.getArrayCount() > 0);
        SkDEBUGCODE(
            uniform.fArrayCount = uniformInfo.fVariable.getArrayCount();
            uniform.fType = uniformInfo.fVariable.getType();
        )
        uniform.fOffset = uniformInfo.fUBOffset;
        ++i;
    }
}

static void copy_uniforms(void* dest, const void* src, int numUniforms, GrSLType uniformType) {
    switch (uniformType) {
        case kInt_GrSLType:
        case kInt2_GrSLType:
        case kInt3_GrSLType:
        case kInt4_GrSLType:
        case kFloat_GrSLType:
        case kFloat2_GrSLType:
        case kFloat3_GrSLType:
        case kFloat4_GrSLType:
        case kHalf_GrSLType:
        case kHalf2_GrSLType:
        case kHalf3_GrSLType:
        case kHalf4_GrSLType:
        case kShort_GrSLType:
        case kShort2_GrSLType:
        case kShort3_GrSLType:
        case kShort4_GrSLType:
            memcpy(dest, src, numUniforms * 4);
            break;

/*
        // TODO(skia:12339): Enable this once MetalCodeGenerator supports half-precision types.
        case kHalf_GrSLType:
        case kHalf2_GrSLType:
        case kHalf3_GrSLType:
        case kHalf4_GrSLType: {
            const float* floatBits = static_cast<const float*>(src);
            SkHalf* halfBits = static_cast<SkHalf*>(dest);
            while (numUniforms-- > 0) {
                *halfBits++ = SkFloatToHalf(*floatBits++);
            }
            break;
        }

        case kShort_GrSLType:
        case kShort2_GrSLType:
        case kShort3_GrSLType:
        case kShort4_GrSLType: {
            const int32_t* intBits = static_cast<const int32_t*>(src);
            short* shortBits = static_cast<short*>(dest);
            while (numUniforms-- > 0) {
                *shortBits++ = (short)(*intBits++);
            }
            break;
        }
*/
        default:
            SkUNREACHABLE;
    }
}

template <int N, GrSLType FullType, GrSLType HalfType>
void GrMtlPipelineStateDataManager::set(UniformHandle u, const void* v) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == FullType || uni.fType == HalfType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    copy_uniforms(buffer, v, N, uni.fType);
}

template <int N, GrSLType FullType, GrSLType HalfType>
void GrMtlPipelineStateDataManager::setv(UniformHandle u, int arrayCount, const void* v) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == FullType || uni.fType == HalfType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    if constexpr (N == 4) {
        copy_uniforms(buffer, v, arrayCount * 4, uni.fType);
    } else {
        constexpr int stride = (N == 3) ? 4 :
                               (N == 2) ? 2 :
                                          1;
        const int uniformSize = (uni.fType == FullType) ? 4 : 2;

        for (int i = 0; i < arrayCount; ++i) {
            copy_uniforms(buffer, v, N, uni.fType);
            buffer = static_cast<char*>(buffer) + (stride * uniformSize);
            v = static_cast<const char*>(v) + (N * 4);
        }
    }
}

void GrMtlPipelineStateDataManager::set1i(UniformHandle u, int32_t i0) const {
    this->set<1, kInt_GrSLType, kShort_GrSLType>(u, &i0);
}

void GrMtlPipelineStateDataManager::set1iv(UniformHandle u,
                                  int arrayCount,
                                  const int32_t v[]) const {
    this->setv<1, kInt_GrSLType, kShort_GrSLType>(u, arrayCount, v);
}

void GrMtlPipelineStateDataManager::set1f(UniformHandle u, float v0) const {
    this->set<1, kFloat_GrSLType, kHalf_GrSLType>(u, &v0);
}

void GrMtlPipelineStateDataManager::set1fv(UniformHandle u,
                                  int arrayCount,
                                  const float v[]) const {
    this->setv<1, kFloat_GrSLType, kHalf_GrSLType>(u, arrayCount, v);
}

void GrMtlPipelineStateDataManager::set2i(UniformHandle u, int32_t i0, int32_t i1) const {
    int32_t v[2] = { i0, i1 };
    this->set<2, kInt2_GrSLType, kShort2_GrSLType>(u, v);
}

void GrMtlPipelineStateDataManager::set2iv(UniformHandle u,
                                  int arrayCount,
                                  const int32_t v[]) const {
    this->setv<2, kInt2_GrSLType, kShort2_GrSLType>(u, arrayCount, v);
}

void GrMtlPipelineStateDataManager::set2f(UniformHandle u, float v0, float v1) const {
    float v[2] = { v0, v1 };
    this->set<2, kFloat2_GrSLType, kHalf2_GrSLType>(u, v);
}

void GrMtlPipelineStateDataManager::set2fv(UniformHandle u,
                                  int arrayCount,
                                  const float v[]) const {
    this->setv<2, kFloat2_GrSLType, kHalf2_GrSLType>(u, arrayCount, v);
}

void GrMtlPipelineStateDataManager::set3i(UniformHandle u,
                                 int32_t i0,
                                 int32_t i1,
                                 int32_t i2) const {
    int32_t v[3] = { i0, i1, i2 };
    this->set<3, kInt3_GrSLType, kShort3_GrSLType>(u, v);
}

void GrMtlPipelineStateDataManager::set3iv(UniformHandle u,
                                  int arrayCount,
                                  const int32_t v[]) const {
    this->setv<3, kInt3_GrSLType, kShort3_GrSLType>(u, arrayCount, v);
}

void GrMtlPipelineStateDataManager::set3f(UniformHandle u, float v0, float v1, float v2) const {
    float v[3] = { v0, v1, v2 };
    this->set<3, kFloat3_GrSLType, kHalf3_GrSLType>(u, v);
}

void GrMtlPipelineStateDataManager::set3fv(UniformHandle u,
                                  int arrayCount,
                                  const float v[]) const {
    this->setv<3, kFloat3_GrSLType, kHalf3_GrSLType>(u, arrayCount, v);
}

void GrMtlPipelineStateDataManager::set4i(UniformHandle u,
                                 int32_t i0,
                                 int32_t i1,
                                 int32_t i2,
                                 int32_t i3) const {
    int32_t v[4] = { i0, i1, i2, i3 };
    this->set<4, kInt4_GrSLType, kShort4_GrSLType>(u, v);
}

void GrMtlPipelineStateDataManager::set4iv(UniformHandle u,
                                  int arrayCount,
                                  const int32_t v[]) const {
    this->setv<4, kInt4_GrSLType, kShort4_GrSLType>(u, arrayCount, v);
}

void GrMtlPipelineStateDataManager::set4f(UniformHandle u,
                                 float v0,
                                 float v1,
                                 float v2,
                                 float v3) const {
    float v[4] = { v0, v1, v2, v3 };
    this->set<4, kFloat4_GrSLType, kHalf4_GrSLType>(u, v);
}

void GrMtlPipelineStateDataManager::set4fv(UniformHandle u,
                                  int arrayCount,
                                  const float v[]) const {
    this->setv<4, kFloat4_GrSLType, kHalf4_GrSLType>(u, arrayCount, v);
}

void GrMtlPipelineStateDataManager::setMatrix2f(UniformHandle u, const float matrix[]) const {
    this->setMatrix2fv(u, 1, matrix);
}

void GrMtlPipelineStateDataManager::setMatrix2fv(UniformHandle u,
                                                 int arrayCount,
                                                 const float m[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2x2_GrSLType || uni.fType == kHalf2x2_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, m, arrayCount * 4 * sizeof(float));
}

void GrMtlPipelineStateDataManager::uploadAndBindUniformBuffers(
        GrMtlGpu* gpu,
        GrMtlRenderCommandEncoder* renderCmdEncoder) const {
    if (fUniformSize && fUniformsDirty) {
        fUniformsDirty = false;
        if (@available(macOS 10.11, iOS 8.3, *)) {
            if (fUniformSize <= gpu->caps()->maxPushConstantsSize()) {
                renderCmdEncoder->setVertexBytes(fUniformData.get(), fUniformSize,
                                                 GrMtlUniformHandler::kUniformBinding);
                renderCmdEncoder->setFragmentBytes(fUniformData.get(), fUniformSize,
                                                   GrMtlUniformHandler::kUniformBinding);
                return;
            }
        }

        // upload the data
        GrRingBuffer::Slice slice = gpu->uniformsRingBuffer()->suballocate(fUniformSize);
        GrMtlBuffer* buffer = (GrMtlBuffer*) slice.fBuffer;
        char* destPtr = static_cast<char*>(slice.fBuffer->map()) + slice.fOffset;
        memcpy(destPtr, fUniformData.get(), fUniformSize);

        renderCmdEncoder->setVertexBuffer(buffer->mtlBuffer(), slice.fOffset,
                                          GrMtlUniformHandler::kUniformBinding);
        renderCmdEncoder->setFragmentBuffer(buffer->mtlBuffer(), slice.fOffset,
                                            GrMtlUniformHandler::kUniformBinding);
    }
}

void GrMtlPipelineStateDataManager::resetDirtyBits() {
    fUniformsDirty = true;
}

GR_NORETAIN_END
