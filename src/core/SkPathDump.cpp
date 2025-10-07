/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkStream.h"
#include "src/base/SkFloatBits.h"
#include "src/core/SkStringUtils.h"

char const * const gFillTypeStrs[] = {
    "Winding",
    "EvenOdd",
    "InverseWinding",
    "InverseEvenOdd",
};

static void append_params(SkString* str, const char label[], const SkPoint pts[],
                          int count, SkScalarAsStringType strType,
                          bool useSemicolon,
                          SkScalar conicWeight = -12345) {
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
    str->append(useSemicolon ? ");" : ")");
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

void SkPath::dump(SkWStream* wStream, bool dumpAsHex) const {
    SkScalarAsStringType asType = dumpAsHex ? kHex_SkScalarAsStringType : kDec_SkScalarAsStringType;

    SkString builder;
    char const * const gFillTypeStrs[] = {
        "Winding",
        "EvenOdd",
        "InverseWinding",
        "InverseEvenOdd",
    };
    builder.printf("path.setFillType(SkPathFillType::k%s);\n",
            gFillTypeStrs[(int) this->getFillType()]);

    Iter iter(*this, false);
    while (auto rec = iter.next()) {
        switch (rec->fVerb) {
            case SkPathVerb::kMove:
                append_params(&builder, "path.moveTo", &rec->fPoints[0], 1, asType, true);
                break;
            case SkPathVerb::kLine:
                append_params(&builder, "path.lineTo", &rec->fPoints[1], 1, asType, true);
                break;
            case SkPathVerb::kQuad:
                append_params(&builder, "path.quadTo", &rec->fPoints[1], 2, asType, true);
                break;
            case SkPathVerb::kConic:
                append_params(&builder, "path.conicTo", &rec->fPoints[1], 2, asType, true,
                              rec->conicWeight());
                break;
            case SkPathVerb::kCubic:
                append_params(&builder, "path.cubicTo", &rec->fPoints[1], 3, asType, true);
                break;
            case SkPathVerb::kClose:
                builder.append("path.close();\n");
                break;
        }
        if (!wStream && builder.size()) {
            SkDebugf("%s", builder.c_str());
            builder.reset();
        }
    }
    if (wStream) {
        wStream->writeText(builder.c_str());
    }
}

void SkPath::dumpArrays(SkWStream* wStream, bool dumpAsHex) const {
    SkString builder;

    auto bool_str = [](bool v) { return v ? "true" : "false"; };

    builder.appendf("// fBoundsIsDirty = %s\n", bool_str(fPathRef->fBoundsIsDirty));
    builder.appendf("// fGenerationID = %u\n", fPathRef->fGenerationID);
    builder.appendf("// fSegmentMask = %d\n", fPathRef->fSegmentMask);

    const char* gTypeStrs[] = {
        "General", "Oval", "RRect",
    };
    builder.appendf("// fType = %s\n", gTypeStrs[static_cast<int>(fPathRef->fType)]);

    auto append_scalar = [&](SkScalar v) {
        if (dumpAsHex) {
            builder.appendf("SkBits2Float(0x%08X) /* %g */", SkFloat2Bits(v), v);
        } else {
            builder.appendf("%g", v);
        }
    };

    builder.append("const SkPoint path_points[] = {\n");
    for (int i = 0; i < this->countPoints(); ++i) {
        SkPoint p = this->getPoint(i);
        builder.append("    { ");
        append_scalar(p.fX);
        builder.append(", ");
        append_scalar(p.fY);
        builder.append(" },\n");
    }
    builder.append("};\n");

    const char* gVerbStrs[] = {
        "Move", "Line", "Quad", "Conic", "Cubic", "Close"
    };
    builder.append("const uint8_t path_verbs[] = {\n    ");
    for (auto v = fPathRef->verbsBegin(); v != fPathRef->verbsEnd(); ++v) {
        builder.appendf("(uint8_t)SkPathVerb::k%s, ", gVerbStrs[(unsigned)*v]);
    }
    builder.append("\n};\n");

    const int nConics = fPathRef->conicWeightsEnd() - fPathRef->conicWeights();
    if (nConics) {
        builder.append("const SkScalar path_conics[] = {\n    ");
        for (auto c = fPathRef->conicWeights(); c != fPathRef->conicWeightsEnd(); ++c) {
            append_scalar(*c);
            builder.append(", ");
        }
        builder.append("\n};\n");
    }

    char const * const gFillTypeStrs[] = {
        "Winding",
        "EvenOdd",
        "InverseWinding",
        "InverseEvenOdd",
    };

    builder.appendf("SkPath path = SkPath::Make(path_points, %d, path_verbs, %d, %s, %d,\n",
                    this->countPoints(), this->countVerbs(),
                    nConics ? "path_conics" : "nullptr", nConics);
    builder.appendf("                           SkPathFillType::k%s, %s);\n",
                    gFillTypeStrs[(int)this->getFillType()],
                    bool_str(fIsVolatile));

    if (wStream) {
        wStream->writeText(builder.c_str());
    } else {
        SkDebugf("%s\n", builder.c_str());
    }
}

///////////////

SkString SkPathBuilder::dumpToString(DumpFormat format) const {
    SkScalarAsStringType asType = format == DumpFormat::kHex ? kHex_SkScalarAsStringType
                                                             : kDec_SkScalarAsStringType;

    SkString builder;
    builder.printf("SkPathBuilder(SkPathFillType::k%s)\n",
                   gFillTypeStrs[(int) this->fillType()]);

    for (auto iter = this->iter(); auto rec = iter.next(); ) {
        switch (rec->fVerb) {
            case SkPathVerb::kMove:
                append_params(&builder, ".moveTo", &rec->fPoints[0], 1, asType, false);
                break;
            case SkPathVerb::kLine:
                append_params(&builder, ".lineTo", &rec->fPoints[1], 1, asType, false);
                break;
            case SkPathVerb::kQuad:
                append_params(&builder, ".quadTo", &rec->fPoints[1], 2, asType, false);
                break;
            case SkPathVerb::kConic:
                append_params(&builder, ".conicTo", &rec->fPoints[1], 2, asType, false,
                              rec->conicWeight());
                break;
            case SkPathVerb::kCubic:
                append_params(&builder, ".cubicTo", &rec->fPoints[1], 3, asType, false);
                break;
            case SkPathVerb::kClose:
                builder.append(".close()\n");
                break;
        }
    }
    return builder;
}

void SkPathBuilder::dump(DumpFormat format) const {
    SkDebugf("%s", dumpToString().c_str());
}
