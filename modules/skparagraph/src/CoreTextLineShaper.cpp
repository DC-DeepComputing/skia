// Copyright 2026 Google LLC
#include "modules/skparagraph/src/CoreTextLineShaper.h"

#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFourByteTag.h"
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
#include "src/core/SkNoDestructor.h"
#include "src/core/SkTHash.h"
#include "src/core/SkUTF.h"
#include "src/utils/mac/SkCGBase.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string_view>
#include <utility>

namespace skia {
namespace textlayout {

struct RunFlags {
    bool ltr;
    bool zeroSize;
    bool fallback;
    bool splitKerning;
};

struct CoreTextRunState {
    CoreTextRunState(int glyphCount,
                     TextRange utf8Range,
                     RunFlags flags,
                     SkFont font,
                     Run::ShapingBuffer* shapingBuffer)
            : fFont(std::move(font))
            , fGlyphCount(glyphCount)
            , fUtf8Range(utf8Range)
            , fIsLtr(flags.ltr)
            , fZeroSize(flags.zeroSize)
            , fIsFallback(flags.fallback)
            , fSplitKerning(flags.splitKerning)
            , fShapingBuffer(shapingBuffer)
            , fClusters(glyphCount)
            , fAdvances(glyphCount)
            , fOrigins(flags.fallback ? 0 : glyphCount)
            , fGlyphs(glyphCount)
            , fSplitOffsets(flags.splitKerning ? glyphCount : 0) {}

    SkFont fFont;
    const TextStyle* fStyle = nullptr;
    const char* fLanguage = "";
    SkFourByteTag fScript = 0;
    uint8_t fBidiLevel = 0;
    int fGlyphCount;
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
        CTRunRef run;
        CFIndex location;
        CFIndex length;
    };

    // Caches the active TextStyle block's resolved primary typeface, allowed fallback typefaces,
    // locale, and per-CTFont allowlist verdicts across neighboring CTRuns in the same Block
    struct BlockStyleState {
        bool initialized = false;
        const Block* block = nullptr;
        const TextStyle* style = nullptr;
        sk_sp<SkTypeface> primary;
        std::vector<sk_sp<SkTypeface>> allowedTypefaces;
        SkString language;
        skia_private::THashMap<CTFontRef, bool> allowedFontCache;
    };

    RegionShapeContext(ParagraphImpl* paragraph,
                       const std::vector<SkUnicode::BidiRegion>& bidiRegions,
                       TextRange regionRange)
            : fText(paragraph->text())
            , fStyles(paragraph->styles())
            , fBidiRegions(bidiRegions)
            , fRegionRange(regionRange)
            , fFallbackDisabled(!paragraph->fontCollection()->fontFallbackEnabled())
            , fBuilder(std::string_view(fText.data() + regionRange.start, regionRange.width()),
                       paragraph->paragraphStyle().getTextDirection(),
                       paragraph->fontCollection(),
                       paragraph->getUnicode().get()) {}

    SkSpan<const char> fText;
    SkSpan<const Block> fStyles;
    const std::vector<SkUnicode::BidiRegion>& fBidiRegions;
    TextRange fRegionRange;
    bool fFallbackDisabled;
    CFAttributedStringRef fAttrStr = nullptr;
    CoreTextAttrStringBuilder fBuilder;
    skia_private::STArray<16, RunItem> fSortedRuns;
    // Declared before fTypefaceCache and fBlockState so a cached unstyled-gap CTFont outlives
    // every raw CTFontRef key in both hash maps
    SkUniqueCFRef<CTFontRef> fGapAttrFont;
    skia_private::THashMap<CTFontRef, sk_sp<SkTypeface>> fTypefaceCache;
    BlockStyleState fBlockState;
    // Run::commit keeps the glyph offsets only when one is not zero, as for SkShaper runs
    Run::ShapingBuffer fShapingBuffer;
};

namespace {

// How shapeRegionRun repairs a CTRun around its region's only '\n'. Pinning needs the run itself
// to be that '\n' and coalescing needs a '\n' CTRun after it, so one run gets at most one repair
enum class NewlineFix { kNone, kPinnedStandalone, kCoalescedTrailing };

struct NewlineRunFixup {
    int ctGlyphCount = 0;
    NewlineFix fix = NewlineFix::kNone;
    CGGlyph coalescedGlyph = 0;
    // First runState slot of ctRun's glyphs and the slot of a coalesced '\n' glyph
    int base = 0;
    int newlineSlot = 0;
};

struct AffixStyleState {
    TextStyle style;
    std::vector<sk_sp<SkTypeface>> allowedTypefaces;
    sk_sp<SkTypeface> primary;
    CTFontRef primaryCtFont = nullptr;
    bool fallbackDisabled = false;
    bool primaryDisallowed = false;
    bool zeroSize = false;
    bool hasTab = false;
    SkFont font;
    uint8_t level = 0;
};

struct AffixGlyphData {
    explicit AffixGlyphData(int totalGlyphs)
            : fCount(totalGlyphs)
            , fGlyphs(totalGlyphs)
            , fOrigins(totalGlyphs)
            , fAdvances(totalGlyphs)
            , fSplitOffsets(totalGlyphs)
            , fClusters(totalGlyphs) {
        std::fill_n(fSplitOffsets.data(), totalGlyphs, 0.0f);
    }

    int fCount;
    SkScalar fTotalAdvance = 0;
    skia_private::AutoSTArray<16, CGGlyph> fGlyphs;
    skia_private::AutoSTArray<16, CGPoint> fOrigins;
    skia_private::AutoSTArray<16, SkScalar> fAdvances;
    // The part of a kerning pair moved into each glyph's offset, as in shapeTextRegion
    skia_private::AutoSTArray<16, SkScalar> fSplitOffsets;
    skia_private::AutoSTArray<16, uint32_t> fClusters;
};

// Writes each glyph's advance from CoreText's base advances, rounded to whole pixels when subpixel
// is off (the meaning of SkFont::isSubpixel), and when splitOffsets is non-empty moves half of
// each spacing glyph's pair kerning (base advance minus nominal advance) onto the next glyph's
// advance and x-offset so drawn x stays unchanged while the caret sits at kern/2
static void coretext_advances(bool subpixel,
                              SkSpan<const CGSize> nominal,
                              SkSpan<const CGSize> advances,
                              SkSpan<SkScalar> out,
                              SkSpan<SkScalar> splitOffsets) {
    SkASSERT(advances.size() == out.size());
    // Half a kerning pair would put the next glyph off the pixel grid
    SkASSERT(subpixel || splitOffsets.empty());
    SkASSERT(splitOffsets.empty() ||
             (splitOffsets.size() == advances.size() && nominal.size() == advances.size()));
    const size_t count = advances.size();
    SkScalar carry = 0;
    for (size_t k = 0; k < count; ++k) {
        const SkScalar advance = SkScalarFromCGFloat(advances[k].width);
        if (splitOffsets.empty()) {
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

// Reports whether font is Apple's LastResort face, which CoreText binds once every cascade font
// lacks the code point; its glyphs stand for unresolved text
static bool is_last_resort_font(CTFontRef font) {
    SkASSERT(font);
    SkUniqueCFRef<CFStringRef> psName(CTFontCopyPostScriptName(font));
    if (!psName) {
        return false;
    }
    const CFRange r = CFStringFind(psName.get(), CFSTR("LastResort"), kCFCompareCaseInsensitive);
    return r.location != kCFNotFound;
}

// Reports whether ctFont is LastResort or an unrequested fallback when font fallback is disabled
static bool is_disallowed_ct_font(CTFontRef ctFont,
                                  bool fallbackDisabled,
                                  const std::vector<sk_sp<SkTypeface>>& allowedTypefaces,
                                  const std::vector<SkString>& fontFamilies) {
    return is_last_resort_font(ctFont) ||
           (fallbackDisabled && !is_allowed_font(ctFont, allowedTypefaces, fontFamilies));
}

// Sets font's edging, hinting and subpixel from style. When fakeMissingFontStyles is true and font
// has a typeface, also sets embolden and skew for the weight or italic that style asks for and the
// bound face lacks; ctFont, when non-null, lets a system UI italic face whose descriptor omits
// kCTFontSlantTrait count as italic. Modifies only font
static void apply_font_style_props(SkFont* font,
                                   CTFontRef ctFont,
                                   const TextStyle& style,
                                   bool fakeMissingFontStyles) {
    SkASSERT(font);
    font->setEdging(style.getFontEdging());
    font->setHinting(style.getFontHinting());
    font->setSubpixel(style.getSubpixel());

    // CoreText binds the closest real face and never synthesizes a missing bold or italic,
    // so the run gets the same fake flags OneLineShaper derives from the typeface gap
    if (!fakeMissingFontStyles || !font->getTypeface()) {
        return;
    }
    const SkFontStyle wanted = style.getFontStyle();
    const SkFontStyle bound = font->getTypeface()->fontStyle();
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
    font->setEmbolden(fakeBold);
    font->setSkewX(fakeItalic ? -SK_Scalar1 / 4 : 0);
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
    apply_font_style_props(&font, isFallback ? nullptr : ctFont, style, fakeMissingFontStyles);
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

// Reordered complex-script glyphs (kCTRunStatusNonMonotonic) can report out-of-order string
// indices; Run::iterateThroughClustersInTextOrder requires non-decreasing indices for LTR and
// non-increasing for RTL while keeping CoreText's glyph IDs and positions untouched
static void normalize_cluster_indices(SkSpan<uint32_t> clusters, bool ltr) {
    const int glyphCount = SkToInt(clusters.size());
    if (glyphCount <= 1) {
        return;
    }
    if (ltr) {
        for (int k = glyphCount - 2; k >= 0; --k) {
            if (clusters[k] > clusters[k + 1]) {
                clusters[k] = clusters[k + 1];
            }
        }
    } else {
        for (int k = 1; k < glyphCount; ++k) {
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
    // CTTypesetterCreateWithAttributedString disables GSUB/GPOS/morx/kerx above 10,240 UTF-16
    // code units unless unbounded layout is explicitly enabled
    static const SkNoDestructor<SkUniqueCFRef<CFDictionaryRef>> kUnboundedOptions([] {
        std::array<const void*, 1> keys = {kCTTypesetterOptionAllowUnboundedLayout};
        std::array<const void*, 1> values = {kCFBooleanTrue};
        return SkUniqueCFRef<CFDictionaryRef>(CFDictionaryCreate(kCFAllocatorDefault,
                                                                 keys.data(),
                                                                 values.data(),
                                                                 SkTo<CFIndex>(keys.size()),
                                                                 &kCFTypeDictionaryKeyCallBacks,
                                                                 &kCFTypeDictionaryValueCallBacks));
    }());
    const CFRange range = CFRangeMake(0, CFAttributedStringGetLength(attrStr));
    SkUniqueCFRef<CTTypesetterRef> typesetter(
            CTTypesetterCreateWithAttributedStringAndOptions(attrStr, kUnboundedOptions->get()));
    if (!typesetter) {
        return nullptr;
    }
    return SkUniqueCFRef<CTLineRef>(CTTypesetterCreateLine(typesetter.get(), range));
}

// Extracts ctRun's glyph IDs, CoreText origins, and horizontal glyph advances into the equally
// sized spans, zeroing advances when zeroSize is true, rounding when subpixel is false, and
// splitting pair kerning into splitOffsetsOut when it is non-empty
static void extract_ctrun_glyphs(CTRunRef ctRun,
                                 CTFontRef ctFont,
                                 bool zeroSize,
                                 bool subpixel,
                                 SkSpan<CGGlyph> glyphsOut,
                                 SkSpan<CGPoint> originsOut,
                                 SkSpan<SkScalar> advancesOut,
                                 SkSpan<SkScalar> splitOffsetsOut) {
    SkASSERT(glyphsOut.size() == originsOut.size() && glyphsOut.size() == advancesOut.size());
    SkASSERT(splitOffsetsOut.empty() || splitOffsetsOut.size() == glyphsOut.size());
    const CFIndex count = SkTo<CFIndex>(glyphsOut.size());
    const CFRange range = CFRangeMake(0, count);
    CTRunGetGlyphs(ctRun, range, glyphsOut.data());
    skia_private::AutoSTArray<64, CGSize> baseAdvances(glyphsOut.size());
    // CTRunGetAdvances derives advances from drawn x, so a zero-advance mark drawn d off the pen
    // shows as +d on the glyph before it in the run and -d on the mark. Base advances leave mark
    // positioning to the origins, as HarfBuzz leaves it to x_offset
    CTRunGetBaseAdvancesAndOrigins(ctRun, range, baseAdvances.data(), originsOut.data());
    // The 12pt stand-in advances CoreText gives zero-size text never reach the run
    if (zeroSize) {
        std::fill(advancesOut.begin(), advancesOut.end(), 0.0f);
        std::fill(splitOffsetsOut.begin(), splitOffsetsOut.end(), 0.0f);
        return;
    }
    skia_private::AutoSTArray<64, CGSize> nominal(splitOffsetsOut.empty() ? 0 : count);
    if (!splitOffsetsOut.empty()) {
        SkASSERT(ctFont);
        CTFontGetAdvancesForGlyphs(
                ctFont, kCTFontOrientationHorizontal, glyphsOut.data(), nominal.data(), count);
    }
    // KEEPER-DEBT: this keeps CoreText's advance to the next 28pt tab stop, where HarfBuzz gives
    // U+0009 its hmtx advance; the stop counts from the CTLine start (hard break or placeholder),
    // so after a soft wrap it is off the line's stops. Remove when tabs get the HarfBuzz advance
    // or line-relative stops
    coretext_advances(subpixel,
                      {nominal.data(), nominal.size()},
                      {baseAdvances.data(), baseAdvances.size()},
                      advancesOut,
                      splitOffsetsOut);
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
static int resolve_affix_runs(CFArrayRef runs,
                              bool fakeMissingFontStyles,
                              uint8_t bidiLevel,
                              AffixStyleState* state) {
    SkASSERT(state);
    // CTLineGetGlyphRuns is declared nonnull and ShapeAffix passes a typeset line
    SkASSERT(runs);
    const CFIndex runCount = CFArrayGetCount(runs);
    // ShapeHandler::runBuffer accepts at most one Run, so multi-run affixes coalesce into one
    // buffer with the first resolved CTRun's font and mark glyphs from other fonts unresolved
    int totalGlyphs = 0;
    CTRunRef primaryRun = nullptr;
    for (CFIndex i = 0; i < runCount; ++i) {
        CTRunRef ctRun = (CTRunRef)CFArrayGetValueAtIndex(runs, i);
        const int gc = SkToInt(CTRunGetGlyphCount(ctRun));
        if (gc <= 0) {
            continue;
        }
        totalGlyphs += gc;
        CTFontRef ctFont = ct_run_font(ctRun);
        const bool disallowed = is_disallowed_ct_font(ctFont,
                                                      state->fallbackDisabled,
                                                      state->allowedTypefaces,
                                                      state->style.getFontFamilies());
        // A disallowed primary (LastResort, or a system font hidden by disabled fallback) zeroes
        // every glyph of the buffer, so the first allowed CTRun takes its place
        if (!primaryRun || (!disallowed && state->primaryDisallowed)) {
            primaryRun = ctRun;
            state->primaryCtFont = ctFont;
            state->primaryDisallowed = disallowed;
        }
    }
    if (totalGlyphs == 0) {
        return 0;
    }
    // The first non-empty CTRun sets primaryRun
    SkASSERT(primaryRun);

    state->zeroSize = state->style.getFontSize() <= 0;
    state->font = make_run_font(state->primaryCtFont,
                                state->primaryDisallowed,
                                state->primary,
                                state->style,
                                fakeMissingFontStyles,
                                nullptr);
    state->level = reconcile_bidi_level(bidiLevel, primaryRun);
    return totalGlyphs;
}

// Returns true when ctFont draws the same face as primary. A cursive CTRun of a letter-spaced
// affix gets the primary face without liga=0, and CFEqual on CTFontRef also compares feature
// settings, so the glyph IDs are checked against the underlying CGFont as well
static bool shares_primary_face(CTFontRef ctFont, CTFontRef primary) {
    if (CFEqual(ctFont, primary)) {
        return true;
    }
    SkUniqueCFRef<CGFontRef> face(CTFontCopyGraphicsFont(ctFont, nullptr));
    SkUniqueCFRef<CGFontRef> primaryFace(CTFontCopyGraphicsFont(primary, nullptr));
    return CFEqual(face.get(), primaryFace.get());
}

// Collects glyphs, origins, advances, split kerning offsets, and normalized UTF-8 clusters across
// all non-empty CTRuns of an affix line, zeroing glyph IDs from disallowed or secondary CTRuns
static void collect_affix_glyphs(CFArrayRef runs,
                                 const CoreTextIndexMap& indexMap,
                                 const AffixStyleState& state,
                                 AffixGlyphData* out) {
    SkASSERT(out);
    // ShapeAffix returns early when resolve_affix_runs finds zero glyphs
    SkASSERT(runs);
    const CFIndex runCount = CFArrayGetCount(runs);
    int runStart = 0;
    for (CFIndex i = 0; i < runCount; ++i) {
        CTRunRef ctRun = (CTRunRef)CFArrayGetValueAtIndex(runs, i);
        const int gc = SkToInt(CTRunGetGlyphCount(ctRun));
        if (gc <= 0) {
            continue;
        }
        const size_t n = SkToSizeT(gc);
        CTFontRef ctFont = ct_run_font(ctRun);
        // Left-to-right subpixel runs without tabs split pair kerning as in shapeTextRegion
        const bool splitKerning = !state.zeroSize && state.font.isSubpixel() && !state.hasTab &&
                                  (CTRunGetStatus(ctRun) & kCTRunStatusRightToLeft) == 0;
        // Base advances keep a mark's drawn offset off the glyph before it, as in shapeTextRegion;
        // a zero-size affix keeps zero advances
        extract_ctrun_glyphs(ctRun,
                             ctFont,
                             state.zeroSize,
                             state.font.isSubpixel(),
                             {out->fGlyphs.data() + runStart, n},
                             {out->fOrigins.data() + runStart, n},
                             {out->fAdvances.data() + runStart, n},
                             splitKerning
                                     ? SkSpan<SkScalar>{out->fSplitOffsets.data() + runStart, n}
                                     : SkSpan<SkScalar>{});
        for (int k = 0; k < gc; ++k) {
            out->fTotalAdvance += out->fAdvances[runStart + k];
        }

        const bool unresolvedRun = state.primaryDisallowed ||
                                   is_disallowed_ct_font(ctFont,
                                                         state.fallbackDisabled,
                                                         state.allowedTypefaces,
                                                         state.style.getFontFamilies()) ||
                                   !shares_primary_face(ctFont, state.primaryCtFont);
        if (unresolvedRun) {
            std::fill_n(out->fGlyphs.data() + runStart, gc, 0);
        }

        skia_private::AutoSTArray<16, CFIndex> indices(gc);
        CTRunGetStringIndices(ctRun, CFRangeMake(0, gc), indices.data());
        for (int k = 0; k < gc; ++k) {
            out->fClusters[runStart + k] = SkToU32(indexMap.toUtf8Index(indices[k]));
        }
        runStart += gc;
    }
    normalize_cluster_indices({out->fClusters.data(), SkToSizeT(out->fCount)},
                              (state.level % 2) == 0);
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
    for (int g = 0; g < data.fCount; ++g) {
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
        ParagraphImpl* paragraph, TextRange regionRange, CoreTextAttrStringBuilder* builder) {
    SkASSERT(paragraph);
    SkASSERT(builder);
    const BlockRange blockRange = paragraph->findAllBlocks(regionRange);
    for (const auto& block : paragraph->blocks(blockRange)) {
        const size_t start = std::max(block.fRange.start, regionRange.start);
        const size_t end = std::min(block.fRange.end, regionRange.end);
        if (start < end) {
            builder->addStyle(TextRange(start - regionRange.start, end - regionRange.start),
                              block.fStyle);
        }
    }
    return builder->build();
}

// Collects non-empty CTRuns from line and sorts them into ascending UTF-16 string order while
// preserving CoreText's relative order for runs sharing a start index
static void collect_sorted_ct_runs(
        CTLineRef line, skia_private::STArray<16, RegionShapeContext::RunItem>* sortedRuns) {
    SkASSERT(sortedRuns);
    CFArrayRef runs = CTLineGetGlyphRuns(line);
    // CTLineGetGlyphRuns is declared nonnull; a line without glyphs gives an empty array
    SkASSERT(runs);
    const CFIndex runCount = CFArrayGetCount(runs);
    for (CFIndex i = 0; i < runCount; ++i) {
        CTRunRef ctRun = (CTRunRef)CFArrayGetValueAtIndex(runs, i);
        // Drop empty runs upfront so the trailing-newline lookahead sees no empty CTRuns after '\n'
        if (CTRunGetGlyphCount(ctRun) == 0) {
            continue;
        }
        const CFRange cfRange = CTRunGetStringRange(ctRun);
        if (cfRange.length == 0) {
            continue;
        }
        sortedRuns->push_back({ctRun, cfRange.location, cfRange.length});
    }
    // CTRuns arrive in visual order from CTLineGetGlyphRuns
    // Logical ascending sort prevents BiDi inversion downstream in cluster layout
    std::stable_sort(sortedRuns->begin(),
                     sortedRuns->end(),
                     [](const RegionShapeContext::RunItem& a,
                        const RegionShapeContext::RunItem& b) { return a.location < b.location; });
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
                                     RegionShapeContext::BlockStyleState* state) {
    SkASSERT(state);
    if (state->initialized && block == state->block) {
        return;
    }
    state->initialized = true;
    state->block = block;
    state->style = block ? &block->fStyle : &paragraph.paragraphStyle().getTextStyle();
    state->language = state->style->getLocale();
    state->allowedFontCache.reset();
    state->allowedTypefaces.clear();
    state->primary = state->style->refTypeface();
    if (state->primary) {
        state->allowedTypefaces.push_back(state->primary);
        return;
    }
    // Same clone the attributed string was built from, so LastResort runs and the allowed-font
    // check see the varied face (or default .AppleSystemUIFont) rather than a null typeface
    state->allowedTypefaces =
            CoreTextFontRegistry::Instance().findTypefaces(*paragraph.fontCollection(),
                                                           state->style->getFontFamilies(),
                                                           state->style->getFontStyle(),
                                                           state->style->getFontArguments());
    if (!state->allowedTypefaces.empty()) {
        state->primary = state->allowedTypefaces.front();
    }
}

// Reports whether ctFont is LastResort or an unrequested fallback when font fallback is disabled,
// memoizing per-CTFont allowlist checks in state for the active Block
static bool is_disallowed_run_font(CTFontRef ctFont,
                                   bool fallbackDisabled,
                                   RegionShapeContext::BlockStyleState* state) {
    SkASSERT(state);
    if (is_last_resort_font(ctFont)) {
        return true;
    }
    if (!fallbackDisabled) {
        return false;
    }
    bool* allowed = state->allowedFontCache.find(ctFont);
    if (!allowed) {
        allowed = state->allowedFontCache.set(
                ctFont,
                is_allowed_font(ctFont, state->allowedTypefaces, state->style->getFontFamilies()));
    }
    return !*allowed;
}

// Reads kCTFontAttributeName at utf16Loc from ctx->fAttrStr to preserve exact descriptor attributes
// (such as liga=0 or variations) for SkTypeface equality, creating and caching the paragraph
// default CTFont once in ctx->fGapAttrFont for unstyled gaps
static CTFontRef attr_primary_ct_font(RegionShapeContext* ctx, CFIndex utf16Loc) {
    SkASSERT(ctx);
    if (CTFontRef font = (CTFontRef)CFAttributedStringGetAttribute(
                ctx->fAttrStr, utf16Loc, kCTFontAttributeName, nullptr)) {
        return font;
    }
    if (ctx->fBlockState.block) {
        return nullptr;
    }
    if (!ctx->fGapAttrFont) {
        const TextStyle& style = *ctx->fBlockState.style;
        const CGFloat standInSize = style.getFontSize() > 0.0f ? style.getFontSize() : 14.0f;
        ctx->fGapAttrFont = CoreTextFontRegistry::Instance().createFont(
                ctx->fBlockState.primary.get(), style.getFontStyle(), standInSize);
    }
    return ctx->fGapAttrFont.get();
}

// CoreText cascades U+000A to a fallback font when the primary face lacks a newline glyph.
// Returns the block's primary CTFont to keep a standalone '\n' on as glyph 0, so empty-line
// metrics and font equality match the primary face, or nullptr when no pin applies
static CTFontRef try_pin_standalone_newline(RegionShapeContext* ctx,
                                            TextRange runUtf8Range,
                                            CFIndex utf16Loc) {
    SkASSERT(ctx);
    if (runUtf8Range.width() != 1 || ctx->fText[runUtf8Range.start] != '\n') {
        return nullptr;
    }
    CTFontRef attrFont = attr_primary_ct_font(ctx, utf16Loc);
    const UniChar nlChar = '\n';
    CGGlyph nlGlyph = 0;
    if (!attrFont || CTFontGetGlyphsForCharacters(attrFont, &nlChar, &nlGlyph, 1) ||
        is_disallowed_run_font(attrFont, ctx->fFallbackDisabled, &ctx->fBlockState)) {
        return nullptr;
    }
    return attrFont;
}

// CoreText splits a trailing '\n' into a separate CTRun when the primary face lacks U+000A or on
// RTL lines. When the CTRun after runIdx is that '\n', returns ctFont's glyph for it (0 when
// unmapped) so the caller folds it into this run and line-end caret hit-testing and multi-line
// spacing see one run per segment; std::nullopt otherwise
static std::optional<CGGlyph> coalesce_trailing_newline(RegionShapeContext* ctx,
                                                        int runIdx,
                                                        CTFontRef ctFont,
                                                        uint8_t bidiLevel,
                                                        TextRange runUtf8Range) {
    SkASSERT(ctx);
    if (runIdx + 2 != ctx->fSortedRuns.size() || runUtf8Range.end + 1 != ctx->fRegionRange.end ||
        ctx->fText[ctx->fRegionRange.end - 1] != '\n') {
        return std::nullopt;
    }
    const RegionShapeContext::RunItem& item = ctx->fSortedRuns[runIdx];
    const RegionShapeContext::RunItem& nlItem = ctx->fSortedRuns[runIdx + 1];
    if (nlItem.location != item.location + item.length || nlItem.length != 1 ||
        CTRunGetGlyphCount(nlItem.run) != 1 ||
        find_style_block(ctx->fStyles, runUtf8Range.end) != ctx->fBlockState.block ||
        reconcile_bidi_level(find_bidi_level(ctx->fBidiRegions, runUtf8Range.end, 0), nlItem.run) !=
                bidiLevel) {
        return std::nullopt;
    }
    CTFontRef nlAttrFont = attr_primary_ct_font(ctx, nlItem.location);
    if (!nlAttrFont || !CFEqual(ctFont, nlAttrFont)) {
        return std::nullopt;
    }
    const UniChar nlChar = '\n';
    // CoreText writes glyph 0 for a character the font does not map
    CGGlyph glyph = 0;
    CTFontGetGlyphsForCharacters(ctFont, &nlChar, &glyph, 1);
    return glyph;
}

// Maps ctRun's UTF-16 string indices to run-relative UTF-8 byte offsets in runState->fClusters,
// normalizes them to monotonic cluster order, and writes a coalesced trailing newline's cluster at
// visual slot 0 for RTL runs or at the end for LTR runs
static void build_run_clusters(CTRunRef ctRun,
                               const CoreTextIndexMap& indexMap,
                               size_t regionStart,
                               const NewlineRunFixup& fixup,
                               CoreTextRunState* runState) {
    SkASSERT(runState);
    const int ctGlyphCount = fixup.ctGlyphCount;
    skia_private::AutoSTArray<64, CFIndex> indexBuffer(ctGlyphCount);
    CTRunGetStringIndices(ctRun, CFRangeMake(0, ctGlyphCount), indexBuffer.data());
    for (int k = 0; k < ctGlyphCount; ++k) {
        const size_t globalU8 = regionStart + indexMap.toUtf8Index(indexBuffer[k]);
        const size_t clusterInRun = (globalU8 >= runState->fUtf8Range.start)
                                            ? (globalU8 - runState->fUtf8Range.start)
                                            : 0;
        runState->fClusters[fixup.base + k] = SkToU32(clusterInRun);
    }
    normalize_cluster_indices({runState->fClusters.data() + fixup.base, SkToSizeT(ctGlyphCount)},
                              runState->fIsLtr);
    if (fixup.fix == NewlineFix::kCoalescedTrailing) {
        runState->fClusters[fixup.newlineSlot] = SkToU32(runState->fUtf8Range.width() - 1);
    }
}

// Returns glyph k's paragraph-global UTF-8 text range from normalized run-relative clusters
static TextRange glyph_text_range(const CoreTextRunState& runState, int k) {
    const size_t cStart = runState.fClusters[k];
    const size_t cEnd =
            runState.fIsLtr ? ((k + 1 < runState.fGlyphCount) ? runState.fClusters[k + 1]
                                                              : runState.fUtf8Range.width())
                            : ((k > 0) ? runState.fClusters[k - 1] : runState.fUtf8Range.width());
    // build_run_clusters normalizes the clusters to text order and keeps each below the width
    SkASSERT(cStart <= cEnd);
    return TextRange(runState.fUtf8Range.start + cStart, runState.fUtf8Range.start + cEnd);
}

// Returns true for the C0/C1 controls with Bidi_Class BN, which a native CTLine drops without a
// glyph or advance; tab, the separators U+001C..U+001F and U+0085 are not BN
static bool is_coretext_omitted_control(SkUnichar u) {
    return (u >= 0x0001 && u <= 0x0008) || (u >= 0x000E && u <= 0x001B) ||
           (u >= 0x007F && u <= 0x0084) || (u >= 0x0086 && u <= 0x009F);
}

// Assigns each fallback cluster the primary font's .notdef advance per code point that takes an
// advance, stores their sum in runState->fRunAdvanceX, and sets runState->fGlyphs[k] to 0 (.notdef
// box) or, for a cluster of only Default_Ignorable code points (Hangul fillers too) and dropped
// controls, to kCGFontIndexInvalid (draws nothing). Mn/Me marks get a box but no advance
static void measure_fallback_glyphs(SkSpan<const char> paragraphText,
                                    SkUnicode* unicode,
                                    CoreTextRunState* runState) {
    SkASSERT(runState);
    SkASSERT(unicode);
    SkScalar notdefAdvance = runState->fFont.getWidth(0);
    if (SkScalarNearlyZero(notdefAdvance)) {
        notdefAdvance = runState->fFont.getSize();
    }
    // .notdef gets the same advance as any other glyph, whole pixels included
    if (!runState->fFont.isSubpixel()) {
        notdefAdvance = SkScalarRoundToScalar(notdefAdvance);
    }

    for (int k = 0; k < runState->fGlyphCount; ++k) {
        const TextRange gRange = glyph_text_range(*runState, k);
        const char* p = paragraphText.data() + gRange.start;
        const char* pEnd = paragraphText.data() + gRange.end;
        int visibleCount = 0;
        bool hasBoxGlyph = false;
        while (p < pEnd) {
            const SkUnichar u = SkUTF::NextUTF8(&p, pEnd);
            // Malformed UTF-8 keeps its box, same as before this cluster check
            if (u < 0) {
                hasBoxGlyph = true;
                break;
            }
            if (unicode->isDefaultIgnorable(u) || is_coretext_omitted_control(u)) {
                continue;
            }
            hasBoxGlyph = true;
            if (!unicode->isNonSpacingMark(u)) {
                ++visibleCount;
            }
        }
        runState->fGlyphs[k] = hasBoxGlyph ? 0 : kCGFontIndexInvalid;
        runState->fAdvances[k] = visibleCount * notdefAdvance;
        runState->fRunAdvanceX += runState->fAdvances[k];
    }
}

// Extracts ctRun's glyphs, origins, and shaped advances (plus split kerning offsets when enabled)
// into runState, populates any pinned or coalesced newline slot, applies the primary font's
// newline advance to U+000A clusters, and stores the total advance in runState->fRunAdvanceX
static void measure_shaped_glyphs(CTRunRef ctRun,
                                  CTFontRef ctFont,
                                  SkSpan<const char> paragraphText,
                                  const NewlineRunFixup& fixup,
                                  CoreTextRunState* runState) {
    SkASSERT(runState);
    // ctRun holds the fallback font's glyph ID rather than the pinned primary face's, so emit
    // .notdef (0) directly and assign its HarfBuzz advance in the newline loop below
    if (fixup.fix == NewlineFix::kPinnedStandalone) {
        runState->fGlyphs[0] = 0;
        runState->fOrigins[0] = CGPointZero;
        runState->fAdvances[0] = 0.0f;
        if (runState->fSplitKerning) {
            runState->fSplitOffsets[0] = 0.0f;
        }
    } else {
        const int base = fixup.base;
        const size_t n = SkToSizeT(fixup.ctGlyphCount);
        extract_ctrun_glyphs(ctRun,
                             ctFont,
                             runState->fZeroSize,
                             runState->fFont.isSubpixel(),
                             {runState->fGlyphs.data() + base, n},
                             {runState->fOrigins.data() + base, n},
                             {runState->fAdvances.data() + base, n},
                             runState->fSplitKerning
                                     ? SkSpan<SkScalar>{runState->fSplitOffsets.data() + base, n}
                                     : SkSpan<SkScalar>{});
        if (fixup.fix == NewlineFix::kCoalescedTrailing) {
            const int nl = fixup.newlineSlot;
            runState->fGlyphs[nl] = fixup.coalescedGlyph;
            runState->fOrigins[nl] = CGPointZero;
            runState->fAdvances[nl] = 0.0f;
            if (runState->fSplitKerning) {
                runState->fSplitOffsets[nl] = 0.0f;
            }
        }
    }

    for (int k = 0; k < runState->fGlyphCount; ++k) {
        // CoreText gives '\n' no advance, HarfBuzz the advance of the primary font's cmap glyph
        // (.notdef when it has none); that advance moves all later lines, so their float positions
        // and widths differ unless '\n' gets one here too
        const TextRange gRange = glyph_text_range(*runState, k);
        if (!runState->fZeroSize && gRange.width() == 1 && paragraphText[gRange.start] == '\n') {
            const SkScalar newlineAdvance =
                    runState->fFont.getWidth(runState->fFont.unicharToGlyph('\n'));
            // Whole pixels for a non-subpixel font, as for every other glyph
            runState->fAdvances[k] = runState->fFont.isSubpixel()
                                             ? newlineAdvance
                                             : SkScalarRoundToScalar(newlineAdvance);
        }
        runState->fRunAdvanceX += runState->fAdvances[k];
    }
}

}  // namespace

void CoreTextLineShaper::ShapeAffix(std::string_view text,
                                    const TextStyle& style,
                                    sk_sp<SkTypeface> clusterTypeface,
                                    bool fakeMissingFontStyles,
                                    uint8_t bidiLevel,
                                    sk_sp<FontCollection> fontCollection,
                                    SkUnicode* unicode,
                                    SkShaper::RunHandler* handler) {
    SkASSERT(handler);
    SkASSERT(unicode);
    // TextLine shapes an ellipsis only when the style is ellipsized, which needs a non-empty
    // string, and every paragraph owns a font collection
    SkASSERT(!text.empty());
    SkASSERT(fontCollection);
    const TextDirection baseDir = (bidiLevel & 1) ? TextDirection::kRtl : TextDirection::kLtr;
    AffixStyleState state;
    state.style = style;
    // Inherit the line's resolved cluster typeface when the ellipsis style does not specify an
    // explicit typeface or font family list
    if (!state.style.refTypeface() && state.style.getFontFamilies().empty() && clusterTypeface) {
        state.style.setTypeface(clusterTypeface);
    }

    state.allowedTypefaces =
            CoreTextFontRegistry::Instance().findTypefaces(*fontCollection,
                                                           state.style.getFontFamilies(),
                                                           state.style.getFontStyle(),
                                                           state.style.getFontArguments());
    state.primary = state.style.refTypeface();
    if (state.primary) {
        state.allowedTypefaces.insert(state.allowedTypefaces.begin(), state.primary);
    } else if (!state.allowedTypefaces.empty()) {
        state.primary = state.allowedTypefaces.front();
    }
    if (clusterTypeface) {
        state.allowedTypefaces.push_back(clusterTypeface);
    }
    state.fallbackDisabled = !fontCollection->fontFallbackEnabled();
    state.hasTab = memchr(text.data(), '\t', text.size()) != nullptr;

    CoreTextAttrStringBuilder builder(text, baseDir, std::move(fontCollection), unicode);
    builder.addStyle(TextRange(0, text.size()), state.style);
    SkUniqueCFRef<CFAttributedStringRef> attrStr = builder.build();
    if (!attrStr) {
        return;
    }
    SkUniqueCFRef<CTLineRef> line = typeset_line(attrStr.get());
    if (!line) {
        return;
    }

    CFArrayRef runs = CTLineGetGlyphRuns(line.get());
    const int totalGlyphs = resolve_affix_runs(runs, fakeMissingFontStyles, bidiLevel, &state);
    if (totalGlyphs == 0) {
        return;
    }

    AffixGlyphData glyphData(totalGlyphs);
    collect_affix_glyphs(runs, builder.indexMap(), state, &glyphData);

    const SkString language = state.style.getLocale();
    const SkShaper::RunHandler::RunInfo info = {state.font,
                                                state.level,
                                                builder.scriptAt(0),
                                                language.c_str(),
                                                SkVector::Make(glyphData.fTotalAdvance, 0),
                                                SkToSizeT(totalGlyphs),
                                                SkShaper::RunHandler::Range(0, text.size())};
    commit_affix_run(info, glyphData, state.zeroSize, handler);
}

CoreTextLineShaper::CoreTextLineShaper(ParagraphImpl* paragraph)
        : fParagraph(paragraph), fUnresolvedGlyphs(0), fAdvanceX(0) {}

bool CoreTextLineShaper::shape() {
    fUnresolvedGlyphs = 0;
    fAdvanceX = 0;
    // Reset before the empty-text return so every false result leaves the output empty
    this->resetParagraphOutput();
    if (fParagraph->text().size() == 0) {
        return false;
    }

    // ParagraphBuilderImpl::Build always appends a trailing zero-width sentinel placeholder
    // whose fTextBefore covers all text after the last explicit placeholder
    SkASSERT(!fParagraph->fPlaceholders.empty());

    for (int i = 0; i < fParagraph->fPlaceholders.size(); ++i) {
        const Placeholder& placeholder = fParagraph->fPlaceholders[i];
        if (placeholder.fTextBefore.width() > 0 && !this->shapeTextRange(placeholder.fTextBefore)) {
            // OneLineShaper appends to fRuns, so the runs of earlier regions must not survive
            this->resetParagraphOutput();
            return false;
        }
        if (placeholder.fRange.width() > 0) {
            this->appendPlaceholderRun(placeholder, SkToSizeT(i));
        }
    }

    return true;
}

void CoreTextLineShaper::resetParagraphOutput() {
    // Release the set, do not clear it: a ParagraphCache entry may share it
    fParagraph->fUnresolvedCodepoints.reset();
    fParagraph->fFontSwitches.clear();
    fParagraph->fRuns.clear();
}

void CoreTextLineShaper::appendPlaceholderRun(const Placeholder& placeholder,
                                              size_t placeholderIndex) {
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
                                               fAdvanceX);

    run.allocateGlyphData(1, /*hasOffsets=*/false);
    run.fGlyphData->glyphs()[0] = 0;
    run.fGlyphData->positions()[0] = {fAdvanceX, 0};
    // CoreText runs keep the end sentinel on the baseline
    run.fGlyphData->positions()[1] = {fAdvanceX + placeholder.fStyle.fWidth, 0};
    run.fGlyphData->clusterIndexes()[0] = 0;
    run.fPlaceholderIndex = placeholderIndex;
    fAdvanceX += placeholder.fStyle.fWidth;
}

// CoreText typesetter produces a single visual line; isolating hard line breaks keeps each
// paragraph segment in one CTLine while CoreTextAttrStringBuilder splits CTRuns across blocks
// with differing Run-level font or metric properties
bool CoreTextLineShaper::shapeTextRange(TextRange range) {
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

        if (!this->shapeTextRegion(TextRange(segStart, segEnd))) {
            return false;
        }

        segStart = segEnd;
    }

    return true;
}

bool CoreTextLineShaper::shapeTextRegion(TextRange regionRange) {
    // shapeTextRange advances segEnd past at least one byte before calling shapeTextRegion
    SkASSERT(regionRange.width() > 0);

    RegionShapeContext ctx(fParagraph, fParagraph->fBidiRegions, regionRange);
    SkUniqueCFRef<CFAttributedStringRef> attrStr =
            build_region_attr_string(fParagraph, regionRange, &ctx.fBuilder);
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

    ctx.fAttrStr = attrStr.get();
    collect_sorted_ct_runs(line.get(), &ctx.fSortedRuns);
    // shapeRegionRun consumes one or two CTRuns, so the loop always moves forward
    for (int runIdx = 0; runIdx < ctx.fSortedRuns.size();) {
        runIdx += this->shapeRegionRun(&ctx, runIdx);
    }

    return true;
}

int CoreTextLineShaper::shapeRegionRun(RegionShapeContext* ctx, int runIdx) {
    SkASSERT(ctx);
    const RegionShapeContext::RunItem& item = ctx->fSortedRuns[runIdx];
    const TextRange relUtf8Range =
            ctx->fBuilder.indexMap().cfrangeToTextRange(CFRangeMake(item.location, item.length));
    TextRange runUtf8Range(ctx->fRegionRange.start + relUtf8Range.start,
                           ctx->fRegionRange.start + relUtf8Range.end);
    if (runUtf8Range.width() == 0) {
        return 1;
    }

    update_block_style_state(
            *fParagraph, find_style_block(ctx->fStyles, runUtf8Range.start), &ctx->fBlockState);

    CTFontRef ctFont = ct_run_font(item.run);
    bool isFallback = is_disallowed_run_font(ctFont, ctx->fFallbackDisabled, &ctx->fBlockState);
    NewlineRunFixup fixup;
    fixup.ctGlyphCount = SkToInt(CTRunGetGlyphCount(item.run));
    if (CTFontRef pinned = try_pin_standalone_newline(ctx, runUtf8Range, item.location)) {
        ctFont = pinned;
        isFallback = false;
        fixup.fix = NewlineFix::kPinnedStandalone;
    }

    const uint8_t bidiLevel = reconcile_bidi_level(
            find_bidi_level(ctx->fBidiRegions, runUtf8Range.start, 0), item.run);
    const bool isLtr = (bidiLevel % 2) == 0;
    std::optional<CGGlyph> newlineGlyph;
    if (!isFallback) {
        newlineGlyph = coalesce_trailing_newline(ctx, runIdx, ctFont, bidiLevel, runUtf8Range);
    }
    if (newlineGlyph) {
        // A pinned run is the region's only '\n', so no trailing '\n' CTRun follows it
        SkASSERT(fixup.fix == NewlineFix::kNone);
        fixup.fix = NewlineFix::kCoalescedTrailing;
        fixup.coalescedGlyph = *newlineGlyph;
        // RTL runs store glyphs in visual order, so a logical-end '\n' sits at visual slot 0
        fixup.base = isLtr ? 0 : 1;
        fixup.newlineSlot = isLtr ? fixup.ctGlyphCount : 0;
        runUtf8Range.end = ctx->fRegionRange.end;
    }

    const int glyphCount = fixup.ctGlyphCount + (newlineGlyph ? 1 : 0);
    const bool zeroSize = ctx->fBlockState.style->getFontSize() <= 0;
    SkFont runFont = make_run_font(ctFont,
                                   isFallback,
                                   ctx->fBlockState.primary,
                                   *ctx->fBlockState.style,
                                   fParagraph->paragraphStyle().fakeMissingFontStyles(),
                                   &ctx->fTypefaceCache);
    // Only left-to-right subpixel runs split pair kerning; a tab's advance to its stop is not
    // kerning, so runs containing a tab keep whole advances
    const bool splitKerning =
            !isFallback && !zeroSize && isLtr && runFont.isSubpixel() &&
            !memchr(ctx->fText.data() + runUtf8Range.start, '\t', runUtf8Range.width());

    CoreTextRunState runState(glyphCount,
                              runUtf8Range,
                              {.ltr = isLtr,
                               .zeroSize = zeroSize,
                               .fallback = isFallback,
                               .splitKerning = splitKerning},
                              std::move(runFont),
                              &ctx->fShapingBuffer);
    runState.fStyle = ctx->fBlockState.style;
    runState.fLanguage = ctx->fBlockState.language.c_str();
    runState.fBidiLevel = bidiLevel;
    // The itemization that keeps cursive ranges unspaced also names the run's script, so
    // Run::isCursiveScript agrees with the glyphs CoreText produced
    runState.fScript = ctx->fBuilder.scriptAt(relUtf8Range.start);

    build_run_clusters(
            item.run, ctx->fBuilder.indexMap(), ctx->fRegionRange.start, fixup, &runState);
    if (isFallback) {
        measure_fallback_glyphs(ctx->fText, fParagraph->fUnicode.get(), &runState);
    } else {
        measure_shaped_glyphs(item.run, ctFont, ctx->fText, fixup, &runState);
    }
    this->appendRegionRun(&runState);
    return newlineGlyph ? 2 : 1;
}

void CoreTextLineShaper::appendRegionRun(CoreTextRunState* runState) {
    SkASSERT(runState);
    SkFontMetrics fontMetrics;
    runState->fFont.getMetrics(&fontMetrics);
    const SkScalar runHeight =
            runState->fStyle->getHeightOverride() ? runState->fStyle->getHeight() : 0.0f;

    const SkShaper::RunHandler::RunInfo runInfo = {
            runState->fFont,
            runState->fBidiLevel,
            runState->fScript,
            runState->fLanguage,
            SkPoint::Make(runState->fRunAdvanceX,
                          fontMetrics.fDescent - fontMetrics.fAscent + fontMetrics.fLeading),
            SkToSizeT(runState->fGlyphCount),
            SkShaper::RunHandler::Range(0, runState->fUtf8Range.width())};

    auto& run = fParagraph->fRuns.emplace_back(fParagraph,
                                               runInfo,
                                               runState->fUtf8Range.start,
                                               runHeight,
                                               runState->fStyle->getHalfLeading(),
                                               runState->fStyle->getBaselineShift(),
                                               fParagraph->fRuns.size(),
                                               fAdvanceX);
    // Also writes the N+1 sentinel (run end position and cluster index) that line breaking and
    // caret hit-testing interpolate against
    run.allocateGlyphData(SkToSizeT(runState->fGlyphCount), /*hasOffsets=*/false);

    if (runState->fIsFallback) {
        this->populateFallbackRun(&run, *runState, fAdvanceX);
    } else {
        this->populateShapedRun(&run, runState, fAdvanceX);
    }

    // CoreText runs keep the end sentinel on the baseline, where allocateGlyphData puts it at
    // fOffset + fAdvance. commit() may have moved the glyph data, so read it again
    run.fGlyphData->positions()[runState->fGlyphCount] =
            SkPoint::Make(fAdvanceX + runState->fRunAdvanceX, 0);

    fParagraph->fFontSwitches.emplace_back(runState->fUtf8Range.start, run.font());
    fAdvanceX += runState->fRunAdvanceX;
}

void CoreTextLineShaper::populateFallbackRun(Run* run,
                                             const CoreTextRunState& runState,
                                             SkScalar runStartX) {
    SkASSERT(run);
    SkPoint* positions = run->fGlyphData->positions();
    uint32_t* clusterIndexes = run->fGlyphData->clusterIndexes();
    SkGlyphID* glyphs = run->fGlyphData->glyphs();

    // CoreText renders missing glyphs through LastResort; remap to glyph 0 (.notdef) to honor
    // SkParagraph unresolved contract. Invisible clusters take the primary font's space glyph, as
    // in populateShapedRun, so they paint no box over the next run
    const SkGlyphID invisibleGlyph = runState.fFont.unicharToGlyph(' ');
    SkScalar accumulatedX = 0;
    for (int k = 0; k < runState.fGlyphCount; ++k) {
        glyphs[k] =
                runState.fGlyphs[k] == kCGFontIndexInvalid ? invisibleGlyph : runState.fGlyphs[k];
        positions[k] = SkPoint::Make(runStartX + accumulatedX, 0);
        accumulatedX += runState.fAdvances[k];
        clusterIndexes[k] = runState.fClusters[k];
        if (runState.fGlyphs[k] == 0) {
            this->recordUnresolvedGlyph(glyph_text_range(runState, k));
        }
    }
}

void CoreTextLineShaper::populateShapedRun(Run* run,
                                           CoreTextRunState* runState,
                                           SkScalar runStartX) {
    SkASSERT(run);
    SkASSERT(runState);
    SkPoint* positions = run->fGlyphData->positions();
    uint32_t* clusterIndexes = run->fGlyphData->clusterIndexes();
    SkGlyphID* glyphs = run->fGlyphData->glyphs();

    if (runState->fShapingBuffer->fOffsets.size() < runState->fGlyphCount) {
        runState->fShapingBuffer->fOffsets.resize_back(runState->fGlyphCount);
    }
    SkPoint* offsets = runState->fShapingBuffer->fOffsets.data();
    // CTRunGetPositions folds mark draw offsets in and lacks the non-subpixel and newline
    // adjustments, so the pen walks fAdvances instead
    SkScalar penX = 0;
    // A run of only characters CoreText drops (C0 controls) keeps one zero-advance
    // kCGFontIndexInvalid placeholder, no glyph id of the font; space draws nothing
    // KEEPER-DEBT: a font without a space glyph gets .notdef at zero advance; remove once Run can
    // hold a glyph-less cluster
    const SkGlyphID invisibleGlyph = runState->fFont.unicharToGlyph(' ');
    for (int k = 0; k < runState->fGlyphCount; ++k) {
        glyphs[k] =
                runState->fGlyphs[k] == kCGFontIndexInvalid ? invisibleGlyph : runState->fGlyphs[k];
        // Cluster widths and hit-testing read the positions, and extendedVisit and getPath hand
        // the positions out without the offsets, so the whole origin goes into the offset as
        // SkShaper's HarfBuzz runs put x_offset and y_offset there
        positions[k] = SkPoint::Make(runStartX + penX, 0);
        penX += runState->fAdvances[k];
        offsets[k] = glyph_draw_offset(runState->fOrigins[k],
                                       runState->fZeroSize,
                                       runState->fSplitKerning ? runState->fSplitOffsets[k] : 0);
        clusterIndexes[k] = runState->fClusters[k];
        if (glyphs[k] == 0) {
            this->recordUnresolvedGlyph(glyph_text_range(*runState, k));
        }
    }
    run->commit(*runState->fShapingBuffer);
}

// Records the code points of glyphRange as unresolved, skipping controls (as HarfBuzz
// OneLineShaper::iterateThroughFontStyles does) and Default_Ignorable code points, which HarfBuzz
// hides inside a neighbouring cluster; combining marks count, so a mark no font has is reported.
// Adds one unresolved glyph when any code point was recorded
void CoreTextLineShaper::recordUnresolvedGlyph(TextRange glyphRange) {
    const char* textStart = fParagraph->text().data();
    const char* p = textStart + glyphRange.start;
    const char* pEnd = textStart + glyphRange.end;
    SkUnicode* unicode = fParagraph->fUnicode.get();
    bool hasNonControl = false;
    while (p < pEnd) {
        const size_t bytePos = SkToSizeT(p - textStart);
        const SkUnichar u = SkUTF::NextUTF8(&p, pEnd);
        if (u < 0) {
            break;
        }
        if (!fParagraph->codeUnitHasProperty(bytePos, SkUnicode::CodeUnitFlags::kControl) &&
            !unicode->isDefaultIgnorable(u)) {
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

namespace {

// Returns the paragraph-global UTF-8 end index of glyph pos's cluster in run, stepping past any
// adjacent glyphs that share pos's cluster index or belong to combining marks skipped by an
// IgnoreMarks ligature
TextIndex cluster_end_utf8(const ParagraphImpl& owner, const Run& run, size_t pos) {
    SkASSERT(pos <= run.size());
    const TextIndex clusterStart8 = run.globalClusterIndex(pos);
    if (run.leftToRight()) {
        // LTR callers only pass glyph indices inside [sliceStart, sliceEnd) with
        // sliceEnd <= run.size()
        SkASSERT(pos < run.size());
        size_t endGlyph = pos + 1;
        while (endGlyph < run.size() &&
               (run.globalClusterIndex(endGlyph) <= clusterStart8 ||
                !owner.codeUnitHasProperty(run.globalClusterIndex(endGlyph),
                                           SkUnicode::CodeUnitFlags::kGraphemeStart))) {
            ++endGlyph;
        }
        return run.globalClusterIndex(endGlyph);
    }
    // RTL glyphs sit in reverse logical order, so the next logical cluster lies at a lower glyph
    // index and glyph 0 ends at run.textRange().end
    size_t endGlyph = pos;
    while (endGlyph > 0 && (run.globalClusterIndex(endGlyph - 1) <= clusterStart8 ||
                            !owner.codeUnitHasProperty(run.globalClusterIndex(endGlyph - 1),
                                                       SkUnicode::CodeUnitFlags::kGraphemeStart))) {
        --endGlyph;
    }
    return endGlyph == 0 ? run.textRange().end : run.globalClusterIndex(endGlyph - 1);
}

struct HitCluster {
    TextIndex start8;
    TextIndex end8;
    size_t startGlyph;
    size_t endGlyph;
};

// Resolves glyph found inside [sliceStart, sliceEnd) of run to its enclosing cluster's UTF-8
// [start8, end8) range and visual [startGlyph, endGlyph) span, anchoring detached combining
// marks onto their base or IgnoreMarks ligature glyph
HitCluster resolve_hit_cluster(const ParagraphImpl& owner,
                               const Run& run,
                               size_t sliceStart,
                               size_t sliceEnd,
                               size_t found) {
    SkASSERT(sliceStart <= found && found < sliceEnd && sliceEnd <= run.size());
    // A detached combining-mark glyph sits right of its base in LTR and left of its base or
    // IgnoreMarks ligature in RTL; step onto the owning grapheme start when the slice has one
    while (!owner.codeUnitHasProperty(run.globalClusterIndex(found),
                                      SkUnicode::CodeUnitFlags::kGraphemeStart)) {
        if (run.leftToRight()) {
            // Stop at the slice edge when a style or line break split a mark from its base
            if (found == sliceStart) {
                break;
            }
            --found;
        } else {
            if (found + 1 >= sliceEnd) {
                break;
            }
            ++found;
        }
    }

    const TextIndex start8 = run.globalClusterIndex(found);
    const TextIndex end8 = cluster_end_utf8(owner, run, found);
    size_t startGlyph = found;
    size_t endGlyph = found + 1;
    // Include co-clustered glyphs and detached combining marks so the visual span covers the full
    // cluster width in both LTR (marks to the right) and RTL (marks to the left)
    if (run.leftToRight()) {
        while (startGlyph > sliceStart && run.globalClusterIndex(startGlyph - 1) == start8) {
            --startGlyph;
        }
        // Absorb trailing glyphs of the same cluster or marks skipped by an IgnoreMarks ligature
        while (endGlyph < sliceEnd &&
               (run.globalClusterIndex(endGlyph) <= start8 ||
                !owner.codeUnitHasProperty(run.globalClusterIndex(endGlyph),
                                           SkUnicode::CodeUnitFlags::kGraphemeStart))) {
            ++endGlyph;
        }
    } else {
        // In RTL, detached marks and later ligature components sit visually left (lower index)
        while (startGlyph > sliceStart &&
               (run.globalClusterIndex(startGlyph - 1) <= start8 ||
                !owner.codeUnitHasProperty(run.globalClusterIndex(startGlyph - 1),
                                           SkUnicode::CodeUnitFlags::kGraphemeStart))) {
            --startGlyph;
        }
        while (endGlyph < sliceEnd && run.globalClusterIndex(endGlyph) == start8) {
            ++endGlyph;
        }
    }
    return {start8, end8, startGlyph, endGlyph};
}

// Filters graphemes in place to keep only visible graphemes (not solely Default_Ignorable)
// unless every grapheme in the cluster is ignorable; returns true when graphemes shrank
bool drop_ignorable_graphemes(const ParagraphImpl& owner,
                              skia_private::TArray<TextIndex>* graphemes) {
    // ParagraphImpl constructors assert fUnicode and layout returns before shaping without it
    SkUnicode* unicode = owner.getUnicode().get();
    SkASSERT(unicode);
    if (graphemes->size() <= 1) {
        return false;
    }
    const SkSpan<const char> text = owner.text();
    int kept = 0;
    for (int i = 0; i < graphemes->size(); ++i) {
        const TextIndex start = (*graphemes)[i];
        SkASSERT(start < text.size());
        const TextIndex end = owner.findNextGraphemeBoundary(start + 1);
        const char* ptr = text.data() + start;
        const char* endPtr = text.data() + end;
        bool ignorable = ptr < endPtr;
        // Any non-ignorable scalar (base letter or visible mark) makes the whole grapheme visible
        while (ptr < endPtr) {
            const SkUnichar u = SkUTF::NextUTF8(&ptr, endPtr);
            if (u < 0 || !unicode->isDefaultIgnorable(u)) {
                ignorable = false;
                break;
            }
        }
        if (!ignorable) {
            (*graphemes)[kept++] = start;
        }
    }
    // Keep an all-ignorable cluster intact so a standalone ignorable glyph still hit-tests
    if (kept == 0 || kept == graphemes->size()) {
        return false;
    }
    graphemes->pop_back_n(graphemes->size() - kept);
    return true;
}

// Finds the glyph in [context.pos, context.pos + context.size) whose left edge is closest at or to
// the left of dx, matching TextLine's 0.01 rounding hack and RTL exact-edge tie-break
size_t find_hit_glyph(const ParagraphImpl& owner,
                      const TextLine::ClipContext& context,
                      SkScalar offsetX,
                      SkScalar dx) {
    size_t found = context.pos;
    for (size_t index = context.pos; index < context.pos + context.size; ++index) {
        SkScalar end = context.run->positionX(index) + context.fTextShift + offsetX;
        if (owner.getApplyRoundingHack()) {
            end = SkScalarRoundToScalar(end * 100.0f) / 100.0f;
        }
        if (end > dx) {
            break;
        }
        // At an exact RTL glyph boundary, pick the glyph starting at dx rather than scanning right
        if (end == dx && !context.run->leftToRight()) {
            return index;
        }
        found = index;
    }
    return found;
}

}  // namespace

bool CoreTextLineShaper::HitTestRunSlice(const ParagraphImpl& owner,
                                         const TextLine::ClipContext& context,
                                         SkScalar offsetX,
                                         SkScalar dx,
                                         PositionWithAffinity* result) {
    SkASSERT(context.run);
    SkASSERT(result);
    const bool ltr = context.run->leftToRight();
    if (dx <= context.clip.fLeft) {
        if (ltr) {
            // A slice can start on a detached combining mark's glyph cluster
            const TextIndex start8 = context.run->globalClusterIndex(context.pos);
            const size_t utf16 = owner.getUTF16Index(owner.findPreviousGraphemeBoundary(start8));
            *result = {SkToS32(utf16), Affinity::kDownstream};
            return false;
        }
        // The leftmost RTL glyph is the slice's logical end and may be a composed grapheme or
        // multi-grapheme ligature, so land at its cluster's trailing grapheme boundary
        const TextIndex end8 = cluster_end_utf8(owner, *context.run, context.pos);
        const size_t utf16 = owner.getUTF16Index(owner.findNextGraphemeBoundary(end8));
        *result = {SkToS32(utf16), Affinity::kUpstream};
        // Visual runs go left to right, so every later slice starts right of dx and would
        // overwrite this caret with an earlier logical position, even when this slice wraps
        return false;
    }

    // Hard-break-only slices have size == 0, and NaN dx skips both <= and >= edge checks
    if (dx >= context.clip.fRight || context.size == 0) {
        // Snap to the outer grapheme edge in case a slice edge splits a mark cluster
        const TextIndex edge8 = context.run->globalClusterIndex(context.pos + context.size);
        const TextIndex snapped8 = ltr ? owner.findNextGraphemeBoundary(edge8)
                                       : owner.findPreviousGraphemeBoundary(edge8);
        *result = {SkToS32(owner.getUTF16Index(snapped8)),
                   ltr ? Affinity::kUpstream : Affinity::kDownstream};
        return true;
    }

    const size_t found = find_hit_glyph(owner, context, offsetX, dx);
    const HitCluster hit = resolve_hit_cluster(
            owner, *context.run, context.pos, context.pos + context.size, found);
    const SkScalar clusterLeft =
            context.run->positionX(hit.startGlyph) + context.fTextShift + offsetX;
    const SkScalar clusterWidth =
            context.run->positionX(hit.endGlyph) - context.run->positionX(hit.startGlyph);
    auto graphemes = owner.countSurroundingGraphemes({hit.start8, hit.end8});
    // CoreText omits Default_Ignorable code points without emitting glyphs; drop them so they take
    // no share of the preceding visible glyph's width
    const bool droppedIgnorable = drop_ignorable_graphemes(owner, &graphemes);

    if (graphemes.size() > 1 || droppedIgnorable) {
        const SkScalar graphemeWidth = clusterWidth / graphemes.size();
        int visualIndex = SkScalarNearlyZero(graphemeWidth)
                                  ? 0
                                  : SkScalarFloorToInt((dx - clusterLeft) / graphemeWidth);
        visualIndex = std::clamp(visualIndex, 0, SkToInt(graphemes.size()) - 1);
        const SkScalar graphemeCenter =
                clusterLeft + visualIndex * graphemeWidth + graphemeWidth / 2;
        // countSurroundingGraphemes lists graphemes in logical order, which runs right-to-left
        // across an RTL ligature
        const size_t logicalIndex =
                ltr ? SkToSizeT(visualIndex) : graphemes.size() - 1 - SkToSizeT(visualIndex);
        const TextIndex grapheme8 = graphemes[logicalIndex];
        if ((dx < graphemeCenter) == ltr) {
            const TextIndex start8 = owner.findPreviousGraphemeBoundary(grapheme8);
            *result = {SkToS32(owner.getUTF16Index(start8)), Affinity::kDownstream};
        } else {
            const TextIndex end8 = owner.findNextGraphemeBoundary(grapheme8 + 1);
            *result = {SkToS32(owner.getUTF16Index(end8)), Affinity::kUpstream};
        }
        return false;
    }

    const SkScalar center = clusterLeft + clusterWidth / 2;
    if ((dx < center) == ltr) {
        const TextIndex start8 = owner.findPreviousGraphemeBoundary(hit.start8);
        *result = {SkToS32(owner.getUTF16Index(start8)), Affinity::kDownstream};
    } else {
        const TextIndex end8 = owner.findNextGraphemeBoundary(hit.end8);
        *result = {SkToS32(owner.getUTF16Index(end8)), Affinity::kUpstream};
    }
    return false;
}

}  // namespace textlayout
}  // namespace skia
