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
        static_assert(VecSize > 0, "T is too big for memsetT");
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
    namespace avx {
        static __always_inline void memset16(uint16_t* buffer, __m256i v, int count) {
            // This helper assumes count >= 16. Smaller cases are handled by GPRs in the caller.
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
            _mm256_storeu_si256((__m256i*)(end - 16), v);
        }

        static __always_inline void memset32(uint32_t* buffer, __m256i v, int count) {
            // This helper assumes count >= 8. Smaller cases are handled by GPRs in the caller.
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
            _mm256_storeu_si256((__m256i*)(end - 8), v);
        }

        static __always_inline void memset64(uint64_t* buffer, __m256i v, int count) {
            // This helper assumes count >= 4. Smaller cases are handled by GPRs in the caller.
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
            _mm256_storeu_si256((__m256i*)(end - 4), v);
        }
    } // namespace avx

    /*not static*/ inline void memset16(uint16_t buffer[], uint16_t value, int count) {
        if (count <= 0) return;
        if (count < 16) {
            // Scalar GPR fast path for small cases to avoid vzeroupper and AVX transitions.
            uint64_t v = value;
            v |= v << 16;
            v |= v << 32;
            if (count >= 8) {
                memcpy(buffer, &v, 8);
                memcpy(buffer + count - 4, &v, 8); // Overlapping 8-byte stores = 16 bytes covered
                return;
            }
            if (count >= 4) {
                memcpy(buffer, &v, 8);
                memcpy(buffer + count - 4, &v, 8);
                return;
            }
            if (count >= 2) {
                uint32_t v32 = (uint32_t)v;
                memcpy(buffer, &v32, 4);
                memcpy(buffer + count - 2, &v32, 4);
                return;
            }
            buffer[0] = value;
            return;
        }
        __m256i v = _mm256_set1_epi16((short)value);
        avx::memset16(buffer, v, count);
        _mm256_zeroupper();
    }

    /*not static*/ inline void memset32(uint32_t buffer[], uint32_t value, int count) {
        if (count <= 0) return;
        if (count < 8) {
            uint64_t v = value;
            v |= v << 32;
            if (count >= 4) {
                memcpy(buffer, &v, 8);
                memcpy(buffer + count - 2, &v, 8);
                if (count > 4) {
                    memcpy(buffer + 2, &v, 8);
                    memcpy(buffer + count - 4, &v, 8);
                }
                return;
            }
            if (count >= 2) {
                memcpy(buffer, &v, 8);
                memcpy(buffer + count - 2, &v, 8);
                return;
            }
            buffer[0] = value;
            return;
        }
        __m256i v = _mm256_set1_epi32((int)value);
        avx::memset32(buffer, v, count);
        _mm256_zeroupper();
    }

    /*not static*/ inline void memset64(uint64_t buffer[], uint64_t value, int count) {
        if (count <= 0) return;
        if (count < 4) {
            if (count >= 2) {
                buffer[0] = value;
                buffer[1] = value;
                buffer[count - 1] = value;
                buffer[count - 2] = value;
                return;
            }
            buffer[0] = value;
            return;
        }
        __m256i v = _mm256_set1_epi64x((long long)value);
        avx::memset64(buffer, v, count);
        _mm256_zeroupper();
    }

    /*not static*/ inline void rect_memset16(uint16_t buffer[], uint16_t value, int count,
                                             size_t rowBytes, int height) {
        if (height <= 0 || count <= 0) return;
        if (count < 16) {
            while (height --> 0) {
                memset16(buffer, value, count);
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
        if (count < 8) {
            while (height --> 0) {
                memset32(buffer, value, count);
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
        if (count < 4) {
            while (height --> 0) {
                memset64(buffer, value, count);
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
