/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGPath.h"

#include "include/core/SkCanvas.h"
#include "include/utils/SkParsePath.h"
#include "modules/svg/include/SkSVGAttribute.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGTypes.h"

class SkPaint;
enum class SkPathFillType;

SkSVGPath::SkSVGPath() : INHERITED(SkSVGTag::kPath) { }

bool SkSVGPath::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setPath(SkSVGAttributeParser::parse<SkPath>("d", n, v));
}

template <>
bool SkSVGAttributeParser::parse<SkPath>(SkPath* path) {
    return SkParsePath::FromSVGString(fCurPos, path);
}

void SkSVGPath::onDraw(SkCanvas* canvas, const SkSVGLengthContext&, const SkPaint& paint,
                       SkPathFillType fillType) const {
    // the passed fillType follows inheritance rules and needs to be applied at draw time.
    SkPath path = fPath.makeFillType(fillType);  // Note: point and verb data are CoW
    canvas->drawPath(path, paint);
}

SkPath SkSVGPath::onAsPath(const SkSVGRenderContext& ctx) const {
    // clip-rule can be inherited and needs to be applied at clip time.
    SkPath path = fPath.makeFillType(ctx.presentationContext().fInherited.fClipRule->asFillType());
    this->mapToParent(&path);
    return path;
}

SkRect SkSVGPath::onTransformableObjectBoundingBox(const SkSVGRenderContext& ctx) const {
    return fPath.computeTightBounds();
}
