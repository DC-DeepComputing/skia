// Copyright 2026 Google LLC
#include "modules/skparagraph/src/CoreTextFontRegistry.h"

#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/ports/SkTypeface_mac.h"
#include "src/utils/mac/SkUniqueCFRef.h"

namespace skia {
namespace textlayout {

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
            auto ctfont = SkTypeface_GetCTFontRef(typeface);
            if (ctfont) {
                // Preserve symbolic traits like bold and italic when instantiating system descriptors
                CTFontSymbolicTraits traits = CTFontGetSymbolicTraits(ctfont);
                if (traits & (kCTFontTraitBold | kCTFontTraitItalic)) {
                    CTFontRef styled = CTFontCreateCopyWithSymbolicTraits(
                        sysFont.get(), 0.0, nullptr, traits, traits);
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
            const void* keys[] = { kCTFontFamilyNameAttribute };
            const void* values[] = { cfFamily.get() };
            SkUniqueCFRef<CFDictionaryRef> attrs(CFDictionaryCreate(
                kCFAllocatorDefault, keys, values, 1,
                &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
            if (attrs) {
                return SkUniqueCFRef<CTFontDescriptorRef>(
                    CTFontDescriptorCreateWithAttributes(attrs.get()));
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

SkUniqueCFRef<CTFontRef> CoreTextFontRegistry::createFont(SkTypeface* typeface, CGFloat size) {
    if (size <= 0.0f) {
        size = 14.0f;
    }

    if (isAppleSystemFont(typeface)) {
        SkUniqueCFRef<CTFontRef> sysFont = create_system_ui_font(size);
        if (typeface) {
            auto ctfont = SkTypeface_GetCTFontRef(typeface);
            if (ctfont) {
                CTFontSymbolicTraits traits = CTFontGetSymbolicTraits(ctfont);
                if (traits & (kCTFontTraitBold | kCTFontTraitItalic)) {
                    CTFontRef styledFont = CTFontCreateCopyWithSymbolicTraits(
                        sysFont.get(), size, nullptr, traits, traits);
                    if (styledFont) {
                        return SkUniqueCFRef<CTFontRef>(styledFont);
                    }
                }
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

SkUniqueCFRef<CTFontRef> CoreTextFontRegistry::createFont(const SkFont& font) {
    return this->createFont(font.getTypeface(), font.getSize());
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
    return name.startsWith(".AppleSystemUIFont") ||
           name.startsWith(".SF") ||
           name.startsWith("SF Pro") ||
           name.startsWith("SF Compact") ||
           name.startsWith("SFUI") ||
           name.startsWith("System Font") ||
           name.startsWith("-apple-system") ||
           name.equals("system-ui") ||
           name.startsWith("CupertinoSystemText");
}

void CoreTextFontRegistry::clearCache() {
    SkAutoMutexExclusive lock(fMutex);
    fDescriptorCache.reset();
}

}  // namespace textlayout
}  // namespace skia

#endif  // defined(SK_PARAGRAPH_NATIVE_CORETEXT)
