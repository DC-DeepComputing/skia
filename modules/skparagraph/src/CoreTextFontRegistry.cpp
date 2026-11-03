// Copyright 2026 Google LLC
#include "modules/skparagraph/src/CoreTextFontRegistry.h"

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/ports/SkTypeface_mac.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#include <dlfcn.h>
#include <iterator>
#include <string_view>

namespace skia {
namespace textlayout {

// Returns a CFData over base[0, len), bytes that stream owns, and hands stream to the CFData:
// descriptors parsed from it stay in fDescriptorCache and back CTFonts after the caller of
// openStream and even the typeface are gone. The pattern of cfdata_from_skdata in
// SkTypeface_mac_ct.cpp. Null, with stream freed, if CoreFoundation cannot allocate
static SkUniqueCFRef<CFDataRef> cfdata_owning_stream(std::unique_ptr<SkStreamAsset> stream,
                                                     const void* base,
                                                     size_t len) {
    SkStreamAsset* owned = stream.release();
    CFAllocatorContext context = {};
    context.info = owned;
    context.deallocate = [](void*, void* info) { delete static_cast<SkStreamAsset*>(info); };
    SkUniqueCFRef<CFAllocatorRef> deallocator(CFAllocatorCreate(kCFAllocatorDefault, &context));
    if (!deallocator) {
        delete owned;
        return nullptr;
    }
    return SkUniqueCFRef<CFDataRef>(CFDataCreateWithBytesNoCopy(
            kCFAllocatorDefault, static_cast<const UInt8*>(base), len, deallocator.get()));
}

// Returns the NSCTFontUIFontDesignTrait value for a family name ending in Rounded, Serif or
// Monospaced, else null for the default design. Apple's Rounded, Serif, and Monospaced UI designs
// are selected through the CoreText traits dictionary key underlying
// NSFontDescriptorSystemDesign / UIFontDescriptorSystemDesign
static CFStringRef system_font_design(const SkString& familyName) {
    std::string_view name(familyName.c_str(), familyName.size());
    if (name.ends_with("Rounded")) {
        return CFSTR("NSCTFontUIFontDesignRounded");
    }
    if (name.ends_with("Serif")) {
        return CFSTR("NSCTFontUIFontDesignSerif");
    }
    if (name.ends_with("Monospaced")) {
        return CFSTR("NSCTFontUIFontDesignMonospaced");
    }
    return nullptr;
}

// Returns the UI design of typeface's family name, as above; null for a null typeface, which
// createFont treats as the default system UI font
static CFStringRef system_font_design(const SkTypeface* typeface) {
    if (!typeface) {
        return nullptr;
    }
    SkString familyName;
    typeface->getFamilyName(&familyName);
    return system_font_design(familyName);
}

// Maps [0, 1000] CSS weights linearly across Apple's UIFontWeight / NSFontWeight table,
// matching Skia's CoreText typeface port without linking an unexported core symbol
static CGFloat ct_weight_for_css_weight(int cssWeight) {
    static constexpr CGFloat kDefaultWeights[11] = {
            -1.00, -0.80, -0.60, -0.40, 0.00, 0.23, 0.30, 0.40, 0.56, 0.62, 1.00};
#if defined(SK_BUILD_FOR_IOS)
#define SK_PARAGRAPH_KIT_WEIGHT_PREFIX "UI"
#else
#define SK_PARAGRAPH_KIT_WEIGHT_PREFIX "NS"
#endif
    static constexpr const char* kWeightNames[9] = {
            SK_PARAGRAPH_KIT_WEIGHT_PREFIX "FontWeightUltraLight",
            SK_PARAGRAPH_KIT_WEIGHT_PREFIX "FontWeightThin",
            SK_PARAGRAPH_KIT_WEIGHT_PREFIX "FontWeightLight",
            SK_PARAGRAPH_KIT_WEIGHT_PREFIX "FontWeightRegular",
            SK_PARAGRAPH_KIT_WEIGHT_PREFIX "FontWeightMedium",
            SK_PARAGRAPH_KIT_WEIGHT_PREFIX "FontWeightSemibold",
            SK_PARAGRAPH_KIT_WEIGHT_PREFIX "FontWeightBold",
            SK_PARAGRAPH_KIT_WEIGHT_PREFIX "FontWeightHeavy",
            SK_PARAGRAPH_KIT_WEIGHT_PREFIX "FontWeightBlack",
    };
#undef SK_PARAGRAPH_KIT_WEIGHT_PREFIX

    static const auto* selectedWeights = []() -> const CGFloat(*)[11] {
        static CGFloat dynamicWeights[11];
        dynamicWeights[0] = -1.00;
        for (size_t i = 0; i < std::size(kWeightNames); ++i) {
            void* ptr = dlsym(RTLD_DEFAULT, kWeightNames[i]);
            if (!ptr) {
                return &kDefaultWeights;
            }
            dynamicWeights[i + 1] = *static_cast<const CGFloat*>(ptr);
        }
        dynamicWeights[10] = 1.00;
        return &dynamicWeights;
    }();

    const CGFloat(&weights)[11] = *selectedWeights;
    if (cssWeight <= 0) {
        return weights[0];
    }
    if (cssWeight >= 1000) {
        return weights[10];
    }
    int step = cssWeight / 100;
    return weights[step] +
           ((cssWeight - step * 100) * (weights[step + 1] - weights[step])) / 100.0;
}

// Builds a CoreText traits dictionary for style: weight on Apple's scale, slant, the italic
// symbolic trait and, with a non-null design, that UI design plus kCTFontTraitUIOptimized. A
// trait whose CFNumber cannot be allocated is left out; null if the dictionary itself is not
static SkUniqueCFRef<CFDictionaryRef> make_traits_dict(const SkFontStyle& style,
                                                       CFStringRef design = nullptr) {
    SkUniqueCFRef<CFMutableDictionaryRef> cfTraits(
        CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                  &kCFTypeDictionaryKeyCallBacks,
                                  &kCFTypeDictionaryValueCallBacks));
    if (!cfTraits) {
        return nullptr;
    }
    CGFloat ctWeight = ct_weight_for_css_weight(style.weight());
    SkUniqueCFRef<CFNumberRef> cfFontWeight(
        CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &ctWeight));
    if (cfFontWeight) {
        CFDictionaryAddValue(cfTraits.get(), kCTFontWeightTrait, cfFontWeight.get());
    }

    // Same slope as SkFontMgr_mac_ct.cpp (taken from WebKit), so an italic request matches the
    // faces Skia's CoreText font manager would pick
    static const CGFloat kSystemFontItalicSlope = 0.07;
    CGFloat ctSlant = (style.slant() == SkFontStyle::kUpright_Slant) ? 0.0 : kSystemFontItalicSlope;
    SkUniqueCFRef<CFNumberRef> cfFontSlant(
        CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &ctSlant));
    if (cfFontSlant) {
        CFDictionaryAddValue(cfTraits.get(), kCTFontSlantTrait, cfFontSlant.get());
    }

    // CoreText requires kCTFontTraitUIOptimized alongside NSCTFontUIFontDesignTrait so that
    // weight and slant traits resolve within the requested UI design family
    uint32_t symTraits = (style.slant() != SkFontStyle::kUpright_Slant ? kCTFontTraitItalic : 0) |
                         (design ? kCTFontTraitUIOptimized : 0);
    if (symTraits != 0) {
        SkUniqueCFRef<CFNumberRef> cfSym(
            CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &symTraits));
        if (cfSym) {
            CFDictionaryAddValue(cfTraits.get(), kCTFontSymbolicTrait, cfSym.get());
        }
    }
    if (design) {
        CFDictionaryAddValue(cfTraits.get(), CFSTR("NSCTFontUIFontDesignTrait"), design);
    }
    return SkUniqueCFRef<CFDictionaryRef>(cfTraits.release());
}

// Wraps make_traits_dict(style, design) in a descriptor that carries only
// kCTFontTraitsAttribute, for CTFontCreateCopyWithAttributes; null if any allocation fails
static SkUniqueCFRef<CTFontDescriptorRef> make_traits_descriptor(const SkFontStyle& style,
                                                                 CFStringRef design = nullptr) {
    SkUniqueCFRef<CFDictionaryRef> traits = make_traits_dict(style, design);
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

// Returns the system UI font at size restyled to style and design (null for the default design),
// or the unstyled UI font when CoreText rejects the traits. Null only when create_system_ui_font
// fails, which CTFontCreateUIFontForLanguage's _Nullable result allows
static SkUniqueCFRef<CTFontRef> styled_system_ui_font(const SkFontStyle& style,
                                                      CFStringRef design,
                                                      CGFloat size) {
    SkUniqueCFRef<CTFontRef> sysFont = create_system_ui_font(size);
    if (!sysFont) {
        return nullptr;
    }
    SkUniqueCFRef<CTFontDescriptorRef> traitsDesc = make_traits_descriptor(style, design);
    if (!traitsDesc) {
        return sysFont;
    }
    if (CTFontRef styled =
                CTFontCreateCopyWithAttributes(sysFont.get(), size, nullptr, traitsDesc.get())) {
        return SkUniqueCFRef<CTFontRef>(styled);
    }
    return sysFont;
}

// .AppleSystemUIFont is not reachable through family-name matching, so it takes the same route
// as createFont; any other name must match an installed family exactly, so a missing one is
// reported instead of becoming whatever CoreText considers closest
static sk_sp<SkTypeface> create_system_family(const SkString& familyName,
                                              const SkFontStyle& style) {
    if (CoreTextFontRegistry::isAppleSystemFont(familyName.c_str())) {
        SkUniqueCFRef<CTFontRef> sysFont =
                styled_system_ui_font(style, system_font_design(familyName), 0.0);
        return sysFont ? SkMakeTypefaceFromCTFont(sysFont.get()) : nullptr;
    }

    SkUniqueCFRef<CFStringRef> cfFamily(CFStringCreateWithCString(
            kCFAllocatorDefault, familyName.c_str(), kCFStringEncodingUTF8));
    SkUniqueCFRef<CFDictionaryRef> traits = make_traits_dict(style);
    if (!cfFamily || !traits) {
        return nullptr;
    }
    const void* keys[] = {kCTFontFamilyNameAttribute, kCTFontTraitsAttribute};
    const void* values[] = {cfFamily.get(), traits.get()};
    SkUniqueCFRef<CFDictionaryRef> attrs(CFDictionaryCreate(kCFAllocatorDefault, keys, values, 2,
                                                            &kCFTypeDictionaryKeyCallBacks,
                                                            &kCFTypeDictionaryValueCallBacks));
    if (!attrs) {
        return nullptr;
    }
    SkUniqueCFRef<CTFontDescriptorRef> request(CTFontDescriptorCreateWithAttributes(attrs.get()));
    const void* mandatory[] = {kCTFontFamilyNameAttribute};
    SkUniqueCFRef<CFSetRef> mandatorySet(
            CFSetCreate(kCFAllocatorDefault, mandatory, 1, &kCFTypeSetCallBacks));
    if (!request || !mandatorySet) {
        return nullptr;
    }
    SkUniqueCFRef<CTFontDescriptorRef> resolved(
            CTFontDescriptorCreateMatchingFontDescriptor(request.get(), mandatorySet.get()));
    if (!resolved) {
        return nullptr;
    }
    SkUniqueCFRef<CTFontRef> font(CTFontCreateWithFontDescriptor(resolved.get(), 0.0, nullptr));
    return font ? SkMakeTypefaceFromCTFont(font.get()) : nullptr;
}

CoreTextFontRegistry& CoreTextFontRegistry::Instance() {
    static CoreTextFontRegistry registry;
    return registry;
}

// Returns typeface's style, made italic when its CTFont is: system UI italic faces
// (.SFNS-RegularItalic) set kCTFontTraitItalic on the CTFont but omit kCTFontSlantTrait from
// CTFontCopyFontDescriptor, so typeface->fontStyle() alone reports upright
static SkFontStyle system_ui_style(SkTypeface* typeface) {
    SkFontStyle style = typeface->fontStyle();
    CTFontRef ctFont = SkTypeface_GetCTFontRef(typeface);
    if (!ctFont) {
        return style;
    }
    if ((CTFontGetSymbolicTraits(ctFont) & kCTFontTraitItalic) == 0 &&
        CTFontGetSlantAngle(ctFont) == 0) {
        return style;
    }
    return SkFontStyle(style.weight(), style.width(), SkFontStyle::kItalic_Slant);
}

// Parses typeface's font data into CoreText descriptors and returns the one for its ttcIndex
// face; null when the face has no readable data or CoreText cannot parse it
static SkUniqueCFRef<CTFontDescriptorRef> descriptor_from_font_data(SkTypeface* typeface) {
    int ttcIndex = 0;
    std::unique_ptr<SkStreamAsset> stream = typeface->openStream(&ttcIndex);
    if (!stream || !stream->hasLength()) {
        return nullptr;
    }
    size_t len = stream->getLength();
    if (len == 0) {
        return nullptr;
    }
    SkUniqueCFRef<CFDataRef> cfData;
    // Memory-backed font streams share their underlying buffer directly to avoid copying
    // multi-megabyte font files on descriptor creation; the CFData keeps the stream alive
    if (const void* base = stream->getMemoryBase()) {
        cfData = cfdata_owning_stream(std::move(stream), base, len);
    } else {
        stream->rewind();
        SkUniqueCFRef<CFMutableDataRef> mutableData(CFDataCreateMutable(kCFAllocatorDefault, len));
        if (mutableData) {
            CFDataSetLength(mutableData.get(), len);
            if (stream->read(CFDataGetMutableBytePtr(mutableData.get()), len) == len) {
                cfData.reset(mutableData.release());
            }
        }
    }
    if (!cfData) {
        return nullptr;
    }
    // CTFontManagerCreateFontDescriptorsFromData creates in-memory descriptors
    // without fontd IPC roundtrips
    SkUniqueCFRef<CFArrayRef> descs(CTFontManagerCreateFontDescriptorsFromData(cfData.get()));
    if (!descs || CFArrayGetCount(descs.get()) == 0) {
        return nullptr;
    }
    CFIndex count = CFArrayGetCount(descs.get());
    // KEEPER-DEBT: an out-of-range ttcIndex silently shapes with face 0. Kept until it is shown
    // whether CoreText can parse fewer faces than the typeface's collection holds (R107 2.8)
    CFIndex idx = (ttcIndex >= 0 && ttcIndex < count) ? ttcIndex : 0;
    CTFontDescriptorRef d = (CTFontDescriptorRef)CFArrayGetValueAtIndex(descs.get(), idx);
    if (!d) {
        return nullptr;
    }
    CFRetain(d);
    return SkUniqueCFRef<CTFontDescriptorRef>(d);
}

// Returns a descriptor that requests typeface's family name in its style; null for an empty
// family name or when an allocation fails
static SkUniqueCFRef<CTFontDescriptorRef> descriptor_from_family_name(SkTypeface* typeface) {
    SkString familyName;
    typeface->getFamilyName(&familyName);
    if (familyName.isEmpty()) {
        return nullptr;
    }
    SkUniqueCFRef<CFStringRef> cfFamily(CFStringCreateWithCString(
            kCFAllocatorDefault, familyName.c_str(), kCFStringEncodingUTF8));
    if (!cfFamily) {
        return nullptr;
    }
    SkUniqueCFRef<CFDictionaryRef> traits = make_traits_dict(typeface->fontStyle());
    if (!traits) {
        return nullptr;
    }
    const void* keys[] = {kCTFontFamilyNameAttribute, kCTFontTraitsAttribute};
    const void* values[] = {cfFamily.get(), traits.get()};
    SkUniqueCFRef<CFDictionaryRef> attrs(CFDictionaryCreate(kCFAllocatorDefault,
                                                            keys,
                                                            values,
                                                            2,
                                                            &kCFTypeDictionaryKeyCallBacks,
                                                            &kCFTypeDictionaryValueCallBacks));
    if (!attrs) {
        return nullptr;
    }
    return SkUniqueCFRef<CTFontDescriptorRef>(CTFontDescriptorCreateWithAttributes(attrs.get()));
}

SkUniqueCFRef<CTFontDescriptorRef> CoreTextFontRegistry::getDescriptor(SkTypeface* typeface) {
    // Callers pass faces from findTypefaces, which drops nulls, and createFont sends a null
    // typeface down its system UI branch before reaching here
    SkASSERT(typeface);

    // A CTFont-backed face already owns its descriptor, so copying it is a retain and caching
    // it would only duplicate state the typeface keeps alive anyway
    if (!isAppleSystemFont(typeface)) {
        if (CTFontRef ctFont = SkTypeface_GetCTFontRef(typeface)) {
            return SkUniqueCFRef<CTFontDescriptorRef>(CTFontCopyFontDescriptor(ctFont));
        }
    }

    SkTypefaceID id = typeface->uniqueID();
    {
        SkAutoMutexExclusive lock(fMutex);
        if (auto* found = fDescriptorCache.find(id)) {
            CFRetain(found->get());
            return SkUniqueCFRef<CTFontDescriptorRef>(found->get());
        }
    }

    // Stream reads and CoreText descriptor parsing run outside fMutex so concurrent paragraphs do
    // not block on font I/O
    SkUniqueCFRef<CTFontDescriptorRef> desc = this->createDescriptorInternal(typeface);
    if (desc) {
        CFRetain(desc.get());
        SkAutoMutexExclusive lock(fMutex);
        fDescriptorCache.insert_or_update(id, SkUniqueCFRef<CTFontDescriptorRef>(desc.get()));
    }
    return desc;
}

SkUniqueCFRef<CTFontDescriptorRef> CoreTextFontRegistry::createDescriptorInternal(
        SkTypeface* typeface) {
    SkASSERT(typeface);
    // Apple system UI font requires dynamic instantiation to resolve optical sizing and system
    // cascade
    if (isAppleSystemFont(typeface)) {
        if (SkUniqueCFRef<CTFontRef> sysFont = styled_system_ui_font(
                    system_ui_style(typeface), system_font_design(typeface), 0.0)) {
            return SkUniqueCFRef<CTFontDescriptorRef>(CTFontCopyFontDescriptor(sysFont.get()));
        }
    }

    if (SkUniqueCFRef<CTFontDescriptorRef> desc = descriptor_from_font_data(typeface)) {
        return desc;
    }

    // Fall back to family name matching when typeface stream is inaccessible or synthetic
    if (SkUniqueCFRef<CTFontDescriptorRef> desc = descriptor_from_family_name(typeface)) {
        return desc;
    }

    SkUniqueCFRef<CTFontRef> sysFont = create_system_ui_font(0.0);
    if (!sysFont) {
        return nullptr;
    }
    return SkUniqueCFRef<CTFontDescriptorRef>(CTFontCopyFontDescriptor(sysFont.get()));
}

SkUniqueCFRef<CTFontRef> CoreTextFontRegistry::createFont(SkTypeface* typeface,
                                                          const SkFontStyle& style,
                                                          CGFloat size) {
    // The single default for a non-positive TextStyle size lives in the callers (14, the same as
    // the shaping path), so a zero here would silently become CoreText's own default of 12
    SkASSERT(size > 0);

    if (isAppleSystemFont(typeface)) {
        return styled_system_ui_font(style, system_font_design(typeface), size);
    }

    if (SkUniqueCFRef<CTFontDescriptorRef> desc = this->getDescriptor(typeface)) {
        if (CTFontRef font = CTFontCreateWithFontDescriptor(desc.get(), size, nullptr)) {
            return SkUniqueCFRef<CTFontRef>(font);
        }
    }

    return create_system_ui_font(size);
}

std::vector<sk_sp<SkTypeface>> CoreTextFontRegistry::findTypefaces(
        FontCollection& fontCollection,
        const std::vector<SkString>& familyNames,
        SkFontStyle fontStyle,
        const std::optional<FontArguments>& fontArgs) {
    // A captureless lambda converts to the plain function pointer FontCollection takes, so the
    // shared FontCollection.cpp needs no CoreText headers
    return fontCollection.findCoreTextTypefaces(
            familyNames, fontStyle, fontArgs, [](const SkString& familyName, SkFontStyle style) {
                return Instance().matchSystemFamily(familyName, style);
            });
}

sk_sp<SkTypeface> CoreTextFontRegistry::matchSystemFamily(const SkString& familyName,
                                                          SkFontStyle fontStyle) {
    SystemFamilyKey key{familyName, fontStyle};
    {
        SkAutoMutexExclusive lock(fMutex);
        if (sk_sp<SkTypeface>* found = fSystemFamilyCache.find(key)) {
            return *found;
        }
    }
    // Resolved outside the lock: descriptor matching may wait on fontd, and a racing thread
    // computes the same face, so the later insert only refreshes an equal entry
    sk_sp<SkTypeface> typeface = create_system_family(familyName, fontStyle);
    SkAutoMutexExclusive lock(fMutex);
    fSystemFamilyCache.insert_or_update(std::move(key), typeface);
    return typeface;
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

// Flutter's CupertinoTextThemeData passes CupertinoSystemText and CupertinoSystemDisplay as
// family names; treating them as Apple system UI font aliases prevents findTypefaces from
// skipping them and falling through to the first fontFamilyFallback entry
bool CoreTextFontRegistry::isAppleSystemFont(const char* familyName) {
    if (!familyName || !*familyName) {
        return true;
    }
    // A view instead of an SkString copy: this runs for every family of every span
    std::string_view name(familyName);
    return name.starts_with(".AppleSystemUIFont") || name.starts_with(".SF") ||
           name.starts_with(".NewYork") || name == "CupertinoSystemText" ||
           name == "CupertinoSystemDisplay";
}

}  // namespace textlayout
}  // namespace skia
