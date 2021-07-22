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

static void* resolve_sym(void* dlhnd, const char name[], int ver, bool* resolved = nullptr) {
    const auto sym = SkStringPrintf("%s_%d", name, ver);
    auto* addr = dlsym(dlhnd, sym.c_str());

    SkDebugf("*** resolving symbol %s -> %p\n", name, addr);

    if (resolved) {
        *resolved &= !!addr;
    }
    return addr;
}

static int find_version(void* dlhnd) {
    static const char kTestSymbol[] = "u_getVersion";
    static constexpr int kMinVersion =  44,
                         kMaxVersion = 100;

    for (auto i = kMinVersion; i <= kMaxVersion; ++i) {
        if (resolve_sym(dlhnd, kTestSymbol, i)) {
            return i;
        }
    }

    return -1;
}

static std::unique_ptr<SkICUFuncs> load_icu() {
    static constexpr char* const gLibPaths[] = {
        "libicuuc.so",
        "/usr/lib/libicuuc.so",
        "/usr/lib64/libicuuc.so",
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
        return nullptr;
    }

    const auto v = find_version(dlhnd);
    if (v < 0) {
        dlclose(dlhnd);
        return nullptr;
    }

    bool resolved_syms = true;
    #define SKICU_FUNC(f) reinterpret_cast<decltype(f)*>(resolve_sym(dlhnd, #f, v, &resolved_syms)),
    SkICUFuncs funcs {
        SKICU_EMIT_FUNCS

        reinterpret_cast<decltype(SkICUFuncs::f_ubrk_clone_)>(resolve_sym(dlhnd, "ubrk_clone", v)),
        reinterpret_cast<decltype(SkICUFuncs::f_ubrk_safeClone_)>(resolve_sym(dlhnd, "ubrk_safeClone", v)),
    };

    return resolved_syms && (funcs.f_ubrk_clone_ || funcs.f_ubrk_safeClone_)
        ? std::make_unique<SkICUFuncs>(funcs)
        : nullptr;
}

const SkICUFuncs* SkRuntimeICU() {
    static const auto gICU = load_icu();

    return gICU.get();
}
