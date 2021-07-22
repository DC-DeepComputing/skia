/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMatrixProvider.h"
#include "src/shaders/SkTexCoordShader.h"

SkTexCoordShader::SkTexCoordShader(const SkShaderBase& shader) : fShader{shader} {}

skvm::Color SkTexCoordShader::onProgram(skvm::Builder* b,
                      skvm::Coord device, skvm::Coord local, skvm::Color color,
                      const SkMatrixProvider& matrices, const SkMatrix* localM,
                      const SkColorInfo& dst,
                      skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    skvm::Coord newLocal = this->applyMatrix(b, matrices.localToDevice(), local, uniforms);
    SkSimpleMatrixProvider matrixProvider{SkMatrix::I()};
    return fShader.program(
            b, device, newLocal, color, matrixProvider, localM, dst, uniforms, alloc);
}

skvm::Coord SkTexCoordShader::applyMatrix(skvm::Builder* b,
                                          const SkMatrix& matrix,
                                          skvm::Coord local,
                                          skvm::Uniforms* uniforms) const {
    skvm::Uniform matrixPtr = uniforms->pushPtr(&fMatrix);
    skvm::F32 matrixVals[9];
    for (int i = 0; i < 9; ++i) {
        matrixVals[i] = b->gatherF(matrixPtr, b->splat(i));
    }

    skvm::F32 x = local.x,
              y = local.y;

    auto dot = [&,x,y](int row) {
        return b->mad(x, matrixVals[3*row+0],
               b->mad(y, matrixVals[3*row+1],
                         matrixVals[3*row+2]));
    };

    x = dot(0);
    y = dot(1);
    if (matrix.hasPerspective() || fShader.getLocalMatrix().hasPerspective()) {
        x = x * (1.0f / dot(2));
        y = y * (1.0f / dot(2));
    }

    return {x, y};
}

bool SkTexCoordShader::update(const SkMatrix& ctm, const SkMatrix* localM, skvm::Uniforms*) const {
    return this->computeTotalInverse(ctm, localM, &fMatrix);
}

bool SkTexCoordShader::onAppendStages(const SkStageRec& rec) const {
    // TODO
    return false;
}
