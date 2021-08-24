/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLProgramDataManager_DEFINED
#define GrGLSLProgramDataManager_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkNoncopyable.h"
#include "src/gpu/GrResourceHandle.h"

#include <tuple>

class SkMatrix;
class SkM44;

#include "include/private/GrTypesPriv.h"

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

    virtual std::tuple<GrSLType, int> willSetUniform(int index) = 0;
    virtual void didSetUniform() {}

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

    GrSLType type() const { return fType; }
    /**
     * Will return GrShaderVar::kNonArray if the uniform is not an array to disambiguate
     * e.g. float3[1] and float3.
     */
    int arrayCount() const { return fN; }

    GrSLType fType = kVoid_GrSLType;
    int fN = -1;
};

/** Manages the resources used by a shader program.
 * The resources are objects the program uses to communicate with the
 * application code.
 */
class GrGLSLProgramDataManager : SkNoncopyable {
public:
    GR_DEFINE_RESOURCE_HANDLE_CLASS(UniformHandle);

    virtual ~GrGLSLProgramDataManager() {}

    /** Functions for uploading uniform values. The varities ending in v can be used to upload to an
     *  array of uniforms. arrayCount must be <= the array count of the uniform.
     */
    virtual void set1i(UniformHandle, int32_t) const = 0;
    virtual void set1iv(UniformHandle, int arrayCount, const int v[]) const = 0;
    virtual void set1f(UniformHandle, float v0) const = 0;
    virtual void set1fv(UniformHandle, int arrayCount, const float v[]) const = 0;
    virtual void set2i(UniformHandle, int32_t, int32_t) const = 0;
    virtual void set2iv(UniformHandle, int arrayCount, const int v[]) const = 0;
    virtual void set2f(UniformHandle, float, float) const = 0;
    virtual void set2fv(UniformHandle, int arrayCount, const float v[]) const = 0;
    virtual void set3i(UniformHandle, int32_t, int32_t, int32_t) const = 0;
    virtual void set3iv(UniformHandle, int arrayCount, const int v[]) const = 0;
    virtual void set3f(UniformHandle, float, float, float) const = 0;
    virtual void set3fv(UniformHandle, int arrayCount, const float v[]) const = 0;
    virtual void set4i(UniformHandle, int32_t, int32_t, int32_t, int32_t) const = 0;
    virtual void set4iv(UniformHandle, int arrayCount, const int v[]) const = 0;
    virtual void set4f(UniformHandle, float, float, float, float) const = 0;
    virtual void set4fv(UniformHandle, int arrayCount, const float v[]) const = 0;
    // matrices are column-major, the first three upload a single matrix, the latter three upload
    // arrayCount matrices into a uniform array.
    virtual void setMatrix2f(UniformHandle, const float matrix[]) const = 0;
    virtual void setMatrix3f(UniformHandle, const float matrix[]) const = 0;
    virtual void setMatrix4f(UniformHandle, const float matrix[]) const = 0;
    virtual void setMatrix2fv(UniformHandle, int arrayCount, const float matrices[]) const = 0;
    virtual void setMatrix3fv(UniformHandle, int arrayCount, const float matrices[]) const = 0;
    virtual void setMatrix4fv(UniformHandle, int arrayCount, const float matrices[]) const = 0;

    // convenience method for uploading a SkMatrix to a 3x3 matrix uniform
    void setSkMatrix(UniformHandle, const SkMatrix&) const;
    // convenience method for uploading a SkMatrix to a 4x4 matrix uniform
    void setSkM44(UniformHandle, const SkM44&) const;

protected:
    GrGLSLProgramDataManager() {}

private:
    using INHERITED = SkNoncopyable;
};

#endif
