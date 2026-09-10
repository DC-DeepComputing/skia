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

// Modelling the cost properly needs more than an operation count.  The
// refcount BackupRefPtr updates lives in the allocation's own slot metadata,
// so an operation on an object that is already in cache costs a couple of
// cycles, while an operation on a cold object costs a cache miss -- two
// orders of magnitude apart.  A declaration touched a million times on one
// hot object is cheap; a declaration touched ten thousand times across ten
// thousand distinct objects is not.
//
// So each type also carries a small direct-mapped table of recently touched
// cache lines.  A "miss" is an operation on a line the type has not touched
// recently, which is the quantity that actually costs.
inline constexpr int kLineTableBits = 10;
inline constexpr int kLineTableSize = 1 << kLineTableBits;

struct Site {
    const char* name;
    std::atomic<uint64_t> ops;
    std::atomic<uint64_t> misses;
    // A few distinct caller addresses, so a type that only ever appears
    // through a template instantiation can be resolved with addr2line.
    std::atomic<int> npc;
    void* pcs[8];
    // Raced on deliberately: this is a heuristic, and a lock here would cost
    // more than the thing being measured.
    std::atomic<uint64_t> lines[kLineTableSize];
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
        return a->misses.load() > b->misses.load();
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
            std::fprintf(f, "%llu\t%llu\t%s",
                         (unsigned long long)s->misses.load(),
                         (unsigned long long)n, s->name);
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
        Site* p = new Site{type_name<T>(), {0}, {0}, {0}, {}, {}};
        std::lock_guard<std::mutex> lock(registry_mutex());
        registry().push_back(p);
        static std::once_flag once;
        std::call_once(once, [] { std::atexit(dump); });
        return p;
    }();
    return *s;
}

template <typename T> inline void bump(const void* p) {
    Site& s = site<T>();
    s.ops.fetch_add(1, std::memory_order_relaxed);

    // Cache-line granularity: BackupRefPtr's refcount sits in the slot
    // metadata of the allocation the pointer refers to.
    const uint64_t line = reinterpret_cast<uintptr_t>(p) >> 6;
    if (line) {
        std::atomic<uint64_t>& slot = s.lines[line & (kLineTableSize - 1)];
        if (slot.load(std::memory_order_relaxed) != line) {
            slot.store(line, std::memory_order_relaxed);
            s.misses.fetch_add(1, std::memory_order_relaxed);
        }
    }

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

#define SK_PTRCOUNT_BUMP(p) ::sk_ptrcount::bump<T>(p)

#endif  // SRC_PARTITION_ALLOC_NOOP_PTRCOUNT_H_
