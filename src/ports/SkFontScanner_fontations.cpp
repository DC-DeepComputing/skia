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

using namespace skia_private;

namespace {
rust::Box<::fontations_ffi::BridgeFontRef> make_bridge_font_ref(sk_sp<SkData> fontData,
                                                                uint32_t index) {
    rust::Slice<const uint8_t> slice{fontData->bytes(), fontData->size()};
    return fontations_ffi::make_font_ref(slice, index);
}
}

class SkFontFileData_Fontations : public SkFontFileData {
public:
    typedef skia_private::AutoSTMalloc<4, SkFixed> AxisFixedValues;

    SkFontFileData_Fontations(int numFaces, sk_sp<SkData> fontData)
        : fNumFaces(numFaces)
        , fFontData(std::move(fontData)) { }
    ~SkFontFileData_Fontations() override = default;
    bool scanFace(SkStreamAsset* stream, int faceIndex, int* numInstances) override;
    /* instanceIndex 0 is the default instance, 1 to numInstances are the named instances. */
    bool scanInstance(SkStreamAsset* stream,
                      int faceIndex,
                      int instanceIndex,
                      SkString* name,
                      SkFontStyle* style,
                      bool* isFixedPitch,
                      AxisValues* axes) override;
    bool computeAxisValues(
            const SkFontArguments::VariationPosition position,
            SkFixed* axisValues,
            const SkString& name,
            const SkFontArguments::VariationPosition::Coordinate* currentPosition = nullptr) override {
        // TODO: Implement
        SkDEBUGFAIL("Not implemented");
    }
    int getNumFaces() const override { return fNumFaces; }
    AxisDefinitions getAxisDefinitions() const override { return fAxisDefinitions; }

private:
    int fNumFaces;
    sk_sp<SkData> fFontData;
    AxisDefinitions fAxisDefinitions;
};

bool SkFontFileData_Fontations::scanFace(SkStreamAsset* stream, int faceIndex, int* numInstances) {

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

std::unique_ptr<SkFontFileData> SkFontScanner_Fontations::scanFile(SkStreamAsset* stream) const {
    sk_sp<SkData> fontData = SkData::MakeFromStream(stream, stream->getLength());
    stream->rewind();
    rust::Slice<const uint8_t> slice{fontData->bytes(), fontData->size()};
    ::std::uint32_t num_fonts;
    if (!fontations_ffi::font_or_collection(slice, num_fonts)) {
        return nullptr;
    }

    return std::make_unique<SkFontFileData_Fontations>(num_fonts == 0 ? 1 : num_fonts, std::move(fontData));
}
