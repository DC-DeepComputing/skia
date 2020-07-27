/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTSearch.h"
#include "src/ports/SkFontHost_FreeType_common.h"
#include "src/utils/SkUTF.h"

#include <android/api-level.h>

/**
 * Technically, the AFont API was introduced in Android 11 (Q, API 29). However...
 *
 * The AFontMatcher API implementation is broken from its introduction until at least API 33. What
 * is desired is to find a font for the given locale which contains the given character. However,
 * the implementation actually attempts to shape the string passed to it with the default font and
 * then returns the font chosen for the first run. However, this produces undesireable results, as
 * it will always prefer the default font over the locale, so any code points covered by the default
 * font will always come from the default font regardless of the requested locale. In addition, this
 * will claim coverage for code points "made up" by the shaper through normalization,
 * denormalization, whitespace synthesis, no-draw synthesis, etc, for the default font, when there
 * may be better choices later in fallback.
 *
 * On Android 10 (Q, API 29) AFont_getLocale always returns nullptr (if there is a locale set) or
 * whatever std::unique_ptr<std::string>()->c_str() returns, which happens to be 0x1. As a result,
 * AFont_getLocale cannot be used until Android 11 (R, API 30). This is b/139201432 and fixed with
 * "Make AFont_getLocale work" [0]. This change is in Android 11 (API 30) but does not appear to
 * have been cherry-picked into Android 10 (Q, API 29).
 * [0]  https://cs.android.com/android/_/android/platform/frameworks/base/+/01709c7469b59e451f064c266bbe442e9bef0ab4
 *
 * As a result, there is no correct way to use locale information from the Android 11 NDK.
 */
#define SK_FONTMGR_ANDROID_NDK_API_LEVEL __ANDROID_API_R__

#if __ANDROID_API__ >= SK_FONTMGR_ANDROID_NDK_API_LEVEL
#include <android/font.h>
#include <android/font_matcher.h>
#include <android/system_fonts.h>
#endif

#include <memory>

#include <dlfcn.h>

struct AFontMatcher;
struct ASystemFontIterator;
struct AFont;

namespace {

[[maybe_unused]] static inline const constexpr bool kSkFontMgrVerbose = true;

struct AndroidFontAPI {
    ASystemFontIterator* (*ASystemFontIterator_open)();
    void (*ASystemFontIterator_close)(ASystemFontIterator*);
    AFont* (*ASystemFontIterator_next)(ASystemFontIterator*);

    void (*AFont_close)(AFont*);
    const char* (*AFont_getFontFilePath)(const AFont*);
    uint16_t (*AFont_getWeight)(const AFont*);
    bool (*AFont_isItalic)(const AFont*);
    const char* (*AFont_getLocale)(const AFont*);
    size_t (*AFont_getCollectionIndex)(const AFont*);
    size_t (*AFont_getAxisCount)(const AFont*);
    uint32_t (*AFont_getAxisTag)(const AFont*, uint32_t axisIndex);
    float (*AFont_getAxisValue)(const AFont*, uint32_t axisIndex);
};

#if __ANDROID_API__ >= SK_FONTMGR_ANDROID_NDK_API_LEVEL

static const AndroidFontAPI* GetAndroidFontAPI() {
    static AndroidFontAPI androidFontAPI {
        ASystemFontIterator_open,
        ASystemFontIterator_close,
        ASystemFontIterator_next,

        AFont_close,
        AFont_getFontFilePath,
        AFont_getWeight,
        AFont_isItalic,
        AFont_getLocale,
        AFont_getCollectionIndex,
        AFont_getAxisCount,
        AFont_getAxisTag,
        AFont_getAxisValue,
    };
    if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: GetAndroidFontAPI direct\n"); }
    return &androidFontAPI;
}

#else

static const AndroidFontAPI* GetAndroidFontAPI() {
    struct OptionalAndroidFontAPI : AndroidFontAPI {
        bool valid = false;
    };
    static OptionalAndroidFontAPI androidFontAPI = [](){
        using DLHandle = std::unique_ptr<void, SkFunctionObject<dlclose>>;
        OptionalAndroidFontAPI api;

        if (android_get_device_api_level() < SK_FONTMGR_ANDROID_NDK_API_LEVEL) {
            return api;
        }

        DLHandle self(dlopen("libandroid.so", RTLD_LAZY | RTLD_LOCAL));
        if (!self) {
            return api;
        }

#define SK_DLSYM_ANDROID_FONT_API(NAME)                           \
        do {                                                      \
            *(void**)(&api.NAME) = dlsym(self.get(), #NAME);      \
            if (!api.NAME) {                                      \
                if constexpr (kSkFontMgrVerbose) {                \
                    SkDebugf("SKIA: Failed to load: " #NAME "\n");\
                }                                                 \
                return api;                                       \
            }                                                     \
        } while (0)

        SK_DLSYM_ANDROID_FONT_API(ASystemFontIterator_open);
        SK_DLSYM_ANDROID_FONT_API(ASystemFontIterator_close);
        SK_DLSYM_ANDROID_FONT_API(ASystemFontIterator_next);

        SK_DLSYM_ANDROID_FONT_API(AFont_close);
        SK_DLSYM_ANDROID_FONT_API(AFont_getFontFilePath);
        SK_DLSYM_ANDROID_FONT_API(AFont_getWeight);
        SK_DLSYM_ANDROID_FONT_API(AFont_isItalic);
        SK_DLSYM_ANDROID_FONT_API(AFont_getLocale);
        SK_DLSYM_ANDROID_FONT_API(AFont_getCollectionIndex);
        SK_DLSYM_ANDROID_FONT_API(AFont_getAxisCount);
        SK_DLSYM_ANDROID_FONT_API(AFont_getAxisTag);
        SK_DLSYM_ANDROID_FONT_API(AFont_getAxisValue);

#undef SK_DLSYM_ANDROID_FONT_API

        api.valid = true;
        return api;
    }();
    if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: GetAndroidFontAPI dlsym\n"); }
    return androidFontAPI.valid ? &androidFontAPI : nullptr;
};

#endif

struct SkAFont {
    SkAFont(const AndroidFontAPI& api, AFont* font) : fAPI(api), fFont(font) {}
    SkAFont(SkAFont&& that) : fAPI(that.fAPI), fFont(that.fFont) {
        that.fFont = nullptr;
    }
    ~SkAFont() {
        if (fFont) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: AFont_close\n"); }
            fAPI.AFont_close(fFont);
        }
    }
    explicit operator bool() { return fFont; }

    const char* getFontFilePath() const { return fAPI.AFont_getFontFilePath(fFont); }
    uint16_t getWeight() const { return fAPI.AFont_getWeight(fFont); }
    bool isItalic() const { return fAPI.AFont_isItalic(fFont); }
    const char* getLocale() const { return fAPI.AFont_getLocale(fFont); }
    size_t getCollectionIndex() const { return fAPI.AFont_getCollectionIndex(fFont); }
    size_t getAxisCount() const { return fAPI.AFont_getAxisCount(fFont); }
    uint32_t getAxisTag(uint32_t index) const { return fAPI.AFont_getAxisTag(fFont, index); }
    float getAxisValue(uint32_t index) const { return fAPI.AFont_getAxisValue(fFont, index); }

private:
    const AndroidFontAPI& fAPI;
    AFont* fFont;
};

struct SkASystemFontIterator {
    SkASystemFontIterator(const AndroidFontAPI& api)
        : fAPI(api)
        , fIterator(fAPI.ASystemFontIterator_open())
    {
        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: ASystemFontIterator_open\n"); }
    }
    SkASystemFontIterator(SkASystemFontIterator&&) = default;
    ~SkASystemFontIterator() {
        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: ASystemFontIterator_close\n"); }
        fAPI.ASystemFontIterator_close(fIterator);
    }
    explicit operator bool() { return fIterator; }

    SkAFont next() { return SkAFont(fAPI, fAPI.ASystemFontIterator_next(fIterator)); }

private:
    const AndroidFontAPI& fAPI;
    ASystemFontIterator* const fIterator;
};

class SkLanguage {
public:
    SkLanguage() { }
    SkLanguage(const SkString& tag) : fTag(tag) { }
    SkLanguage(const char* tag) : fTag(tag) { }
    SkLanguage(const char* tag, size_t len) : fTag(tag, len) { }
    SkLanguage(const SkLanguage&) = default;
    SkLanguage& operator=(const SkLanguage& b) = default;

    /** Gets a BCP 47 language identifier for this SkLanguage.
        @return a BCP 47 language identifier representing this language
    */
    const SkString& getTag() const { return fTag; }

    /** Performs BCP 47 fallback to return an SkLanguage one step more general.
        @return an SkLanguage one step more general
    */
    SkLanguage getParent() const {
        SkASSERT(!fTag.isEmpty());
        const char* tag = fTag.c_str();

        // strip off the rightmost "-.*"
        const char* parentTagEnd = strrchr(tag, '-');
        if (parentTagEnd == nullptr) {
            return SkLanguage();
        }
        size_t parentTagLen = parentTagEnd - tag;
        return SkLanguage(tag, parentTagLen);
    }

    bool operator==(const SkLanguage& b) const {
        return fTag == b.fTag;
    }
    bool operator!=(const SkLanguage& b) const {
        return fTag != b.fTag;
    }

    using sk_is_trivially_relocatable = std::true_type;
private:
    //! BCP 47 language identifier
    SkString fTag;
    static_assert(::sk_is_trivially_relocatable<decltype(fTag)>::value);
};

class SkTypeface_Android : public SkTypeface_FreeType {
public:
    SkTypeface_Android(const SkFontStyle& style,
                       bool isFixedPitch,
                       const SkString& familyName)
        : INHERITED(style, isFixedPitch)
        , fFamilyName(familyName)
    {}

protected:
    void onGetFamilyName(SkString* familyName) const override {
        *familyName = fFamilyName;
    }

    SkString fFamilyName;

private:
    using INHERITED = SkTypeface_FreeType;
};

class SkTypeface_AndroidSystem : public SkTypeface_Android {
public:
    SkTypeface_AndroidSystem(std::unique_ptr<SkStreamAsset> file,
                             const SkString& pathName,
                             const bool cacheFontFiles,
                             int index,
                             const SkFixed* axes, int axesCount,
                             const SkFontStyle& style,
                             bool isFixedPitch,
                             const SkString& familyName,
                             SkTArray<SkLanguage>&& lang)
        : INHERITED(style, isFixedPitch, familyName)
        , fPathName(pathName)
        , fIndex(index)
        , fAxes(axes, axesCount)
        , fLang(std::move(lang))
        , fFile((cacheFontFiles &&  file) ? std::move(file)
               :(cacheFontFiles && !file) ? SkStream::MakeFromFile(fPathName.c_str())
               : nullptr)
        , fCacheFontFiles(cacheFontFiles)
    {
        if (cacheFontFiles) {
            SkASSERT(fFile);
        }
    }

    std::unique_ptr<SkStreamAsset> makeStream() const {
        if (fFile) {
            return fFile->duplicate();
        }
        return SkStream::MakeFromFile(fPathName.c_str());
    }

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const override {
        SkASSERT(desc);
        SkASSERT(serialize);
        desc->setFamilyName(fFamilyName.c_str());
        desc->setStyle(this->fontStyle());
        *serialize = false;
    }
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override {
        *ttcIndex = fIndex;
        return this->makeStream();
    }
    std::unique_ptr<SkFontData> onMakeFontData() const override {
        return std::make_unique<SkFontData>(
                this->makeStream(), fIndex, 0, fAxes.begin(), fAxes.size(), nullptr, 0);
    }
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        std::unique_ptr<SkFontData> data = this->cloneFontData(args);
        if (!data) {
            return nullptr;
        }
        return sk_make_sp<SkTypeface_AndroidSystem>(fFile->duplicate(),
                                                    fPathName,
                                                    fCacheFontFiles,
                                                    fIndex,
                                                    data->getAxis(),
                                                    data->getAxisCount(),
                                                    this->fontStyle(),
                                                    this->isFixedPitch(),
                                                    fFamilyName,
                                                    SkTArray<SkLanguage>());
    }

    const SkString fPathName;
    int fIndex;
    const SkSTArray<4, SkFixed> fAxes;
    const SkSTArray<4, SkLanguage> fLang;
    std::unique_ptr<SkStreamAsset> fFile;
    bool fCacheFontFiles;

    using INHERITED = SkTypeface_Android;
};

class SkTypeface_AndroidStream : public SkTypeface_Android {
public:
    SkTypeface_AndroidStream(std::unique_ptr<SkFontData> data,
                             const SkFontStyle& style,
                             bool isFixedPitch,
                             const SkString& familyName)
        : INHERITED(style, isFixedPitch, familyName)
        , fData(std::move(data))
    { }

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const override {
        SkASSERT(desc);
        SkASSERT(serialize);
        desc->setFamilyName(fFamilyName.c_str());
        SkTypeface_FreeType::FontDataPaletteToDescriptorPalette(*fData, desc);
        *serialize = true;
    }

    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override {
        *ttcIndex = fData->getIndex();
        return fData->getStream()->duplicate();
    }

    std::unique_ptr<SkFontData> onMakeFontData() const override {
        return std::make_unique<SkFontData>(*fData);
    }

    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        std::unique_ptr<SkFontData> data = this->cloneFontData(args);
        if (!data) {
            return nullptr;
        }
        return sk_make_sp<SkTypeface_AndroidStream>(std::move(data),
                                                    this->fontStyle(),
                                                    this->isFixedPitch(),
                                                    fFamilyName);
    }

private:
    const std::unique_ptr<const SkFontData> fData;
    using INHERITED = SkTypeface_Android;
};

class SkFontStyleSet_Android : public SkFontStyleSet {
    typedef SkTypeface_FreeType::Scanner Scanner;

public:
    explicit SkFontStyleSet_Android() { }

    int count() override {
        return fStyles.size();
    }
    void getStyle(int index, SkFontStyle* style, SkString* name) override {
        if (index < 0 || fStyles.size() <= index) {
            return;
        }
        if (style) {
            *style = fStyles[index]->fontStyle();
        }
        if (name) {
            name->reset();
        }
    }
    SkTypeface_AndroidSystem* createTypeface(int index) override {
        if (index < 0 || fStyles.size() <= index) {
            return nullptr;
        }
        return SkRef(fStyles[index].get());
    }

    SkTypeface_AndroidSystem* matchStyle(const SkFontStyle& pattern) override {
        return static_cast<SkTypeface_AndroidSystem*>(this->matchStyleCSS3(pattern));
    }

private:
    SkTArray<sk_sp<SkTypeface_AndroidSystem>> fStyles;
    friend class SkFontMgr_Android_NDK;
    using INHERITED = SkFontStyleSet;
};

struct NameToFamily {
    SkString name;
    SkString normalizedName;
    SkFontStyleSet_Android* styleSet;

    using sk_is_trivially_relocatable = std::true_type;
    static_assert(::sk_is_trivially_relocatable<decltype(name)>::value);
    static_assert(::sk_is_trivially_relocatable<decltype(normalizedName)>::value);
    static_assert(::sk_is_trivially_relocatable<decltype(styleSet)>::value);
};

class SkFontMgr_Android_NDK : public SkFontMgr {
    void addSystemTypeface(sk_sp<SkTypeface_AndroidSystem> typeface, const SkString& name) {
        NameToFamily* nameToFamily = nullptr;
        for (NameToFamily& current : fNameToFamilyMap) {
            if (current.name == name) {
                nameToFamily = &current;
                break;
            }
        }
        if (!nameToFamily) {
            sk_sp<SkFontStyleSet_Android> newSet = sk_make_sp<SkFontStyleSet_Android>();
            SkAutoAsciiToLC tolc(name.c_str());
            nameToFamily = &fNameToFamilyMap.emplace_back(
                NameToFamily{name, SkString(tolc.lc()), newSet.get()});
            fStyleSets.push_back(std::move(newSet));
        }
        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: Adding member to %s\n", name.c_str()); }
        nameToFamily->styleSet->fStyles.push_back(typeface);
    }

public:
    SkFontMgr_Android_NDK(const AndroidFontAPI& androidFontAPI, bool const cacheFontFiles)
        : fAPI(androidFontAPI)
    {
        SkASystemFontIterator fontIter(fAPI);
        if (!fontIter) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: No ASystemFontIterator"); }
            return;
        }

        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: Iterating over AFonts\n"); }
        while (SkAFont font = fontIter.next()) {
            sk_sp<SkTypeface_AndroidSystem> typeface = this->make(std::move(font), cacheFontFiles);
            if (!typeface) {
                continue;
            }

            SkString name;
            typeface->getFamilyName(&name);
            this->addSystemTypeface(typeface, name);

            // A font may have many localized family names.
            sk_sp<SkTypeface::LocalizedStrings> names(typeface->createFamilyNameIterator());
            SkTypeface::LocalizedString localeName;
            while (names->next(&localeName)) {
                if (localeName.fString != name) {
                    this->addSystemTypeface(typeface, localeName.fString);
                }
            }
        }

        if (fStyleSets.empty()) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: No fonts!"); }
        } else {
            this->findDefaultStyleSet();
        }
    }

protected:
    /** Returns not how many families we have, but how many unique names
     *  exist among the families.
     */
    int onCountFamilies() const override {
        return fNameToFamilyMap.size();
    }

    void onGetFamilyName(int index, SkString* familyName) const override {
        if (index < 0 || fNameToFamilyMap.size() <= index) {
            familyName->reset();
            return;
        }
        familyName->set(fNameToFamilyMap[index].name);
    }

    SkFontStyleSet* onCreateStyleSet(int index) const override {
        if (index < 0 || fNameToFamilyMap.size() <= index) {
            return nullptr;
        }
        return SkRef(fNameToFamilyMap[index].styleSet);
    }

    SkFontStyleSet* onMatchFamily(const char familyName[]) const override {
        if (!familyName) {
            return nullptr;
        }
        SkAutoAsciiToLC tolc(familyName);
        for (int i = 0; i < fNameToFamilyMap.size(); ++i) {
            if (fNameToFamilyMap[i].normalizedName.equals(tolc.lc())) {
                return SkRef(fNameToFamilyMap[i].styleSet);
            }
        }
        return nullptr;
    }

    SkTypeface* onMatchFamilyStyle(const char familyName[],
                                   const SkFontStyle& style) const override {
        sk_sp<SkFontStyleSet> sset(this->matchFamily(familyName));
        return sset->matchStyle(style);
    }

    sk_sp<SkTypeface_AndroidSystem> make(SkAFont font, bool cacheFontFiles) const {
        const char* filePath = font.getFontFilePath();

        std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(filePath);
        if (!stream) {
            if constexpr (kSkFontMgrVerbose) {
                SkDebugf("SKIA: Font file %s does not exist or cannot be opened.\n", filePath);
            }
            return nullptr;
        }

        size_t collectionIndex = font.getCollectionIndex();
        if constexpr (kSkFontMgrVerbose) {
            SkDebugf("SKIA: Making font from %s#%zu\n", filePath, collectionIndex);
        }
        if (!SkTFitsIn<int>(collectionIndex)) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: Collection index invalid!"); }
            return nullptr;
        }
        const int ttcIndex = SkTo<int>(collectionIndex);

        SkString familyName;
        SkFontStyle style;
        bool isFixedWidth;
        SkTypeface_FreeType::Scanner::AxisDefinitions axisDefinitions;
        if (!fScanner.scanFont(stream.get(), ttcIndex,
                               &familyName, &style, &isFixedWidth, &axisDefinitions))
        {
            if constexpr (kSkFontMgrVerbose) {
                SkDebugf("SKIA: Font file %s exists, but is not a valid font.\n", filePath);
            }
            return nullptr;
        }

        int weight = SkTo<int>(font.getWeight());
        SkFontStyle::Slant slant = style.slant();
        if (font.isItalic()) {
            slant = SkFontStyle::kItalic_Slant;
        }
        int width = style.width();
        constexpr SkFourByteTag wdth = SkSetFourByteTag('w','d','t','h');

        // The family name(s) are not reported.
        // This would be very helpful for aliases, like "sans-serif", "Arial", etc.

        size_t requestAxisCount = font.getAxisCount();
        if (!SkTFitsIn<int>(requestAxisCount)) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: Axis count unreasonable!"); }
            return nullptr;
        }
        using Coordinate = SkFontArguments::VariationPosition::Coordinate;
        SkAutoSTMalloc<4, Coordinate> requestAxisValues(requestAxisCount);
        for (size_t i = 0; i < requestAxisCount; ++i) {
            uint32_t tag = font.getAxisTag(i);
            float value = font.getAxisValue(i);
            requestAxisValues[i] = { tag, value };
            if (tag == wdth) {
                // Set the width based on the requested `wdth` axis value.
                width = SkFontDescriptor::SkFontStyleWidthForWidthAxisValue(value);
            }
        }

        SkAutoSTMalloc<4, SkFixed> axisValues(axisDefinitions.size());
        SkFontArguments::VariationPosition position = {
            requestAxisValues.get(), SkTo<int>(requestAxisCount)
        };
        SkTypeface_FreeType::Scanner::computeAxisValues(axisDefinitions, position,
                                                        axisValues, familyName);

        SkSTArray<4, SkLanguage> skLangs;
        const char* aLangs = font.getLocale();
        if (aLangs) {
            if constexpr (kSkFontMgrVerbose) {
                SkDebugf("SKIA: %s Langs %s\n", familyName.c_str(), aLangs);
            }
            // Format: ',' or '\0' are terminators, '\0' is the final terminator.
            const char* begin = aLangs;
            const char* end = aLangs;
            while (true) {
                while (*end != '\0' && *end != ',') {
                    ++end;
                }
                const size_t size = end - begin;
                if (size) {
                    skLangs.emplace_back(begin, size);
                }
                if (*end == '\0') {
                    break;
                }
                ++end;
                begin = end;
            }
        }

        style = SkFontStyle(weight, width, slant);
        if constexpr (kSkFontMgrVerbose) {
            SkDebugf("SKIA: New typeface %s [%d %d %d]\n", familyName.c_str(), weight,width,slant);
        }
        return sk_sp<SkTypeface_AndroidSystem>(new SkTypeface_AndroidSystem(
                std::move(stream), SkString(filePath), cacheFontFiles, ttcIndex,
                axisValues.get(), axisDefinitions.size(),
                style, isFixedWidth, familyName, std::move(skLangs)));
    }


    static sk_sp<SkTypeface_AndroidSystem> find_family_style_character(
            const SkString& familyName,
            const SkTArray<NameToFamily, true>& nameToFamilyMap,
            const SkFontStyle& style,
            const SkString& langTag, SkUnichar character)
    {
        for (int i = 0; i < nameToFamilyMap.size(); ++i) {
            SkFontStyleSet_Android* family = nameToFamilyMap[i].styleSet;
            sk_sp<SkTypeface_AndroidSystem> face(family->matchStyle(style));

            if (!langTag.isEmpty() &&
                std::none_of(face->fLang.begin(), face->fLang.end(), [&](SkLanguage lang) {
                    return lang.getTag().startsWith(langTag.c_str());
                }))
            {
                continue;
            }

            if (face->unicharToGlyph(character) != 0) {
                return face;
            }
        }
        return nullptr;
    }

    SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                            const SkFontStyle& style,
                                            const char* bcp47[],
                                            int bcp47Count,
                                            SkUnichar character) const override {
        // If at some point AFontMatcher becomes usable, the code for using it is at
        // https://skia-review.googlesource.com/c/skia/+/585970/13/src/ports/SkFontMgr_android_ndk.cpp#766

        SkString familyNameString(familyName);
        for (const SkString& currentFamilyName : { familyNameString, SkString() }) {
            for (int bcp47Index = bcp47Count; bcp47Index --> 0;) {
                SkLanguage lang(bcp47[bcp47Index]);
                while (!lang.getTag().isEmpty()) {
                    sk_sp<SkTypeface_AndroidSystem> matchingTypeface =
                        find_family_style_character(currentFamilyName, fNameToFamilyMap,
                                                    style, lang.getTag(), character);
                    if (matchingTypeface) {
                        return matchingTypeface.release();
                    }

                    lang = lang.getParent();
                }
            }
            sk_sp<SkTypeface_AndroidSystem> matchingTypeface =
                find_family_style_character(currentFamilyName, fNameToFamilyMap,
                                            style, SkString(), character);
            if (matchingTypeface) {
                return matchingTypeface.release();
            }
        }
        return nullptr;
    }

    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override {
        return this->makeFromStream(
            std::unique_ptr<SkStreamAsset>(new SkMemoryStream(std::move(data))), ttcIndex);
    }

    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override {
        std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(path);
        return stream ? this->makeFromStream(std::move(stream), ttcIndex) : nullptr;
    }

    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                            int ttcIndex) const override {
        bool isFixedPitch;
        SkFontStyle style;
        SkString name;
        if (!fScanner.scanFont(stream.get(), ttcIndex, &name, &style, &isFixedPitch, nullptr)) {
            return nullptr;
        }
        auto data = std::make_unique<SkFontData>(std::move(stream), ttcIndex, 0,
                                                 nullptr, 0, nullptr, 0);
        return sk_sp<SkTypeface>(new SkTypeface_AndroidStream(std::move(data),
                                                              style, isFixedPitch, name));
    }

    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                           const SkFontArguments& args) const override {
        using Scanner = SkTypeface_FreeType::Scanner;
        bool isFixedPitch;
        SkFontStyle style;
        SkString name;
        Scanner::AxisDefinitions axisDefinitions;
        if (!fScanner.scanFont(stream.get(), args.getCollectionIndex(),
                               &name, &style, &isFixedPitch, &axisDefinitions))
        {
            return nullptr;
        }

        SkAutoSTMalloc<4, SkFixed> axisValues(axisDefinitions.size());
        Scanner::computeAxisValues(axisDefinitions, args.getVariationDesignPosition(),
                                   axisValues, name);

        auto data = std::make_unique<SkFontData>(
            std::move(stream), args.getCollectionIndex(), args.getPalette().index,
            axisValues.get(), axisDefinitions.size(),
            args.getPalette().overrides, args.getPalette().overrideCount);
        return sk_sp<SkTypeface>(new SkTypeface_AndroidStream(std::move(data),
                                                              style, isFixedPitch, name));
    }

    sk_sp<SkTypeface> onLegacyMakeTypeface(const char name[], SkFontStyle style) const override {
        if (name) {
            // On Android, we must return nullptr when we can't find the requested
            // named typeface so that the system/app can provide their own recovery
            // mechanism. On other platforms we'd provide a typeface from the
            // default family instead.
            return sk_sp<SkTypeface>(this->onMatchFamilyStyle(name, style));
        }
        if (fDefaultStyleSet) {
            return sk_sp<SkTypeface>(fDefaultStyleSet->matchStyle(style));
        }
        return nullptr;
    }


private:
    const AndroidFontAPI& fAPI;

    SkTypeface_FreeType::Scanner fScanner;

    SkTArray<sk_sp<SkFontStyleSet_Android>> fStyleSets;
    sk_sp<SkFontStyleSet> fDefaultStyleSet;

    SkTArray<NameToFamily> fNameToFamilyMap;

    void findDefaultStyleSet() {
        SkASSERT(!fStyleSets.empty());

        static const char* defaultNames[] = { "sans-serif", "Roboto" };
        for (const char* defaultName : defaultNames) {
            fDefaultStyleSet.reset(this->onMatchFamily(defaultName));
            if (fDefaultStyleSet) {
                break;
            }
        }
        if (nullptr == fDefaultStyleSet) {
            fDefaultStyleSet = fStyleSets[0];
        }
        SkASSERT(fDefaultStyleSet);
    }

    using INHERITED = SkFontMgr;
};

}  // namespace

sk_sp<SkFontMgr> SkFontMgr_New_Android_NDK(bool cacheFontFiles) {
    AndroidFontAPI const * const androidFontAPI = GetAndroidFontAPI();
    if (!androidFontAPI) {
        return nullptr;
    }
    return sk_make_sp<SkFontMgr_Android_NDK>(*androidFontAPI, cacheFontFiles);
}
