/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkKeyBuilder.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterPipeline.h"
#include "src/shaders/SkTransformShader.h"

SkTransformShader::SkTransformShader(const SkShaderBase& shader) : fShader{shader} {}

skvm::Color SkTransformShader::onProgram(skvm::Builder* builder,
                                         skvm::Coord device,
                                         skvm::Coord local,
                                         skvm::Color paint,
                                         const SkMatrixProvider& matrices,
                                         const SkMatrix* localM,
                                         const SkColorInfo& dstInfo,
                                         skvm::UniformsAllocator* uniforms) const {
    skvm::Coord newLocal = this->applyMatrix(builder, matrices.localToDevice(), local, uniforms);
    SkMatrixProvider matrixProvider{SkMatrix::I()};
    return fShader.program(
            builder, device, newLocal, paint, matrixProvider, localM, dstInfo, uniforms);
}

bool SkTransformShader::onKeyAndUniforms(SkKeyBuilder* key,
                                         skvm::UniformsData* uniforms,
                                         const SkMatrixProvider& matrices,
                                         const SkMatrix* localM,
                                         const SkColorInfo& dstInfo,
                                         SkArenaAlloc* alloc) const {
    this->applyKeyAndUniforms(key, uniforms, matrices.localToDevice());
    SkMatrixProvider matrixProvider{SkMatrix::I()};
    return fShader.keyAndUniforms(key, uniforms, matrixProvider, localM, dstInfo, alloc);
}

skvm::Coord SkTransformShader::applyMatrix(skvm::Builder* b,
                                           const SkMatrix& matrix,
                                           skvm::Coord local,
                                           skvm::UniformsAllocator* uniforms) const {
    fMatrix = uniforms->pushPtr(/*&fMatrixStorage*/);

    skvm::F32 x = local.x,
              y = local.y;

    auto dot = [&,x,y](int row) {
        return b->mad(x, b->arrayF(fMatrix, 3*row+0),
                      b->mad(y, b->arrayF(fMatrix, 3*row+1),
                             b->arrayF(fMatrix, 3*row+2)));
    };

    x = dot(0);
    y = dot(1);
    fProcessingAsPerspective = matrix.hasPerspective() || fShader.getLocalMatrix().hasPerspective();
    if (fProcessingAsPerspective) {
        x = x * (1.0f / dot(2));
        y = y * (1.0f / dot(2));
    }

    return {x, y};
}

void SkTransformShader::applyKeyAndUniforms(SkKeyBuilder* key,
                                            skvm::UniformsData* uniforms,
                                            const SkMatrix& matrix) const {
    fProcessingAsPerspective = matrix.hasPerspective() || fShader.getLocalMatrix().hasPerspective();
    key->addBool(fProcessingAsPerspective, "perspective");
    uniforms->pushPtr(&fMatrixStorage);
}

void SkTransformShader::appendMatrix(const SkMatrix& matrix, SkRasterPipeline* p) const {
    fProcessingAsPerspective = matrix.hasPerspective() || fShader.getLocalMatrix().hasPerspective();
    if (fProcessingAsPerspective) {
        p->append(SkRasterPipeline::matrix_perspective, fMatrixStorage);
    } else {
        p->append(SkRasterPipeline::matrix_2x3, fMatrixStorage);
    }
}

bool SkTransformShader::update(const SkMatrix& ctm) const {
    if (SkMatrix matrix; fShader.computeTotalInverse(ctm, nullptr, &matrix)) {
        if (!fProcessingAsPerspective) {
            SkASSERT(!matrix.hasPerspective());
            if (matrix.hasPerspective()) {
                return false;
            }
        }

        matrix.get9(fMatrixStorage);
        return true;
    }
    return false;
}

bool SkTransformShader::onAppendStages(const SkStageRec& rec) const {
    // TODO
    return false;
}
