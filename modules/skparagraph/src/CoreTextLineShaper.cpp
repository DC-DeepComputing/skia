// Copyright 2026 Google LLC
#include "modules/skparagraph/src/CoreTextLineShaper.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

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
    if (!cfStr || skStr.isEmpty()) {
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
    if (!ctFont || CoreTextLineShaper::isLastResortFont(ctFont)) {
        return false;
    }
    SkUniqueCFRef<CFStringRef> ctPsName(CTFontCopyPostScriptName(ctFont));
    SkUniqueCFRef<CFStringRef> ctFamName(CTFontCopyFamilyName(ctFont));

    for (const auto& tf : allowedTypefaces) {
        if (!tf) {
            continue;
        }
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
    if (!utf8 || len == 0) {
        return SkSetFourByteTag('L', 'a', 't', 'n');
    }
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
    if (!font) {
        return false;
    }
    SkUniqueCFRef<CFStringRef> psName(CTFontCopyPostScriptName(font));
    if (psName) {
        CFRange r = CFStringFind(psName.get(), CFSTR("LastResort"), kCFCompareCaseInsensitive);
        if (r.location != kCFNotFound) {
            return true;
        }
    }
    return false;
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
        if (!this->shapeTextRegion(entireText, advanceX)) {
            return false;
        }
    } else {
        for (auto& placeholder : fParagraph->fPlaceholders) {
            if (placeholder.fTextBefore.width() > 0) {
                if (!this->shapeTextRegion(placeholder.fTextBefore, advanceX)) {
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

            std::vector<sk_sp<SkTypeface>> typefaces = fParagraph->fFontCollection->findTypefaces(
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
                0.0f,
                false,
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
    };
    skia_private::STArray<16, CTRunItem> sortedRuns(runCount);
    for (CFIndex i = 0; i < runCount; ++i) {
        CTRunRef ctRun = (CTRunRef)CFArrayGetValueAtIndex(runs, i);
        CFRange cfRange = CTRunGetStringRange(ctRun);
        sortedRuns.push_back({ctRun, cfRange.location, cfRange.length});
    }

    // CTRuns arrive in visual order from CTLineGetGlyphRuns
    // Logical ascending sort prevents BiDi inversion downstream in cluster layout
    std::stable_sort(sortedRuns.begin(), sortedRuns.end(),
                     [](const CTRunItem& a, const CTRunItem& b) {
                         return a.location < b.location;
                     });

    skia_private::THashMap<CTFontRef, sk_sp<SkTypeface>> typefaceCache;

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

        const TextStyle* style = &fParagraph->paragraphStyle().getTextStyle();
        for (const auto& block : fParagraph->fTextStyles) {
            if (runUtf8Range.start >= block.fRange.start && runUtf8Range.start < block.fRange.end) {
                style = &block.fStyle;
                break;
            }
        }

        sk_sp<SkTypeface> primary = style->refTypeface();
        std::vector<sk_sp<SkTypeface>> allowedTypefaces;
        if (primary) {
            allowedTypefaces.push_back(primary);
        } else if (fParagraph->fFontCollection) {
            allowedTypefaces = fParagraph->fFontCollection->findTypefaces(
                style->getFontFamilies(), style->getFontStyle());
            if (!allowedTypefaces.empty()) {
                primary = allowedTypefaces.front();
            }
        }

        CFDictionaryRef runAttrs = CTRunGetAttributes(ctRun);
        CTFontRef ctFont = (CTFontRef)CFDictionaryGetValue(runAttrs, kCTFontAttributeName);
        bool isLastResort = isLastResortFont(ctFont);
        const bool fallbackDisabled = fParagraph->fFontCollection && !fParagraph->fFontCollection->fontFallbackEnabled();
        const bool isFallback = isLastResort || (fallbackDisabled && !is_allowed_font(ctFont, allowedTypefaces, style->getFontFamilies()));

        CGFloat fontSize = ctFont ? CTFontGetSize(ctFont) : (style->getFontSize() > 0 ? style->getFontSize() : 14.0f);
        SkFont font;
        if (isFallback) {
            font = SkFont(primary, SkScalarFromCGFloat(fontSize));
        } else {
            sk_sp<SkTypeface> typeface;
            if (ctFont) {
                sk_sp<SkTypeface>* cached = typefaceCache.find(ctFont);
                if (cached) {
                    typeface = *cached;
                } else {
                    typeface = SkMakeTypefaceFromCTFont(ctFont);
                    typefaceCache.set(ctFont, typeface);
                }
            }
            font = SkFont(std::move(typeface), SkScalarFromCGFloat(fontSize));
        }

        font.setEdging(style->getFontEdging());
        font.setHinting(style->getFontHinting());
        font.setSubpixel(style->getSubpixel());

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
                    u16End = (k + 1 < glyphCount) ? indexBuffer[k + 1] : (cfRange.location + cfRange.length);
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
                glyphAdvances[k] = SkScalarFromCGFloat(advances[k].width);
                runAdvanceX += glyphAdvances[k];
            }
        }

        SkFontMetrics fontMetrics;
        font.getMetrics(&fontMetrics);
        SkScalar runHeight = style->getHeightOverride() ? style->getHeight() : 0.0f;

        SkString language(style->getLocale().c_str());
        const SkShaper::RunHandler::RunInfo runInfo = {
            font,
            bidiLevel,
            script,
            language.c_str(),
            SkPoint::Make(runAdvanceX, fontMetrics.fDescent - fontMetrics.fAscent + fontMetrics.fLeading),
            static_cast<size_t>(glyphCount),
            SkShaper::RunHandler::Range(0, runUtf8Range.width())
        };

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
            SkScalar accumulatedX = 0;
            for (CFIndex k = 0; k < glyphCount; ++k) {
                run.fGlyphs[k] = 0;
                ++fUnresolvedGlyphs;

                run.fPositions[k] = SkPoint::Make(advanceX + accumulatedX, 0);
                accumulatedX += glyphAdvances[k];
                run.fOffsets[k] = SkPoint::Make(0, 0);

                size_t u8InRegion = builder.indexMap().toUtf8Index(indexBuffer[k]);
                size_t globalU8 = regionRange.start + u8InRegion;
                size_t clusterInRun = (globalU8 >= runUtf8Range.start) ? (globalU8 - runUtf8Range.start) : 0;
                run.fClusterIndexes[k] = SkToU32(clusterInRun);
            }
        } else {
            skia_private::AutoSTArray<64, CGGlyph> glyphBuffer(glyphCount);
            CTRunGetGlyphs(ctRun, CFRangeMake(0, glyphCount), glyphBuffer.data());

            skia_private::AutoSTArray<64, CGPoint> posBuffer(glyphCount);
            CTRunGetPositions(ctRun, CFRangeMake(0, glyphCount), posBuffer.data());

            for (CFIndex k = 0; k < glyphCount; ++k) {
                run.fGlyphs[k] = glyphBuffer[k];
                if (run.fGlyphs[k] == 0) {
                    ++fUnresolvedGlyphs;
                }

                CGFloat relX = posBuffer[k].x - posBuffer[0].x;
                run.fPositions[k] = SkPoint::Make(
                    advanceX + SkScalarFromCGFloat(relX),
                    -SkScalarFromCGFloat(posBuffer[k].y));
                run.fOffsets[k] = SkPoint::Make(0, 0);

                size_t u8InRegion = builder.indexMap().toUtf8Index(indexBuffer[k]);
                size_t globalU8 = regionRange.start + u8InRegion;
                size_t clusterInRun = (globalU8 >= runUtf8Range.start) ? (globalU8 - runUtf8Range.start) : 0;
                run.fClusterIndexes[k] = SkToU32(clusterInRun);
            }
        }

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

#endif  // defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
