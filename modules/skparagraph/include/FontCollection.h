// Copyright 2019 Google LLC.
#ifndef FontCollection_DEFINED
#define FontCollection_DEFINED

#include <memory>
#include <set>
#include "include/core/SkFontMgr.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkTHash.h"
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/include/TextStyle.h"

namespace skia {
namespace textlayout {

class TextStyle;
class Paragraph;
class FontCollection : public SkRefCnt {
public:
    FontCollection();

    size_t getFontManagersCount() const;

    void setAssetFontManager(sk_sp<SkFontMgr> fontManager);
    void setDynamicFontManager(sk_sp<SkFontMgr> fontManager);
    void setTestFontManager(sk_sp<SkFontMgr> fontManager);
    void setDefaultFontManager(sk_sp<SkFontMgr> fontManager);
    void setDefaultFontManager(sk_sp<SkFontMgr> fontManager, const char defaultFamilyName[]);
    void setDefaultFontManager(sk_sp<SkFontMgr> fontManager, const std::vector<SkString>& defaultFamilyNames);

    sk_sp<SkFontMgr> getFallbackManager() const { return fDefaultFontManager; }

    std::vector<sk_sp<SkTypeface>> findTypefaces(const std::vector<SkString>& familyNames, SkFontStyle fontStyle);

    sk_sp<SkTypeface> defaultFallback(SkUnichar unicode, SkFontStyle fontStyle, const SkString& locale);
    sk_sp<SkTypeface> defaultFallback();

    void disableFontFallback();
    void enableFontFallback();
    bool fontFallbackEnabled() { return fEnableFontFallback; }

    ParagraphCache* getParagraphCache() { return &fParagraphCache; }

    void clearCaches();

    void addOptimizedLayout() { ++stats.optimized_layout; }
    void addOptimizedPaint() { ++stats.optimized_paint; }
    void addQueries() { ++stats.queries; }
    void addNotOptimized(ParagraphImpl* paragraph);

private:
    std::vector<sk_sp<SkFontMgr>> getFontManagerOrder() const;

    sk_sp<SkTypeface> matchTypeface(const SkString& familyName, SkFontStyle fontStyle);

    struct FamilyKey {
        FamilyKey(const std::vector<SkString>& familyNames, SkFontStyle style)
                : fFamilyNames(familyNames), fFontStyle(style) {}

        FamilyKey() {}

        std::vector<SkString> fFamilyNames;
        SkFontStyle fFontStyle;

        bool operator==(const FamilyKey& other) const;

        struct Hasher {
            size_t operator()(const FamilyKey& key) const;
        };
    };

    bool fEnableFontFallback;
    SkTHashMap<FamilyKey, std::vector<sk_sp<SkTypeface>>, FamilyKey::Hasher> fTypefaces;
    sk_sp<SkFontMgr> fDefaultFontManager;
    sk_sp<SkFontMgr> fAssetFontManager;
    sk_sp<SkFontMgr> fDynamicFontManager;
    sk_sp<SkFontMgr> fTestFontManager;

    std::vector<SkString> fDefaultFamilyNames;
    ParagraphCache fParagraphCache;

    static struct {
        size_t optimized_layout = 0;
        size_t optimized_paint = 0;
        size_t not_optimized = 0;
        size_t queries = 0;

        size_t many_styles = 0;
        size_t letter_spacings = 0;
        size_t word_spacings = 0;
        size_t baseline_shifts = 0;
        size_t ellipsis = 0;
        size_t text_aligns = 0;
        size_t backgrounds = 0;
        size_t decorations = 0;
        size_t shadows = 0;
        size_t many_runs = 0;
        size_t run_sizes = 0;
        size_t many_lines = 0;
        size_t line_breaks = 0;
        size_t place_holders = 0;

    } stats;
};
}  // namespace textlayout
}  // namespace skia

#endif  // FontCollection_DEFINED
