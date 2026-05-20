/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapProcState_opts_DEFINED
#define SkBitmapProcState_opts_DEFINED

#include "src/base/SkMSAN.h"
#include "src/base/SkVx.h"
#include "src/core/SkBitmapProcState.h"

// SkBitmapProcState optimized Shader, Sample, or Matrix procs.
//
// Only S32_alpha_D32_filter_DX exploits instructions beyond
// our common baseline SSE2/NEON instruction sets, so that's
// all that lives here.
//
// The rest are scattershot at the moment but I want to get them
// all migrated to be normal code inside SkBitmapProcState.cpp.

#if SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_SSE2
    #include <immintrin.h>
#elif defined(SK_ARM_HAS_NEON)
    #include <arm_neon.h>
#elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
    #include <lasxintrin.h>
#elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
    #include <lsxintrin.h>
#endif

namespace SK_OPTS_NS {

// This same basic packing scheme is used throughout the file.
template <typename U32, typename Out>
static void decode_packed_coordinates_and_weight(U32 packed, Out* v0, Out* v1, Out* w) {
    *v0 = (packed >> 18);       // Integer coordinate x0 or y0.
    *v1 = (packed & 0x3fff);    // Integer coordinate x1 or y1.
    *w  = (packed >> 14) & 0xf; // Lerp weight for v1; weight for v0 is 16-w.
}

#if SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_SSSE3

    /*not static*/ inline
    void S32_alpha_D32_filter_DX(const SkBitmapProcState& s,
                                 const uint32_t* xy, int count, uint32_t* colors) {
        SkASSERT(count > 0 && colors != nullptr);
        SkASSERT(s.fBilerp);
        SkASSERT(kN32_SkColorType == s.fPixmap.colorType());
        SkASSERT(s.fAlphaScale <= 256);

        // interpolate_in_x() is the crux of the SSSE3 implementation,
        // interpolating in X for up to two output pixels (A and B) using _mm_maddubs_epi16().
        auto interpolate_in_x = [](uint32_t A0, uint32_t A1,
                                   uint32_t B0, uint32_t B1,
                                   __m128i interlaced_x_weights) {
            // _mm_maddubs_epi16() is a little idiosyncratic, but great as the core of a lerp.
            //
            // It takes two arguments interlaced byte-wise:
            //    - first  arg: [ l,r, ... 7 more pairs of unsigned 8-bit values ...]
            //    - second arg: [ w,W, ... 7 more pairs of   signed 8-bit values ...]
            // and returns 8 signed 16-bit values: [ l*w + r*W, ... 7 more ... ].
            //
            // That's why we go to all this trouble to make interlaced_x_weights,
            // and here we're about to interlace A0 with A1 and B0 with B1 to match.
            //
            // Our interlaced_x_weights are all in [0,16], and so we need not worry about
            // the signedness of that input nor about the signedness of the output.

            __m128i interlaced_A = _mm_unpacklo_epi8(_mm_cvtsi32_si128(A0), _mm_cvtsi32_si128(A1)),
                    interlaced_B = _mm_unpacklo_epi8(_mm_cvtsi32_si128(B0), _mm_cvtsi32_si128(B1));

            return _mm_maddubs_epi16(_mm_unpacklo_epi64(interlaced_A, interlaced_B),
                                     interlaced_x_weights);
        };

        // Interpolate {A0..A3} --> output pixel A, and {B0..B3} --> output pixel B.
        // Returns two pixels, with each color channel in a 16-bit lane of the __m128i.
        auto interpolate_in_x_and_y = [&](uint32_t A0, uint32_t A1,
                                          uint32_t A2, uint32_t A3,
                                          uint32_t B0, uint32_t B1,
                                          uint32_t B2, uint32_t B3,
                                          __m128i interlaced_x_weights,
                                          int wy) {
            // Interpolate each row in X, leaving 16-bit lanes scaled by interlaced_x_weights.
            __m128i top = interpolate_in_x(A0,A1, B0,B1, interlaced_x_weights),
                    bot = interpolate_in_x(A2,A3, B2,B3, interlaced_x_weights);

            // Interpolate in Y.  As in the SSE2 code, we calculate top*(16-wy) + bot*wy
            // as 16*top + (bot-top)*wy to save a multiply.
            __m128i px = _mm_add_epi16(_mm_slli_epi16(top, 4),
                                       _mm_mullo_epi16(_mm_sub_epi16(bot, top),
                                                       _mm_set1_epi16(wy)));

            // Scale down by total max weight 16x16 = 256.
            px = _mm_srli_epi16(px, 8);

            // Scale by alpha if needed.
            if (s.fAlphaScale < 256) {
                px = _mm_srli_epi16(_mm_mullo_epi16(px, _mm_set1_epi16(s.fAlphaScale)), 8);
            }
            return px;
        };

        // We're in _DX mode here, so we're only varying in X.
        // That means the first entry of xy is our constant pair of Y coordinates and weight in Y.
        // All the other entries in xy will be pairs of X coordinates and the X weight.
        int y0, y1, wy;
        decode_packed_coordinates_and_weight(*xy++, &y0, &y1, &wy);

        auto row0 = (const uint32_t*)((const uint8_t*)s.fPixmap.addr() + y0 * s.fPixmap.rowBytes()),
             row1 = (const uint32_t*)((const uint8_t*)s.fPixmap.addr() + y1 * s.fPixmap.rowBytes());

        while (count >= 4) {
            // We can really get going, loading 4 X-pairs at a time to produce 4 output pixels.
            int x0[4],
                x1[4];
            __m128i wx;

            // decode_packed_coordinates_and_weight(), 4x.
            __m128i packed = _mm_loadu_si128((const __m128i*)xy);
            _mm_storeu_si128((__m128i*)x0, _mm_srli_epi32(packed, 18));
            _mm_storeu_si128((__m128i*)x1, _mm_and_si128 (packed, _mm_set1_epi32(0x3fff)));
            wx = _mm_and_si128(_mm_srli_epi32(packed, 14), _mm_set1_epi32(0xf));  // [0,15]

            // Splat each x weight 4x (for each color channel) as wr for pixels on the right at x1,
            // and sixteen minus that as wl for pixels on the left at x0.
            __m128i wr = _mm_shuffle_epi8(wx, _mm_setr_epi8(0,0,0,0,4,4,4,4,8,8,8,8,12,12,12,12)),
                    wl = _mm_sub_epi8(_mm_set1_epi8(16), wr);

            // We need to interlace wl and wr for _mm_maddubs_epi16().
            __m128i interlaced_x_weights_AB = _mm_unpacklo_epi8(wl,wr),
                    interlaced_x_weights_CD = _mm_unpackhi_epi8(wl,wr);

            enum { A,B,C,D };

            // interpolate_in_x_and_y() can produce two output pixels (A and B) at a time
            // from eight input pixels {A0..A3} and {B0..B3}, arranged in a 2x2 grid for each.
            __m128i AB = interpolate_in_x_and_y(row0[x0[A]], row0[x1[A]],
                                                row1[x0[A]], row1[x1[A]],
                                                row0[x0[B]], row0[x1[B]],
                                                row1[x0[B]], row1[x1[B]],
                                                interlaced_x_weights_AB, wy);

            // Once more with the other half of the x-weights for two more pixels C,D.
            __m128i CD = interpolate_in_x_and_y(row0[x0[C]], row0[x1[C]],
                                                row1[x0[C]], row1[x1[C]],
                                                row0[x0[D]], row0[x1[D]],
                                                row1[x0[D]], row1[x1[D]],
                                                interlaced_x_weights_CD, wy);

            // Scale by alpha, pack back together to 8-bit lanes, and write out four pixels!
            _mm_storeu_si128((__m128i*)colors, _mm_packus_epi16(AB, CD));
            xy     += 4;
            colors += 4;
            count  -= 4;
        }

        while (count --> 0) {
            // This is exactly the same flow as the count >= 4 loop above, but writing one pixel.
            int x0, x1, wx;
            decode_packed_coordinates_and_weight(*xy++, &x0, &x1, &wx);

            // As above, splat out wx four times as wr, and sixteen minus that as wl.
            __m128i wr = _mm_set1_epi8(wx),     // This splats it out 16 times, but that's fine.
                    wl = _mm_sub_epi8(_mm_set1_epi8(16), wr);

            __m128i interlaced_x_weights = _mm_unpacklo_epi8(wl, wr);

            __m128i A = interpolate_in_x_and_y(row0[x0], row0[x1],
                                               row1[x0], row1[x1],
                                                      0,        0,
                                                      0,        0,
                                               interlaced_x_weights, wy);

            *colors++ = _mm_cvtsi128_si32(_mm_packus_epi16(A, _mm_setzero_si128()));
        }
    }


#elif SK_CPU_X64_LEVEL >= SK_CPU_X64_LEVEL_SSE2

    /*not static*/ inline
    void S32_alpha_D32_filter_DX(const SkBitmapProcState& s,
                                 const uint32_t* xy, int count, uint32_t* colors) {
        SkASSERT(count > 0 && colors != nullptr);
        SkASSERT(s.fBilerp);
        SkASSERT(kN32_SkColorType == s.fPixmap.colorType());
        SkASSERT(s.fAlphaScale <= 256);

        int y0, y1, wy;
        decode_packed_coordinates_and_weight(*xy++, &y0, &y1, &wy);

        auto row0 = (const uint32_t*)( (const char*)s.fPixmap.addr() + y0 * s.fPixmap.rowBytes() ),
             row1 = (const uint32_t*)( (const char*)s.fPixmap.addr() + y1 * s.fPixmap.rowBytes() );

        // We'll put one pixel in the low 4 16-bit lanes to line up with wy,
        // and another in the upper 4 16-bit lanes to line up with 16 - wy.
        const __m128i allY = _mm_unpacklo_epi64(_mm_set1_epi16(   wy),   // Bottom pixel goes here.
                                                _mm_set1_epi16(16-wy));  // Top pixel goes here.

        while (count --> 0) {
            int x0, x1, wx;
            decode_packed_coordinates_and_weight(*xy++, &x0, &x1, &wx);

            // Load the 4 pixels we're interpolating, in this grid:
            //    | tl  tr |
            //    | bl  br |
            const __m128i tl = _mm_cvtsi32_si128(row0[x0]), tr = _mm_cvtsi32_si128(row0[x1]),
                          bl = _mm_cvtsi32_si128(row1[x0]), br = _mm_cvtsi32_si128(row1[x1]);

            // We want to calculate a sum of 4 pixels weighted in two directions:
            //
            //  sum = tl * (16-wy) * (16-wx)
            //      + bl * (   wy) * (16-wx)
            //      + tr * (16-wy) * (   wx)
            //      + br * (   wy) * (   wx)
            //
            // (Notice top --> 16-wy, bottom --> wy, left --> 16-wx, right --> wx.)
            //
            // We've already prepared allY as a vector containing [wy, 16-wy] as a way
            // to apply those y-direction weights.  So we'll start on the x-direction
            // first, grouping into left and right halves, lined up with allY:
            //
            //     L = [bl, tl]
            //     R = [br, tr]
            //
            //   sum = horizontalSum( allY * (L*(16-wx) + R*wx) )
            //
            // Rewriting that one more step, we can replace a multiply with a shift:
            //
            //   sum = horizontalSum( allY * (16*L + (R-L)*wx) )
            //
            // That's how we'll actually do this math.

            __m128i L = _mm_unpacklo_epi8(_mm_unpacklo_epi32(bl, tl), _mm_setzero_si128()),
                    R = _mm_unpacklo_epi8(_mm_unpacklo_epi32(br, tr), _mm_setzero_si128());

            __m128i inner = _mm_add_epi16(_mm_slli_epi16(L, 4),
                                          _mm_mullo_epi16(_mm_sub_epi16(R,L), _mm_set1_epi16(wx)));

            __m128i sum_in_x = _mm_mullo_epi16(inner, allY);

            // sum = horizontalSum( ... )
            __m128i sum = _mm_add_epi16(sum_in_x, _mm_srli_si128(sum_in_x, 8));

            // Get back to [0,255] by dividing by maximum weight 16x16 = 256.
            sum = _mm_srli_epi16(sum, 8);

            if (s.fAlphaScale < 256) {
                // Scale by alpha, which is in [0,256].
                sum = _mm_mullo_epi16(sum, _mm_set1_epi16(s.fAlphaScale));
                sum = _mm_srli_epi16(sum, 8);
            }

            // Pack back into 8-bit values and store.
            *colors++ = _mm_cvtsi128_si32(_mm_packus_epi16(sum, _mm_setzero_si128()));
        }
    }

#elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
    /*not static*/ inline
    void S32_alpha_D32_filter_DX(const SkBitmapProcState& s,
                                 const uint32_t* xy, int count, uint32_t* colors) {
        SkASSERT(count > 0 && colors != nullptr);
        SkASSERT(s.fBilerp);
        SkASSERT(kN32_SkColorType == s.fPixmap.colorType());
        SkASSERT(s.fAlphaScale <= 256);

        int y0, y1, wy;
        decode_packed_coordinates_and_weight(*xy++, &y0, &y1, &wy);

        auto row0 = (const uint32_t*)( (const char*)s.fPixmap.addr() + y0 * s.fPixmap.rowBytes() ),
             row1 = (const uint32_t*)( (const char*)s.fPixmap.addr() + y1 * s.fPixmap.rowBytes() );

        // We'll put one pixel in the low 16 16-bit lanes to line up with wy,
        // and another in the upper 16 16-bit lanes to line up with 16 - wy.
        __m256i allY = __lasx_xvilvl_d(__lasx_xvreplgr2vr_h(16-wy), __lasx_xvreplgr2vr_h(wy));

        while (count --> 0) {
            int x0, x1, wx;
            decode_packed_coordinates_and_weight(*xy++, &x0, &x1, &wx);

            // Load the 4 pixels we're interpolating, in this grid:
            //    | tl  tr |
            //    | bl  br |

            const __m256i zeros = __lasx_xvldi(0);
            const __m256i tl = __lasx_xvinsgr2vr_w(zeros, row0[x0], 0),
                          tr = __lasx_xvinsgr2vr_w(zeros, row0[x1], 0),
                          bl = __lasx_xvinsgr2vr_w(zeros, row1[x0], 0),
                          br = __lasx_xvinsgr2vr_w(zeros, row1[x1], 0);

            // We want to calculate a sum of 8 pixels weighted in two directions:
            //
            //  sum = tl * (16-wy) * (16-wx)
            //      + bl * (   wy) * (16-wx)
            //      + tr * (16-wy) * (   wx)
            //      + br * (   wy) * (   wx)
            //
            // (Notice top --> 16-wy, bottom --> wy, left --> 16-wx, right --> wx.)
            //
            // We've already prepared allY as a vector containing [wy, 16-wy] as a way
            // to apply those y-direction weights.  So we'll start on the x-direction
            // first, grouping into left and right halves, lined up with allY:
            //
            //     L = [bl, tl]
            //     R = [br, tr]
            //
            //   sum = horizontalSum( allY * (L*(16-wx) + R*wx) )
            //
            // Rewriting that one more step, we can replace a multiply with a shift:
            //
            //   sum = horizontalSum( allY * (16*L + (R-L)*wx) )
            //
            // That's how we'll actually do this math.

            __m256i L = __lasx_xvilvl_b(__lasx_xvldi(0), __lasx_xvilvl_w(tl, bl)),
                    R = __lasx_xvilvl_b(__lasx_xvldi(0), __lasx_xvilvl_w(tr, br));

            __m256i inner = __lasx_xvadd_h(__lasx_xvslli_h(L, 4),
                                           __lasx_xvmul_h(__lasx_xvsub_h(R,L),
                                                          __lasx_xvreplgr2vr_h(wx)));

            __m256i sum_in_x = __lasx_xvmul_h(inner, allY);

            // sum = horizontalSum( ... )
            __m256i sum = __lasx_xvadd_h(sum_in_x, __lasx_xvbsrl_v(sum_in_x, 8));

            // Get back to [0,255] by dividing by maximum weight 16x16 = 256.
            sum = __lasx_xvsrli_h(sum, 8);

            if (s.fAlphaScale < 256) {
                // Scale by alpha, which is in [0,256].
                sum = __lasx_xvmul_h(sum, __lasx_xvreplgr2vr_h(s.fAlphaScale));
                sum = __lasx_xvsrli_h(sum, 8);
            }

            // Pack back into 8-bit values and store.
            *colors++ = __lasx_xvpickve2gr_w(__lasx_xvpickev_b(__lasx_xvldi(0),
                                                               __lasx_xvsat_hu(sum, 8)), 0);
        }
    }

#elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX

    /*not static*/ inline
    void S32_alpha_D32_filter_DX(const SkBitmapProcState& s,
                                 const uint32_t* xy, int count, uint32_t* colors) {
        SkASSERT(count > 0 && colors != nullptr);
        SkASSERT(s.fBilerp);
        SkASSERT(kN32_SkColorType == s.fPixmap.colorType());
        SkASSERT(s.fAlphaScale <= 256);

        int y0, y1, wy;
        decode_packed_coordinates_and_weight(*xy++, &y0, &y1, &wy);

        auto row0 = (const uint32_t*)( (const char*)s.fPixmap.addr() + y0 * s.fPixmap.rowBytes() ),
             row1 = (const uint32_t*)( (const char*)s.fPixmap.addr() + y1 * s.fPixmap.rowBytes() );

        // We'll put one pixel in the low 8 16-bit lanes to line up with wy,
        // and another in the upper 8 16-bit lanes to line up with 16 - wy.
        __m128i allY = __lsx_vilvl_d(__lsx_vreplgr2vr_h(16-wy), __lsx_vreplgr2vr_h(wy));

        while (count --> 0) {
            int x0, x1, wx;
            decode_packed_coordinates_and_weight(*xy++, &x0, &x1, &wx);

            // Load the 4 pixels we're interpolating, in this grid:
            //    | tl  tr |
            //    | bl  br |
            const __m128i zeros = __lsx_vldi(0);
            const __m128i tl = __lsx_vinsgr2vr_w(zeros, row0[x0], 0),
                          tr = __lsx_vinsgr2vr_w(zeros, row0[x1], 0),
                          bl = __lsx_vinsgr2vr_w(zeros, row1[x0], 0),
                          br = __lsx_vinsgr2vr_w(zeros, row1[x1], 0);

            // We want to calculate a sum of 8 pixels weighted in two directions:
            //
            //  sum = tl * (16-wy) * (16-wx)
            //      + bl * (   wy) * (16-wx)
            //      + tr * (16-wy) * (   wx)
            //      + br * (   wy) * (   wx)
            //
            // (Notice top --> 16-wy, bottom --> wy, left --> 16-wx, right --> wx.)
            //
            // We've already prepared allY as a vector containing [wy, 16-wy] as a way
            // to apply those y-direction weights.  So we'll start on the x-direction
            // first, grouping into left and right halves, lined up with allY:
            //
            //     L = [bl, tl]
            //     R = [br, tr]
            //
            //   sum = horizontalSum( allY * (L*(16-wx) + R*wx) )
            //
            // Rewriting that one more step, we can replace a multiply with a shift:
            //
            //   sum = horizontalSum( allY * (16*L + (R-L)*wx) )
            //
            // That's how we'll actually do this math.


            __m128i L = __lsx_vilvl_b(__lsx_vldi(0), __lsx_vilvl_w(tl, bl)),
                    R = __lsx_vilvl_b(__lsx_vldi(0), __lsx_vilvl_w(tr, br));

            __m128i inner = __lsx_vadd_h(__lsx_vslli_h(L, 4),
                                         __lsx_vmul_h(__lsx_vsub_h(R,L),
                                                      __lsx_vreplgr2vr_h(wx)));

            __m128i sum_in_x = __lsx_vmul_h(inner, allY);

            // sum = horizontalSum( ... )
            __m128i sum = __lsx_vadd_h(sum_in_x, __lsx_vbsrl_v(sum_in_x, 8));

            // Get back to [0,255] by dividing by maximum weight 16x16 = 256.
            sum = __lsx_vsrli_h(sum, 8);

            if (s.fAlphaScale < 256) {
                // Scale by alpha, which is in [0,256].
                sum = __lsx_vmul_h(sum, __lsx_vreplgr2vr_h(s.fAlphaScale));
                sum = __lsx_vsrli_h(sum, 8);
            }

            // Pack back into 8-bit values and store.
            *colors++ = __lsx_vpickve2gr_w(__lsx_vpickev_b(__lsx_vldi(0),
                                                           __lsx_vsat_hu(sum, 8)), 0);
        }
    }
#elif defined(SK_ARM_HAS_NEON) && !defined(__ARM_64BIT_STATE)

#define S32_ALPHA_D32_FILTER_DX_1PIX_NEON(opt)               \
            "ldr         %[x], [%[xy]], #4           \n\t"   \
            "uxth        %[tmp2], %[x], ror #16      \n\t"   \
            "lsl         %[tmp3], %[x], #2           \n\t"   \
            "bic         %[tmp2], #3                 \n\t"   \
            "uxth        %[tmp3], %[tmp3]            \n\t"   \
            "add         %[tmp0], %[row0], %[tmp2]   \n\t"   \
            "add         %[tmp1], %[row0], %[tmp3]   \n\t"   \
            "add         %[tmp2], %[row1], %[tmp2]   \n\t"   \
            "add         %[tmp3], %[row1], %[tmp3]   \n\t"   \
            "lsr         %[x], #14                   \n\t"   \
            "vldr        s0, [%[tmp0]]               \n\t"   \
            "and         %[x], #0xf                  \n\t"   \
            "vldr        s1, [%[tmp1]]               \n\t"   \
            "vldr        s2, [%[tmp2]]               \n\t"   \
            "vldr        s3, [%[tmp3]]               \n\t"   \
            "vdup.16     d2, %[x]                    \n\t"   \
            "vsub.i16    d3, d23, d2                 \n\t"   \
            "vmull.u8    q2, d0, d31                 \n\t"   \
            "vmlal.u8    q2, d1, d30                 \n\t"   \
            "vmul.u16    d0, d4, d3                  \n\t"   \
            "vmla.u16    d0, d5, d2                  \n\t"   \
            "vshr.u16    d0, #8                      \n\t"   \
            "vmul.u16    d0, d10                     \n\t"   \
            opt"                                     \n\t"   \
            "vshrn.u16   d0, q0, #8                  \n\t"   \
            "vst1.32     {d0[0]}, [%[dst]:32]!       \n\t"   \

void S32_alpha_D32_filter_DX(const SkBitmapProcState& s,
                             const uint32_t* SK_RESTRICT xy,
                             int count, SkPMColor* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != nullptr);
    SkASSERT(4 == s.fPixmap.info().bytesPerPixel());
    SkASSERT(s.fAlphaScale <= 256);

    int y0, y1, wy;
    decode_packed_coordinates_and_weight(*xy++, &y0, &y1, &wy);

    auto row0 = (const uint32_t*)( (const char*)s.fPixmap.addr() + y0 * s.fPixmap.rowBytes() ),
         row1 = (const uint32_t*)( (const char*)s.fPixmap.addr() + y1 * s.fPixmap.rowBytes() );

    uint32_t tmp0, tmp1, tmp2, tmp3, x;
    __asm__ volatile (
            "vmov.i16    d22, #0xf                   \n\t"
            "vmov.i16    d23, #0x10                  \n\t"
            "vmov.i32    q12, #0x3fff                \n\t"
            "vdup.32     q13, %[row0]                \n\t"
            "vdup.32     q14, %[row1]                \n\t"
            "vdup.i8     d30, %[subY]                \n\t"
            "vmov.i8     d31, #16                    \n\t"
            "vdup.16     q5, %[alpha]                \n\t"
            "vshl.i32    q12, #2                     \n\t"
            "tst         %[dst], #0xc                \n\t"
            "vsub.i8     d31, d30                    \n\t"
            "beq         2f                          \n\t"

            "1:                                      \n\t"
            S32_ALPHA_D32_FILTER_DX_1PIX_NEON(
            "add         %[tmp0], %[dst], #4         \n\t"
            "subs        %[len], #1                  \n\t"
            "it          ne                          \n\t"
            "tstne       %[tmp0], #0xc"
            )
            "bne         1b                          \n\t"

            "2:"
            "subs        %[len], #4                  \n\t"
            "bmi         13f                         \n\t"

            "vld1.32     {q8}, [%[xy]]!              \n\t"
            "vshr.u32    q9, q8, #16                 \n\t"
            "vand        q9, q12                     \n\t"
            "vadd.i32    q1, q13, q9                 \n\t"
            "vshl.i32    q0, q8, #2                  \n\t"
            "vand        q0, q12                     \n\t"
            "vadd.i32    q2, q13, q0                 \n\t"
            "vmov        %[tmp0], s4                 \n\t"
            "vmov        %[tmp1], s5                 \n\t"
            "vadd.i32    q3, q14, q9                 \n\t"
            "vmov        %[tmp2], %[tmp3], d3        \n\t"

            "11:                                     \n\t"
            "vadd.i32    q4, q14, q0                 \n\t"
            "vldr        s4, [%[tmp0]]               \n\t"
            "vmov        %[tmp0], s8                 \n\t"
            "vldr        s5, [%[tmp1]]               \n\t"
            "vmov        %[tmp1], s9                 \n\t"
            "vldr        s6, [%[tmp2]]               \n\t"
            "vmov        %[tmp2], s10                \n\t"
            "vldr        s7, [%[tmp3]]               \n\t"
            "vmov        %[tmp3], s11                \n\t"
            "vldr        s8, [%[tmp0]]               \n\t"
            "vmov        %[tmp0], s12                \n\t"
            "vldr        s9, [%[tmp1]]               \n\t"
            "vmov        %[tmp1], s13                \n\t"
            "vldr        s10, [%[tmp2]]              \n\t"
            "vmov        %[tmp2], s14                \n\t"
            "vldr        s11, [%[tmp3]]              \n\t"
            "vmov        %[tmp3], s15                \n\t"
            "vldr        s12, [%[tmp0]]              \n\t"
            "vmov        %[tmp0], s16                \n\t"
            "vldr        s13, [%[tmp1]]              \n\t"
            "vmov        %[tmp1], s17                \n\t"
            "vldr        s14, [%[tmp2]]              \n\t"
            "vmov        %[tmp2], s18                \n\t"
            "vldr        s15, [%[tmp3]]              \n\t"
            "vmov        %[tmp3], s19                \n\t"
            "vldr        s16, [%[tmp0]]              \n\t"
            "vshrn.i32   d1, q8, #14                 \n\t"
            "vldr        s17, [%[tmp1]]              \n\t"
            "vand        d1, d22                     \n\t"
            "vldr        s18, [%[tmp2]]              \n\t"
            "vsub.i16    d0, d23, d1                 \n\t"
            "vldr        s19, [%[tmp3]]              \n\t"
            "vmull.u8    q10, d2, d31                \n\t"
            "vmlal.u8    q10, d6, d30                \n\t"
            "vmull.u8    q1, d3, d31                 \n\t"
            "vmlal.u8    q1, d7, d30                 \n\t"
            "vmull.u8    q3, d4, d31                 \n\t"
            "subs        %[len], #4                  \n\t"
            "vmlal.u8    q3, d8, d30                 \n\t"
            "bmi         12f                         \n\t"

            "  vld1.32     {q8}, [%[xy]]!            \n\t"
            "vmull.u8    q2, d5, d31                 \n\t"
            "vmlal.u8    q2, d9, d30                 \n\t"
            "vmul.u16    d8, d20, d0[0]              \n\t"
            "  vshr.u32    d18, d16, #16             \n\t"
            "vmul.u16    d9, d21, d0[1]              \n\t"
            "  vshr.u32    d19, d17, #16             \n\t"
            "vmul.u16    d20, d2, d0[2]              \n\t"
            "  vand        d18, d24                  \n\t"
            "vmul.u16    d21, d3, d0[3]              \n\t"
            "  vand        d19, d25                  \n\t"
            "vmla.u16    d8, d6, d1[0]               \n\t"
            "  vadd.i32    d2, d26, d18              \n\t"
            "vmla.u16    d9, d7, d1[1]               \n\t"
            "  vadd.i32    d3, d27, d19              \n\t"
            "vmla.u16    d20, d4, d1[2]              \n\t"
            "  vshl.i32    d0, d16, #2               \n\t"
            "vmla.u16    d21, d5, d1[3]              \n\t"
            "  vshl.i32    d1, d17, #2               \n\t"
            "  vand        q0, q12                   \n\t"
            "  vadd.i32    q2, q13, q0               \n\t"
            "vshr.u16    q4, #8                      \n\t"
            "vshr.u16    q10, #8                     \n\t"
            "vmul.u16    q4, q5                      \n\t"
            "vmul.u16    q10, q5                     \n\t"
            "  vmov        %[tmp0], %[tmp1], d2      \n\t"
            "  vadd.i32    q3, q14, q9               \n\t"
            "  vmov        %[tmp2], %[tmp3], d3      \n\t"
            "vshrn.u16   d8, q4, #8                  \n\t"
            "vshrn.u16   d9, q10, #8                 \n\t"
            "vst1.32     {q4}, [%[dst]:128]!         \n\t"
            "b           11b                         \n\t"

            "12:                                     \n\t"
            "vmull.u8    q2, d5, d31                 \n\t"
            "vmlal.u8    q2, d9, d30                 \n\t"
            "vmul.u16    d8, d20, d0[0]              \n\t"
            "vmul.u16    d9, d21, d0[1]              \n\t"
            "vmul.u16    d20, d2, d0[2]              \n\t"
            "vmul.u16    d21, d3, d0[3]              \n\t"
            "vmla.u16    d8, d6, d1[0]               \n\t"
            "vmla.u16    d9, d7, d1[1]               \n\t"
            "vmla.u16    d20, d4, d1[2]              \n\t"
            "vmla.u16    d21, d5, d1[3]              \n\t"
            "vshr.u16    q4, #8                      \n\t"
            "vshr.u16    q10, #8                     \n\t"
            "vmul.u16    q4, q5                      \n\t"
            "vmul.u16    q10, q5                     \n\t"
            "vshrn.u16   d8, q4, #8                  \n\t"
            "vshrn.u16   d9, q10, #8                 \n\t"
            "vst1.32     {q4}, [%[dst]:128]!         \n\t"

            "13:                                     \n\t"
            "adds        %[len], #4-1                \n\t"
            "bmi         22f                         \n\t"

            "21:                                     \n\t"
            S32_ALPHA_D32_FILTER_DX_1PIX_NEON("subs %[len], #1")
            "bpl         21b                         \n\t"

            "22:                                     \n\t"
    : // Outputs
             [dst]"+r"(colors),
              [xy]"+r"(xy),
             [len]"+r"(count),
            [tmp0]"=&r"(tmp0),
            [tmp1]"=&r"(tmp1),
            [tmp2]"=&r"(tmp2),
            [tmp3]"=&r"(tmp3),
               [x]"=&r"(x)
    : // Inputs
            [alpha]"r"(s.fAlphaScale),
             [row0]"r"(row0),
             [row1]"r"(row1),
             [subY]"r"(wy)
    : // Clobbers
            "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q8", "q9", "q10", "q11", "q12", "q13", "q14"
    );
}
#if 1
        // Copied from just below
        static void filter_and_scale_by_alpha(unsigned x, unsigned y,
                                              SkPMColor a00, SkPMColor a01,
                                              SkPMColor a10, SkPMColor a11,
                                              SkPMColor *dst,
                                              uint16_t scale) {
            uint8x8_t vy, vconst16_8, v16_y, vres;
            uint16x4_t vx, vconst16_16, v16_x, tmp, vscale;
            uint32x2_t va0, va1;
            uint16x8_t tmp1, tmp2;

            vy = vdup_n_u8(y);                // duplicate y into vy
            vconst16_8 = vmov_n_u8(16);       // set up constant in vconst16_8
            v16_y = vsub_u8(vconst16_8, vy);  // v16_y = 16-y

            va0 = vdup_n_u32(a00);            // duplicate a00
            va1 = vdup_n_u32(a10);            // duplicate a10
            va0 = vset_lane_u32(a01, va0, 1); // set top to a01
            va1 = vset_lane_u32(a11, va1, 1); // set top to a11

            tmp1 = vmull_u8(vreinterpret_u8_u32(va0), v16_y); // tmp1 = [a01|a00] * (16-y)
            tmp2 = vmull_u8(vreinterpret_u8_u32(va1), vy);    // tmp2 = [a11|a10] * y

            vx = vdup_n_u16(x);                // duplicate x into vx
            vconst16_16 = vmov_n_u16(16);      // set up constant in vconst16_16
            v16_x = vsub_u16(vconst16_16, vx); // v16_x = 16-x

            tmp = vmul_u16(vget_high_u16(tmp1), vx);        // tmp  = a01 * x
            tmp = vmla_u16(tmp, vget_high_u16(tmp2), vx);   // tmp += a11 * x
            tmp = vmla_u16(tmp, vget_low_u16(tmp1), v16_x); // tmp += a00 * (16-x)
            tmp = vmla_u16(tmp, vget_low_u16(tmp2), v16_x); // tmp += a10 * (16-x)

            if (scale < 256) {
                vscale = vdup_n_u16(scale);        // duplicate scale
                tmp = vshr_n_u16(tmp, 8);          // shift down result by 8
                tmp = vmul_u16(tmp, vscale);       // multiply result by scale
            }

            vres = vshrn_n_u16(vcombine_u16(tmp, vcreate_u16(0)), 8); // shift down result by 8
            vst1_lane_u32(dst, vreinterpret_u32_u8(vres), 0);         // store result
        }
#endif

#else

    // The NEON code only actually differs from the portable code in the
    // filtering step after we've loaded all four pixels we want to bilerp.

    #if defined(SK_ARM_HAS_NEON)
        static void filter_and_scale_by_alpha(unsigned x, unsigned y,
                                              SkPMColor a00, SkPMColor a01,
                                              SkPMColor a10, SkPMColor a11,
                                              SkPMColor *dst,
                                              uint16_t scale) {
            uint8x8_t vy, vconst16_8, v16_y, vres;
            uint16x4_t vx, vconst16_16, v16_x, tmp, vscale;
            uint32x2_t va0, va1;
            uint16x8_t tmp1, tmp2;

            vy = vdup_n_u8(y);                // duplicate y into vy
            vconst16_8 = vmov_n_u8(16);       // set up constant in vconst16_8
            v16_y = vsub_u8(vconst16_8, vy);  // v16_y = 16-y

            va0 = vdup_n_u32(a00);            // duplicate a00
            va1 = vdup_n_u32(a10);            // duplicate a10
            va0 = vset_lane_u32(a01, va0, 1); // set top to a01
            va1 = vset_lane_u32(a11, va1, 1); // set top to a11

            tmp1 = vmull_u8(vreinterpret_u8_u32(va0), v16_y); // tmp1 = [a01|a00] * (16-y)
            tmp2 = vmull_u8(vreinterpret_u8_u32(va1), vy);    // tmp2 = [a11|a10] * y

            vx = vdup_n_u16(x);                // duplicate x into vx
            vconst16_16 = vmov_n_u16(16);      // set up constant in vconst16_16
            v16_x = vsub_u16(vconst16_16, vx); // v16_x = 16-x

            tmp = vmul_u16(vget_high_u16(tmp1), vx);        // tmp  = a01 * x
            tmp = vmla_u16(tmp, vget_high_u16(tmp2), vx);   // tmp += a11 * x
            tmp = vmla_u16(tmp, vget_low_u16(tmp1), v16_x); // tmp += a00 * (16-x)
            tmp = vmla_u16(tmp, vget_low_u16(tmp2), v16_x); // tmp += a10 * (16-x)

            if (scale < 256) {
                vscale = vdup_n_u16(scale);        // duplicate scale
                tmp = vshr_n_u16(tmp, 8);          // shift down result by 8
                tmp = vmul_u16(tmp, vscale);       // multiply result by scale
            }

            vres = vshrn_n_u16(vcombine_u16(tmp, vcreate_u16((uint64_t)0)), 8); // shift down result by 8
            vst1_lane_u32(dst, vreinterpret_u32_u8(vres), 0);         // store result
        }
    #else
        static void filter_and_scale_by_alpha(unsigned x, unsigned y,
                                              SkPMColor a00, SkPMColor a01,
                                              SkPMColor a10, SkPMColor a11,
                                              SkPMColor* dstColor,
                                              unsigned alphaScale) {
            SkASSERT((unsigned)x <= 0xF);
            SkASSERT((unsigned)y <= 0xF);
            SkASSERT(alphaScale <= 256);

            int xy = x * y;
            const uint32_t mask = 0xFF00FF;

            int scale = 256 - 16*y - 16*x + xy;
            uint32_t lo = (a00 & mask) * scale;
            uint32_t hi = ((a00 >> 8) & mask) * scale;

            scale = 16*x - xy;
            lo += (a01 & mask) * scale;
            hi += ((a01 >> 8) & mask) * scale;

            scale = 16*y - xy;
            lo += (a10 & mask) * scale;
            hi += ((a10 >> 8) & mask) * scale;

            lo += (a11 & mask) * xy;
            hi += ((a11 >> 8) & mask) * xy;

            if (alphaScale < 256) {
                lo = ((lo >> 8) & mask) * alphaScale;
                hi = ((hi >> 8) & mask) * alphaScale;
            }

            *dstColor = ((lo >> 8) & mask) | (hi & ~mask);
        }
    #endif


    /*not static*/ inline
    void S32_alpha_D32_filter_DX(const SkBitmapProcState& s,
                                 const uint32_t* xy, int count, SkPMColor* colors) {
        SkASSERT(count > 0 && colors != nullptr);
        SkASSERT(s.fBilerp);
        SkASSERT(4 == s.fPixmap.info().bytesPerPixel());
        SkASSERT(s.fAlphaScale <= 256);

        int y0, y1, wy;
        decode_packed_coordinates_and_weight(*xy++, &y0, &y1, &wy);

        auto row0 = (const uint32_t*)( (const char*)s.fPixmap.addr() + y0 * s.fPixmap.rowBytes() ),
             row1 = (const uint32_t*)( (const char*)s.fPixmap.addr() + y1 * s.fPixmap.rowBytes() );

        while (count --> 0) {
            int x0, x1, wx;
            decode_packed_coordinates_and_weight(*xy++, &x0, &x1, &wx);

            filter_and_scale_by_alpha(wx, wy,
                                      row0[x0], row0[x1],
                                      row1[x0], row1[x1],
                                      colors++,
                                      s.fAlphaScale);
        }
    }

#endif

#if defined(SK_ARM_HAS_NEON)
    /*not static*/ inline
    void S32_alpha_D32_filter_DXDY(const SkBitmapProcState& s,
                                   const uint32_t* xy, int count, SkPMColor* colors) {
        SkASSERT(count > 0 && colors != nullptr);
        SkASSERT(s.fBilerp);
        SkASSERT(4 == s.fPixmap.info().bytesPerPixel());
        SkASSERT(s.fAlphaScale <= 256);

        auto src = (const char*)s.fPixmap.addr();
        size_t rb = s.fPixmap.rowBytes();

        while (count --> 0) {
            int y0, y1, wy,
                x0, x1, wx;
            decode_packed_coordinates_and_weight(*xy++, &y0, &y1, &wy);
            decode_packed_coordinates_and_weight(*xy++, &x0, &x1, &wx);

            auto row0 = (const uint32_t*)(src + y0*rb),
                 row1 = (const uint32_t*)(src + y1*rb);

            filter_and_scale_by_alpha(wx, wy,
                                      row0[x0], row0[x1],
                                      row1[x0], row1[x1],
                                      colors++,
                                      s.fAlphaScale);
        }
    }
#else
    // It's not yet clear whether it's worthwhile specializing for other architectures.
    constexpr static void (*S32_alpha_D32_filter_DXDY)(const SkBitmapProcState&,
                                                       const uint32_t*, int, SkPMColor*) = nullptr;
#endif

#if defined(SK_ARM_HAS_NEON) && !defined(__ARM_64BIT_STATE)

#define S32_OPAQUE_D32_FILTER_DX_1PIX_NEON(opt)              \
            "ldr         %[x], [%[xy]], #4           \n\t"   \
            "uxth        %[tmp2], %[x], ror #16      \n\t"   \
            "lsl         %[tmp3], %[x], #2           \n\t"   \
            "bic         %[tmp2], #3                 \n\t"   \
            "uxth        %[tmp3], %[tmp3]            \n\t"   \
            "add         %[tmp0], %[row0], %[tmp2]   \n\t"   \
            "add         %[tmp1], %[row0], %[tmp3]   \n\t"   \
            "add         %[tmp2], %[row1], %[tmp2]   \n\t"   \
            "add         %[tmp3], %[row1], %[tmp3]   \n\t"   \
            "lsr         %[x], #14                   \n\t"   \
            "vldr        s0, [%[tmp0]]               \n\t"   \
            "and         %[x], #0xf                  \n\t"   \
            "vldr        s1, [%[tmp1]]               \n\t"   \
            "vldr        s2, [%[tmp2]]               \n\t"   \
            "vldr        s3, [%[tmp3]]               \n\t"   \
            "vdup.16     d2, %[x]                    \n\t"   \
            "vsub.i16    d3, d23, d2                 \n\t"   \
            "vmull.u8    q2, d0, d31                 \n\t"   \
            "vmlal.u8    q2, d1, d30                 \n\t"   \
            "vmul.u16    d0, d4, d3                  \n\t"   \
            "vmla.u16    d0, d5, d2                  \n\t"   \
            opt"                                     \n\t"   \
            "vshrn.u16   d0, q0, #8                  \n\t"   \
            "vst1.32     {d0[0]}, [%[dst]:32]!       \n\t"   \

void S32_opaque_D32_filter_DX(const SkBitmapProcState& s,
                              const uint32_t* SK_RESTRICT xy,
                              int count, SkPMColor* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != nullptr);
    SkASSERT(4 == s.fPixmap.info().bytesPerPixel());
    SkASSERT(s.fAlphaScale == 256);

    int y0, y1, wy;
    decode_packed_coordinates_and_weight(*xy++, &y0, &y1, &wy);

    auto row0 = (const uint32_t*)( (const char*)s.fPixmap.addr() + y0 * s.fPixmap.rowBytes() ),
         row1 = (const uint32_t*)( (const char*)s.fPixmap.addr() + y1 * s.fPixmap.rowBytes() );

    uint32_t tmp0, tmp1, tmp2, tmp3, x;
    __asm__ volatile (
            "vmov.i16    d22, #0xf                   \n\t"
            "vmov.i16    d23, #0x10                  \n\t"
            "vmov.i32    q12, #0x3fff                \n\t"
            "vdup.32     q13, %[row0]                \n\t"
            "vdup.32     q14, %[row1]                \n\t"
            "vdup.i8     d30, %[subY]                \n\t"
            "vmov.i8     d31, #16                    \n\t"
            "vshl.i32    q12, #2                     \n\t"
            "tst         %[dst], #0xc                \n\t"
            "vsub.i8     d31, d30                    \n\t"
            "beq         2f                          \n\t"

            "1:                                      \n\t"
            S32_OPAQUE_D32_FILTER_DX_1PIX_NEON(
            "add         %[tmp0], %[dst], #4         \n\t"
            "subs        %[len], #1                  \n\t"
            "it          ne                          \n\t"
            "tstne       %[tmp0], #0xc"
            )
            "bne         1b                          \n\t"

            "2:"
            "subs        %[len], #4                  \n\t"
            "bmi         13f                         \n\t"

            "vld1.32     {q8}, [%[xy]]!              \n\t"
            "vshr.u32    q9, q8, #16                 \n\t"
            "vand        q9, q12                     \n\t"
            "vadd.i32    q1, q13, q9                 \n\t"
            "vshl.i32    q0, q8, #2                  \n\t"
            "vand        q0, q12                     \n\t"
            "vadd.i32    q2, q13, q0                 \n\t"
            "vmov        %[tmp0], s4                 \n\t"
            "vmov        %[tmp1], s5                 \n\t"

            "11:                                     \n\t"
            "vadd.i32    q3, q14, q9                 \n\t"
            "vmov        %[tmp2], %[tmp3], d3        \n\t"
            "vadd.i32    q4, q14, q0                 \n\t"
            "vldr        s4, [%[tmp0]]               \n\t"
            "vmov        %[tmp0], s8                 \n\t"
            "vldr        s5, [%[tmp1]]               \n\t"
            "vmov        %[tmp1], s9                 \n\t"
            "vldr        s6, [%[tmp2]]               \n\t"
            "vmov        %[tmp2], s10                \n\t"
            "vldr        s7, [%[tmp3]]               \n\t"
            "vmov        %[tmp3], s11                \n\t"
            "vldr        s8, [%[tmp0]]               \n\t"
            "vmov        %[tmp0], s12                \n\t"
            "vldr        s9, [%[tmp1]]               \n\t"
            "vmov        %[tmp1], s13                \n\t"
            "vldr        s10, [%[tmp2]]              \n\t"
            "vmov        %[tmp2], s14                \n\t"
            "vldr        s11, [%[tmp3]]              \n\t"
            "vmov        %[tmp3], s15                \n\t"
            "vldr        s12, [%[tmp0]]              \n\t"
            "vmov        %[tmp0], s16                \n\t"
            "vldr        s13, [%[tmp1]]              \n\t"
            "vmov        %[tmp1], s17                \n\t"
            "vldr        s14, [%[tmp2]]              \n\t"
            "vmov        %[tmp2], s18                \n\t"
            "vldr        s15, [%[tmp3]]              \n\t"
            "vmov        %[tmp3], s19                \n\t"
            "vldr        s16, [%[tmp0]]              \n\t"
            "vshrn.i32   d1, q8, #14                 \n\t"
            "vldr        s17, [%[tmp1]]              \n\t"
            "vand        d1, d22                     \n\t"
            "vldr        s18, [%[tmp2]]              \n\t"
            "vsub.i16    d0, d23, d1                 \n\t"
            "vldr        s19, [%[tmp3]]              \n\t"
            "vmull.u8    q10, d2, d31                \n\t"
            "vmlal.u8    q10, d6, d30                \n\t"
            "vmull.u8    q1, d3, d31                 \n\t"
            "vmlal.u8    q1, d7, d30                 \n\t"
            "vmull.u8    q3, d4, d31                 \n\t"
            "subs        %[len], #4                  \n\t"
            "vmlal.u8    q3, d8, d30                 \n\t"
            "bmi         12f                         \n\t"

            "  vld1.32     {q8}, [%[xy]]!            \n\t"
            "vmull.u8    q2, d5, d31                 \n\t"
            "vmlal.u8    q2, d9, d30                 \n\t"
            "vmul.u16    d8, d20, d0[0]              \n\t"
            "  vshr.u32    d18, d16, #16             \n\t"
            "vmul.u16    d9, d21, d0[1]              \n\t"
            "  vshr.u32    d19, d17, #16             \n\t"
            "vmul.u16    d20, d2, d0[2]              \n\t"
            "  vand        d18, d24                  \n\t"
            "vmul.u16    d21, d3, d0[3]              \n\t"
            "  vand        d19, d25                  \n\t"
            "vmla.u16    d8, d6, d1[0]               \n\t"
            "  vadd.i32    d2, d26, d18              \n\t"
            "vmla.u16    d9, d7, d1[1]               \n\t"
            "  vadd.i32    d3, d27, d19              \n\t"
            "vmla.u16    d20, d4, d1[2]              \n\t"
            "  vshl.i32    d0, d16, #2               \n\t"
            "vmla.u16    d21, d5, d1[3]              \n\t"
            "  vshl.i32    d1, d17, #2               \n\t"
            "  vand        q0, q12                   \n\t"
            "  vadd.i32    q2, q13, q0               \n\t"
            "  vmov        %[tmp0], s4               \n\t"
            "  vmov        %[tmp1], s5               \n\t"
            "vshrn.u16   d8, q4, #8                  \n\t"
            "vshrn.u16   d9, q10, #8                 \n\t"
            "vst1.32     {q4}, [%[dst]:128]!         \n\t"
            "b           11b                         \n\t"

            "12:                                     \n\t"
            "vmull.u8    q2, d5, d31                 \n\t"
            "vmlal.u8    q2, d9, d30                 \n\t"
            "vmul.u16    d8, d20, d0[0]              \n\t"
            "vmul.u16    d9, d21, d0[1]              \n\t"
            "vmul.u16    d20, d2, d0[2]              \n\t"
            "vmul.u16    d21, d3, d0[3]              \n\t"
            "vmla.u16    d8, d6, d1[0]               \n\t"
            "vmla.u16    d9, d7, d1[1]               \n\t"
            "vmla.u16    d20, d4, d1[2]              \n\t"
            "vmla.u16    d21, d5, d1[3]              \n\t"
            "vshrn.u16   d8, q4, #8                  \n\t"
            "vshrn.u16   d9, q10, #8                 \n\t"
            "vst1.32     {q4}, [%[dst]:128]!         \n\t"

            "13:                                     \n\t"
            "adds        %[len], #4-1                \n\t"
            "bmi         22f                         \n\t"

            "21:                                     \n\t"
            S32_OPAQUE_D32_FILTER_DX_1PIX_NEON("subs %[len], #1")
            "bpl         21b                         \n\t"

            "22:                                     \n\t"
    : // Outputs
             [dst]"+r"(colors),
              [xy]"+r"(xy),
             [len]"+r"(count),
            [tmp0]"=&r"(tmp0),
            [tmp1]"=&r"(tmp1),
            [tmp2]"=&r"(tmp2),
            [tmp3]"=&r"(tmp3),
               [x]"=&r"(x)
    : // Inputs
            [row0]"r"(row0),
            [row1]"r"(row1),
            [subY]"r"(wy)
    : // Clobbers
            "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q8", "q9", "q10", "q11", "q12", "q13", "q14"
    );
}

#endif

}  // namespace SK_OPTS_NS

namespace sktests {
    template <typename U32, typename Out>
    void decode_packed_coordinates_and_weight(U32 packed, Out* v0, Out* v1, Out* w) {
        SK_OPTS_NS::decode_packed_coordinates_and_weight<U32, Out>(packed, v0, v1, w);
    }
}

#endif
