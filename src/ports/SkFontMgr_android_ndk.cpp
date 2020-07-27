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

#if __ANDROID_API__ >= 29
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

[[maybe_unused]] static inline const constexpr bool kSkFontMgrVerbose = false;

struct AndroidFontAPI {
    static const constexpr uint32_t AFAMILY_VARIANT_DEFAULT = 0;
    static const constexpr uint32_t AFAMILY_VARIANT_COMPACT = 1;
    static const constexpr uint32_t AFAMILY_VARIANT_ELEGANT = 2;

    AFontMatcher* (*AFontMatcher_create)();
    void (*AFontMatcher_destroy)(AFontMatcher*);
    void (*AFontMatcher_setStyle)(AFontMatcher*, uint16_t weight, bool italic);
    // languageTags is '\0' terminated ',' separated BCP47
    void (*AFontMatcher_setLocales)(AFontMatcher*, const char* languageTags);
    void (*AFontMatcher_setFamilyVariant)(AFontMatcher*, uint32_t familyVariant);
    AFont* (*AFontMatcher_match)(const AFontMatcher*, const char* familyName,
                                 const uint16_t* text, const uint32_t textLength,
                                 uint32_t* runLengthOut);

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

#if __ANDROID_API__ >= 29

static const AndroidFontAPI* GetAndroidFontAPI() {
    static AndroidFontAPI androidFontAPI {
        AFontMatcher_create,
        AFontMatcher_destroy,
        AFontMatcher_setStyle,
        AFontMatcher_setLocales,
        AFontMatcher_setFamilyVariant,
        AFontMatcher_match,

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
    return &androidFontAPI;
}

#else

static const AndroidFontAPI* GetAndroidFontAPI() {
    struct OptionalAndroidFontAPI : AndroidFontAPI {
        bool valid = false;
    };
    static OptionalAndroidFontAPI androidFontAPI = [](){
        using DLHandle = std::unique_ptr<void, SkFunctionWrapper<decltype(dlclose), dlclose>>;
        OptionalAndroidFontAPI api;

        DLHandle self(dlopen("libandroid.so", RTLD_LAZY | RTLD_LOCAL));
        if (!self) {
            return api;
        }

#define SK_DLSYM_ANDROID_FONT_API(NAME)                      \
        do {                                                 \
            *(void**)(&api.NAME) = dlsym(self.get(), #NAME); \
            if (!api.NAME) {                                 \
                if constexpr (kSkFontMgrVerbose) {                     \
                    SkDebugf("SKIA: Failed to load: " #NAME);\
                }                                            \
                return api;                                  \
            }                                                \
        } while (0)

        SK_DLSYM_ANDROID_FONT_API(AFontMatcher_create);
        SK_DLSYM_ANDROID_FONT_API(AFontMatcher_destroy);
        SK_DLSYM_ANDROID_FONT_API(AFontMatcher_setStyle);
        SK_DLSYM_ANDROID_FONT_API(AFontMatcher_setLocales);
        SK_DLSYM_ANDROID_FONT_API(AFontMatcher_setFamilyVariant);
        SK_DLSYM_ANDROID_FONT_API(AFontMatcher_match);

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
    return androidFontAPI.valid ? &androidFontAPI : nullptr;
};

#endif

struct SkAFont {
    SkAFont(const AndroidFontAPI& api, AFont* font) : fAPI(api), fFont(font) {}
    SkAFont(SkAFont&&) = default;
    ~SkAFont() { fAPI.AFont_close(fFont); }
    explicit operator bool() { return fFont; }

    const char* getFontFilePath() { return fAPI.AFont_getFontFilePath(fFont); }
    uint16_t getWeight() { return fAPI.AFont_getWeight(fFont); }
    bool isItalic() { return fAPI.AFont_isItalic(fFont); }
    const char* getLocale() { return fAPI.AFont_getLocale(fFont); }
    size_t getCollectionIndex() { return fAPI.AFont_getCollectionIndex(fFont); }
    size_t getAxisCount() { return fAPI.AFont_getAxisCount(fFont); }
    uint32_t getAxisTag(uint32_t axisIndex) { return fAPI.AFont_getAxisTag(fFont, axisIndex); }
    float getAxisValue(uint32_t axisIndex) { return fAPI.AFont_getAxisValue(fFont, axisIndex); }

private:
    const AndroidFontAPI& fAPI;
    AFont* const fFont;
};

struct SkASystemFontIterator {
    SkASystemFontIterator(const AndroidFontAPI& api)
        : fAPI(api)
        , fIterator(fAPI.ASystemFontIterator_open())
    {
        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: ASystemFontIterator_open"); }
    }
    SkASystemFontIterator(SkASystemFontIterator&&) = default;
    ~SkASystemFontIterator() {
        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: ASystemFontIterator_close"); }
        fAPI.ASystemFontIterator_close(fIterator);
    }
    explicit operator bool() { return fIterator; }

    SkAFont next() { return SkAFont(fAPI, fAPI.ASystemFontIterator_next(fIterator)); }

private:
    const AndroidFontAPI& fAPI;
    ASystemFontIterator* const fIterator;
};

struct SkAFontMatcher {
    SkAFontMatcher(const AndroidFontAPI& api)
        : fAPI(api)
        , fMatcher(fAPI.AFontMatcher_create())
    {
        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: AFontMatcher_create"); }
    }
    SkAFontMatcher(SkAFontMatcher&&) = default;
    ~SkAFontMatcher() {
        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: AFontMatcher_destroy"); }
        fAPI.AFontMatcher_destroy(fMatcher);
    }
    explicit operator bool() { return fMatcher; }

    void setStyle(const SkFontStyle& style) {
        bool italic = style.slant() != SkFontStyle::kUpright_Slant;
        fAPI.AFontMatcher_setStyle(fMatcher, style.weight(), italic);
    }
    void setLocales(const char* bcp47[], int bcp47Count) {
        if (bcp47Count <= 0) {
            // Add nothing
        } else if (bcp47Count == 1) {
            fAPI.AFontMatcher_setLocales(fMatcher, bcp47[0]);
        } else {
            // `setLocales` expects a comma separated '\0' terminated string of locales.
            SkString locales(bcp47[0]);
            for (int i = 1; i < bcp47Count; ++i) {
                locales.appendUnichar(',');
                locales.append(bcp47[i]);
            }
            fAPI.AFontMatcher_setLocales(fMatcher, locales.c_str());
        }
    }
    void setFamilyVariant(uint32_t familyVariant) {
        fAPI.AFontMatcher_setFamilyVariant(fMatcher, familyVariant);
    }
    SkAFont match(const char* familyName,
                  const uint16_t* text, const uint32_t textLength,
                  uint32_t* runLengthOut) {
        if constexpr (kSkFontMgrVerbose) {
            SkDebugf("SKIA: AFontMatcher_match(matcher, \"%s\", %d, 1)", familyName, text[0]);
        }
        return SkAFont(fAPI, fAPI.AFontMatcher_match(fMatcher, familyName, text, textLength,
                                                     runLengthOut));
    }

private:
    const AndroidFontAPI& fAPI;
    AFontMatcher* const fMatcher;
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
                             const SkString& familyName
                             /*, const SkTArray<SkLanguage, true>& lang*/)
        : INHERITED(style, isFixedPitch, familyName)
        , fPathName(pathName)
        , fIndex(index)
        , fAxes(axes, axesCount)
        /*, fLang(lang)*/
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
                this->makeStream(), fIndex, 0, fAxes.begin(), fAxes.count(), nullptr, 0);
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
                                                    fFamilyName
                                                    /*, fLang*/);
    }

    const SkString fPathName;
    int fIndex;
    const SkSTArray<4, SkFixed, true> fAxes;
    /*const SkSTArray<4, SkLanguage, true> fLang;*/
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
        return fStyles.count();
    }
    void getStyle(int index, SkFontStyle* style, SkString* name) override {
        if (index < 0 || fStyles.count() <= index) {
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
        if (index < 0 || fStyles.count() <= index) {
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
        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: Adding member to %s", name.c_str()); }
        nameToFamily->styleSet->fStyles.push_back(typeface);
    }

public:
    SkFontMgr_Android_NDK(const AndroidFontAPI& androidFontAPI, bool const cacheFontFiles)
        : fAPI(androidFontAPI)
    {
        SkASystemFontIterator fontIter(fAPI);
        if (!fontIter) {
            return;
        }

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

        this->findDefaultStyleSet();
    }

protected:
    /** Returns not how many families we have, but how many unique names
     *  exist among the families.
     */
    int onCountFamilies() const override {
        return fNameToFamilyMap.count();
    }

    void onGetFamilyName(int index, SkString* familyName) const override {
        if (index < 0 || fNameToFamilyMap.count() <= index) {
            familyName->reset();
            return;
        }
        familyName->set(fNameToFamilyMap[index].name);
    }

    SkFontStyleSet* onCreateStyleSet(int index) const override {
        if (index < 0 || fNameToFamilyMap.count() <= index) {
            return nullptr;
        }
        return SkRef(fNameToFamilyMap[index].styleSet);
    }

    SkFontStyleSet* onMatchFamily(const char familyName[]) const override {
        if (!familyName) {
            return nullptr;
        }
        SkAutoAsciiToLC tolc(familyName);
        for (int i = 0; i < fNameToFamilyMap.count(); ++i) {
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

        return this->make(std::move(font), std::move(stream), cacheFontFiles);
    }
    sk_sp<SkTypeface_AndroidSystem> make(SkAFont font, std::unique_ptr<SkStreamAsset> stream,
                                         bool cacheFontFiles) const {
        const char* filePath = font.getFontFilePath();

        size_t collectionIndex = font.getCollectionIndex();
        if (!SkTFitsIn<int>(collectionIndex)) {
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

        // The family name is not reported.

        size_t requestAxisCount = font.getAxisCount();
        if (!SkTFitsIn<int>(requestAxisCount)) {
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

        SkAutoSTMalloc<4, SkFixed> axisValues(axisDefinitions.count());
        SkFontArguments::VariationPosition position = {
            requestAxisValues.get(), SkTo<int>(requestAxisCount)
        };
        SkTypeface_FreeType::Scanner::computeAxisValues(axisDefinitions, position,
                                                        axisValues, familyName);

        const char* langs = font.getLocale();
        //TODO: split langs on ',' and pass languages.
        (void)langs;

        style = SkFontStyle(weight, width, slant);
        if constexpr (kSkFontMgrVerbose) {SkDebugf("SKIA: New typeface %s", familyName.c_str());}
        return sk_sp<SkTypeface_AndroidSystem>(new SkTypeface_AndroidSystem(
                std::move(stream), SkString(filePath), cacheFontFiles, ttcIndex,
                axisValues.get(), axisDefinitions.count(),
                style, isFixedWidth, familyName/*, SkTArray<SkLanguage, true>()*/));
    }

    SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                            const SkFontStyle& style,
                                            const char* bcp47[],
                                            int bcp47Count,
                                            SkUnichar character) const override {
        SkAFontMatcher matcher(fAPI);
        if (!matcher) {
            return nullptr;
        }

        if (familyName == nullptr) {
            // AFontMatcher_match requires that the family name be non-nullptr.
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: Matcher rescuing nullptr"); }
            familyName = "";
        }

        matcher.setLocales(bcp47, bcp47Count);
        matcher.setFamilyVariant(AndroidFontAPI::AFAMILY_VARIANT_DEFAULT);
        matcher.setStyle(style);
        uint16_t utf16[2];
        size_t utf16Size = SkUTF::ToUTF16(character, utf16);
        SkAFont font = matcher.match(familyName, utf16, utf16Size, nullptr);

        const char* filePath = font.getFontFilePath();
        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: AFontMatcher_match matched: %s", filePath); }
        size_t filePathLen = strlen(filePath);
        size_t collectionIndex = font.getCollectionIndex();
        if (!SkTFitsIn<int>(collectionIndex)) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: Bailing early"); }
            return nullptr;
        }
        const int ttcIndex = SkTo<int>(collectionIndex);
        int weight = SkTo<int>(font.getWeight());
        SkFontStyle::Slant slant = style.slant();
        if (font.isItalic()) {
            slant = SkFontStyle::kItalic_Slant;
        }

        sk_sp<SkTypeface> typeface;
        sk_sp<SkTypeface> typefaceWithRightFile;
        // Find an existing SkTypeface which matches this AFont.
        for (const sk_sp<SkFontStyleSet_Android>& styleSet : fStyleSets) {
            for (const sk_sp<SkTypeface_AndroidSystem>& candidate : styleSet->fStyles) {
                if (candidate->fPathName.equals(filePath, filePathLen)) {
                    if (candidate->fIndex == ttcIndex &&
                        candidate->fontStyle().weight() == weight &&
                        candidate->fontStyle().slant() == slant
                        /* TODO: && requested axes (required, some only very on width which is not reported) and locale are same? */)
                    {
                        typeface = candidate;
                        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: Exact typeface match"); }
                    } else if (!typefaceWithRightFile) {
                        typefaceWithRightFile = candidate;
                    }
                }
            }
        }
        if (!typeface && typefaceWithRightFile) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: Partial typeface match"); }
            auto stream = typefaceWithRightFile->openExistingStream(nullptr);
            typeface = this->make(std::move(font), std::move(stream), false);
        }
        // This may not be possible in a tight sandbox.
        if (!typeface) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: No typeface match"); }
            typeface = this->make(std::move(font), false);
        }

        if (typeface) {
            return typeface.release();
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

        SkAutoSTMalloc<4, SkFixed> axisValues(axisDefinitions.count());
        Scanner::computeAxisValues(axisDefinitions, args.getVariationDesignPosition(),
                                   axisValues, name);

        auto data = std::make_unique<SkFontData>(
            std::move(stream), args.getCollectionIndex(), args.getPalette().index,
            axisValues.get(), axisDefinitions.count(),
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
        return sk_sp<SkTypeface>(fDefaultStyleSet->matchStyle(style));
    }


private:
    const AndroidFontAPI& fAPI;

    SkTypeface_FreeType::Scanner fScanner;

    SkTArray<sk_sp<SkFontStyleSet_Android>> fStyleSets;
    sk_sp<SkFontStyleSet> fDefaultStyleSet;

    SkTArray<NameToFamily, true> fNameToFamilyMap;

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
