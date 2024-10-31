/*
* Copyright 2024 The Android Open Source Project
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkFontMgr_proxy_DEFINED
#define SkFontMgr_proxy_DEFINED

#include "include/core/SkDataTable.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontScanner.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTDArray.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkThreadAnnotations.h"
#include "src/base/SkTSort.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkTypefaceCache.h"

#include <string.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

class SkTypeface_proxy;
class SkScalerContext_proxy : public SkScalerContext {
public:
    SkScalerContext_proxy(std::unique_ptr<SkScalerContext> proxy,
                          sk_sp<SkTypeface_proxy> typeface,
                          const SkScalerContextEffects& effects,
                          const SkDescriptor* desc);

    ~SkScalerContext_proxy() override = default;
protected:
    SkScalerContext::GlyphMetrics generateMetrics(const SkGlyph& glyph, SkArenaAlloc* alloc) override;
    void generateImage(const SkGlyph& glyph, void* imageBuffer) override;
    bool generatePath(const SkGlyph& glyph, SkPath* path, bool* modified) override ;
    sk_sp<SkDrawable> generateDrawable(const SkGlyph& glyph) override;
    void generateFontMetrics(SkFontMetrics* metrics) override ;
private:
    std::unique_ptr<SkScalerContext> fProxy;
};

class SkTypeface_proxy : public SkTypeface {
public:
    SkTypeface_proxy(const SkFontStyle& style, bool isFixedPitch = false)
        : SkTypeface(style, isFixedPitch)
        , fProxy(nullptr) { }

    protected:
        int onGetUPEM() const override;
        std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;
        sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override;
        bool onGlyphMaskNeedsCurrentColor() const override;
        int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                         int coordinateCount) const override;
        int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                           int parameterCount) const override;
        void onGetFamilyName(SkString* familyName) const override;
        bool onGetPostScriptName(SkString* postScriptName) const override;
        SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;
        int onGetTableTags(SkFontTableTag tags[]) const override;
        size_t onGetTableData(SkFontTableTag tag, size_t offset, size_t length, void* data) const override;
        std::unique_ptr<SkScalerContext> onCreateScalerContext(
                const SkScalerContextEffects& effects, const SkDescriptor* desc) const override;
        void onFilterRec(SkScalerContextRec* rec) const override;
        void onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const override;
        void getGlyphToUnicodeMap(SkUnichar* unichar) const override;
        void getPostScriptGlyphNames(SkString* glyphNames) const override;
        std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;
        void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override;
        int onCountGlyphs() const override;
        void* onGetCTFontRef() const override;
        bool onGetKerningPairAdjustments(const uint16_t glyphs[],
                                         int count,
                                         int32_t adjustments[]) const override;

    protected:
    sk_sp<SkTypeface> fProxy;
};

#endif // SkFontMgr_proxy_DEFINED
