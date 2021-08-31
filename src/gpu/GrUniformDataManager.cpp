/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrUniformDataManager.h"

#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrShaderVar.h"

// ensure that these types are the sizes the uniform data is expecting
static_assert(sizeof(int32_t) == 4);
static_assert(sizeof(float) == 4);

//////////////////////////////////////////////////////////////////////////////

GrUniformDataManager::UniformManager::UniformManager(ProgramUniforms uniforms, Layout layout)
        : fUniforms(std::move(uniforms)), fLayout(layout) {}

template <typename T, int VecLength, ptrdiff_t Stride = sizeof(T)*VecLength>
static size_t write_vec(void* dst, int n, const T v[]) {
    if (dst) {
        if (Stride == sizeof(T) * VecLength) {
            std::memcpy(dst, v, n * VecLength * sizeof(T));
        } else {
            for (int i = 0; i < n; ++i) {
                std::memcpy(dst, v, VecLength * sizeof(T));
                v += VecLength;
                dst = SkTAddOffset<void>(dst, Stride);
            }
        }
    }
    return n*Stride;
}

template <typename T, ptrdiff_t Stride = sizeof(T)>
static size_t write(void* d, int n, const T v[]) {
    return write_vec<T, 1, Stride>(d, n, v);
}

template <typename T, int C, int R, ptrdiff_t ColStride = R*sizeof(T)>
static size_t write_matrices(void* d, int n, const T m[]) {
    return write_vec<T, R, ColStride>(d, C*n, m);
}

template<size_t M3x3Writer(void*, int, const float[])>
size_t write_sk_matrices(void* d, int n, const SkMatrix m[]) {
    size_t offset = 0;
    for (int i = 0; i < n; ++i) {
        float mt[] = {
                m[i].get(SkMatrix::kMScaleX),
                m[i].get(SkMatrix::kMSkewY),
                m[i].get(SkMatrix::kMPersp0),
                m[i].get(SkMatrix::kMSkewX),
                m[i].get(SkMatrix::kMScaleY),
                m[i].get(SkMatrix::kMPersp1),
                m[i].get(SkMatrix::kMTransX),
                m[i].get(SkMatrix::kMTransY),
                m[i].get(SkMatrix::kMPersp2),
        };
        offset += M3x3Writer(SkTAddOffset<void>(d, offset), 1, mt);
    }
    return offset;
}

// Scalar bool, int, uint, float, and double:
// Both the size and alignment are the size of the scalar in basic machine types.
//
// Two-component vectors (e.g. ivec2):
// Both the size and alignment are twice the size of the underlying scalar type.
//
// Three-component vectors (e.g. vec3) and four-component vectors (e.g. vec4):
// Both the size and alignment are four times the size of the underlying scalar type.
//
// An array of scalars or vectors: "The size of each element in the array will be the size of
// the element type, rounded up to a multiple of the size of a vec4"
template <typename T, int VecLength = 1>
static constexpr ptrdiff_t Stride140() {
    size_t kVec4Size    = sizeof(float)*4;
    size_t kElementSize = sizeof(T)*VecLength;
    size_t m            = (kElementSize + kVec4Size - 1)/kVec4Size;
    return static_cast<ptrdiff_t>(m*kVec4Size);
}

template<typename T>
static size_t write_140(void* d, int n, const T v[]) {
    return write<T, Stride140<T>()>(d, n, v);
}

template<typename T, int N>
static size_t write_vec_140(void* d, int n, const T v[]) {
    return write_vec<T, N, Stride140<T,N>()>(d, n, v);
}

// "A column-major matrix or an array of column-major matrices of size C columns and R rows:
// Same layout as an array of N vectors each with R components, where N is the total number of
// columns present."
template<typename T, int C, int R>
static size_t write_matrices_140(void* d, int n, const T v[]) {
    return write_matrices<T, C, R, Stride140<T,R>()>(d, n, v);
}
using CType = GrProcessor::Uniform::CType;

static size_t write_uniform_140(GrSLType type, CType ctype, void* d, int n, const void* v) {
    switch (type) {
        case kInt_GrSLType:
            return write_140<int32_t>(d, n, static_cast<const int32_t*>(v));

        case kInt2_GrSLType:
            return write_vec_140<int32_t, 2>(d, n, static_cast<const int32_t*>(v));

        case kInt3_GrSLType:
            return write_vec_140<int32_t, 3>(d, n, static_cast<const int32_t*>(v));

        case kInt4_GrSLType:
            return write_vec_140<int32_t, 4>(d, n, static_cast<const int32_t*>(v));

        case kHalf_GrSLType:
        case kFloat_GrSLType:
            return write_140<float>(d, n, static_cast<const float*>(v));

        case kHalf2_GrSLType:
        case kFloat2_GrSLType:
            return write_vec_140<float, 2>(d, n, static_cast<const float*>(v));

        case kHalf3_GrSLType:
        case kFloat3_GrSLType:
            return write_vec_140<float, 3>(d, n, static_cast<const float*>(v));

        case kHalf4_GrSLType:
        case kFloat4_GrSLType:
            return write_vec_140<float, 4>(d, n, static_cast<const float*>(v));

        case kHalf2x2_GrSLType:
        case kFloat2x2_GrSLType:
            return write_matrices_140<float, 2, 2>(d, n, static_cast<const float*>(v));

        case kHalf3x3_GrSLType:
        case kFloat3x3_GrSLType: {
            static constexpr auto writer = write_matrices_140<float, 3, 3>;
            switch (ctype) {
                case CType::kDefault:
                    return writer(d, n, static_cast<const float*>(v));
                case CType::kSkMatrix:
                    return write_sk_matrices<writer>(d, n, static_cast<const SkMatrix*>(v));
            }
            SkUNREACHABLE;
        }

        case kHalf4x4_GrSLType:
        case kFloat4x4_GrSLType:
            return write_matrices_140<float, 2, 2>(d, n, static_cast<const float*>(v));

        default:
            SK_ABORT("Unexpect uniform type");
    }
}

static size_t write_uniform_430(GrSLType type, CType ctype, void* d, int n, const void* v) {
    switch (type) {
        // Scalar bool, int, uint, float, and double:
        // Both the size and alignment are the size of the scalar in basic machine types.
        //
        // Two-component vectors (e.g. ivec2):
        // Both the size and alignment are twice the size of the underlying scalar type.
        //
        // Three-component vectors (e.g. vec3) and four-component vectors (e.g. vec4):
        // Both the size and alignment are four times the size of the underlying scalar type. However,
        // this is true only when the member is not part of an array or nested structure.
        //
        // An array of scalars or vectors: The size of each element in the array will be the same size
        // of the element type, where three-component vectors *are not* rounded up to the size of four-
        // component vectors. This is also array's alignment. The array's size will be the element's
        // size times the number of elements in the array.
        case kInt_GrSLType:
            return write<int32_t>(d, n, static_cast<const int32_t*>(v));

        case kInt2_GrSLType:
            return write_vec<int32_t, 2>(d, n, static_cast<const int32_t*>(v));

        case kInt3_GrSLType:
            return write_vec<int32_t, 3>(d, n, static_cast<const int32_t*>(v));

        case kInt4_GrSLType:
            return write_vec<int32_t, 4>(d, n, static_cast<const int32_t*>(v));

        case kHalf_GrSLType:
        case kFloat_GrSLType:
            return write<float>(d, n, static_cast<const float*>(v));

        case kHalf2_GrSLType:
        case kFloat2_GrSLType:
            return write_vec<float, 2>(d, n, static_cast<const float*>(v));

        case kHalf3_GrSLType:
        case kFloat3_GrSLType:
            return write_vec<float, 3>(d, n, static_cast<const float*>(v));

        case kHalf4_GrSLType:
        case kFloat4_GrSLType:
            return write_vec<float, 4>(d, n, static_cast<const float*>(v));

        // "A column-major matrix or an array of column-major matrices of size C columns and R rows:
        // Same layout as an array of N vectors each with R components, where N is the total number
        // of columns present."
        case kHalf2x2_GrSLType:
        case kFloat2x2_GrSLType:
            return write_matrices<float, 2, 2>(d, n, static_cast<const float*>(v));

        case kHalf3x3_GrSLType:
        case kFloat3x3_GrSLType: {
            static constexpr auto writer = write_matrices<float, 3, 3, 4*sizeof(float)>;
            switch (ctype) {
                case CType::kDefault:
                    return writer(d, n, static_cast<const float*>(v));
                case CType::kSkMatrix:
                    return write_sk_matrices<writer>(d, n, static_cast<const SkMatrix*>(v));
            }
            SkUNREACHABLE;
        }

        case kHalf4x4_GrSLType:
        case kFloat4x4_GrSLType:
            return write_matrices<float, 2, 2>(d, n, static_cast<const float*>(v));

        default:
            SK_ABORT("Unexpect uniform type");
    }
}

static size_t write_uniform_metal(GrSLType type, CType ctype, void* d, int n, const void* v) {
    // This strides used here were derived from the rules we've imposed on ourselves in
    // GrMtlPipelineStateDataManger. Everything is tight except 3-component which have the stride of
    // their 4-component equivalents.
    switch (type) {
        case kInt_GrSLType:
            return write<int32_t>(d, n, static_cast<const int32_t*>(v));

        case kInt2_GrSLType:
            return write_vec<int32_t, 2>(d, n, static_cast<const int32_t*>(v));

        case kInt3_GrSLType:
            return write_vec<int32_t, 3, 4*sizeof(int32_t)>(d, n, static_cast<const int32_t*>(v));

        case kInt4_GrSLType:
            return write_vec<int32_t, 4>(d, n, static_cast<const int32_t*>(v));

        case kHalf_GrSLType:
        case kFloat_GrSLType:
            return write<float>(d, n, static_cast<const float*>(v));

        case kHalf2_GrSLType:
        case kFloat2_GrSLType:
            return write_vec<float, 2>(d, n, static_cast<const float*>(v));

        case kHalf3_GrSLType:
        case kFloat3_GrSLType:
            return write_vec<float, 3, 4*sizeof(float)>(d, n, static_cast<const float*>(v));

        case kHalf4_GrSLType:
        case kFloat4_GrSLType:
            return write_vec<float, 4>(d, n, static_cast<const float*>(v));

        case kHalf2x2_GrSLType:
        case kFloat2x2_GrSLType:
            return write_matrices<float, 2, 2>(d, n, static_cast<const float*>(v));

        case kHalf3x3_GrSLType:
        case kFloat3x3_GrSLType: {
            static constexpr auto writer = write_matrices<float, 3, 3, 4*sizeof(float)>;
            switch (ctype) {
                case CType::kDefault:
                    return writer(d, n, static_cast<const float*>(v));
                case CType::kSkMatrix:
                    return write_sk_matrices<writer>(d, n, static_cast<const SkMatrix*>(v));
            }
            SkUNREACHABLE;
        }

        case kHalf4x4_GrSLType:
        case kFloat4x4_GrSLType:
            return write_matrices<float, 2, 2>(d, n, static_cast<const float*>(v));

        default:
            SK_ABORT("Unexpect uniform type");
    }
}

bool GrUniformDataManager::UniformManager::writeUniforms(const GrProgramInfo& info, void* buffer) {
    decltype(&write_uniform_140) write;
    switch (fLayout) {
        case Layout::kStd140:
            write = write_uniform_140;
            break;
        case Layout::kStd430:
            write = write_uniform_430;
            break;
        case Layout::kMetal:
            write = write_uniform_metal;
            break;
    }
    bool wrote = false;
    auto set = [&, processorIndex = 0](const GrProcessor& p) mutable {
        const ProcessorUniforms& uniforms = fUniforms[processorIndex];
        for (const NewUniform& u : uniforms) {
            if (u.type != kVoid_GrSLType) {
                SkASSERT(u.count >= 0);
                static_assert(GrShaderVar::kNonArray == 0);
                int n = std::max(u.count, 1);
                void* d = SkTAddOffset<void>(buffer, u.offset);
                size_t index = u.indexInProcessor;
                const void* v = p.uniformData(index);
                write(u.type, p.uniforms()[index].ctype(), d, n, v);
                wrote = true;
            }
        }
        ++processorIndex;
    };

    info.visitProcessors(set);
    return wrote;
}

//////////////////////////////////////////////////////////////////////////////

GrUniformDataManager::GrUniformDataManager(ProgramUniforms uniforms,
                                           Layout layout,
                                           uint32_t uniformCount,
                                           uint32_t uniformSize)
        : fUniformSize(uniformSize)
        , fUniformsDirty(false)
        , fUniformManager(std::move(uniforms), layout) {
    fUniformData.reset(uniformSize);
    fUniforms.push_back_n(uniformCount);
    // subclasses fill in the legacy uniforms in their constructor
}

void GrUniformDataManager::setUniforms(const GrProgramInfo& info) {
    if (fUniformManager.writeUniforms(info, fUniformData.get())) {
        this->markDirty();
    }
}

void* GrUniformDataManager::getBufferPtrAndMarkDirty(const Uniform& uni) const {
    fUniformsDirty = true;
    return static_cast<char*>(fUniformData.get())+uni.fOffset;
}

void GrUniformDataManager::set1i(UniformHandle u, int32_t i) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt_GrSLType || uni.fType == kShort_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, &i, sizeof(int32_t));
}

void GrUniformDataManager::set1iv(UniformHandle u,
                                          int arrayCount,
                                          const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt_GrSLType || uni.fType == kShort_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const int32_t* curVec = &v[i];
        memcpy(buffer, curVec, sizeof(int32_t));
        buffer = static_cast<char*>(buffer) + 4*sizeof(int32_t);
    }
}

void GrUniformDataManager::set1f(UniformHandle u, float v0) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType || uni.fType == kHalf_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, &v0, sizeof(float));
}

void GrUniformDataManager::set1fv(UniformHandle u,
                                          int arrayCount,
                                          const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType || uni.fType == kHalf_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const float* curVec = &v[i];
        memcpy(buffer, curVec, sizeof(float));
        buffer = static_cast<char*>(buffer) + 4*sizeof(float);
    }
}

void GrUniformDataManager::set2i(UniformHandle u, int32_t i0, int32_t i1) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt2_GrSLType || uni.fType == kShort2_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    int32_t v[2] = { i0, i1 };
    memcpy(buffer, v, 2 * sizeof(int32_t));
}

void GrUniformDataManager::set2iv(UniformHandle u,
                                          int arrayCount,
                                          const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt2_GrSLType || uni.fType == kShort2_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const int32_t* curVec = &v[2 * i];
        memcpy(buffer, curVec, 2 * sizeof(int32_t));
        buffer = static_cast<char*>(buffer) + 4*sizeof(int32_t);
    }
}

void GrUniformDataManager::set2f(UniformHandle u, float v0, float v1) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2_GrSLType || uni.fType == kHalf2_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    float v[2] = { v0, v1 };
    memcpy(buffer, v, 2 * sizeof(float));
}

void GrUniformDataManager::set2fv(UniformHandle u,
                                          int arrayCount,
                                          const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2_GrSLType || uni.fType == kHalf2_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const float* curVec = &v[2 * i];
        memcpy(buffer, curVec, 2 * sizeof(float));
        buffer = static_cast<char*>(buffer) + 4*sizeof(float);
    }
}

void GrUniformDataManager::set3i(UniformHandle u,
                                         int32_t i0,
                                         int32_t i1,
                                         int32_t i2) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt3_GrSLType || uni.fType == kShort3_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    int32_t v[3] = { i0, i1, i2 };
    memcpy(buffer, v, 3 * sizeof(int32_t));
}

void GrUniformDataManager::set3iv(UniformHandle u,
                                          int arrayCount,
                                          const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt3_GrSLType || uni.fType == kShort3_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const int32_t* curVec = &v[3 * i];
        memcpy(buffer, curVec, 3 * sizeof(int32_t));
        buffer = static_cast<char*>(buffer) + 4*sizeof(int32_t);
    }
}

void GrUniformDataManager::set3f(UniformHandle u, float v0, float v1, float v2) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat3_GrSLType || uni.fType == kHalf3_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    float v[3] = { v0, v1, v2 };
    memcpy(buffer, v, 3 * sizeof(float));
}

void GrUniformDataManager::set3fv(UniformHandle u,
                                          int arrayCount,
                                          const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat3_GrSLType || uni.fType == kHalf3_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const float* curVec = &v[3 * i];
        memcpy(buffer, curVec, 3 * sizeof(float));
        buffer = static_cast<char*>(buffer) + 4*sizeof(float);
    }
}

void GrUniformDataManager::set4i(UniformHandle u,
                                         int32_t i0,
                                         int32_t i1,
                                         int32_t i2,
                                         int32_t i3) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt4_GrSLType || uni.fType == kShort4_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    int32_t v[4] = { i0, i1, i2, i3 };
    memcpy(buffer, v, 4 * sizeof(int32_t));
}

void GrUniformDataManager::set4iv(UniformHandle u,
                                          int arrayCount,
                                          const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt4_GrSLType || uni.fType == kShort4_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, v, arrayCount * 4 * sizeof(int32_t));
}

void GrUniformDataManager::set4f(UniformHandle u,
                                         float v0,
                                         float v1,
                                         float v2,
                                         float v3) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat4_GrSLType || uni.fType == kHalf4_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    float v[4] = { v0, v1, v2, v3 };
    memcpy(buffer, v, 4 * sizeof(float));
}

void GrUniformDataManager::set4fv(UniformHandle u,
                                          int arrayCount,
                                          const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat4_GrSLType || uni.fType == kHalf4_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, v, arrayCount * 4 * sizeof(float));
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

template<int N> struct set_uniform_matrix;

template<int N> inline void GrUniformDataManager::setMatrices(UniformHandle u,
                                                              int arrayCount,
                                                              const float matrices[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2x2_GrSLType + (N - 2) ||
             uni.fType == kHalf2x2_GrSLType + (N - 2));
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = fUniformData.get();
    fUniformsDirty = true;

    set_uniform_matrix<N>::set(buffer, uni.fOffset, arrayCount, matrices);
}

template<int N> struct set_uniform_matrix {
    inline static void set(void* buffer, int uniformOffset, int count, const float matrices[]) {
        buffer = static_cast<char*>(buffer) + uniformOffset;
        for (int i = 0; i < count; ++i) {
            const float* matrix = &matrices[N * N * i];
            for (int j = 0; j < N; ++j) {
                memcpy(buffer, &matrix[j * N], N * sizeof(float));
                buffer = static_cast<char*>(buffer) + 4 * sizeof(float);
            }
        }
    }
};

template<> struct set_uniform_matrix<4> {
    inline static void set(void* buffer, int uniformOffset, int count, const float matrices[]) {
        buffer = static_cast<char*>(buffer) + uniformOffset;
        memcpy(buffer, matrices, count * 16 * sizeof(float));
    }
};

