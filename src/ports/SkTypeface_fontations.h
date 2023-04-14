/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypeface_Fontations_DEFINED
#define SkTypeface_Fontations_DEFINED

#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkScalerContext.h"

#include <memory>

class SkStreamAsset;

/** SkTypeface implementation based on Google Fonts Fontations Rust libraries. */
class SkTypeface_Fontations : public SkTypeface {
public:
    SkTypeface_Fontations(std::unique_ptr<SkStreamAsset> font_data)
            : SkTypeface(SkFontStyle(), true), fFontData(std::move(font_data)) {}

protected:

    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override { return nullptr; }
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        return sk_ref_sp(this);
    }
    std::unique_ptr<SkScalerContext> onCreateScalerContext(const SkScalerContextEffects& effects,
                                                           const SkDescriptor* desc) const override;
    void onFilterRec(SkScalerContextRec*) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override {
        return nullptr;
    }
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override {}
    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override;
    int onCountGlyphs() const override { return 1; }
    void getPostScriptGlyphNames(SkString*) const override {}
    void getGlyphToUnicodeMap(SkUnichar*) const override {}
    int onGetUPEM() const override;
    class EmptyLocalizedStrings : public SkTypeface::LocalizedStrings {
    public:
        bool next(SkTypeface::LocalizedString*) override { return false; }
    };
    void onGetFamilyName(SkString* familyName) const override { familyName->reset(); }
    bool onGetPostScriptName(SkString*) const override { return false; }
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override {
        return new EmptyLocalizedStrings;
    }
    bool onGlyphMaskNeedsCurrentColor() const override { return false; }
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override {
        return 0;
    }
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                       int parameterCount) const override {
        return 0;
    }
    int onGetTableTags(SkFontTableTag tags[]) const override { return 0; }
    size_t onGetTableData(SkFontTableTag, size_t, size_t, void*) const override { return 0; }
 private:
    std::unique_ptr<SkStreamAsset> fFontData;
};

#endif  // SkTypeface_Fontations_DEFINED
