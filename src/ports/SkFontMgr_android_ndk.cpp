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

static AndroidFontAPI* GetAndroidFontAPI() {
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

static AndroidFontAPI* GetAndroidFontAPI() {
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
                SkDebugf("SKIA: Failed to load: " #NAME);    \
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

class SkTypeface_Android : public SkTypeface_FreeType {
public:
    SkTypeface_Android(const SkFontStyle& style,
                       bool isFixedPitch,
                       const SkString& familyName)
        : INHERITED(style, isFixedPitch)
        , fFamilyName(familyName)
        { }

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
    SkTypeface_AndroidSystem(const SkString& pathName,
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
        , fFile(cacheFontFiles ? sk_fopen(fPathName.c_str(), kRead_SkFILE_Flag) : nullptr) {
        if (cacheFontFiles) {
            SkASSERT(fFile);
        }
    }

    std::unique_ptr<SkStreamAsset> makeStream() const {
        if (fFile) {
            sk_sp<SkData> data(SkData::MakeFromFILE(fFile));
            return data ? std::make_unique<SkMemoryStream>(std::move(data)) : nullptr;
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
        return sk_make_sp<SkTypeface_AndroidSystem>(fPathName,
                                                    fFile,  // TODO: whoops
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
    SkAutoTCallVProc<FILE, sk_fclose> fFile;

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
public:
    SkFontMgr_Android_NDK(AndroidFontAPI* androidFontAPI, bool cacheFontFiles) : fAPI(androidFontAPI) {
        SkASSERT_RELEASE(fAPI);

        SkDebugf("SKIA: ASystemFontIterator_open");
        ASystemFontIterator* fontIter = fAPI->ASystemFontIterator_open();
        if (!fontIter) {
            return;
        }

        while (AFont* font = fAPI->ASystemFontIterator_next(fontIter)) {
            sk_sp<SkTypeface_AndroidSystem> typeface = this->make(font, cacheFontFiles);

            // Add to an existing syle set or a new one.
            SkString name;
            typeface->getFamilyName(&name);

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
            SkDebugf("SKIA: Creating typeface %s", name.c_str());
            nameToFamily->styleSet->fStyles.push_back(std::move(typeface));
        }

        SkDebugf("SKIA: ASystemFontIterator_close");
        fAPI->ASystemFontIterator_close(fontIter);

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

    sk_sp<SkTypeface_AndroidSystem> make(AFont* font, bool cacheFontFiles) const {
        const char* filePath = fAPI->AFont_getFontFilePath(font);

        std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(filePath);
        if (!stream) {
            SkDEBUGF("Requested font file %s does not exist or cannot be opened.\n", filePath);
            return nullptr;
        }

        return this->make(font, std::move(stream), cacheFontFiles);
    }
    sk_sp<SkTypeface_AndroidSystem> make(AFont* font, std::unique_ptr<SkStreamAsset> stream, bool cacheFontFiles) const {
        const char* filePath = fAPI->AFont_getFontFilePath(font);

        size_t collectionIndex = fAPI->AFont_getCollectionIndex(font);
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
            SkDEBUGF("Requested font file %s exists, but is not a valid font.\n", filePath);
            return nullptr;
        }

        int weight = SkTo<int>(fAPI->AFont_getWeight(font));
        SkFontStyle::Slant slant = style.slant();
        if (fAPI->AFont_isItalic(font)) {
            slant = SkFontStyle::kItalic_Slant;
        }
        style = SkFontStyle(weight, style.width(), slant);

        // The family name is not reported.

        size_t requestedAxisCount = fAPI->AFont_getAxisCount(font);
        if (!SkTFitsIn<int>(requestedAxisCount)) {
            return nullptr;
        }
        SkAutoSTMalloc<4, SkFontArguments::VariationPosition::Coordinate> requestedAxisValues(requestedAxisCount);
        for (size_t i = 0; i < requestedAxisCount; ++i) {
            requestedAxisValues[i] = { fAPI->AFont_getAxisTag(font, i), fAPI->AFont_getAxisValue(font, i) };
        }

        SkAutoSTMalloc<4, SkFixed> axisValues(axisDefinitions.count());
        SkFontArguments::VariationPosition position = {
            requestedAxisValues.get(), SkTo<int>(requestedAxisCount)
        };
        SkTypeface_FreeType::Scanner::computeAxisValues(axisDefinitions, position,
                                                        axisValues, familyName);

        const char* langs = fAPI->AFont_getLocale(font);
        //TODO: split langs on ',' and pass an languages.
        (void)langs;

        return sk_sp<SkTypeface_AndroidSystem>(new SkTypeface_AndroidSystem(
                SkString(filePath), false, ttcIndex, axisValues.get(), axisDefinitions.count(),
                style, isFixedWidth, familyName/*, SkTArray<SkLanguage, true>()*/));
    }

    SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                            const SkFontStyle& style,
                                            const char* bcp47[],
                                            int bcp47Count,
                                            SkUnichar character) const override {
        SkDebugf("SKIA: AFontMatcher_create");
        AFontMatcher* matcher = fAPI->AFontMatcher_create();

        if (familyName == nullptr) {
            // AFontMatcher_match requires that the family name be non-nullptr.
            SkDebugf("SKIA: Matcher rescuing nullptr");
            familyName = "";
        }

        if (bcp47Count > 0) {
            // TODO: expects comma separated '\0' terminated string of locales.
            fAPI->AFontMatcher_setLocales(matcher, bcp47[0]);
        }
        fAPI->AFontMatcher_setFamilyVariant(matcher, AndroidFontAPI::AFAMILY_VARIANT_DEFAULT);
        fAPI->AFontMatcher_setStyle(matcher, style.weight(), style.slant() != SkFontStyle::kUpright_Slant);
        uint16_t utf16[2];
        size_t utf16Size = SkUTF::ToUTF16(character, utf16);
        SkDebugf("SKIA: AFontMatcher_match(matcher, \"%s\", %d, 1)", familyName, utf16[0]);
        AFont* font = fAPI->AFontMatcher_match(matcher, familyName, utf16, utf16Size, nullptr);

        const char* filePath = fAPI->AFont_getFontFilePath(font);
        SkDebugf("SKIA: AFontMatcher_match matched: %s", filePath);
        size_t filePathLen = strlen(filePath);
        size_t collectionIndex = fAPI->AFont_getCollectionIndex(font);
        if (!SkTFitsIn<int>(collectionIndex)) {
            SkDebugf("SKIA: Bailing early");
            // TODO: auto close and destroy
            fAPI->AFont_close(font);
            fAPI->AFontMatcher_destroy(matcher);
            return nullptr;
        }
        const int ttcIndex = SkTo<int>(collectionIndex);
        int weight = SkTo<int>(fAPI->AFont_getWeight(font));
        SkFontStyle::Slant slant = style.slant();
        if (fAPI->AFont_isItalic(font)) {
            slant = SkFontStyle::kItalic_Slant;
        }

        sk_sp<SkTypeface> typeface;
        sk_sp<SkTypeface> typefaceWithRightFile;
        // find in cache an SkTypeface which matches this AFont.
        for (const sk_sp<SkFontStyleSet_Android>& styleSet : fStyleSets) {
            for (const sk_sp<SkTypeface_AndroidSystem>& candidate : styleSet->fStyles) {
                if (candidate->fPathName.equals(filePath, filePathLen)) {
                    if (candidate->fIndex == ttcIndex &&
                        candidate->fontStyle().weight() == weight &&
                        candidate->fontStyle().slant() == slant
                        /* && axes and locale are same? */)
                    {
                        typeface = candidate;
                        SkDebugf("SKIA: Exact typeface match");
                    } else if (!typefaceWithRightFile) {
                        typefaceWithRightFile = candidate;
                    }
                }
            }
        }
        if (!typeface && typefaceWithRightFile) {
            SkDebugf("SKIA: Partial typeface match");
            typeface = this->make(font, typefaceWithRightFile->openExistingStream(nullptr), false);
        }
        // This may not be possible in the Chromium sandbox.
        if (!typeface) {
            SkDebugf("SKIA: No typeface match");
            typeface = this->make(font, false);
        }

        fAPI->AFont_close(font);
        SkDebugf("SKIA: AFontMatcher_destroy");
        fAPI->AFontMatcher_destroy(matcher);
        if (typeface) {
            return typeface.release();
        }
        return nullptr;
    }

    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override {
        return this->makeFromStream(std::unique_ptr<SkStreamAsset>(new SkMemoryStream(std::move(data))),
                                    ttcIndex);
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

    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle style) const override {
        if (familyName) {
            // On Android, we must return nullptr when we can't find the requested
            // named typeface so that the system/app can provide their own recovery
            // mechanism. On other platforms we'd provide a typeface from the
            // default family instead.
            return sk_sp<SkTypeface>(this->onMatchFamilyStyle(familyName, style));
        }
        return sk_sp<SkTypeface>(fDefaultStyleSet->matchStyle(style));
    }


private:
    AndroidFontAPI* fAPI;

    SkTypeface_FreeType::Scanner fScanner;

    SkTArray<sk_sp<SkFontStyleSet_Android>> fStyleSets;
    sk_sp<SkFontStyleSet> fDefaultStyleSet;

    SkTArray<NameToFamily, true> fNameToFamilyMap;

    void findDefaultStyleSet() {
        SkASSERT(!fStyleSets.empty());

        static const char* defaultNames[] = { "sans-serif" };
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
    AndroidFontAPI* androidFontAPI = GetAndroidFontAPI();
    if (!androidFontAPI) {
        return nullptr;
    }
    return sk_make_sp<SkFontMgr_Android_NDK>(androidFontAPI, cacheFontFiles);
}
