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
#include <errno.h>

static std::unique_ptr<SkICUFuncs> load_icu() {
    static constexpr char const* gLibPaths[] = {
        // Android, Linux
        "libicuuc.so",
        // Mac
        "/usr/local/opt/icu4c/lib/libicuuc.dylib",
    };

    void* dlhnd = nullptr;
    for (const auto path : gLibPaths) {
        dlhnd = dlopen(path, RTLD_LAZY);
        if (dlhnd) {
            break;
        }
    }

    if (!dlhnd) {
        SkDEBUGF("ICU loader: failed to open libicuuc.");
        return nullptr;
    }

    int icu_ver = -1;

    bool resolved_required_syms = true;

    auto resolve_sym = [&](void* hnd, const char name[], bool required = false) -> void* {
        static constexpr int kMinVer =  44,
                             kMaxVer = 100;
        // First call performs a search to determine the actual lib version.
        // Subsequent calls are pinned to the found version.
        const auto search_to = icu_ver > 0 ? icu_ver : kMaxVer;
        icu_ver              = icu_ver > 0 ? icu_ver : kMinVer;

        for (;;) {
            const auto sym = SkStringPrintf("%s_%d", name, icu_ver);
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

    #define SKICU_FUNC(f) reinterpret_cast<decltype(f)*>(resolve_sym(dlhnd, #f, true)),
    SkICUFuncs funcs {
        SKICU_EMIT_FUNCS

        reinterpret_cast<decltype(SkICUFuncs::f_ubrk_clone_)>(
            resolve_sym(dlhnd, "ubrk_clone")),
        reinterpret_cast<decltype(SkICUFuncs::f_ubrk_safeClone_)>(
            resolve_sym(dlhnd, "ubrk_safeClone")),
    };

    if (!resolved_required_syms || (!funcs.f_ubrk_clone_ && !funcs.f_ubrk_safeClone_)) {
        SkDEBUGF("ICU loader: failed to resolve required symbols.");
        dlclose(dlhnd);
        return nullptr;
    }

    return std::make_unique<SkICUFuncs>(funcs);
}

const SkICUFuncs* SkRuntimeICU() {
    static const auto gICU = load_icu();

    return gICU.get();
}
