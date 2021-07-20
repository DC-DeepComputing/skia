/*
* Copyright 2021 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#ifndef SkICU_DEFINED
#define SkICU_DEFINED

#include <unicode/ubidi.h>
#include <unicode/ubrk.h>
#include <unicode/uscript.h>
#include <unicode/ustring.h>
#include <unicode/utext.h>
#include <unicode/utypes.h>

#include "include/core/SkTypes.h"

#define SK_ICU_FUNC(funcname) decltype(funcname)* f_##funcname;

#define SK_ICU_FUNCS                   \
    SK_ICU_FUNC(u_errorName)           \
    SK_ICU_FUNC(u_getIntPropertyValue) \
    SK_ICU_FUNC(u_iscntrl)             \
    SK_ICU_FUNC(u_isspace)             \
    SK_ICU_FUNC(u_isWhitespace)        \
    SK_ICU_FUNC(ubidi_close)           \
    SK_ICU_FUNC(ubidi_getLength)       \
    SK_ICU_FUNC(ubidi_getLevelAt)      \
    SK_ICU_FUNC(ubidi_openSized)       \
    SK_ICU_FUNC(ubidi_reorderVisual)   \
    SK_ICU_FUNC(ubidi_setPara)         \
    SK_ICU_FUNC(ubrk_close)            \
    SK_ICU_FUNC(ubrk_current)          \
    SK_ICU_FUNC(ubrk_first)            \
    SK_ICU_FUNC(ubrk_following)        \
    SK_ICU_FUNC(ubrk_getRuleStatus)    \
    SK_ICU_FUNC(ubrk_next)             \
    SK_ICU_FUNC(ubrk_open)             \
    SK_ICU_FUNC(ubrk_preceding)        \
    SK_ICU_FUNC(ubrk_setText)          \
    SK_ICU_FUNC(ubrk_setUText)         \
    SK_ICU_FUNC(uloc_getDefault)       \
    SK_ICU_FUNC(uscript_getScript)     \
    SK_ICU_FUNC(utext_close)           \
    SK_ICU_FUNC(utext_openUChars)      \
    SK_ICU_FUNC(utext_openUTF8)        \


struct SkICUFuncs {
    SK_ICU_FUNCS

    // ubrk_clone added as draft in ICU69 and Android API 31 (first ICU NDK).
    // ubrk_safeClone deprecated in ICU69 and not exposed by Android.
    UBreakIterator* (*f_ubrk_clone_)(const UBreakIterator*, UErrorCode*);
    UBreakIterator* (*f_ubrk_safeClone_)(const UBreakIterator*, void*, int32_t*, UErrorCode*);

    UBreakIterator* f_ubrk_clone(const UBreakIterator* bi, UErrorCode* status) const {
        SkASSERT(f_ubrk_clone_ || f_ubrk_safeClone_);
        return f_ubrk_clone_
            ? f_ubrk_clone_(bi, status)
            : f_ubrk_safeClone_(bi, nullptr, nullptr, status);
    }
};

#undef SK_ICU_FUNC

SkICUFuncs SkLoadICUFuncs();

#endif // SkICU_DEFINED
