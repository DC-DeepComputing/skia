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

#if defined(SK_CPU_X64_LEVEL) && SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_AVX
#include <immintrin.h>
#else
#include "src/base/SkVx.h"
#endif

namespace SK_OPTS_NS {

#if !(defined(SK_CPU_X64_LEVEL) && SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_AVX)
    template <typename T>
    static void memsetT(T buffer[], T value, int count) {
        static constexpr int VecSize = 16 / sizeof(T);
        skvx::Vec<VecSize,T> wideValue(value);
        while (count >= VecSize) {
            wideValue.store(buffer);
            buffer += VecSize;
            count  -= VecSize;
        }
        while (count-- > 0) {
            *buffer++ = value;
        }
    }
#endif

#if defined(SK_CPU_X64_LEVEL) && SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_AVX
    namespace sse {
        static __always_inline void memset16(uint16_t* buffer, __m128i v, int count) {
            uint16_t* end = buffer + count;
            while (count >= 32) {
                _mm_storeu_si128((__m128i*)buffer, v);
                _mm_storeu_si128((__m128i*)(buffer + 8), v);
                buffer += 16;
                count -= 16;
            }
            if (count >= 16) {
                _mm_storeu_si128((__m128i*)buffer, v);
                _mm_storeu_si128((__m128i*)(buffer + 8), v);
            } else if (count >= 8) {
                _mm_storeu_si128((__m128i*)buffer, v);
            }
            if (count & 7) {
                _mm_storeu_si128((__m128i*)(end - 8), v);
            }
        }
        static __always_inline void memset32(uint32_t* buffer, __m128i v, int count) {
            uint32_t* end = buffer + count;
            while (count >= 16) {
                _mm_storeu_si128((__m128i*)buffer, v);
                _mm_storeu_si128((__m128i*)(buffer + 4), v);
                buffer += 8;
                count -= 8;
            }
            if (count >= 8) {
                _mm_storeu_si128((__m128i*)buffer, v);
                _mm_storeu_si128((__m128i*)(buffer + 4), v);
            } else if (count >= 4) {
                _mm_storeu_si128((__m128i*)buffer, v);
            }
            if (count & 3) {
                _mm_storeu_si128((__m128i*)(end - 4), v);
            }
        }
        static __always_inline void memset64(uint64_t* buffer, __m128i v, int count) {
            uint64_t* end = buffer + count;
            while (count >= 8) {
                _mm_storeu_si128((__m128i*)buffer, v);
                _mm_storeu_si128((__m128i*)(buffer + 2), v);
                buffer += 4;
                count -= 4;
            }
            if (count >= 4) {
                _mm_storeu_si128((__m128i*)buffer, v);
                _mm_storeu_si128((__m128i*)(buffer + 2), v);
            } else if (count >= 2) {
                _mm_storeu_si128((__m128i*)buffer, v);
            }
            if (count & 1) {
                _mm_storeu_si128((__m128i*)(end - 2), v);
            }
        }
    }

    namespace avx {
        static __always_inline void memset16(uint16_t* buffer, __m256i v, int count) {
            uint16_t* end = buffer + count;
            while (count >= 64) {
                _mm256_storeu_si256((__m256i*)(buffer +  0), v);
                _mm256_storeu_si256((__m256i*)(buffer + 16), v);
                _mm256_storeu_si256((__m256i*)(buffer + 32), v);
                _mm256_storeu_si256((__m256i*)(buffer + 48), v);
                buffer += 64;
                count -= 64;
            }
            if (count >= 32) {
                _mm256_storeu_si256((__m256i*)buffer, v);
                _mm256_storeu_si256((__m256i*)(buffer + 16), v);
                buffer += 32;
                count -= 32;
            }
            if (count >= 16) {
                _mm256_storeu_si256((__m256i*)buffer, v);
            }
            if (count & 15) {
                _mm256_storeu_si256((__m256i*)(end - 16), v);
            }
        }

        static __always_inline void memset32(uint32_t* buffer, __m256i v, int count) {
            uint32_t* end = buffer + count;
            while (count >= 32) {
                _mm256_storeu_si256((__m256i*)(buffer +  0), v);
                _mm256_storeu_si256((__m256i*)(buffer +  8), v);
                _mm256_storeu_si256((__m256i*)(buffer + 16), v);
                _mm256_storeu_si256((__m256i*)(buffer + 24), v);
                buffer += 32;
                count -= 32;
            }
            if (count >= 16) {
                _mm256_storeu_si256((__m256i*)buffer, v);
                _mm256_storeu_si256((__m256i*)(buffer + 8), v);
                buffer += 16;
                count -= 16;
            }
            if (count >= 8) {
                _mm256_storeu_si256((__m256i*)buffer, v);
            }
            if (count & 7) {
                _mm256_storeu_si256((__m256i*)(end - 8), v);
            }
        }

        static __always_inline void memset64(uint64_t* buffer, __m256i v, int count) {
            uint64_t* end = buffer + count;
            while (count >= 16) {
                _mm256_storeu_si256((__m256i*)(buffer +  0), v);
                _mm256_storeu_si256((__m256i*)(buffer +  4), v);
                _mm256_storeu_si256((__m256i*)(buffer +  8), v);
                _mm256_storeu_si256((__m256i*)(buffer + 12), v);
                buffer += 16;
                count -= 16;
            }
            if (count >= 8) {
                _mm256_storeu_si256((__m256i*)buffer, v);
                _mm256_storeu_si256((__m256i*)(buffer + 4), v);
                buffer += 8;
                count -= 8;
            }
            if (count >= 4) {
                _mm256_storeu_si256((__m256i*)buffer, v);
            }
            if (count & 3) {
                _mm256_storeu_si256((__m256i*)(end - 4), v);
            }
        }
    } // namespace avx

    /*not static*/ inline void memset16(uint16_t buffer[], uint16_t value, int count) {
        if (count <= 0) return;
        if (count <= 8) { // <= 16 bytes: branchless GPR overlap
            uint64_t v = value;
            v |= v << 16;
            v |= v << 32;
            if (count >= 4) {
                memcpy(buffer, &v, 8);
                memcpy(buffer + count - 4, &v, 8);
            } else {
                buffer[0] = value;
                buffer[count - 1] = value;
                if (count > 2) buffer[1] = value;
            }
            return;
        }
        if (count < 64) { // 18 - 128 bytes: SSE
            sse::memset16(buffer, _mm_set1_epi16((short)value), count);
            return;
        }
        avx::memset16(buffer, _mm256_set1_epi16((short)value), count);
        _mm256_zeroupper();
    }

    /*not static*/ inline void memset32(uint32_t buffer[], uint32_t value, int count) {
        if (count <= 0) return;
        if (count <= 4) { // <= 16 bytes: branchless GPR overlap
            uint64_t v = value;
            v |= v << 32;
            if (count >= 2) {
                memcpy(buffer, &v, 8);
                memcpy(buffer + count - 2, &v, 8);
            } else {
                buffer[0] = value;
            }
            return;
        }
        if (count < 32) { // 20 - 128 bytes: SSE
            sse::memset32(buffer, _mm_set1_epi32((int)value), count);
            return;
        }
        avx::memset32(buffer, _mm256_set1_epi32((int)value), count);
        _mm256_zeroupper();
    }

    /*not static*/ inline void memset64(uint64_t buffer[], uint64_t value, int count) {
        if (count <= 0) return;
        if (count <= 2) { // <= 16 bytes: branchless GPR overlap
            buffer[0] = value;
            buffer[count - 1] = value;
            return;
        }
        if (count < 16) { // 24 - 128 bytes: SSE
            sse::memset64(buffer, _mm_set1_epi64x((long long)value), count);
            return;
        }
        avx::memset64(buffer, _mm256_set1_epi64x((long long)value), count);
        _mm256_zeroupper();
    }

    /*not static*/ inline void rect_memset16(uint16_t buffer[], uint16_t value, int count,
                                             size_t rowBytes, int height) {
        if (height <= 0 || count <= 0) return;
        if (count < 64) {
            __m128i v = _mm_set1_epi16((short)value);
            while (height --> 0) {
                sse::memset16(buffer, v, count);
                buffer = (uint16_t*)((char*)buffer + rowBytes);
            }
            return;
        }
        __m256i v = _mm256_set1_epi16((short)value);
        while (height --> 0) {
            avx::memset16(buffer, v, count);
            buffer = (uint16_t*)((char*)buffer + rowBytes);
        }
        _mm256_zeroupper();
    }
    /*not static*/ inline void rect_memset32(uint32_t buffer[], uint32_t value, int count,
                                             size_t rowBytes, int height) {
        if (height <= 0 || count <= 0) return;
        if (count < 32) {
            __m128i v = _mm_set1_epi32((int)value);
            while (height --> 0) {
                sse::memset32(buffer, v, count);
                buffer = (uint32_t*)((char*)buffer + rowBytes);
            }
            return;
        }
        __m256i v = _mm256_set1_epi32((int)value);
        while (height --> 0) {
            avx::memset32(buffer, v, count);
            buffer = (uint32_t*)((char*)buffer + rowBytes);
        }
        _mm256_zeroupper();
    }
    /*not static*/ inline void rect_memset64(uint64_t buffer[], uint64_t value, int count,
                                             size_t rowBytes, int height) {
        if (height <= 0 || count <= 0) return;
        if (count < 16) {
            __m128i v = _mm_set1_epi64x((long long)value);
            while (height --> 0) {
                sse::memset64(buffer, v, count);
                buffer = (uint64_t*)((char*)buffer + rowBytes);
            }
            return;
        }
        __m256i v = _mm256_set1_epi64x((long long)value);
        while (height --> 0) {
            avx::memset64(buffer, v, count);
            buffer = (uint64_t*)((char*)buffer + rowBytes);
        }
        _mm256_zeroupper();
    }
#else
    /*not static*/ inline void memset16(uint16_t buffer[], uint16_t value, int count) {
        memsetT(buffer, value, count);
    }
    /*not static*/ inline void memset32(uint32_t buffer[], uint32_t value, int count) {
        memsetT(buffer, value, count);
    }
    /*not static*/ inline void memset64(uint64_t buffer[], uint64_t value, int count) {
        memsetT(buffer, value, count);
    }
    /*not static*/ inline void rect_memset16(uint16_t buffer[], uint16_t value, int count,
                                             size_t rowBytes, int height) {
        while (height --> 0) {
            memsetT(buffer, value, count);
            buffer = (uint16_t*)((char*)buffer + rowBytes);
        }
    }
    /*not static*/ inline void rect_memset32(uint32_t buffer[], uint32_t value, int count,
                                             size_t rowBytes, int height) {
        while (height --> 0) {
            memsetT(buffer, value, count);
            buffer = (uint32_t*)((char*)buffer + rowBytes);
        }
    }
    /*not static*/ inline void rect_memset64(uint64_t buffer[], uint64_t value, int count,
                                             size_t rowBytes, int height) {
        while (height --> 0) {
            memsetT(buffer, value, count);
            buffer = (uint64_t*)((char*)buffer + rowBytes);
        }
    }
#endif

}  // namespace SK_OPTS_NS

#endif//SkUtils_opts_DEFINED
