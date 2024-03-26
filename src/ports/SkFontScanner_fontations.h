/*
* Copyright 2024 The Android Open Source Project
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SKFONTSCANNER_FONTATIONS_H_
#define SKFONTSCANNER_FONTATIONS_H_

#include "include/core/SkSpan.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkSharedMutex.h"
#include "src/core/SkFontScanner.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkScalerContext.h"
#include "src/utils/SkCharToGlyphCache.h"

struct SkAdvancedTypefaceMetrics;
class SkFontDescriptor;
class SkFontData;
class SkFontScanner_Fontations;

class SkFontFileData_Fontations : public SkFontFileData {
 public:
    SkFontFileData_Fontations(const SkFontScanner_Fontations* parent)
        : fParentScanner(parent) { }

    ~SkFontFileData_Fontations() override = default;
    bool scanFace(SkStreamAsset* stream,
                  int faceIndex,
                  int* numInstances,
                  AxisDefinitions* axisDefinitions) override;
    /* instanceIndex 0 is the default instance, 1 to numInstances are the named instances. */
    bool scanInstance(SkStreamAsset* stream,
                      int faceIndex,
                      int instanceIndex,
                      SkString* name,
                      SkFontStyle* style,
                      bool* isFixedPitch,
                      AxisValues* axes) override;
    bool computeAxisValues(
            AxisDefinitions* axisDefinitions,
            const SkFontArguments::VariationPosition position,
            SkFixed* axisValues,
            const SkString& name,
            const SkFontArguments::VariationPosition::Coordinate* currentPosition = nullptr) override;
 private:
    [[maybe_unused]]const SkFontScanner_Fontations* fParentScanner;
};

class SkFontScanner_Fontations : public SkFontScanner {
public:
    SkFontScanner_Fontations() = default;
    ~SkFontScanner_Fontations() override = default;
    std::unique_ptr<SkFontFileData> scanFile(SkStreamAsset* stream, int* numFaces) const override;
};

#endif // SKFONTSCANNER_FONTATIONS_H_
