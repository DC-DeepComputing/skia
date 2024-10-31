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

#include <fontconfig/fontconfig.h>

#include <string.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

namespace SkFontMgr_utils {

// FontConfig was thread antagonistic until 2.10.91 with known thread safety issues until 2.13.93.
// Before that, lock with a global mutex.
// See https://bug.skia.org/1497 and cl/339089311 for background.
static SkMutex& f_c_mutex() {
    static SkMutex& mutex = *(new SkMutex);
    return mutex;
}

class FCLocker {
    inline static constexpr int FontConfigThreadSafeVersion = 21393;

    // Assume FcGetVersion() has always been thread safe.
    static void lock() SK_NO_THREAD_SAFETY_ANALYSIS {
        if (FcGetVersion() < FontConfigThreadSafeVersion) {
            f_c_mutex().acquire();
        }
    }
    static void unlock() SK_NO_THREAD_SAFETY_ANALYSIS {
        AssertHeld();
        if (FcGetVersion() < FontConfigThreadSafeVersion) {
            f_c_mutex().release();
        }
    }

public:
    FCLocker() { lock(); }
    ~FCLocker() { unlock(); }

    static void AssertHeld() { SkDEBUGCODE(
                if (FcGetVersion() < FontConfigThreadSafeVersion) {
                    f_c_mutex().assertHeld();
                }
        ) }
};

template<typename T, void (*D)(T*)> void FcTDestroy(T* t) {
    FCLocker::AssertHeld();
    D(t);
}
template <typename T, T* (*C)(), void (*D)(T*)> class SkAutoFc
        : public SkAutoTCallVProc<T, FcTDestroy<T, D>> {
    using inherited = SkAutoTCallVProc<T, FcTDestroy<T, D>>;
public:
    SkAutoFc() : SkAutoTCallVProc<T, FcTDestroy<T, D>>( C() ) {
        T* obj = this->operator T*();
        SkASSERT_RELEASE(nullptr != obj);
    }
    explicit SkAutoFc(T* obj) : inherited(obj) {}
    SkAutoFc(const SkAutoFc&) = delete;
    SkAutoFc(SkAutoFc&& that) : inherited(std::move(that)) {}
};

typedef SkAutoFc<FcCharSet, FcCharSetCreate, FcCharSetDestroy> SkAutoFcCharSet;
typedef SkAutoFc<FcConfig, FcConfigCreate, FcConfigDestroy> SkAutoFcConfig;
typedef SkAutoFc<FcFontSet, FcFontSetCreate, FcFontSetDestroy> SkAutoFcFontSet;
typedef SkAutoFc<FcLangSet, FcLangSetCreate, FcLangSetDestroy> SkAutoFcLangSet;
typedef SkAutoFc<FcObjectSet, FcObjectSetCreate, FcObjectSetDestroy> SkAutoFcObjectSet;
typedef SkAutoFc<FcPattern, FcPatternCreate, FcPatternDestroy> SkAutoFcPattern;

bool get_bool(FcPattern* pattern, const char object[], bool missing = false);
int get_int(FcPattern* pattern, const char object[], int missing);
const char* get_string(FcPattern* pattern, const char object[], const char* missing = "");
const FcMatrix* get_matrix(FcPattern* pattern, const char object[]);
int map_range(SkScalar value,
                     SkScalar old_min, SkScalar old_max,
                     SkScalar new_min, SkScalar new_max);
struct MapRanges {
    SkScalar old_val;
    SkScalar new_val;
};
SkScalar map_ranges(SkScalar val, MapRanges const ranges[], int rangesCount);

#ifndef FC_WEIGHT_DEMILIGHT
#define FC_WEIGHT_DEMILIGHT        65
#endif

SkFontStyle skfontstyle_from_fcpattern(FcPattern* pattern);
void fcpattern_from_skfontstyle(SkFontStyle style, FcPattern* pattern);
};

class SkTypeface_proxy : public SkTypeface {
public:
    static sk_sp<SkTypeface_proxy> Make(SkFontMgr_utils::SkAutoFcPattern pattern,
                                        SkString sysroot,
                                        SkFontScanner* scanner);

    mutable SkFontMgr_utils::SkAutoFcPattern fPattern;  // Mutable for passing to FontConfig API.
    const SkString fSysroot;

    void onGetFamilyName(SkString* familyName) const override;
    void onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const override;
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;
    void onFilterRec(SkScalerContextRec* rec) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override;

    std::unique_ptr<SkScalerContext> onCreateScalerContext(const SkScalerContextEffects& effects, const SkDescriptor* descr) const override;

    void getGlyphToUnicodeMap(SkUnichar* glyphToUnicode) const override;
    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override;
    int onCountGlyphs() const override;
    int onGetUPEM() const override;
    bool onGetKerningPairAdjustments(const uint16_t glyphs[], int count,
                                     int32_t adjustments[]) const override;
    bool onGetPostScriptName(SkString* postScriptName) const override;
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;
    void getPostScriptGlyphNames(SkString* names) const override;
    bool onGlyphMaskNeedsCurrentColor() const override;
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override;
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                       int parameterCount) const override;
    int onGetTableTags(SkFontTableTag tags[]) const override;
    size_t onGetTableData(SkFontTableTag tag, size_t offset, size_t length, void* data) const override;
    ~SkTypeface_proxy() override;

private:
    SkTypeface_proxy(SkFontMgr_utils::SkAutoFcPattern pattern, SkString sysroot, SkFontScanner* fontScanner);
    sk_sp<SkTypeface> fProxy;
};

class SkScalerContext_proxy : public SkScalerContext {
private:
    std::unique_ptr<SkScalerContext> fProxy;
public:
    SkScalerContext_proxy(std::unique_ptr<SkScalerContext> proxy,
                               sk_sp<SkTypeface_proxy> typeface,
                               const SkScalerContextEffects& effects,
                               const SkDescriptor* desc);

    ~SkScalerContext_proxy() override = default;
protected:
    SkScalerContext::GlyphMetrics generateMetrics(const SkGlyph& glyph, SkArenaAlloc* alloc) override;

    void generateImage(const SkGlyph& glyph, void* imageBuffer) override;

    bool generatePath(const SkGlyph& glyph, SkPath* path, bool* modified) override;

    sk_sp<SkDrawable> generateDrawable(const SkGlyph& glyph) override;

    void generateFontMetrics(SkFontMetrics* metrics) override;
};

#endif // SkFontMgr_proxy_DEFINED
