// Copyright 2026 Google LLC
#include "modules/skparagraph/include/FontCollection.h"

#include "include/core/SkFontMgr.h"
#include "include/core/SkTypeface.h"
#include "modules/skparagraph/src/FontCollectionFaceCache.h"

#include <vector>

namespace skia {
namespace textlayout {

std::vector<sk_sp<SkTypeface>> FontCollection::findCoreTextTypefaces(
        const std::vector<SkString>& familyNames,
        SkFontStyle fontStyle,
        const std::optional<FontArguments>& fontArgs,
        SystemFamilyLookup systemFamily) {
    FaceCache::FamilyKey familyKey(familyNames, fontStyle, fontArgs, /*coreText=*/true);
    if (auto cached = fFaceCache->find(familyKey)) {
        return std::move(*cached);
    }

    std::vector<sk_sp<SkTypeface>> typefaces;
    for (const SkString& familyName : familyNames) {
        if (sk_sp<SkTypeface> match =
                    this->matchCoreTextFamily(familyName, fontStyle, systemFamily)) {
            typefaces.push_back(std::move(match));
        }
    }
    if (typefaces.empty()) {
        if (sk_sp<SkTypeface> standIn = this->coreTextStandIn(fontStyle, systemFamily)) {
            typefaces.push_back(std::move(standIn));
        }
    }
    if (fontArgs) {
        // SkTypeface::makeClone returns null when variation cloning fails; drop null entries
        // as FontCollection::findTypefaces does so cascade and allowlists hold valid faces
        std::erase_if(typefaces, [&](sk_sp<SkTypeface>& typeface) {
            typeface = this->cloneTypeface(typeface, *fontArgs);
            return !typeface;
        });
    }
    return fFaceCache->insert(familyKey, std::move(typefaces));
}

sk_sp<SkTypeface> FontCollection::matchCoreTextFamily(const SkString& familyName,
                                                      SkFontStyle fontStyle,
                                                      SystemFamilyLookup systemFamily) {
    // A face the client registered under a family name wins over an installed one of the same
    // name, in the same dynamic -> asset -> test order as getFontManagerOrder
    for (SkFontMgr* manager :
         {fDynamicFontManager.get(), fAssetFontManager.get(), fTestFontManager.get()}) {
        if (!manager) {
            continue;
        }
        sk_sp<SkFontStyleSet> set(manager->matchFamily(familyName.c_str()));
        if (!set || set->count() == 0) {
            continue;
        }
        if (sk_sp<SkTypeface> match = set->matchStyle(fontStyle)) {
            return match;
        }
    }
    // When the client registered user font managers without a defaultFontManager (e.g. isolated
    // asset-only tests), do not resolve system families ahead of later user families in the list
    if (!fDefaultFontManager && (fDynamicFontManager || fAssetFontManager || fTestFontManager)) {
        return nullptr;
    }
    // disableFontFallback drops the default manager from getFontManagerOrder, so FontCollection
    // sees only user faces; installed families stay hidden here for the same reason
    if (!fEnableFontFallback) {
        return nullptr;
    }
    return systemFamily(familyName, fontStyle);
}

sk_sp<SkTypeface> FontCollection::coreTextStandIn(SkFontStyle fontStyle,
                                                  SystemFamilyLookup systemFamily) {
    for (const SkString& familyName : fDefaultFamilyNames) {
        if (sk_sp<SkTypeface> match =
                    this->matchCoreTextFamily(familyName, fontStyle, systemFamily)) {
            return match;
        }
    }
    // Same order as getFontManagerOrder: user registries give the stand-in first; the
    // system one stands in last and only with fallback enabled
    for (SkFontMgr* manager :
         {fDynamicFontManager.get(), fAssetFontManager.get(), fTestFontManager.get()}) {
        if (!manager) {
            continue;
        }
        if (sk_sp<SkTypeface> match = manager->legacyMakeTypeface(nullptr, fontStyle)) {
            return match;
        }
    }
    if (!fEnableFontFallback) {
        return nullptr;
    }
    return systemFamily(SkString(".AppleSystemUIFont"), fontStyle);
}

}  // namespace textlayout
}  // namespace skia
