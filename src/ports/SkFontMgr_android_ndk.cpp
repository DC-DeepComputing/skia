/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontScanner.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/ports/SkFontMgr_android_ndk.h"
#include "include/private/SkAssert.h"
#include "include/private/SkFeatures.h"
#include "include/private/SkFloatingPoint.h"
#include "include/private/SkOnlyStackAllocatable.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkSharedMutex.h"
#include "src/core/SkTHash.h"
#include "src/core/SkTSort.h"
#include "src/core/SkUTF.h"
#include "src/partition_alloc/raw_ptr.h"
#include "src/ports/SkTypeface_proxy.h"

#include <unicode/uchar.h>
#include <unicode/ustring.h>

#include <algorithm>
#include <cmath>
#include <cinttypes>
#include <memory>
#include <vector>
#include <dlfcn.h>

#if defined(SK_BUILD_FOR_ANDROID)
#include <android/api-level.h>
#else
#define __ANDROID_API__ 0
#define __ANDROID_API_S__ 31
int android_get_device_api_level() { return __ANDROID_API__; };
#endif

using namespace skia_private;

struct AAliasDescriptor;
struct AFont;
struct AFontDescriptor;
struct AFontFamilyDescriptor;
struct AFontManager;
struct AFontMatchResult;
struct AFontMatcher;

namespace {

[[maybe_unused]] static inline const constexpr bool kSkFontMgrVerbose = false;

namespace variation {
using Coordinate = SkFontArguments::VariationPosition::Coordinate;
using Storage = AutoSTArray<4, Coordinate>;

static constexpr SkFourByteTag wghtTag = SkSetFourByteTag('w','g','h','t');
static constexpr SkFourByteTag wdthTag = SkSetFourByteTag('w','d','t','h');
static constexpr SkFourByteTag slntTag = SkSetFourByteTag('s','l','n','t');
static constexpr SkFourByteTag italTag = SkSetFourByteTag('i','t','a','l');

static bool coordinateLess(const Coordinate& a, const Coordinate& b) {
    return a.axis != b.axis ? a.axis < b.axis : a.value < b.value;
}

static bool coordinateEqual(const Coordinate& a, const Coordinate& b) {
    return a.axis == b.axis && a.value == b.value;
}

static SkSpan<Coordinate> Get(const SkTypeface& typeface, Storage& storage) {
    if (storage.size() < Storage::kCount) {
        storage.reset(Storage::kCount);
    }
    int numAxes = typeface.getVariationDesignPosition(SkSpan(storage));
    if (SkToInt(storage.size()) < numAxes) {
        storage.reset(numAxes);
        numAxes = typeface.getVariationDesignPosition(SkSpan(storage));
    }
    if (numAxes < 0) {
        numAxes = 0;
    }

    return SkSpan(storage.data(), numAxes);
}

/* Normalize the values. NaN and -0.0 => 0.
 * Should normalize before sorting or comparing.
 */
static SkSpan<Coordinate> Normalize(SkSpan<Coordinate> variation) {
    for (auto&& coord : variation) {
        if (coord.value == 0 || SkIsNaN(coord.value)) {
            coord.value = 0.0f;
        }
    }
    return variation;
}

static SkSpan<Coordinate> Sort(SkSpan<Coordinate> variation) {
    SkTQSort(variation.begin(), variation.end(), variation::coordinateLess);
    return variation;
}

}  // namespace variation

struct AndroidFontAPI {
    // AFont APIs (from AFontMatcher_match results)
    void (*AFont_close)(AFont*) = nullptr;
    const char* (*AFont_getFontFilePath)(const AFont*) = nullptr;
    uint16_t (*AFont_getWeight)(const AFont*) = nullptr;
    bool (*AFont_isItalic)(const AFont*) = nullptr;
    const char* (*AFont_getLocale)(const AFont*) = nullptr;
    size_t (*AFont_getCollectionIndex)(const AFont*) = nullptr;
    size_t (*AFont_getAxisCount)(const AFont*) = nullptr;
    uint32_t (*AFont_getAxisTag)(const AFont*, uint32_t axisIndex) = nullptr;
    float (*AFont_getAxisValue)(const AFont*, uint32_t axisIndex) = nullptr;

    // AFontMatcher APIs (for character-based fallback matching)
    AFontMatcher* (*AFontMatcher_create)() = nullptr;
    void (*AFontMatcher_destroy)(AFontMatcher*) = nullptr;
    void (*AFontMatcher_setStyle)(AFontMatcher*, uint16_t weight, bool italic) = nullptr;
    void (*AFontMatcher_setLocales)(AFontMatcher*, const char* languageTags) = nullptr;
    AFont* (*AFontMatcher_match)(const AFontMatcher*,
                                 const char* familyName,
                                 const uint16_t* text,
                                 const uint32_t textLength,
                                 uint32_t* runLengthOut) = nullptr;

    // AFontManager & Descriptor APIs (Android 38+ Font Catalog)
    AFontManager* (*AFontManager_create)() = nullptr;
    void (*AFontManager_destroy)(AFontManager*) = nullptr;
    size_t (*AFontManager_getNamedFontFamilySize)(const AFontManager*) = nullptr;
    const AFontFamilyDescriptor* (*AFontManager_getNamedFontFamily)(
            const AFontManager*, size_t index) = nullptr;
    size_t (*AFontManager_getAliasSize)(const AFontManager*) = nullptr;
    const AAliasDescriptor* (*AFontManager_getAlias)(const AFontManager*, size_t index) = nullptr;
    AFontMatchResult* (*AFontManager_findByFamilyName)(
            const AFontManager*, const char* familyName) = nullptr;
    void (*AFontMatchResult_destroy)(AFontMatchResult*) = nullptr;
    size_t (*AFontMatchResult_getSize)(const AFontMatchResult*) = nullptr;
    const char* (*AFontMatchResult_getFilePath)(const AFontMatchResult*, size_t index) = nullptr;
    uint32_t (*AFontMatchResult_getCollectionIndex)(
            const AFontMatchResult*, size_t index) = nullptr;
    uint32_t (*AFontMatchResult_getMatchFlags)(const AFontMatchResult*, size_t index) = nullptr;

    const char* (*AFontFamilyDescriptor_getName)(const AFontFamilyDescriptor*) = nullptr;
    const char* (*AFontFamilyDescriptor_getLocaleList)(const AFontFamilyDescriptor*) = nullptr;
    size_t (*AFontFamilyDescriptor_getFontSize)(const AFontFamilyDescriptor*) = nullptr;
    const AFontDescriptor* (*AFontFamilyDescriptor_getFont)(
            const AFontFamilyDescriptor*, size_t index) = nullptr;

    const char* (*AFontDescriptor_getFilePath)(const AFontDescriptor*) = nullptr;
    uint16_t (*AFontDescriptor_getWeight)(const AFontDescriptor*) = nullptr;
    uint32_t (*AFontDescriptor_getStyle)(const AFontDescriptor*) = nullptr;
    size_t (*AFontDescriptor_getVariationSettingSize)(const AFontDescriptor*) = nullptr;
    uint32_t (*AFontDescriptor_getVariationSettingTag)(
            const AFontDescriptor*, size_t index) = nullptr;
    float (*AFontDescriptor_getVariationSettingValue)(
            const AFontDescriptor*, size_t index) = nullptr;

    const char* (*AAliasDescriptor_getName)(const AAliasDescriptor*) = nullptr;
    const char* (*AAliasDescriptor_getOriginal)(const AAliasDescriptor*) = nullptr;
    uint16_t (*AAliasDescriptor_getWeight)(const AAliasDescriptor*) = nullptr;

private:
    std::unique_ptr<void, SkFunctionObject<dlclose>> fSelf;

public:
    AndroidFontAPI() = default;
    AndroidFontAPI(const AndroidFontAPI&) = delete;
    AndroidFontAPI& operator=(const AndroidFontAPI&) = delete;
    AndroidFontAPI(AndroidFontAPI&&) = default;
    AndroidFontAPI& operator=(AndroidFontAPI&&) = default;

    static std::optional<AndroidFontAPI> Make() {
        AndroidFontAPI api;
        api.fSelf.reset(dlopen("libandroid.so", RTLD_LAZY | RTLD_LOCAL));
        if (!api.fSelf) {
            return std::nullopt;
        }

#define SK_DLSYM_ANDROID_FONT_API(NAME)                           \
        do {                                                      \
            *(void**)(&api.NAME) = dlsym(api.fSelf.get(), #NAME); \
            if (!api.NAME) {                                      \
                if constexpr (kSkFontMgrVerbose) {                \
                    SkDebugf("SKIA: Failed to load: " #NAME "\n");\
                }                                                 \
                return std::nullopt;                              \
            }                                                     \
        } while (0)

        SK_DLSYM_ANDROID_FONT_API(AFont_close);
        SK_DLSYM_ANDROID_FONT_API(AFont_getFontFilePath);
        SK_DLSYM_ANDROID_FONT_API(AFont_getWeight);
        SK_DLSYM_ANDROID_FONT_API(AFont_isItalic);
        SK_DLSYM_ANDROID_FONT_API(AFont_getLocale);
        SK_DLSYM_ANDROID_FONT_API(AFont_getCollectionIndex);
        SK_DLSYM_ANDROID_FONT_API(AFont_getAxisCount);
        SK_DLSYM_ANDROID_FONT_API(AFont_getAxisTag);
        SK_DLSYM_ANDROID_FONT_API(AFont_getAxisValue);

        SK_DLSYM_ANDROID_FONT_API(AFontMatcher_create);
        SK_DLSYM_ANDROID_FONT_API(AFontMatcher_destroy);
        SK_DLSYM_ANDROID_FONT_API(AFontMatcher_setStyle);
        SK_DLSYM_ANDROID_FONT_API(AFontMatcher_setLocales);
        SK_DLSYM_ANDROID_FONT_API(AFontMatcher_match);

        SK_DLSYM_ANDROID_FONT_API(AFontManager_create);
        SK_DLSYM_ANDROID_FONT_API(AFontManager_destroy);
        SK_DLSYM_ANDROID_FONT_API(AFontManager_getNamedFontFamilySize);
        SK_DLSYM_ANDROID_FONT_API(AFontManager_getNamedFontFamily);
        SK_DLSYM_ANDROID_FONT_API(AFontManager_getAliasSize);
        SK_DLSYM_ANDROID_FONT_API(AFontManager_getAlias);
        SK_DLSYM_ANDROID_FONT_API(AFontManager_findByFamilyName);
        SK_DLSYM_ANDROID_FONT_API(AFontMatchResult_destroy);
        SK_DLSYM_ANDROID_FONT_API(AFontMatchResult_getSize);
        SK_DLSYM_ANDROID_FONT_API(AFontMatchResult_getFilePath);
        SK_DLSYM_ANDROID_FONT_API(AFontMatchResult_getCollectionIndex);
        SK_DLSYM_ANDROID_FONT_API(AFontMatchResult_getMatchFlags);

        SK_DLSYM_ANDROID_FONT_API(AFontFamilyDescriptor_getName);
        SK_DLSYM_ANDROID_FONT_API(AFontFamilyDescriptor_getLocaleList);
        SK_DLSYM_ANDROID_FONT_API(AFontFamilyDescriptor_getFontSize);
        SK_DLSYM_ANDROID_FONT_API(AFontFamilyDescriptor_getFont);

        SK_DLSYM_ANDROID_FONT_API(AFontDescriptor_getFilePath);
        SK_DLSYM_ANDROID_FONT_API(AFontDescriptor_getWeight);
        SK_DLSYM_ANDROID_FONT_API(AFontDescriptor_getStyle);
        SK_DLSYM_ANDROID_FONT_API(AFontDescriptor_getVariationSettingSize);
        SK_DLSYM_ANDROID_FONT_API(AFontDescriptor_getVariationSettingTag);
        SK_DLSYM_ANDROID_FONT_API(AFontDescriptor_getVariationSettingValue);

        SK_DLSYM_ANDROID_FONT_API(AAliasDescriptor_getName);
        SK_DLSYM_ANDROID_FONT_API(AAliasDescriptor_getOriginal);
        SK_DLSYM_ANDROID_FONT_API(AAliasDescriptor_getWeight);
#undef SK_DLSYM_ANDROID_FONT_API

        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: GetAndroidFontAPI loaded\n"); }
        return api;
    }
};

struct AndroidIcuAPI {
    /* The result is case folded UTF-8. This is normalized case, it isn't upper or lower case. */
    SkString casefold(SkSpan<const char> s) const {
        return fHasICU ? this->icuCaseFold(s) : this->cToWLower(s);
    }

private:
    int32_t (*u_strFoldCase)(UChar *dest, int32_t destCapacity, const UChar *src, int32_t srcLength,
                             uint32_t options, UErrorCode *pErrorCode) = nullptr;
    UChar* (*u_strFromUTF8)(UChar *dest, int32_t destCapacity, int32_t *pDestLength,
                            const char *src, int32_t srcLength, UErrorCode *pErrorCode) = nullptr;
    char * (*u_strToUTF8)(char *dest, int32_t destCapacity, int32_t *pDestLength,
                          const UChar *src, int32_t srcLength, UErrorCode *pErrorCode) = nullptr;
    bool fHasICU = false;

    SkString cToWLower(SkSpan<const char> s) const {
        // Find length of result
        size_t retLen = 0;
        bool isDifferent = false;
        const char* src = s.begin();
        while (src != s.end()) {
            SkUnichar uni = SkUTF::NextUTF8(&src, s.end());
            if (uni < 0) {
                return SkString(s.data(), s.size());
            }
            // On Android 2.3 (API 9) and later wchar_t is 32 bit, UTF-32 like.
            // towlower only provides simple case folding, but this should be fine for Android 11.
            wint_t wlow = towlower(uni);
            char buffer[SkUTF::kMaxBytesInUTF8Sequence];
            size_t buflen = SkUTF::ToUTF8(wlow, buffer);
            if (buflen == 0) {
                return SkString(s.data(), s.size());
            }
            isDifferent |= wlow != SkToU32(uni);
            retLen += buflen;
        }

        // No change needed
        if (!isDifferent) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: TL \"%s\" unchanged\n", s.data()); }
            return SkString(s.data(), s.size());
        }

        // Create result
        SkString ret(retLen);
        src = s.begin();
        char* dst = ret.begin();
        while (src != s.end()) {
            SkUnichar uni = SkUTF::NextUTF8(&src, s.end());
            wint_t wlow = towlower(uni);
            char buffer[SkUTF::kMaxBytesInUTF8Sequence];
            size_t buflen = SkUTF::ToUTF8(wlow, buffer);
            memcpy(dst, buffer, buflen);
            dst += buflen;
        }

        if constexpr (kSkFontMgrVerbose) {
            SkDebugf("SKIA: TL \"%s\" to \"%s\"\n", s.data(), ret.c_str());
        }
        return ret;
    }

    SkString icuCaseFold(SkSpan<const char> s) const {
        if (!SkTFitsIn<int32_t>(s.size())) {
            return SkString(s.data(), s.size());
        }

        using Storage = AutoSTMalloc<32, UChar>;
        UErrorCode error = U_ZERO_ERROR;

        // Convert to UTF-16
        int32_t uStrSize = 0;
        Storage uStr(Storage::kCount);
        this->u_strFromUTF8(uStr, Storage::kCount, &uStrSize, s.data(), s.size(), &error);
        if (error == U_BUFFER_OVERFLOW_ERROR) {
            error = U_ZERO_ERROR;
        }
        if (U_FAILURE(error)) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: CF UTF-16 length\n"); }
            return SkString(s.data(), s.size());
        }
        if (SkTo<int32_t>(Storage::kCount) < uStrSize) {
            uStr.reset(uStrSize);
            int32_t uStrSizePrev = uStrSize;
            this->u_strFromUTF8(uStr, uStrSize, &uStrSize, s.data(), s.size(), &error);
            if (U_FAILURE(error) || uStrSizePrev < uStrSize) {
                if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: CF UTF-16 wrong length\n"); }
                return SkString(s.data(), s.size());
            }
        }

        // Case fold
        Storage uStrFolded(Storage::kCount);
        int32_t uStrFoldedSize = this->u_strFoldCase(uStrFolded, Storage::kCount, uStr, uStrSize,
                                                     U_FOLD_CASE_EXCLUDE_SPECIAL_I, &error);
        if (error == U_BUFFER_OVERFLOW_ERROR) {
            error = U_ZERO_ERROR;
        }
        if (U_FAILURE(error)) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: CF folded length\n"); }
            return SkString(s.data(), s.size());
        }
        if (SkTo<int32_t>(Storage::kCount) < uStrFoldedSize) {
            uStrFolded.reset(uStrFoldedSize);
            int32_t uStrFoldedSizePrev = uStrFoldedSize;
            uStrFoldedSize = this->u_strFoldCase(uStrFolded, uStrFoldedSize, uStr, uStrSize,
                                                 U_FOLD_CASE_EXCLUDE_SPECIAL_I, &error);
            if (U_FAILURE(error) || uStrFoldedSizePrev < uStrFoldedSize) {
                if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: CF folded wrong length\n"); }
                return SkString(s.data(), s.size());
            }
        }

        // Convert to UTF-8
        SkString ret;
        int32_t retSize = 0;
        this->u_strToUTF8(nullptr, 0, &retSize, uStrFolded, uStrFoldedSize, &error);
        if (error == U_BUFFER_OVERFLOW_ERROR) {
            error = U_ZERO_ERROR;
        }
        if (U_FAILURE(error)) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: CF UTF-8 length\n"); }
            return SkString(s.data(), s.size());
        }
        ret.resize(retSize);
        int32_t retSizePrev = retSize;
        this->u_strToUTF8(ret.data(), retSize, &retSize, uStrFolded, uStrFoldedSize, &error);
        if (U_FAILURE(error) || retSizePrev != retSize) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: CF UTF-8 wrong length\n"); }
            return SkString(s.data(), s.size());
        }

        // Return result
        if constexpr (kSkFontMgrVerbose) {
            SkDebugf("SKIA: CF \"%s\" to \"%s\"\n", s.data(), ret.c_str());
        }
        return ret;
    }

#if __ANDROID_API__ >= __ANDROID_API_S__
public:
    AndroidIcuAPI()
        : u_strFoldCase(::u_strFoldCase)
        , u_strFromUTF8(::u_strFromUTF8)
        , u_strToUTF8(::u_strToUTF8)
        , fHasICU(true)
    {
        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: GetAndroidIcuAPI direct\n"); }
    }

#else

private:
    std::unique_ptr<void, SkFunctionObject<dlclose>> fSelf;
public:
    AndroidIcuAPI(const AndroidIcuAPI&) = delete;
    AndroidIcuAPI& operator=(const AndroidIcuAPI&) = delete;
    AndroidIcuAPI(AndroidIcuAPI&&) = default;
    AndroidIcuAPI& operator=(AndroidIcuAPI&&) = default;

    AndroidIcuAPI() {
        fSelf.reset(dlopen("libicu.so", RTLD_LAZY | RTLD_LOCAL));
        if (!fSelf) {
            return;
        }

#define SK_DLSYM_ANDROID_ICU_API(NAME)                            \
        do {                                                      \
            *(void**)(&NAME) = dlsym(fSelf.get(), #NAME);         \
            if (!NAME) {                                          \
                if constexpr (kSkFontMgrVerbose) {                \
                    SkDebugf("SKIA: Failed to load: " #NAME "\n");\
                }                                                 \
                return;                                           \
            }                                                     \
        } while (0)

        SK_DLSYM_ANDROID_ICU_API(u_strFoldCase);
        SK_DLSYM_ANDROID_ICU_API(u_strFromUTF8);
        SK_DLSYM_ANDROID_ICU_API(u_strToUTF8);
#undef SK_DLSYM_ANDROID_ICU_API

        fHasICU = true;
        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: GetAndroidIcuAPI dlsym\n"); }
    }
#endif
};

// bcp47 is ascii only and only does 1:1 replacements for case folding.
static void normalizeAsciiCase(SkSpan<char> s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](char c){ return (c < 'A' || 'Z' < c) ? c : c + 'a' - 'A'; });
}

class SkALanguage {
public:
    SkALanguage() {}
    SkALanguage(const char* tag) : SkALanguage(SkSpan<const char>(tag, tag ? strlen(tag) : 0)) {}
    SkALanguage(SkSpan<const char> tag) {
        fLanguage = consumeSubtag(tag);
        if (fLanguage.equals("und", 3)) {
            fLanguage = SkString();
        }
        fScript = consumeSubtag(tag);
        fRegion = consumeSubtag(tag);
    }
    SkALanguage(const SkALanguage&) = default;
    SkALanguage& operator=(const SkALanguage& b) = default;

    SkALanguage lessSpecific() const {
        SkALanguage lessSpecific(*this);
        if (!lessSpecific.fRegion.isEmpty()) {
            lessSpecific.fRegion = SkString();
            return lessSpecific;
        }
        if (!lessSpecific.fScript.isEmpty()) {
            lessSpecific.fScript = SkString();
            return lessSpecific;
          }

        if (!lessSpecific.fLanguage.isEmpty()) {
            lessSpecific.fLanguage = SkString();
            return lessSpecific;
        }
        return lessSpecific;
    }

    bool isEmpty() const {
        return fLanguage.isEmpty() && fScript.isEmpty() && fRegion.isEmpty();
    }

    bool startsWith(const SkALanguage& that) {
        return (that.fLanguage.isEmpty() || fLanguage == that.fLanguage) &&
               (that.fScript.isEmpty() || fScript == that.fScript) &&
               (that.fRegion.isEmpty() || fRegion == that.fRegion);
    }

    const SkString& getLanguage() const { return fLanguage; }
    const SkString& getScript() const { return fScript; }
    const SkString& getRegion() const { return fRegion; }

    using sk_is_trivially_relocatable = std::true_type;
private:
    SkString consumeSubtag(SkSpan<const char>& tag) {
        if (tag.size() == 0) {
            return SkString();
        }
        const char* subtagEnd = static_cast<const char*>(memchr(tag.data(), '-', tag.size()));
        if (!subtagEnd) {
            SkString ret(tag.data(), tag.size());
            tag = SkSpan<const char>();
            return ret;
        }
        size_t subtagLength = subtagEnd - tag.data();
        SkString ret(tag.data(), subtagLength);
        normalizeAsciiCase({ret.begin(), ret.size()});
        tag = tag.subspan(subtagLength + 1);
        return ret;
    }
    SkString fLanguage;
    SkString fScript;
    SkString fRegion;
    // variants, extensions, and privateuses are ignored as no known font configuration uses them.
    static_assert(::sk_is_trivially_relocatable<decltype(fLanguage)>::value);
    static_assert(::sk_is_trivially_relocatable<decltype(fScript)>::value);
    static_assert(::sk_is_trivially_relocatable<decltype(fRegion)>::value);
};

class SkTypeface_AndroidNDK : public SkTypeface_proxy {
public:
    struct AutoAxis {
        static constexpr struct KnownAxis {
            SkFourByteTag tag;
            int flag;
        } kKnownAxis[] = {
            { variation::wghtTag, 1 << 0 },
            { variation::wdthTag, 1 << 1 },
            { variation::slntTag, 1 << 2 },
            { variation::italTag, 1 << 3 },
        };

        AutoAxis() = default;
        AutoAxis(const AutoAxis&) = default;
        AutoAxis& operator=(const AutoAxis&) = default;
        AutoAxis(SkSpan<const SkFontArguments::VariationPosition::Coordinate> pos) {
            for (auto&& coord : pos) {
                for (auto&& axis : kKnownAxis) {
                    if (coord.axis == axis.tag) {
                        fFlags |= axis.flag;
                    }
                }
            }
        }

        void remove(const AutoAxis& that) {
            fFlags &= ~that.fFlags;
        }

        bool weight() const { return SkToBool(fFlags & kKnownAxis[0].flag); }
        bool width() const { return SkToBool(fFlags & kKnownAxis[1].flag); }
        bool slant() const { return SkToBool(fFlags & kKnownAxis[2].flag); }
        bool italic() const { return SkToBool(fFlags & kKnownAxis[3].flag); }
        bool none() const { return fFlags == 0; }
        uint8_t fFlags = 0;
    };
    static sk_sp<SkTypeface_AndroidNDK> Make(sk_sp<SkTypeface> realTypeface,
                                             const SkFontStyle& style,
                                             bool isFixedPitch,
                                             const SkString& familyName,
                                             TArray<SkString>&& extraFamilyNames,
                                             TArray<SkALanguage>&& lang,
                                             const AutoAxis& autoAxis) {
        SkASSERT(realTypeface);
        return sk_sp<SkTypeface_AndroidNDK>(new SkTypeface_AndroidNDK(std::move(realTypeface),
                                                                      style,
                                                                      isFixedPitch,
                                                                      familyName,
                                                                      std::move(extraFamilyNames),
                                                                      std::move(lang),
                                                                      autoAxis));
    }

private:
    SkTypeface_AndroidNDK(sk_sp<SkTypeface> realTypeface,
                          const SkFontStyle& style,
                          bool isFixedPitch,
                          const SkString& familyName,
                          TArray<SkString>&& extraFamilyNames,
                          TArray<SkALanguage>&& lang,
                          const AutoAxis& autoAxis)
        : SkTypeface_proxy(std::move(realTypeface), style, isFixedPitch)
        , fFamilyName(familyName)
        , fExtraFamilyNames(std::move(extraFamilyNames))
        , fLang(std::move(lang))
        , fAutoAxis(autoAxis)
    { }

    void onGetFamilyName(SkString* familyName) const override {
        *familyName = fFamilyName;
    }

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const override {
        SkASSERT(desc);
        SkASSERT(serialize);
        SkTypeface_proxy::onGetFontDescriptor(desc, serialize);
        desc->setFamilyName(fFamilyName.c_str());
        desc->setStyle(this->fontStyle());
        *serialize = false;
    }

    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        auto proxy = SkTypeface_proxy::onMakeClone(args);
        if (proxy == nullptr) {
            return nullptr;
        }
        SkFontStyle style = proxy->fontStyle();
        bool fixedPitch = proxy->isFixedPitch();
        return SkTypeface_AndroidNDK::Make(
                std::move(proxy),
                style,
                fixedPitch,
                fFamilyName,
                TArray<SkString>(fExtraFamilyNames),
                TArray<SkALanguage>(),
                AutoAxis());
    }

    SkFontStyle onGetFontStyle() const override {
        return SkTypeface::onGetFontStyle();
    }

    bool onGetFixedPitch() const override {
        return SkTypeface::onGetFixedPitch();
    }

    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override {
        class ALocalizedStrings : public SkTypeface::LocalizedStrings {
        public:
            ALocalizedStrings(sk_sp<SkTypeface_AndroidNDK> typeface,
                              sk_sp<SkTypeface::LocalizedStrings> base)
                : fTypeface(std::move(typeface))
                , fBase(std::move(base))
                , fExtraFamilyName(nullptr) {}
        private:
            sk_sp<SkTypeface_AndroidNDK> fTypeface;
            sk_sp<SkTypeface::LocalizedStrings> fBase;
            raw_ptr<const SkString, AllowPtrArithmetic> fExtraFamilyName;

            bool next(LocalizedString* localizedString) override {
                if (!fExtraFamilyName) {
                    if (fBase->next(localizedString)) {
                        return true;
                    }
                    fExtraFamilyName = fTypeface->fExtraFamilyNames.begin();
                }
                if (fExtraFamilyName == fTypeface->fExtraFamilyNames.end()) {
                    return false;
                }
                *localizedString = {*fExtraFamilyName, SkString()};
                ++fExtraFamilyName;
                return true;
            }
        };
        sk_sp<SkTypeface::LocalizedStrings> base(SkTypeface_proxy::onCreateFamilyNameIterator());
        return new ALocalizedStrings(sk_ref_sp(this), std::move(base));
    }

public:
    const SkString fFamilyName;
    const TArray<SkString> fExtraFamilyNames;
    const STArray<4, SkALanguage> fLang;
    const AutoAxis fAutoAxis;
};

class TypefaceCache : public SkRefCnt {
public:
    TypefaceCache() : fRequests(64), fMatches(64) {}
    ~TypefaceCache() override {}

    class Request {
    public:
        Request(SkTypefaceID id, SkFontStyle style) : fId(id), fStyle(style) {
            SkGoodHash hasher;
            fHash = hasher(fId);
            fHash ^= hasher(fStyle);
        }
        bool operator==(const Request& that) const {
            return fId == that.fId && fStyle == that.fStyle;
        }
        struct Hash { uint32_t operator()(const Request& a) { return a.fHash; } };
    private:
        const SkTypefaceID fId;
        const SkFontStyle fStyle;
        uint32_t fHash;
    };
    sk_sp<SkTypeface> find(const Request& request) {
        SkAutoSharedMutexShared lock(fMutex);
        sk_sp<SkTypeface>* typeface = fRequests.find(request);
        if (typeface) {
            return *typeface;
        }
        return nullptr;
    }
    void add(const Request& request, sk_sp<SkTypeface> typeface) {
        SkAutoSharedMutexExclusive lock(fMutex);
        fRequests.insert_or_update(request, std::move(typeface));
    }

    class Match {
    public:
        /* variation is expected to be normalized and sorted. */
        Match(SkTypefaceID id, variation::Storage&& variation)
            : fId(id)
            , fVariation(std::move(variation))
        {
            SkGoodHash hasher;
            fHash = hasher(id);
            for (auto&& coord : fVariation) {
                fHash ^= hasher(coord.axis);
                fHash ^= hasher(FloatBits(coord.value));
            }
        }
        Match(const Match&) = delete;
        Match& operator=(const Match&) = delete;
        Match(Match&& that) : fId(std::move(that.fId))
                            , fVariation(std::move(that.fVariation))
                            , fHash(std::move(that.fHash)) {}
        Match& operator=(Match&&) = delete;
        bool operator==(const Match& that) const {
            return fId == that.fId &&
                   fVariation.size() == that.fVariation.size() &&
                   std::equal(fVariation.begin(), fVariation.end(),
                              that.fVariation.begin(), variation::coordinateEqual);
        }
        struct Hash { uint32_t operator()(const Match& a) { return a.fHash; } };
    private:
        static uint32_t FloatBits(float f) {
            static_assert(sizeof(uint32_t) == sizeof(float));
            uint32_t bits;
            std::memcpy(&bits, &f, sizeof(uint32_t));
            return bits;
        }
        const SkTypefaceID fId;
        variation::Storage fVariation;
        uint32_t fHash;
    };
    sk_sp<SkTypeface> find(const Match& match) {
        SkAutoSharedMutexShared lock(fMutex);
        sk_sp<SkTypeface>* typeface = fMatches.find(match);
        if (typeface) {
            return *typeface;
        }
        return nullptr;
    }
    void add(Match&& match, sk_sp<SkTypeface> typeface) {
        SkAutoSharedMutexExclusive lock(fMutex);
        fMatches.insert_or_update(std::move(match), typeface);
    }
private:
    SkLRUCache<Request, sk_sp<SkTypeface>, Request::Hash> fRequests;
    SkLRUCache<Match, sk_sp<SkTypeface>, Match::Hash> fMatches;
    SkSharedMutex fMutex;
};

sk_sp<SkTypeface> adjustForStyle(sk_sp<SkTypeface_AndroidNDK>&& typeface, SkFontStyle style,
                                 TypefaceCache& cache) {
    if (!typeface) {
        return std::move(typeface);
    }

    SkFontStyle typefaceStyle = typeface->fontStyle();
    if (typefaceStyle == style || typeface->fAutoAxis.none()) {
        return std::move(typeface);
    }

    SkFontArguments::VariationPosition::Coordinate coord[4];
    int numCoords = 0;
    if (typefaceStyle.weight() != style.weight() && typeface->fAutoAxis.weight()) {
        coord[numCoords++] = {variation::wghtTag, static_cast<float>(style.weight())};
    }
    if (typefaceStyle.width() != style.width() && typeface->fAutoAxis.width()) {
        coord[numCoords++] = {variation::wdthTag,
                              SkFontDescriptor::SkFontWidthAxisValueForStyleWidth(style.width())};
    }
    if (typefaceStyle.slant() != style.slant()) {
        switch (style.slant()) {
        case SkFontStyle::Slant::kItalic_Slant:
            if (typeface->fAutoAxis.italic()) {
                coord[numCoords++] = {variation::italTag, 1.0};
            }
            break;
        case SkFontStyle::Slant::kOblique_Slant:
            if (typeface->fAutoAxis.slant()) {
                coord[numCoords++] = {variation::slntTag, -7.0};
            }
            break;
        case SkFontStyle::Slant::kUpright_Slant:
            if (typeface->fAutoAxis.italic()) {
                coord[numCoords++] = {variation::italTag, 0.0};
            }
            if (typeface->fAutoAxis.slant()) {
                coord[numCoords++] = {variation::slntTag, 0.0};
            }
            break;
        }
    }
    if (numCoords == 0) {
        return std::move(typeface);
    }

    TypefaceCache::Request request(typeface->uniqueID(), style);
    if (sk_sp<SkTypeface> cachedTypeface = cache.find(request)) {
        if constexpr (kSkFontMgrVerbose) {
            SkString familyName;
            typeface->getFamilyName(&familyName);
            SkFontStyle s = cachedTypeface->fontStyle();
            SkDebugf("Cached request of \"%s\" weight:%d width: %d slant %d\n",
                     familyName.c_str(), s.weight(), s.width(), s.slant());
        }
        return cachedTypeface;
    }

    sk_sp<SkTypeface> newTypeface = typeface->makeClone(
        SkFontArguments().setVariationDesignPosition({coord, numCoords}));
    if (!newTypeface) {
        if constexpr (kSkFontMgrVerbose) {
            SkString familyName;
            typeface->getFamilyName(&familyName);
            SkDebugf("Failed to clone \"%s\"\n", familyName.c_str());
        }
        return std::move(typeface);
    }

    variation::Storage variationStorage;
    SkSpan<variation::Coordinate> newVariation = variation::Get(*newTypeface, variationStorage);
    variation::Sort(variation::Normalize(newVariation));
    variationStorage.trimTo(newVariation.size());
    TypefaceCache::Match match(typeface->uniqueID(), std::move(variationStorage));
    if (sk_sp<SkTypeface> cachedTypeface = cache.find(match)) {
        if constexpr (kSkFontMgrVerbose) {
            SkString familyName;
            typeface->getFamilyName(&familyName);
            SkFontStyle s = cachedTypeface->fontStyle();
            SkDebugf("Cached match of \"%s\" weight:%d width: %d slant %d\n",
                     familyName.c_str(), s.weight(), s.width(), s.slant());
        }
        cache.add(std::move(request), cachedTypeface);
        return cachedTypeface;
    }

    if constexpr (kSkFontMgrVerbose) {
        SkString familyName;
        typeface->getFamilyName(&familyName);
        SkFontStyle s = newTypeface->fontStyle();
        SkDebugf("New variant of \"%s\" weight:%d width: %d slant %d\n",
                 familyName.c_str(), s.weight(), s.width(), s.slant());
    }
    cache.add(std::move(match), newTypeface);
    cache.add(std::move(request), newTypeface);
    return newTypeface;
}

class SkFontMgr_AndroidNDK;

struct FontEntry {
    SkString fFilePath;
    size_t fCollectionIndex = 0;
    int fWeight = 400;
    bool fIsItalic = false;
    SkString fLocale;
    TArray<SkFontArguments::VariationPosition::Coordinate> fAxes;
};

class SkFontStyleSet_AndroidNDK : public SkFontStyleSet {
public:
    SkFontStyleSet_AndroidNDK(sk_sp<const SkFontMgr_AndroidNDK> fontMgr,
                              SkString familyName,
                              TArray<FontEntry>&& entries,
                              sk_sp<TypefaceCache> cache)
        : fFontMgr(std::move(fontMgr))
        , fFamilyName(std::move(familyName))
        , fEntries(std::move(entries))
        , fCache(std::move(cache)) {}

    int count() override {
        return fEntries.size();
    }

    void getStyle(int index, SkFontStyle* style, SkString* name) override {
        if (index < 0 || fEntries.size() <= index) {
            return;
        }
        if (style) {
            const FontEntry& entry = fEntries[index];
            int width = SkFontStyle::kNormal_Width;
            for (const auto& coord : entry.fAxes) {
                if (coord.axis == variation::wdthTag) {
                    width = SkFontDescriptor::SkFontStyleWidthForWidthAxisValue(coord.value);
                    break;
                }
            }
            *style = SkFontStyle(
                    entry.fWeight,
                    width,
                    entry.fIsItalic ? SkFontStyle::kItalic_Slant : SkFontStyle::kUpright_Slant);
        }
        if (name) {
            name->reset();
        }
    }

    sk_sp<SkTypeface_AndroidNDK> createATypeface(int index);

    sk_sp<SkTypeface> createTypeface(int index) override {
        return this->createATypeface(index);
    }

    sk_sp<SkTypeface_AndroidNDK> matchAStyle(const SkFontStyle& pattern) {
        sk_sp<SkTypeface> match = this->matchStyleCSS3(pattern);
        return sk_sp<SkTypeface_AndroidNDK>(static_cast<SkTypeface_AndroidNDK*>(match.release()));
    }

    sk_sp<SkTypeface> matchStyle(const SkFontStyle& pattern) override {
        if (fEntries.size() == 1) {
            return this->createTypeface(0);
        }
        return adjustForStyle(this->matchAStyle(pattern), pattern, *fCache);
    }

private:
    sk_sp<const SkFontMgr_AndroidNDK> fFontMgr;
    SkString fFamilyName;
    TArray<FontEntry> fEntries;
    sk_sp<TypefaceCache> fCache;
};

class SkFontMgr_AndroidNDK : public SkFontMgr {
public:
    SkFontMgr_AndroidNDK(AndroidFontAPI&& fontAPI,
                          std::unique_ptr<SkFontScanner> scanner)
        : fAPI(std::move(fontAPI))
        , fScanner(std::move(scanner))
        , fCache(new TypefaceCache())
    {
        fFontManager = fAPI.AFontManager_create();
        if (fFontManager) {
            size_t famCount = fAPI.AFontManager_getNamedFontFamilySize(fFontManager);
            for (size_t i = 0; i < famCount; ++i) {
                const AFontFamilyDescriptor* fam =
                        fAPI.AFontManager_getNamedFontFamily(fFontManager, i);
                if (!fam) {
                    continue;
                }
                const char* name = fAPI.AFontFamilyDescriptor_getName(fam);
                if (name && name[0] != '\0') {
                    SkString skName(name);
                    SkString normName(fICU.casefold({skName.data(), skName.size()}));
                    fAvailableNames.push_back({std::move(skName), std::move(normName)});
                }
            }
            size_t aliasCount = fAPI.AFontManager_getAliasSize(fFontManager);
            for (size_t i = 0; i < aliasCount; ++i) {
                const AAliasDescriptor* alias = fAPI.AFontManager_getAlias(fFontManager, i);
                if (!alias) {
                    continue;
                }
                const char* name = fAPI.AAliasDescriptor_getName(alias);
                if (name && name[0] != '\0') {
                    SkString skName(name);
                    SkString normName(fICU.casefold({skName.data(), skName.size()}));
                    fAvailableNames.push_back({std::move(skName), std::move(normName)});
                }
            }
        }
    }

    ~SkFontMgr_AndroidNDK() override {
        if (fFontManager) {
            fAPI.AFontManager_destroy(fFontManager);
        }
    }

    sk_sp<SkTypeface_AndroidNDK> getOrCreateTypeface(const FontEntry& entry,
                                                     const SkString& familyName) const {
        SkString key(entry.fFilePath);
        key.appendf("#%zu#%d#%d", entry.fCollectionIndex, entry.fWeight, entry.fIsItalic ? 1 : 0);
        STArray<4, SkFontArguments::VariationPosition::Coordinate> sortedAxes(entry.fAxes);
        SkTQSort(sortedAxes.begin(), sortedAxes.end(), [](const auto& a, const auto& b) {
            return a.axis < b.axis;
        });
        for (const auto& coord : sortedAxes) {
            key.appendf("#%u:%.2f", coord.axis, coord.value);
        }

        {
            SkAutoSharedMutexShared lock(fMutex);
            if (sk_sp<SkTypeface_AndroidNDK>* found = fCreatedTypefaces.find(key)) {
                return *found;
            }
        }

        std::unique_ptr<SkStreamAsset> baseStream;
        {
            SkAutoSharedMutexShared lock(fMutex);
            if (std::unique_ptr<SkStreamAsset>* cachedStream =
                        fStreamForPath.find(entry.fFilePath)) {
                if (*cachedStream) {
                    baseStream = (*cachedStream)->duplicate();
                }
            }
        }
        if (!baseStream) {
            auto opened = SkStream::MakeFromFile(entry.fFilePath.c_str());
            if (opened) {
                baseStream = opened->duplicate();
                SkAutoSharedMutexExclusive lock(fMutex);
                fStreamForPath.set(entry.fFilePath, std::move(opened));
            }
        }
        if (!baseStream) {
            return nullptr;
        }

        std::optional<int> requestedWidth;
        for (const auto& coord : entry.fAxes) {
            if (coord.axis == variation::wdthTag) {
                requestedWidth = SkFontDescriptor::SkFontStyleWidthForWidthAxisValue(coord.value);
            }
        }

        SkFontArguments::VariationPosition requestedPos = {
                entry.fAxes.data(), SkTo<int>(entry.fAxes.size())};
        auto proxy = fScanner->MakeFromStream(
                std::move(baseStream),
                SkFontArguments()
                        .setCollectionIndex(entry.fCollectionIndex)
                        .setVariationDesignPosition(requestedPos));
        if (!proxy) {
            return nullptr;
        }

        variation::Storage variationStorage;
        SkSpan<variation::Coordinate> variation = variation::Get(*proxy, variationStorage);
        SkTypeface_AndroidNDK::AutoAxis autoAxis(variation);
        autoAxis.remove(SkTypeface_AndroidNDK::AutoAxis(
                SkSpan(requestedPos.coordinates, requestedPos.coordinateCount)));

        SkFontStyle proxyStyle = proxy->fontStyle();
        SkFontStyle::Slant slant =
                entry.fIsItalic ? SkFontStyle::kItalic_Slant : proxyStyle.slant();
        int width = requestedWidth.value_or(proxyStyle.width());
        SkFontStyle finalStyle(entry.fWeight, width, slant);

        SkString resolvedFamilyName = familyName;
        if (resolvedFamilyName.isEmpty()) {
            proxy->getFamilyName(&resolvedFamilyName);
        }

        STArray<4, SkALanguage> skLangs;
        if (!entry.fLocale.isEmpty()) {
            const char* begin = entry.fLocale.c_str();
            const char* end = begin;
            while (true) {
                while (*end != '\0' && *end != ',') {
                    ++end;
                }
                if (end > begin) {
                    skLangs.emplace_back(SkSpan(begin, static_cast<size_t>(end - begin)));
                }
                if (*end == '\0') {
                    break;
                }
                ++end;
                begin = end;
            }
        }

        sk_sp<SkTypeface_AndroidNDK> created = SkTypeface_AndroidNDK::Make(
                std::move(proxy),
                finalStyle,
                false,
                resolvedFamilyName,
                TArray<SkString>(),
                std::move(skLangs),
                autoAxis);

        SkAutoSharedMutexExclusive lock(fMutex);
        if (sk_sp<SkTypeface_AndroidNDK>* found = fCreatedTypefaces.find(key)) {
            return *found;
        }
        fCreatedTypefaces.set(key, created);
        return created;
    }

protected:
    int onCountFamilies() const override {
        return fAvailableNames.size();
    }

    void onGetFamilyName(int index, SkString* familyName) const override {
        if (index < 0 || fAvailableNames.size() <= index) {
            familyName->reset();
            return;
        }
        *familyName = fAvailableNames[index].fName;
    }

    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override {
        if (index < 0 || fAvailableNames.size() <= index) {
            return nullptr;
        }
        return this->onMatchFamily(fAvailableNames[index].fName.c_str());
    }

    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override {
        if (!familyName || familyName[0] == '\0') {
            return nullptr;
        }
        SkString normalized(fICU.casefold({familyName, strlen(familyName)}));
        {
            SkAutoSharedMutexShared lock(fMutex);
            if (sk_sp<SkFontStyleSet>* cached = fStyleSetCache.find(normalized)) {
                return *cached;
            }
        }

        const char* lookupName = familyName;
        for (const auto& pair : fAvailableNames) {
            if (pair.fNormalized == normalized) {
                lookupName = pair.fName.c_str();
                break;
            }
        }

        TArray<FontEntry> entries;
        SkString displayFamilyName(familyName);

        if (fFontManager) {
            // 1. Check aliases and named font families from FontFamilyDescriptor first
            const char* targetFamilyName = lookupName;
            uint16_t aliasWeight = 0;
            size_t aliasCount = fAPI.AFontManager_getAliasSize(fFontManager);
            for (size_t i = 0; i < aliasCount; ++i) {
                const AAliasDescriptor* alias = fAPI.AFontManager_getAlias(fFontManager, i);
                if (!alias) {
                    continue;
                }
                const char* aliasName = fAPI.AAliasDescriptor_getName(alias);
                if (aliasName && strcmp(aliasName, lookupName) == 0) {
                    const char* orig = fAPI.AAliasDescriptor_getOriginal(alias);
                    if (orig && orig[0] != '\0') {
                        targetFamilyName = orig;
                        aliasWeight = fAPI.AAliasDescriptor_getWeight(alias);
                        displayFamilyName.set(aliasName);
                    }
                    break;
                }
            }

            size_t famCount = fAPI.AFontManager_getNamedFontFamilySize(fFontManager);
            for (size_t i = 0; i < famCount; ++i) {
                const AFontFamilyDescriptor* famDesc =
                        fAPI.AFontManager_getNamedFontFamily(fFontManager, i);
                if (!famDesc) {
                    continue;
                }
                const char* descName = fAPI.AFontFamilyDescriptor_getName(famDesc);
                if (descName && strcmp(descName, targetFamilyName) == 0) {
                    if (aliasWeight == 0) {
                        displayFamilyName.set(descName);
                    }
                    this->appendEntriesFromAFontFamilyDescriptor(famDesc, aliasWeight, entries);
                }
            }

            // 2. If not matched by named family/alias, query the font name map
            // via AFontManager_findByFamilyName
            if (entries.empty()) {
                if (AFontMatchResult* matchResult =
                            fAPI.AFontManager_findByFamilyName(fFontManager, lookupName)) {
                    size_t count = fAPI.AFontMatchResult_getSize(matchResult);
                    for (size_t i = 0; i < count; ++i) {
                        const char* path = fAPI.AFontMatchResult_getFilePath(matchResult, i);
                        if (!path) {
                            continue;
                        }
                        FontEntry entry;
                        entry.fFilePath.set(path);
                        entry.fCollectionIndex =
                                fAPI.AFontMatchResult_getCollectionIndex(matchResult, i);
                        auto stream = SkStream::MakeFromFile(path);
                        if (stream) {
                            if (auto proxy = fScanner->MakeFromStream(
                                        std::move(stream),
                                        SkFontArguments().setCollectionIndex(
                                                entry.fCollectionIndex))) {
                                SkFontStyle fs = proxy->fontStyle();
                                entry.fWeight = fs.weight();
                                entry.fIsItalic = (fs.slant() != SkFontStyle::kUpright_Slant);
                            }
                        }
                        entries.push_back(std::move(entry));
                    }
                    fAPI.AFontMatchResult_destroy(matchResult);
                    if (!entries.empty()) {
                        displayFamilyName.set(lookupName);
                    }
                }
            }
        }

        if (entries.empty()) {
            return nullptr;
        }

        sk_sp<SkFontStyleSet> styleSet(new SkFontStyleSet_AndroidNDK(
                sk_ref_sp(this), std::move(displayFamilyName), std::move(entries), fCache));
        {
            SkAutoSharedMutexExclusive lock(fMutex);
            fStyleSetCache.set(normalized, styleSet);
        }
        return styleSet;
    }

    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                         const SkFontStyle& style) const override {
        sk_sp<SkFontStyleSet> sset = this->onMatchFamily(familyName);
        if (!sset) {
            return nullptr;
        }
        return sset->matchStyle(style);
    }

    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[],
                                                  const SkFontStyle& style,
                                                  const char* bcp47[],
                                                  int bcp47Count,
                                                  SkUnichar character) const override {
        // 1. Check requested family first if provided
        if (familyName && familyName[0] != '\0') {
            sk_sp<SkTypeface> famTypeface = this->onMatchFamilyStyle(familyName, style);
            if (famTypeface && famTypeface->unicharToGlyph(character) != 0) {
                return famTypeface;
            }
        }

        // 2. Fallback matching using AFontMatcher_match
        if (fAPI.AFontMatcher_create) {
            uint16_t utf16[4];
            size_t utf16Len = SkUTF::ToUTF16(character, utf16);
            AFontMatcher* matcher = fAPI.AFontMatcher_create();
            if (matcher) {
                fAPI.AFontMatcher_setStyle(
                        matcher,
                        static_cast<uint16_t>(style.weight()),
                        style.slant() != SkFontStyle::kUpright_Slant);
                if (bcp47Count > 0) {
                    SkString locales;
                    for (int i = 0; i < bcp47Count; ++i) {
                        if (bcp47[i] && bcp47[i][0] != '\0') {
                            if (!locales.isEmpty()) {
                                locales.append(",");
                            }
                            locales.append(bcp47[i]);
                        }
                    }
                    if (!locales.isEmpty()) {
                        fAPI.AFontMatcher_setLocales(matcher, locales.c_str());
                    }
                }
                uint32_t runLen = 0;
                AFont* matchedAFont = fAPI.AFontMatcher_match(
                        matcher,
                        familyName,
                        utf16,
                        static_cast<uint32_t>(utf16Len),
                        &runLen);
                fAPI.AFontMatcher_destroy(matcher);
                if (matchedAFont) {
                    FontEntry entry;
                    const char* path = fAPI.AFont_getFontFilePath(matchedAFont);
                    if (path) {
                        entry.fFilePath.set(path);
                    }
                    entry.fCollectionIndex = fAPI.AFont_getCollectionIndex(matchedAFont);
                    entry.fWeight = fAPI.AFont_getWeight(matchedAFont);
                    entry.fIsItalic = fAPI.AFont_isItalic(matchedAFont);
                    const char* loc = fAPI.AFont_getLocale(matchedAFont);
                    if (loc) {
                        entry.fLocale.set(loc);
                    }
                    size_t axisCount = fAPI.AFont_getAxisCount(matchedAFont);
                    for (size_t i = 0; i < axisCount; ++i) {
                        entry.fAxes.push_back({fAPI.AFont_getAxisTag(matchedAFont, i),
                                               fAPI.AFont_getAxisValue(matchedAFont, i)});
                    }
                    fAPI.AFont_close(matchedAFont);
                    sk_sp<SkTypeface_AndroidNDK> tf =
                            this->getOrCreateTypeface(entry, SkString());
                    if (tf && tf->unicharToGlyph(character) != 0) {
                        return adjustForStyle(std::move(tf), style, *fCache);
                    }
                }
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
        return this->makeFromStream(std::move(stream),
                                    SkFontArguments().setCollectionIndex(ttcIndex));
    }

    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                            const SkFontArguments& args) const override {
        return fScanner->MakeFromStream(std::move(stream), args);
    }

    sk_sp<SkTypeface> onLegacyMakeTypeface(const char name[], SkFontStyle style) const override {
        if (name) {
            return this->onMatchFamilyStyle(name, style);
        }
        return this->onMatchFamilyStyle("sans-serif", style);
    }

private:
    struct AvailableName {
        SkString fName;
        SkString fNormalized;
    };

    void appendEntriesFromAFontFamilyDescriptor(const AFontFamilyDescriptor* famDesc,
                                                uint16_t aliasWeight,
                                                TArray<FontEntry>& outEntries) const {
        if (!famDesc) {
            return;
        }
        const char* loc = fAPI.AFontFamilyDescriptor_getLocaleList(famDesc);
        size_t fontCount = fAPI.AFontFamilyDescriptor_getFontSize(famDesc);
        for (size_t i = 0; i < fontCount; ++i) {
            const AFontDescriptor* fontDesc = fAPI.AFontFamilyDescriptor_getFont(famDesc, i);
            if (!fontDesc) {
                continue;
            }
            uint16_t w = fAPI.AFontDescriptor_getWeight(fontDesc);
            if (aliasWeight != 0 && w != aliasWeight) {
                continue;
            }
            FontEntry entry;
            const char* path = fAPI.AFontDescriptor_getFilePath(fontDesc);
            if (path) {
                entry.fFilePath.set(path);
            }
            entry.fCollectionIndex = 0;
            entry.fWeight = w;
            entry.fIsItalic = (fAPI.AFontDescriptor_getStyle(fontDesc) != 0);
            if (loc) {
                entry.fLocale.set(loc);
            }
            size_t varCount = fAPI.AFontDescriptor_getVariationSettingSize(fontDesc);
            for (size_t k = 0; k < varCount; ++k) {
                entry.fAxes.push_back({fAPI.AFontDescriptor_getVariationSettingTag(fontDesc, k),
                                       fAPI.AFontDescriptor_getVariationSettingValue(fontDesc, k)});
            }
            outEntries.push_back(std::move(entry));
        }
    }

    AndroidFontAPI fAPI;
    AndroidIcuAPI fICU;
    std::unique_ptr<SkFontScanner> fScanner;
    sk_sp<TypefaceCache> fCache;

    AFontManager* fFontManager = nullptr;
    TArray<AvailableName> fAvailableNames;

    mutable SkSharedMutex fMutex;
    mutable skia_private::THashMap<SkString, sk_sp<SkFontStyleSet>> fStyleSetCache;
    mutable skia_private::THashMap<SkString, std::unique_ptr<SkStreamAsset>> fStreamForPath;
    mutable skia_private::THashMap<SkString, sk_sp<SkTypeface_AndroidNDK>> fCreatedTypefaces;
};

sk_sp<SkTypeface_AndroidNDK> SkFontStyleSet_AndroidNDK::createATypeface(int index) {
    if (index < 0 || fEntries.size() <= index) {
        return nullptr;
    }
    return fFontMgr->getOrCreateTypeface(fEntries[index], fFamilyName);
}

}  // namespace

sk_sp<SkFontMgr> SkFontMgr_New_AndroidNDK([[maybe_unused]] bool cacheFontFiles,
                                          std::unique_ptr<SkFontScanner> scanner)
{
    std::optional<AndroidFontAPI> fontAPI = AndroidFontAPI::Make();
    if (!fontAPI) {
        return nullptr;
    }
    return sk_sp(new SkFontMgr_AndroidNDK(*std::move(fontAPI), std::move(scanner)));
}

