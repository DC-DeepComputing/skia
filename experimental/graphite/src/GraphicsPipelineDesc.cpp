/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/GraphicsPipelineDesc.h"

namespace skgpu {

GraphicsPipelineDesc::GraphicsPipelineDesc() {}

void GraphicsPipelineDesc::setProgram(TestVertexShader shader, Combination combo) {
    fVertexShader = shader;
    if (fKey.count() >= 1) {
        fKey[0] = static_cast<uint32_t>(shader);
    } else {
        fKey.push_back(static_cast<uint32_t>(shader));
    }

    fCombination = combo;
    if (fKey.count() >= 2) {
        fKey[1] = combo.key();
    } else {
        fKey.push_back(combo.key());
    }

    // TODO: Hard-coded strides based on vertex shader definitions in MtlGraphicsPipeline.mm
    switch(shader) {
        case TestVertexShader::kUnitRectangle: [[fallthrough]];
        case TestVertexShader::kRectangle:
            fVertexStride = 0;
            fInstanceStride = 0;
            break;
        case TestVertexShader::kInstanceRectangle:
            fVertexStride = 0;
            fInstanceStride = 4 * sizeof(float);
            break;
        case TestVertexShader::kVertexTris:
            fVertexStride = 2 * sizeof(float);
            fInstanceStride = 0;
            break;
    }
}

} // namespace skgpu
