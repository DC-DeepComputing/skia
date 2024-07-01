/*
* Copyright 2024 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SKFONTSCANNER_H_
#define SKFONTSCANNER_H_

#include "include/core/SkFontArguments.h"
#include "include/core/SkFontParameters.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTArray.h"
class SkFontStyle;
class SkStreamAsset;
class SkString;

class SkFontFileData {
public:
    typedef skia_private::STArray<4, SkFontParameters::Variation::Axis, true> AxisDefinitions;
    typedef skia_private::STArray<4, SkScalar, true> AxisValues;
    virtual ~SkFontFileData() = default;
    virtual bool scanFace(SkStreamAsset* stream,
                          int faceIndex,
                          int* numInstances,
                          AxisDefinitions* axisDefinitions) = 0;
    /* instanceIndex 0 is the default instance, 1 to numInstances are the named instances. */
    virtual bool scanInstance(SkStreamAsset* stream,
                              int faceIndex,
                              int instanceIndex,
                              SkString* name,
                              SkFontStyle* style,
                              bool* isFixedPitch,
                              AxisValues* axes) = 0;
    virtual bool computeAxisValues(
            AxisDefinitions& axisDefinitions,
            const SkFontArguments::VariationPosition position,
            SkFixed* axisValues,
            const SkString& name,
            SkFontStyle* style,
            const SkFontArguments::VariationPosition::Coordinate* currentPosition = nullptr) = 0;
};

class SkFontScanner : public SkNoncopyable {
public:
    virtual ~SkFontScanner() = default;
    virtual std::unique_ptr<SkFontFileData> scanFile(SkStreamAsset* stream, int* numFaces) const = 0;
};

#endif // SKFONTSCANNER_H_
