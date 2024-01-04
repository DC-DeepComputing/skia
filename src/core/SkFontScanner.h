#ifndef SKFONTSCANNER_H_
#define SKFONTSCANNER_H_

#include "include/core/SkFontArguments.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkTArray.h"

class FontScanner : public SkRefCnt {
public:
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