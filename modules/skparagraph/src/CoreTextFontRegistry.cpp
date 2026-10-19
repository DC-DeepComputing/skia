// Copyright 2026 Google LLC
#include "modules/skparagraph/src/CoreTextFontRegistry.h"

#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/ports/SkTypeface_mac.h"
#include "src/ports/SkTypeface_mac_ct.h"
#include "src/utils/mac/SkUniqueCFRef.h"

namespace skia {
namespace textlayout {

static SkUniqueCFRef<CFDictionaryRef> make_traits_dict(const SkFontStyle& style) {
    SkUniqueCFRef<CFMutableDictionaryRef> cfTraits(
        CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                  &kCFTypeDictionaryKeyCallBacks,
                                  &kCFTypeDictionaryValueCallBacks));
    if (!cfTraits) {
        return nullptr;
    }
    CGFloat ctWeight = SkCTFontCTWeightForCSSWeight(style.weight());
    SkUniqueCFRef<CFNumberRef> cfFontWeight(
        CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &ctWeight));
    if (cfFontWeight) {
        CFDictionaryAddValue(cfTraits.get(), kCTFontWeightTrait, cfFontWeight.get());
    }

    static const CGFloat kSystemFontItalicSlope = 0.07;
    CGFloat ctSlant = (style.slant() == SkFontStyle::kUpright_Slant) ? 0.0 : kSystemFontItalicSlope;
    SkUniqueCFRef<CFNumberRef> cfFontSlant(
        CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &ctSlant));
    if (cfFontSlant) {
        CFDictionaryAddValue(cfTraits.get(), kCTFontSlantTrait, cfFontSlant.get());
    }

    if (style.slant() != SkFontStyle::kUpright_Slant) {
        uint32_t symTraits = kCTFontTraitItalic;
        SkUniqueCFRef<CFNumberRef> cfSym(
            CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &symTraits));
        if (cfSym) {
            CFDictionaryAddValue(cfTraits.get(), kCTFontSymbolicTrait, cfSym.get());
        }
    }
    return SkUniqueCFRef<CFDictionaryRef>(cfTraits.release());
}

static SkUniqueCFRef<CTFontDescriptorRef> make_traits_descriptor(const SkFontStyle& style) {
    SkUniqueCFRef<CFDictionaryRef> traits = make_traits_dict(style);
    if (!traits) {
        return nullptr;
    }
    const void* keys[] = { kCTFontTraitsAttribute };
    const void* values[] = { traits.get() };
    SkUniqueCFRef<CFDictionaryRef> attrs(
        CFDictionaryCreate(kCFAllocatorDefault, keys, values, 1,
                           &kCFTypeDictionaryKeyCallBacks,
                           &kCFTypeDictionaryValueCallBacks));
    if (!attrs) {
        return nullptr;
    }
    return SkUniqueCFRef<CTFontDescriptorRef>(
        CTFontDescriptorCreateWithAttributes(attrs.get()));
}

// .AppleSystemUIFont triggers system cascade lists and optical size selection (Text vs Display)
static SkUniqueCFRef<CTFontRef> create_system_ui_font(CGFloat size) {
    CTFontRef font = CTFontCreateWithName(CFSTR(".AppleSystemUIFont"), size, nullptr);
    if (font) {
        return SkUniqueCFRef<CTFontRef>(font);
    }
    return SkUniqueCFRef<CTFontRef>(
        CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size, nullptr));
}

CoreTextFontRegistry& CoreTextFontRegistry::Instance() {
    static CoreTextFontRegistry registry;
    return registry;
}

SkUniqueCFRef<CTFontDescriptorRef> CoreTextFontRegistry::getDescriptor(SkTypeface* typeface) {
    if (!typeface) {
        SkUniqueCFRef<CTFontRef> sysFont = create_system_ui_font(0.0);
        if (sysFont) {
            CTFontDescriptorRef desc = CTFontCopyFontDescriptor(sysFont.get());
            return SkUniqueCFRef<CTFontDescriptorRef>(desc);
        }
        return nullptr;
    }

    SkTypefaceID id = typeface->uniqueID();
    SkAutoMutexExclusive lock(fMutex);
    if (auto* found = fDescriptorCache.find(id)) {
        CFRetain(found->get());
        return SkUniqueCFRef<CTFontDescriptorRef>(found->get());
    }

    SkUniqueCFRef<CTFontDescriptorRef> desc = this->createDescriptorInternal(typeface);
    if (desc) {
        CFRetain(desc.get());
        fDescriptorCache.set(id, SkUniqueCFRef<CTFontDescriptorRef>(desc.get()));
    }
    return desc;
}

SkUniqueCFRef<CTFontDescriptorRef> CoreTextFontRegistry::createDescriptorInternal(SkTypeface* typeface) {
    // Apple system UI font requires dynamic instantiation to resolve optical sizing and system cascade
    if (isAppleSystemFont(typeface)) {
        SkUniqueCFRef<CTFontRef> sysFont = create_system_ui_font(0.0);
        if (sysFont) {
            if (typeface) {
                SkUniqueCFRef<CTFontDescriptorRef> traitsDesc = make_traits_descriptor(typeface->fontStyle());
                if (traitsDesc) {
                    CTFontRef styled = CTFontCreateCopyWithAttributes(
                        sysFont.get(), 0.0, nullptr, traitsDesc.get());
                    if (styled) {
                        sysFont.reset(styled);
                    }
                }
            }
            CTFontDescriptorRef desc = CTFontCopyFontDescriptor(sysFont.get());
            return SkUniqueCFRef<CTFontDescriptorRef>(desc);
        }
    }

    CTFontRef ctfont = SkTypeface_GetCTFontRef(typeface);
    if (ctfont) {
        return SkUniqueCFRef<CTFontDescriptorRef>(CTFontCopyFontDescriptor(ctfont));
    }

    int ttcIndex = 0;
    std::unique_ptr<SkStreamAsset> stream = typeface->openStream(&ttcIndex);
    if (stream && stream->hasLength()) {
        size_t len = stream->getLength();
        if (len > 0) {
            sk_sp<SkData> data = SkData::MakeUninitialized(len);
            if (stream->read(data->writable_data(), len) == len) {
                SkUniqueCFRef<CFDataRef> cfData(
                    CFDataCreate(kCFAllocatorDefault, (const UInt8*)data->bytes(), data->size()));
                if (cfData) {
                    // CTFontManagerCreateFontDescriptorsFromData creates in-memory descriptors without fontd IPC roundtrips
                    SkUniqueCFRef<CFArrayRef> descs(
                        CTFontManagerCreateFontDescriptorsFromData(cfData.get()));
                    if (descs && CFArrayGetCount(descs.get()) > 0) {
                        CFIndex count = CFArrayGetCount(descs.get());
                        CFIndex idx = (ttcIndex >= 0 && ttcIndex < count) ? ttcIndex : 0;
                        CTFontDescriptorRef d = (CTFontDescriptorRef)CFArrayGetValueAtIndex(descs.get(), idx);
                        if (d) {
                            CFRetain(d);
                            return SkUniqueCFRef<CTFontDescriptorRef>(d);
                        }
                    }
                }
            }
        }
    }

    // Fall back to family name matching when typeface stream is inaccessible or synthetic
    SkString familyName;
    typeface->getFamilyName(&familyName);
    if (!familyName.isEmpty()) {
        SkUniqueCFRef<CFStringRef> cfFamily(CFStringCreateWithCString(
            kCFAllocatorDefault, familyName.c_str(), kCFStringEncodingUTF8));
        if (cfFamily) {
            SkUniqueCFRef<CFDictionaryRef> traits = make_traits_dict(typeface->fontStyle());
            if (traits) {
                const void* keys[] = { kCTFontFamilyNameAttribute, kCTFontTraitsAttribute };
                const void* values[] = { cfFamily.get(), traits.get() };
                SkUniqueCFRef<CFDictionaryRef> attrs(CFDictionaryCreate(
                    kCFAllocatorDefault, keys, values, 2,
                    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
                if (attrs) {
                    return SkUniqueCFRef<CTFontDescriptorRef>(
                        CTFontDescriptorCreateWithAttributes(attrs.get()));
                }
            }
        }
    }

    SkUniqueCFRef<CTFontRef> sysFont = create_system_ui_font(0.0);
    if (sysFont) {
        CTFontDescriptorRef desc = CTFontCopyFontDescriptor(sysFont.get());
        return SkUniqueCFRef<CTFontDescriptorRef>(desc);
    }
    return nullptr;
}

SkUniqueCFRef<CTFontRef> CoreTextFontRegistry::createFont(SkTypeface* typeface, const SkFontStyle& style, CGFloat size) {
    if (size <= 0.0f) {
        size = 14.0f;
    }

    if (isAppleSystemFont(typeface)) {
        SkUniqueCFRef<CTFontRef> sysFont = create_system_ui_font(size);
        SkFontStyle fontStyle = (style.slant() != SkFontStyle::kUpright_Slant) ? style : (typeface ? typeface->fontStyle() : style);
        SkUniqueCFRef<CTFontDescriptorRef> traitsDesc = make_traits_descriptor(fontStyle);
        if (traitsDesc) {
            CTFontRef styledFont = CTFontCreateCopyWithAttributes(
                sysFont.get(), size, nullptr, traitsDesc.get());
            if (styledFont) {
                return SkUniqueCFRef<CTFontRef>(styledFont);
            }
        }
        return sysFont;
    }

    SkUniqueCFRef<CTFontDescriptorRef> desc = this->getDescriptor(typeface);
    if (desc) {
        CTFontRef font = CTFontCreateWithFontDescriptor(desc.get(), size, nullptr);
        if (font) {
            return SkUniqueCFRef<CTFontRef>(font);
        }
    }

    return create_system_ui_font(size);
}

bool CoreTextFontRegistry::isAppleSystemFont(const SkTypeface* tf) {
    if (!tf) {
        return true;
    }
    SkString name;
    tf->getFamilyName(&name);
    if (!name.isEmpty() && isAppleSystemFont(name.c_str())) {
        return true;
    }
    tf->getPostScriptName(&name);
    if (!name.isEmpty() && isAppleSystemFont(name.c_str())) {
        return true;
    }
    return false;
}

bool CoreTextFontRegistry::isAppleSystemFont(const char* familyName) {
    if (!familyName || !*familyName) {
        return true;
    }
    SkString name(familyName);
    return name.startsWith(".AppleSystemUIFont") || name.startsWith(".SF");
}

}  // namespace textlayout
}  // namespace skia

#endif  // defined(SK_PARAGRAPH_NATIVE_CORETEXT)
