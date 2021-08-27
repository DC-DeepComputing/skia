/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrUniformDataManager.h"

#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrShaderVar.h"
#include "src/gpu/GrUniformWriter.h"

// ensure that these types are the sizes the uniform data is expecting
static_assert(sizeof(int32_t) == 4);
static_assert(sizeof(float) == 4);

class GrUniformDataManager::UniformManager::UniformWriter : public GrUniformWriter {
public:
    UniformWriter(void* dst) : fDst(dst) {}

    void setUniforms(const ProcessorUniforms* uniforms) { fUniforms = uniforms; }

    bool wrote() const { return fWrote; }

protected:
    template <typename T, int VecLength, ptrdiff_t Stride = sizeof(T)*VecLength>
    void writeVec(int n, const T v[]) {
        if (fCurrDst) {
            void* dst = fCurrDst;
            if (Stride == sizeof(T) * VecLength) {
                std::memcpy(dst, v, n * VecLength * sizeof(T));
            } else {
                for (int i = 0; i < n; ++i) {
                    std::memcpy(dst, v, VecLength * sizeof(T));
                    v += VecLength;
                    dst = SkTAddOffset<void>(dst, Stride);
                }
            }
            fWrote = true;
        }
    }

    template <typename T, ptrdiff_t Stride = sizeof(T)> void write(int n, const T v[]) {
        this->writeVec<T, 1, Stride>(n, v);
    }

    template <typename T, int C, int R, ptrdiff_t ColStride = R*sizeof(T)>
    void writeMatrices(int n, const T m[]) {
        this->writeVec<T, R, ColStride>(C*n, m);
    }

private:
    std::tuple<GrSLType, int> willSetUniform(int index) override {
        SkASSERT(fUniforms);
        if (auto iter = fUniforms->find(index); iter != fUniforms->end()) {
            const NewUniform& u = iter->second;
            fCurrDst = SkTAddOffset<void>(fDst, u.offset);
            return {u.type, u.count};
        }
        return std::tuple<GrSLType, int>();
    }

    void didSetUniform() override { fCurrDst = nullptr; }

    const ProcessorUniforms* fUniforms = nullptr;
    void*                    fCurrDst  = nullptr;
    void* const              fDst;
    bool                     fWrote    = false;
};

class GrUniformDataManager::UniformManager::Std140UniformWriter : public UniformWriter {
public:
    Std140UniformWriter(void* dst) : UniformWriter(dst) {}

private:
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
    template <typename T, int VecLength = 1> static constexpr ptrdiff_t Stride() {
        size_t kVec4Size = sizeof(float)*4;
        size_t kElementSize = sizeof(T)*VecLength;
        size_t m = (kElementSize + kVec4Size - 1)/kVec4Size;
        return static_cast<ptrdiff_t>(m*kVec4Size);
    }

    void onWriteiv(int n, const int32_t v[]) override {
        this->write<int32_t, Stride<int32_t>()>(n, v);
    }
    void onWritefv(int n, const float v[]) override { this->write<float, Stride<float>()>(n, v); }

    void onWrite2iv(int n, const int32_t v[]) override {
        this->writeVec<int32_t, 2, Stride<int32_t, 2>()>(n, v);
    }
    void onWrite2fv(int n, const float v[]) override {
        this->writeVec<float, 2, Stride<float, 2>()>(n, v);
    }

    void onWrite3iv(int n, const int32_t v[]) override {
        this->writeVec<int32_t, 3, Stride<int32_t, 3>()>(n, v);
    }
    void onWrite3fv(int n, const float v[]) override {
        this->writeVec<float, 3, Stride<float, 3>()>(n, v);
    }

    void onWrite4iv(int n, const int32_t v[]) override {
        this->writeVec<int32_t, 4, Stride<int32_t, 4>()>(n, v);
    }
    void onWrite4fv(int n, const float v[]) override {
        this->writeVec<float, 4, Stride<float, 4>()>(n, v);
    }

    // "A column-major matrix or an array of column-major matrices of size C columns and R rows:
    // Same layout as an array of N vectors each with R components, where N is the total number of
    // columns present."
    void onWriteMatrix2x2fv(int n, const float matrices[]) override {
        this->writeMatrices<float, 2, 2, Stride<float, 2>()>(n, matrices);
    }

    void onWriteMatrix3x3fv(int n, const float matrices[]) override {
        this->writeMatrices<float, 3, 3, Stride<float, 3>()>(n, matrices);
    }

    void onWriteMatrix4x4fv(int n, const float matrices[]) override {
        this->writeMatrices<float, 4, 4, Stride<float, 4>()>(n, matrices);
    }
};

class GrUniformDataManager::UniformManager::Std430UniformWriter : public UniformWriter {
public:
    Std430UniformWriter(void* dst) : UniformWriter(dst) {}

private:
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

    void onWriteiv(int n, const int32_t v[]) override { this->write<int32_t>(n, v); }
    void onWritefv(int n, const float v[])   override { this->write<float  >(n, v); }

    void onWrite2iv(int n, const int32_t v[]) override { this->writeVec<int32_t, 2>(n, v); }
    void onWrite2fv(int n, const float   v[]) override { this->writeVec<float  , 2>(n, v); }

    void onWrite3iv(int n, const int32_t v[]) override { this->writeVec<int32_t, 3>(n, v); }
    void onWrite3fv(int n, const float   v[]) override { this->writeVec<float,   3>(n, v); }

    void onWrite4iv(int n, const int32_t v[]) override { this->writeVec<int32_t, 4>(n, v); }
    void onWrite4fv(int n, const float   v[]) override { this->writeVec<float  , 4>(n, v); }

    // "A column-major matrix or an array of column-major matrices of size C columns and R rows:
    // Same layout as an array of N vectors each with R components, where N is the total number of
    // columns present."
    void onWriteMatrix2x2fv(int n, const float matrices[]) override {
        this->writeMatrices<float, 2, 2>(n, matrices);
    }

    void onWriteMatrix3x3fv(int n, const float matrices[]) override {
        this->writeMatrices<float, 3, 3, 4*sizeof(float)>(n, matrices);
    }

    void onWriteMatrix4x4fv(int n, const float matrices[]) override {
        this->writeMatrices<float, 4, 4>(n, matrices);
    }
};

// This strides used here were derived from the rules we've imposed on ourselves in
// GrMtlPipelineStateDataManger. Everything is tight except 3-component which have the stride of
// their 4-component equivalents.
class GrUniformDataManager::UniformManager::MetalUniformWriter : public UniformWriter {
public:
    MetalUniformWriter(void* dst) : UniformWriter(dst) {}

private:
    void onWriteiv(int n, const int32_t v[]) override { this->write<int32_t>(n, v); }
    void onWritefv(int n, const float v[])   override { this->write<float  >(n, v); }

    void onWrite2iv(int n, const int32_t v[]) override { this->writeVec<int32_t, 2>(n, v); }
    void onWrite2fv(int n, const float   v[]) override { this->writeVec<float  , 2>(n, v); }

    void onWrite3iv(int n, const int32_t v[]) override {
        this->writeVec<int32_t, 3, 4*sizeof(int32_t)>(n, v);
    }
    void onWrite3fv(int n, const float v[]) override {
        this->writeVec<float, 3, 4*sizeof(float)>(n, v);
    }

    void onWrite4iv(int n, const int32_t v[]) override { this->writeVec<int32_t, 4>(n, v); }
    void onWrite4fv(int n, const float   v[]) override { this->writeVec<float  , 4>(n, v); }

    void onWriteMatrix2x2fv(int n, const float matrices[]) override {
        this->writeMatrices<float, 2, 2>(n, matrices);
    }

    void onWriteMatrix3x3fv(int n, const float matrices[]) override {
        this->writeMatrices<float, 3, 3, 4*sizeof(float)>(n, matrices);
    }

    void onWriteMatrix4x4fv(int n, const float matrices[]) override {
        this->writeMatrices<float, 4, 4>(n, matrices);
    }
};

//////////////////////////////////////////////////////////////////////////////

GrUniformDataManager::UniformManager::UniformManager(ProgramUniforms uniforms, Layout layout)
        : fUniforms(std::move(uniforms)), fLayout(layout) {}

bool GrUniformDataManager::UniformManager::writeUniforms(const GrProgramInfo& info, void* buffer) {
    SkTLazy<Std140UniformWriter> std140Writer;
    SkTLazy<Std430UniformWriter> std430Writer;
    SkTLazy<MetalUniformWriter>  metalWriter;
    UniformWriter* writer;
    switch (fLayout) {
        case Layout::kStd140:
            std140Writer.init(buffer);
            writer = std140Writer.get();
            break;
        case Layout::kStd430:
            std430Writer.init(buffer);
            writer = std430Writer.get();
            break;
        case Layout::kMetal:
            metalWriter.init(buffer);
            writer = metalWriter.get();
            break;
    }

    auto set = [&, processorIndex = 0](const GrProcessor& p) mutable {
        auto iter = fUniforms.find(processorIndex);
        if (iter != fUniforms.end()) {
            const ProcessorUniforms& uniforms = iter->second;
            writer->setUniforms(&uniforms);
            p.writeUniforms(writer);
        }
        ++processorIndex;
    };
    set(info.geomProc());
    for (int i = 0; i < info.pipeline().numFragmentProcessors(); ++i) {
        info.pipeline().getFragmentProcessor(i).visit([&](const GrFragmentProcessor& fp) {
            set(fp);
        });
    }
    set(info.pipeline().getXferProcessor());
    return writer->wrote();
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
    // subclasses fill in the uniforms in their constructor
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

