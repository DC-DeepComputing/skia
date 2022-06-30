/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkEmptyShader.h"

#include "src/core/SkVM.h"

skvm::Color SkEmptyShader::onProgram(skvm::Builder*, skvm::Coord, skvm::Coord, skvm::Color,
                                     const SkMatrixProvider&, const SkMatrix*, const SkColorInfo&,
                                     skvm::Uniforms*, SkArenaAlloc*) const {
    return {};  // signal failure
}

sk_sp<SkFlattenable> SkEmptyShader::CreateProc(SkReadBuffer&) {
    return SkShaders::Empty();
}
