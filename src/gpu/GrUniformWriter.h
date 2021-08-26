/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrUniformWriter_DEFINED
#define GrUniformWriter_DEFINED

#include "include/private/GrTypesPriv.h"

#include <tuple>

class SkMatrix;
class SkM44;
struct SkRect;

/**
 * Interface use by GrProcessors to write their uniform values. Asserts that type implied by
 * the called write function match the actual uniform types.
 */
class GrUniformWriter {
public:
    virtual ~GrUniformWriter() = default;

    // Arrays of scalars and vectors
    void writeiv (int index, int n, const int32_t[]);
    void writefv (int index, int n, const   float[]);
    void write2iv(int index, int n, const int32_t[]);
    void write2fv(int index, int n, const float  []);
    void write3iv(int index, int n, const int32_t[]);
    void write3fv(int index, int n, const float  []);
    void write4iv(int index, int n, const int32_t[]);
    void write4fv(int index, int n, const float  []);

    // Matrices are column-major
    void writeMatrix2x2fv(int index, int n, const float matrices[]);
    void writeMatrix3x3fv(int index, int n, const float matrices[]);
    void writeMatrix4x4fv(int index, int n, const float matrices[]);

    // Convenience method for non-arrays
    void writei(int index, int32_t v) { this->writeiv(index, 1, &v); }
    void writef(int index, float   v) { this->writefv(index, 1, &v); }
    void write2i(int index, int32_t v0, int32_t v1) {
        int32_t v[] = {v0, v1};
        this->write2iv(index, 1, v);
    }
    void write2f(int index, float v0, float v1) {
        float v[] = {v0, v1};
        this->write2fv(index, 1, v);
    }
    void write3i(int index, int32_t v0, int32_t v1, int32_t v2) {
        int32_t v[] = {v0, v1, v2};
        this->write3iv(index, 1, v);
    }
    void write3f(int index, float v0, float v1, float v2) {
        float v[] = {v0, v1, v2};
        this->write3fv(index, 1, v);
    }
    void write4i(int index, int32_t v0, int32_t v1, int32_t v2, int32_t v3) {
        int32_t v[] = {v0, v1, v2, v3};
        this->write4iv(index, 1, v);
    }
    void write4f(int index, float v0, float v1, float v2, float v3) {
        float v[] = {v0, v1, v2, v3};
        this->write4fv(index, 1, v);
    }
    void writeMatrix2x2f(int index, const float matrix[]) {
        this->writeMatrix2x2fv(index, 1, matrix);
    }
    void writeMatrix3x3f(int index, const float matrix[]) {
        this->writeMatrix3x3fv(index, 1, matrix);
    }
    void writeMatrix4x4f(int index, const float matrix[]) {
        this->writeMatrix4x4fv(index, 1, matrix);
    }

    // convenience method for writing a SkMatrix as a 3x3 matrix uniform
    void writeSkMatrix(int index, const SkMatrix&);

    // convenience method for writing a SkM44 as a 4x4 matrix uniform
    void writeSkM44(int index, const SkM44&);

    // convenience method for writing a SkRect as a float4 matrix uniform (LTRB)
    void writeRect(int index, const SkRect&);

protected:
    GrUniformWriter() = default;
    GrUniformWriter(const GrUniformWriter&) = delete;
    GrUniformWriter& operator=(const GrUniformWriter&) = delete;

private:
    class AutoSetUniform;

    /**
     * Will be called before the virtual onWrite methods below. Subclass should find the uniform
     * and if it exists (i.e. wasn't elided in the program) returns its type and arroy size. If it
     * doesn't exist return kVoid as the type.
     */
    virtual std::tuple<GrSLType, int> willSetUniform(int index) = 0;
    /**
     * Called after the virtual onWrite method.
     */
    virtual void didSetUniform() = 0;

    virtual void onWriteiv (int n, const int32_t[]) = 0;
    virtual void onWritefv (int n, const   float[]) = 0;
    virtual void onWrite2iv(int n, const int32_t[]) = 0;
    virtual void onWrite2fv(int n, const float  []) = 0;
    virtual void onWrite3iv(int n, const int32_t[]) = 0;
    virtual void onWrite3fv(int n, const float  []) = 0;
    virtual void onWrite4iv(int n, const int32_t[]) = 0;
    virtual void onWrite4fv(int n, const float  []) = 0;

    virtual void onWriteMatrix2x2fv(int n, const float matrix[]) = 0;
    virtual void onWriteMatrix3x3fv(int n, const float matrix[]) = 0;
    virtual void onWriteMatrix4x4fv(int n, const float matrix[]) = 0;

    GrSLType fType = kVoid_GrSLType;
    int fN = -1;
};

#endif
