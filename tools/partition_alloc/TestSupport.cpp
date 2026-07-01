/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tools/partition_alloc/TestSupport.h"

#include <partition_alloc/buildflags.h>

#if PA_BUILDFLAG(USE_ALLOCATOR_SHIM)
#include <partition_alloc/shim/allocator_shim.h>
#include <partition_alloc/shim/allocator_shim_default_dispatch_to_partition_alloc.h>
#endif

#if PA_BUILDFLAG(ENABLE_DANGLING_RAW_PTR_CHECKS)
#include <partition_alloc/dangling_raw_ptr_checks.h>
#include "include/core/SkTypes.h"
#include "include/private/SkMutex.h"

#include <algorithm>
#include <array>
#include <cstring>

#if defined(SK_BUILD_FOR_WIN)
#include <windows.h>
#include <DbgHelp.h>
#else
#include <unwind.h>
#include <dlfcn.h>
#include <cxxabi.h>
#endif
#endif

namespace skiatest {

void InitializePartitionAllocForTesting() {
#if PA_BUILDFLAG(USE_ALLOCATOR_SHIM)
    allocator_shim::ConfigurePartitionsForTesting();
    allocator_shim::internal::PartitionAllocMalloc::Allocator()->EnableThreadCacheIfSupported();
#endif
}

#if PA_BUILDFLAG(ENABLE_DANGLING_RAW_PTR_CHECKS)

namespace {

struct StackTrace {
    std::array<void*, 64> frames = {};
    int depth = 0;

    void Capture(int skip) {
#if defined(SK_BUILD_FOR_WIN)
        // CaptureStackBackTrace is safe and doesn't allocate.
        // skip + 1 to skip this Capture frame itself.
        depth = CaptureStackBackTrace(skip + 1, frames.size(), frames.data(), nullptr);
#else
        struct State {
            void** frames;
            int max_depth;
            int depth;
            int skip;
            int count;
        };
        State state = {frames.data(), static_cast<int>(frames.size()), 0, skip + 1, 0};

        _Unwind_Backtrace([](struct _Unwind_Context* context, void* arg) -> _Unwind_Reason_Code {
            State* s = static_cast<State*>(arg);
            uintptr_t ip = _Unwind_GetIP(context);
            if (ip == 0) {
                return _URC_END_OF_STACK;
            }
            if (s->count < s->skip) {
                s->count++;
                return _URC_NO_REASON;
            }
            if (s->depth >= s->max_depth) {
                return _URC_END_OF_STACK;
            }
            s->frames[s->depth++] = reinterpret_cast<void*>(ip);
            return _URC_NO_REASON;
        }, &state);

        depth = state.depth;
#endif
    }

    void Print(const char* prefix) const {
        if (depth <= 0) {
            SkDebugf("%s  (no stack trace available)\n", prefix);
            return;
        }
#if defined(SK_BUILD_FOR_WIN)
        HANDLE hProcess = GetCurrentProcess();
        SymInitialize(hProcess, nullptr, TRUE);
        for (int i = 0; i < depth; i++) {
            alignas(SYMBOL_INFO) char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(char)];
            SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = MAX_SYM_NAME;
            DWORD64 displacement = 0;
            if (SymFromAddr(hProcess, reinterpret_cast<DWORD64>(frames[i]), &displacement, symbol)) {
                SkDebugf("%s  #%d %s +0x%llx\n", prefix, i, symbol->Name, displacement);
            } else {
                SkDebugf("%s  #%d %p\n", prefix, i, frames[i]);
            }
        }
#else
        for (int i = 0; i < depth; i++) {
            Dl_info info;
            if (dladdr(frames[i], &info)) {
                const char* libname = info.dli_fname ? info.dli_fname : "unknown";
                const char* basename = strrchr(libname, '/');
                basename = basename ? basename + 1 : libname;

                if (info.dli_sname) {
                    char demangled[256];
                    size_t len = std::size(demangled);
                    int ok;
                    abi::__cxa_demangle(info.dli_sname, demangled, &len, &ok);
                    uintptr_t sym_offset = reinterpret_cast<uintptr_t>(frames[i]) -
                                           reinterpret_cast<uintptr_t>(info.dli_saddr);
                    if (ok == 0) {
                        SkDebugf("%s  #%d %s(%s+0x%zx) [%p]\n", prefix, i, basename, demangled,
                                 sym_offset, frames[i]);
                        continue;
                    }
                    SkDebugf("%s  #%d %s(%s+0x%zx) [%p]\n", prefix, i, basename, info.dli_sname,
                             sym_offset, frames[i]);
                } else {
                    uintptr_t lib_offset = reinterpret_cast<uintptr_t>(frames[i]) -
                                           reinterpret_cast<uintptr_t>(info.dli_fbase);
                    SkDebugf("%s  #%d %s(+0x%zx) [%p]\n", prefix, i, basename, lib_offset, frames[i]);
                }
            } else {
                SkDebugf("%s  #%d %p\n", prefix, i, frames[i]);
            }
        }
#endif
    }
};

struct DanglingTraceEntry {
    uintptr_t ptr = 0;
    StackTrace free_trace;
};

class DanglingTraceRegistry {
public:
    void Record(uintptr_t ptr, int skip) {
        SkAutoMutexExclusive lock(fMutex);
        for (auto& entry : fEntries) {
            if (entry.ptr == 0) {
                entry.ptr = ptr;
                entry.free_trace.Capture(skip + 1);
                return;
            }
        }
    }

    bool Take(uintptr_t ptr, StackTrace& out_trace) {
        SkAutoMutexExclusive lock(fMutex);
        for (auto& entry : fEntries) {
            if (entry.ptr == ptr) {
                out_trace = entry.free_trace;
                entry.ptr = 0;
                return true;
            }
        }
        return false;
    }

private:
    SkMutex fMutex;
    std::array<DanglingTraceEntry, 64> fEntries = {};
};

DanglingTraceRegistry g_registry;

}  // namespace

#endif  // PA_BUILDFLAG(ENABLE_DANGLING_RAW_PTR_CHECKS)

void InitializeDanglingPointerChecksForTesting() {
#if PA_BUILDFLAG(ENABLE_DANGLING_RAW_PTR_CHECKS)
    // Called when a pointer becomes dangling (during free).
    // MUST NOT allocate memory.
    partition_alloc::SetDanglingRawPtrDetectedFn([](uintptr_t ptr) {
        // Skip 1 frame to skip this lambda
        g_registry.Record(ptr, 1);
    });

    // Called when the dangling pointer is released (destroyed or reassigned).
    // Allowed to allocate memory.
    partition_alloc::SetDanglingRawPtrReleasedFn([](uintptr_t ptr) {
        SkDebugf("\n=================================================================\n");
        SkDebugf("DanglingPointerDetector: A pointer was dangling!\n");

        StackTrace free_trace;
        if (g_registry.Take(ptr, free_trace)) {
            SkDebugf("\nMemory was freed at:\n");
            free_trace.Print("  ");
        } else {
            SkDebugf("\nMemory free stack trace was not recorded (registry full or already released).\n");
        }

        SkDebugf("\nDangling raw_ptr was released at:\n");
        StackTrace release_trace;
        release_trace.Capture(1); // Skip 1 frame to skip this lambda
        release_trace.Print("  ");
        SkDebugf("=================================================================\n\n");

        // Temporarily non-fatal to gather more logs.
        // SK_ABORT("DanglingPointerDetector: A pointer was dangling!");
    });
#endif
}

}  // namespace skiatest
