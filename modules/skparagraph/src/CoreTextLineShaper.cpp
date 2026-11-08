// Copyright 2026 Google LLC
#include "modules/skparagraph/src/CoreTextLineShaper.h"

#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkTypeface_mac.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/src/CoreTextAttrStringBuilder.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "src/core/SkTHash.h"
#include "src/core/SkUTF.h"
#include "src/utils/mac/SkCGBase.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <utility>

namespace skia {
namespace textlayout {

struct CoreTextRunState {
    CoreTextRunState(CFIndex glyphCount,
                     TextRange utf8Range,
                     bool isLtr,
                     bool zeroSize,
                     bool isFallback,
                     bool splitKerning,
                     SkFont font,
                     Run::ShapingBuffer* shapingBuffer)
            : fFont(std::move(font))
            , fGlyphCount(glyphCount)
            , fUtf8Range(utf8Range)
            , fIsLtr(isLtr)
            , fZeroSize(zeroSize)
            , fIsFallback(isFallback)
            , fSplitKerning(splitKerning)
            , fShapingBuffer(shapingBuffer)
            , fClusters(glyphCount)
            , fAdvances(glyphCount)
            , fOrigins(isFallback ? 0 : glyphCount)
            , fGlyphs(isFallback ? 0 : glyphCount)
            , fSplitOffsets(splitKerning ? glyphCount : 0) {}

    SkFont fFont;
    const TextStyle* fStyle = nullptr;
    const char* fLanguage = "";
    SkFourByteTag fScript = 0;
    uint8_t fBidiLevel = 0;
    CFIndex fGlyphCount;
    TextRange fUtf8Range;
    bool fIsLtr;
    bool fZeroSize;
    bool fIsFallback;
    bool fSplitKerning;
    SkScalar fRunAdvanceX = 0;
    Run::ShapingBuffer* fShapingBuffer;
    skia_private::AutoSTArray<64, uint32_t> fClusters;
    skia_private::AutoSTArray<64, SkScalar> fAdvances;
    skia_private::AutoSTArray<64, CGPoint> fOrigins;
    skia_private::AutoSTArray<64, CGGlyph> fGlyphs;
    skia_private::AutoSTArray<64, SkScalar> fSplitOffsets;
};

struct RegionShapeContext {
    struct RunItem {
        CTRunRef fRun;
        CFIndex fLocation;
        CFIndex fLength;
    };

    // Caches the active TextStyle block's resolved primary typeface, allowed fallback typefaces,
    // locale, and per-CTFont allowlist verdicts across neighboring CTRuns in the same Block
    struct BlockStyleState {
        bool fInitialized = false;
        const Block* fBlock = nullptr;
        const TextStyle* fStyle = nullptr;
        sk_sp<SkTypeface> fPrimary;
        std::vector<sk_sp<SkTypeface>> fAllowedTypefaces;
        SkString fLanguage;
        skia_private::THashMap<CTFontRef, bool> fAllowedFontCache;
    };

    RegionShapeContext(ParagraphImpl& paragraph,
                       const std::vector<SkUnicode::BidiRegion>& bidiRegions,
                       TextRange regionRange)
            : fText(paragraph.text())
            , fStyles(paragraph.styles())
            , fBidiRegions(bidiRegions)
            , fRegionRange(regionRange)
            , fFallbackDisabled(!paragraph.fontCollection()->fontFallbackEnabled())
            , fBuilder(fText.data() + regionRange.start,
                       regionRange.width(),
                       paragraph.paragraphStyle().getTextDirection(),
                       paragraph.fontCollection()) {}

    SkSpan<const char> fText;
    SkSpan<const Block> fStyles;
    const std::vector<SkUnicode::BidiRegion>& fBidiRegions;
    TextRange fRegionRange;
    bool fFallbackDisabled;
    CoreTextAttrStringBuilder fBuilder;
    skia_private::STArray<16, RunItem> fSortedRuns;
    skia_private::THashMap<CTFontRef, sk_sp<SkTypeface>> fTypefaceCache;
    BlockStyleState fBlockState;
    // Run::commit keeps the glyph offsets only when one is not zero, as for SkShaper runs
    Run::ShapingBuffer fShapingBuffer;
};

namespace {

struct AffixStyleState {
    TextStyle fStyle;
    std::vector<sk_sp<SkTypeface>> fAllowedTypefaces;
    sk_sp<SkTypeface> fPrimary;
    CTFontRef fPrimaryCtFont = nullptr;
    bool fFallbackDisabled = false;
    bool fPrimaryDisallowed = false;
    bool fZeroSize = false;
    bool fHasTab = false;
    SkFont fFont;
    uint8_t fLevel = 0;
};

struct AffixGlyphData {
    explicit AffixGlyphData(CFIndex totalGlyphs)
            : fCount(totalGlyphs)
            , fGlyphs(totalGlyphs)
            , fOrigins(totalGlyphs)
            , fAdvances(totalGlyphs)
            , fSplitOffsets(totalGlyphs)
            , fClusters(totalGlyphs) {
        std::fill_n(fSplitOffsets.data(), totalGlyphs, 0.0f);
    }

    CFIndex fCount;
    SkScalar fTotalAdvance = 0;
    skia_private::AutoSTArray<16, CGGlyph> fGlyphs;
    skia_private::AutoSTArray<16, CGPoint> fOrigins;
    skia_private::AutoSTArray<16, SkScalar> fAdvances;
    // The part of a kerning pair moved into each glyph's offset, as in shapeTextRegion
    skia_private::AutoSTArray<16, SkScalar> fSplitOffsets;
    skia_private::AutoSTArray<16, uint32_t> fClusters;
};

// Writes each glyph's advance from CoreText's base advances, rounded to whole pixels when subpixel
// is off (the meaning of SkFont::isSubpixel), and when splitOffsets is non-null moves half of each
// spacing glyph's pair kerning (base advance minus nominal advance) onto the next glyph's advance
// and x-offset so drawn x stays unchanged while the caret sits at kern/2
static void coretext_advances(bool subpixel,
                              const CGSize* nominal,
                              const CGSize* advances,
                              SkScalar* out,
                              SkScalar* splitOffsets,
                              CFIndex count) {
    // Half a kerning pair would put the next glyph off the pixel grid
    SkASSERT(subpixel || !splitOffsets);
    SkASSERT(!splitOffsets || nominal);
    SkScalar carry = 0;
    for (CFIndex k = 0; k < count; ++k) {
        const SkScalar advance = SkScalarFromCGFloat(advances[k].width);
        if (!splitOffsets) {
            out[k] = subpixel ? advance : SkScalarRoundToScalar(advance);
            continue;
        }
        splitOffsets[k] = carry;
        out[k] = advance + carry;
        carry = 0;
        // CoreText kerns no pair across zero-advance glyphs or past the run end
        if (advance != 0 && k + 1 < count && advances[k + 1].width != 0) {
            carry = (advance - SkScalarFromCGFloat(nominal[k].width)) / 2;
            out[k] -= carry;
        }
    }
}

// Compares cfStr and UTF-8 skStr case-insensitively without copying skStr's bytes
static bool cfstring_equals_skstring(CFStringRef cfStr, const SkString& skStr) {
    SkASSERT(cfStr);
    if (skStr.isEmpty()) {
        return false;
    }
    SkUniqueCFRef<CFStringRef> skCFStr(
            CFStringCreateWithBytesNoCopy(kCFAllocatorDefault,
                                          reinterpret_cast<const UInt8*>(skStr.c_str()),
                                          skStr.size(),
                                          kCFStringEncodingUTF8,
                                          false,
                                          kCFAllocatorNull));
    if (!skCFStr) {
        return false;
    }
    return CFStringCompare(cfStr, skCFStr.get(), kCFCompareCaseInsensitive) == kCFCompareEqualTo;
}

// Checks whether ctFont's PostScript or family name matches any face in allowedTypefaces or any
// requested family name in fontFamilies
static bool is_allowed_font(CTFontRef ctFont,
                            const std::vector<sk_sp<SkTypeface>>& allowedTypefaces,
                            const std::vector<SkString>& fontFamilies) {
    SkASSERT(ctFont);
    SkUniqueCFRef<CFStringRef> ctPsName(CTFontCopyPostScriptName(ctFont));
    SkUniqueCFRef<CFStringRef> ctFamName(CTFontCopyFamilyName(ctFont));

    for (const auto& tf : allowedTypefaces) {
        SkASSERT(tf);
        SkString tfPs;
        tf->getPostScriptName(&tfPs);
        if (ctPsName && cfstring_equals_skstring(ctPsName.get(), tfPs)) {
            return true;
        }
        SkString tfFam;
        tf->getFamilyName(&tfFam);
        if (ctFamName && cfstring_equals_skstring(ctFamName.get(), tfFam)) {
            return true;
        }
    }

    for (const auto& fam : fontFamilies) {
        if (ctFamName && cfstring_equals_skstring(ctFamName.get(), fam)) {
            return true;
        }
        if (ctPsName && cfstring_equals_skstring(ctPsName.get(), fam)) {
            return true;
        }
    }

    return false;
}

// Returns the CTFont CoreText bound to ctRun during typesetting and font substitution
static CTFontRef ct_run_font(CTRunRef ctRun) {
    // CoreText manufactures run attributes during font substitution, and they always carry the
    // font the glyphs were shaped with
    CTFontRef ctFont =
            (CTFontRef)CFDictionaryGetValue(CTRunGetAttributes(ctRun), kCTFontAttributeName);
    SkASSERT(ctFont);
    return ctFont;
}

// Reports whether ctFont is LastResort or an unrequested fallback when font fallback is disabled
static bool is_disallowed_ct_font(CTFontRef ctFont,
                                  bool fallbackDisabled,
                                  const std::vector<sk_sp<SkTypeface>>& allowedTypefaces,
                                  const std::vector<SkString>& fontFamilies) {
    return CoreTextLineShaper::isLastResortFont(ctFont) ||
           (fallbackDisabled && !is_allowed_font(ctFont, allowedTypefaces, fontFamilies));
}

// Sets edging, hinting, subpixel, and optional synthetic bold/italic flags on font; bound ctFont
// is passed when available so system UI italic faces whose descriptor omits kCTFontSlantTrait are
// still recognized as italic
static void apply_font_style_props(SkFont& font,
                                   CTFontRef ctFont,
                                   const TextStyle& style,
                                   bool fakeMissingFontStyles) {
    font.setEdging(style.getFontEdging());
    font.setHinting(style.getFontHinting());
    font.setSubpixel(style.getSubpixel());

    // CoreText binds the closest real face and never synthesizes a missing bold or italic,
    // so the run gets the same fake flags OneLineShaper derives from the typeface gap
    if (fakeMissingFontStyles && font.getTypeface()) {
        const SkFontStyle wanted = style.getFontStyle();
        const SkFontStyle bound = font.getTypeface()->fontStyle();
        const bool fakeBold = wanted.weight() >= SkFontStyle::kSemiBold_Weight &&
                              wanted.weight() - bound.weight() >= 200;
        // System UI italic faces (.SFNS-RegularItalic) set kCTFontTraitItalic on the CTFont but
        // omit kCTFontSlantTrait from CTFontCopyFontDescriptor, so SkTypeface::fontStyle() reports
        // upright; inspect the bound CTFont before adding synthetic skew
        const bool isCtItalic =
                ctFont && ((CTFontGetSymbolicTraits(ctFont) & kCTFontTraitItalic) != 0 ||
                           CTFontGetSlantAngle(ctFont) != 0);
        const bool fakeItalic = wanted.slant() == SkFontStyle::kItalic_Slant &&
                                bound.slant() != SkFontStyle::kItalic_Slant && !isCtItalic;
        font.setEmbolden(fakeBold);
        font.setSkewX(fakeItalic ? -SK_Scalar1 / 4 : 0);
    }
}

// Builds a run's SkFont from ctFont (or fallbackPrimary when isFallback is true), collapsing
// stand-in sizes to zero when style.getFontSize() <= 0, applying style properties, and caching
// CTFont-to-SkTypeface conversions when typefaceCache is non-null
static SkFont make_run_font(CTFontRef ctFont,
                            bool isFallback,
                            const sk_sp<SkTypeface>& fallbackPrimary,
                            const TextStyle& style,
                            bool fakeMissingFontStyles,
                            skia_private::THashMap<CTFontRef, sk_sp<SkTypeface>>* typefaceCache) {
    // CoreText reads a zero point size as its 12pt default, so the attributed string shapes
    // such text at a stand-in size and the run collapses to a zero-size font with zero
    // advances, matching HarfBuzz, so builds without HarfBuzz lay it out too
    const SkScalar fontSize =
            style.getFontSize() <= 0 ? 0 : SkScalarFromCGFloat(CTFontGetSize(ctFont));
    sk_sp<SkTypeface> typeface;
    if (isFallback) {
        typeface = fallbackPrimary;
    } else if (typefaceCache) {
        if (sk_sp<SkTypeface>* cached = typefaceCache->find(ctFont)) {
            typeface = *cached;
        } else {
            typeface = SkMakeTypefaceFromCTFont(ctFont);
            typefaceCache->set(ctFont, typeface);
        }
    } else {
        typeface = SkMakeTypefaceFromCTFont(ctFont);
    }
    SkFont font(std::move(typeface), fontSize);
    // Fallback runs use the primary typeface rather than the bound CTFont
    apply_font_style_props(font, isFallback ? nullptr : ctFont, style, fakeMissingFontStyles);
    return font;
}

// Returns the BiDi embedding level covering utf8Pos, or defaultLevel when no region covers it
static uint8_t find_bidi_level(const std::vector<SkUnicode::BidiRegion>& regions,
                               size_t utf8Pos,
                               uint8_t defaultLevel) {
    for (const auto& r : regions) {
        if (utf8Pos >= r.start && utf8Pos < r.end) {
            return r.level;
        }
    }
    return defaultLevel;
}

// Reconciles an ICU/Unicode BiDi embedding level's parity with ctRun's shaped direction to ensure
// downstream visual reordering matches
static uint8_t reconcile_bidi_level(uint8_t level, CTRunRef ctRun) {
    const bool ctIsRtl = (CTRunGetStatus(ctRun) & kCTRunStatusRightToLeft) != 0;
    if ((level % 2 != 0) != ctIsRtl) {
        return ctIsRtl ? (level | 1) : (level > 0 ? level - 1 : 0);
    }
    return level;
}

// Looks up utf8Pos in regions and reconciles its BiDi embedding level parity with ctRun
static uint8_t resolve_run_bidi_level(const std::vector<SkUnicode::BidiRegion>& regions,
                                      size_t utf8Pos,
                                      CTRunRef ctRun) {
    return reconcile_bidi_level(find_bidi_level(regions, utf8Pos, 0), ctRun);
}

// Reordered complex-script glyphs (kCTRunStatusNonMonotonic) can report out-of-order string
// indices; Run::iterateThroughClustersInTextOrder requires non-decreasing indices for LTR and
// non-increasing for RTL while keeping CoreText's glyph IDs and positions untouched
static void normalize_cluster_indices(uint32_t* clusters, CFIndex glyphCount, bool ltr) {
    if (glyphCount <= 1) {
        return;
    }
    if (ltr) {
        for (CFIndex k = glyphCount - 2; k >= 0; --k) {
            if (clusters[k] > clusters[k + 1]) {
                clusters[k] = clusters[k + 1];
            }
        }
    } else {
        for (CFIndex k = 1; k < glyphCount; ++k) {
            if (clusters[k] > clusters[k - 1]) {
                clusters[k] = clusters[k - 1];
            }
        }
    }
}

// One CTLine for the whole of attrStr. Every font in it keeps its trak, fallback fonts too, so
// the letterSpacing SkParagraph adds later stacks on the trak as native tracking does, except
// in cursive scripts, which SkParagraph does not space
static SkUniqueCFRef<CTLineRef> typeset_line(CFAttributedStringRef attrStr) {
    const CFRange range = CFRangeMake(0, CFAttributedStringGetLength(attrStr));
    SkUniqueCFRef<CTTypesetterRef> typesetter(CTTypesetterCreateWithAttributedString(attrStr));
    if (!typesetter) {
        return nullptr;
    }
    return SkUniqueCFRef<CTLineRef>(CTTypesetterCreateLine(typesetter.get(), range));
}

// Extracts ctRun's glyph IDs, CoreText origins, and horizontal glyph advances into caller buffers,
// zeroing advances when zeroSize is true, rounding when subpixel is false, and splitting pair
// kerning into splitOffsetsOut when non-null
static void extract_ctrun_glyphs(CTRunRef ctRun,
                                 CTFontRef ctFont,
                                 CFIndex count,
                                 bool zeroSize,
                                 bool subpixel,
                                 CGGlyph* glyphsOut,
                                 CGPoint* originsOut,
                                 SkScalar* advancesOut,
                                 SkScalar* splitOffsetsOut) {
    CTRunGetGlyphs(ctRun, CFRangeMake(0, count), glyphsOut);
    skia_private::AutoSTArray<64, CGSize> baseAdvances(count);
    // CTRunGetAdvances derives advances from drawn x, so a zero-advance mark drawn d off the pen
    // shows as +d on the glyph before it in the run and -d on the mark. Base advances leave mark
    // positioning to the origins, as HarfBuzz leaves it to x_offset
    CTRunGetBaseAdvancesAndOrigins(ctRun, CFRangeMake(0, count), baseAdvances.data(), originsOut);
    // The 12pt stand-in advances CoreText gives zero-size text never reach the run
    if (zeroSize) {
        std::fill_n(advancesOut, count, 0.0f);
        if (splitOffsetsOut) {
            std::fill_n(splitOffsetsOut, count, 0.0f);
        }
        return;
    }
    skia_private::AutoSTArray<64, CGSize> nominal(splitOffsetsOut ? count : 0);
    if (splitOffsetsOut) {
        SkASSERT(ctFont);
        CTFontGetAdvancesForGlyphs(
                ctFont, kCTFontOrientationHorizontal, glyphsOut, nominal.data(), count);
    }
    // KEEPER-DEBT: this keeps CoreText's advance to the next 28pt tab stop, where HarfBuzz gives
    // U+0009 its hmtx advance; the stop counts from the CTLine start (hard break or placeholder),
    // so after a soft wrap it is off the line's stops. Remove when tabs get the HarfBuzz advance
    // or line-relative stops
    coretext_advances(subpixel,
                      splitOffsetsOut ? nominal.data() : nullptr,
                      baseAdvances.data(),
                      advancesOut,
                      splitOffsetsOut,
                      count);
}

// Converts a CoreText glyph origin (y-up) into a Skia run glyph offset (y-down), pinning
// zero-size stand-in origins to (0, 0) and adding splitOffsetX to restore drawn x after moving
// half a kerning pair onto the glyph's advance
static SkPoint glyph_draw_offset(CGPoint origin, bool zeroSize, SkScalar splitOffsetX) {
    const CGPoint pinned = zeroSize ? CGPointZero : origin;
    return SkPoint::Make(SkScalarFromCGFloat(pinned.x) + splitOffsetX,
                         -SkScalarFromCGFloat(pinned.y));
}

// Selects the primary CTRun and CTFont for an affix line, populates state's SkFont and reconciled
// BiDi level, and returns the total glyph count across non-empty CTRuns (0 if none exist)
static CFIndex resolve_affix_runs(CFArrayRef runs,
                                  bool fakeMissingFontStyles,
                                  uint8_t bidiLevel,
                                  AffixStyleState& state) {
    const CFIndex runCount = runs ? CFArrayGetCount(runs) : 0;
    // ShapeHandler::runBuffer accepts at most one Run, so multi-run affixes coalesce into one
    // buffer with the first resolved CTRun's font and mark glyphs from other fonts unresolved
    CFIndex totalGlyphs = 0;
    CTRunRef primaryRun = nullptr;
    for (CFIndex i = 0; i < runCount; ++i) {
        CTRunRef ctRun = (CTRunRef)CFArrayGetValueAtIndex(runs, i);
        const CFIndex gc = CTRunGetGlyphCount(ctRun);
        if (gc <= 0) {
            continue;
        }
        totalGlyphs += gc;
        CTFontRef ctFont = ct_run_font(ctRun);
        const bool disallowed = is_disallowed_ct_font(ctFont,
                                                      state.fFallbackDisabled,
                                                      state.fAllowedTypefaces,
                                                      state.fStyle.getFontFamilies());
        // A disallowed primary (LastResort, or a system font hidden by disabled fallback) zeroes
        // every glyph of the buffer, so the first allowed CTRun takes its place
        if (!primaryRun || (!disallowed && state.fPrimaryDisallowed)) {
            primaryRun = ctRun;
            state.fPrimaryCtFont = ctFont;
            state.fPrimaryDisallowed = disallowed;
        }
    }
    if (totalGlyphs == 0) {
        return 0;
    }
    // The first non-empty CTRun sets primaryRun
    SkASSERT(primaryRun);

    state.fZeroSize = state.fStyle.getFontSize() <= 0;
    state.fFont = make_run_font(state.fPrimaryCtFont,
                                state.fPrimaryDisallowed,
                                state.fPrimary,
                                state.fStyle,
                                fakeMissingFontStyles,
                                nullptr);
    state.fLevel = reconcile_bidi_level(bidiLevel, primaryRun);
    return totalGlyphs;
}

// Collects glyphs, origins, advances, split kerning offsets, and normalized UTF-8 clusters across
// all non-empty CTRuns of an affix line, zeroing glyph IDs from disallowed or secondary CTRuns
static void collect_affix_glyphs(CFArrayRef runs,
                                 const CoreTextIndexMap& indexMap,
                                 const AffixStyleState& state,
                                 AffixGlyphData& out) {
    // ShapeAffix returns early when resolve_affix_runs finds zero glyphs
    SkASSERT(runs);
    const CFIndex runCount = CFArrayGetCount(runs);
    CFIndex runStart = 0;
    for (CFIndex i = 0; i < runCount; ++i) {
        CTRunRef ctRun = (CTRunRef)CFArrayGetValueAtIndex(runs, i);
        const CFIndex gc = CTRunGetGlyphCount(ctRun);
        if (gc <= 0) {
            continue;
        }
        CTFontRef ctFont = ct_run_font(ctRun);
        // Left-to-right subpixel runs without tabs split pair kerning as in shapeTextRegion
        const bool splitKerning = !state.fZeroSize && state.fFont.isSubpixel() && !state.fHasTab &&
                                  (CTRunGetStatus(ctRun) & kCTRunStatusRightToLeft) == 0;
        // Base advances keep a mark's drawn offset off the glyph before it, as in shapeTextRegion;
        // a zero-size affix keeps zero advances
        extract_ctrun_glyphs(ctRun,
                             ctFont,
                             gc,
                             state.fZeroSize,
                             state.fFont.isSubpixel(),
                             out.fGlyphs.data() + runStart,
                             out.fOrigins.data() + runStart,
                             out.fAdvances.data() + runStart,
                             splitKerning ? out.fSplitOffsets.data() + runStart : nullptr);
        for (CFIndex k = 0; k < gc; ++k) {
            out.fTotalAdvance += out.fAdvances[runStart + k];
        }

        const bool unresolvedRun = state.fPrimaryDisallowed ||
                                   is_disallowed_ct_font(ctFont,
                                                         state.fFallbackDisabled,
                                                         state.fAllowedTypefaces,
                                                         state.fStyle.getFontFamilies()) ||
                                   !CFEqual(ctFont, state.fPrimaryCtFont);
        if (unresolvedRun) {
            std::fill_n(out.fGlyphs.data() + runStart, gc, 0);
        }

        skia_private::AutoSTArray<16, CFIndex> indices(gc);
        CTRunGetStringIndices(ctRun, CFRangeMake(0, gc), indices.data());
        for (CFIndex k = 0; k < gc; ++k) {
            out.fClusters[runStart + k] = SkToU32(indexMap.toUtf8Index(indices[k]));
        }
        runStart += gc;
    }
    normalize_cluster_indices(out.fClusters.data(), out.fCount, (state.fLevel % 2) == 0);
}

// Commits a coalesced affix RunInfo and writes its glyphs, pen positions, draw offsets, and
// normalized cluster indices into handler
static void commit_affix_run(const SkShaper::RunHandler::RunInfo& info,
                             const AffixGlyphData& data,
                             bool zeroSize,
                             SkShaper::RunHandler* handler) {
    static_assert(sizeof(CGGlyph) == sizeof(SkGlyphID));
    handler->beginLine();
    handler->runInfo(info);
    handler->commitRunInfo();
    SkShaper::RunHandler::Buffer buffer = handler->runBuffer(info);

    // Same pen as in shapeTextRegion: base advances summed in float, with each origin apart in
    // the offset when the handler takes offsets, as SkShaper's HarfBuzz runs do
    SkScalar penX = 0;
    for (CFIndex g = 0; g < data.fCount; ++g) {
        buffer.glyphs[g] = data.fGlyphs[g];
        const SkPoint offset = glyph_draw_offset(data.fOrigins[g], zeroSize, data.fSplitOffsets[g]);
        if (buffer.offsets) {
            buffer.positions[g] = buffer.point + SkPoint::Make(penX, 0);
            buffer.offsets[g] = offset;
        } else {
            buffer.positions[g] = buffer.point + SkPoint::Make(penX, 0) + offset;
        }
        penX += data.fAdvances[g];
    }
    if (buffer.clusters) {
        std::copy_n(data.fClusters.data(), data.fCount, buffer.clusters);
    }
    handler->commitRunBuffer(info);
    handler->commitLine();
}

// Adds each intersecting style block of regionRange to builder and builds its attributed string
static SkUniqueCFRef<CFAttributedStringRef> build_region_attr_string(
        ParagraphImpl& paragraph, TextRange regionRange, CoreTextAttrStringBuilder& builder) {
    const BlockRange blockRange = paragraph.findAllBlocks(regionRange);
    for (const auto& block : paragraph.blocks(blockRange)) {
        const size_t start = std::max(block.fRange.start, regionRange.start);
        const size_t end = std::min(block.fRange.end, regionRange.end);
        if (start < end) {
            builder.addStyle(TextRange(start - regionRange.start, end - regionRange.start),
                             block.fStyle);
        }
    }
    return builder.build();
}

// Collects non-empty CTRuns from line and sorts them into ascending UTF-16 string order while
// preserving CoreText's relative order for runs sharing a start index
static void collect_sorted_ct_runs(
        CTLineRef line, skia_private::STArray<16, RegionShapeContext::RunItem>& sortedRuns) {
    CFArrayRef runs = CTLineGetGlyphRuns(line);
    const CFIndex runCount = runs ? CFArrayGetCount(runs) : 0;
    for (CFIndex i = 0; i < runCount; ++i) {
        CTRunRef ctRun = (CTRunRef)CFArrayGetValueAtIndex(runs, i);
        if (CTRunGetGlyphCount(ctRun) == 0) {
            continue;
        }
        const CFRange cfRange = CTRunGetStringRange(ctRun);
        sortedRuns.push_back({ctRun, cfRange.location, cfRange.length});
    }
    // CTRuns arrive in visual order from CTLineGetGlyphRuns
    // Logical ascending sort prevents BiDi inversion downstream in cluster layout
    std::stable_sort(
            sortedRuns.begin(),
            sortedRuns.end(),
            [](const RegionShapeContext::RunItem& a, const RegionShapeContext::RunItem& b) {
                return a.fLocation < b.fLocation;
            });
}

// Returns the style Block covering utf8Pos in sorted textStyles, or nullptr in unstyled gaps
static const Block* find_style_block(SkSpan<const Block> textStyles, size_t utf8Pos) {
    // fTextStyles is sorted by start, so the candidate is the last block starting at or before
    // utf8Pos; a position in a gap between blocks keeps the paragraph default style
    const Block* nextBlock = std::upper_bound(
            textStyles.begin(), textStyles.end(), utf8Pos, [](size_t pos, const Block& block) {
                return pos < block.fRange.start;
            });
    if (nextBlock != textStyles.begin() && utf8Pos < nextBlock[-1].fRange.end) {
        return nextBlock - 1;
    }
    return nullptr;
}

// Refreshes state when entering a new style Block (or on the first run of the region), reusing
// typeface lookups and allowed-font verdicts across neighboring CTRuns in the same Block
static void update_block_style_state(const ParagraphImpl& paragraph,
                                     const Block* block,
                                     RegionShapeContext::BlockStyleState& state) {
    if (state.fInitialized && block == state.fBlock) {
        return;
    }
    state.fInitialized = true;
    state.fBlock = block;
    state.fStyle = block ? &block->fStyle : &paragraph.paragraphStyle().getTextStyle();
    state.fLanguage = state.fStyle->getLocale();
    state.fAllowedFontCache.reset();
    state.fAllowedTypefaces.clear();
    state.fPrimary = state.fStyle->refTypeface();
    if (state.fPrimary) {
        state.fAllowedTypefaces.push_back(state.fPrimary);
        return;
    }
    // Same clone the attributed string was built from, so LastResort runs and the allowed-font
    // check see the varied face (or default .AppleSystemUIFont) rather than a null typeface
    state.fAllowedTypefaces =
            CoreTextFontRegistry::Instance().findTypefaces(*paragraph.fontCollection(),
                                                           state.fStyle->getFontFamilies(),
                                                           state.fStyle->getFontStyle(),
                                                           state.fStyle->getFontArguments());
    if (!state.fAllowedTypefaces.empty()) {
        state.fPrimary = state.fAllowedTypefaces.front();
    }
}

// Reports whether ctFont is LastResort or an unrequested fallback when font fallback is disabled,
// memoizing per-CTFont allowlist checks in state for the active Block
static bool is_disallowed_run_font(CTFontRef ctFont,
                                   bool fallbackDisabled,
                                   RegionShapeContext::BlockStyleState& state) {
    if (CoreTextLineShaper::isLastResortFont(ctFont)) {
        return true;
    }
    if (!fallbackDisabled) {
        return false;
    }
    bool* allowed = state.fAllowedFontCache.find(ctFont);
    if (!allowed) {
        allowed = state.fAllowedFontCache.set(
                ctFont,
                is_allowed_font(ctFont, state.fAllowedTypefaces, state.fStyle->getFontFamilies()));
    }
    return !*allowed;
}

// Maps ctRun's UTF-16 string indices to run-relative UTF-8 byte offsets in runState.fClusters and
// normalizes them to monotonic cluster order
static void build_run_clusters(CTRunRef ctRun,
                               const CoreTextIndexMap& indexMap,
                               size_t regionStart,
                               CoreTextRunState& runState) {
    skia_private::AutoSTArray<64, CFIndex> indexBuffer(runState.fGlyphCount);
    CTRunGetStringIndices(ctRun, CFRangeMake(0, runState.fGlyphCount), indexBuffer.data());
    for (CFIndex k = 0; k < runState.fGlyphCount; ++k) {
        const size_t globalU8 = regionStart + indexMap.toUtf8Index(indexBuffer[k]);
        const size_t clusterInRun = (globalU8 >= runState.fUtf8Range.start)
                                            ? (globalU8 - runState.fUtf8Range.start)
                                            : 0;
        runState.fClusters[k] = SkToU32(clusterInRun);
    }
    normalize_cluster_indices(runState.fClusters.data(), runState.fGlyphCount, runState.fIsLtr);
}

// Returns glyph k's paragraph-global UTF-8 text range from normalized run-relative clusters
static TextRange glyph_text_range(const CoreTextRunState& runState, CFIndex k) {
    size_t cStart = runState.fClusters[k];
    size_t cEnd = runState.fIsLtr
                          ? ((k + 1 < runState.fGlyphCount) ? runState.fClusters[k + 1]
                                                            : runState.fUtf8Range.width())
                          : ((k > 0) ? runState.fClusters[k - 1] : runState.fUtf8Range.width());
    if (cStart > cEnd) {
        std::swap(cStart, cEnd);
    }
    return TextRange(runState.fUtf8Range.start + cStart, runState.fUtf8Range.start + cEnd);
}

// Assigns each fallback cluster the primary font's .notdef advance per non-zero-width code point
// and stores their sum in runState.fRunAdvanceX
static void measure_fallback_glyphs(SkSpan<const char> paragraphText, CoreTextRunState& runState) {
    SkScalar notdefAdvance = runState.fFont.getWidth(0);
    if (SkScalarNearlyZero(notdefAdvance)) {
        notdefAdvance = runState.fFont.getSize();
    }
    // .notdef gets the same advance as any other glyph, whole pixels included
    if (!runState.fFont.isSubpixel()) {
        notdefAdvance = SkScalarRoundToScalar(notdefAdvance);
    }

    for (CFIndex k = 0; k < runState.fGlyphCount; ++k) {
        const TextRange gRange = glyph_text_range(runState, k);
        const char* p = paragraphText.data() + gRange.start;
        const char* pEnd = paragraphText.data() + gRange.end;
        size_t visibleCount = 0;
        while (p < pEnd) {
            const SkUnichar u = SkUTF::NextUTF8(&p, pEnd);
            if (u < 0) {
                break;
            }
            if (!CoreTextLineShaper::IsZeroWidthCodepoint(u)) {
                ++visibleCount;
            }
        }
        runState.fAdvances[k] = visibleCount * notdefAdvance;
        runState.fRunAdvanceX += runState.fAdvances[k];
    }
}

// Extracts ctRun's glyphs, origins, and shaped advances (plus split kerning offsets when enabled)
// into runState, applies the primary font's newline advance to U+000A clusters, and stores the
// total advance in runState.fRunAdvanceX
static void measure_shaped_glyphs(CTRunRef ctRun,
                                  CTFontRef ctFont,
                                  SkSpan<const char> paragraphText,
                                  CoreTextRunState& runState) {
    extract_ctrun_glyphs(ctRun,
                         ctFont,
                         runState.fGlyphCount,
                         runState.fZeroSize,
                         runState.fFont.isSubpixel(),
                         runState.fGlyphs.data(),
                         runState.fOrigins.data(),
                         runState.fAdvances.data(),
                         runState.fSplitKerning ? runState.fSplitOffsets.data() : nullptr);
    for (CFIndex k = 0; k < runState.fGlyphCount; ++k) {
        // CoreText gives '\n' no advance, HarfBuzz the advance of the primary font's cmap glyph
        // (.notdef when it has none); that advance moves all later lines, so their float positions
        // and widths differ unless '\n' gets one here too
        const TextRange gRange = glyph_text_range(runState, k);
        if (!runState.fZeroSize && gRange.width() == 1 && paragraphText[gRange.start] == '\n') {
            // KEEPER-DEBT: this is the run font's U+000A glyph; with a primary font that lacks
            // U+000A, CoreText puts '\n' in a fallback run, so this is not the primary .notdef
            // HarfBuzz uses and its advance can differ. Remove when '\n' stays in the primary
            // font's run
            const SkScalar newlineAdvance =
                    runState.fFont.getWidth(runState.fFont.unicharToGlyph('\n'));
            // Whole pixels for a non-subpixel font, as for every other glyph
            runState.fAdvances[k] = runState.fFont.isSubpixel()
                                            ? newlineAdvance
                                            : SkScalarRoundToScalar(newlineAdvance);
        }
        runState.fRunAdvanceX += runState.fAdvances[k];
    }
}

}  // namespace

bool CoreTextLineShaper::IsZeroWidthCodepoint(SkUnichar utf8) {
    // Default-ignorable format characters and non-spacing combining marks take no advance of their
    // own, so a fallback .notdef box is not widened for them. Neither do the C0/C1 controls with
    // Bidi_Class BN, which a native CTLine drops (tab, separators and U+0085 are not BN)
    static constexpr std::array<std::pair<SkUnichar, SkUnichar>, 17> ranges{{
            {0x0001, 0x0008},
            {0x000E, 0x001B},
            {0x007F, 0x0084},
            {0x0086, 0x009F},
            {0x00AD, 0x00AD},
            {0x0300, 0x036F},
            {0x061C, 0x061C},
            {0x1AB0, 0x1AFF},
            {0x1DC0, 0x1DFF},
            {0x200B, 0x200F},
            {0x202A, 0x202E},
            {0x2060, 0x2069},
            {0x20D0, 0x20FF},
            {0xFE00, 0xFE0F},
            {0xFE20, 0xFE2F},
            {0xFEFF, 0xFEFF},
            {0xE0100, 0xE01EF},
    }};
    auto it = std::ranges::lower_bound(ranges, utf8, {}, &std::pair<SkUnichar, SkUnichar>::second);
    return it != ranges.end() && utf8 >= it->first;
}

bool CoreTextLineShaper::isLastResortFont(CTFontRef font) {
    SkASSERT(font);
    SkUniqueCFRef<CFStringRef> psName(CTFontCopyPostScriptName(font));
    if (psName) {
        CFRange r = CFStringFind(psName.get(), CFSTR("LastResort"), kCFCompareCaseInsensitive);
        if (r.location != kCFNotFound) {
            return true;
        }
    }
    return false;
}

void CoreTextLineShaper::ShapeAffix(const SkString& text,
                                    const TextStyle& style,
                                    sk_sp<SkTypeface> clusterTypeface,
                                    bool fakeMissingFontStyles,
                                    uint8_t bidiLevel,
                                    sk_sp<FontCollection> fontCollection,
                                    SkShaper::RunHandler* handler) {
    SkASSERT(handler);
    // TextLine shapes an ellipsis only when the style is ellipsized, which needs a non-empty
    // string, and every paragraph owns a font collection
    SkASSERT(!text.isEmpty());
    SkASSERT(fontCollection);
    const TextDirection baseDir = (bidiLevel & 1) ? TextDirection::kRtl : TextDirection::kLtr;
    AffixStyleState state;
    state.fStyle = style;
    // Inherit the line's resolved cluster typeface when the ellipsis style does not specify an
    // explicit typeface or font family list
    if (!state.fStyle.refTypeface() && state.fStyle.getFontFamilies().empty() && clusterTypeface) {
        state.fStyle.setTypeface(clusterTypeface);
    }

    state.fAllowedTypefaces =
            CoreTextFontRegistry::Instance().findTypefaces(*fontCollection,
                                                           state.fStyle.getFontFamilies(),
                                                           state.fStyle.getFontStyle(),
                                                           state.fStyle.getFontArguments());
    state.fPrimary = state.fStyle.refTypeface();
    if (state.fPrimary) {
        state.fAllowedTypefaces.insert(state.fAllowedTypefaces.begin(), state.fPrimary);
    } else if (!state.fAllowedTypefaces.empty()) {
        state.fPrimary = state.fAllowedTypefaces.front();
    }
    if (clusterTypeface) {
        state.fAllowedTypefaces.push_back(clusterTypeface);
    }
    state.fFallbackDisabled = !fontCollection->fontFallbackEnabled();
    state.fHasTab = memchr(text.c_str(), '\t', text.size()) != nullptr;

    CoreTextAttrStringBuilder builder(
            text.c_str(), text.size(), baseDir, std::move(fontCollection));
    builder.addStyle(TextRange(0, text.size()), state.fStyle);
    SkUniqueCFRef<CFAttributedStringRef> attrStr = builder.build();
    if (!attrStr) {
        return;
    }
    SkUniqueCFRef<CTLineRef> line = typeset_line(attrStr.get());
    if (!line) {
        return;
    }

    CFArrayRef runs = CTLineGetGlyphRuns(line.get());
    const CFIndex totalGlyphs = resolve_affix_runs(runs, fakeMissingFontStyles, bidiLevel, state);
    if (totalGlyphs == 0) {
        return;
    }

    AffixGlyphData glyphData(totalGlyphs);
    collect_affix_glyphs(runs, builder.indexMap(), state, glyphData);

    const SkString language = state.fStyle.getLocale();
    const SkShaper::RunHandler::RunInfo info = {state.fFont,
                                                state.fLevel,
                                                builder.scriptAt(0),
                                                language.c_str(),
                                                SkVector::Make(glyphData.fTotalAdvance, 0),
                                                static_cast<size_t>(totalGlyphs),
                                                SkShaper::RunHandler::Range(0, text.size())};
    commit_affix_run(info, glyphData, state.fZeroSize, handler);
}

CoreTextLineShaper::CoreTextLineShaper(ParagraphImpl* paragraph)
        : fParagraph(paragraph), fUnresolvedGlyphs(0) {}

bool CoreTextLineShaper::shape() {
    if (fParagraph->text().size() == 0) {
        return false;
    }

    fUnresolvedGlyphs = 0;
    // Release the set, do not clear it: a ParagraphCache entry may share it
    fParagraph->fUnresolvedCodepoints.reset();
    fParagraph->fFontSwitches.clear();
    fParagraph->fRuns.clear();

    // ParagraphBuilderImpl::Build always appends a trailing zero-width sentinel placeholder
    // whose fTextBefore covers all text after the last explicit placeholder
    SkASSERT(!fParagraph->fPlaceholders.empty());

    SkScalar advanceX = 0;
    for (int i = 0; i < fParagraph->fPlaceholders.size(); ++i) {
        const Placeholder& placeholder = fParagraph->fPlaceholders[i];
        if (placeholder.fTextBefore.width() > 0 &&
            !this->shapeTextRange(placeholder.fTextBefore, advanceX)) {
            return false;
        }
        if (placeholder.fRange.width() > 0) {
            this->appendPlaceholderRun(placeholder, SkToSizeT(i), advanceX);
        }
    }

    return true;
}

void CoreTextLineShaper::appendPlaceholderRun(const Placeholder& placeholder,
                                              size_t placeholderIndex,
                                              SkScalar& advanceX) {
    const uint8_t placeholderLevel =
            find_bidi_level(fParagraph->fBidiRegions, placeholder.fRange.start, 2);

    std::vector<sk_sp<SkTypeface>> typefaces = CoreTextFontRegistry::Instance().findTypefaces(
            *fParagraph->fFontCollection,
            placeholder.fTextStyle.getFontFamilies(),
            placeholder.fTextStyle.getFontStyle(),
            placeholder.fTextStyle.getFontArguments());
    sk_sp<SkTypeface> typeface = typefaces.empty() ? nullptr : typefaces.front();
    SkFont font(std::move(typeface), placeholder.fTextStyle.getFontSize());
    font.setEdging(placeholder.fTextStyle.getFontEdging());
    font.setHinting(placeholder.fTextStyle.getFontHinting());
    font.setSubpixel(placeholder.fTextStyle.getSubpixel());

    const SkShaper::RunHandler::RunInfo runInfo = {
            font,
            placeholderLevel,
            0,
            "",
            SkPoint::Make(placeholder.fStyle.fWidth, placeholder.fStyle.fHeight),
            1,
            SkShaper::RunHandler::Range(0, placeholder.fRange.width())};

    auto& run = fParagraph->fRuns.emplace_back(fParagraph,
                                               runInfo,
                                               placeholder.fRange.start,
                                               0.0f,
                                               false,
                                               0.0f,
                                               fParagraph->fRuns.size(),
                                               advanceX);

    run.allocateGlyphData(1, /*hasOffsets=*/false);
    run.fGlyphData->glyphs()[0] = 0;
    run.fGlyphData->positions()[0] = {advanceX, 0};
    // CoreText runs keep the end sentinel on the baseline
    run.fGlyphData->positions()[1] = {advanceX + placeholder.fStyle.fWidth, 0};
    run.fGlyphData->clusterIndexes()[0] = 0;
    run.fPlaceholderIndex = placeholderIndex;
    advanceX += placeholder.fStyle.fWidth;
}

// CoreText typesetter produces a single visual line; isolating hard line breaks keeps each
// paragraph segment in one CTLine while CoreTextAttrStringBuilder splits CTRuns across blocks
// with differing Run-level font or metric properties
bool CoreTextLineShaper::shapeTextRange(TextRange range, SkScalar& advanceX) {
    // shape() skips empty fTextBefore ranges before calling shapeTextRange
    SkASSERT(range.width() > 0);

    const char* text = fParagraph->text().data();
    size_t segStart = range.start;

    while (segStart < range.end) {
        // The '\n' ends its segment, so each CTLine holds one hard-break paragraph for BiDi and
        // typesetting
        size_t segEnd = segStart;
        while (segEnd < range.end && text[segEnd++] != '\n') {
        }

        // Shape the entire hard-break segment in a single CTLine so contextual joining and BiDi
        // reordering operate across adjacent Block boundaries
        if (!this->shapeTextRegion(TextRange(segStart, segEnd), advanceX)) {
            return false;
        }

        segStart = segEnd;
    }

    return true;
}

bool CoreTextLineShaper::shapeTextRegion(TextRange regionRange, SkScalar& advanceX) {
    // shapeTextRange advances segEnd past at least one byte before calling shapeTextRegion
    SkASSERT(regionRange.width() > 0);

    RegionShapeContext ctx(*fParagraph, fParagraph->fBidiRegions, regionRange);
    SkUniqueCFRef<CFAttributedStringRef> attrStr =
            build_region_attr_string(*fParagraph, regionRange, ctx.fBuilder);
    if (!attrStr) {
        return false;
    }
    if (CFAttributedStringGetLength(attrStr.get()) == 0) {
        return true;
    }

    SkUniqueCFRef<CTLineRef> line = typeset_line(attrStr.get());
    if (!line) {
        return false;
    }

    collect_sorted_ct_runs(line.get(), ctx.fSortedRuns);
    for (int runIdx = 0; runIdx < ctx.fSortedRuns.size(); ++runIdx) {
        this->shapeRegionRun(ctx, runIdx, advanceX);
    }

    return true;
}

void CoreTextLineShaper::shapeRegionRun(RegionShapeContext& ctx, int runIdx, SkScalar& advanceX) {
    const RegionShapeContext::RunItem& item = ctx.fSortedRuns[runIdx];
    const TextRange relUtf8Range =
            ctx.fBuilder.indexMap().cfrangeToTextRange(CFRangeMake(item.fLocation, item.fLength));
    const TextRange runUtf8Range(ctx.fRegionRange.start + relUtf8Range.start,
                                 ctx.fRegionRange.start + relUtf8Range.end);
    if (runUtf8Range.width() == 0) {
        return;
    }

    update_block_style_state(
            *fParagraph, find_style_block(ctx.fStyles, runUtf8Range.start), ctx.fBlockState);

    CTFontRef ctFont = ct_run_font(item.fRun);
    const bool isFallback = is_disallowed_run_font(ctFont, ctx.fFallbackDisabled, ctx.fBlockState);
    const uint8_t bidiLevel =
            resolve_run_bidi_level(ctx.fBidiRegions, runUtf8Range.start, item.fRun);
    const bool isLtr = (bidiLevel % 2) == 0;
    const bool zeroSize = ctx.fBlockState.fStyle->getFontSize() <= 0;
    SkFont runFont = make_run_font(ctFont,
                                   isFallback,
                                   ctx.fBlockState.fPrimary,
                                   *ctx.fBlockState.fStyle,
                                   fParagraph->paragraphStyle().fakeMissingFontStyles(),
                                   &ctx.fTypefaceCache);
    // Only left-to-right subpixel runs split pair kerning; a tab's advance to its stop is not
    // kerning, so runs containing a tab keep whole advances
    const bool splitKerning =
            !isFallback && !zeroSize && isLtr && runFont.isSubpixel() &&
            !memchr(ctx.fText.data() + runUtf8Range.start, '\t', runUtf8Range.width());

    CoreTextRunState runState(CTRunGetGlyphCount(item.fRun),
                              runUtf8Range,
                              isLtr,
                              zeroSize,
                              isFallback,
                              splitKerning,
                              std::move(runFont),
                              &ctx.fShapingBuffer);
    runState.fStyle = ctx.fBlockState.fStyle;
    runState.fLanguage = ctx.fBlockState.fLanguage.c_str();
    runState.fBidiLevel = bidiLevel;
    // The itemization that keeps cursive ranges unspaced also names the run's script, so
    // Run::isCursiveScript agrees with the glyphs CoreText produced
    runState.fScript = ctx.fBuilder.scriptAt(relUtf8Range.start);

    build_run_clusters(item.fRun, ctx.fBuilder.indexMap(), ctx.fRegionRange.start, runState);
    if (isFallback) {
        measure_fallback_glyphs(ctx.fText, runState);
    } else {
        measure_shaped_glyphs(item.fRun, ctFont, ctx.fText, runState);
    }
    this->appendRegionRun(runState, advanceX);
}

void CoreTextLineShaper::appendRegionRun(CoreTextRunState& runState, SkScalar& advanceX) {
    SkFontMetrics fontMetrics;
    runState.fFont.getMetrics(&fontMetrics);
    const SkScalar runHeight =
            runState.fStyle->getHeightOverride() ? runState.fStyle->getHeight() : 0.0f;

    const SkShaper::RunHandler::RunInfo runInfo = {
            runState.fFont,
            runState.fBidiLevel,
            runState.fScript,
            runState.fLanguage,
            SkPoint::Make(runState.fRunAdvanceX,
                          fontMetrics.fDescent - fontMetrics.fAscent + fontMetrics.fLeading),
            static_cast<size_t>(runState.fGlyphCount),
            SkShaper::RunHandler::Range(0, runState.fUtf8Range.width())};

    auto& run = fParagraph->fRuns.emplace_back(fParagraph,
                                               runInfo,
                                               runState.fUtf8Range.start,
                                               runHeight,
                                               runState.fStyle->getHalfLeading(),
                                               runState.fStyle->getBaselineShift(),
                                               fParagraph->fRuns.size(),
                                               advanceX);
    // Also writes the N+1 sentinel (run end position and cluster index) that line breaking and
    // caret hit-testing interpolate against
    run.allocateGlyphData(static_cast<size_t>(runState.fGlyphCount), /*hasOffsets=*/false);

    if (runState.fIsFallback) {
        this->populateFallbackRun(run, runState, advanceX);
    } else {
        this->populateShapedRun(run, runState, advanceX);
    }

    // CoreText runs keep the end sentinel on the baseline, where allocateGlyphData puts it at
    // fOffset + fAdvance. commit() may have moved the glyph data, so read it again
    run.fGlyphData->positions()[runState.fGlyphCount] =
            SkPoint::Make(advanceX + runState.fRunAdvanceX, 0);

    fParagraph->fFontSwitches.emplace_back(runState.fUtf8Range.start, run.font());
    advanceX += runState.fRunAdvanceX;
}

void CoreTextLineShaper::populateFallbackRun(Run& run,
                                             const CoreTextRunState& runState,
                                             SkScalar runStartX) {
    SkPoint* positions = run.fGlyphData->positions();
    uint32_t* clusterIndexes = run.fGlyphData->clusterIndexes();
    SkGlyphID* glyphs = run.fGlyphData->glyphs();

    // CoreText renders missing glyphs through LastResort; remap to glyph 0 (.notdef) to honor
    // SkParagraph unresolved contract
    SkScalar accumulatedX = 0;
    for (CFIndex k = 0; k < runState.fGlyphCount; ++k) {
        glyphs[k] = 0;
        positions[k] = SkPoint::Make(runStartX + accumulatedX, 0);
        accumulatedX += runState.fAdvances[k];
        clusterIndexes[k] = runState.fClusters[k];
        this->recordUnresolvedGlyph(glyph_text_range(runState, k));
    }
}

void CoreTextLineShaper::populateShapedRun(Run& run,
                                           CoreTextRunState& runState,
                                           SkScalar runStartX) {
    SkPoint* positions = run.fGlyphData->positions();
    uint32_t* clusterIndexes = run.fGlyphData->clusterIndexes();
    SkGlyphID* glyphs = run.fGlyphData->glyphs();

    if (runState.fShapingBuffer->fOffsets.size() < SkToInt(runState.fGlyphCount)) {
        runState.fShapingBuffer->fOffsets.resize_back(SkToInt(runState.fGlyphCount));
    }
    SkPoint* offsets = runState.fShapingBuffer->fOffsets.data();
    // CTRunGetPositions folds mark draw offsets in and lacks the non-subpixel and newline
    // adjustments, so the pen walks fAdvances instead
    SkScalar penX = 0;
    // A run of only characters CoreText drops (C0 controls) keeps one zero-advance
    // kCGFontIndexInvalid placeholder, no glyph id of the font; space draws nothing
    // KEEPER-DEBT: a font without a space glyph gets .notdef at zero advance; remove once Run can
    // hold a glyph-less cluster
    const SkGlyphID invisibleGlyph = runState.fFont.unicharToGlyph(' ');
    for (CFIndex k = 0; k < runState.fGlyphCount; ++k) {
        glyphs[k] =
                runState.fGlyphs[k] == kCGFontIndexInvalid ? invisibleGlyph : runState.fGlyphs[k];
        // Cluster widths and hit-testing read the positions, and extendedVisit and getPath hand
        // the positions out without the offsets, so the whole origin goes into the offset as
        // SkShaper's HarfBuzz runs put x_offset and y_offset there
        positions[k] = SkPoint::Make(runStartX + penX, 0);
        penX += runState.fAdvances[k];
        offsets[k] = glyph_draw_offset(runState.fOrigins[k],
                                       runState.fZeroSize,
                                       runState.fSplitKerning ? runState.fSplitOffsets[k] : 0);
        clusterIndexes[k] = runState.fClusters[k];
        if (glyphs[k] == 0) {
            this->recordUnresolvedGlyph(glyph_text_range(runState, k));
        }
    }
    run.commit(*runState.fShapingBuffer);
}

// Control and zero-width format characters never count toward unresolved glyphs or codepoints,
// matching HarfBuzz OneLineShaper::iterateThroughFontStyles
void CoreTextLineShaper::recordUnresolvedGlyph(TextRange glyphRange) {
    const char* textStart = fParagraph->text().data();
    const char* p = textStart + glyphRange.start;
    const char* pEnd = textStart + glyphRange.end;
    bool hasNonControl = false;
    while (p < pEnd) {
        const size_t bytePos = SkToSizeT(p - textStart);
        const SkUnichar u = SkUTF::NextUTF8(&p, pEnd);
        if (u < 0) {
            break;
        }
        if (!fParagraph->codeUnitHasProperty(bytePos, SkUnicode::CodeUnitFlags::kControl) &&
            !IsZeroWidthCodepoint(u)) {
            hasNonControl = true;
            if (fParagraph->fUnresolvedCodepoints == nullptr) {
                fParagraph->fUnresolvedCodepoints =
                        std::make_shared<std::unordered_set<SkUnichar>>();
            }
            // No cache entry may see the set change
            SkASSERT(fParagraph->fUnresolvedCodepoints.use_count() == 1);
            fParagraph->fUnresolvedCodepoints->emplace(u);
        }
    }
    if (hasNonControl) {
        ++fUnresolvedGlyphs;
    }
}

}  // namespace textlayout
}  // namespace skia
