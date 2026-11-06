/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#include "include/core/SkData.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontScanner.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/ports/SkFontMgr_android.h"
#include "include/private/SkFixed.h"
#include "include/private/SkMutex.h"
#include "include/private/SkOnce.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTDArray.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTHash.h"
#include "src/core/SkTSearch.h"
#include "src/core/SkTypefaceCache.h"
#include "src/ports/SkFontMgr_android_parser.h"
#include "src/ports/SkTypeface_proxy.h"

#include <algorithm>
#include <array>
#include <limits>

using namespace skia_private;

class SkData;

namespace {

[[maybe_unused]] static inline const constexpr bool kSkFontMgrVerbose = false;

class SkTypeface_AndroidSystem : public SkTypeface_proxy {
public:
    static sk_sp<SkTypeface_AndroidSystem> Make(sk_sp<SkTypeface> realTypeface,
                                                const SkFontStyle& style,
                                                bool isFixedPitch,
                                                const SkString& familyName,
                                                const TArray<SkLanguage, true>& lang,
                                                FontVariant variantStyle) {
        SkASSERT(realTypeface);
        return sk_sp(new SkTypeface_AndroidSystem(std::move(realTypeface),
                                                  style, isFixedPitch,
                                                  familyName, lang, variantStyle));
    }

    const SkString fFamilyName;
    const STArray<4, SkLanguage, true> fLang;
    const FontVariant fVariantStyle;

protected:
    SkTypeface_AndroidSystem(sk_sp<SkTypeface> realTypeface,
                             const SkFontStyle& style,
                             bool isFixedPitch,
                             const SkString& familyName,
                             const TArray<SkLanguage, true>& lang,
                             FontVariant variantStyle)
            : SkTypeface_proxy(std::move(realTypeface), style, isFixedPitch)
            , fFamilyName(familyName)
            , fLang(lang)
            , fVariantStyle(variantStyle)
    {}

    void onGetFamilyName(SkString* familyName) const override {
        *familyName = fFamilyName;
    }

    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        auto proxy = SkTypeface_proxy::onMakeClone(args);
        if (proxy == nullptr) {
            return nullptr;
        }
        return SkTypeface_AndroidSystem::Make(std::move(proxy),
                                              this->fontStyle(), this->isFixedPitch(),
                                              fFamilyName, fLang, fVariantStyle);
    }

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const override {
        SkTypeface_proxy::onGetFontDescriptor(desc, serialize);

        SkASSERT(desc);
        SkASSERT(serialize);
        desc->setFamilyName(fFamilyName.c_str());
        desc->setStyle(this->fontStyle());
        *serialize = false;
    }

    SkFontStyle onGetFontStyle() const override {
        return SkTypeface::onGetFontStyle();
    }

    bool onGetFixedPitch() const override {
        return SkTypeface::onGetFixedPitch();
    }
};

template <typename D, typename S> sk_sp<D> sk_sp_static_cast(sk_sp<S>&& s) {
    return sk_sp<D>(static_cast<D*>(s.release()));
}

using StreamForPathCache = skia_private::THashMap<SkString, std::unique_ptr<SkStreamAsset>>;

// Style sets can outlive the manager (callers keep sk_sp<SkFontStyleSet>), and their lazy
// entries share one mmap per materialized font path across TTC faces and variation instances
struct SharedScanner : public SkNVRefCnt<SharedScanner> {
    explicit SharedScanner(std::unique_ptr<SkFontScanner> scanner) : fScanner(std::move(scanner)) {}

    std::unique_ptr<SkStreamAsset> openStream(const SkString& pathName) {
        SkAutoMutexExclusive lock(fStreamMutex);
        if (std::unique_ptr<SkStreamAsset>* cached = fStreamForPath.find(pathName)) {
            return (*cached)->duplicate();
        }
        std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(pathName.c_str());
        if (!stream) {
            return nullptr;
        }
        return (*fStreamForPath.set(pathName, std::move(stream)))->duplicate();
    }

    const std::unique_ptr<SkFontScanner> fScanner;
    SkMutex fStreamMutex;
    StreamForPathCache fStreamForPath SK_GUARDED_BY(fStreamMutex);
};

static SkFontArguments font_arguments(const FontFileInfo& fontFile) {
    const SkFontArguments::VariationPosition position = {fontFile.fVariationDesignPosition.begin(),
                                                         fontFile.fVariationDesignPosition.size()};
    return SkFontArguments()
            .setCollectionIndex(fontFile.fIndex)
            .setVariationDesignPosition(position);
}

// fonts.xml weight and style take precedence over what the font file reports
static SkFontStyle apply_xml_style(const FontFileInfo& fontFile, SkFontStyle fontStyle) {
    int weight = fontFile.fWeight != 0 ? fontFile.fWeight : fontStyle.weight();
    SkFontStyle::Slant slant = fontStyle.slant();
    switch (fontFile.fStyle) {
        case FontFileInfo::Style::kAuto:
            break;
        case FontFileInfo::Style::kNormal:
            slant = SkFontStyle::kUpright_Slant;
            break;
        case FontFileInfo::Style::kItalic:
            slant = SkFontStyle::kItalic_Slant;
            break;
        default:
            SkASSERT(false);
            break;
    }
    return SkFontStyle(weight, fontStyle.width(), slant);
}

class SkFontStyleSet_Android : public SkFontStyleSet {
    // fStyle is final at construction so matching never opens a font; fTypeface is created at
    // most once and stays null if a lazily opened file turns out to be unusable
    struct Entry {
        SkString fPathName;
        FontFileInfo fFontFile;
        SkFontStyle fStyle;
        SkOnce fOnce;
        sk_sp<SkTypeface_AndroidSystem> fTypeface;
    };

public:
    SkFontStyleSet_Android(const FontFamily& family,
                           sk_sp<SharedScanner> scanner,
                           bool lazy,
                           StreamForPathCache& streamForPath)
            : fScanner(std::move(scanner))
            , fFamilyName(family.fNames.empty() ? SkString() : family.fNames[0])
            , fLanguages(family.fLanguages)
            , fVariant(kDefault_FontVariant == family.fVariant
                               ? kCompact_FontVariant | kElegant_FontVariant
                               : family.fVariant)
            , fFallbackFor(family.fFallbackFor) {
        for (const FontFileInfo& fontFile : family.fFonts) {
            SkString pathName(family.fBasePath);
            pathName.append(fontFile.fFileName);

            std::unique_ptr<SkStreamAsset>* streamPtr = streamForPath.find(pathName);
            if (!streamPtr) {
                streamPtr = streamForPath.set(pathName, SkStream::MakeFromFile(pathName.c_str()));
            }
            if (!*streamPtr) {
                SkDEBUGF("Requested font file %s cannot be opened.\n", pathName.c_str());
                continue;
            }

            auto entry = std::make_unique<Entry>();
            entry->fPathName = pathName;
            entry->fFontFile = fontFile;

            SkFontStyle scannedStyle;
            if (lazy && fScanner->fScanner->scanStyle(
                                streamPtr->get(), font_arguments(fontFile), &scannedStyle)) {
                entry->fStyle = apply_xml_style(fontFile, scannedStyle);
                fEntries.push_back(std::move(entry));
                continue;
            }

            std::unique_ptr<SkStreamAsset> stream = (*streamPtr)->duplicate();
            if (!stream) {
                SkDEBUGF("Requested font file %s could not be duplicated.\n", pathName.c_str());
                continue;
            }
            // TODO: this creates the proxy with the given stream, so always cacheFontFiles.
            sk_sp<SkTypeface> real =
                    fScanner->fScanner->MakeFromStream(std::move(stream), font_arguments(fontFile));
            if (!real) {
                SkDEBUGF("Requested font file %s does not have valid font data.\n",
                         pathName.c_str());
                continue;
            }
            entry->fStyle = apply_xml_style(fontFile, real->fontStyle());
            sk_sp<SkTypeface_AndroidSystem> typeface = this->wrap(std::move(real), entry->fStyle);
            entry->fOnce([&] { entry->fTypeface = std::move(typeface); });
            fEntries.push_back(std::move(entry));
        }
    }

    int count() override { return fEntries.size(); }
    void getStyle(int index, SkFontStyle* style, SkString* name) override {
        if (index < 0 || fEntries.size() <= index) {
            return;
        }
        if (style) {
            *style = fEntries[index]->fStyle;
        }
        if (name) {
            name->reset();
        }
    }
    sk_sp<SkTypeface> createTypeface(int index) override {
        if (index < 0 || fEntries.size() <= index) {
            return nullptr;
        }
        return this->typeface(index);
    }

    // May return nullptr when the best style fails to open lazily
    sk_sp<SkTypeface_AndroidSystem> matchAStyle(const SkFontStyle& pattern) {
        return sk_sp_static_cast<SkTypeface_AndroidSystem>(this->matchStyleCSS3(pattern));
    }
    sk_sp<SkTypeface> matchStyle(const SkFontStyle& pattern) override {
        return this->matchAStyle(pattern);
    }

private:
    sk_sp<SkTypeface_AndroidSystem> typeface(int index) {
        Entry& entry = *fEntries[index];
        entry.fOnce([&] {
            std::unique_ptr<SkStreamAsset> stream = fScanner->openStream(entry.fPathName);
            sk_sp<SkTypeface> real =
                    stream ? fScanner->fScanner->MakeFromStream(std::move(stream),
                                                                font_arguments(entry.fFontFile))
                           : nullptr;
            if (!real) {
                SkDEBUGF("Font file %s failed to open lazily.\n", entry.fPathName.c_str());
                return;
            }
            SkASSERTF(apply_xml_style(entry.fFontFile, real->fontStyle()) == entry.fStyle,
                      "scanStyle diverged from MakeFromStream for %s",
                      entry.fPathName.c_str());
            entry.fTypeface = this->wrap(std::move(real), entry.fStyle);
        });
        return entry.fTypeface;
    }

    sk_sp<SkTypeface_AndroidSystem> wrap(sk_sp<SkTypeface> real, SkFontStyle style) const {
        SkString familyName = fFamilyName;
        if (familyName.isEmpty()) {
            real->getFamilyName(&familyName);
        }
        const bool isFixedPitch = real->isFixedPitch();
        return SkTypeface_AndroidSystem::Make(
                std::move(real), style, isFixedPitch, familyName, fLanguages, fVariant);
    }

    const sk_sp<SharedScanner> fScanner;
    const SkString fFamilyName;
    const STArray<4, SkLanguage, true> fLanguages;
    const FontVariant fVariant;
    SkString fFallbackFor;
    // unique_ptr because SkOnce is neither copyable nor movable
    TArray<std::unique_ptr<Entry>> fEntries;

    friend struct NameToFamily;
    friend class SkFontMgr_Android;
};

struct NameToFamily {
    SkString name;
    SkFontStyleSet_Android* styleSet;
};

class SkFontMgr_Android : public SkFontMgr {
public:
    SkFontMgr_Android(const SkFontMgr_Android_CustomFonts* custom,
                      std::unique_ptr<SkFontScanner> scanner,
                      SkFontMgr_Android_FontLoading loading)
            : fScanner(sk_make_sp<SharedScanner>(std::move(scanner))) {
        // Isolated managers promise no file IO after construction
        const bool lazy =
                SkFontMgr_Android_FontLoading::kLazy == loading && !(custom && custom->fIsolated);
        std::vector<std::unique_ptr<FontFamily>> families;
        if (custom && SkFontMgr_Android_CustomFonts::kPreferSystem != custom->fSystemFontUse) {
            SkString base(custom->fBasePath);
            SkFontMgr_Android_Parser::GetCustomFontFamilies(
                families, base, custom->fFontsXml, custom->fFallbackFontsXml);
        }
        if (!custom ||
            (custom && SkFontMgr_Android_CustomFonts::kOnlyCustom != custom->fSystemFontUse))
        {
            SkFontMgr_Android_Parser::GetSystemFontFamilies(families);
        }
        if (custom && SkFontMgr_Android_CustomFonts::kPreferSystem == custom->fSystemFontUse) {
            SkString base(custom->fBasePath);
            SkFontMgr_Android_Parser::GetCustomFontFamilies(
                families, base, custom->fFontsXml, custom->fFallbackFontsXml);
        }
        this->buildNameToFamilyMap(families, lazy);
        this->findDefaultStyleSet();
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

    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override {
        if (index < 0 || fNameToFamilyMap.size() <= index) {
            return nullptr;
        }
        return sk_ref_sp(fNameToFamilyMap[index].styleSet);
    }

    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override {
        if (!familyName) {
            return nullptr;
        }
        SkAutoAsciiToLC tolc(familyName);
        for (int i = 0; i < fNameToFamilyMap.size(); ++i) {
            if (fNameToFamilyMap[i].name.equals(tolc.lc())) {
                return sk_ref_sp(fNameToFamilyMap[i].styleSet);
            }
        }
        // TODO: eventually we should not need to name fallback families.
        for (int i = 0; i < fFallbackNameToFamilyMap.size(); ++i) {
            if (fFallbackNameToFamilyMap[i].name.equals(tolc.lc())) {
                return sk_ref_sp(fFallbackNameToFamilyMap[i].styleSet);
            }
        }
        return nullptr;
    }

    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                         const SkFontStyle& style) const override {
        sk_sp<SkFontStyleSet> sset(this->matchFamily(familyName));
        return sset->matchStyle(style);
    }

    using Matcher = bool (*)(const SkString& familyName, const NameToFamily&);
    static sk_sp<SkTypeface_AndroidSystem> find_family_style_character(
            const SkString& familyName,
            const TArray<NameToFamily, true>& nameToFamilyMap,
            const SkFontStyle& style, bool elegant,
            const SkString& langTag, SkUnichar character,
            Matcher matcher)
    {
        for (auto&& nameToFamily : nameToFamilyMap) {
            if (!matcher(familyName, nameToFamily)) {
                continue;
            }
            // Language and variant are per family, so reject before opening any font file
            const SkFontStyleSet_Android& set = *nameToFamily.styleSet;
            if (!langTag.isEmpty() &&
                std::none_of(
                        set.fLanguages.begin(), set.fLanguages.end(), [&](const SkLanguage& lang) {
                            return lang.getTag().startsWith(langTag.c_str());
                        })) {
                continue;
            }
            if (SkToBool(set.fVariant & kElegant_FontVariant) != elegant) {
                continue;
            }

            sk_sp<SkTypeface_AndroidSystem> face(nameToFamily.styleSet->matchAStyle(style));
            // A family whose matched style failed to open lazily cannot cover any character
            if (!face) {
                continue;
            }

            if (face->unicharToGlyph(character) != 0) {
                return face;
            }
        }
        return nullptr;
    }

    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[],
                                                  const SkFontStyle& style,
                                                  const char* bcp47[],
                                                  int bcp47Count,
                                                  SkUnichar character) const override {
        // The variant 'elegant' is 'not squashed', 'compact' is 'stays in ascent/descent'.
        // The variant 'default' means 'compact and elegant'.
        // As a result, it is not possible to know the variant context from the font alone.
        // TODO: add 'is_elegant' and 'is_compact' bits to 'style' request.

        // The order of family name matching is
        // 1. named font that matches familyName (if familyName not empty)
        // 2. fallback font with fallback-for that matches familyName (if familyName not empty)
        // 3. fallback font with fallback-for which is empty
        // 4. named font which does not match familyName
        // 5. fallback font with fallback-for which does not match familyName or empty

        auto find = [this, &style, &character](const SkString& familyName,
                                               const SkString& lang,
                                               bool elegant) -> sk_sp<SkTypeface_AndroidSystem>
        {
            sk_sp<SkTypeface_AndroidSystem> matchingTypeface;
            if (!familyName.isEmpty()) {
                matchingTypeface = find_family_style_character(
                    familyName, fNameToFamilyMap, style, elegant, lang, character,
                    [](const SkString& familyName, const NameToFamily& nameToFamily) {
                        return familyName == nameToFamily.name;
                    });
                if (matchingTypeface) {
                    return matchingTypeface;
                }
                matchingTypeface = find_family_style_character(
                    familyName, fFallbackNameToFamilyMap, style, elegant, lang, character,
                    [](const SkString& familyName, const NameToFamily& nameToFamily) {
                        return familyName == nameToFamily.styleSet->fFallbackFor;
                    });
                if (matchingTypeface) {
                    return matchingTypeface;
                }
            }
            matchingTypeface = find_family_style_character(
                SkString(), fFallbackNameToFamilyMap, style, elegant, lang, character,
                [](const SkString&, const NameToFamily& nameToFamily) {
                    return nameToFamily.styleSet->fFallbackFor.isEmpty();
                });
            if (matchingTypeface) {
                return matchingTypeface;
            }
            matchingTypeface = find_family_style_character(
                familyName, fNameToFamilyMap, style, elegant, lang, character,
                [](const SkString& familyName, const NameToFamily& nameToFamily) {
                    return familyName != nameToFamily.name;
                });
            if (matchingTypeface) {
                return matchingTypeface;
            }
            matchingTypeface = find_family_style_character(
                familyName, fFallbackNameToFamilyMap, style, elegant, lang, character,
                [](const SkString& familyName, const NameToFamily& nameToFamily) {
                    return familyName != nameToFamily.styleSet->fFallbackFor &&
                           !nameToFamily.styleSet->fFallbackFor.isEmpty();
                });
            if (matchingTypeface) {
                return matchingTypeface;
            }
            return nullptr;
        };


        sk_sp<SkTypeface_AndroidSystem> matchingTypeface;
        // If familyName is empty, don't look through everything twice.
        SkString familyNames[2] = { SkString(familyName), SkString() };
        size_t familyNamesCount = familyNames[0].isEmpty() ? 1 : 2;
        for (const SkString& currentFamilyName : SkSpan(familyNames, familyNamesCount)) {
            for (bool elegant : {true, false}) {
                for (int bcp47Index = bcp47Count; bcp47Index --> 0;) {
                    SkLanguage lang(bcp47[bcp47Index]);
                    while (!lang.getTag().isEmpty()) {
                        matchingTypeface = find(currentFamilyName, lang.getTag(), elegant);
                        if (matchingTypeface) {
                            return matchingTypeface;
                        }

                        lang = lang.getParent();
                    }
                }
                matchingTypeface = find(currentFamilyName, SkString(), elegant);
                if (matchingTypeface) {
                    return matchingTypeface;
                }
            }
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
        return this->makeFromStream(std::move(stream),
                                    SkFontArguments().setCollectionIndex(ttcIndex));
    }

    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                           const SkFontArguments& args) const override {
        return fScanner->fScanner->MakeFromStream(std::move(stream), args);
    }

    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle style) const override {
        if (familyName) {
            // On Android, we must return nullptr when we can't find the requested
            // named typeface so that the system/app can provide their own recovery
            // mechanism. On other platforms we'd provide a typeface from the
            // default family instead.
            return sk_sp<SkTypeface>(this->onMatchFamilyStyle(familyName, style));
        }
        if (fDefaultStyleSet) {
            if (sk_sp<SkTypeface> typeface = fDefaultStyleSet->matchStyle(style)) {
                return typeface;
            }
        }
        return SkTypeface::MakeEmpty();
    }


private:
    sk_sp<SharedScanner> fScanner;

    TArray<sk_sp<SkFontStyleSet_Android>> fStyleSets;
    sk_sp<SkFontStyleSet> fDefaultStyleSet;

    TArray<NameToFamily, true> fNameToFamilyMap;
    TArray<NameToFamily, true> fFallbackNameToFamilyMap;

    void addFamily(FontFamily& family,
                   bool lazy,
                   int familyIndex,
                   StreamForPathCache& streamForPath) {
        TArray<NameToFamily, true>* nameToFamily = &fNameToFamilyMap;
        if (family.fIsFallbackFont) {
            nameToFamily = &fFallbackNameToFamilyMap;

            if (family.fNames.empty()) {
                SkString& fallbackName = family.fNames.push_back();
                fallbackName.printf("%.2x##fallback", (uint32_t)familyIndex);
            }
        }

        sk_sp<SkFontStyleSet_Android> newSet =
                sk_make_sp<SkFontStyleSet_Android>(family, fScanner, lazy, streamForPath);
        if (0 == newSet->count()) {
            return;
        }

        for (const SkString& name : family.fNames) {
            nameToFamily->emplace_back(NameToFamily{name, newSet.get()});
        }
        fStyleSets.emplace_back(std::move(newSet));
    }
    // Lazy entries keep no stream; the mappings opened here are released on return
    void buildNameToFamilyMap(std::vector<std::unique_ptr<FontFamily>>& families, bool lazy) {
        StreamForPathCache streamForPath;
        int familyIndex = 0;
        for (std::unique_ptr<FontFamily>& family : families) {
            addFamily(*family, lazy, familyIndex++, streamForPath);
            for (const auto& [unused, fallbackFamily] : family->fallbackFamilies) {
                addFamily(*fallbackFamily, lazy, familyIndex++, streamForPath);
            }
        }
    }

    void findDefaultStyleSet() {
        static const char* defaultNames[] = { "sans-serif" };
        for (const char* defaultName : defaultNames) {
            fDefaultStyleSet = this->onMatchFamily(defaultName);
            if (fDefaultStyleSet) {
                break;
            }
        }
        if (!fDefaultStyleSet && !fStyleSets.empty()) {
            fDefaultStyleSet = fStyleSets[0];
        }
    }

    using INHERITED = SkFontMgr;
};

#ifdef SK_DEBUG
static constexpr auto gSystemFontUseStrings = std::to_array<const char*>({
        "OnlyCustom",
        "PreferCustom",
        "PreferSystem",
});
#endif

}  // namespace

sk_sp<SkFontMgr> SkFontMgr_New_Android(const SkFontMgr_Android_CustomFonts* custom,
                                       std::unique_ptr<SkFontScanner> scanner) {
    return SkFontMgr_New_Android(custom, std::move(scanner), SkFontMgr_Android_FontLoading::kEager);
}

sk_sp<SkFontMgr> SkFontMgr_New_Android(const SkFontMgr_Android_CustomFonts* custom,
                                       std::unique_ptr<SkFontScanner> scanner,
                                       SkFontMgr_Android_FontLoading loading) {
    if (custom) {
        SkASSERT(0 <= custom->fSystemFontUse);
        SkASSERT(custom->fSystemFontUse < std::size(gSystemFontUseStrings));
        if constexpr (kSkFontMgrVerbose) {
            SkDEBUGF("SystemFontUse: %s BasePath: %s Fonts: %s FallbackFonts: %s\n",
                     gSystemFontUseStrings[custom->fSystemFontUse],
                     custom->fBasePath,
                     custom->fFontsXml,
                     custom->fFallbackFontsXml);
        }
    }
    return sk_make_sp<SkFontMgr_Android>(custom, std::move(scanner), loading);
}
