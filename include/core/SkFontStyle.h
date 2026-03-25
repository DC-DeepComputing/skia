/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontStyle_DEFINED
#define SkFontStyle_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/base/SkTPin.h"

#include <cstdint>

class SK_API SkFontStyle {
public:
    enum Weight {
        kInvisible_Weight   =    0,
        kThin_Weight        =  100,
        kExtraLight_Weight  =  200,
        kLight_Weight       =  300,
        kNormal_Weight      =  400,
        kMedium_Weight      =  500,
        kSemiBold_Weight    =  600,
        kBold_Weight        =  700,
        kExtraBold_Weight   =  800,
        kBlack_Weight       =  900,
        kExtraBlack_Weight  = 1000,
    };

    enum Width {
        kUltraCondensed_Width   = 1,
        kExtraCondensed_Width   = 2,
        kCondensed_Width        = 3,
        kSemiCondensed_Width    = 4,
        kNormal_Width           = 5,
        kSemiExpanded_Width     = 6,
        kExpanded_Width         = 7,
        kExtraExpanded_Width    = 8,
        kUltraExpanded_Width    = 9,
    };

    enum Slant : uint8_t {
        kUpright_Slant,
        kItalic_Slant,
        kOblique_Slant,
    };

    enum Synthetic : uint8_t {
        kAllowed_Synthetic, // On request allow synthetics for better match, on report means unknown
        kNo_Synthetic, // On request no synthetics if possible, on report means not synthetic
        kYes_Synthetic, // On request prefer synthetic (usually to match), on report means synthetic
    };

    constexpr SkFontStyle(int weight, int width, Slant slant)
        : SkFontStyle(weight, width, slant, kAllowed_Synthetic, kAllowed_Synthetic)
    { }

    constexpr SkFontStyle(int weight, int width, Slant slant, Synthetic bold, Synthetic oblique)
    : fValue(
        (SkTPin<int>(weight, kInvisible_Weight, kExtraBlack_Weight)) +
        (SkTPin<int>(width, kUltraCondensed_Width, kUltraExpanded_Width) << 16) +
        (SkTPin<int>(slant, kUpright_Slant, kOblique_Slant) << 24) +
        (SkTPin<int>(bold, kAllowed_Synthetic, kYes_Synthetic) << 28) +
        (SkTPin<int>(oblique, kAllowed_Synthetic, kYes_Synthetic) << 30)
     ) { }

    constexpr SkFontStyle() : SkFontStyle(kNormal_Weight, kNormal_Width, kUpright_Slant) { }

    bool operator==(const SkFontStyle& rhs) const {
        return fValue == rhs.fValue;
    }

    int weight() const { return fValue & 0xFFFF; }
    int width() const { return (fValue >> 16) & 0xFF; }
    Slant slant() const { return (Slant)((fValue >> 24) & 0xF); }
    Synthetic syntheticBold() const { return (Synthetic)(fValue >> 28 & 0x7); }
    Synthetic syntheticOblique() const { return (Synthetic)(fValue >> 30 & 0x7); }

    static constexpr SkFontStyle Normal() {
        return SkFontStyle(kNormal_Weight, kNormal_Width, kUpright_Slant);
    }
    static constexpr SkFontStyle Bold() {
        return SkFontStyle(kBold_Weight,   kNormal_Width, kUpright_Slant);
    }
    static constexpr SkFontStyle Italic() {
        return SkFontStyle(kNormal_Weight, kNormal_Width, kItalic_Slant );
    }
    static constexpr SkFontStyle BoldItalic() {
        return SkFontStyle(kBold_Weight,   kNormal_Width, kItalic_Slant );
    }

private:
    friend class SkTypefaceProxyPrototype;  // To serialize fValue
    int32_t fValue;
};

#endif
