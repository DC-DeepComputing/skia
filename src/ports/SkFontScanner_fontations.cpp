/*
 * Copyright 2024 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkFontScanner.h"
#include "src/sfnt/SkOTUtils.h"

#include "src/ports/SkFontScanner_fontations.h"
#include "src/ports/SkTypeface_fontations_priv.h"
#include "src/ports/fontations/src/skpath_bridge.h"

#if 1
    #define LOG_INFO(...)
#else
    #define LOG_INFO SkDEBUGF
#endif

using namespace skia_private;

namespace {
rust::Box<::fontations_ffi::BridgeFontRef> make_bridge_font_ref(sk_sp<SkData> fontData,
                                                                uint32_t index) {
    rust::Slice<const uint8_t> slice{fontData->bytes(), fontData->size()};
    return fontations_ffi::make_font_ref(slice, index);
}
}

std::unique_ptr<SkFontFileData> SkFontScanner_Fontations::scanFile(SkStreamAsset* stream, int* numFaces) const {
    sk_sp<SkData> fontData = SkData::MakeFromStream(stream, stream->getLength());
    stream->rewind();
    rust::Slice<const uint8_t> slice{fontData->bytes(), fontData->size()};
    ::std::uint32_t num_fonts;
    if (!fontations_ffi::font_or_collection(slice, num_fonts)) {
        return nullptr;
    }
    if (numFaces) {
        *numFaces = num_fonts == 0 ? 1 : num_fonts;
    }
    return std::make_unique<SkFontFileData_Fontations>(this);
}

bool SkFontFileData_Fontations::scanFace(SkStreamAsset* stream,
                                        int faceIndex,
                                        int* numInstances,
                                        AxisDefinitions* axisDefinitions) {
    rust::Box<fontations_ffi::BridgeFontRef> fontRef =
            make_bridge_font_ref(SkData::MakeFromStream(stream, stream->getLength()), faceIndex);
    stream->rewind();
    if (!fontations_ffi::font_ref_is_valid(*fontRef)) {
        return false;
    }

    // TODO: For now assume only the default instance (not variation)
    if (numInstances) {
        *numInstances = 1;
    }
    return true;
}

// TODO: For now assume only the default instance (not variation)
bool SkFontFileData_Fontations::scanInstance(SkStreamAsset* stream,
                                             int faceIndex,
                                             int instanceIndex,
                                             SkString* name,
                                             SkFontStyle* style,
                                             bool* isFixedPitch,
                                             AxisValues* axes) {
    SkASSERT(instanceIndex == 0);
    rust::Box<fontations_ffi::BridgeFontRef> fontRef =
            make_bridge_font_ref(SkData::MakeFromStream(stream, stream->getLength()), faceIndex);
    if (!fontations_ffi::font_ref_is_valid(*fontRef)) {
        return false;
    }

    if (name != nullptr) {
        rust::String readFamilyName = fontations_ffi::family_name(*fontRef);
        *name = SkString(readFamilyName.data(), readFamilyName.size());
    }

    if (style != nullptr) {
        fontations_ffi::BridgeFontStyle fontStyle;
        if (fontations_ffi::get_font_style(*fontRef, fontStyle)) {
            *style = SkFontStyle(fontStyle.weight, fontStyle.width,  (SkFontStyle::Slant)fontStyle.slant);
        } else {
            *style = SkFontStyle::Normal();
        }
    }

    return true;
}

bool SkFontFileData_Fontations::computeAxisValues(
        AxisDefinitions* axisDefinitions,
        const SkFontArguments::VariationPosition position,
        SkFixed* axisValues,
        const SkString& name,
        const SkFontArguments::VariationPosition::Coordinate* currentPosition) {

    for (int i = 0; i < axisDefinitions->size(); ++i) {
        const SkFontParameters::Variation::Axis& axisDefinition = (*axisDefinitions)[i];
        const SkScalar axisMin = SkFixedToScalar(axisDefinition.min);
        const SkScalar axisMax = SkFixedToScalar(axisDefinition.max);

        // Start with the default value.
        axisValues[i] = axisDefinition.def;

        // Then the current value.
        if (currentPosition) {
            for (int j = 0; j < axisDefinitions->size(); ++j) {
                const auto& coordinate = currentPosition[j];
                if (axisDefinition.tag == coordinate.axis) {
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
            if (axisDefinition.tag == coordinate.axis) {
                const SkScalar axisValue = SkTPin(coordinate.value, axisMin, axisMax);
                if (coordinate.value != axisValue) {
                    LOG_INFO("Requested font axis value out of range: "
                            "%s '%c%c%c%c' %f; pinned to %f.\n",
                            name.c_str(),
                            (axisDefinition.tag >> 24) & 0xFF,
                            (axisDefinition.tag >> 16) & 0xFF,
                            (axisDefinition.tag >>  8) & 0xFF,
                            (axisDefinition.tag      ) & 0xFF,
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
                for (int j = 0; j < axisDefinitions->size(); ++j) {
                    if (skTag == (*axisDefinitions)[j].tag) {
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
    return true;
}
