/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Memset_opts_impl_DEFINED
#define Memset_opts_impl_DEFINED

#if !defined(SK_OPTS_NS)
#error "SK_OPTS_NS must be defined, e.g. to 'portable' or 'avx' typically using SkOpts_namespace.h"
#endif

#include "include/private/base/SkFeatures.h"
#include "src/base/SkVx.h"

#include <cstdint>

namespace SK_OPTS_NS {

    template <typename T>
    static void memsetT(T buffer[], T value, int count) {
    #if defined(SK_CPU_SSE_LEVEL) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX
        static const int N = 32 / sizeof(T);
    #else
        static const int N = 16 / sizeof(T);
    #endif
        while (count >= N) {
            skvx::Vec<N,T>(value).store(buffer);
            buffer += N;
            count  -= N;
        }
        while (count --> 0) {
            *buffer++ = value;
        }
    }

    /*not static*/ inline void memset16(uint16_t buffer[], uint16_t value, int count) {
        memsetT(buffer, value, count);
    }
    /*not static*/ inline void memset32(uint32_t buffer[], uint32_t value, int count) {
        memsetT(buffer, value, count);
    }
    /*not static*/ inline void memset64(uint64_t buffer[], uint64_t value, int count) {
        memsetT(buffer, value, count);
    }

    template <typename T>
    static void rect_memsetT(T buffer[], T value, int count, size_t rowBytes, int height) {
        while (height --> 0) {
            memsetT(buffer, value, count);
            buffer = (T*)((char*)buffer + rowBytes);
        }
    }

    /*not static*/ inline void rect_memset16(uint16_t buffer[], uint16_t value, int count,
                                             size_t rowBytes, int height) {
        rect_memsetT(buffer, value, count, rowBytes, height);
    }
    /*not static*/ inline void rect_memset32(uint32_t buffer[], uint32_t value, int count,
                                             size_t rowBytes, int height) {
        rect_memsetT(buffer, value, count, rowBytes, height);
    }
    /*not static*/ inline void rect_memset64(uint64_t buffer[], uint64_t value, int count,
                                             size_t rowBytes, int height) {
        rect_memsetT(buffer, value, count, rowBytes, height);
    }

}  // namespace SK_OPTS_NS

#endif //Memset_opts_DEFINED
