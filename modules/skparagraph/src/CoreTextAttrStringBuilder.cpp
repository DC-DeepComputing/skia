// Copyright 2026 Google LLC
#include "modules/skparagraph/src/CoreTextAttrStringBuilder.h"

#include "include/core/SkString.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "src/core/SkUTF.h"

#include <algorithm>

namespace skia {
namespace textlayout {

namespace {

static const CFStringRef kStyleGroupAttributeName = CFSTR("SkParagraphStyleGroup");

// Styles that share all font, metric, rasterization, and shaping attributes can coalesce into
// one CFAttributedString style span so paint-only boundaries (color, decoration, shadows) do not
// split CTRuns or ligatures
bool can_coalesce_styles(const TextStyle& a, const TextStyle& b) {
    return a.matchOneAttribute(StyleType::kFont, b) &&
           a.getTypeface() == b.getTypeface() &&
           a.getFontFeatures() == b.getFontFeatures() &&
           SkScalarNearlyEqual(a.getLetterSpacing(), b.getLetterSpacing());
}

// The font restyled by attrs, or font itself when there is nothing to apply or CoreText
// rejects the descriptor
SkUniqueCFRef<CTFontRef> copy_with_attributes(CTFontRef font,
                                              CGFloat fontSize,
                                              CFDictionaryRef attrs) {
    SkASSERT(font);
    if (CFDictionaryGetCount(attrs) > 0) {
        SkUniqueCFRef<CTFontDescriptorRef> desc(CTFontDescriptorCreateWithAttributes(attrs));
        CTFontRef styled = CTFontCreateCopyWithAttributes(font, fontSize, nullptr, desc.get());
        if (styled) {
            return SkUniqueCFRef<CTFontRef>(styled);
        }
    }
    return SkUniqueCFRef<CTFontRef>((CTFontRef)CFRetain(font));
}

}  // namespace

SkFourByteTag CoreTextAttrStringBuilder::ResolveCodepointScript(SkUnichar c) {
    // Map Unicode script blocks used for OpenType/CoreText script itemization; return 0 for
    // Common and Inherited code points so they inherit the surrounding run's script
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

CoreTextAttrStringBuilder::CoreTextAttrStringBuilder(
        const char* utf8, size_t utf8Bytes,
        TextDirection baseDirection,
        sk_sp<FontCollection> fontCollection)
    : fUtf8(utf8)
    , fUtf8Bytes(utf8Bytes)
    , fBaseDirection(baseDirection)
    , fFontCollection(std::move(fontCollection))
    , fIndexMap(utf8, utf8Bytes) {
    SkASSERT(fUtf8);
    SkASSERT(fFontCollection);

    SkFourByteTag current = 0;
    size_t runStart = 0;
    const char* ptr = fUtf8;
    const char* end = fUtf8 + fUtf8Bytes;
    // Every step consumes at least one byte, invalid ones included
    while (ptr < end) {
        const char* prev = ptr;
        SkUnichar u = SkUTF::NextUTF8(&ptr, end);
        if (u < 0) {
            ptr = prev + 1;
            continue;
        }
        SkFourByteTag script = ResolveCodepointScript(u);
        if (script == 0 || script == current) {
            continue;
        }
        if (current != 0) {
            size_t pos = prev - fUtf8;
            fScriptRuns.push_back({TextRange(runStart, pos), current});
            runStart = pos;
        }
        current = script;
    }
    fScriptRuns.push_back({TextRange(runStart, fUtf8Bytes),
                           current != 0 ? current : SkSetFourByteTag('L', 'a', 't', 'n')});
}

SkFourByteTag CoreTextAttrStringBuilder::scriptAt(size_t utf8Index) const {
    SkASSERT(!fScriptRuns.empty());
    for (const ScriptRun& run : fScriptRuns) {
        if (utf8Index < run.fRange.end) {
            return run.fScript;
        }
    }
    return fScriptRuns.back().fScript;
}

void CoreTextAttrStringBuilder::addStyle(TextRange utf8Range, const TextStyle& style) {
    if (utf8Range.start >= utf8Range.end || utf8Range.start >= fUtf8Bytes) {
        return;
    }
    size_t end = std::min(utf8Range.end, fUtf8Bytes);
    // Merge adjacent spans that differ only in paint attributes so ligatures and kerning are
    // preserved across paint-only Block boundaries
    if (!fStyles.empty() &&
        fStyles.back().fRange.end == utf8Range.start &&
        can_coalesce_styles(fStyles.back().fStyle, style)) {
        fStyles.back().fRange.end = end;
        return;
    }
    fStyles.push_back({TextRange(utf8Range.start, end), style});
}

SkUniqueCFRef<CFAttributedStringRef> CoreTextAttrStringBuilder::build() {
    // CFStringCreateWithBytes drops a leading U+FEFF from UTF-8 as a byte order mark, which
    // would shift every CFRange from CoreTextIndexMap; UTF-16 input keeps it as a character.
    // One U+FFFD per invalid byte, exactly as CoreTextIndexMap counts them
    const size_t utf16Length = fIndexMap.toUtf16Index(fUtf8Bytes);
    skia_private::AutoSTArray<256, uint16_t> utf16(utf16Length);
    size_t utf16Pos = 0;
    const char* ptr = fUtf8;
    const char* end = fUtf8 + fUtf8Bytes;
    while (ptr < end) {
        const char* prev = ptr;
        SkUnichar u = SkUTF::NextUTF8(&ptr, end);
        if (u < 0) {
            ptr = prev + 1;
            u = 0xFFFD;
        }
        SkASSERT(utf16Pos + SkUTF::ToUTF16(u) <= utf16Length);
        utf16Pos += SkUTF::ToUTF16(u, utf16.data() + utf16Pos);
    }
    SkASSERT(utf16Pos == utf16Length);
    SkUniqueCFRef<CFStringRef> cfStr(CFStringCreateWithCharacters(
            kCFAllocatorDefault, reinterpret_cast<const UniChar*>(utf16.data()),
            SkTo<CFIndex>(utf16Length)));
    if (!cfStr) {
        return nullptr;
    }

    CFIndex utf16Len = CFStringGetLength(cfStr.get());
    SkUniqueCFRef<CFMutableAttributedStringRef> attrStr(
        CFAttributedStringCreateMutable(kCFAllocatorDefault, utf16Len));
    CFAttributedStringReplaceString(attrStr.get(), CFRangeMake(0, 0), cfStr.get());

    if (utf16Len == 0) {
        return SkUniqueCFRef<CFAttributedStringRef>(attrStr.release());
    }

    CFRange fullRange = CFRangeMake(0, utf16Len);

    // Base writing direction controls CoreText paragraph-level BiDi embedding
    CTWritingDirection dir = (fBaseDirection == TextDirection::kRtl)
        ? kCTWritingDirectionRightToLeft
        : kCTWritingDirectionLeftToRight;
    CTParagraphStyleSetting dirSetting = {
        kCTParagraphStyleSpecifierBaseWritingDirection,
        sizeof(CTWritingDirection),
        &dir
    };
    SkUniqueCFRef<CTParagraphStyleRef> pStyle(CTParagraphStyleCreate(&dirSetting, 1));
    if (pStyle) {
        CFAttributedStringSetAttribute(
            attrStr.get(), fullRange, kCTParagraphStyleAttributeName, pStyle.get());
    }

    for (int spanIdx = 0; spanIdx < fStyles.size(); ++spanIdx) {
        const auto& span = fStyles[spanIdx];
        CFRange cfRange = fIndexMap.utf8RangeToCFRange(span.fRange);
        if (cfRange.length <= 0) {
            continue;
        }
        // Tag each non-coalesced span with a distinct attribute value so CoreText splits CTRuns at
        // span boundaries even when two spans resolve to the same CTFont (e.g. LastResort fallback
        // or differing SkFont edging/baselineShift)
        if (spanIdx > 0) {
            SkUniqueCFRef<CFNumberRef> groupNum(
                CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &spanIdx));
            if (groupNum) {
                CFAttributedStringSetAttribute(
                    attrStr.get(), cfRange, kStyleGroupAttributeName, groupNum.get());
            }
        }

        sk_sp<SkTypeface> primary = span.fStyle.refTypeface();
        std::vector<sk_sp<SkTypeface>> typefaces;
        const std::optional<FontArguments>& fontArgs = span.fStyle.getFontArguments();
        // Empty font families must bypass FontCollection fallback to allow CoreText to bind
        // .AppleSystemUIFont
        if (!primary && !span.fStyle.getFontFamilies().empty()) {
            // Clones carry the variation in their CTFontDescriptor, so the primary font and
            // every cascade entry below shape with the same axes as the HarfBuzz path
            typefaces = CoreTextFontRegistry::Instance().findTypefaces(
                    *fFontCollection,
                    span.fStyle.getFontFamilies(),
                    span.fStyle.getFontStyle(),
                    fontArgs);
            if (!typefaces.empty()) {
                primary = typefaces.front();
            }
        }

        CGFloat fontSize = span.fStyle.getFontSize() > 0.0f ? span.fStyle.getFontSize() : 14.0f;
        SkUniqueCFRef<CTFontRef> baseFont = CoreTextFontRegistry::Instance().createFont(
                primary.get(), span.fStyle.getFontStyle(), fontSize);

        SkUniqueCFRef<CFMutableDictionaryRef> descAttrs(
            CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks));

        // Cascade list provides font fallback chain for characters outside primary typeface
        const bool fallbackDisabled = !fFontCollection->fontFallbackEnabled();
        if (typefaces.size() > 1 || fallbackDisabled) {
            SkUniqueCFRef<CFMutableArrayRef> cascadeList(
                CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks));
            for (size_t i = 1; i < typefaces.size(); ++i) {
                SkUniqueCFRef<CTFontDescriptorRef> desc =
                        CoreTextFontRegistry::Instance().getDescriptor(typefaces[i].get());
                if (desc) {
                    CFArrayAppendValue(cascadeList.get(), desc.get());
                }
            }
            if (fallbackDisabled) {
                SkUniqueCFRef<CTFontRef> lrFont(
                        CTFontCreateWithName(CFSTR("LastResort"), fontSize, nullptr));
                if (lrFont) {
                    SkUniqueCFRef<CTFontDescriptorRef> lrDesc(
                            CTFontCopyFontDescriptor(lrFont.get()));
                    if (lrDesc) {
                        CFArrayAppendValue(cascadeList.get(), lrDesc.get());
                    }
                }
            }
            if (CFArrayGetCount(cascadeList.get()) > 0) {
                CFDictionarySetValue(
                        descAttrs.get(), kCTFontCascadeListAttribute, cascadeList.get());
            }
        }

        // CoreText expects 4-character OpenType feature tags packed into descriptor settings
        const auto& features = span.fStyle.getFontFeatures();
        const SkScalar letterSpacing = span.fStyle.getLetterSpacing();
        // letterSpacing needs feature settings even without user features; the extra array
        // slot holds its liga=0
        if (!features.empty() || letterSpacing != 0) {
            SkUniqueCFRef<CFMutableArrayRef> featureArray(
                CFArrayCreateMutable(kCFAllocatorDefault, features.size() + 1,
                                     &kCFTypeArrayCallBacks));
            auto appendFeature = [&](const char* tag, int val) {
                SkUniqueCFRef<CFStringRef> tagStr(
                    CFStringCreateWithCString(kCFAllocatorDefault, tag, kCFStringEncodingASCII));
                SkUniqueCFRef<CFNumberRef> valNum(
                    CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &val));
                const void* fKeys[] = {kCTFontOpenTypeFeatureTag, kCTFontOpenTypeFeatureValue};
                const void* fVals[] = { tagStr.get(), valNum.get() };
                SkUniqueCFRef<CFDictionaryRef> featDict(CFDictionaryCreate(
                    kCFAllocatorDefault, fKeys, fVals, 2,
                    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
                CFArrayAppendValue(featureArray.get(), featDict.get());
            };
            for (const auto& feat : features) {
                if (feat.fName.size() == 4) {
                    appendFeature(feat.fName.c_str(), feat.fValue);
                }
            }
            // KEEPER-DEBT: OneLineShaper keeps liga for negative letterSpacing; native tracking and
            // CSS Text 3 drop optional ligatures for any nonzero spacing. Remove once it uses != 0.
            // Spacing goes between clusters, so a ligature would hide it; liga goes off for the
            // whole block after the user features, cursive scripts included, as in OneLineShaper
            if (letterSpacing != 0) {
                appendFeature("liga", 0);
            }
            if (CFArrayGetCount(featureArray.get()) > 0) {
                CFDictionarySetValue(
                        descAttrs.get(), kCTFontFeatureSettingsAttribute, featureArray.get());
            }
        }

        // The registry rebuilds system fonts by name and drops cloned variations, so only the
        // requested axes are re-applied: CoreText merges them and keeps deriving opsz from the
        // point size, which a full design position would pin. Palette overrides have no
        // CoreText equivalent and stay HarfBuzz-only
        if (fontArgs.has_value() && !fontArgs->getVariationCoordinates().empty()) {
            SkUniqueCFRef<CFMutableDictionaryRef> variation(
                CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                          &kCFTypeDictionaryKeyCallBacks,
                                          &kCFTypeDictionaryValueCallBacks));
            // css-fonts-4: the last value given for an axis wins, which SetValue provides
            for (const auto& coord : fontArgs->getVariationCoordinates()) {
                int64_t axisTag = coord.axis;
                double axisValue = coord.value;
                SkUniqueCFRef<CFNumberRef> tagNum(
                    CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &axisTag));
                SkUniqueCFRef<CFNumberRef> valNum(
                    CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &axisValue));
                if (tagNum && valNum) {
                    CFDictionarySetValue(variation.get(), tagNum.get(), valNum.get());
                }
            }
            if (CFDictionaryGetCount(variation.get()) > 0) {
                CFDictionarySetValue(descAttrs.get(), kCTFontVariationAttribute, variation.get());
            }
        }

        SkUniqueCFRef<CTFontRef> font =
                copy_with_attributes(baseFont.get(), fontSize, descAttrs.get());
        CFAttributedStringSetAttribute(attrStr.get(), cfRange, kCTFontAttributeName, font.get());

        // Han unification leaves one code point per ideograph, so CoreText picks the regional
        // cascade (and locl forms) from the string language; without the tag it follows the
        // machine's preferred languages, unlike the HarfBuzz path where
        // FontCollection::defaultFallback already receives this locale
        SkString locale = span.fStyle.getLocale();
        if (!locale.isEmpty()) {
            SkUniqueCFRef<CFStringRef> language(CFStringCreateWithCString(
                kCFAllocatorDefault, locale.c_str(), kCFStringEncodingUTF8));
            if (language) {
                CFAttributedStringSetAttribute(
                    attrStr.get(), cfRange, kCTLanguageAttributeName, language.get());
            }
        }

        // Run::fBaselineShift is the sole carrier of the style's baselineShift: TextLine paints
        // each run at baseline + shift, so a kCTBaselineOffsetAttributeName (y-up) would land in
        // glyph fY with the opposite sign and cancel the visual shift
    }

    return SkUniqueCFRef<CFAttributedStringRef>(attrStr.release());
}

}  // namespace textlayout
}  // namespace skia
