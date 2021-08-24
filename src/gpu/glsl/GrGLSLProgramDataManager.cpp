/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/glsl/GrGLSLProgramDataManager.h"

#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "src/core/SkMatrixPriv.h"

#include "src/gpu/GrShaderVar.h"

class GrUniformWriter::AutoSetUniform {
public:
    AutoSetUniform(GrUniformWriter* writer, int index) : fWriter(writer) {
        std::tie(fWriter->fType, fWriter->fN) = fWriter->willSetUniform(index);
    }

    ~AutoSetUniform() {
        fWriter->didSetUniform();
        fWriter->fType = kVoid_GrSLType;
        fWriter->fN = -1;
    }

private:
    GrUniformWriter* fWriter;
};

void GrUniformWriter::writeiv(int index, int n, const int32_t v[]) {
    AutoSetUniform asu(this, index);
    SkASSERT(GrSLTypeIsIntegralType(fType));
    SkASSERT(GrSLTypeVecLength(fType) == 1);
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWriteiv(n, v);
}

void GrUniformWriter::writefv(int index, int n, const float v[]) {
    AutoSetUniform asu(this, index);
    SkASSERT(GrSLTypeIsFloatType(fType));
    SkASSERT(GrSLTypeVecLength(fType) == 1);
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWritefv(n, v);
}

void GrUniformWriter::write2iv(int index, int n, const int32_t v[]) {
    AutoSetUniform asu(this, index);
    SkASSERT(GrSLTypeIsIntegralType(fType));
    SkASSERT(GrSLTypeVecLength(fType) == 2);
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWrite2iv(n, v);
}

void GrUniformWriter::write2fv(int index, int n, const float v[]) {
    AutoSetUniform asu(this, index);
    SkASSERT(GrSLTypeIsFloatType(fType));
    SkASSERT(GrSLTypeVecLength(fType) == 2);
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWrite2fv(n, v);
}

void GrUniformWriter::write3iv(int index, int n, const int32_t v[]) {
    AutoSetUniform asu(this, index);
    SkASSERT(GrSLTypeIsIntegralType(fType));
    SkASSERT(GrSLTypeVecLength(fType) == 3);
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWrite3iv(n, v);
}

void GrUniformWriter::write3fv(int index, int n, const float v[]) {
    AutoSetUniform asu(this, index);
    SkASSERT(GrSLTypeIsFloatType(fType));
    SkASSERT(GrSLTypeVecLength(fType) == 3);
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWrite3fv(n, v);
}

void GrUniformWriter::write4iv(int index, int n, const int32_t v[]) {
    AutoSetUniform asu(this, index);
    SkASSERT(GrSLTypeIsIntegralType(fType));
    SkASSERT(GrSLTypeVecLength(fType) == 4);
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWrite4iv(n, v);
}

void GrUniformWriter::write4fv(int index, int n, const float v[]) {
    AutoSetUniform asu(this, index);
    SkASSERT(GrSLTypeIsFloatType(fType));
    SkASSERT(GrSLTypeVecLength(fType) == 4);
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWrite4fv(n, v);
}

void GrUniformWriter::writeMatrix2x2fv(int index, int n, const float matrices[]) {
    AutoSetUniform asu(this, index);
    SkASSERT(GrSLTypeIsFloatType(fType));
    SkASSERT(GrSLTypeMatrixDims(fType) == SkISize::Make(2, 2));
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWriteMatrix2x2fv(n, matrices);
}

void GrUniformWriter::writeMatrix3x3fv(int index, int n, const float matrices[]) {
    AutoSetUniform asu(this, index);
    SkASSERT(GrSLTypeIsFloatType(fType));
    SkASSERT(GrSLTypeMatrixDims(fType) == SkISize::Make(3, 3));
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWriteMatrix3x3fv(n, matrices);
}

void GrUniformWriter::writeMatrix4x4fv(int index, int n, const float matrices[]) {
    AutoSetUniform asu(this, index);
    SkASSERT(GrSLTypeIsFloatType(fType));
    SkASSERT(GrSLTypeMatrixDims(fType) == SkISize::Make(4, 4));
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWriteMatrix4x4fv(n, matrices);
}

void GrUniformWriter::writeSkMatrix(int index, const SkMatrix& matrix) {
    float mt[] = {
            matrix.get(SkMatrix::kMScaleX),
            matrix.get(SkMatrix::kMSkewY),
            matrix.get(SkMatrix::kMPersp0),
            matrix.get(SkMatrix::kMSkewX),
            matrix.get(SkMatrix::kMScaleY),
            matrix.get(SkMatrix::kMPersp1),
            matrix.get(SkMatrix::kMTransX),
            matrix.get(SkMatrix::kMTransY),
            matrix.get(SkMatrix::kMPersp2),
    };
    this->writeMatrix3x3f(index, mt);
}

void GrUniformWriter::writeSkM44(int index, const SkM44& matrix) {
    this->writeMatrix4x4f(index, SkMatrixPriv::M44ColMajor(matrix));
}

void GrUniformWriter::writeRect(int index, const SkRect& r) {
    this->write4f(index, r.left(), r.top(), r.right(), r.bottom());
}

void GrGLSLProgramDataManager::setSkMatrix(UniformHandle u, const SkMatrix& matrix) const {
    float mt[] = {
        matrix.get(SkMatrix::kMScaleX),
        matrix.get(SkMatrix::kMSkewY),
        matrix.get(SkMatrix::kMPersp0),
        matrix.get(SkMatrix::kMSkewX),
        matrix.get(SkMatrix::kMScaleY),
        matrix.get(SkMatrix::kMPersp1),
        matrix.get(SkMatrix::kMTransX),
        matrix.get(SkMatrix::kMTransY),
        matrix.get(SkMatrix::kMPersp2),
    };
    this->setMatrix3f(u, mt);
}

void GrGLSLProgramDataManager::setSkM44(UniformHandle u, const SkM44& matrix) const {
    this->setMatrix4f(u, SkMatrixPriv::M44ColMajor(matrix));
}
