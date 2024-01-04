/*
* Copyright 2024 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkFontArguments.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkTPin.h"
#include "src/core/SkFontScanner.h"
class SkString;

#if 1
#define LOG_INFO(...)
#else
#define LOG_INFO SkDEBUGF
#endif

/*static*/ void FontScanner::computeAxisValues(
        AxisDefinitions axisDefinitions,
        const SkFontArguments::VariationPosition position,
        SkFixed* axisValues,
        const SkString& name,
        const SkFontArguments::VariationPosition::Coordinate* current)
{
    for (int i = 0; i < axisDefinitions.size(); ++i) {
        const AxisDefinition& axisDefinition = axisDefinitions[i];
        const SkScalar axisMin = SkFixedToScalar(axisDefinition.fMinimum);
        const SkScalar axisMax = SkFixedToScalar(axisDefinition.fMaximum);

        // Start with the default value.
        axisValues[i] = axisDefinition.fDefault;

        // Then the current value.
        if (current) {
            for (int j = 0; j < axisDefinitions.size(); ++j) {
                const auto& coordinate = current[j];
                if (axisDefinition.fTag == coordinate.axis) {
                    const SkScalar axisValue = SkTPin(coordinate.value, axisMin, axisMax);
                    axisValues[i] = SkScalarToFixed(axisValue);
                    break;
                }
            }
        }

        // Then the requested value.
        // The position may be over specified. If there are multiple values for a given axis,
        // use the last one since that's what css-fonts-4 requires.
        for (int j = position.coordinateCount; j --> 0;) {
            const auto& coordinate = position.coordinates[j];
            if (axisDefinition.fTag == coordinate.axis) {
                const SkScalar axisValue = SkTPin(coordinate.value, axisMin, axisMax);
                if (coordinate.value != axisValue) {
                    LOG_INFO("Requested font axis value out of range: "
                            "%s '%c%c%c%c' %f; pinned to %f.\n",
                            name.c_str(),
                            (axisDefinition.fTag >> 24) & 0xFF,
                            (axisDefinition.fTag >> 16) & 0xFF,
                            (axisDefinition.fTag >>  8) & 0xFF,
                            (axisDefinition.fTag      ) & 0xFF,
                            SkScalarToDouble(coordinate.value),
                            SkScalarToDouble(axisValue));
                }
                axisValues[i] = SkScalarToFixed(axisValue);
                break;
            }
        }
        // TODO: warn on defaulted axis?
    }

    SkDEBUGCODE(
            // Check for axis specified, but not matched in font.
            for (int i = 0; i < position.coordinateCount; ++i) {
                SkFourByteTag skTag = position.coordinates[i].axis;
                bool found = false;
                for (int j = 0; j < axisDefinitions.size(); ++j) {
                    if (skTag == axisDefinitions[j].fTag) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    LOG_INFO("Requested font axis not found: %s '%c%c%c%c'\n",
                             name.c_str(),
                             (skTag >> 24) & 0xFF,
                             (skTag >> 16) & 0xFF,
                             (skTag >>  8) & 0xFF,
                             (skTag)       & 0xFF);
                }
            }
    )
}
