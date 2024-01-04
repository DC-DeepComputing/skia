/*
* Copyright 2024 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SKFONTSCANNER_H_
#define SKFONTSCANNER_H_

#include "include/core/SkFontArguments.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTArray.h"
class SkFontStyle;
class SkStreamAsset;
class SkString;

class FontScanner : public SkNoncopyable {
public:
    virtual ~FontScanner() = default;
    struct AxisDefinition {
        SkFourByteTag fTag;
        SkFixed fMinimum;
        SkFixed fDefault;
        SkFixed fMaximum;
    };
    typedef skia_private::STArray<4, AxisDefinition, true> AxisDefinitions;
    virtual bool recognizedFont(SkStreamAsset* stream, int* numFonts) const = 0;
    virtual bool scanFont(SkStreamAsset* stream, int ttcIndex,
                          SkString* name, SkFontStyle* style, bool* isFixedPitch,
                          AxisDefinitions* axes) const = 0;
    static void computeAxisValues(
            AxisDefinitions axisDefinitions,
            const SkFontArguments::VariationPosition position,
            SkFixed* axisValues,
            const SkString& name,
            const SkFontArguments::VariationPosition::Coordinate* currentPosition = nullptr);
};

#endif // SKFONTSCANNER_H_
