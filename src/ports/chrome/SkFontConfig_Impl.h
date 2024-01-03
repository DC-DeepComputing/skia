#ifndef SkFontConfig_Impl_DEFINED
#define SkFontConfig_Impl_DEFINED

#if defined(SK_BUILD_FOR_WIN) && (defined(SK_FONTMGR_GDI_AVAILABLE) || \
                                  defined(SK_FONTMGR_DIRECTWRITE_AVAILABLE))
#include "src/ports/chrome/SkDWrite_Impl.h"
#endif

#include "src/ports/chrome/SkFontMgr_Interface.h"

namespace ChromeInterfaces {

struct FontConfigMatchResult {};

class FontConfigMgr_Interface : public FontMgrInterface {
    private:
    sk_sp<SkFontMgr> fFontMgr;

    public:
    FontConfigMgr_Interface(sk_sp<SkFontMgr> fontMgr) : fFontMgr(fontMgr) { }
    FontMatchResults matchFamilyStyle(const char familyName[],
                                      const SkFontStyle& fontStyle) override;
    FontMatchResults matchFamilyStyleCharacter(const char familyName[],
                                               const SkFontStyle&,
                                               const char* bcp47[],
                                               int bcp47Count,
                                               SkUnichar character) override;
};

template<>
sk_sp<SkTypeface> FontMgrInterface::makeTypeface<FontConfigMatchResult>
                                     (const FontConfigMatchResult& results);

#if defined(SK_BUILD_FOR_WIN) && (defined(SK_FONTMGR_GDI_AVAILABLE) || \
                                  defined(SK_FONTMGR_DIRECTWRITE_AVAILABLE))
template<>
sk_sp<SkTypeface> FontMgrInterface::makeTypeface<DWriteMatchResult>
                                      (const DWriteMatchResult& results);
#endif

sk_sp<FontMgrInterface> Make_New_FontConfigMgr_Interface(sk_sp<SkFontMgr> fontMgr) {
    return sk_make_sp<FontConfigMgr_Interface>(fontMgr);
}

} // ChromeInterfaces
#endif // SkFontConfig_Impl_DEFINED