// Copyright 2026 Google LLC
#include "modules/skparagraph/src/CoreTextAttrStringBuilder.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#include "include/core/SkPaint.h"
#include "include/core/SkString.h"
#include "src/core/SkUTF.h"

namespace skia {
namespace textlayout {

namespace {

static const CFStringRef kTrackingAttributeName = CFSTR("CTTracking");
static const CFStringRef kBaselineOffsetAttributeName = CFSTR("CTBaselineOffset");

struct AppleHIGTrackingPoint {
    float fontSize;
    float tracking; // 1/1000 em
};

static constexpr AppleHIGTrackingPoint kAppleHIGTrackingText[] = {
    {  6.0f,  41.0f },
    {  7.0f,  34.0f },
    {  8.0f,  26.0f },
    {  9.0f,  19.0f },
    { 10.0f,  12.0f },
    { 11.0f,   6.0f },
    { 12.0f,   0.0f },
    { 13.0f,  -6.0f },
    { 14.0f, -11.0f },
    { 15.0f, -16.0f },
    { 16.0f, -20.0f },
    { 17.0f, -26.0f },
    { 18.0f, -25.0f },
    { 19.0f, -24.0f },
};

static constexpr AppleHIGTrackingPoint kAppleHIGTrackingDisplay[] = {
    { 20.0f,  19.0f },
    { 24.0f,  10.0f },
    { 28.0f,   3.0f },
    { 32.0f,  -2.0f },
    { 36.0f,  -6.0f },
    { 40.0f, -10.0f },
    { 48.0f, -16.0f },
    { 64.0f, -22.0f },
    { 80.0f, -24.0f },
};

template <size_t N>
static float interpolate_tracking_table(const AppleHIGTrackingPoint (&table)[N], float fontSize) {
    if (fontSize <= table[0].fontSize) {
        return table[0].tracking / 1000.0f;
    }
    if (fontSize >= table[N - 1].fontSize) {
        return table[N - 1].tracking / 1000.0f;
    }
    for (size_t i = 0; i < N - 1; ++i) {
        if (fontSize >= table[i].fontSize && fontSize <= table[i + 1].fontSize) {
            float t = (fontSize - table[i].fontSize) / (table[i + 1].fontSize - table[i].fontSize);
            float track = table[i].tracking + t * (table[i + 1].tracking - table[i].tracking);
            return track / 1000.0f;
        }
    }
    return 0.0f;
}

static float get_apple_hig_tracking(float fontSize) {
    if (fontSize < 20.0f) {
        return interpolate_tracking_table(kAppleHIGTrackingText, fontSize);
    }
    return interpolate_tracking_table(kAppleHIGTrackingDisplay, fontSize);
}

static SkUniqueCFRef<CGColorRef> make_cgcolor(SkColor color) {
    CGFloat components[4] = {
        SkColorGetR(color) / 255.0,
        SkColorGetG(color) / 255.0,
        SkColorGetB(color) / 255.0,
        SkColorGetA(color) / 255.0
    };
    SkUniqueCFRef<CGColorSpaceRef> space(CGColorSpaceCreateDeviceRGB());
    return SkUniqueCFRef<CGColorRef>(CGColorCreate(space.get(), components));
}

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

bool CoreTextAttrStringBuilder::isCursiveCodepoint(SkUnichar c) {
    return (c >= 0x0600 && c <= 0x06FF) ||  // Arabic
           (c >= 0x0700 && c <= 0x074F) ||  // Syriac
           (c >= 0x0750 && c <= 0x077F) ||  // Arabic Supplement
           (c >= 0x07C0 && c <= 0x07FF) ||  // NKo
           (c >= 0x0840 && c <= 0x085F) ||  // Mandaic
           (c >= 0x0860 && c <= 0x086F) ||  // Syriac Supplement
           (c >= 0x08A0 && c <= 0x08FF) ||  // Arabic Extended-A
           (c >= 0x1800 && c <= 0x18AF) ||  // Mongolian
           (c >= 0xA840 && c <= 0xA87F) ||  // Phags-pa
           (c >= 0x10B80 && c <= 0x10BAF) || // Manichaean
           (c >= 0x10D00 && c <= 0x10D3F) || // Hanifi Rohingya
           (c >= 0x10E60 && c <= 0x10E7E) || // Rumi Numeral Symbols
           (c >= 0x10F30 && c <= 0x10F6F) || // Sogdian / Old Uyghur
           (c >= 0x11660 && c <= 0x1167F) || // Mongolian Supplement
           (c >= 0x1EC70 && c <= 0x1ECBF) || // Indic Siyaq Numbers
           (c >= 0x1ED00 && c <= 0x1ED4F) || // Ottoman Siyaq Numbers
           (c >= 0x1EE00 && c <= 0x1EEFF) || // Arabic Mathematical Alphabetic Symbols
           (c >= 0xFB50 && c <= 0xFDFF) ||  // Arabic Presentation Forms-A
           (c >= 0xFE70 && c <= 0xFEFF);    // Arabic Presentation Forms-B
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
    }

    if (!cfStr) {
        cfStr.reset(CFStringCreateWithCString(kCFAllocatorDefault, "", kCFStringEncodingUTF8));
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
        if (fFontCollection) {
            typefaces = fFontCollection->findTypefaces(
                span.fStyle.getFontFamilies(), span.fStyle.getFontStyle());
        }
        if (!primary && !typefaces.empty()) {
            primary = typefaces.front();
        }

        CGFloat fontSize = span.fStyle.getFontSize() > 0.0f ? span.fStyle.getFontSize() : 14.0f;
        SkUniqueCFRef<CTFontRef> baseFont =
            CoreTextFontRegistry::Instance().createFont(primary.get(), fontSize);

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

        SkColor color = span.fStyle.hasForeground() ? span.fStyle.getForeground().getColor()
                                                    : span.fStyle.getColor();
        SkUniqueCFRef<CGColorRef> cgColor = make_cgcolor(color);
        if (cgColor) {
            CFAttributedStringSetAttribute(
                attrStr.get(), cfRange, kCTForegroundColorAttributeName, cgColor.get());
        }

        if (span.fStyle.getBaselineShift() != 0.0f) {
            CGFloat shift = span.fStyle.getBaselineShift();
            SkUniqueCFRef<CFNumberRef> shiftNum(
                CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &shift));
            CFAttributedStringSetAttribute(
                attrStr.get(), cfRange, kBaselineOffsetAttributeName, shiftNum.get());
        }

        SkScalar letterSpacing = span.fStyle.getLetterSpacing();
        if (letterSpacing != 0.0f) {
            const char* curr = fUtf8 + span.fRange.start;
            const char* const spanEnd = fUtf8 + span.fRange.end;
            while (curr < spanEnd) {
                const char* prev = curr;
                SkUnichar u = SkUTF::NextUTF8(&curr, spanEnd);
                if (u < 0) {
                    curr = prev + 1;
                    u = 0xFFFD;
                }
                bool isCursive = isCursiveCodepoint(u);
                const char* runStart = prev;
                while (curr < spanEnd) {
                    const char* nextPrev = curr;
                    SkUnichar nextU = SkUTF::NextUTF8(&curr, spanEnd);
                    if (nextU < 0) {
                        curr = nextPrev + 1;
                        nextU = 0xFFFD;
                    }
                    if (isCursiveCodepoint(nextU) != isCursive) {
                        curr = nextPrev;
                        break;
                    }
                }
                const char* runEnd = curr;
                // Inter-character kerning breaks cursive script shaping; suppress kerning for connected scripts
                if (!isCursive && runEnd > runStart) {
                    size_t u8Start = runStart - fUtf8;
                    size_t u8End = runEnd - fUtf8;
                    CFRange kernCfRange = fIndexMap.utf8RangeToCFRange(TextRange(u8Start, u8End));
                    if (kernCfRange.length > 0) {
                        CGFloat kernVal = letterSpacing;
                        SkUniqueCFRef<CFNumberRef> kernNum(
                            CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &kernVal));
                        CFAttributedStringSetAttribute(
                            attrStr.get(), kernCfRange, kCTKernAttributeName, kernNum.get());
                    }
                }
            }
        } else if (CoreTextFontRegistry::isAppleSystemFont(primary.get())) {
            // Apply Apple HIG optical tracking when caller does not specify explicit letter spacing
            float tracking = get_apple_hig_tracking(fontSize) * fontSize;
            if (tracking != 0.0f) {
                CGFloat trackVal = tracking;
                SkUniqueCFRef<CFNumberRef> trackNum(
                    CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &trackVal));
                CFAttributedStringSetAttribute(
                    attrStr.get(), cfRange, kTrackingAttributeName, trackNum.get());
            }
        }
    }

    return SkUniqueCFRef<CFAttributedStringRef>(attrStr.release());
}

}  // namespace textlayout
}  // namespace skia

#endif  // defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
