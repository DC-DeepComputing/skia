/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFeatures.h"  // IWYU pragma: keep
#include "modules/skunicode/src/SkUnicode_icupriv.h"

#if defined(SK_USING_THIRD_PARTY_ICU) && defined(SK_BUILD_FOR_WIN)
#include "SkLoadICU.h"
#include "include/private/base/SkOnce.h"
#endif

#include <unicode/ubrk.h>
#include <unicode/uloc.h>
#include <unicode/utypes.h>
#include <memory>
#include <type_traits>
#include <utility>

namespace {

// ubrk_clone added as draft in ICU69 and Android API 31 (first ICU NDK).
// ubrk_safeClone deprecated in ICU69 and not exposed by Android.
template<typename T, typename = void>
struct SkUbrkClone {
    static UBreakIterator* clone(T bi, UErrorCode* status) {
        return ubrk_safeClone(bi, nullptr, nullptr, status);
    }
};
template<typename T>
struct SkUbrkClone<T, std::void_t<decltype(ubrk_clone(std::declval<T>(), nullptr))>> {
    static UBreakIterator* clone(T bi, UErrorCode* status) {
        return ubrk_clone(bi, status);
    }
};

// ubrk_getLocaleByType has been in ICU since version 2.8
// However, it was not included in the Android NDK
template<typename T, typename = void>
struct SkUbrkGetLocaleByType {
    static const char* getLocaleByType(T bi, ULocDataLocaleType type, UErrorCode* status) {
        *status = U_UNSUPPORTED_ERROR;
        return nullptr;
    }
};
template<typename T>
struct SkUbrkGetLocaleByType<
    T,
    std::void_t<decltype(ubrk_getLocaleByType(std::declval<T>(),
                                              std::declval<ULocDataLocaleType>(),
                                              nullptr))>>
{
    static const char* getLocaleByType(T bi, ULocDataLocaleType type, UErrorCode* status) {
        return ubrk_getLocaleByType(bi, type, status);
    }
};

}  // namespace

#define SKICU_FUNC(funcname) funcname,
std::unique_ptr<SkICULib> SkLoadICULib() {
    // We haven't yet created a way to encode the ICU data for assembly on Windows,
    // so we use a helper library to load icudtl.dat from the harddrive.
#if defined(SK_USING_THIRD_PARTY_ICU) && defined(SK_BUILD_FOR_WIN)
    static SkOnce once;
    once([] {
        if (!SkLoadICU()) {
            SkDEBUGF("SkLoadICU() failed!\n");
            return nullptr;
        }
    });
#endif
    return std::make_unique<SkICULib>(SkICULib{
        SKICU_EMIT_FUNCS
        &SkUbrkClone<const UBreakIterator*>::clone,
        nullptr,
        &SkUbrkGetLocaleByType<const UBreakIterator*>::getLocaleByType,
    });
}

