/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkTextCoordShader_DEFINED
#define SkTextCoordShader_DEFINED

#include "src/core/SkVM.h"
#include "src/shaders/SkShaderBase.h"

// SkTransformShader allows the transform used by the shader to change without regenerating the
// jitted code. This supports the drawVertices call to change the mapping as the texture
// coordinates associated with each vertex change with each new triangle.
class SkTransformShader : public SkUpdatableShader {
public:
    explicit SkTransformShader(const SkShaderBase& shader);

    // Adds instructions to use the mapping stored in the uniforms represented by fMatrix. After
    // generating a new skvm::Coord, it passes the mapped coordinates to fShader's onProgram
    // along with the identity matrix.
    skvm::Color onProgram(skvm::Builder* builder,
                          skvm::Coord device,
                          skvm::Coord local,
                          skvm::Color paint,
                          const SkMatrixProvider& matrices,
                          const SkMatrix* localM,
                          const SkColorInfo& dstInfo,
                          skvm::UniformsAllocator* uniforms) const override;
    bool onKeyAndUniforms(SkKeyBuilder* key,
                          skvm::UniformsData* uniforms,
                          const SkMatrixProvider& matrices,
                          const SkMatrix* localM,
                          const SkColorInfo& dstInfo,
                          SkArenaAlloc* alloc) const override;

    // Add code to calculate a new coordinate given local using the mapping in fMatrix.
    void appendMatrix(const SkMatrix& matrix, SkRasterPipeline* p) const;

    skvm::Coord applyMatrix(skvm::Builder* b,
                            const SkMatrix& matrix,
                            skvm::Coord local,
                            skvm::UniformsAllocator* uniforms) const;
    void applyKeyAndUniforms(SkKeyBuilder* key,
                             skvm::UniformsData* uniforms,
                             const SkMatrix& matrix) const;

    // Change the values represented by the uniforms in fMatrix.
    bool update(const SkMatrix& ctm) const override;
    bool onAppendStages(const SkStageRec& rec) const override;

    // For serialization. This will never be called.
    Factory getFactory() const override { return nullptr; }
    const char* getTypeName() const override { return "TransformShader"; }

private:
    const SkShaderBase& fShader;
    mutable SkScalar fMatrixStorage[9];
    mutable skvm::Uniform fMatrix;
    mutable bool fProcessingAsPerspective{false};

};
#endif  //SkTextCoordShader_DEFINED
