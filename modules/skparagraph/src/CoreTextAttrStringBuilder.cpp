// Copyright 2026 Google LLC
#include "modules/skparagraph/src/CoreTextAttrStringBuilder.h"

#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)

#include "include/core/SkString.h"
#include "src/core/SkUTF.h"

namespace skia {
namespace textlayout {

namespace {

static const CFStringRef kTrackingAttributeName = CFSTR("CTTracking");

}  // namespace

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
}

void CoreTextAttrStringBuilder::addStyle(TextRange utf8Range, const TextStyle& style) {
    if (utf8Range.start >= utf8Range.end || utf8Range.start >= fUtf8Bytes) {
        return;
    }
    size_t end = std::min(utf8Range.end, fUtf8Bytes);
    fStyles.push_back({TextRange(utf8Range.start, end), style});
}

SkUniqueCFRef<CFAttributedStringRef> CoreTextAttrStringBuilder::build() {
    SkUniqueCFRef<CFStringRef> cfStr(CFStringCreateWithBytes(kCFAllocatorDefault,
                                                             reinterpret_cast<const UInt8*>(fUtf8),
                                                             fUtf8Bytes,
                                                             kCFStringEncodingUTF8,
                                                             false));
    if (!cfStr) {
        SkString sanitized;
        const char* ptr = fUtf8;
        const char* end = fUtf8 + fUtf8Bytes;
        // One U+FFFD per invalid byte, exactly as CoreTextIndexMap counts them, so CFRanges
        // derived from the map stay inside the string
        while (ptr < end) {
            const char* prev = ptr;
            SkUnichar u = SkUTF::NextUTF8(&ptr, end);
            if (u < 0) {
                ptr = prev + 1;
                u = 0xFFFD;
            }
            char utf8Buf[SkUTF::kMaxBytesInUTF8Sequence];
            size_t bytes = SkUTF::ToUTF8(u, utf8Buf);
            sanitized.append(utf8Buf, bytes);
        }
        cfStr.reset(CFStringCreateWithBytes(kCFAllocatorDefault,
                                            reinterpret_cast<const UInt8*>(sanitized.c_str()),
                                            sanitized.size(),
                                            kCFStringEncodingUTF8,
                                            false));
    }

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

    for (const auto& span : fStyles) {
        CFRange cfRange = fIndexMap.utf8RangeToCFRange(span.fRange);
        if (cfRange.length <= 0) {
            continue;
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
        if (!features.empty()) {
            SkUniqueCFRef<CFMutableArrayRef> featureArray(
                CFArrayCreateMutable(kCFAllocatorDefault, features.size(), &kCFTypeArrayCallBacks));
            for (const auto& feat : features) {
                if (feat.fName.size() == 4) {
                    SkUniqueCFRef<CFStringRef> tagStr(CFStringCreateWithCString(
                        kCFAllocatorDefault, feat.fName.c_str(), kCFStringEncodingASCII));
                    int val = feat.fValue;
                    SkUniqueCFRef<CFNumberRef> valNum(
                        CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &val));
                    const void* fKeys[] = {kCTFontOpenTypeFeatureTag, kCTFontOpenTypeFeatureValue};
                    const void* fVals[] = { tagStr.get(), valNum.get() };
                    SkUniqueCFRef<CFDictionaryRef> featDict(CFDictionaryCreate(
                        kCFAllocatorDefault, fKeys, fVals, 2,
                        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
                    CFArrayAppendValue(featureArray.get(), featDict.get());
                }
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

        // Explicit letterSpacing (e.g. CupertinoTextThemeData's HIG values) must replace
        // .SFNS's built-in trak table rather than stack on top of it, while keeping opsz
        if (span.fStyle.getLetterSpacing() != 0.0f) {
            int zero = 0;
            SkUniqueCFRef<CFNumberRef> noTrak(
                CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &zero));
            if (noTrak) {
                CFDictionarySetValue(descAttrs.get(),
                                     CFSTR("NSCTFontUnscaledTrackingAttribute"),
                                     noTrak.get());
            }
        }

        if (CFDictionaryGetCount(descAttrs.get()) > 0) {
            SkUniqueCFRef<CTFontDescriptorRef> featDesc(
                CTFontDescriptorCreateWithAttributes(descAttrs.get()));
            CTFontRef styledFont = CTFontCreateCopyWithAttributes(
                baseFont.get(), fontSize, nullptr, featDesc.get());
            if (styledFont) {
                baseFont.reset(styledFont);
            }
        }

        CFAttributedStringSetAttribute(
            attrStr.get(), cfRange, kCTFontAttributeName, baseFont.get());

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

        // CoreText contract: tracking applies to every script, joining ones included (CoreText
        // keeps the joins with kashida glyphs where the font has them), whereas the HarfBuzz
        // path skips cursive runs in ParagraphImpl::applySpacingAndBuildClusterTable
        SkScalar letterSpacing = span.fStyle.getLetterSpacing();
        if (letterSpacing != 0.0f) {
            CGFloat trackVal = letterSpacing;
            SkUniqueCFRef<CFNumberRef> trackNum(
                CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &trackVal));
            if (trackNum) {
                CFAttributedStringSetAttribute(
                    attrStr.get(), cfRange, kTrackingAttributeName, trackNum.get());
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

#endif  // defined(SK_PARAGRAPH_NATIVE_CORETEXT)
