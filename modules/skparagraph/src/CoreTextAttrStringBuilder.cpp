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
}

void CoreTextAttrStringBuilder::addStyle(TextRange utf8Range, const TextStyle& style) {
    if (utf8Range.start >= utf8Range.end || utf8Range.start >= fUtf8Bytes) {
        return;
    }
    size_t end = std::min(utf8Range.end, fUtf8Bytes);
    fStyles.push_back({TextRange(utf8Range.start, end), style});
}

SkUniqueCFRef<CFAttributedStringRef> CoreTextAttrStringBuilder::build() {
    SkUniqueCFRef<CFStringRef> cfStr;
    if (fUtf8Bytes == 0 || !fUtf8) {
        cfStr.reset(CFStringCreateWithCString(kCFAllocatorDefault, "", kCFStringEncodingUTF8));
    } else {
        cfStr.reset(CFStringCreateWithBytes(
            kCFAllocatorDefault,
            reinterpret_cast<const UInt8*>(fUtf8),
            fUtf8Bytes,
            kCFStringEncodingUTF8,
            false));
        if (!cfStr) {
            SkString sanitized;
            const char* ptr = fUtf8;
            const char* end = fUtf8 + fUtf8Bytes;
            while (ptr < end) {
                SkUnichar u = SkUTF::NextUTF8WithReplacement(&ptr, end);
                char utf8Buf[SkUTF::kMaxBytesInUTF8Sequence];
                size_t bytes = SkUTF::ToUTF8(u, utf8Buf);
                sanitized.append(utf8Buf, bytes);
            }
            cfStr.reset(CFStringCreateWithBytes(
                kCFAllocatorDefault,
                reinterpret_cast<const UInt8*>(sanitized.c_str()),
                sanitized.size(),
                kCFStringEncodingUTF8,
                false));
        }
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
        // Empty font families must bypass FontCollection fallback to allow CoreText to bind .AppleSystemUIFont
        if (fFontCollection && !span.fStyle.getFontFamilies().empty()) {
            typefaces = fFontCollection->findTypefaces(
                span.fStyle.getFontFamilies(), span.fStyle.getFontStyle());
        }
        if (!primary && !typefaces.empty()) {
            primary = typefaces.front();
        }

        CGFloat fontSize = span.fStyle.getFontSize() > 0.0f ? span.fStyle.getFontSize() : 14.0f;
        SkUniqueCFRef<CTFontRef> baseFont =
            CoreTextFontRegistry::Instance().createFont(primary.get(), span.fStyle.getFontStyle(), fontSize);

        SkUniqueCFRef<CFMutableDictionaryRef> descAttrs(
            CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks));

        // Cascade list provides font fallback chain for characters outside primary typeface
        const bool fallbackDisabled = fFontCollection && !fFontCollection->fontFallbackEnabled();
        if (typefaces.size() > 1 || fallbackDisabled) {
            SkUniqueCFRef<CFMutableArrayRef> cascadeList(
                CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks));
            for (size_t i = 1; i < typefaces.size(); ++i) {
                if (typefaces[i]) {
                    SkUniqueCFRef<CTFontDescriptorRef> desc =
                        CoreTextFontRegistry::Instance().getDescriptor(typefaces[i].get());
                    if (desc) {
                        CFArrayAppendValue(cascadeList.get(), desc.get());
                    }
                }
            }
            if (fallbackDisabled) {
                SkUniqueCFRef<CTFontRef> lrFont(CTFontCreateWithName(CFSTR("LastResort"), fontSize, nullptr));
                if (lrFont) {
                    SkUniqueCFRef<CTFontDescriptorRef> lrDesc(CTFontCopyFontDescriptor(lrFont.get()));
                    if (lrDesc) {
                        CFArrayAppendValue(cascadeList.get(), lrDesc.get());
                    }
                }
            }
            if (CFArrayGetCount(cascadeList.get()) > 0) {
                CFDictionarySetValue(descAttrs.get(), kCTFontCascadeListAttribute, cascadeList.get());
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
                    const void* fKeys[] = { kCTFontOpenTypeFeatureTag, kCTFontOpenTypeFeatureValue };
                    const void* fVals[] = { tagStr.get(), valNum.get() };
                    SkUniqueCFRef<CFDictionaryRef> featDict(CFDictionaryCreate(
                        kCFAllocatorDefault, fKeys, fVals, 2,
                        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
                    CFArrayAppendValue(featureArray.get(), featDict.get());
                }
            }
            if (CFArrayGetCount(featureArray.get()) > 0) {
                CFDictionarySetValue(descAttrs.get(), kCTFontFeatureSettingsAttribute, featureArray.get());
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

        SkScalar baselineShift = span.fStyle.getBaselineShift();
        if (baselineShift != 0.0f) {
            CGFloat shiftVal = baselineShift;
            SkUniqueCFRef<CFNumberRef> shiftNum(
                CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &shiftVal));
            if (shiftNum) {
                CFAttributedStringSetAttribute(
                    attrStr.get(), cfRange, kCTBaselineOffsetAttributeName, shiftNum.get());
            }
        }
    }

    return SkUniqueCFRef<CFAttributedStringRef>(attrStr.release());
}

}  // namespace textlayout
}  // namespace skia

#endif  // defined(SK_PARAGRAPH_NATIVE_CORETEXT)
