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

static SkFourByteTag resolve_codepoint_script(SkUnichar c) {
    if ((c >= 0x0600 && c <= 0x06FF) ||
        (c >= 0x0750 && c <= 0x077F) ||
        (c >= 0x08A0 && c <= 0x08FF) ||
        (c >= 0xFB50 && c <= 0xFDFF) ||
        (c >= 0xFE70 && c <= 0xFEFF)) {
        return SkSetFourByteTag('A', 'r', 'a', 'b');
    }
    if ((c >= 0x0700 && c <= 0x074F) ||
        (c >= 0x0860 && c <= 0x086F)) {
        return SkSetFourByteTag('S', 'y', 'r', 'c');
    }
    if (c >= 0x10D00 && c <= 0x10D3F) {
        return SkSetFourByteTag('R', 'o', 'h', 'g');
    }
    if (c >= 0x0840 && c <= 0x085F) {
        return SkSetFourByteTag('M', 'a', 'n', 'd');
    }
    if ((c >= 0x1800 && c <= 0x18AF) ||
        (c >= 0x11660 && c <= 0x1167F)) {
        return SkSetFourByteTag('M', 'o', 'n', 'g');
    }
    if (c >= 0x07C0 && c <= 0x07FF) {
        return SkSetFourByteTag('N', 'k', 'o', 'o');
    }
    if (c >= 0xA840 && c <= 0xA87F) {
        return SkSetFourByteTag('P', 'h', 'a', 'g');
    }
    if ((c >= 0x0590 && c <= 0x05FF) ||
        (c >= 0xFB1D && c <= 0xFB4F)) {
        return SkSetFourByteTag('H', 'e', 'b', 'r');
    }
    if ((c >= 0x0400 && c <= 0x052F) ||
        (c >= 0x2DE0 && c <= 0x2DFF) ||
        (c >= 0xA640 && c <= 0xA69F)) {
        return SkSetFourByteTag('C', 'y', 'r', 'l');
    }
    if ((c >= 0x0370 && c <= 0x03FF) ||
        (c >= 0x1F00 && c <= 0x1FFF)) {
        return SkSetFourByteTag('G', 'r', 'e', 'k');
    }
    if ((c >= 0x0900 && c <= 0x097F) ||
        (c >= 0xA8E0 && c <= 0xA8FF)) {
        return SkSetFourByteTag('D', 'e', 'v', 'a');
    }
    if (c >= 0x0E00 && c <= 0x0E7F) {
        return SkSetFourByteTag('T', 'h', 'a', 'i');
    }
    if ((c >= 0x4E00 && c <= 0x9FFF) ||
        (c >= 0x3400 && c <= 0x4DBF) ||
        (c >= 0xF900 && c <= 0xFAFF) ||
        (c >= 0x20000 && c <= 0x2EBEF)) {
        return SkSetFourByteTag('H', 'a', 'n', 'i');
    }
    if (c >= 0x3040 && c <= 0x309F) {
        return SkSetFourByteTag('H', 'i', 'r', 'a');
    }
    if ((c >= 0x30A0 && c <= 0x30FF) ||
        (c >= 0x31F0 && c <= 0x31FF)) {
        return SkSetFourByteTag('K', 'a', 'n', 'a');
    }
    if ((c >= 0xAC00 && c <= 0xD7AF) ||
        (c >= 0x1100 && c <= 0x11FF) ||
        (c >= 0x3130 && c <= 0x318F)) {
        return SkSetFourByteTag('H', 'a', 'n', 'g');
    }
    if ((c >= 0x10A0 && c <= 0x10FF) ||
        (c >= 0x2D00 && c <= 0x2D2F)) {
        return SkSetFourByteTag('G', 'e', 'o', 'r');
    }
    if (c >= 0x0530 && c <= 0x058F) {
        return SkSetFourByteTag('A', 'r', 'm', 'n');
    }
    if ((c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z') ||
        (c >= 0x00C0 && c <= 0x02AF) ||
        (c >= 0x1E00 && c <= 0x1EFF)) {
        return SkSetFourByteTag('L', 'a', 't', 'n');
    }
    return 0;
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

}  // namespace

SkFourByteTag CoreTextLineShaper::detectScript(const char* utf8, size_t len) {
    SkASSERT(utf8);
    const char* ptr = utf8;
    const char* end = utf8 + len;
    while (ptr < end) {
        SkUnichar u = SkUTF::NextUTF8(&ptr, end);
        if (u < 0) {
            break;
        }
        SkFourByteTag script = resolve_codepoint_script(u);
        if (script != 0) {
            return script;
        }
    }
    return SkSetFourByteTag('L', 'a', 't', 'n');
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
                                    uint8_t bidiLevel,
                                    sk_sp<FontCollection> fontCollection,
                                    SkShaper::RunHandler* handler) {
    SkASSERT(handler);
    if (text.isEmpty()) {
        return;
    }
    const TextDirection baseDir = (bidiLevel & 1) ? TextDirection::kRtl : TextDirection::kLtr;
    CoreTextAttrStringBuilder builder(
            text.c_str(), text.size(), baseDir, std::move(fontCollection));
    builder.addStyle(TextRange(0, text.size()), style);
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

    // Same zero-size stand-in as shapeTextRegion: CoreText shaped at its default size
    const bool zeroSize = style.getFontSize() <= 0;
    const SkString language = style.getLocale();
    // CTRun positions are relative to the line, while each Run starts at its own origin
    CGFloat runOriginX = 0;
    handler->beginLine();
    for (CFIndex i = 0; i < runCount; ++i) {
        CTRunRef ctRun = (CTRunRef)CFArrayGetValueAtIndex(runs, i);
        const CGFloat runWidth =
                CTRunGetTypographicBounds(ctRun, CFRangeMake(0, 0), nullptr, nullptr, nullptr);
        const CFIndex glyphCount = CTRunGetGlyphCount(ctRun);
        if (glyphCount == 0) {
            runOriginX += runWidth;
            continue;
        }

        CTFontRef ctFont =
                (CTFontRef)CFDictionaryGetValue(CTRunGetAttributes(ctRun), kCTFontAttributeName);
        SkASSERT(ctFont);
        const bool lastResort = isLastResortFont(ctFont);
        SkFont font(SkMakeTypefaceFromCTFont(ctFont),
                    zeroSize ? 0 : SkScalarFromCGFloat(CTFontGetSize(ctFont)));
        font.setEdging(style.getFontEdging());
        font.setHinting(style.getFontHinting());
        font.setSubpixel(style.getSubpixel());

        const bool ctIsRtl = (CTRunGetStatus(ctRun) & kCTRunStatusRightToLeft) != 0;
        uint8_t level = bidiLevel;
        if ((level % 2 != 0) != ctIsRtl) {
            level = ctIsRtl ? (level | 1) : (level > 0 ? level - 1 : 0);
        }

        const TextRange utf8Range =
                builder.indexMap().cfrangeToTextRange(CTRunGetStringRange(ctRun));

        skia_private::AutoSTArray<16, CGSize> advances(glyphCount);
        CTRunGetAdvances(ctRun, CFRangeMake(0, glyphCount), advances.data());
        SkScalar runAdvance = 0;
        for (CFIndex k = 0; k < glyphCount; ++k) {
            runAdvance += zeroSize ? 0 : SkScalarFromCGFloat(advances[k].width);
        }

        const SkShaper::RunHandler::RunInfo info = {
                font,
                level,
                detectScript(text.c_str() + utf8Range.start, utf8Range.width()),
                language.c_str(),
                SkVector::Make(runAdvance, 0),
                static_cast<size_t>(glyphCount),
                SkShaper::RunHandler::Range(utf8Range.start, utf8Range.width())};
        handler->runInfo(info);
        handler->commitRunInfo();
        SkShaper::RunHandler::Buffer buffer = handler->runBuffer(info);

        static_assert(sizeof(CGGlyph) == sizeof(SkGlyphID));
        CTRunGetGlyphs(ctRun, CFRangeMake(0, glyphCount), buffer.glyphs);
        skia_private::AutoSTArray<16, CGPoint> positions(glyphCount);
        CTRunGetPositions(ctRun, CFRangeMake(0, glyphCount), positions.data());
        skia_private::AutoSTArray<16, CFIndex> indices(glyphCount);
        CTRunGetStringIndices(ctRun, CFRangeMake(0, glyphCount), indices.data());
        for (CFIndex k = 0; k < glyphCount; ++k) {
            if (lastResort) {
                buffer.glyphs[k] = 0;
            }
            // CoreText is y-up, Skia y-down
            const CGFloat x = zeroSize ? 0 : positions[k].x - runOriginX;
            const CGFloat y = zeroSize ? 0 : positions[k].y;
            buffer.positions[k] = buffer.point + SkPoint::Make(SkScalarFromCGFloat(x),
                                                               -SkScalarFromCGFloat(y));
            if (buffer.offsets) {
                buffer.offsets[k] = {0, 0};
            }
            if (buffer.clusters) {
                buffer.clusters[k] = SkToU32(builder.indexMap().toUtf8Index(indices[k]));
            }
        }
        handler->commitRunBuffer(info);
        runOriginX += runWidth;
    }
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

    auto metricsDiffer = [](const TextStyle& a, const TextStyle& b) {
        SkScalar hA = a.getHeightOverride() ? a.getHeight() : 0.0f;
        SkScalar hB = b.getHeightOverride() ? b.getHeight() : 0.0f;
        return hA != hB ||
               a.getHalfLeading() != b.getHalfLeading() ||
               a.getBaselineShift() != b.getBaselineShift();
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
                } else if (metricsDiffer(*currentStyle, blocks[b].fStyle)) {
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
            } else if (!style->getFontFamilies().empty()) {
                // Same clone the attributed string was built from, so LastResort runs and the
                // allowed-font check see the varied face rather than the default instance
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

        font.setEdging(style->getFontEdging());
        font.setHinting(style->getFontHinting());
        font.setSubpixel(style->getSubpixel());

        // CoreText binds the closest real face and never synthesizes a missing bold or italic,
        // so the run gets the same fake flags OneLineShaper derives from the typeface gap
        if (fParagraph->paragraphStyle().fakeMissingFontStyles()) {
            const SkFontStyle wanted = style->getFontStyle();
            const SkFontStyle bound = font.getTypeface()->fontStyle();
            const bool fakeBold = wanted.weight() >= SkFontStyle::kSemiBold_Weight &&
                                  wanted.weight() - bound.weight() >= 200;
            const bool fakeItalic = wanted.slant() == SkFontStyle::kItalic_Slant &&
                                    bound.slant() != SkFontStyle::kItalic_Slant;
            font.setEmbolden(fakeBold);
            font.setSkewX(fakeItalic ? -SK_Scalar1 / 4 : 0);
        }

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

        SkFourByteTag script = detectScript(
            fParagraph->text().data() + runUtf8Range.start, runUtf8Range.width());

        skia_private::AutoSTArray<64, CFIndex> indexBuffer(glyphCount);
        CTRunGetStringIndices(ctRun, CFRangeMake(0, glyphCount), indexBuffer.data());

        skia_private::AutoSTArray<64, SkScalar> glyphAdvances(glyphCount);
        SkScalar runAdvanceX = 0;

        if (isFallback) {
            SkScalar notdefAdvance = font.getWidth(0);
            if (SkScalarNearlyZero(notdefAdvance)) {
                notdefAdvance = font.getSize();
            }

            for (CFIndex k = 0; k < glyphCount; ++k) {
                CFIndex u16Start = indexBuffer[k];
                CFIndex u16End = u16Start;
                if (ctIsRtl) {
                    u16End = (k > 0) ? indexBuffer[k - 1] : (cfRange.location + cfRange.length);
                } else {
                    u16End = (k + 1 < glyphCount) ? indexBuffer[k + 1]
                                                  : (cfRange.location + cfRange.length);
                }
                if (u16Start > u16End) {
                    std::swap(u16Start, u16End);
                }

                TextRange u8GlyphRelRange = builder.indexMap().cfrangeToTextRange(
                    CFRangeMake(u16Start, u16End - u16Start));
                size_t glyphCharStart = regionRange.start + u8GlyphRelRange.start;
                size_t glyphCharEnd = regionRange.start + u8GlyphRelRange.end;

                const char* p = fParagraph->text().data() + glyphCharStart;
                const char* pEnd = fParagraph->text().data() + glyphCharEnd;
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

                size_t u8InRegion = builder.indexMap().toUtf8Index(indexBuffer[k]);
                size_t globalU8 = regionRange.start + u8InRegion;
                if (!fParagraph->codeUnitHasProperty(globalU8,
                                                     SkUnicode::CodeUnitFlags::kControl)) {
                    ++fUnresolvedGlyphs;
                }
                size_t clusterInRun =
                        (globalU8 >= runUtf8Range.start) ? (globalU8 - runUtf8Range.start) : 0;
                run.fClusterIndexes[k] = SkToU32(clusterInRun);
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

                size_t u8InRegion = builder.indexMap().toUtf8Index(indexBuffer[k]);
                size_t globalU8 = regionRange.start + u8InRegion;
                if (run.fGlyphs[k] == 0 && !fParagraph->codeUnitHasProperty(
                                                   globalU8, SkUnicode::CodeUnitFlags::kControl)) {
                    ++fUnresolvedGlyphs;
                }
                size_t clusterInRun =
                        (globalU8 >= runUtf8Range.start) ? (globalU8 - runUtf8Range.start) : 0;
                run.fClusterIndexes[k] = SkToU32(clusterInRun);
            }
        }

        // Trailing N+1 sentinel terminates position and cluster interpolations during line breaking
        // and caret hit-testing
        run.fPositions[glyphCount] = SkPoint::Make(advanceX + runAdvanceX, 0);
        run.fOffsets[glyphCount] = SkPoint::Make(0, 0);
        run.fClusterIndexes[glyphCount] = run.leftToRight() ? SkToU32(runUtf8Range.width()) : 0;

        if (isFallback) {
            fParagraph->addUnresolvedCodepoints(runUtf8Range);
        }

        fParagraph->fFontSwitches.emplace_back(runUtf8Range.start, run.font());
        advanceX += runAdvanceX;
    }

    return true;
}

}  // namespace textlayout
}  // namespace skia

#endif  // defined(SK_PARAGRAPH_NATIVE_CORETEXT)
