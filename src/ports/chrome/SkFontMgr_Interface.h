#ifndef SkFontMgr_Interface_DEFINED
#define SkFontMgr_Interface_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkTypeface.h"

#if defined(SK_BUILD_FOR_WIN) && (defined(SK_FONTMGR_GDI_AVAILABLE) || \
                                  defined(SK_FONTMGR_DIRECTWRITE_AVAILABLE))
#include "include/ports/SkTypeface_win.h"
#endif

#if defined(SK_BUILD_FOR_ANDROID) && defined(SK_FONTMGR_ANDROID_AVAILABLE)
#include "include/ports/SkFontMgr_android.h"
#endif

#if defined(SK_FONTMGR_CORETEXT_AVAILABLE) && (defined(SK_BUILD_FOR_IOS) || \
                                               defined(SK_BUILD_FOR_MAC))
#include "include/ports/SkFontMgr_mac_ct.h"
#endif

#if defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
#include "include/ports/SkFontMgr_fontconfig.h"
#endif

#if defined(SK_FONTMGR_FREETYPE_DIRECTORY_AVAILABLE)
#include "include/ports/SkFontMgr_directory.h"
#endif

#if defined(SK_FONTMGR_FREETYPE_EMPTY_AVAILABLE)
#include "include/ports/SkFontMgr_empty.h"
#endif

namespace ChromeInterfaces {

struct FontMatchResults { };

class FontMgrInterface : public SkRefCnt {
    public:
        virtual FontMatchResults matchFamilyStyle(const char familyName[],
                                                  const SkFontStyle& fontStyle) = 0;
        virtual FontMatchResults matchFamilyStyleCharacter(const char familyName[],
                                                           const SkFontStyle&,
                                                           const char* bcp47[],
                                                           int bcp47Count,
                                                           SkUnichar character) = 0;
        template<typename T>
        sk_sp<SkTypeface> makeTypeface(const T& results);
};

} // ChromeInterfaces
#endif // SkFontMgr_Interface_DEFINED