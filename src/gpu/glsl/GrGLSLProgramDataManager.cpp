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

void GrUniformWriter::writeiv (int n, const int32_t v[]) {
    SkASSERT(GrSLTypeIsIntegralType(fType));
    SkASSERT(GrSLTypeVecLength(fType) == 1);
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWriteiv(n, v);
}

void GrUniformWriter::writefv(int n, const float v[]) {
    SkASSERT(GrSLTypeIsFloatType(fType));
    SkASSERT(GrSLTypeVecLength(fType) == 1);
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWritefv(n, v);
}

void GrUniformWriter::write2iv(int n, const int32_t v[]) {
    SkASSERT(GrSLTypeIsIntegralType(fType));
    SkASSERT(GrSLTypeVecLength(fType) == 2);
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWrite2iv(n, v);
}

void GrUniformWriter::write2fv(int n, const float v[]) {
    SkASSERT(GrSLTypeIsFloatType(fType));
    SkASSERT(GrSLTypeVecLength(fType) == 2);
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWrite2fv(n, v);
}

void GrUniformWriter::write3iv(int n, const int32_t v[]) {
    SkASSERT(GrSLTypeIsIntegralType(fType));
    SkASSERT(GrSLTypeVecLength(fType) == 3);
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWrite3iv(n, v);
}

void GrUniformWriter::write3fv(int n, const float v[]) {
    SkASSERT(GrSLTypeIsFloatType(fType));
    SkASSERT(GrSLTypeVecLength(fType) == 3);
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWrite3fv(n, v);
}

void GrUniformWriter::write4iv(int n, const int32_t v[]) {
    SkASSERT(GrSLTypeIsIntegralType(fType));
    SkASSERT(GrSLTypeVecLength(fType) == 4);
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWrite4iv(n, v);
}

void GrUniformWriter::write4fv(int n, const float v[]) {
    SkASSERT(GrSLTypeIsFloatType(fType));
    SkASSERT(GrSLTypeVecLength(fType) == 4);
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWrite4fv(n, v);
}

void GrUniformWriter::writeMatrix2x2fv(int n, const float matrices[]) {
    SkASSERT(GrSLTypeIsFloatType(fType));
    SkASSERT(GrSLTypeMatrixDims(fType) == SkISize::Make(2, 2));
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWriteMatrix2x2fv(n, matrices);
}

void GrUniformWriter::writeMatrix3x3fv(int n, const float matrices[]) {
    SkASSERT(GrSLTypeIsFloatType(fType));
    SkASSERT(GrSLTypeMatrixDims(fType) == SkISize::Make(3, 3));
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWriteMatrix3x3fv(n, matrices);
}

void GrUniformWriter::writeMatrix4x4fv(int n, const float matrices[]) {
    SkASSERT(GrSLTypeIsFloatType(fType));
    SkASSERT(GrSLTypeMatrixDims(fType) == SkISize::Make(4, 4));
    SkASSERT(n <= fN || (n == 1 && fN == GrShaderVar::kNonArray));
    this->onWriteMatrix4x4fv(n, matrices);
}

void GrUniformWriter::writeSkMatrix(const SkMatrix& matrix) {
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
    this->writeMatrix3x3f(mt);
}

void GrUniformWriter::writeSkM44(const SkM44& matrix) {
    this->writeMatrix4x4f(SkMatrixPriv::M44ColMajor(matrix));
}

void GrUniformWriter::writeRect(const SkRect& r) {
    this->write4f(r.left(), r.top(), r.right(), r.bottom());
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
