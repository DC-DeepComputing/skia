/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathTypes.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkPathRaw.h"

const uint8_t gVerbToSegmentMask[] = {
    0,  // move
    kLine_SkPathSegmentMask,
    kQuad_SkPathSegmentMask,
    kConic_SkPathSegmentMask,
    kCubic_SkPathSegmentMask,
    0,  // close
};

uint8_t SkPathPriv::ComputeSegmentMask(SkSpan<const SkPathVerb> verbs) {
    unsigned mask = 0;
    for (auto v : verbs) {
        unsigned i = static_cast<unsigned>(v);
        SkASSERT(i < std::size(gVerbToSegmentMask));
        mask |= gVerbToSegmentMask[i];
    }
    return SkTo<uint8_t>(mask);
}

std::optional<SkRect> SkPathRaw::isRect() const {
    if (auto rc = SkPathPriv::IsRectContour(fPoints, fVerbs, false)) {
        return rc->fRect;
    }
    return {};
}

/////////////////////////////////////////////////////////////////////////

#include "src/core/SkStringUtils.h"

static void append_params(SkString* str, const char label[], const SkPoint pts[],
                          int count, SkScalarAsStringType strType, SkScalar conicWeight = -12345) {
    str->append(label);
    str->append("(");

    const SkScalar* values = &pts[0].fX;
    count *= 2;

    for (int i = 0; i < count; ++i) {
        SkAppendScalar(str, values[i], strType);
        if (i < count - 1) {
            str->append(", ");
        }
    }
    if (conicWeight != -12345) {
        str->append(", ");
        SkAppendScalar(str, conicWeight, strType);
    }
    str->append(");");
    if (kHex_SkScalarAsStringType == strType) {
        str->append("  // ");
        for (int i = 0; i < count; ++i) {
            SkAppendScalarDec(str, values[i]);
            if (i < count - 1) {
                str->append(", ");
            }
        }
        if (conicWeight >= 0) {
            str->append(", ");
            SkAppendScalarDec(str, conicWeight);
        }
    }
    str->append("\n");
}

void SkPathRaw::dump(bool dumpAsHex) const {
    SkScalarAsStringType asType = dumpAsHex ? kHex_SkScalarAsStringType : kDec_SkScalarAsStringType;

    SkString builder;
    char const * const gFillTypeStrs[] = {
        "Winding",
        "EvenOdd",
        "InverseWinding",
        "InverseEvenOdd",
    };

    const auto r = this->bounds();
    builder.printf("path.setFillType(SkPathFillType::k%s);\n/* bounds: %.9g %.9g %.9g %.9g */\nisConvex: %d\n",
                   gFillTypeStrs[(int) this->fillType()],
                    r.fLeft, r.fTop, r.fRight, r.fBottom,
                    fIsConvex);

    for (auto iter = this->iter(); auto rec = iter.next(); ) {
        switch (rec->fVerb) {
            case SkPathVerb::kMove:
                append_params(&builder, "path.moveTo", &rec->fPoints[0], 1, asType);
                break;
            case SkPathVerb::kLine:
                append_params(&builder, "path.lineTo", &rec->fPoints[1], 1, asType);
                break;
            case SkPathVerb::kQuad:
                append_params(&builder, "path.quadTo", &rec->fPoints[1], 2, asType);
                break;
            case SkPathVerb::kConic:
                append_params(&builder, "path.conicTo", &rec->fPoints[1], 2, asType,
                              rec->conicWeight());
                break;
            case SkPathVerb::kCubic:
                append_params(&builder, "path.cubicTo", &rec->fPoints[1], 3, asType);
                break;
            case SkPathVerb::kClose:
                builder.append("path.close();\n");
                break;
        }
        if (builder.size()) {
            SkDebugf("%s", builder.c_str());
            builder.reset();
        }
    }
}
