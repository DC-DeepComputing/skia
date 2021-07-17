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

class SkTexCoordShader : public SkUpdatableShader {
public:
    explicit SkTexCoordShader(const SkShaderBase& shader);

    skvm::Color onProgram(skvm::Builder* b,
                          skvm::Coord device, skvm::Coord local, skvm::Color color,
                          const SkMatrixProvider& matrices, const SkMatrix* localM,
                          const SkColorInfo& dst,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override;
    skvm::Coord applyMatrix(
            skvm::Builder* b, const SkMatrix& matrix, skvm::Coord local,
            skvm::Uniforms* uniforms) const;
    bool update(const SkMatrix& ctm, const SkMatrix* localM) const override;
    bool onAppendStages(const SkStageRec& rec) const override;

private:
    // For serialization.  This will never be called.
    Factory getFactory() const override { return nullptr; }
    const char* getTypeName() const override { return nullptr; }

    const SkShaderBase& fShader;
    mutable skvm::Uniforms* fUniforms{nullptr};
    mutable skvm::Uniform fMatrix[9];
};
#endif  //SkTextCoordShader_DEFINED
