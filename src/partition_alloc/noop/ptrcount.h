/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// LOCAL INSTRUMENTATION -- DO NOT LAND.
//
// Counts raw_ptr operations per pointee type.
//
// BackupRefPtr is free on dereference; it costs a refcount update on
// construction, copy and assignment.  So the expensive declarations are
// exactly the ones built most often per frame, and that is a property of the
// program, not of the machine -- it can be counted locally and exactly,
// instead of being inferred from a 2.2%-resolution benchmark on a borrowed
// Windows laptop.
//
// Counting is per pointee type rather than per declaration site because a
// class template cannot see its own declaration's __LINE__, and because the
// regressions found so far are per-class anyway (KeyContext, the *Priv
// wrappers, SkAutoSpinlock).
//
// Set SK_PTRCOUNT_OUT to a path to dump on exit; otherwise stderr.

#ifndef SRC_PARTITION_ALLOC_NOOP_PTRCOUNT_H_
#define SRC_PARTITION_ALLOC_NOOP_PTRCOUNT_H_

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>

namespace sk_ptrcount {

struct Site {
    const char* name;
    std::atomic<uint64_t> ops;
    // A few distinct caller addresses, so a type that only ever appears
    // through a template instantiation can be resolved with addr2line.
    std::atomic<int> npc;
    void* pcs[8];
};

inline std::vector<Site*>& registry() {
    static std::vector<Site*>* r = new std::vector<Site*>();
    return *r;
}

inline std::mutex& registry_mutex() {
    static std::mutex* m = new std::mutex();
    return *m;
}

inline void dump() {
    std::vector<Site*> sites;
    {
        std::lock_guard<std::mutex> lock(registry_mutex());
        sites = registry();
    }
    std::stable_sort(sites.begin(), sites.end(), [](const Site* a, const Site* b) {
        return a->ops.load() > b->ops.load();
    });
    const char* path = std::getenv("SK_PTRCOUNT_OUT");
    std::FILE* f = path ? std::fopen(path, "w") : stderr;
    if (!f) {
        return;
    }
    // The binary is PIE, so the captured addresses are only meaningful
    // relative to the load base.  Record it.
    if (std::FILE* maps = std::fopen("/proc/self/maps", "r")) {
        char line[512];
        if (std::fgets(line, sizeof(line), maps)) {
            std::fprintf(f, "#base\t%s", line);
        }
        std::fclose(maps);
    }
    for (const Site* s : sites) {
        uint64_t n = s->ops.load();
        if (n) {
            std::fprintf(f, "%llu\t%s", (unsigned long long)n, s->name);
            int k = s->npc.load();
            for (int i = 0; i < k && i < 8; ++i) {
                std::fprintf(f, "\t%p", s->pcs[i]);
            }
            std::fprintf(f, "\n");
        }
    }
    if (path) {
        std::fclose(f);
    }
}

// `__PRETTY_FUNCTION__` is the only portable-enough way to name T without
// RTTI, which Skia builds without.  The caller trims it.
template <typename T> const char* type_name() {
#if defined(__GNUC__) || defined(__clang__)
    return __PRETTY_FUNCTION__;
#else
    return "?";
#endif
}

template <typename T> Site& site() {
    static Site* s = [] {
        Site* p = new Site{type_name<T>(), {0}, {0}, {}};
        std::lock_guard<std::mutex> lock(registry_mutex());
        registry().push_back(p);
        static std::once_flag once;
        std::call_once(once, [] { std::atexit(dump); });
        return p;
    }();
    return *s;
}

template <typename T> inline void bump() {
    Site& s = site<T>();
    s.ops.fetch_add(1, std::memory_order_relaxed);
    if (s.npc.load(std::memory_order_relaxed) < 8) {
        // Frame 0 is always inside raw_ptr.h -- the shim's constructor is a
        // real call in this unoptimised build.  Frame 1 is the code that
        // actually constructed the pointer, which is what we want to name.
        void* pc = __builtin_return_address(1);
        std::lock_guard<std::mutex> lock(registry_mutex());
        int k = s.npc.load(std::memory_order_relaxed);
        for (int i = 0; i < k; ++i) {
            if (s.pcs[i] == pc) {
                return;
            }
        }
        if (k < 8) {
            s.pcs[k] = pc;
            s.npc.store(k + 1, std::memory_order_relaxed);
        }
    }
}

}  // namespace sk_ptrcount

#define SK_PTRCOUNT_BUMP() ::sk_ptrcount::bump<T>()

#endif  // SRC_PARTITION_ALLOC_NOOP_PTRCOUNT_H_
