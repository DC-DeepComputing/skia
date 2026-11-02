// Copyright 2026 Google LLC
#include "modules/skparagraph/src/CoreTextLineShaper.h"

#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)

#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPoint.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkTypeface_mac.h"
#include "include/private/SkFloatingPoint.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/src/CoreTextAttrStringBuilder.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkNoDestructor.h"
#include "src/core/SkTHash.h"
#include "src/core/SkUTF.h"
#include "src/utils/mac/SkCGBase.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <utility>

namespace skia {
namespace textlayout {

namespace {

// HarfBuzz carries advances as 16.16 fixed (skhb_position in SkShaper_harfbuzz.cpp) and scales
// them back through double; rounding the same way makes float sums bit-identical to HarfBuzz,
// which Flutter widths (compared with ==) and goldens rely on
static SkScalar round_like_harfbuzz(SkScalar value) {
    return static_cast<SkScalar>(SkScalarRoundToInt(value * (1 << 16)) * 1.52587890625e-5);
}

// HarfBuzz scales v font units of kerning to 16.16 as (v * mult + 32768) >> 16, with mult =
// em_mult from harfbuzz_advances; CoreText gives v * size / upem. This only matches one
// whole-unit adjustment: a pair kerned by several lookups, trak or variation deltas is rounded as
// is and can stay one 16.16 unit off
static int64_t harfbuzz_kerning16(double kerning,
                                  double unitsPerPixel,
                                  int64_t mult,
                                  // Set for whole font units, the only kerning HarfBuzz splits
                                  bool* fontUnits) {
    const double units = kerning * unitsPerPixel;
    const double wholeUnits = std::round(units);
    // Kerning values are FWords, so any int16 value counts
    *fontUnits = std::abs(units - wholeUnits) <= 1e-3 && wholeUnits >= INT16_MIN &&
                 wholeUnits <= INT16_MAX;
    if (*fontUnits) {
        return (static_cast<int64_t>(wholeUnits) * mult + 32768) >> 16;
    }
    return sk_double_saturate2int(std::floor(kerning * 65536 + 0.5));
}

// Writes the advances HarfBuzz gives the run's glyphs from their nominal (font) advances.
// HarfBuzz rounds the nominal advance to 16.16, or to whole pixels for a non-subpixel font
// (skhb_glyph_h_advances), then adds 16.16 GPOS kerning; CoreText base advances fold kerning in,
// so the two parts are rounded apart
static void harfbuzz_advances(CTFontRef ctFont,
                              bool subpixel,
                              const CGSize* nominal,
                              const CGSize* advances,
                              SkScalar* out,
                              // When set, pair kerning is split as in HarfBuzz and each glyph
                              // gets the 16.16 offset that keeps its drawn x where CoreText puts it
                              int64_t* splitOffsets16,
                              CFIndex count) {
    const SkScalar size = SkScalarFromCGFloat(CTFontGetSize(ctFont));
    const unsigned upem = CTFontGetUnitsPerEm(ctFont);
    SkASSERT(size > 0 && upem > 0);
    // The scale OneLineShaper gives hb_font_set_scale (skhb_position of the font size)
    const int64_t scale16 = static_cast<int64_t>(SkScalarRoundToInt(size * (1 << 16))) << 16;
    // hb_font_t::changed divides by upem as a float, so mult is a float quotient truncated,
    // which can be a few units above the exact one and move a kerning value across .5
    const int64_t mult =
            static_cast<int64_t>(static_cast<float>(scale16) / static_cast<float>(upem));
    const double unitsPerPixel = static_cast<double>(upem) / size;
    // The part of the previous glyph's pair kerning HarfBuzz puts on this glyph
    int64_t carry16 = 0;
    for (CFIndex k = 0; k < count; ++k) {
        // ZWJ, soft hyphen and zeroed marks keep a nominal advance, but HarfBuzz zeroes them
        // after rounding, so a zero CoreText advance stays zero
        if (advances[k].width == 0) {
            out[k] = 0;
            // No kerning is carried over them: a split only goes to the glyph right after
            if (splitOffsets16) {
                splitOffsets16[k] = 0;
            }
            continue;
        }
        SkScalar nominalWidth = SkScalarFromCGFloat(nominal[k].width);
        if (!subpixel) {
            nominalWidth = static_cast<SkScalar>(SkScalarRoundToInt(nominalWidth));
        }
        // SkScalarRoundToInt saturates as skhb_position does, so an advance of 32768 px or more
        // comes out as INT_MAX in 16.16 on both paths; the sum is clamped to the same range
        const int64_t nominal16 = SkScalarRoundToInt(nominalWidth * (1 << 16));
        bool fontUnits = false;
        int64_t kerning16 = harfbuzz_kerning16(
                advances[k].width - nominal[k].width, unitsPerPixel, mult, &fontUnits);
        int64_t advance16 = nominal16 + carry16;
        if (splitOffsets16) {
            splitOffsets16[k] = carry16;
        }
        carry16 = 0;
        // hb_kern_machine_t (hb-kern.hh) keeps kerning >> 1 on the glyph and puts the rest on
        // the next glyph, as advance and offset. Only whole font units are pair kerning (trak
        // and variations are not). CoreText kerns no pair across a zero-advance glyph, so
        // kerning before one, or on the run's last glyph, stays whole
        if (splitOffsets16 && fontUnits && k + 1 < count && advances[k + 1].width != 0) {
            carry16 = kerning16 - (kerning16 >> 1);
            kerning16 >>= 1;
        }
        advance16 = std::clamp<int64_t>(advance16 + kerning16, INT32_MIN, INT32_MAX);
        out[k] = static_cast<SkScalar>(advance16 * 1.52587890625e-5);
    }
}

static uint32_t read_u16(const uint8_t* p) { return (static_cast<uint32_t>(p[0]) << 8) | p[1]; }

static uint32_t read_u32(const uint8_t* p) { return (read_u16(p) << 16) | read_u16(p + 2); }

// True when the subtables of the kern or kerx table HarfBuzz applies to horizontal text
// (AAT::KerxTable::apply) kern each pair with one plain pair subtable run by hb_kern_machine_t.
// CoreText reports only the sum of all subtables per glyph, which cannot be split per subtable
static bool kerns_each_pair_once(CFDataRef table, bool kerx) {
    const uint8_t* data = CFDataGetBytePtr(table);
    const size_t size = SkToSizeT(CFDataGetLength(table));
    if (size < 8) {
        return false;
    }
    // kern version 0 (OpenType) has 16-bit counts and lengths, version 1 (Apple) 32-bit ones;
    // kerx keeps the Apple layout with 32-bit coverage and a tuple count
    const bool otKern = !kerx && read_u16(data) == 0;
    if (!kerx && !otKern && read_u16(data) != 1) {
        return false;
    }
    const uint32_t tableCount = otKern ? read_u16(data + 2) : read_u32(data + 4);
    const size_t headerSize = otKern ? 6 : kerx ? 12 : 8;
    size_t offset = otKern ? 4 : 8;
    int applicable = 0;
    bool pairOnly = true;
    // Format 0 pair lists of the applicable subtables, as (first pair, pair count)
    skia_private::STArray<4, std::pair<const uint8_t*, uint32_t>> pairLists;
    for (uint32_t i = 0; i < tableCount; ++i) {
        if (offset + headerSize > size) {
            return false;
        }
        const uint8_t* header = data + offset;
        uint32_t format = 0;
        bool horizontal = false;
        bool pair = false;
        if (otKern) {
            const uint32_t coverage = header[5];
            format = header[4];
            horizontal = (coverage & 0x01) != 0;
            pair = (format == 0 || format == 2 || format == 3) && (coverage & 0x04) == 0;
        } else if (kerx) {
            const uint32_t coverage = read_u32(header + 4);
            format = coverage & 0xFF;
            horizontal = (coverage & 0x80000000) == 0;
            pair = (format == 0 || format == 2 || format == 6) &&
                   (coverage & (0x40000000 | 0x10000000)) == 0;
        } else {
            const uint32_t coverage = header[4];
            format = header[5];
            // HarfBuzz skips variation subtables of the Apple kern table
            horizontal = (coverage & (0x80 | 0x20)) == 0;
            pair = (format == 0 || format == 2 || format == 3) && (coverage & 0x40) == 0;
        }
        if (horizontal) {
            ++applicable;
            pairOnly = pairOnly && pair;
            if (pair && format == 0) {
                // nPairs, then three binary search fields of the same width, then the pairs
                const size_t fieldSize = kerx ? 4 : 2;
                const size_t pairsAt = offset + headerSize + 4 * fieldSize;
                if (pairsAt > size) {
                    return false;
                }
                const uint8_t* fields = header + headerSize;
                const uint32_t pairCount = kerx ? read_u32(fields) : read_u16(fields);
                if (pairCount > (size - pairsAt) / 6) {
                    return false;
                }
                pairLists.push_back({data + pairsAt, pairCount});
            }
        }
        // HarfBuzz lets the last subtable run past its length (large format 0 subtables), so
        // only lengths that lead to another subtable are checked
        if (i + 1 < tableCount) {
            const size_t length = otKern ? read_u16(header + 2) : read_u32(header);
            if (length < headerSize || length > size - offset) {
                return false;
            }
            offset += length;
        }
    }
    if (applicable == 1) {
        return pairOnly;
    }
    // HarfBuzz runs the machine once per subtable and splits each value apart, so the split of
    // CoreText's sum is exact only while no pair sits in two subtables. Pair lists are sorted
    // by (left, right) for HarfBuzz's binary search, so a merge walk finds a shared pair
    if (applicable == 0 || pairLists.size() != applicable) {
        return false;
    }
    for (const auto& [pairs, count] : pairLists) {
        for (uint32_t k = 1; k < count; ++k) {
            if (read_u32(pairs + 6 * (k - 1)) >= read_u32(pairs + 6 * k)) {
                return false;
            }
        }
    }
    for (int a = 0; a < pairLists.size(); ++a) {
        for (int b = a + 1; b < pairLists.size(); ++b) {
            uint32_t i = 0;
            uint32_t j = 0;
            while (i < pairLists[a].second && j < pairLists[b].second) {
                const uint32_t left = read_u32(pairLists[a].first + 6 * i);
                const uint32_t right = read_u32(pairLists[b].first + 6 * j);
                if (left == right) {
                    return false;
                }
                if (left < right) {
                    ++i;
                } else {
                    ++j;
                }
            }
        }
    }
    return true;
}

// True when HarfBuzz positions this font's default-shaper text (Latin, Greek, Cyrillic) with
// one pair subtable of kerx or legacy kern, which hb_kern_machine_t splits per pair. Follows
// hb_ot_shape_planner_t::compile (hb-ot-shape.cc): kerx wins unless GSUB and GPOS are both
// there, and legacy kern applies only when GPOS has no kern feature for the chosen script
static bool splits_pair_kerning(CTFontRef ctFont) {
    auto copyTable = [ctFont](char a, char b, char c, char d) {
        return SkUniqueCFRef<CFDataRef>(
                CTFontCopyTable(ctFont, SkSetFourByteTag(a, b, c, d), kCTFontTableOptionNoOptions));
    };
    // HarfBuzz reads a table as absent when its version is zero or it fails to sanitize
    auto version = [](const SkUniqueCFRef<CFDataRef>& table, bool wide) -> uint32_t {
        if (!table || CFDataGetLength(table.get()) < 4) {
            return 0;
        }
        const uint8_t* data = CFDataGetBytePtr(table.get());
        return wide ? read_u32(data) : read_u16(data);
    };
    SkUniqueCFRef<CFDataRef> kerx = copyTable('k', 'e', 'r', 'x');
    SkUniqueCFRef<CFDataRef> gpos = copyTable('G', 'P', 'O', 'S');
    const bool hasMorx = version(copyTable('m', 'o', 'r', 'x'), false) != 0 ||
                         version(copyTable('m', 'o', 'r', 't'), false) != 0;
    const bool hasKerx = version(kerx, false) != 0;
    // GSUB and GPOS sanitize only with major version 1
    const bool hasGsub = !hasMorx && version(copyTable('G', 'S', 'U', 'B'), true) >> 16 == 1;
    const bool hasGpos = version(gpos, true) >> 16 == 1;
    const bool applyKerx = hasKerx && !(hasGsub && hasGpos);
    if (!applyKerx && hasGpos) {
        // Which LangSys HarfBuzz picks depends on script and language, so a GPOS kern feature
        // anywhere in the font leaves the choice unknown
        const uint8_t* data = CFDataGetBytePtr(gpos.get());
        const size_t size = SkToSizeT(CFDataGetLength(gpos.get()));
        if (size < 10) {
            return false;
        }
        const size_t featureList = read_u16(data + 6);
        if (featureList + 2 > size) {
            return false;
        }
        // A zero offset is HarfBuzz's empty Null table
        const uint32_t featureCount = featureList ? read_u16(data + featureList) : 0;
        if (featureList + 2 + 6 * static_cast<size_t>(featureCount) > size) {
            return false;
        }
        for (uint32_t i = 0; i < featureCount; ++i) {
            if (read_u32(data + featureList + 2 + 6 * i) == SkSetFourByteTag('k', 'e', 'r', 'n')) {
                return false;
            }
        }
    }
    // Without GPOS kerning HarfBuzz takes kerx, then legacy kern
    if (hasKerx) {
        return kerns_each_pair_once(kerx.get(), true);
    }
    SkUniqueCFRef<CFDataRef> kern = copyTable('k', 'e', 'r', 'n');
    return version(kern, true) != 0 && kerns_each_pair_once(kern.get(), false);
}

// CoreText shares one CGFont across all sizes and attribute copies of a face, so a size change
// still hits; a font with several kern subtables takes about 25 us to check. Each entry holds
// its CGFont, so no other font can take that address while it lives. A hit takes the lock but
// does not allocate
static bool splits_pair_kerning_cached(CTFontRef ctFont) {
    struct Entry {
        SkUniqueCFRef<CGFontRef> fFont;
        bool fSplits;
    };
    struct Cache {
        SkMutex fMutex;
        SkLRUCache<CGFontRef, Entry> fEntries SK_GUARDED_BY(fMutex){64};
    };
    static SkNoDestructor<Cache> cache;
    SkUniqueCFRef<CGFontRef> cgFont(CTFontCopyGraphicsFont(ctFont, nullptr));
    SkASSERT(cgFont);
    {
        SkAutoMutexExclusive lock(cache->fMutex);
        if (const Entry* entry = cache->fEntries.find(cgFont.get())) {
            return entry->fSplits;
        }
    }
    // Parsed outside the lock so other paragraphs do not wait on it
    const bool splits = splits_pair_kerning(ctFont);
    CGFontRef key = cgFont.get();
    SkAutoMutexExclusive lock(cache->fMutex);
    cache->fEntries.insert_or_update(key, Entry{std::move(cgFont), splits});
    return splits;
}

// HarfBuzz applies legacy kern only with a shaper that has fallback_position. These blocks hold
// only Latin, Greek, Cyrillic, Armenian, Common and Inherited code points, which all get the
// default shaper, and it has fallback_position
static bool has_only_split_kerning_text(const char* utf8, const char* end) {
    while (utf8 < end) {
        const SkUnichar c = SkUTF::NextUTF8(&utf8, end);
        // A tab keeps CoreText's advance to the tab stop (KEEPER-DEBT in shapeTextRegion), which
        // can be whole font units over its hmtx advance and would be split as pair kerning
        if (c == '\t') {
            return false;
        }
        const bool defaultShaper = (c >= 0 && c <= 0x058F) || (c >= 0x1D00 && c <= 0x1FFF) ||
                                   (c >= 0x2000 && c <= 0x2BFF) || (c >= 0x2C60 && c <= 0x2C7F) ||
                                   (c >= 0xA720 && c <= 0xA7FF) || (c >= 0xFB00 && c <= 0xFB06);
        if (!defaultShaper) {
            return false;
        }
    }
    return true;
}

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

// Bound CTFont is passed when available so system UI italic faces whose descriptor omits
// kCTFontSlantTrait are still recognized as italic
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
    // CTTypesetterCreateWithAttributedString disables GSUB/GPOS/morx/kerx above 10,240 UTF-16
    // code units unless unbounded layout is explicitly enabled
    static const SkNoDestructor<SkUniqueCFRef<CFDictionaryRef>> kUnboundedOptions([] {
        const void* keys[] = {kCTTypesetterOptionAllowUnboundedLayout};
        const void* values[] = {kCFBooleanTrue};
        return SkUniqueCFRef<CFDictionaryRef>(CFDictionaryCreate(
                kCFAllocatorDefault,
                keys,
                values,
                1,
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

}  // namespace

bool CoreTextLineShaper::IsZeroWidthCodepoint(SkUnichar utf8) {
    // Default-ignorable format characters and non-spacing combining marks take no advance of their
    // own, so a fallback .notdef box is not widened for them
    static constexpr std::array<std::pair<SkUnichar, SkUnichar>, 13> ranges {{
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
    auto it = std::ranges::lower_bound(
            ranges, utf8, {}, &std::pair<SkUnichar, SkUnichar>::second);
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
    if (text.isEmpty() || !fontCollection) {
        return;
    }
    const TextDirection baseDir = (bidiLevel & 1) ? TextDirection::kRtl : TextDirection::kLtr;
    // Inherit the line's resolved cluster typeface when the ellipsis style does not specify an
    // explicit typeface or font family list
    TextStyle affixStyle = style;
    if (!affixStyle.refTypeface() && affixStyle.getFontFamilies().empty() && clusterTypeface) {
        affixStyle.setTypeface(clusterTypeface);
    }

    std::vector<sk_sp<SkTypeface>> allowedTypefaces =
            CoreTextFontRegistry::Instance().findTypefaces(*fontCollection,
                                                           affixStyle.getFontFamilies(),
                                                           affixStyle.getFontStyle(),
                                                           affixStyle.getFontArguments());
    sk_sp<SkTypeface> primary = affixStyle.refTypeface();
    if (primary) {
        allowedTypefaces.insert(allowedTypefaces.begin(), primary);
    } else if (!allowedTypefaces.empty()) {
        primary = allowedTypefaces.front();
    }
    if (clusterTypeface) {
        allowedTypefaces.push_back(clusterTypeface);
    }

    const bool fallbackDisabled = !fontCollection->fontFallbackEnabled();
    CoreTextAttrStringBuilder builder(
            text.c_str(), text.size(), baseDir, std::move(fontCollection));
    builder.addStyle(TextRange(0, text.size()), affixStyle);
    SkUniqueCFRef<CFAttributedStringRef> attrStr = builder.build();
    if (!attrStr) {
        return;
    }
    SkUniqueCFRef<CTLineRef> line = typeset_line(attrStr.get());
    if (!line) {
        return;
    }
    CFArrayRef runs = CTLineGetGlyphRuns(line.get());
    const CFIndex runCount = runs ? CFArrayGetCount(runs) : 0;

    // ShapeHandler::runBuffer accepts at most one Run, so multi-run affixes coalesce into one
    // buffer with the first resolved CTRun's font and mark glyphs from other fonts unresolved
    CFIndex totalGlyphs = 0;
    CTRunRef primaryRun = nullptr;
    CTFontRef primaryCtFont = nullptr;
    for (CFIndex i = 0; i < runCount; ++i) {
        CTRunRef ctRun = (CTRunRef)CFArrayGetValueAtIndex(runs, i);
        CFIndex gc = CTRunGetGlyphCount(ctRun);
        if (gc <= 0) {
            continue;
        }
        totalGlyphs += gc;
        CTFontRef ctFont =
                (CTFontRef)CFDictionaryGetValue(CTRunGetAttributes(ctRun), kCTFontAttributeName);
        SkASSERT(ctFont);
        bool disallowed =
                isLastResortFont(ctFont) ||
                (fallbackDisabled &&
                 !is_allowed_font(ctFont, allowedTypefaces, affixStyle.getFontFamilies()));
        if (!primaryRun || (!disallowed && isLastResortFont(primaryCtFont))) {
            primaryRun = ctRun;
            primaryCtFont = ctFont;
        }
    }
    if (totalGlyphs == 0 || !primaryRun) {
        return;
    }

    const bool zeroSize = affixStyle.getFontSize() <= 0;
    const bool primaryDisallowed =
            isLastResortFont(primaryCtFont) ||
            (fallbackDisabled &&
             !is_allowed_font(primaryCtFont, allowedTypefaces, affixStyle.getFontFamilies()));
    sk_sp<SkTypeface> runTypeface =
            primaryDisallowed ? primary : SkMakeTypefaceFromCTFont(primaryCtFont);
    SkFont font(std::move(runTypeface),
                zeroSize ? 0 : SkScalarFromCGFloat(CTFontGetSize(primaryCtFont)));
    // Only pass primaryCtFont when it was not rejected as LastResort or disallowed fallback
    apply_font_style_props(font,
                           primaryDisallowed ? nullptr : primaryCtFont,
                           affixStyle,
                           fakeMissingFontStyles);

    const bool ctIsRtl = (CTRunGetStatus(primaryRun) & kCTRunStatusRightToLeft) != 0;
    uint8_t level = bidiLevel;
    if ((level % 2 != 0) != ctIsRtl) {
        level = ctIsRtl ? (level | 1) : (level > 0 ? level - 1 : 0);
    }

    // Fetched and rounded once per run; the pen walk below reads them again
    skia_private::AutoSTArray<16, CGGlyph> glyphs(totalGlyphs);
    skia_private::AutoSTArray<16, CGSize> advances(totalGlyphs);
    skia_private::AutoSTArray<16, CGPoint> origins(totalGlyphs);
    skia_private::AutoSTArray<16, SkScalar> hbAdvances(totalGlyphs);
    // The part of a kerning pair HarfBuzz moves into each glyph's offset, as in shapeTextRegion
    skia_private::AutoSTArray<16, int64_t> splitOffsets16(totalGlyphs);
    std::fill_n(splitOffsets16.data(), totalGlyphs, 0);
    const bool splittableText =
            has_only_split_kerning_text(text.c_str(), text.c_str() + text.size());
    SkScalar totalAdvance = 0;
    CFIndex runStart = 0;
    for (CFIndex i = 0; i < runCount; ++i) {
        CTRunRef ctRun = (CTRunRef)CFArrayGetValueAtIndex(runs, i);
        const CFIndex gc = CTRunGetGlyphCount(ctRun);
        if (gc <= 0) {
            continue;
        }
        CTRunGetGlyphs(ctRun, CFRangeMake(0, gc), glyphs.data() + runStart);
        // Base advances keep a mark's drawn offset off the glyph before it, as in shapeTextRegion
        CTRunGetBaseAdvancesAndOrigins(
                ctRun, CFRangeMake(0, gc), advances.data() + runStart, origins.data() + runStart);
        // A zero-size affix keeps zero advances, as in shapeTextRegion
        if (!zeroSize) {
            skia_private::AutoSTArray<16, CGSize> nominal(gc);
            const CTFontRef ctFont = (CTFontRef)CFDictionaryGetValue(CTRunGetAttributes(ctRun),
                                                                     kCTFontAttributeName);
            SkASSERT(ctFont);
            CTFontGetAdvancesForGlyphs(ctFont,
                                       kCTFontOrientationHorizontal,
                                       glyphs.data() + runStart,
                                       nominal.data(),
                                       gc);
            // The gate of shapeTextRegion, with the whole affix as the run text
            const bool splitKerning = splittableText &&
                                      (CTRunGetStatus(ctRun) & kCTRunStatusRightToLeft) == 0 &&
                                      splits_pair_kerning_cached(ctFont);
            // Rounded as in shapeTextRegion, so an ellipsis is exactly as wide as on HarfBuzz
            harfbuzz_advances(ctFont,
                              font.isSubpixel(),
                              nominal.data(),
                              advances.data() + runStart,
                              hbAdvances.data() + runStart,
                              splitKerning ? splitOffsets16.data() + runStart : nullptr,
                              gc);
            for (CFIndex k = 0; k < gc; ++k) {
                totalAdvance += hbAdvances[runStart + k];
            }
        }
        runStart += gc;
    }

    const SkString language = affixStyle.getLocale();
    const SkShaper::RunHandler::RunInfo info = {
            font,
            level,
            builder.scriptAt(0),
            language.c_str(),
            SkVector::Make(totalAdvance, 0),
            static_cast<size_t>(totalGlyphs),
            SkShaper::RunHandler::Range(0, text.size())};
    handler->beginLine();
    handler->runInfo(info);
    handler->commitRunInfo();
    SkShaper::RunHandler::Buffer buffer = handler->runBuffer(info);

    static_assert(sizeof(CGGlyph) == sizeof(SkGlyphID));
    CFIndex outIdx = 0;
    // Same pen as in shapeTextRegion: rounded base advances summed in float, with each origin
    // apart in the offset when the handler takes offsets, as SkShaper's HarfBuzz runs do
    SkScalar penX = 0;
    for (CFIndex i = 0; i < runCount; ++i) {
        CTRunRef ctRun = (CTRunRef)CFArrayGetValueAtIndex(runs, i);
        const CFIndex gc = CTRunGetGlyphCount(ctRun);
        if (gc <= 0) {
            continue;
        }
        CTFontRef ctFont =
                (CTFontRef)CFDictionaryGetValue(CTRunGetAttributes(ctRun), kCTFontAttributeName);
        const bool unresolvedRun =
                primaryDisallowed || isLastResortFont(ctFont) ||
                (fallbackDisabled &&
                 !is_allowed_font(ctFont, allowedTypefaces, affixStyle.getFontFamilies())) ||
                !CFEqual(ctFont, primaryCtFont);

        skia_private::AutoSTArray<16, CFIndex> indices(gc);
        CTRunGetStringIndices(ctRun, CFRangeMake(0, gc), indices.data());
        for (CFIndex k = 0; k < gc; ++k) {
            const CFIndex g = outIdx + k;
            buffer.glyphs[g] = unresolvedRun ? 0 : glyphs[g];
            // CoreText is y-up, Skia y-down
            const CGPoint origin = zeroSize ? CGPointZero : origins[g];
            // Same 16.16 rounding as round_like_harfbuzz, plus the split share of a kerning pair
            const int64_t offsetX16 =
                    SkScalarRoundToInt(SkScalarFromCGFloat(origin.x) * (1 << 16)) +
                    splitOffsets16[g];
            const SkPoint offset =
                    SkPoint::Make(static_cast<SkScalar>(offsetX16 * 1.52587890625e-5),
                                  round_like_harfbuzz(-SkScalarFromCGFloat(origin.y)));
            if (buffer.offsets) {
                buffer.positions[g] = buffer.point + SkPoint::Make(penX, 0);
                buffer.offsets[g] = offset;
            } else {
                buffer.positions[g] = buffer.point + SkPoint::Make(penX, 0) + offset;
            }
            if (!zeroSize) {
                penX += hbAdvances[g];
            }
            if (buffer.clusters) {
                buffer.clusters[g] = SkToU32(builder.indexMap().toUtf8Index(indices[k]));
            }
        }
        outIdx += gc;
    }
    if (buffer.clusters) {
        normalize_cluster_indices(buffer.clusters, totalGlyphs, (level % 2) == 0);
    }
    handler->commitRunBuffer(info);
    handler->commitLine();
}

CoreTextLineShaper::CoreTextLineShaper(ParagraphImpl* paragraph)
    : fParagraph(paragraph)
    , fUnresolvedGlyphs(0) {
}

bool CoreTextLineShaper::shape() {
    if (fParagraph->text().size() == 0) {
        return false;
    }

    fUnresolvedGlyphs = 0;
    fParagraph->fUnresolvedCodepoints.clear();
    fParagraph->fFontSwitches.clear();
    fParagraph->fRuns.clear();

    SkScalar advanceX = 0;

    if (fParagraph->fPlaceholders.empty()) {
        TextRange entireText(0, fParagraph->text().size());
        if (!this->shapeTextRange(entireText, advanceX)) {
            return false;
        }
    } else {
        for (auto& placeholder : fParagraph->fPlaceholders) {
            if (placeholder.fTextBefore.width() > 0) {
                if (!this->shapeTextRange(placeholder.fTextBefore, advanceX)) {
                    return false;
                }
            }

            if (placeholder.fRange.width() == 0) {
                continue;
            }

            uint8_t placeholderLevel = 2;
            for (const auto& r : fParagraph->fBidiRegions) {
                if (placeholder.fRange.start >= r.start && placeholder.fRange.start < r.end) {
                    placeholderLevel = r.level;
                    break;
                }
            }

            std::vector<sk_sp<SkTypeface>> typefaces =
                    CoreTextFontRegistry::Instance().findTypefaces(
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
                SkShaper::RunHandler::Range(0, placeholder.fRange.width())
            };

            auto& run = fParagraph->fRuns.emplace_back(
                fParagraph,
                runInfo,
                placeholder.fRange.start,
                0.0f,
                false,
                0.0f,
                fParagraph->fRuns.size(),
                advanceX
            );

            run.fPositions[0] = { advanceX, 0 };
            run.fPositions[1] = { advanceX + placeholder.fStyle.fWidth, 0 };
            run.fOffsets[0] = {0, 0};
            run.fClusterIndexes[0] = 0;
            run.fPlaceholderIndex = &placeholder - fParagraph->fPlaceholders.begin();
            advanceX += placeholder.fStyle.fWidth;
        }
    }

    return true;
}

// CoreText typesetter produces a single visual line; isolating hard line breaks keeps each
// paragraph segment in one CTLine while CoreTextAttrStringBuilder splits CTRuns across blocks
// with differing Run-level font or metric properties
bool CoreTextLineShaper::shapeTextRange(TextRange range, SkScalar& advanceX) {
    if (range.width() == 0) {
        return true;
    }

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
    if (regionRange.width() == 0) {
        return true;
    }

    const char* regionUtf8 = fParagraph->text().data() + regionRange.start;
    size_t regionBytes = regionRange.width();
    TextDirection baseDir = fParagraph->paragraphStyle().getTextDirection();

    CoreTextAttrStringBuilder builder(
        regionUtf8, regionBytes, baseDir, fParagraph->fFontCollection);

    auto blockRange = fParagraph->findAllBlocks(regionRange);
    for (const auto& block : fParagraph->blocks(blockRange)) {
        size_t start = std::max(block.fRange.start, regionRange.start);
        size_t end = std::min(block.fRange.end, regionRange.end);
        if (start >= end) {
            continue;
        }
        TextRange relRange(start - regionRange.start, end - regionRange.start);
        builder.addStyle(relRange, block.fStyle);
    }

    SkUniqueCFRef<CFAttributedStringRef> attrStr = builder.build();
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

    CFArrayRef runs = CTLineGetGlyphRuns(line.get());
    CFIndex runCount = runs ? CFArrayGetCount(runs) : 0;
    if (runCount == 0) {
        return true;
    }

    struct CTRunItem {
        CTRunRef run;
        CFIndex location;
        CFIndex length;
    };
    skia_private::STArray<16, CTRunItem> sortedRuns(runCount);
    for (CFIndex i = 0; i < runCount; ++i) {
        CTRunRef ctRun = (CTRunRef)CFArrayGetValueAtIndex(runs, i);
        // Drop empty runs upfront so the trailing-newline lookahead sees no empty CTRuns after '\n'
        if (CTRunGetGlyphCount(ctRun) == 0) {
            continue;
        }
        CFRange cfRange = CTRunGetStringRange(ctRun);
        if (cfRange.length == 0) {
            continue;
        }
        sortedRuns.push_back({ctRun, cfRange.location, cfRange.length});
    }

    // CTRuns arrive in visual order from CTLineGetGlyphRuns
    // Logical ascending sort prevents BiDi inversion downstream in cluster layout
    std::stable_sort(sortedRuns.begin(), sortedRuns.end(),
                     [](const CTRunItem& a, const CTRunItem& b) {
                         return a.location < b.location;
                     });

    skia_private::THashMap<CTFontRef, sk_sp<SkTypeface>> typefaceCache;
    const bool fallbackDisabled = !fParagraph->fFontCollection->fontFallbackEnabled();

    // Neighboring runs mostly share a style block, so the typeface lookups and the
    // allowed-font verdicts are rebuilt only when the block changes, not for every run
    bool hasBlockState = false;
    const Block* runBlock = nullptr;
    const TextStyle* style = nullptr;
    sk_sp<SkTypeface> primary;
    std::vector<sk_sp<SkTypeface>> allowedTypefaces;
    SkString language;
    skia_private::THashMap<CTFontRef, bool> allowedFontCache;

    const auto& textStyles = fParagraph->fTextStyles;
    // fTextStyles is sorted by start, so the candidate is the last block starting at or before
    // utf8Pos; a position in a gap between blocks keeps the paragraph default style
    auto blockAt = [&](size_t utf8Pos) -> const Block* {
        const Block* nextBlock = std::upper_bound(
                textStyles.begin(),
                textStyles.end(),
                utf8Pos,
                [](size_t pos, const Block& b) { return pos < b.fRange.start; });
        if (nextBlock != textStyles.begin() && utf8Pos < nextBlock[-1].fRange.end) {
            return nextBlock - 1;
        }
        return nullptr;
    };

    auto isAllowedCtFont = [&](CTFontRef f) -> bool {
        bool* allowed = allowedFontCache.find(f);
        if (!allowed) {
            allowed = allowedFontCache.set(
                    f, is_allowed_font(f, allowedTypefaces, style->getFontFamilies()));
        }
        return *allowed;
    };

    // Read kCTFontAttributeName from attrStr to preserve exact descriptor attributes (such as
    // liga=0 or variations) for SkTypeface equality, falling back to createFont in unstyled gaps
    auto attrPrimaryCtFont = [&](CFIndex utf16Loc, SkUniqueCFRef<CTFontRef>& owned) -> CTFontRef {
        CTFontRef f = (CTFontRef)CFAttributedStringGetAttribute(
                attrStr.get(), utf16Loc, kCTFontAttributeName, nullptr);
        if (!f) {
            CGFloat standInSize = style->getFontSize() > 0.0f ? style->getFontSize() : 14.0f;
            owned = CoreTextFontRegistry::Instance().createFont(
                    primary.get(), style->getFontStyle(), standInSize);
            f = owned.get();
        }
        return f;
    };

    auto resolveBidiLevel = [&](size_t utf8Pos, CTRunRef r) -> uint8_t {
        const bool ctIsRtl = (CTRunGetStatus(r) & kCTRunStatusRightToLeft) != 0;
        uint8_t level = 0;
        bool foundRegion = false;
        for (const auto& reg : fParagraph->fBidiRegions) {
            if (utf8Pos >= reg.start && utf8Pos < reg.end) {
                level = reg.level;
                foundRegion = true;
                break;
            }
        }
        if (foundRegion) {
            // Reconcile ICU embedding level parity with CoreText run direction to ensure downstream
            // visual reordering matches
            if ((level % 2 != 0) != ctIsRtl) {
                level = ctIsRtl ? (level | 1) : (level > 0 ? level - 1 : 0);
            }
        } else {
            level = ctIsRtl ? 1 : 0;
        }
        return level;
    };

    for (int runIdx = 0; runIdx < sortedRuns.size(); ++runIdx) {
        const CTRunItem& item = sortedRuns[runIdx];
        CTRunRef ctRun = item.run;
        CFIndex glyphCount = CTRunGetGlyphCount(ctRun);

        CFRange cfRange = CFRangeMake(item.location, item.length);
        TextRange relUtf8Range = builder.indexMap().cfrangeToTextRange(cfRange);
        TextRange runUtf8Range(regionRange.start + relUtf8Range.start,
                               regionRange.start + relUtf8Range.end);
        if (runUtf8Range.width() == 0) {
            continue;
        }

        const Block* block = blockAt(runUtf8Range.start);

        if (!hasBlockState || block != runBlock) {
            hasBlockState = true;
            runBlock = block;
            style = block ? &block->fStyle : &fParagraph->paragraphStyle().getTextStyle();
            language = style->getLocale();
            allowedFontCache.reset();
            allowedTypefaces.clear();
            primary = style->refTypeface();
            if (primary) {
                allowedTypefaces.push_back(primary);
            } else {
                // Same clone the attributed string was built from, so LastResort runs and the
                // allowed-font check see the varied face (or default .AppleSystemUIFont) rather
                // than a null typeface
                allowedTypefaces = CoreTextFontRegistry::Instance().findTypefaces(
                        *fParagraph->fFontCollection,
                        style->getFontFamilies(),
                        style->getFontStyle(),
                        style->getFontArguments());
                if (!allowedTypefaces.empty()) {
                    primary = allowedTypefaces.front();
                }
            }
        }

        CFDictionaryRef runAttrs = CTRunGetAttributes(ctRun);
        // CoreText manufactures run attributes during font substitution, and they always
        // carry the font the glyphs were shaped with
        CTFontRef ctFont = (CTFontRef)CFDictionaryGetValue(runAttrs, kCTFontAttributeName);
        SkASSERT(ctFont);
        bool isFallback = isLastResortFont(ctFont);
        if (!isFallback && fallbackDisabled) {
            isFallback = !isAllowedCtFont(ctFont);
        }

        // CoreText cascades U+000A to a fallback font when the primary face lacks a newline glyph;
        // keep a standalone '\n' on the block's primary CTFont as glyph 0 so empty-line metrics
        // and font equality match the primary face
        SkUniqueCFRef<CTFontRef> ownedAttrFont;
        bool pinnedStandaloneNewline = false;
        if (runUtf8Range.width() == 1 && fParagraph->text()[runUtf8Range.start] == '\n') {
            CTFontRef attrFont = attrPrimaryCtFont(item.location, ownedAttrFont);
            const UniChar nlChar = '\n';
            CGGlyph nlGlyph = 0;
            if (attrFont && !CTFontGetGlyphsForCharacters(attrFont, &nlChar, &nlGlyph, 1) &&
                !isLastResortFont(attrFont) && (!fallbackDisabled || isAllowedCtFont(attrFont))) {
                ctFont = attrFont;
                isFallback = false;
                pinnedStandaloneNewline = true;
            }
        }

        // CoreText reads a zero point size as its 12pt default, so the attributed string shapes
        // such text at a stand-in size and the run collapses to a zero-size font with zero
        // advances, matching HarfBuzz, so builds without HarfBuzz lay it out too
        const bool zeroSize = style->getFontSize() <= 0;
        CGFloat fontSize = zeroSize ? 0 : CTFontGetSize(ctFont);
        SkFont font;
        if (isFallback) {
            font = SkFont(primary, SkScalarFromCGFloat(fontSize));
        } else {
            sk_sp<SkTypeface> typeface;
            if (sk_sp<SkTypeface>* cached = typefaceCache.find(ctFont)) {
                typeface = *cached;
            } else {
                typeface = SkMakeTypefaceFromCTFont(ctFont);
                typefaceCache.set(ctFont, typeface);
            }
            font = SkFont(std::move(typeface), SkScalarFromCGFloat(fontSize));
        }

        // Fallback runs use the primary typeface rather than the bound CTFont
        apply_font_style_props(font,
                               isFallback ? nullptr : ctFont,
                               *style,
                               fParagraph->paragraphStyle().fakeMissingFontStyles());

        uint8_t bidiLevel = resolveBidiLevel(runUtf8Range.start, ctRun);

        // The itemization that keeps cursive ranges unspaced also names the run's
        // script, so Run::isCursiveScript agrees with the glyphs CoreText produced
        SkFourByteTag script = builder.scriptAt(relUtf8Range.start);

        // CoreText splits a trailing '\n' into a separate CTRun when the primary face lacks U+000A
        // or on RTL lines; fold it back into the preceding primary-font run so line-end caret
        // hit-testing and multi-line spacing see one run per segment
        const CFIndex ctGlyphCount = glyphCount;
        bool coalescedNewline = false;
        CGGlyph coalescedNlGlyph = 0;
        if (!isFallback && runIdx + 2 == sortedRuns.size() &&
            runUtf8Range.end + 1 == regionRange.end &&
            fParagraph->text()[regionRange.end - 1] == '\n') {
            const CTRunItem& nlItem = sortedRuns[runIdx + 1];
            if (nlItem.location == item.location + item.length && nlItem.length == 1 &&
                CTRunGetGlyphCount(nlItem.run) == 1 && blockAt(runUtf8Range.end) == block &&
                resolveBidiLevel(runUtf8Range.end, nlItem.run) == bidiLevel) {
                SkUniqueCFRef<CTFontRef> ownedNlFont;
                CTFontRef nlAttrFont = attrPrimaryCtFont(nlItem.location, ownedNlFont);
                if (nlAttrFont && CFEqual(ctFont, nlAttrFont)) {
                    const UniChar nlChar = '\n';
                    if (!CTFontGetGlyphsForCharacters(ctFont, &nlChar, &coalescedNlGlyph, 1)) {
                        coalescedNlGlyph = 0;
                    }
                    coalescedNewline = true;
                    glyphCount = ctGlyphCount + 1;
                    runUtf8Range.end = regionRange.end;
                    ++runIdx;
                }
            }
        }

        // RTL runs store glyphs in visual order, so a logical-end '\n' sits at visual slot 0
        const bool runIsLtr = (bidiLevel % 2) == 0;
        const CFIndex base = (coalescedNewline && !runIsLtr) ? 1 : 0;
        const CFIndex nlOutIdx = runIsLtr ? ctGlyphCount : 0;

        skia_private::AutoSTArray<64, CFIndex> indexBuffer(ctGlyphCount);
        CTRunGetStringIndices(ctRun, CFRangeMake(0, ctGlyphCount), indexBuffer.data());

        skia_private::AutoSTArray<64, uint32_t> clusterBuffer(glyphCount);
        for (CFIndex k = 0; k < ctGlyphCount; ++k) {
            size_t u8InRegion = builder.indexMap().toUtf8Index(indexBuffer[k]);
            size_t globalU8 = regionRange.start + u8InRegion;
            size_t clusterInRun =
                    (globalU8 >= runUtf8Range.start) ? (globalU8 - runUtf8Range.start) : 0;
            clusterBuffer[base + k] = SkToU32(clusterInRun);
        }
        normalize_cluster_indices(clusterBuffer.data() + base, ctGlyphCount, runIsLtr);
        if (coalescedNewline) {
            clusterBuffer[nlOutIdx] = SkToU32(runUtf8Range.width() - 1);
        }

        auto glyphTextRange = [&](CFIndex k) -> TextRange {
            size_t cStart = clusterBuffer[k];
            size_t cEnd = cStart;
            if (runIsLtr) {
                cEnd = (k + 1 < glyphCount) ? clusterBuffer[k + 1] : runUtf8Range.width();
            } else {
                cEnd = (k > 0) ? clusterBuffer[k - 1] : runUtf8Range.width();
            }
            if (cStart > cEnd) {
                std::swap(cStart, cEnd);
            }
            return TextRange(runUtf8Range.start + cStart, runUtf8Range.start + cEnd);
        };

        // Control and zero-width format characters never count toward unresolved glyphs or
        // codepoints, matching HarfBuzz OneLineShaper::iterateThroughFontStyles
        auto recordUnresolvedGlyph = [&](TextRange gRange) {
            const char* p = fParagraph->text().data() + gRange.start;
            const char* pEnd = fParagraph->text().data() + gRange.end;
            bool hasNonControl = false;
            while (p < pEnd) {
                size_t bytePos = p - fParagraph->text().data();
                SkUnichar u = SkUTF::NextUTF8(&p, pEnd);
                if (u < 0) {
                    break;
                }
                if (!fParagraph->codeUnitHasProperty(bytePos, SkUnicode::CodeUnitFlags::kControl) &&
                    !IsZeroWidthCodepoint(u)) {
                    hasNonControl = true;
                    fParagraph->fUnresolvedCodepoints.emplace(u);
                }
            }
            if (hasNonControl) {
                ++fUnresolvedGlyphs;
            }
        };

        skia_private::AutoSTArray<64, SkScalar> glyphAdvances(glyphCount);
        skia_private::AutoSTArray<64, CGSize> advances(isFallback ? 0 : glyphCount);
        skia_private::AutoSTArray<64, CGPoint> origins(isFallback ? 0 : glyphCount);
        skia_private::AutoSTArray<64, CGGlyph> glyphBuffer(isFallback ? 0 : glyphCount);
        // Splitting follows the HarfBuzz plan only as far as it was checked: left-to-right runs
        // of default-shaper text
        bool splitKerning = false;
        if (!isFallback && !zeroSize && runIsLtr) {
            splitKerning =
                    splits_pair_kerning_cached(ctFont) &&
                    has_only_split_kerning_text(fParagraph->text().data() + runUtf8Range.start,
                                                fParagraph->text().data() + runUtf8Range.end);
        }
        skia_private::AutoSTArray<64, int64_t> splitOffsets16(splitKerning ? glyphCount : 0);
        SkScalar runAdvanceX = 0;

        if (isFallback) {
            SkScalar notdefAdvance = font.getWidth(0);
            if (SkScalarNearlyZero(notdefAdvance)) {
                notdefAdvance = font.getSize();
            }
            // HarfBuzz gives .notdef the same advance as any other glyph, whole pixels included
            if (!font.isSubpixel()) {
                notdefAdvance = SkScalarRoundToInt(notdefAdvance);
            }
            notdefAdvance = round_like_harfbuzz(notdefAdvance);

            for (CFIndex k = 0; k < glyphCount; ++k) {
                TextRange gRange = glyphTextRange(k);
                const char* p = fParagraph->text().data() + gRange.start;
                const char* pEnd = fParagraph->text().data() + gRange.end;
                size_t visibleCount = 0;
                while (p < pEnd) {
                    SkUnichar u = SkUTF::NextUTF8(&p, pEnd);
                    if (u < 0) {
                        break;
                    }
                    if (!IsZeroWidthCodepoint(u)) {
                        ++visibleCount;
                    }
                }

                glyphAdvances[k] = visibleCount * notdefAdvance;
                runAdvanceX += glyphAdvances[k];
            }
        } else {
            // ctRun holds the fallback font's glyph ID rather than the pinned primary face's, so
            // emit .notdef (0) directly and assign its HarfBuzz advance in the newline loop below
            if (pinnedStandaloneNewline) {
                glyphBuffer[0] = 0;
                origins[0] = CGPointZero;
                glyphAdvances[0] = 0.0f;
                if (splitKerning) {
                    splitOffsets16[0] = 0;
                }
            } else {
                CTRunGetGlyphs(ctRun, CFRangeMake(0, ctGlyphCount), glyphBuffer.data() + base);
                // CTRunGetAdvances derives advances from drawn x, so a zero-advance mark drawn d
                // off the pen shows as +d on the glyph before it in the run and -d on the mark.
                // Base advances leave mark positioning to the origins, as HarfBuzz leaves it to
                // x_offset
                CTRunGetBaseAdvancesAndOrigins(ctRun,
                                               CFRangeMake(0, ctGlyphCount),
                                               advances.data() + base,
                                               origins.data() + base);
                skia_private::AutoSTArray<64, CGSize> nominal(ctGlyphCount);
                CTFontGetAdvancesForGlyphs(ctFont,
                                           kCTFontOrientationHorizontal,
                                           glyphBuffer.data() + base,
                                           nominal.data(),
                                           ctGlyphCount);
                // The 12pt stand-in advances CoreText gives zero-size text never reach the run
                if (zeroSize) {
                    std::fill_n(glyphAdvances.data(), glyphCount, 0.0f);
                } else {
                    // KEEPER-DEBT: this keeps CoreText's advance to the next 28pt tab stop, where
                    // HarfBuzz gives U+0009 its hmtx advance; the stop counts from the CTLine start
                    // (hard break or placeholder), so after a soft wrap it is off the line's stops.
                    // Remove when tabs get the HarfBuzz advance or line-relative stops
                    harfbuzz_advances(ctFont,
                                      font.isSubpixel(),
                                      nominal.data(),
                                      advances.data() + base,
                                      glyphAdvances.data() + base,
                                      splitKerning ? splitOffsets16.data() + base : nullptr,
                                      ctGlyphCount);
                }
                if (coalescedNewline) {
                    glyphBuffer[nlOutIdx] = coalescedNlGlyph;
                    origins[nlOutIdx] = CGPointZero;
                    glyphAdvances[nlOutIdx] = 0.0f;
                    if (splitKerning) {
                        splitOffsets16[nlOutIdx] = 0;
                    }
                }
            }
            for (CFIndex k = 0; k < glyphCount; ++k) {
                // CoreText gives '\n' no advance, HarfBuzz the advance of the primary font's
                // cmap glyph (.notdef when it has none); that advance moves all later lines, so
                // their float positions and widths differ unless '\n' gets one here too
                TextRange gRange = glyphTextRange(k);
                if (!zeroSize && gRange.width() == 1 && fParagraph->text()[gRange.start] == '\n') {
                    SkScalar newlineAdvance = font.getWidth(font.unicharToGlyph('\n'));
                    // Whole pixels for a non-subpixel font, as skhb_glyph_h_advances does
                    if (!font.isSubpixel()) {
                        newlineAdvance = SkScalarRoundToInt(newlineAdvance);
                    }
                    glyphAdvances[k] = round_like_harfbuzz(newlineAdvance);
                }
                runAdvanceX += glyphAdvances[k];
            }
        }

        SkFontMetrics fontMetrics;
        font.getMetrics(&fontMetrics);
        SkScalar runHeight = style->getHeightOverride() ? style->getHeight() : 0.0f;

        const SkShaper::RunHandler::RunInfo runInfo = {
                font,
                bidiLevel,
                script,
                language.c_str(),
                SkPoint::Make(runAdvanceX,
                              fontMetrics.fDescent - fontMetrics.fAscent + fontMetrics.fLeading),
                static_cast<size_t>(glyphCount),
                SkShaper::RunHandler::Range(0, runUtf8Range.width())};

        auto& run = fParagraph->fRuns.emplace_back(
            fParagraph,
            runInfo,
            runUtf8Range.start,
            runHeight,
            style->getHalfLeading(),
            style->getBaselineShift(),
            fParagraph->fRuns.size(),
            advanceX
        );

        if (isFallback) {
            // CoreText renders missing glyphs through LastResort; remap to glyph 0 (.notdef) to
            // honor SkParagraph unresolved contract
            SkScalar accumulatedX = 0;
            for (CFIndex k = 0; k < glyphCount; ++k) {
                run.fGlyphs[k] = 0;
                run.fPositions[k] = SkPoint::Make(advanceX + accumulatedX, 0);
                accumulatedX += glyphAdvances[k];
                run.fOffsets[k] = SkPoint::Make(0, 0);
                run.fClusterIndexes[k] = clusterBuffer[k];
                recordUnresolvedGlyph(glyphTextRange(k));
            }
        } else {
            // The pen walks the rounded advances in float as HarfBuzz does
            SkScalar penX = 0;
            for (CFIndex k = 0; k < glyphCount; ++k) {
                run.fGlyphs[k] = glyphBuffer[k];

                // Zero-size text pins the origins to 0, as its advances
                const CGPoint origin = zeroSize ? CGPointZero : origins[k];
                // Cluster widths and hit-testing read fPositions, and extendedVisit and getPath
                // hand fPositions out without the offsets, so the whole origin goes into the
                // offset as SkShaper's HarfBuzz runs put x_offset and y_offset there
                run.fPositions[k] = SkPoint::Make(advanceX + penX, 0);
                penX += glyphAdvances[k];
                // CoreText is y-up, Skia y-down
                // Same 16.16 rounding as round_like_harfbuzz, plus the part of a kerning pair
                // HarfBuzz moves from the advance before into this glyph's offset
                const int64_t offsetX16 =
                        SkScalarRoundToInt(SkScalarFromCGFloat(origin.x) * (1 << 16)) +
                        (splitKerning ? splitOffsets16[k] : 0);
                run.fOffsets[k] =
                        SkPoint::Make(static_cast<SkScalar>(offsetX16 * 1.52587890625e-5),
                                      round_like_harfbuzz(-SkScalarFromCGFloat(origin.y)));
                run.fClusterIndexes[k] = clusterBuffer[k];
                if (run.fGlyphs[k] == 0) {
                    recordUnresolvedGlyph(glyphTextRange(k));
                }
            }
        }

        // Trailing N+1 sentinel terminates position and cluster interpolations during line breaking
        // and caret hit-testing
        run.fPositions[glyphCount] = SkPoint::Make(advanceX + runAdvanceX, 0);
        run.fOffsets[glyphCount] = SkPoint::Make(0, 0);
        run.fClusterIndexes[glyphCount] = run.leftToRight() ? SkToU32(runUtf8Range.width()) : 0;

        fParagraph->fFontSwitches.emplace_back(runUtf8Range.start, run.font());
        advanceX += runAdvanceX;
    }

    return true;
}

}  // namespace textlayout
}  // namespace skia

#endif  // defined(SK_PARAGRAPH_NATIVE_CORETEXT)
