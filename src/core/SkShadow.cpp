/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkShadow.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"


sk_sp<SkFlattenable> SkShadow::CreateProc(SkReadBuffer& buffer) {
    SkScalar dx = buffer.readScalar();
    SkScalar dy = buffer.readScalar();
    SkScalar sigmaX = buffer.readScalar();
    SkScalar sigmaY = buffer.readScalar();
    SkColor4f color;
    buffer.readColor4f(&color);
    return sk_make_sp<SkShadow>(dx, dy, sigmaX, sigmaY, color);
}

void SkShadow::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fDx);
    buffer.writeScalar(fDy);
    buffer.writeScalar(fSigmaX);
    buffer.writeScalar(fSigmaY);
    buffer.writeColor4f(fColor);
}
