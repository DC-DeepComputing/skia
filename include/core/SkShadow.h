/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShadow_DEFINED
#define SkShadow_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkScalar.h"

/**
 * SkShadow holds all attributes required to render a shadow.
 */
class SK_API SkShadow : public SkFlattenable {
public:
    SkShadow(SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY, SkColor4f color)
            : fDx(dx), fDy(dy), fSigmaX(sigmaX), fSigmaY(sigmaY), fColor(color) {}

    SkScalar dx() const { return fDx; }
    SkScalar dy() const { return fDy; }
    SkScalar sigmaX() const { return fSigmaX; }
    SkScalar sigmaY() const { return fSigmaY; }
    SkColor4f color() const { return fColor; }

    SkFlattenable::Type getFlattenableType() const override {
        return SkFlattenable::Type::kSkShadow_Type;
    }

    SK_FLATTENABLE_HOOKS(SkShadow)

private:
    using INHERITED = SkFlattenable;

    void flatten(SkWriteBuffer& buffer) const override;

    SkScalar fDx;
    SkScalar fDy;
    SkScalar fSigmaX;
    SkScalar fSigmaY;
    SkColor4f fColor;
};

#endif
