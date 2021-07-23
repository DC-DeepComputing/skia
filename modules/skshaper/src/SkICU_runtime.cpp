/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "modules/skshaper/src/SkICU.h"

#include <dlfcn.h>

#ifndef SK_RUNTIME_ICU_PATHS
    #define SK_RUNTIME_ICU_PATHS "libicuuc.so"
#endif

#ifndef SK_RUNTIME_ICU_SYMPREFIX
    #define SK_RUNTIME_ICU_SYMPREFIX
#endif

#ifndef SK_RUNTIME_ICU_SYMSUFFIX
    #define SK_RUNTIME_ICU_SYMSUFFIX
#endif

#define TOSTR_(x...) #x
#define STRINGIFY(x) TOSTR_(x)

std::unique_ptr<SkICULib> SkLoadICULib() {
    static constexpr char const* gLibPaths[] = { SK_RUNTIME_ICU_PATHS };
    static_assert (SK_ARRAY_COUNT(gLibPaths) > 0,
                   "Runtime ICU requires a non-empty library lookup list (SK_RUNTIME_ICU_PATHS)");

    void* dlhnd = nullptr;
    for (const auto path : gLibPaths) {
        dlhnd = dlopen(path, RTLD_LAZY);
        if (dlhnd) {
            break;
        }
    }

    if (!dlhnd) {
        SkDEBUGF("ICU loader: failed to open libicuuc (%s).\n", STRINGIFY(SK_RUNTIME_ICU_PATHS));
        return nullptr;
    }

    int icu_ver = -1;

    bool resolved_required_syms = true;

    auto resolve_sym = [&](void* hnd, const char name[], bool required = false) -> void* {
        static constexpr int kMinVer =  44,
                             kMaxVer = 100;

        // First call performs a search to determine the actual lib version.
        // Subsequent calls are pinned to the version found.
        const auto search_to = icu_ver > 0 ? icu_ver : kMaxVer;
        icu_ver              = icu_ver > 0 ? icu_ver : kMinVer - 1;

        for (;;) {
            // First try is just client provided prefix/suffix,
            // while subsequent attempts include an additional version suffix.
            auto sym = SkStringPrintf("%s%s%s",
                                      STRINGIFY(SK_RUNTIME_ICU_SYMPREFIX),
                                      name,
                                      STRINGIFY(SK_RUNTIME_ICU_SYMSUFFIX));
            if (icu_ver >= kMinVer) {
                sym.appendf("_%d", icu_ver);
            }

            if (auto* addr = dlsym(dlhnd, sym.c_str())) {
                return addr;
            }

            if (icu_ver == search_to) {
                break;
            }

            icu_ver++;
        }

        if (required) {
            resolved_required_syms = false;
        }
        return nullptr;
    };

    SkICULib lib {};

    // When using dlsym
    // *(void**)(&procPtr) = dlsym(self, "proc");
    // is non-standard, but safe for POSIX. Cannot write
    // *reinterpret_cast<void**>(&procPtr) = dlsym(self, "proc");
    // because clang has not implemented DR573. See http://clang.llvm.org/cxx_dr_status.html .
    #define SKICU_FUNC(fname) *(void**)(&lib.f_##fname) = resolve_sym(dlhnd, #fname, true);
    SKICU_EMIT_FUNCS

    *(void**)(&lib.f_ubrk_clone_)     = resolve_sym(dlhnd, "ubrk_clone");
    *(void**)(&lib.f_ubrk_safeClone_) = resolve_sym(dlhnd, "ubrk_safeClone");

    if (!resolved_required_syms || (!lib.f_ubrk_clone_ && !lib.f_ubrk_safeClone_)) {
        SkDEBUGF("ICU loader: failed to resolve required symbols.");
        dlclose(dlhnd);
        return nullptr;
    }

    return std::make_unique<SkICULib>(lib);
}
