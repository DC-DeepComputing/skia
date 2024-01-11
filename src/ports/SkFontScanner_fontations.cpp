/*
 * Copyright 2024 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBBHFactory.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkTSearch.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkFDot6.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkFontScanner.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskGamma.h"
#include "src/core/SkScalerContext.h"
#include "src/ports/SkFontHost_FreeType_common.h"
#include "src/sfnt/SkOTUtils.h"
#include "src/sfnt/SkSFNTHeader.h"
#include "src/sfnt/SkTTCFHeader.h"
#include "src/utils/SkCallableTraits.h"
#include "src/utils/SkMatrix22.h"

#include "src/ports/SkFontScanner_fontations.h"
#include "src/ports/SkTypeface_fontations_priv.h"
#include "src/ports/fontations/src/skpath_bridge.h"

#include <memory>
#include <optional>
#include <tuple>

using namespace skia_private;

#if 1
    #define LOG_INFO(...)
#else
    #define LOG_INFO SkDEBUGF
#endif

namespace {
rust::Box<::fontations_ffi::BridgeFontRef> make_bridge_font_ref(sk_sp<SkData> fontData,
                                                                uint32_t index) {
    rust::Slice<const uint8_t> slice{fontData->bytes(), fontData->size()};
    return fontations_ffi::make_font_ref(slice, index);
}
}

SkFontScanner_Fontations::SkFontScanner_Fontations() {
}

SkFontScanner_Fontations::~SkFontScanner_Fontations() {
}

bool SkFontScanner_Fontations::recognizedFont(SkStreamAsset* stream, int* numFaces) const {
    sk_sp<SkData> fontData = SkData::MakeFromStream(stream, stream->getLength());
    rust::Slice<const uint8_t> slice{fontData->bytes(), fontData->size()};
    ::std::uint32_t num_fonts;
    if (!fontations_ffi::font_or_collection(slice, num_fonts)) {
        return false;
    }
    *numFaces = num_fonts == 0 ? 1 : num_fonts;
    return true;
}

// Assuming default values for everything we don't have right now
bool SkFontScanner_Fontations::scanFont(
        SkStreamAsset* stream, int ttcIndex,
        SkString* name, SkFontStyle* style, bool* isFixedPitch, AxisDefinitions* axes) const
{
    rust::Box<fontations_ffi::BridgeFontRef> fontRef =
            make_bridge_font_ref(SkData::MakeFromStream(stream, stream->getLength()), ttcIndex);
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

