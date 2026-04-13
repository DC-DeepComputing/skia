/*
 * Copyright 2017 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUtils_opts_DEFINED
#define SkUtils_opts_DEFINED

#include <stdint.h>
#include <string.h>
#include "src/base/SkVx.h"

#if defined(SK_CPU_X64_LEVEL) && SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_AVX
#include <immintrin.h>
#endif

namespace SK_OPTS_NS {

    template <typename T>
    static void memsetT(T buffer[], T value, int count) {
    #if defined(SK_CPU_X64_LEVEL) && SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_AVX
        static constexpr int VecSize = 32 / sizeof(T);
    #else
        static constexpr int VecSize = 16 / sizeof(T);
    #endif
        static_assert(VecSize > 0, "T is too big for memsetT");
        skvx::Vec<VecSize,T> wideValue(value);

        while (count >= 4 * VecSize) {
            wideValue.store(buffer);
            wideValue.store(buffer + VecSize);
            wideValue.store(buffer + 2 * VecSize);
            wideValue.store(buffer + 3 * VecSize);
            buffer += 4 * VecSize;
            count  -= 4 * VecSize;
        }

        while (count >= VecSize) {
            wideValue.store(buffer);
            buffer += VecSize;
            count  -= VecSize;
        }
        while (count-- > 0) {
            *buffer++ = value;
        }
    }

    /*not static*/ inline void memset16(uint16_t buffer[], uint16_t value, int count) {
#if defined(SK_CPU_X64_LEVEL) && SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_AVX
        if (count == 0) return;
        uint16_t* dst = buffer;
        uint16_t val16 = value;
        if (count == 1) {
            dst[0] = val16;
            return;
        }
        if (count < 4) {
            uint32_t val32 = ((uint32_t)val16 << 16) | val16;
            memcpy(dst, &val32, 4);
            memcpy(dst + count - 2, &val32, 4);
            return;
        }
        if (count < 8) {
            uint64_t val64;
            uint32_t val32 = ((uint32_t)val16 << 16) | val16;
            val64 = ((uint64_t)val32 << 32) | val32;
            memcpy(dst, &val64, 8);
            memcpy(dst + count - 4, &val64, 8);
            return;
        }

        __m128i val128 = _mm_set1_epi16(val16);
        if (count < 16) {
            _mm_storeu_si128((__m128i*)dst, val128);
            _mm_storeu_si128((__m128i*)(dst + count - 8), val128);
            return;
        }

        __m256i val256 = _mm256_set1_epi16(val16);
        if (count < 32) {
            _mm256_storeu_si256((__m256i*)dst, val256);
            _mm256_storeu_si256((__m256i*)(dst + count - 16), val256);
            return;
        }

        uint16_t* end = dst + count;
        while (dst + 64 <= end) {
            _mm256_storeu_si256((__m256i*)(dst + 0), val256);
            _mm256_storeu_si256((__m256i*)(dst + 16), val256);
            _mm256_storeu_si256((__m256i*)(dst + 32), val256);
            _mm256_storeu_si256((__m256i*)(dst + 48), val256);
            dst += 64;
        }
        if (dst + 32 <= end) {
            _mm256_storeu_si256((__m256i*)(dst + 0), val256);
            _mm256_storeu_si256((__m256i*)(dst + 16), val256);
            dst += 32;
        }
        if (dst + 16 <= end) {
            _mm256_storeu_si256((__m256i*)dst, val256);
            dst += 16;
        }
        if (dst < end) {
            _mm256_storeu_si256((__m256i*)(end - 16), val256);
        }
#else
        memsetT(buffer, value, count);
#endif
    }

    /*not static*/ inline void memset32(uint32_t buffer[], uint32_t value, int count) {
#if defined(SK_CPU_X64_LEVEL) && SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_AVX
        if (count == 0) return;
        uint32_t* dst = buffer;
        uint32_t val32 = value;
        if (count == 1) {
            dst[0] = val32;
            return;
        }
        if (count < 4) {
            uint64_t val64 = ((uint64_t)val32 << 32) | val32;
            memcpy(dst, &val64, 8);
            memcpy(dst + count - 2, &val64, 8);
            return;
        }

        __m128i val128 = _mm_set1_epi32(val32);
        if (count < 8) {
            _mm_storeu_si128((__m128i*)dst, val128);
            _mm_storeu_si128((__m128i*)(dst + count - 4), val128);
            return;
        }

        __m256i val256 = _mm256_set1_epi32(val32);
        if (count < 16) {
            _mm256_storeu_si256((__m256i*)dst, val256);
            _mm256_storeu_si256((__m256i*)(dst + count - 8), val256);
            return;
        }

        uint32_t* end = dst + count;
        while (dst + 32 <= end) {
            _mm256_storeu_si256((__m256i*)(dst + 0), val256);
            _mm256_storeu_si256((__m256i*)(dst + 8), val256);
            _mm256_storeu_si256((__m256i*)(dst + 16), val256);
            _mm256_storeu_si256((__m256i*)(dst + 24), val256);
            dst += 32;
        }

        if (dst + 16 <= end) {
            _mm256_storeu_si256((__m256i*)(dst + 0), val256);
            _mm256_storeu_si256((__m256i*)(dst + 8), val256);
            dst += 16;
        }
        if (dst + 8 <= end) {
            _mm256_storeu_si256((__m256i*)dst, val256);
            dst += 8;
        }
        if (dst < end) {
            _mm256_storeu_si256((__m256i*)(end - 8), val256);
        }
#else
        memsetT(buffer, value, count);
#endif
    }

    /*not static*/ inline void memset64(uint64_t buffer[], uint64_t value, int count) {
#if defined(SK_CPU_X64_LEVEL) && SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_AVX
        if (count == 0) return;
        uint64_t* dst = buffer;
        uint64_t val64 = value;
        if (count == 1) {
            dst[0] = val64;
            return;
        }
        if (count < 4) {
            __m128i val128 = _mm_set1_epi64x(val64);
            _mm_storeu_si128((__m128i*)dst, val128);
            _mm_storeu_si128((__m128i*)(dst + count - 2), val128);
            return;
        }

        __m256i val256 = _mm256_set1_epi64x(val64);
        if (count < 8) {
            _mm256_storeu_si256((__m256i*)dst, val256);
            _mm256_storeu_si256((__m256i*)(dst + count - 4), val256);
            return;
        }

        uint64_t* end = dst + count;
        while (dst + 16 <= end) {
            _mm256_storeu_si256((__m256i*)(dst + 0), val256);
            _mm256_storeu_si256((__m256i*)(dst + 4), val256);
            _mm256_storeu_si256((__m256i*)(dst + 8), val256);
            _mm256_storeu_si256((__m256i*)(dst + 12), val256);
            dst += 16;
        }
        if (dst + 8 <= end) {
            _mm256_storeu_si256((__m256i*)(dst + 0), val256);
            _mm256_storeu_si256((__m256i*)(dst + 4), val256);
            dst += 8;
        }
        if (dst + 4 <= end) {
            _mm256_storeu_si256((__m256i*)dst, val256);
            dst += 4;
        }
        if (dst < end) {
            _mm256_storeu_si256((__m256i*)(end - 4), val256);
        }
#else
        memsetT(buffer, value, count);
#endif
    }

    /*not static*/ inline void rect_memset16(uint16_t buffer[], uint16_t value, int count,
                                             size_t rowBytes, int height) {
        while (height --> 0) {
            memset16(buffer, value, count);
            buffer = (uint16_t*)((char*)buffer + rowBytes);
        }
    }
    /*not static*/ inline void rect_memset32(uint32_t buffer[], uint32_t value, int count,
                                             size_t rowBytes, int height) {
        while (height --> 0) {
            memset32(buffer, value, count);
            buffer = (uint32_t*)((char*)buffer + rowBytes);
        }
    }
    /*not static*/ inline void rect_memset64(uint64_t buffer[], uint64_t value, int count,
                                             size_t rowBytes, int height) {
        while (height --> 0) {
            memset64(buffer, value, count);
            buffer = (uint64_t*)((char*)buffer + rowBytes);
        }
    }
    /*not static*/ inline void rect_memset128(void* buffer, SkColor4f value, int count,
                                              size_t rowBytes, int height) {
        while (height --> 0) {
            memsetT(buffer, value, count);
            buffer = (SkColor4f*)((char*)buffer + rowBytes);
        }
    }

}  // namespace SK_OPTS_NS

#endif//SkUtils_opts_DEFINED
