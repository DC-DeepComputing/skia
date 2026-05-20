/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitRow_opts_DEFINED
#define SkBlitRow_opts_DEFINED

#include "src/base/SkMSAN.h"
#include "src/base/SkVx.h"
#include "src/core/SkColorData.h"

// Helpers for blit_row_s32a_opaque(),
// then blit_row_s32a_opaque() itself,
// then unrelated blit_row_color32() at the bottom.
//
// To keep Skia resistant to timing attacks, it's important not to branch on pixel data.
// In particular, don't be tempted to [v]ptest, pmovmskb, etc. to branch on the source alpha.

#if SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_AVX2
    #include <immintrin.h>

    static inline __m256i SkPMSrcOver_AVX2(const __m256i& src, const __m256i& dst) {
        // Abstractly srcover is
        //     b = s + d*(1-srcA)
        //
        // In terms of unorm8 bytes, that works out to
        //     b = s + (d*(255-srcA) + 127) / 255
        //
        // But we approximate that to within a bit with
        //     b = s + (d*(255-srcA) + d) / 256
        // a.k.a
        //     b = s + (d*(256-srcA)) >> 8

        // The bottleneck of this math is the multiply, and we want to do it as
        // narrowly as possible, here getting inputs into 16-bit lanes and
        // using 16-bit multiplies.  We can do twice as many multiplies at once
        // as using naive 32-bit multiplies, and on top of that, the 16-bit multiplies
        // are themselves a couple cycles quicker.  Win-win.

        // We'll get everything in 16-bit lanes for two multiplies, one
        // handling dst red and blue, the other green and alpha.  (They're
        // conveniently 16-bits apart, you see.) We don't need the individual
        // src channels beyond alpha until the very end when we do the "s + "
        // add, and we don't even need to unpack them; the adds cannot overflow.

        // Shuffle each pixel's srcA to the low byte of each 16-bit half of the pixel.
        const int _ = -1;   // fills a literal 0 byte.
        __m256i srcA_x2 = _mm256_shuffle_epi8(src,
                _mm256_setr_epi8(3,_,3,_, 7,_,7,_, 11,_,11,_, 15,_,15,_,
                                 3,_,3,_, 7,_,7,_, 11,_,11,_, 15,_,15,_));
        __m256i scale_x2 = _mm256_sub_epi16(_mm256_set1_epi16(256),
                                            srcA_x2);

        // Scale red and blue, leaving results in the low byte of each 16-bit lane.
        __m256i rb = _mm256_and_si256(_mm256_set1_epi32(0x00ff00ff), dst);
        rb = _mm256_mullo_epi16(rb, scale_x2);
        rb = _mm256_srli_epi16 (rb, 8);

        // Scale green and alpha, leaving results in the high byte, masking off the low bits.
        __m256i ga = _mm256_srli_epi16(dst, 8);
        ga = _mm256_mullo_epi16(ga, scale_x2);
        ga = _mm256_andnot_si256(_mm256_set1_epi32(0x00ff00ff), ga);

        return _mm256_adds_epu8(src, _mm256_or_si256(rb, ga));
    }
#endif

#if SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_SSE2
    #include <immintrin.h>

    static inline __m128i SkPMSrcOver_SSE2(const __m128i& src, const __m128i& dst) {
        __m128i scale = _mm_sub_epi32(_mm_set1_epi32(256),
                                      _mm_srli_epi32(src, 24));
        __m128i scale_x2 = _mm_or_si128(_mm_slli_epi32(scale, 16), scale);

        __m128i rb = _mm_and_si128(_mm_set1_epi32(0x00ff00ff), dst);
        rb = _mm_mullo_epi16(rb, scale_x2);
        rb = _mm_srli_epi16(rb, 8);

        __m128i ga = _mm_srli_epi16(dst, 8);
        ga = _mm_mullo_epi16(ga, scale_x2);
        ga = _mm_andnot_si128(_mm_set1_epi32(0x00ff00ff), ga);

        return _mm_adds_epu8(src, _mm_or_si128(rb, ga));
    }
#endif

#if defined(SK_ARM_HAS_NEON)
#ifdef __ARM_64BIT_STATE
// No attempt has been made to adapt the inline assembly version for AArch64
// so fall back to the less performant version that uses intrinsics instead

    #include <arm_neon.h>

    // SkMulDiv255Round() applied to each lane.
    static inline uint8x8_t SkMulDiv255Round_neon8(uint8x8_t x, uint8x8_t y) {
        uint16x8_t prod = vmull_u8(x, y);
        return vraddhn_u16(prod, vrshrq_n_u16(prod, 8));
    }

    static inline uint8x8x4_t SkPMSrcOver_neon8(uint8x8x4_t dst, uint8x8x4_t src) {
        uint8x8_t nalphas = vmvn_u8(src.val[3]);  // 256 - alpha
        return {
            vqadd_u8(src.val[0], SkMulDiv255Round_neon8(nalphas,  dst.val[0])),
            vqadd_u8(src.val[1], SkMulDiv255Round_neon8(nalphas,  dst.val[1])),
            vqadd_u8(src.val[2], SkMulDiv255Round_neon8(nalphas,  dst.val[2])),
            vqadd_u8(src.val[3], SkMulDiv255Round_neon8(nalphas,  dst.val[3])),
        };
    }

    // Variant assuming dst and src contain the color components of two consecutive pixels.
    static inline uint8x8_t SkPMSrcOver_neon2(uint8x8_t dst, uint8x8_t src) {
        const uint8x8_t alpha_indices = vcreate_u8(0x0707070703030303);
        uint8x8_t nalphas = vmvn_u8(vtbl1_u8(src, alpha_indices));
        return vqadd_u8(src, SkMulDiv255Round_neon8(nalphas, dst));
    }

#else // __ARM_64BIT_STATE
// Inline ARM AArch32 assembly version

// Macros to specify instructions to only include if targeting ARM or Thumb instruction sets
#ifdef __thumb__
#define A(x)
#define T(x) x
#else
#define A(x) x
#define T(x)
#endif

#if SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
    #include <lasxintrin.h>

    static inline __m256i SkPMSrcOver_LASX(const __m256i& src, const __m256i& dst) {
        __m256i val = __lasx_xvreplgr2vr_w(256);
        __m256i scale = __lasx_xvsub_w(val, __lasx_xvsrli_w(src, 24));
        __m256i scale_x2 = __lasx_xvor_v(__lasx_xvslli_w(scale, 16), scale);

        val = __lasx_xvreplgr2vr_w(0x00ff00ff);
        __m256i rb = __lasx_xvand_v(val, dst);
        rb = __lasx_xvmul_h(rb, scale_x2);
        rb = __lasx_xvsrli_h(rb, 8);

        __m256i ga = __lasx_xvsrli_h(dst, 8);
        ga = __lasx_xvmul_h(ga, scale_x2);
        ga = __lasx_xvandn_v(val, ga);

        return __lasx_xvsadd_bu(src, __lasx_xvor_v(rb, ga));
    }
#endif

#if SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
    #include <lsxintrin.h>

    static inline __m128i SkPMSrcOver_LSX(const __m128i& src, const __m128i& dst) {
        __m128i val = __lsx_vreplgr2vr_w(256);
        __m128i scale = __lsx_vsub_w(val, __lsx_vsrli_w(src, 24));
        __m128i scale_x2 = __lsx_vor_v(__lsx_vslli_w(scale, 16), scale);

        val = __lsx_vreplgr2vr_w(0x00ff00ff);
        __m128i rb = __lsx_vand_v(val, dst);
        rb = __lsx_vmul_h(rb, scale_x2);
        rb = __lsx_vsrli_h(rb, 8);

        __m128i ga = __lsx_vsrli_h(dst, 8);
        ga = __lsx_vmul_h(ga, scale_x2);
        ga = __lsx_vandn_v(val, ga);

        return __lsx_vsadd_bu(src, __lsx_vor_v(rb, ga));
    }
#endif

// These macros permit optionally-included features to be switched using a parameter to another macro
#define YES(x) x
#define NO(x)

// How far ahead (pixels) to preload (undefine to disable prefetch) - determined empirically
#undef PREFETCH_DISTANCE
#define PREFETCH_DISTANCE "24"

#ifdef PREFETCH_DISTANCE
#define IF_PRELOAD YES
#else
#define IF_PRELOAD NO
#endif

/// Macro to load or store 1..7 pixels in growing powers-of-2 in size - suitable for leading pixels
#define S32A_LOADSTORE_LEADING_7(ls, r0, r1, r2, r3, base, opt)                                       \
                "tst         %[group_size], #1                                                \n\t"   \
                opt"                                                                          \n\t"   \
                "beq         1f                                                               \n\t"   \
                "v"#ls"4.8   {"#r0"[1],"#r1"[1],"#r2"[1],"#r3"[1]}, [%["#base"]:32]!          \n\t"   \
                "1:                                                                           \n\t"   \
                "lsls        %[tmp], %[group_size], #30                                       \n\t"   \
                "add         %[tmp], %["#base"], #4                                           \n\t"   \
                "bpl         1f                                                               \n\t"   \
                "v"#ls"4.8   {"#r0"[2],"#r1"[2],"#r2"[2],"#r3"[2]}, [%["#base"]:32], %[eight] \n\t"   \
                "v"#ls"4.8   {"#r0"[3],"#r1"[3],"#r2"[3],"#r3"[3]}, [%[tmp]:32], %[eight]     \n\t"   \
                "1:                                                                           \n\t"   \
                "bcc         1f                                                               \n\t"   \
                "v"#ls"4.8   {"#r0"[4],"#r1"[4],"#r2"[4],"#r3"[4]}, [%["#base"]:32], %[eight] \n\t"   \
                "v"#ls"4.8   {"#r0"[5],"#r1"[5],"#r2"[5],"#r3"[5]}, [%[tmp]:32], %[eight]     \n\t"   \
                "v"#ls"4.8   {"#r0"[6],"#r1"[6],"#r2"[6],"#r3"[6]}, [%["#base"]:32], %[eight] \n\t"   \
                "v"#ls"4.8   {"#r0"[7],"#r1"[7],"#r2"[7],"#r3"[7]}, [%[tmp]:32], %[eight]     \n\t"   \
                "1:                                                                           \n\t"   \

/// Macro to load or store 1..7 pixels in shrink powers-of-2 in size - suitable for trailing pixels
#define S32A_LOADSTORE_TRAILING_7(ls, r0, r1, r2, r3, base, opt)                                      \
                "lsls        %[tmp], %[group_size], #30                                       \n\t"   \
                "add         %[tmp], %["#base"], #4                                           \n\t"   \
                opt"                                                                          \n\t"   \
                "bcc         1f                                                               \n\t"   \
                "v"#ls"4.8   {"#r0"[0],"#r1"[0],"#r2"[0],"#r3"[0]}, [%["#base"]:32], %[eight] \n\t"   \
                "v"#ls"4.8   {"#r0"[1],"#r1"[1],"#r2"[1],"#r3"[1]}, [%[tmp]:32], %[eight]     \n\t"   \
                "v"#ls"4.8   {"#r0"[2],"#r1"[2],"#r2"[2],"#r3"[2]}, [%["#base"]:32], %[eight] \n\t"   \
                "v"#ls"4.8   {"#r0"[3],"#r1"[3],"#r2"[3],"#r3"[3]}, [%[tmp]:32], %[eight]     \n\t"   \
                "1:                                                                           \n\t"   \
                "bpl         1f                                                               \n\t"   \
                "v"#ls"4.8   {"#r0"[4],"#r1"[4],"#r2"[4],"#r3"[4]}, [%["#base"]:32], %[eight] \n\t"   \
                "v"#ls"4.8   {"#r0"[5],"#r1"[5],"#r2"[5],"#r3"[5]}, [%[tmp]:32], %[eight]     \n\t"   \
                "1:                                                                           \n\t"   \
                "tst         %[group_size], #1                                                \n\t"   \
                "beq         1f                                                               \n\t"   \
                "v"#ls"4.8   {"#r0"[6],"#r1"[6],"#r2"[6],"#r3"[6]}, [%["#base"]:32]!          \n\t"   \
                "1:                                                                           \n\t"   \

/// Macro to do testing and "over" compositing of a group of 1..7 32bpp premultiplied ARGB leading or trailing pixels
#define S32A_OPAQUE_7PIX_PROCESS(loadstore_7, size)                                                   \
    do {                                                                                              \
        __asm__ volatile (                                                                            \
                /* Load the leading/trailing source pixels,                                           \
                 * after initialising all the unused indexes from the first pixel                     \
                 * so the all-opaque and all-transparent tests still work */                          \
                loadstore_7(ld, d0, d1, d2, d3, src,                                                  \
                "vld4.8      {d0[],d1[],d2[],d3[]}, [%[src]]")                                        \
                /* Test for all-opaque or all-transparent */                                          \
                "vmov        %[alo], s6                                                       \n\t"   \
                "vmov        %[ahi], s7                                                       \n\t"   \
                "vmvn        d31, d3                                                          \n\t"   \
                "orrs        %[tmp], %[alo], %[ahi]                                           \n\t"   \
                "it          eq                                                               \n\t"   \
                "addeq       %[dst], %[group_size], lsl #2                                    \n\t"   \
                "beq         9f                                                               \n\t"   \
                "cmp         %[alo], #-1                                                      \n\t"   \
                "it          eq                                                               \n\t"   \
                "cmpeq       %[ahi], #-1                                                      \n\t"   \
                "beq         5f                                                               \n\t"   \
                /* Translucency used, or a mixture of opaque and transparent */                       \
                loadstore_7(ld, d20, d21, d22, d23, dst, )                                            \
                "sub         %[dst], %[group_size], lsl #2                                    \n\t"   \
                S32A_OPAQUE_8PIX_BLEND(, , q0, q1,, NO, NO, NO)                                       \
                "5:                                                                           \n\t"   \
                loadstore_7(st, d0, d1, d2, d3, dst, )                                                \
                /* Drop through */                                                                    \
                "9:                                                                           \n\t"   \
        : /* Outputs */                                                                               \
                [alo]"=&r"(alo),                                                                      \
                [ahi]"=&r"(ahi),                                                                      \
                [tmp]"=&r"(tmp),                                                                      \
                [src]"+r"(src),                                                                       \
                [dst]"+r"(dst)                                                                        \
        : /* Inputs */                                                                                \
                [group_size]"r"(size),                                                                \
                     [eight]"r"(eight)                                                                \
        : /* Clobbers */                                                                              \
                "cc", "memory", "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15" \
        );                                                                                            \
    } while (0)

/// Macro to do testing and "over" compositing of an aligned group of 8 32bpp premultiplied ARGB leading or trailing pixels
#define S32A_OPAQUE_8PIX_PROCESS(align, if_load)                                                      \
    do {                                                                                              \
        __asm__ (                                                                                     \
if_load(        "vld4.8      {d0-d3}, [%[src]]!                                               \n\t")  \
                /* Test for all-opaque or all-transparent */                                          \
                "vmov        %[alo], s6                                                       \n\t"   \
                "vmov        %[ahi], s7                                                       \n\t"   \
                "vmvn        d31, d3                                                          \n\t"   \
                "orrs        %[tmp], %[alo], %[ahi]                                           \n\t"   \
                "it          eq                                                               \n\t"   \
                "addeq       %[dst], #8*4                                                     \n\t"   \
                "beq         9f                                                               \n\t"   \
                "cmp         %[alo], #-1                                                      \n\t"   \
                "it          eq                                                               \n\t"   \
                "cmpeq       %[ahi], #-1                                                      \n\t"   \
                "beq         5f                                                               \n\t"   \
                /* Translucency used, or a mixture of opaque and transparent */                       \
                S32A_OPAQUE_8PIX_BLEND(align, , q0, q1, "5:", YES, NO, NO)                            \
                /* Drop through */                                                                    \
                "9:                                                                           \n\t"   \
        :  /* Outputs */                                                                              \
                [alo]"=&r"(alo),                                                                      \
                [ahi]"=&r"(ahi),                                                                      \
                [tmp]"=&r"(tmp),                                                                      \
                [src]"+r"(src),                                                                       \
                [dst]"+r"(dst)                                                                        \
        : /* Inputs */                                                                                \
        : /* Clobbers */                                                                              \
                "cc", "memory", "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15" \
        );                                                                                            \
    } while (0)

/// Macro to do "over" compositing blending on 8 32bpp premultiplied ARGB pixels
/// which are with either translucent or a mixture of opaque and transparent.
/// Relies on A(x) to determine whether to emit code in ARM state (as opposed to Thumb state).
/// @arg align           bit-alignment specifier on destination loads/stores (optional)
/// @arg other_src_alpha D-register specifier for alpha source in other bank (only IF_OVERLAP)
/// @arg src0            Q-register specifier for blue/green source in this bank
/// @arg src1            Q-register specifier for red/alpha source in this bank
/// @arg opt             optional instruction to emit
/// @arg if_loadstore    YES or NO: whether to do load/store
/// @arg if_overlap      YES or NO: whether to interleave processing of next iteration
/// @arg if_preload      YES or NO: whether to insert prefetch instructions
#define S32A_OPAQUE_8PIX_BLEND(align, other_src_alpha, src0, src1, opt, if_loadstore, if_overlap, if_preload) \
if_loadstore(   "vld4.8      {d20-d23}, [%[dst]"#align"]                                      \n\t")  \
if_preload(     "sub         %[tmp], %[len], #1                                               \n\t")  \
if_overlap(     "vmov        %[alo], %[ahi], "#other_src_alpha"                               \n\t")  \
if_preload(     "cmp         %[tmp], #" PREFETCH_DISTANCE "                                   \n\t")  \
                "vmull.u8    q8, d20, d31                                                     \n\t"   \
if_preload(     "it          cs                                                               \n\t")  \
if_preload(     "movcs       %[tmp], #" PREFETCH_DISTANCE "                                   \n\t")  \
                "vmull.u8    q9, d21, d31                                                     \n\t"   \
                "vmull.u8    q10, d22, d31                                                    \n\t"   \
                "vmull.u8    q11, d23, d31                                                    \n\t"   \
if_preload(     "pld         [%[src], %[tmp], lsl #2]                                         \n\t")  \
                "vrshr.u16   q12, q8, #8                                                      \n\t"   \
if_preload(     "add         %[tmp], #(32+32)/4                                               \n\t")  \
                "vrshr.u16   q13, q9, #8                                                      \n\t"   \
                "vrshr.u16   q14, q10, #8                                                     \n\t"   \
                "vrshr.u16   q15, q11, #8                                                     \n\t"   \
if_preload(     "pld         [%[dst], %[tmp], lsl #2]                                         \n\t")  \
                "vraddhn.u16 d16, q8, q12                                                     \n\t"   \
                "vraddhn.u16 d17, q9, q13                                                     \n\t"   \
                "vraddhn.u16 d18, q10, q14                                                    \n\t"   \
                "vraddhn.u16 d19, q11, q15                                                    \n\t"   \
if_overlap(     "mvn         %[tmp], %[alo]                                                   \n\t")  \
if_overlap(     "vmvn        d31, "#other_src_alpha"                                          \n\t")  \
if_overlap(A(   "orr         %[alo], %[ahi]                                                   \n\t")) \
                "vadd.u8     "#src0", q8                                                      \n\t"   \
if_overlap(A(   "mvn         %[ahi], %[ahi]                                                   \n\t")) \
                "vadd.u8     "#src1", q9                                                      \n\t"   \
                opt"                                                                          \n\t"   \
if_loadstore(   "vst4.8      {"#src0", "#src1"}, [%[dst]"#align"]!                            \n\t")  \

#endif // __ARM_64BIT_STATE
#endif // defined(SK_ARM_HAS_NEON)

namespace SK_OPTS_NS {

/*not static*/
inline void blit_row_s32a_opaque(SkPMColor* dst, const SkPMColor* src, int len, U8CPU alpha) {
    SkASSERT(alpha == 0xFF);
    sk_msan_assert_initialized(src, src+len);

#if SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_AVX2
    while (len >= 8) {
        _mm256_storeu_si256((__m256i*)dst,
                            SkPMSrcOver_AVX2(_mm256_loadu_si256((const __m256i*)src),
                                             _mm256_loadu_si256((const __m256i*)dst)));
        src += 8;
        dst += 8;
        len -= 8;
    }
#endif

#if SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_SSE2
    while (len >= 4) {
        _mm_storeu_si128((__m128i*)dst, SkPMSrcOver_SSE2(_mm_loadu_si128((const __m128i*)src),
                                                         _mm_loadu_si128((const __m128i*)dst)));
        src += 4;
        dst += 4;
        len -= 4;
    }
#endif

#if defined(SK_ARM_HAS_NEON)
#ifdef __ARM_64BIT_STATE
    // No attempt has been made to adapt the inline assembly version for AArch64
    // so fall back to the less performant version that uses intrinsics instead

    while (len >= 8) {
        vst4_u8((uint8_t*)dst, SkPMSrcOver_neon8(vld4_u8((const uint8_t*)dst),
                                                 vld4_u8((const uint8_t*)src)));
        src += 8;
        dst += 8;
        len -= 8;
    }

    while (len >= 2) {
        vst1_u8((uint8_t*)dst, SkPMSrcOver_neon2(vld1_u8((const uint8_t*)dst),
                                                 vld1_u8((const uint8_t*)src)));
        src += 2;
        dst += 2;
        len -= 2;
    }

    if (len != 0) {
        uint8x8_t result = SkPMSrcOver_neon2(vcreate_u8((uint64_t)*dst),
                                             vcreate_u8((uint64_t)*src));
        vst1_lane_u32(dst, vreinterpret_u32_u8(result), 0);
    }
    return;

#else // __ARM_64BIT_STATE
    // Inline ARM AArch32 assembly version
    uint32_t tmp, alo, ahi;
    const int eight = 8;
    if (len < 15) {
        // Too short to attempt aligned processing
        if (len & 8)
            S32A_OPAQUE_8PIX_PROCESS(, YES);
        if (len & 7)
            S32A_OPAQUE_7PIX_PROCESS(S32A_LOADSTORE_TRAILING_7, len & 7);
    } else {
        // The last 8 - 15 pixels (starting from a 4-pixel boundary) are handled together
        uintptr_t startrup = (uintptr_t) dst / sizeof (*dst) + 3;
        uintptr_t end = (uintptr_t) dst / sizeof (*dst) + len;
        size_t trailing;
        if ((end & 3) == 0)
            // No blocks of <8 pixels used at end in these cases
            trailing = 8;
        else
            // If length (discounting alignment to 4-pixel boundaries) is an odd number of 4-pixels,
            // assign this to trailing end to avoid possibility of a leading run of exactly 4
            trailing = 8 + ((startrup ^ end) & 4) + (end & 3);
        // The inner loop handles an integer number (0+) of 16-pixel blocks at 4-pixel boundaries
        // The 0..15 pixels leading up to this are handled together
        size_t leading8 = (len - trailing) & 8;
        size_t leading7 = (len - trailing) & 7;

        // Do leading pixels
        if (leading7 != 0) {
            len -= leading7;
            S32A_OPAQUE_7PIX_PROCESS(S32A_LOADSTORE_LEADING_7, leading7);
        }
        if (leading8 != 0) {
            len -= 8;
            S32A_OPAQUE_8PIX_PROCESS(:128, YES);
        }

        // Do inner loop
        __asm__ (
                // We enter and leave each iteration of the inner loop with the source
                // pointer 8 pixels ahead and the destination pointer 8 pixels behind
                // in order to permit good pipelining. The count of remaining pixels is
                // reduced by 16 to allow the loop termination test to be combined with
                // the decrementing of the remaining length.
                "sub         %[dst], #8*4                                                     \n\t"
                "vld4.8      {d0-d3}, [%[src]]!                                               \n\t"
                "subs        %[len], #16                                                      \n\t"
                "bcc         49f                                                              \n\t"

                "10:                                                                          \n\t"
                // Move alpha to ARM registers for comparison
                "vmov        %[alo], s6                                                       \n\t"
                "vmov        %[ahi], s7                                                       \n\t"
                // Fetch source data for next iteration
                "vld4.8      {d4-d7}, [%[src]]!                                               \n\t"
                "add         %[dst], #8*4                                                     \n\t"
                // Test if all source pixels are transparent (alpha=0)
                "orrs        %[tmp], %[alo], %[ahi]                                           \n\t"
                "beq         19f                                                              \n\t"
                // Find inverse alpha in case full blending required
                "vmvn        d31, d3                                                          \n\t"
                // Test if all source pixels are opaque (alpha=0xff)
                "cmp         %[alo], #-1                                                      \n\t"
                "it          eq                                                               \n\t"
                "cmpeq       %[ahi], #-1                                                      \n\t"
                "bne         30f                                                              \n\t"
                // Opaque case: copy source to destination
                "vst4.8      {d0-d3}, [%[dst]:128]                                            \n\t"
                // Drop through
                "19:                                                                          \n\t"

                // Move alpha to ARM registers for comparison
                "vmov        %[alo], s14                                                      \n\t"
                "vmov        %[ahi], s15                                                      \n\t"
                // Fetch source data for next iteration
                "vld4.8      {d0-d3}, [%[src]]!                                               \n\t"
                "add         %[dst], #8*4                                                     \n\t"
                // Test if all source pixels are transparent (alpha=0)
                "orrs        %[tmp], %[alo], %[ahi]                                           \n\t"
                "beq         29f                                                              \n\t"
                // Find inverse alpha in case full blending required
                "vmvn        d31, d7                                                          \n\t"
                // Test if all source pixels are opaque (alpha=0xff)
                "cmp         %[alo], #-1                                                      \n\t"
                "it          eq                                                               \n\t"
                "cmpeq       %[ahi], #-1                                                      \n\t"
                "bne         40f                                                              \n\t"
                // Opaque case: copy source to destination
                "vst4.8      {d4-d7}, [%[dst]:128]                                            \n\t"
                // Drop through
                "29:                                                                          \n\t"
                "subs        %[len], #16                                                      \n\t"
                "bcs         10b                                                              \n\t"
                "b           49f                                                              \n\t"

                // Mixed or translucent pixels in d0-d3
                "30:                                                                          \n\t"
                S32A_OPAQUE_8PIX_BLEND(:128, d7, q0, q1,, YES, YES, IF_PRELOAD)
A(              "teq         %[alo], #0                                                       \n\t")
T(              "orrs        %[alo], %[alo], %[ahi]                                           \n\t")
                "vld4.8      {d0-d3}, [%[src]]!                                               \n\t"
                "beq         29b                                                              \n\t"
A(              "orrs        %[tmp], %[tmp], %[ahi]                                           \n\t")
T(              "orns        %[tmp], %[tmp], %[ahi]                                           \n\t")
                "bne         40f                                                              \n\t"
                "vst4.8      {d4-d7}, [%[dst]:128]                                            \n\t"
                "b           29b                                                              \n\t"

                // Mixed or translucent pixels in d4-d7
                "40:                                                                          \n\t"
                S32A_OPAQUE_8PIX_BLEND(:128, d3, q2, q3, \
                "subs        %[len], #16", YES, YES, NO)
                "bcc         50f                                                              \n\t"
A(              "teq         %[alo], #0                                                       \n\t")
T(              "orrs        %[alo], %[alo], %[ahi]                                           \n\t")
                "vld4.8      {d4-d7}, [%[src]]!                                               \n\t"
                "beq         19b                                                              \n\t"
A(              "orrs        %[tmp], %[tmp], %[ahi]                                           \n\t")
T(              "orns        %[tmp], %[tmp], %[ahi]                                           \n\t")
                "bne         30b                                                              \n\t"
                "vst4.8      {d0-d3}, [%[dst]:128]                                            \n\t"
                "b           19b                                                              \n\t"

                "49:                                                                          \n\t"
                "add         %[dst], #8*4                                                     \n\t"
                "50:                                                                          \n\t"
        : // Outputs
                [dst]"+r"(dst),
                [src]"+r"(src),
                [len]"+r"(len),
                [alo]"+r"(alo),
                [ahi]"+r"(ahi),
                [tmp]"+r"(tmp)
        : // Inputs
        : // Clobbers
                "cc", "memory", "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
        );

        // Do trailing pixels.
        // There will always be more than 8 of these, and the first 8 are already in d0-d3
        S32A_OPAQUE_8PIX_PROCESS(:128, NO);
        if (len & 7)
            S32A_OPAQUE_7PIX_PROCESS(S32A_LOADSTORE_TRAILING_7, len & 7);
    }
    return;

#endif // __ARM_64BIT_STATE
#endif

#if SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
    while (len >= 8) {
        __lasx_xvst(SkPMSrcOver_LASX(__lasx_xvld(src, 0),
                                     __lasx_xvld(dst, 0)), (__m256i*)dst, 0);
        src += 8;
        dst += 8;
        len -= 8;
    }
#endif

#if SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
    while (len >= 4) {
        __lsx_vst(SkPMSrcOver_LSX(__lsx_vld(src, 0),
                                  __lsx_vld(dst, 0)), (__m128i*)dst, 0);
        src += 4;
        dst += 4;
        len -= 4;
    }
#endif

    while (len --> 0) {
        *dst = SkPMSrcOver(*src, *dst);
        src++;
        dst++;
    }
}

// Blend constant color over count dst pixels
/*not static*/
inline void blit_row_color32(SkPMColor* dst, int count, SkPMColor color) {
    constexpr int N = 4;  // 8, 16 also reasonable choices
    using U32 = skvx::Vec<  N, uint32_t>;
    using U16 = skvx::Vec<4*N, uint16_t>;
    using U8  = skvx::Vec<4*N, uint8_t>;

    // Note when the kernel is used below, the "src" is the existing pixel color.
    auto kernel = [color](U32 src) {
        unsigned invA = SkAlpha255To256(255 - SkGetPackedA32(color));
        SkASSERT(0 < invA && invA < 256);  // We handle alpha == 0 or alpha == 255 specially.

        // color is premul, so the channels have already been
        // scaled by alpha. We just need to scale src by (255 - a)
        // using the trick of adding 1 and dividing by 256 which is
        // much faster than dividing by 255. Then we can add that
        // to color to get the result.
        U8 s = sk_bit_cast<U8>(src),
           a = U8(invA);
        U16 c = skvx::cast<uint16_t>(sk_bit_cast<U8>(U32(color))),
            r = (mull(s,a) >> 8) + c;
        return sk_bit_cast<U32>(skvx::cast<uint8_t>(r));
    };

    while (count >= N) {
        kernel(U32::Load(dst)).store(dst);
        dst   += N;
        count -= N;
    }
    while (count --> 0) {
        *dst = kernel(U32{*dst})[0];
        dst++;
    }
}

}  // namespace SK_OPTS_NS

#endif//SkBlitRow_opts_DEFINED
