/*
* Copyright 2024 The Android Open Source Project
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/ports/SkFontMgr_proxy.h"

#include <memory>
#include <utility>

namespace SkFontMgr_utils {
bool get_bool(FcPattern* pattern, const char object[], bool missing) {
    FcBool value;
    if (FcPatternGetBool(pattern, object, 0, &value) != FcResultMatch) {
        return missing;
    }
    return value;
}

int get_int(FcPattern* pattern, const char object[], int missing) {
    int value;
    if (FcPatternGetInteger(pattern, object, 0, &value) != FcResultMatch) {
        return missing;
    }
    return value;
}

const char* get_string(FcPattern* pattern,
                                        const char object[],
                                        const char* missing) {
    FcChar8* value;
    if (FcPatternGetString(pattern, object, 0, &value) != FcResultMatch) {
        return missing;
    }
    return (const char*)value;
}

const FcMatrix* get_matrix(FcPattern* pattern, const char object[]) {
    FcMatrix* matrix;
    if (FcPatternGetMatrix(pattern, object, 0, &matrix) != FcResultMatch) {
        return nullptr;
    }
    return matrix;
}

int map_range(
        SkScalar value, SkScalar old_min, SkScalar old_max, SkScalar new_min, SkScalar new_max) {
    SkASSERT(old_min < old_max);
    SkASSERT(new_min <= new_max);
    return new_min + ((value - old_min) * (new_max - new_min) / (old_max - old_min));
}

SkFontStyle skfontstyle_from_fcpattern(FcPattern* pattern) {
    typedef SkFontStyle SkFS;

    // FcWeightToOpenType was buggy until 2.12.4
    static constexpr MapRanges weightRanges[] = {
            {FC_WEIGHT_THIN, SkFS::kThin_Weight},
            {FC_WEIGHT_EXTRALIGHT, SkFS::kExtraLight_Weight},
            {FC_WEIGHT_LIGHT, SkFS::kLight_Weight},
            {FC_WEIGHT_DEMILIGHT, 350},
            {FC_WEIGHT_BOOK, 380},
            {FC_WEIGHT_REGULAR, SkFS::kNormal_Weight},
            {FC_WEIGHT_MEDIUM, SkFS::kMedium_Weight},
            {FC_WEIGHT_DEMIBOLD, SkFS::kSemiBold_Weight},
            {FC_WEIGHT_BOLD, SkFS::kBold_Weight},
            {FC_WEIGHT_EXTRABOLD, SkFS::kExtraBold_Weight},
            {FC_WEIGHT_BLACK, SkFS::kBlack_Weight},
            {FC_WEIGHT_EXTRABLACK, SkFS::kExtraBlack_Weight},
    };
    SkScalar weight = map_ranges(
            get_int(pattern, FC_WEIGHT, FC_WEIGHT_REGULAR), weightRanges, std::size(weightRanges));

    static constexpr MapRanges widthRanges[] = {
            {FC_WIDTH_ULTRACONDENSED, SkFS::kUltraCondensed_Width},
            {FC_WIDTH_EXTRACONDENSED, SkFS::kExtraCondensed_Width},
            {FC_WIDTH_CONDENSED, SkFS::kCondensed_Width},
            {FC_WIDTH_SEMICONDENSED, SkFS::kSemiCondensed_Width},
            {FC_WIDTH_NORMAL, SkFS::kNormal_Width},
            {FC_WIDTH_SEMIEXPANDED, SkFS::kSemiExpanded_Width},
            {FC_WIDTH_EXPANDED, SkFS::kExpanded_Width},
            {FC_WIDTH_EXTRAEXPANDED, SkFS::kExtraExpanded_Width},
            {FC_WIDTH_ULTRAEXPANDED, SkFS::kUltraExpanded_Width},
    };
    SkScalar width = map_ranges(
            get_int(pattern, FC_WIDTH, FC_WIDTH_NORMAL), widthRanges, std::size(widthRanges));

    SkFS::Slant slant = SkFS::kUpright_Slant;
    switch (get_int(pattern, FC_SLANT, FC_SLANT_ROMAN)) {
        case FC_SLANT_ROMAN:
            slant = SkFS::kUpright_Slant;
            break;
        case FC_SLANT_ITALIC:
            slant = SkFS::kItalic_Slant;
            break;
        case FC_SLANT_OBLIQUE:
            slant = SkFS::kOblique_Slant;
            break;
        default:
            SkASSERT(false);
            break;
    }

    return SkFontStyle(SkScalarRoundToInt(weight), SkScalarRoundToInt(width), slant);
}

void fcpattern_from_skfontstyle(SkFontStyle style, FcPattern* pattern) {
    FCLocker::AssertHeld();

    typedef SkFontStyle SkFS;

    // FcWeightFromOpenType was buggy until 2.12.4
    static constexpr MapRanges weightRanges[] = {
            {SkFS::kThin_Weight, FC_WEIGHT_THIN},
            {SkFS::kExtraLight_Weight, FC_WEIGHT_EXTRALIGHT},
            {SkFS::kLight_Weight, FC_WEIGHT_LIGHT},
            {350, FC_WEIGHT_DEMILIGHT},
            {380, FC_WEIGHT_BOOK},
            {SkFS::kNormal_Weight, FC_WEIGHT_REGULAR},
            {SkFS::kMedium_Weight, FC_WEIGHT_MEDIUM},
            {SkFS::kSemiBold_Weight, FC_WEIGHT_DEMIBOLD},
            {SkFS::kBold_Weight, FC_WEIGHT_BOLD},
            {SkFS::kExtraBold_Weight, FC_WEIGHT_EXTRABOLD},
            {SkFS::kBlack_Weight, FC_WEIGHT_BLACK},
            {SkFS::kExtraBlack_Weight, FC_WEIGHT_EXTRABLACK},
    };
    int weight = map_ranges(style.weight(), weightRanges, std::size(weightRanges));

    static constexpr MapRanges widthRanges[] = {
            {SkFS::kUltraCondensed_Width, FC_WIDTH_ULTRACONDENSED},
            {SkFS::kExtraCondensed_Width, FC_WIDTH_EXTRACONDENSED},
            {SkFS::kCondensed_Width, FC_WIDTH_CONDENSED},
            {SkFS::kSemiCondensed_Width, FC_WIDTH_SEMICONDENSED},
            {SkFS::kNormal_Width, FC_WIDTH_NORMAL},
            {SkFS::kSemiExpanded_Width, FC_WIDTH_SEMIEXPANDED},
            {SkFS::kExpanded_Width, FC_WIDTH_EXPANDED},
            {SkFS::kExtraExpanded_Width, FC_WIDTH_EXTRAEXPANDED},
            {SkFS::kUltraExpanded_Width, FC_WIDTH_ULTRAEXPANDED},
    };
    int width = map_ranges(style.width(), widthRanges, std::size(widthRanges));

    int slant = FC_SLANT_ROMAN;
    switch (style.slant()) {
        case SkFS::kUpright_Slant:
            slant = FC_SLANT_ROMAN;
            break;
        case SkFS::kItalic_Slant:
            slant = FC_SLANT_ITALIC;
            break;
        case SkFS::kOblique_Slant:
            slant = FC_SLANT_OBLIQUE;
            break;
        default:
            SkASSERT(false);
            break;
    }

    FcPatternAddInteger(pattern, FC_WEIGHT, weight);
    FcPatternAddInteger(pattern, FC_WIDTH, width);
    FcPatternAddInteger(pattern, FC_SLANT, slant);
}

SkScalar map_ranges(SkScalar val, MapRanges const ranges[], int rangesCount) {
    // -Inf to [0]
    if (val < ranges[0].old_val) {
        return ranges[0].new_val;
    }

    // Linear from [i] to [i+1]
    for (int i = 0; i < rangesCount - 1; ++i) {
        if (val < ranges[i + 1].old_val) {
            return map_range(val,
                             ranges[i].old_val,
                             ranges[i + 1].old_val,
                             ranges[i].new_val,
                             ranges[i + 1].new_val);
        }
    }

    // From [n] to +Inf
    // if (fcweight < Inf)
    return ranges[rangesCount - 1].new_val;
}
}

sk_sp<SkTypeface_proxy> SkTypeface_proxy::Make(SkFontMgr_utils::SkAutoFcPattern pattern,
                                         SkString sysroot,
                                         SkFontScanner* scanner) {
    return sk_sp<SkTypeface_proxy>(
            new SkTypeface_proxy(std::move(pattern), std::move(sysroot), scanner));
}

void SkTypeface_proxy::onGetFamilyName(SkString* familyName) const {
    *familyName = SkFontMgr_utils::get_string(fPattern, FC_FAMILY);
}
void SkTypeface_proxy::onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const {
    // TODO: need to serialize FC_MATRIX and FC_EMBOLDEN
    SkFontMgr_utils::FCLocker lock;
    fProxy->onGetFontDescriptor(desc, serialize);
    desc->setFamilyName(SkFontMgr_utils::get_string(fPattern, FC_FAMILY));
    desc->setFullName(SkFontMgr_utils::get_string(fPattern, FC_FULLNAME));
    desc->setPostscriptName(SkFontMgr_utils::get_string(fPattern, FC_POSTSCRIPT_NAME));
    desc->setStyle(this->fontStyle());
    *serialize = false;
}

std::unique_ptr<SkStreamAsset> SkTypeface_proxy::onOpenStream(int* ttcIndex) const {
    return fProxy->onOpenStream(ttcIndex);
}

void SkTypeface_proxy::onFilterRec(SkScalerContextRec* rec) const {
    // FontConfig provides 10-scale-bitmap-fonts.conf which applies an inverse "pixelsize"
    // matrix. It is not known if this .conf is active or not, so it is not clear if
    // "pixelsize" should be applied before this matrix. Since using a matrix with a bitmap
    // font isn't a great idea, only apply the matrix to outline fonts.
    const FcMatrix* fcMatrix = SkFontMgr_utils::get_matrix(fPattern, FC_MATRIX);
    bool fcOutline = SkFontMgr_utils::get_bool(fPattern, FC_OUTLINE, true);
    if (fcOutline && fcMatrix) {
        // fPost2x2 is column-major, left handed (y down).
        // FcMatrix is column-major, right handed (y up).
        SkMatrix fm;
        fm.setAll(fcMatrix->xx,-fcMatrix->xy, 0,
                  -fcMatrix->yx, fcMatrix->yy, 0,
                  0           , 0           , 1);

        SkMatrix sm;
        rec->getMatrixFrom2x2(&sm);

        sm.preConcat(fm);
        rec->fPost2x2[0][0] = sm.getScaleX();
        rec->fPost2x2[0][1] = sm.getSkewX();
        rec->fPost2x2[1][0] = sm.getSkewY();
        rec->fPost2x2[1][1] = sm.getScaleY();
    }
    if (SkFontMgr_utils::get_bool(fPattern, FC_EMBOLDEN)) {
        rec->fFlags |= SkScalerContext::kEmbolden_Flag;
    }

    fProxy->filterRec(rec);
}

std::unique_ptr<SkAdvancedTypefaceMetrics> SkTypeface_proxy::onGetAdvancedMetrics() const {
    std::unique_ptr<SkAdvancedTypefaceMetrics> info = fProxy->onGetAdvancedMetrics();
    // Simulated fonts shouldn't be considered to be of the type of their data.
    if (SkFontMgr_utils::get_matrix(fPattern, FC_MATRIX) ||
        SkFontMgr_utils::get_bool(fPattern, FC_EMBOLDEN)) {
        info->fType = SkAdvancedTypefaceMetrics::kOther_Font;
    }
    return info;
}

sk_sp<SkTypeface> SkTypeface_proxy::onMakeClone(const SkFontArguments& args) const {
    // TODO: need to clone FC_MATRIX and FC_EMBOLDEN by wrapping this
    return fProxy->onMakeClone(args);
}

std::unique_ptr<SkScalerContext> SkTypeface_proxy::onCreateScalerContext(
        const SkScalerContextEffects& effects, const SkDescriptor* descr) const {
    auto proxyScaler = fProxy->onCreateScalerContextAsProxyTypeface(effects, descr, sk_ref_sp(this));
    auto scaler = std::make_unique<SkScalerContext_proxy>(
            std::move(proxyScaler),
            sk_ref_sp(const_cast<SkTypeface_proxy*>(this)),
            effects,
            descr);
    return scaler;
}

void SkTypeface_proxy::getGlyphToUnicodeMap(SkUnichar* glyphToUnicode) const {
    fProxy->getGlyphToUnicodeMap(glyphToUnicode);
}

void SkTypeface_proxy::onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const {
    fProxy->unicharsToGlyphs(chars, count, glyphs);
}

int SkTypeface_proxy::onCountGlyphs() const { return fProxy->countGlyphs(); }

int SkTypeface_proxy::onGetUPEM() const { return fProxy->getUnitsPerEm(); }

bool SkTypeface_proxy::onGetKerningPairAdjustments(const uint16_t glyphs[], int count,
                                 int32_t adjustments[]) const  {
    return fProxy->onGetKerningPairAdjustments(glyphs, count, adjustments);
}

bool SkTypeface_proxy::onGetPostScriptName(SkString* postScriptName) const {
    return fProxy->getPostScriptName(postScriptName);
}

SkTypeface::LocalizedStrings* SkTypeface_proxy::onCreateFamilyNameIterator() const {
    return fProxy->createFamilyNameIterator();
}

void SkTypeface_proxy::getPostScriptGlyphNames(SkString* names) const {
    return fProxy->getPostScriptGlyphNames(names);
}

bool SkTypeface_proxy::onGlyphMaskNeedsCurrentColor() const {
    return fProxy->glyphMaskNeedsCurrentColor();
}

int SkTypeface_proxy::onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                 int coordinateCount) const {
    return fProxy->onGetVariationDesignPosition(coordinates, coordinateCount);
}

int SkTypeface_proxy::onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                   int parameterCount) const {
    return fProxy->onGetVariationDesignParameters(parameters, parameterCount);
}

int SkTypeface_proxy::onGetTableTags(SkFontTableTag tags[]) const {
    return fProxy->getTableTags(tags);
}

size_t SkTypeface_proxy::onGetTableData(SkFontTableTag tag, size_t offset, size_t length, void* data) const {
    return fProxy->getTableData(tag, offset, length, data);
}

SkTypeface_proxy::~SkTypeface_proxy() {
    // Hold the lock while unrefing the pattern.
    SkFontMgr_utils::FCLocker lock;
    fPattern.reset();
}

SkTypeface_proxy::SkTypeface_proxy(SkFontMgr_utils::SkAutoFcPattern pattern, SkString sysroot, SkFontScanner* fontScanner)
        : SkTypeface(SkFontMgr_utils::skfontstyle_from_fcpattern(pattern),
                     FC_PROPORTIONAL != SkFontMgr_utils::get_int(pattern, FC_SPACING, FC_PROPORTIONAL))
        , fPattern(std::move(pattern))
        , fSysroot(std::move(sysroot)) {
    // TODO: for now only FreeTypeStream
    SkString resolvedFilename;
    SkFontMgr_utils::FCLocker lock;
    const char* filename = SkFontMgr_utils::get_string(fPattern, FC_FILE);
    // See FontAccessible for note on searching sysroot then non-sysroot path.
    if (!fSysroot.isEmpty()) {
        resolvedFilename = fSysroot;
        resolvedFilename += filename;
        if (sk_exists(resolvedFilename.c_str(), kRead_SkFILE_Flag)) {
            filename = resolvedFilename.c_str();
        }
    }
    // TODO: FC_VARIABLE and FC_FONT_VARIATIONS in arguments
    auto ttcIndex = SkFontMgr_utils::get_int(fPattern, FC_INDEX, 0);
    fProxy = fontScanner->MakeFromStream(SkStream::MakeFromFile(filename),
                                         SkFontArguments().setCollectionIndex(ttcIndex));
}

SkScalerContext_proxy::SkScalerContext_proxy(std::unique_ptr<SkScalerContext> proxy,
                           sk_sp<SkTypeface_proxy> typeface,
                           const SkScalerContextEffects& effects,
                           const SkDescriptor* desc)
        : SkScalerContext(std::move(typeface), effects, desc)
        , fProxy(std::move(proxy))
{ }

SkScalerContext::GlyphMetrics SkScalerContext_proxy::generateMetrics(const SkGlyph& glyph, SkArenaAlloc* alloc) {
    return fProxy->generateMetrics(glyph, alloc);
}

void SkScalerContext_proxy::generateImage(const SkGlyph& glyph, void* imageBuffer) {
    fProxy->generateImage(glyph, imageBuffer);
}

bool SkScalerContext_proxy::generatePath(const SkGlyph& glyph, SkPath* path, bool* modified) {
    return fProxy->generatePath(glyph, path, modified);
}

sk_sp<SkDrawable> SkScalerContext_proxy::generateDrawable(const SkGlyph& glyph) {
    return fProxy->generateDrawable(glyph);
}

void SkScalerContext_proxy::generateFontMetrics(SkFontMetrics* metrics) {
    fProxy->generateFontMetrics(metrics);
}
