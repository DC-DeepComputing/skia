/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterPipeline.h"
#include "src/shaders/SkTransformShader.h"

SkTransformShader::SkTransformShader(const SkShaderBase& shader, bool allowPerspective)
        : fShader{shader}
        , fAllowPerspective{allowPerspective} {
    SkMatrix::I().get9(fMatrixStorage);
}

skvm::Color SkTransformShader::onProgram(skvm::Builder* b,
                      skvm::Coord device, skvm::Coord local, skvm::Color color,
                      const SkMatrixProvider& matrices, const SkMatrix* localM,
                      const SkColorInfo& dst,
                      skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    // We assume the caller is folding the CTM into the per-update matrix for effeciency.
    SkASSERT(matrices.localToDevice().isIdentity());

    auto matrix = uniforms->pushPtr(&fMatrixStorage);

    skvm::F32 x = local.x,
              y = local.y;

    auto dot = [&,x,y](int row) {
        return b->mad(x, b->arrayF(matrix, 3*row+0),
                      b->mad(y, b->arrayF(matrix, 3*row+1),
                             b->arrayF(matrix, 3*row+2)));
    };

    x = dot(0);
    y = dot(1);
    if (fAllowPerspective) {
        x = x * (1.0f / dot(2));
        y = y * (1.0f / dot(2));
    }

    skvm::Coord newLocal = {x, y};
    SkMatrixProvider matrixProvider{SkMatrix::I()};
    return fShader.program(
            b, device, newLocal, color, matrixProvider, localM, dst, uniforms, alloc);
}

bool SkTransformShader::update(const SkMatrix& matrix) {
    if (SkMatrix inv; matrix.invert(&inv)) {
        if (!fAllowPerspective && inv.hasPerspective()) {
            return false;
        }

        inv.get9(fMatrixStorage);
        return true;
    }
    return false;
}

bool SkTransformShader::onAppendStages(const SkStageRec& rec, const LocalMatrixRec& lmRec) const {
    // We assume the caller is folding the CTM into the per-update matrix for effeciency.
    SkASSERT(rec.fMatrixProvider.localToDevice().isIdentity() &&
             !lmRec.hasPendingLocalMatrix());
    auto type = fAllowPerspective ? SkRasterPipelineOp::matrix_perspective :
                                    SkRasterPipelineOp::matrix_2x3;
    rec.fPipeline->append(type, fMatrixStorage);

    // We didn't incorporate fMatrixStorage into the total matrix and it's contents can change
    // between runs of the pipeline.
    LocalMatrixRec newLMRec = lmRec;
    newLMRec.markTotalMatrixInvalid();
    fShader.onAppendStages(rec, newLMRec);
    return true;
}
