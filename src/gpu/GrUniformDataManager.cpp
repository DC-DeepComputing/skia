/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrUniformDataManager.h"

#include "src/gpu/GrShaderVar.h"

// ensure that these types are the sizes the uniform data is expecting
static_assert(sizeof(int32_t) == 4);
static_assert(sizeof(float) == 4);

// We rely on these GrSLTypes being adjacent in the enum for FloatNType/HalfNType.
static_assert(kFloat_GrSLType + 1 == kFloat2_GrSLType &&
              kFloat_GrSLType + 2 == kFloat3_GrSLType &&
              kFloat_GrSLType + 3 == kFloat4_GrSLType);
static_assert(kHalf_GrSLType + 1 == kHalf2_GrSLType &&
              kHalf_GrSLType + 2 == kHalf3_GrSLType &&
              kHalf_GrSLType + 3 == kHalf4_GrSLType);
// We rely on these GrSLTypes being adjacent in the enum for IntNType/ShortNType.
static_assert(kInt_GrSLType + 1   == kInt2_GrSLType &&
              kInt_GrSLType + 2   == kInt3_GrSLType &&
              kInt_GrSLType + 3   == kInt4_GrSLType);
static_assert(kShort_GrSLType + 1 == kShort2_GrSLType &&
              kShort_GrSLType + 2 == kShort3_GrSLType &&
              kShort_GrSLType + 3 == kShort4_GrSLType);

GrUniformDataManager::GrUniformDataManager(uint32_t uniformCount, uint32_t uniformSize)
    : fUniformSize(uniformSize)
    , fUniformsDirty(false) {
    fUniformData.reset(uniformSize);
    fUniforms.push_back_n(uniformCount);
    // subclasses fill in the uniforms in their constructor
}

void* GrUniformDataManager::getBufferPtrAndMarkDirty(const Uniform& uni) const {
    fUniformsDirty = true;
    return static_cast<char*>(fUniformData.get())+uni.fOffset;
}

template <int N>
void GrUniformDataManager::seti(UniformHandle u, const int32_t* v) const {
    constexpr GrSLType kIntNType   = (GrSLType)(kInt_GrSLType + N - 1);
    constexpr GrSLType kShortNType = (GrSLType)(kShort_GrSLType + N - 1);

    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kIntNType || uni.fType == kShortNType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, v, N * sizeof(int32_t));
}

template <int N>
void GrUniformDataManager::setiv(UniformHandle u, int arrayCount, const int32_t v[]) const {
    constexpr GrSLType kIntNType   = (GrSLType)(kInt_GrSLType + N - 1);
    constexpr GrSLType kShortNType = (GrSLType)(kShort_GrSLType + N - 1);

    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kIntNType || uni.fType == kShortNType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    if constexpr (N == 4) {
        memcpy(buffer, v, arrayCount * 4 * sizeof(int32_t));
    }
    else {
        for (int i = 0; i < arrayCount; ++i) {
            memcpy(buffer, v, 2 * sizeof(int32_t));
            buffer = static_cast<char*>(buffer) + N*sizeof(int32_t);
            v += N;
        }
    }
}

template <int N>
void GrUniformDataManager::setf(UniformHandle u, const float* v) const {
    constexpr GrSLType kFloatNType = (GrSLType)(kFloat_GrSLType + N - 1);
    constexpr GrSLType kHalfNType  = (GrSLType)(kHalf_GrSLType + N - 1);

    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloatNType || uni.fType == kHalfNType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, v, N * sizeof(float));
}

template <int N>
void GrUniformDataManager::setfv(UniformHandle u, int arrayCount, const float v[]) const {
    constexpr GrSLType kFloatNType = (GrSLType)(kFloat_GrSLType + N - 1);
    constexpr GrSLType kHalfNType  = (GrSLType)(kHalf_GrSLType + N - 1);

    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloatNType || uni.fType == kHalfNType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    if constexpr (N == 4) {
        memcpy(buffer, v, 4 * sizeof(float) * arrayCount);
    } else {
        for (int i = 0; i < arrayCount; ++i) {
            memcpy(buffer, v, N * sizeof(float));
            buffer = static_cast<char*>(buffer) + 4*sizeof(float);
            v += N;
        }
    }
}

void GrUniformDataManager::set1i(UniformHandle u, int32_t i0) const {
    this->seti<1>(u, &i0);
}

void GrUniformDataManager::set1iv(UniformHandle u,
                                  int arrayCount,
                                  const int32_t v[]) const {
    this->setiv<1>(u, arrayCount, v);
}

void GrUniformDataManager::set1f(UniformHandle u, float v0) const {
    this->setf<1>(u, &v0);
}

void GrUniformDataManager::set1fv(UniformHandle u,
                                  int arrayCount,
                                  const float v[]) const {
    this->setfv<1>(u, arrayCount, v);
}

void GrUniformDataManager::set2i(UniformHandle u, int32_t i0, int32_t i1) const {
    int32_t v[2] = { i0, i1 };
    this->seti<2>(u, v);
}

void GrUniformDataManager::set2iv(UniformHandle u,
                                  int arrayCount,
                                  const int32_t v[]) const {
    this->setiv<2>(u, arrayCount, v);
}

void GrUniformDataManager::set2f(UniformHandle u, float v0, float v1) const {
    float v[2] = { v0, v1 };
    this->setf<2>(u, v);
}

void GrUniformDataManager::set2fv(UniformHandle u,
                                  int arrayCount,
                                  const float v[]) const {
    this->setfv<2>(u, arrayCount, v);
}

void GrUniformDataManager::set3i(UniformHandle u,
                                 int32_t i0,
                                 int32_t i1,
                                 int32_t i2) const {
    int32_t v[3] = { i0, i1, i2 };
    this->seti<3>(u, v);
}

void GrUniformDataManager::set3iv(UniformHandle u,
                                  int arrayCount,
                                  const int32_t v[]) const {
    this->setiv<3>(u, arrayCount, v);
}

void GrUniformDataManager::set3f(UniformHandle u, float v0, float v1, float v2) const {
    float v[3] = { v0, v1, v2 };
    this->setf<3>(u, v);
}

void GrUniformDataManager::set3fv(UniformHandle u,
                                  int arrayCount,
                                  const float v[]) const {
    this->setfv<3>(u, arrayCount, v);
}

void GrUniformDataManager::set4i(UniformHandle u,
                                 int32_t i0,
                                 int32_t i1,
                                 int32_t i2,
                                 int32_t i3) const {
    int32_t v[4] = { i0, i1, i2, i3 };
    this->seti<4>(u, v);
}

void GrUniformDataManager::set4iv(UniformHandle u,
                                  int arrayCount,
                                  const int32_t v[]) const {
    this->setiv<4>(u, arrayCount, v);
}

void GrUniformDataManager::set4f(UniformHandle u,
                                 float v0,
                                 float v1,
                                 float v2,
                                 float v3) const {
    float v[4] = { v0, v1, v2, v3};
    this->setf<4>(u, v);
}

void GrUniformDataManager::set4fv(UniformHandle u,
                                  int arrayCount,
                                  const float v[]) const {
    this->setfv<4>(u, arrayCount, v);
}

void GrUniformDataManager::setMatrix2f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<2>(u, 1, matrix);
}

void GrUniformDataManager::setMatrix2fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<2>(u, arrayCount, m);
}

void GrUniformDataManager::setMatrix3f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<3>(u, 1, matrix);
}

void GrUniformDataManager::setMatrix3fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<3>(u, arrayCount, m);
}

void GrUniformDataManager::setMatrix4f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<4>(u, 1, matrix);
}

void GrUniformDataManager::setMatrix4fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<4>(u, arrayCount, m);
}

template <int N> inline void GrUniformDataManager::setMatrices(UniformHandle u,
                                                               int arrayCount,
                                                               const float matrices[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2x2_GrSLType + (N - 2) ||
             uni.fType == kHalf2x2_GrSLType + (N - 2));
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    if constexpr (N == 4) {
        memcpy(buffer, matrices, arrayCount * 16 * sizeof(float));
    } else {
        for (int i = 0; i < arrayCount; ++i) {
            const float* matrix = &matrices[N * N * i];
            for (int j = 0; j < N; ++j) {
                memcpy(buffer, &matrix[j * N], N * sizeof(float));
                buffer = static_cast<char*>(buffer) + 4 * sizeof(float);
            }
        }
    }
}
