/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/ports/SkTypeface_mac_symbols.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#ifdef SK_BUILD_FOR_MAC
#import <AppKit/AppKit.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#import <UIKit/UIKit.h>
#endif
#import <objc/message.h>

#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkFixed.h"
#include "include/private/SkOnce.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkScalerContext.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace {

static const std::vector<std::string>& GetSymbolNames() {
    static std::vector<std::string>* sNames = []() {
        auto* vec = new std::vector<std::string>();
        @autoreleasepool {
            NSString* path = @"/System/Library/PrivateFrameworks/SFSymbols.framework/Versions/A/Resources/CoreGlyphs.bundle/Contents/Resources/symbol_order.plist";
            NSArray* arr = [NSArray arrayWithContentsOfFile:path];
            if (!arr) {
                path = @"/System/Library/CoreServices/CoreGlyphs.bundle/symbol_order.plist";
                arr = [NSArray arrayWithContentsOfFile:path];
            }
            if (arr) {
                vec->reserve([arr count]);
                for (NSObject* item in arr) {
                    if ([item isKindOfClass:[NSString class]]) {
                        const char* str = [(NSString*)item UTF8String];
                        if (str) {
                            vec->push_back(str);
                        }
                    }
                }
            }
        }
        return vec;
    }();
    return *sNames;
}

class SingleLocalizedString : public SkTypeface::LocalizedStrings {
public:
    SingleLocalizedString(SkString name, SkString lang)
        : fName(std::move(name)), fLang(std::move(lang)) {}
    bool next(SkTypeface::LocalizedString* localizedString) override {
        if (fDone) return false;
        fDone = true;
        localizedString->fString = fName;
        localizedString->fLanguage = fLang;
        return true;
    }
private:
    SkString fName;
    SkString fLang;
    bool fDone = false;
};

class SkTypeface_SFSymbols;

class SkScalerContext_SFSymbols : public SkScalerContext {
public:
    SkScalerContext_SFSymbols(SkTypeface_SFSymbols& face,
                              const SkScalerContextEffects& effects,
                              const SkDescriptor* desc);

protected:
    SkScalerContext::GlyphMetrics generateMetrics(const SkGlyph& glyph, SkArenaAlloc*) override;
    void generateImage(const SkGlyph& glyph, void* imageBuffer) override;
    void generateFontMetrics(SkFontMetrics* metrics) override;
    std::optional<GeneratedPath> generatePath(const SkGlyph&) override {
        return std::nullopt;
    }
};

class SkTypeface_SFSymbols : public SkTypeface {
public:
    static constexpr SkTypeface::FactoryId FactoryId = SkSetFourByteTag('s','f','s','y');

    static sk_sp<SkTypeface> Make(const SkFontStyle& style) {
        return sk_sp<SkTypeface>(new SkTypeface_SFSymbols(style));
    }

private:
    friend class SkScalerContext_SFSymbols;

    SkTypeface_SFSymbols(const SkFontStyle& style)
        : SkTypeface(style, false) {}

    sk_sp<SkTypeface> onMakeClone(const SkFontArguments&) const override {
        return sk_ref_sp(this);
    }

    std::unique_ptr<SkScalerContext> onCreateScalerContext(
        const SkScalerContextEffects& effects, const SkDescriptor* desc) const override {
        return std::make_unique<SkScalerContext_SFSymbols>(
            *const_cast<SkTypeface_SFSymbols*>(this), effects, desc);
    }

    void onFilterRec(SkScalerContextRec* rec) const override {
        rec->useStrokeForFakeBold();
        rec->setHinting(SkFontHinting::kNone);
    }

    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override {
        return nullptr;
    }

    void getPostScriptGlyphNames(SkString*) const override {}

    void getGlyphToUnicodeMap(SkSpan<SkUnichar> dst) const override {
        const size_t count = GetSymbolNames().size();
        size_t limit = std::min(dst.size(), count + 1);
        if (limit > 0) dst[0] = 0;
        for (size_t gid = 1; gid < limit; ++gid) {
            dst[gid] = kSFSymbolsBaseCodepoint + (SkUnichar)(gid - 1);
        }
    }

    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override {
        return nullptr;
    }

    bool onGlyphMaskNeedsCurrentColor() const override {
        return false;
    }

    int onGetVariationDesignPosition(
        SkSpan<SkFontArguments::VariationPosition::Coordinate>) const override {
        return 0;
    }

    int onGetVariationDesignParameters(
        SkSpan<SkFontParameters::Variation::Axis>) const override {
        return 0;
    }

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const override {
        desc->setFactoryId(FactoryId);
        desc->setFamilyName("SF Symbols");
        desc->setStyle(this->fontStyle());
        *isLocal = true;
    }

    void onCharsToGlyphs(SkSpan<const SkUnichar> chars, SkSpan<SkGlyphID> glyphs) const override {
        const auto& names = GetSymbolNames();
        const size_t count = names.size();
        for (size_t i = 0; i < chars.size(); ++i) {
            SkUnichar c = chars[i];
            if (c >= kSFSymbolsBaseCodepoint && (size_t)(c - kSFSymbolsBaseCodepoint) < count) {
                glyphs[i] = (SkGlyphID)((c - kSFSymbolsBaseCodepoint) + 1);
            } else {
                glyphs[i] = 0;
            }
        }
    }

    int onCountGlyphs() const override {
        return (int)(GetSymbolNames().size() + 1);
    }

    int onGetUPEM() const override { return 1000; }

    bool onComputeBounds(SkRect* bounds) const override {
        bounds->setLTRB(0, -1000, 1000, 200);
        return true;
    }

    void onGetFamilyName(SkString* familyName) const override {
        *familyName = "SF Symbols";
    }

    bool onGetPostScriptName(SkString* psName) const override {
        *psName = "SF-Symbols";
        return true;
    }

    LocalizedStrings* onCreateFamilyNameIterator() const override {
        return new SingleLocalizedString(SkString("SF Symbols"), SkString("en"));
    }

    int onGetTableTags(SkSpan<SkFontTableTag>) const override { return 0; }

    size_t onGetTableData(SkFontTableTag, size_t, size_t, void*) const override {
        return 0;
    }
};

SkScalerContext_SFSymbols::SkScalerContext_SFSymbols(SkTypeface_SFSymbols& face,
                                                     const SkScalerContextEffects& effects,
                                                     const SkDescriptor* desc)
    : SkScalerContext(face, effects, desc) {}

void SkScalerContext_SFSymbols::generateFontMetrics(SkFontMetrics* metrics) {
    if (!metrics) return;
    sk_bzero(metrics, sizeof(*metrics));
    SkScalar size = SkScalarAbs(fRec.fTextSize);
    if (size <= 0) size = 12.0f;
    metrics->fTop = -size * 1.0f;
    metrics->fAscent = -size * 0.8f;
    metrics->fDescent = size * 0.2f;
    metrics->fBottom = size * 0.3f;
    metrics->fLeading = size * 0.1f;
    metrics->fCapHeight = size * 0.7f;
    metrics->fXHeight = size * 0.5f;
}

SkScalerContext::GlyphMetrics SkScalerContext_SFSymbols::generateMetrics(const SkGlyph& glyph, SkArenaAlloc*) {
    GlyphMetrics mx(glyph.maskFormat());
    mx.neverRequestPath = true;

    SkGlyphID gid = glyph.getGlyphID();
    const auto& names = GetSymbolNames();
    if (gid == 0 || gid > names.size()) {
        return mx;
    }

    SkScalar ptSize = SkScalarAbs(fRec.fTextSize);
    if (ptSize <= 0) ptSize = 12.0f;
    int weight = this->getTypeface()->fontStyle().weight();

    @autoreleasepool {
        NSString* name = [NSString stringWithUTF8String:names[gid - 1].c_str()];
#ifdef SK_BUILD_FOR_MAC
        NSImage* baseImg = [NSImage imageWithSystemSymbolName:name accessibilityDescription:nil];
        if (!baseImg) return mx;

        NSFontWeight nsWeight = NSFontWeightRegular;
        if (weight <= 150) nsWeight = NSFontWeightUltraLight;
        else if (weight <= 250) nsWeight = NSFontWeightThin;
        else if (weight <= 350) nsWeight = NSFontWeightLight;
        else if (weight <= 450) nsWeight = NSFontWeightRegular;
        else if (weight <= 550) nsWeight = NSFontWeightMedium;
        else if (weight <= 650) nsWeight = NSFontWeightSemibold;
        else if (weight <= 750) nsWeight = NSFontWeightBold;
        else if (weight <= 850) nsWeight = NSFontWeightHeavy;
        else nsWeight = NSFontWeightBlack;

        NSImageSymbolConfiguration* config =
            [NSImageSymbolConfiguration configurationWithPointSize:ptSize weight:nsWeight];
        NSImage* img = [baseImg imageWithSymbolConfiguration:config];
        if (!img) img = baseImg;

        NSSize size = [img size];
        CGFloat baseline = 0;
        SEL baselineSel = @selector(baselineOffsetFromBottom);
        if ([img respondsToSelector:baselineSel]) {
            baseline = ((CGFloat (*)(id, SEL))objc_msgSend)(img, baselineSel);
        }
        if (baseline <= 0) {
            baseline = size.height * 0.2f;
        }

        mx.advance = {(SkScalar)size.width, 0};
        mx.maskFormat = SkMask::kA8_Format;

        SkRect bounds = SkRect::MakeXYWH(0, -(SkScalar)(size.height - baseline), (SkScalar)size.width, (SkScalar)size.height);
        bounds.offset(SkFixedToScalar(glyph.getSubXFixed()),
                      SkFixedToScalar(glyph.getSubYFixed()));
        bounds.roundOut(&mx.bounds);
#elif defined(SK_BUILD_FOR_IOS)
        UIImageSymbolWeight uiWeight = UIImageSymbolWeightRegular;
        if (weight <= 150) uiWeight = UIImageSymbolWeightUltraLight;
        else if (weight <= 250) uiWeight = UIImageSymbolWeightThin;
        else if (weight <= 350) uiWeight = UIImageSymbolWeightLight;
        else if (weight <= 450) uiWeight = UIImageSymbolWeightRegular;
        else if (weight <= 550) uiWeight = UIImageSymbolWeightMedium;
        else if (weight <= 650) uiWeight = UIImageSymbolWeightSemibold;
        else if (weight <= 750) uiWeight = UIImageSymbolWeightBold;
        else if (weight <= 850) uiWeight = UIImageSymbolWeightHeavy;
        else uiWeight = UIImageSymbolWeightBlack;

        UIImageConfiguration* config =
            [UIImageSymbolConfiguration configurationWithPointSize:ptSize weight:uiWeight];
        UIImage* img = [UIImage systemImageNamed:name withConfiguration:config];
        if (!img) return mx;

        CGSize size = [img size];
        CGFloat baseline = [img baselineOffsetFromBottom];
        if (baseline <= 0) {
            baseline = size.height * 0.2f;
        }

        mx.advance = {(SkScalar)size.width, 0};
        mx.maskFormat = SkMask::kA8_Format;

        SkRect bounds = SkRect::MakeXYWH(0, -(SkScalar)(size.height - baseline), (SkScalar)size.width, (SkScalar)size.height);
        bounds.offset(SkFixedToScalar(glyph.getSubXFixed()),
                      SkFixedToScalar(glyph.getSubYFixed()));
        bounds.roundOut(&mx.bounds);
#endif
    }
    return mx;
}

void SkScalerContext_SFSymbols::generateImage(const SkGlyph& glyph, void* imageBuffer) {
    SkGlyphID gid = glyph.getGlyphID();
    const auto& names = GetSymbolNames();
    if (gid == 0 || gid > names.size() || !imageBuffer) return;

    int width = glyph.width();
    int height = glyph.height();
    if (width <= 0 || height <= 0) return;

    SkScalar ptSize = SkScalarAbs(fRec.fTextSize);
    if (ptSize <= 0) ptSize = 12.0f;
    int weight = this->getTypeface()->fontStyle().weight();

    @autoreleasepool {
        NSString* name = [NSString stringWithUTF8String:names[gid - 1].c_str()];
        CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
        CGContextRef cg = CGBitmapContextCreate(nullptr, width, height, 8, width * 4, cs,
                                                kCGImageAlphaPremultipliedLast);
        CGColorSpaceRelease(cs);
        if (!cg) return;

#ifdef SK_BUILD_FOR_MAC
        NSImage* baseImg = [NSImage imageWithSystemSymbolName:name accessibilityDescription:nil];
        if (baseImg) {
            NSFontWeight nsWeight = NSFontWeightRegular;
            if (weight <= 150) nsWeight = NSFontWeightUltraLight;
            else if (weight <= 250) nsWeight = NSFontWeightThin;
            else if (weight <= 350) nsWeight = NSFontWeightLight;
            else if (weight <= 450) nsWeight = NSFontWeightRegular;
            else if (weight <= 550) nsWeight = NSFontWeightMedium;
            else if (weight <= 650) nsWeight = NSFontWeightSemibold;
            else if (weight <= 750) nsWeight = NSFontWeightBold;
            else if (weight <= 850) nsWeight = NSFontWeightHeavy;
            else nsWeight = NSFontWeightBlack;

            NSImageSymbolConfiguration* config =
                [NSImageSymbolConfiguration configurationWithPointSize:ptSize weight:nsWeight];
            NSImage* img = [baseImg imageWithSymbolConfiguration:config];
            if (!img) img = baseImg;

            [NSGraphicsContext saveGraphicsState];
            [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithCGContext:cg flipped:NO]];
            [img drawInRect:NSMakeRect(0, 0, width, height)];
            [NSGraphicsContext restoreGraphicsState];
        }
#elif defined(SK_BUILD_FOR_IOS)
        UIImageSymbolWeight uiWeight = UIImageSymbolWeightRegular;
        if (weight <= 150) uiWeight = UIImageSymbolWeightUltraLight;
        else if (weight <= 250) uiWeight = UIImageSymbolWeightThin;
        else if (weight <= 350) uiWeight = UIImageSymbolWeightLight;
        else if (weight <= 450) uiWeight = UIImageSymbolWeightRegular;
        else if (weight <= 550) uiWeight = UIImageSymbolWeightMedium;
        else if (weight <= 650) uiWeight = UIImageSymbolWeightSemibold;
        else if (weight <= 750) uiWeight = UIImageSymbolWeightBold;
        else if (weight <= 850) uiWeight = UIImageSymbolWeightHeavy;
        else uiWeight = UIImageSymbolWeightBlack;

        UIImageConfiguration* config =
            [UIImageSymbolConfiguration configurationWithPointSize:ptSize weight:uiWeight];
        UIImage* img = [UIImage systemImageNamed:name withConfiguration:config];
        if (img) {
            UIGraphicsPushContext(cg);
            [img drawInRect:CGRectMake(0, 0, width, height)];
            UIGraphicsPopContext();
        }
#endif

        const uint8_t* rgba = (const uint8_t*)CGBitmapContextGetData(cg);
        uint8_t* dst = static_cast<uint8_t*>(imageBuffer);
        for (int y = 0; y < height; ++y) {
            uint8_t* dstRow = dst + y * glyph.rowBytes();
            const uint8_t* srcRow = rgba + (height - 1 - y) * (width * 4);
            for (int x = 0; x < width; ++x) {
                dstRow[x] = srcRow[x * 4 + 3];
            }
        }
        CGContextRelease(cg);
    }
}

} // namespace

bool SkIsSFSymbolsCodepoint(SkUnichar codepoint) {
    if (codepoint < kSFSymbolsBaseCodepoint) return false;
    size_t idx = (size_t)(codepoint - kSFSymbolsBaseCodepoint);
    return idx < GetSymbolNames().size();
}

size_t SkGetSFSymbolsCount() {
    return GetSymbolNames().size();
}

const char* SkGetSFSymbolName(SkUnichar codepoint) {
    if (!SkIsSFSymbolsCodepoint(codepoint)) return nullptr;
    return GetSymbolNames()[codepoint - kSFSymbolsBaseCodepoint].c_str();
}

sk_sp<SkTypeface> SkMakeTypeface_SFSymbols(const SkFontStyle& style) {
    return SkTypeface_SFSymbols::Make(style);
}

#endif // SK_BUILD_FOR_MAC || SK_BUILD_FOR_IOS
