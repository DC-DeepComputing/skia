// Copyright 2026 Google LLC
#include "modules/skparagraph/src/CoreTextLineShaper.h"

#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)

#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPoint.h"
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

namespace skia {
namespace textlayout {

namespace {

// Code points that take no advance of their own, so a fallback .notdef box is not widened
// for them
bool is_zero_width_codepoint(SkUnichar u) {
    return (u >= 0x200B && u <= 0x200F)
        || (u >= 0x202A && u <= 0x202E)
        || (u >= 0x2060 && u <= 0x2069)
        || (u >= 0x0300 && u <= 0x036F)
        || (u >= 0x1AB0 && u <= 0x1AFF)
        || (u >= 0x1DC0 && u <= 0x1DFF)
        || (u >= 0x20D0 && u <= 0x20FF)
        || (u >= 0xFE00 && u <= 0xFE0F)
        || (u >= 0xFE20 && u <= 0xFE2F)
        || (u >= 0xE0100 && u <= 0xE01EF)
        || u == 0xFEFF
        || u == 0x00AD
        || u == 0x034F
        || u == 0x061C;
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

static void apply_font_style_props(SkFont& font,
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
        const bool fakeItalic = wanted.slant() == SkFontStyle::kItalic_Slant &&
                                bound.slant() != SkFontStyle::kItalic_Slant;
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

}  // namespace

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
    const CFIndex attrLen = CFAttributedStringGetLength(attrStr.get());
    SkUniqueCFRef<CTTypesetterRef> typesetter(
            CTTypesetterCreateWithAttributedString(attrStr.get()));
    if (!typesetter) {
        return;
    }
    SkUniqueCFRef<CTLineRef> line(
            CTTypesetterCreateLine(typesetter.get(), CFRangeMake(0, attrLen)));
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
    apply_font_style_props(font, affixStyle, fakeMissingFontStyles);

    const bool ctIsRtl = (CTRunGetStatus(primaryRun) & kCTRunStatusRightToLeft) != 0;
    uint8_t level = bidiLevel;
    if ((level % 2 != 0) != ctIsRtl) {
        level = ctIsRtl ? (level | 1) : (level > 0 ? level - 1 : 0);
    }

    SkScalar totalAdvance = 0;
    for (CFIndex i = 0; i < runCount; ++i) {
        CTRunRef ctRun = (CTRunRef)CFArrayGetValueAtIndex(runs, i);
        const CFIndex gc = CTRunGetGlyphCount(ctRun);
        if (gc <= 0) {
            continue;
        }
        skia_private::AutoSTArray<16, CGSize> advances(gc);
        CTRunGetAdvances(ctRun, CFRangeMake(0, gc), advances.data());
        for (CFIndex k = 0; k < gc; ++k) {
            totalAdvance += zeroSize ? 0 : SkScalarFromCGFloat(advances[k].width);
        }
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

        CTRunGetGlyphs(ctRun, CFRangeMake(0, gc), buffer.glyphs + outIdx);
        skia_private::AutoSTArray<16, CGPoint> positions(gc);
        CTRunGetPositions(ctRun, CFRangeMake(0, gc), positions.data());
        skia_private::AutoSTArray<16, CFIndex> indices(gc);
        CTRunGetStringIndices(ctRun, CFRangeMake(0, gc), indices.data());
        for (CFIndex k = 0; k < gc; ++k) {
            if (unresolvedRun) {
                buffer.glyphs[outIdx + k] = 0;
            }
            // CoreText is y-up, Skia y-down
            const CGFloat x = zeroSize ? 0 : positions[k].x;
            const CGFloat y = zeroSize ? 0 : positions[k].y;
            buffer.positions[outIdx + k] =
                    buffer.point + SkPoint::Make(SkScalarFromCGFloat(x), -SkScalarFromCGFloat(y));
            if (buffer.offsets) {
                buffer.offsets[outIdx + k] = {0, 0};
            }
            if (buffer.clusters) {
                buffer.clusters[outIdx + k] =
                        SkToU32(builder.indexMap().toUtf8Index(indices[k]));
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

// CoreText typesetter produces a single visual line; isolating hard line breaks
// and splitting across differing height/shift metrics guarantees 1:1 Run correspondence
bool CoreTextLineShaper::shapeTextRange(TextRange range, SkScalar& advanceX) {
    if (range.width() == 0) {
        return true;
    }

    // CTRun shares one SkFont across all glyphs, so blocks with differing vertical metrics,
    // rasterization flags, or synthetic style traits must split into separate CTLine regions
    auto runPropsDiffer = [](const TextStyle& a, const TextStyle& b) {
        SkScalar hA = a.getHeightOverride() ? a.getHeight() : 0.0f;
        SkScalar hB = b.getHeightOverride() ? b.getHeight() : 0.0f;
        return hA != hB ||
               a.getHalfLeading() != b.getHalfLeading() ||
               a.getBaselineShift() != b.getBaselineShift() ||
               a.getFontEdging() != b.getFontEdging() ||
               a.getFontHinting() != b.getFontHinting() ||
               a.getSubpixel() != b.getSubpixel() ||
               a.getFontStyle() != b.getFontStyle();
    };

    const char* text = fParagraph->text().data();
    size_t segStart = range.start;

    while (segStart < range.end) {
        if (text[segStart] == '\n') {
            if (!this->shapeTextRegion(TextRange(segStart, segStart + 1), advanceX)) {
                return false;
            }
            ++segStart;
            continue;
        }

        size_t segEnd = segStart + 1;
        while (segEnd < range.end && text[segEnd] != '\n') {
            ++segEnd;
        }

        auto blockRange = fParagraph->findAllBlocks(TextRange(segStart, segEnd));
        if (blockRange.empty()) {
            if (!this->shapeTextRegion(TextRange(segStart, segEnd), advanceX)) {
                return false;
            }
        } else {
            auto blocks = fParagraph->blocks(blockRange);
            size_t subStart = segStart;
            const TextStyle* currentStyle = nullptr;

            for (size_t b = 0; b < blocks.size(); ++b) {
                size_t bStart = std::max(blocks[b].fRange.start, segStart);
                size_t bEnd = std::min(blocks[b].fRange.end, segEnd);
                if (bStart >= bEnd) {
                    continue;
                }

                if (!currentStyle) {
                    currentStyle = &blocks[b].fStyle;
                } else if (runPropsDiffer(*currentStyle, blocks[b].fStyle)) {
                    if (!this->shapeTextRegion(TextRange(subStart, bStart), advanceX)) {
                        return false;
                    }
                    subStart = bStart;
                    currentStyle = &blocks[b].fStyle;
                } else {
                    currentStyle = &blocks[b].fStyle;
                }
            }

            if (subStart < segEnd) {
                if (!this->shapeTextRegion(TextRange(subStart, segEnd), advanceX)) {
                    return false;
                }
            }
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

    CFIndex attrLen = CFAttributedStringGetLength(attrStr.get());
    if (attrLen == 0) {
        return true;
    }

    SkUniqueCFRef<CTTypesetterRef> typesetter(
        CTTypesetterCreateWithAttributedString(attrStr.get()));
    if (!typesetter) {
        return false;
    }

    SkUniqueCFRef<CTLineRef> line(
        CTTypesetterCreateLine(typesetter.get(), CFRangeMake(0, attrLen)));
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
        CGFloat visualOriginX;
    };
    skia_private::STArray<16, CTRunItem> sortedRuns(runCount);
    CGFloat currentVisualX = 0;
    for (CFIndex i = 0; i < runCount; ++i) {
        CTRunRef ctRun = (CTRunRef)CFArrayGetValueAtIndex(runs, i);
        CFRange cfRange = CTRunGetStringRange(ctRun);
        CGFloat runWidth =
                CTRunGetTypographicBounds(ctRun, CFRangeMake(0, 0), nullptr, nullptr, nullptr);
        sortedRuns.push_back({ctRun, cfRange.location, cfRange.length, currentVisualX});
        currentVisualX += runWidth;
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
    for (const auto& item : sortedRuns) {
        CTRunRef ctRun = item.run;
        CFIndex glyphCount = CTRunGetGlyphCount(ctRun);
        if (glyphCount == 0) {
            continue;
        }

        CFRange cfRange = CFRangeMake(item.location, item.length);
        TextRange relUtf8Range = builder.indexMap().cfrangeToTextRange(cfRange);
        TextRange runUtf8Range(regionRange.start + relUtf8Range.start,
                               regionRange.start + relUtf8Range.end);
        if (runUtf8Range.width() == 0) {
            continue;
        }

        // fTextStyles is sorted by start, so the candidate is the last block starting at or
        // before the run; a run in a gap between blocks keeps the paragraph default style
        const Block* nextBlock = std::upper_bound(
                textStyles.begin(),
                textStyles.end(),
                runUtf8Range.start,
                [](size_t pos, const Block& block) { return pos < block.fRange.start; });
        const Block* block = nullptr;
        if (nextBlock != textStyles.begin() && runUtf8Range.start < nextBlock[-1].fRange.end) {
            block = nextBlock - 1;
        }

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
            bool* allowed = allowedFontCache.find(ctFont);
            if (!allowed) {
                allowed = allowedFontCache.set(
                        ctFont,
                        is_allowed_font(ctFont, allowedTypefaces, style->getFontFamilies()));
            }
            isFallback = !*allowed;
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

        apply_font_style_props(
                font, *style, fParagraph->paragraphStyle().fakeMissingFontStyles());

        CTRunStatus status = CTRunGetStatus(ctRun);
        bool ctIsRtl = (status & kCTRunStatusRightToLeft) != 0;
        uint8_t bidiLevel = 0;
        bool foundRegion = false;
        for (const auto& r : fParagraph->fBidiRegions) {
            if (runUtf8Range.start >= r.start && runUtf8Range.start < r.end) {
                bidiLevel = r.level;
                foundRegion = true;
                break;
            }
        }
        if (foundRegion) {
            // Reconcile ICU embedding level parity with CoreText run direction to ensure downstream
            // visual reordering matches
            if ((bidiLevel % 2 != 0) != ctIsRtl) {
                bidiLevel = ctIsRtl ? (bidiLevel | 1) : (bidiLevel > 0 ? bidiLevel - 1 : 0);
            }
        } else {
            bidiLevel = ctIsRtl ? 1 : 0;
        }

        // The itemization that withheld tracking from cursive ranges also names the run's
        // script, so Run::isCursiveScript agrees with the glyphs CoreText produced
        SkFourByteTag script = builder.scriptAt(relUtf8Range.start);

        skia_private::AutoSTArray<64, CFIndex> indexBuffer(glyphCount);
        CTRunGetStringIndices(ctRun, CFRangeMake(0, glyphCount), indexBuffer.data());

        skia_private::AutoSTArray<64, uint32_t> clusterBuffer(glyphCount);
        for (CFIndex k = 0; k < glyphCount; ++k) {
            size_t u8InRegion = builder.indexMap().toUtf8Index(indexBuffer[k]);
            size_t globalU8 = regionRange.start + u8InRegion;
            size_t clusterInRun =
                    (globalU8 >= runUtf8Range.start) ? (globalU8 - runUtf8Range.start) : 0;
            clusterBuffer[k] = SkToU32(clusterInRun);
        }
        const bool runIsLtr = (bidiLevel % 2) == 0;
        normalize_cluster_indices(clusterBuffer.data(), glyphCount, runIsLtr);

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
                    !is_zero_width_codepoint(u)) {
                    hasNonControl = true;
                    fParagraph->fUnresolvedCodepoints.emplace(u);
                }
            }
            if (hasNonControl) {
                ++fUnresolvedGlyphs;
            }
        };

        skia_private::AutoSTArray<64, SkScalar> glyphAdvances(glyphCount);
        SkScalar runAdvanceX = 0;

        if (isFallback) {
            SkScalar notdefAdvance = font.getWidth(0);
            if (SkScalarNearlyZero(notdefAdvance)) {
                notdefAdvance = font.getSize();
            }

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
                    if (!is_zero_width_codepoint(u)) {
                        ++visibleCount;
                    }
                }

                glyphAdvances[k] = visibleCount * notdefAdvance;
                runAdvanceX += glyphAdvances[k];
            }
        } else {
            skia_private::AutoSTArray<64, CGSize> advances(glyphCount);
            CTRunGetAdvances(ctRun, CFRangeMake(0, glyphCount), advances.data());
            for (CFIndex k = 0; k < glyphCount; ++k) {
                glyphAdvances[k] = zeroSize ? 0 : SkScalarFromCGFloat(advances[k].width);
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
            skia_private::AutoSTArray<64, CGGlyph> glyphBuffer(glyphCount);
            CTRunGetGlyphs(ctRun, CFRangeMake(0, glyphCount), glyphBuffer.data());

            skia_private::AutoSTArray<64, CGPoint> posBuffer(glyphCount);
            CTRunGetPositions(ctRun, CFRangeMake(0, glyphCount), posBuffer.data());

            for (CFIndex k = 0; k < glyphCount; ++k) {
                run.fGlyphs[k] = glyphBuffer[k];

                CGFloat relX = zeroSize ? 0 : posBuffer[k].x - item.visualOriginX;
                CGFloat relY = zeroSize ? 0 : posBuffer[k].y;
                // Invert Y coordinate because CoreText uses a bottom-left Cartesian origin while
                // Skia expects a top-left origin
                run.fPositions[k] = SkPoint::Make(advanceX + SkScalarFromCGFloat(relX),
                                                  -SkScalarFromCGFloat(relY));
                run.fOffsets[k] = SkPoint::Make(0, 0);
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
