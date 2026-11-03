// Copyright 2019 Google LLC
#ifndef FontCollection_DEFINED
#define FontCollection_DEFINED

#include <memory>
#include <optional>
#include <set>
#include "include/core/SkFontMgr.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "modules/skparagraph/include/FontArguments.h"
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/include/TextStyle.h"

namespace skia {
namespace textlayout {

class Paragraph;
class TextStyle;
class FontCollection : public SkRefCnt {
public:
    FontCollection();
    ~FontCollection() override;

    size_t getFontManagersCount() const;

    void setAssetFontManager(sk_sp<SkFontMgr> fontManager);
    void setDynamicFontManager(sk_sp<SkFontMgr> fontManager);
    void setTestFontManager(sk_sp<SkFontMgr> fontManager);
    void setDefaultFontManager(sk_sp<SkFontMgr> fontManager);
    void setDefaultFontManager(sk_sp<SkFontMgr> fontManager, const char defaultFamilyName[]);
    void setDefaultFontManager(sk_sp<SkFontMgr> fontManager, const std::vector<SkString>& defaultFamilyNames);

    sk_sp<SkFontMgr> getFallbackManager() const { return fDefaultFontManager; }

    std::vector<sk_sp<SkTypeface>> findTypefaces(const std::vector<SkString>& familyNames, SkFontStyle fontStyle);
    std::vector<sk_sp<SkTypeface>> findTypefaces(const std::vector<SkString>& familyNames, SkFontStyle fontStyle, const std::optional<FontArguments>& fontArgs);

    sk_sp<SkTypeface> defaultFallback(SkUnichar unicode, const std::vector<SkString>& families,
                                      SkFontStyle fontStyle, const SkString& locale,
                                      const std::optional<FontArguments>& fontArgs);
    sk_sp<SkTypeface> defaultEmojiFallback(SkUnichar emojiStart, SkFontStyle fontStyle, const SkString& locale);
    sk_sp<SkTypeface> defaultFallback();

    void disableFontFallback();
    void enableFontFallback();
    bool fontFallbackEnabled() { return fEnableFontFallback; }

    ParagraphCache* getParagraphCache() { return &fParagraphCache; }

    void clearCaches();

private:
    void resetResolutionCaches();

    std::vector<sk_sp<SkFontMgr>> getFontManagerOrder() const;

    sk_sp<SkTypeface> matchTypeface(const SkString& familyName, SkFontStyle fontStyle);

    // CoreTextFontRegistry::findTypefaces is the only caller of findCoreTextTypefaces: system
    // families resolve through CoreText, never through fDefaultFontManager
    friend class CoreTextFontRegistry;

    // Looks up an installed family by exact name in fontStyle; null when it is missing
    using SystemFamilyLookup = sk_sp<SkTypeface> (*)(const SkString& familyName,
                                                     SkFontStyle fontStyle);

    // Resolves familyNames for CoreText paragraphs and caches the list in fFaceCache apart from
    // findTypefaces entries. Each family comes from the user managers, then systemFamily; with
    // no match coreTextStandIn supplies one face. fontArgs clones every face, dropping failures
    std::vector<sk_sp<SkTypeface>> findCoreTextTypefaces(
            const std::vector<SkString>& familyNames,
            SkFontStyle fontStyle,
            const std::optional<FontArguments>& fontArgs,
            SystemFamilyLookup systemFamily);

    // Returns the face for one family: the user managers first (dynamic, asset, test), then
    // systemFamily, unless user managers exist without a default manager or font fallback is
    // disabled. Null when nothing matches; fDefaultFontManager itself is never queried
    sk_sp<SkTypeface> matchCoreTextFamily(const SkString& familyName,
                                          SkFontStyle fontStyle,
                                          SystemFamilyLookup systemFamily);

    // Returns the face that stands in when no requested family matched: the first matching
    // default family, then a user manager's default face, then .AppleSystemUIFont from
    // systemFamily when font fallback is enabled. Null when none of them exists
    sk_sp<SkTypeface> coreTextStandIn(SkFontStyle fontStyle, SystemFamilyLookup systemFamily);

    sk_sp<SkTypeface> cloneTypeface(const sk_sp<SkTypeface>& typeface, const FontArguments& args);

    struct FaceCache;
    std::unique_ptr<FaceCache> fFaceCache;
    struct VariationCache;
    std::unique_ptr<VariationCache> fVariationCache;
    struct FallbackCache;
    std::unique_ptr<FallbackCache> fFallbackCache;
    bool fEnableFontFallback;
    sk_sp<SkFontMgr> fDefaultFontManager;
    sk_sp<SkFontMgr> fAssetFontManager;
    sk_sp<SkFontMgr> fDynamicFontManager;
    sk_sp<SkFontMgr> fTestFontManager;

    std::vector<SkString> fDefaultFamilyNames;
    ParagraphCache fParagraphCache;
};
}  // namespace textlayout
}  // namespace skia

#endif  // FontCollection_DEFINED
