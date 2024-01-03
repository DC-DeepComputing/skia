#ifndef SkDWrite_Impl_DEFINED
#define SkDWrite_Impl_DEFINED

#if defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
#include "src/ports/chrome/SkFontConfig_Impl.h"
#endif

#include "src/ports/chrome/SkFontMgr_Interface.h"

namespace ChromeInterfaces {

struct DWriteMatchResult {};

class DWriteMgr_Interface : public FontMgrInterface {
    private:
    sk_sp<SkFontMgr> fFontMgr;

    public:
    DWriteMgr_Interface(sk_sp<SkFontMgr> fontMgr) : fFontMgr(fontMgr) { }
    FontMatchResults matchFamilyStyle(const char familyName[],
                                      const SkFontStyle& fontStyle) override;
    FontMatchResults matchFamilyStyleCharacter(const char familyName[],
                                               const SkFontStyle&,
                                               const char* bcp47[],
                                               int bcp47Count,
                                               SkUnichar character) override;
};

template<>
sk_sp<SkTypeface> FontMgrInterface::makeTypeface<DWriteMatchResult>
                                     (const DWriteMatchResult& results);

#if defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
template<>
sk_sp<SkTypeface> FontMgrInterface::makeTypeface<FontConfigMatchResult>
                                      (const FontConfigMatchResult& results);
#endif

sk_sp<FontMgrInterface> Make_New_DWriteMgr_Interface(sk_sp<SkFontMgr> fontMgr) {
    return sk_make_sp<DWriteMgr_Interface>(fontMgr);
}

} // ChromeInterfaces
#endif // SkDWrite_Impl_DEFINED