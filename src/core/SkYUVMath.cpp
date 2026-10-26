/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkYUVMath.h"

#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/private/SkAssert.h"
#include "include/private/SkDebug.h"

#include <array>
#include <cstring>

// in SkColorMatrix order (row-major)
// Created by running SkColorMatrix_DumpYUVMatrixTables()
constexpr float Rec601_8bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.322d0ep-2f,    0x1.2c8b42p-1f,    0x1.d2f1aap-4f,           0x0p+0f,           0x0p+0f,
    -0x1.599234p-3f,   -0x1.5336e4p-2f,           0x1p-1f,           0x0p+0f,    0x1.010102p-1f,
            0x1p-1f,    -0x1.acbc7p-2f,   -0x1.4d0e3ep-4f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.299000f,         0.587000f,         0.114000f,         0.000000f,         0.000000f,
         -0.168736f,        -0.331264f,         0.500000f,         0.000000f,         0.501961f,
          0.500000f,        -0.418688f,        -0.081312f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec601_8bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,  -0x1.d5babap-27f,    0x1.66e978p+0f,           0x0p+0f,   -0x1.6851ccp-1f,
     0x1.000002p+0f,   -0x1.606544p-2f,   -0x1.6da346p-1f,           0x0p+0f,     0x1.0ffafp-1f,
            0x1p+0f,    0x1.c5a1cap+0f,    0x1.72004p-27f,           0x0p+0f,   -0x1.c76936p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -0.000000f,         1.402000f,         0.000000f,        -0.703749f,
          1.000000f,        -0.344136f,        -0.714136f,         0.000000f,         0.531211f,
          1.000000f,         1.772000f,         0.000000f,         0.000000f,        -0.889475f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec601_8bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.06f37ep-2f,    0x1.021d3ep-1f,    0x1.9105c2p-4f,           0x0p+0f,    0x1.010102p-4f,
    -0x1.2f8f7ep-3f,   -0x1.29fa02p-2f,    0x1.c1c1c2p-2f,           0x0p+0f,    0x1.010102p-1f,
     0x1.c1c1c2p-2f,   -0x1.789d7ep-2f,   -0x1.249108p-4f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.256788f,         0.504129f,         0.097906f,         0.000000f,         0.062745f,
         -0.148223f,        -0.290993f,         0.439216f,         0.000000f,         0.501961f,
          0.439216f,        -0.367788f,        -0.071427f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec601_8bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2a150cp+0f,  -0x1.eeec98p-31f,    0x1.989536p+0f,           0x0p+0f,    -0x1.bf977p-1f,
     0x1.2a150cp+0f,   -0x1.912a24p-2f,   -0x1.a03d4ep-1f,           0x0p+0f,    0x1.1036c6p-1f,
     0x1.2a150cp+0f,    0x1.0234aap+1f,   0x1.2de0b6p-26f,           0x0p+0f,   -0x1.15ebe8p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.164384f,        -0.000000f,         1.596027f,         0.000000f,        -0.874202f,
          1.164384f,        -0.391762f,        -0.812968f,         0.000000f,         0.531668f,
          1.164384f,         2.017232f,         0.000000f,         0.000000f,        -1.085631f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec601_10bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.322d0ep-2f,    0x1.2c8b42p-1f,    0x1.d2f1aap-4f,           0x0p+0f,           0x0p+0f,
    -0x1.599234p-3f,   -0x1.5336e4p-2f,           0x1p-1f,           0x0p+0f,     0x1.00401p-1f,
            0x1p-1f,    -0x1.acbc7p-2f,   -0x1.4d0e3ep-4f,           0x0p+0f,     0x1.00401p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.299000f,         0.587000f,         0.114000f,         0.000000f,         0.000000f,
         -0.168736f,        -0.331264f,         0.500000f,         0.000000f,         0.500489f,
          0.500000f,        -0.418688f,        -0.081312f,         0.000000f,         0.500489f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec601_10bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,  -0x1.d5babap-27f,    0x1.66e978p+0f,           0x0p+0f,   -0x1.67434ap-1f,
     0x1.000002p+0f,   -0x1.606544p-2f,   -0x1.6da346p-1f,           0x0p+0f,     0x1.0f2ecp-1f,
            0x1p+0f,    0x1.c5a1cap+0f,    0x1.72004p-27f,           0x0p+0f,    -0x1.c6135p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -0.000000f,         1.402000f,         0.000000f,        -0.701685f,
          1.000000f,        -0.344136f,        -0.714136f,         0.000000f,         0.529654f,
          1.000000f,         1.772000f,         0.000000f,         0.000000f,        -0.886866f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec601_10bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.062e16p-2f,    0x1.015b78p-1f,    0x1.8fd8b4p-4f,           0x0p+0f,     0x1.00401p-4f,
    -0x1.2eab98p-3f,   -0x1.291a4ep-2f,    0x1.c0701cp-2f,           0x0p+0f,     0x1.00401p-1f,
     0x1.c0701ep-2f,   -0x1.7782c4p-2f,   -0x1.23b566p-4f,           0x0p+0f,     0x1.00401p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.256035f,         0.502651f,         0.097619f,         0.000000f,         0.062561f,
         -0.147788f,        -0.290139f,         0.437928f,         0.000000f,         0.500489f,
          0.437928f,        -0.366710f,        -0.071218f,         0.000000f,         0.500489f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec601_10bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2af57cp+0f,  -0x1.0d4a26p-26f,    0x1.99c8d8p+0f,           0x0p+0f,   -0x1.bf976cp-1f,
     0x1.2af57cp+0f,   -0x1.925834p-2f,   -0x1.a176b2p-1f,           0x0p+0f,    0x1.1036c4p-1f,
     0x1.2af57ap+0f,    0x1.02f714p+1f,   0x1.391742p-27f,           0x0p+0f,   -0x1.15ebe6p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.167808f,        -0.000000f,         1.600721f,         0.000000f,        -0.874202f,
          1.167808f,        -0.392915f,        -0.815359f,         0.000000f,         0.531668f,
          1.167808f,         2.023165f,         0.000000f,         0.000000f,        -1.085631f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec601_12bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.322d0ep-2f,    0x1.2c8b42p-1f,    0x1.d2f1aap-4f,           0x0p+0f,           0x0p+0f,
    -0x1.599234p-3f,   -0x1.5336e4p-2f,           0x1p-1f,           0x0p+0f,    0x1.001002p-1f,
            0x1p-1f,    -0x1.acbc7p-2f,   -0x1.4d0e3ep-4f,           0x0p+0f,    0x1.001002p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.299000f,         0.587000f,         0.114000f,         0.000000f,         0.000000f,
         -0.168736f,        -0.331264f,         0.500000f,         0.000000f,         0.500122f,
          0.500000f,        -0.418688f,        -0.081312f,         0.000000f,         0.500122f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec601_12bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,  -0x1.d5babap-27f,    0x1.66e978p+0f,           0x0p+0f,   -0x1.66ffeap-1f,
     0x1.000002p+0f,   -0x1.606544p-2f,   -0x1.6da346p-1f,           0x0p+0f,    0x1.0efbe6p-1f,
            0x1p+0f,    0x1.c5a1cap+0f,    0x1.72004p-27f,           0x0p+0f,   -0x1.c5be28p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -0.000000f,         1.402000f,         0.000000f,        -0.701171f,
          1.000000f,        -0.344136f,        -0.714136f,         0.000000f,         0.529266f,
          1.000000f,         1.772000f,         0.000000f,         0.000000f,        -0.886216f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec601_12bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.05fceap-2f,    0x1.012b34p-1f,    0x1.8f8db6p-4f,           0x0p+0f,    0x1.001002p-4f,
    -0x1.2e72d6p-3f,   -0x1.28e296p-2f,    0x1.c01c02p-2f,           0x0p+0f,    0x1.001002p-1f,
     0x1.c01c02p-2f,   -0x1.773c54p-2f,   -0x1.237eaep-4f,           0x0p+0f,    0x1.001002p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.255848f,         0.502283f,         0.097547f,         0.000000f,         0.062515f,
         -0.147680f,        -0.289927f,         0.437607f,         0.000000f,         0.500122f,
          0.437607f,        -0.366441f,        -0.071166f,         0.000000f,         0.500122f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec601_12bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2b2d96p+0f,  -0x1.4714d8p-27f,    0x1.9a15c2p+0f,           0x0p+0f,    -0x1.bf977p-1f,
     0x1.2b2d98p+0f,   -0x1.92a3b6p-2f,   -0x1.a1c50ep-1f,           0x0p+0f,    0x1.1036c6p-1f,
     0x1.2b2d98p+0f,    0x1.0327aep+1f,     0x1.c93ep-25f,           0x0p+0f,   -0x1.15ebe8p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.168664f,        -0.000000f,         1.601894f,         0.000000f,        -0.874202f,
          1.168664f,        -0.393203f,        -0.815957f,         0.000000f,         0.531668f,
          1.168664f,         2.024648f,         0.000000f,         0.000000f,        -1.085631f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec601_16bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.322d0ep-2f,    0x1.2c8b42p-1f,    0x1.d2f1aap-4f,           0x0p+0f,           0x0p+0f,
    -0x1.599234p-3f,   -0x1.5336e4p-2f,           0x1p-1f,           0x0p+0f,      0x1.0001p-1f,
            0x1p-1f,    -0x1.acbc7p-2f,   -0x1.4d0e3ep-4f,           0x0p+0f,      0x1.0001p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.299000f,         0.587000f,         0.114000f,         0.000000f,         0.000000f,
         -0.168736f,        -0.331264f,         0.500000f,         0.000000f,         0.500008f,
          0.500000f,        -0.418688f,        -0.081312f,         0.000000f,         0.500008f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec601_16bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,  -0x1.d5babap-27f,    0x1.66e978p+0f,           0x0p+0f,    -0x1.66eaep-1f,
     0x1.000002p+0f,   -0x1.606544p-2f,   -0x1.6da346p-1f,           0x0p+0f,    0x1.0eec04p-1f,
            0x1p+0f,    0x1.c5a1cap+0f,    0x1.72004p-27f,           0x0p+0f,    -0x1.c5a39p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -0.000000f,         1.402000f,         0.000000f,        -0.701011f,
          1.000000f,        -0.344136f,        -0.714136f,         0.000000f,         0.529144f,
          1.000000f,         1.772000f,         0.000000f,         0.000000f,        -0.886014f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec601_16bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
      0x1.05ed9p-2f,    0x1.011c22p-1f,    0x1.8f764cp-4f,           0x0p+0f,      0x1.0001p-4f,
    -0x1.2e611cp-3f,    -0x1.28d13p-2f,     0x1.c001cp-2f,           0x0p+0f,      0x1.0001p-1f,
      0x1.c001cp-2f,   -0x1.772658p-2f,   -0x1.236d9ap-4f,           0x0p+0f,      0x1.0001p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.255789f,         0.502168f,         0.097525f,         0.000000f,         0.062501f,
         -0.147646f,        -0.289860f,         0.437507f,         0.000000f,         0.500008f,
          0.437507f,        -0.366357f,        -0.071149f,         0.000000f,         0.500008f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec601_16bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
      0x1.2b3f2p+0f,  -0x1.4db148p-27f,    0x1.9a2dccp+0f,           0x0p+0f,   -0x1.bf976ep-1f,
     0x1.2b3f22p+0f,    -0x1.92bb5p-2f,   -0x1.a1dd8ap-1f,           0x0p+0f,    0x1.1036c6p-1f,
      0x1.2b3f2p+0f,    0x1.0336dep+1f,   0x1.134dcep-25f,           0x0p+0f,   -0x1.15ebe6p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.168932f,        -0.000000f,         1.602261f,         0.000000f,        -0.874202f,
          1.168932f,        -0.393293f,        -0.816143f,         0.000000f,         0.531668f,
          1.168932f,         2.025112f,         0.000000f,         0.000000f,        -1.085631f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_8bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
      0x1.b367ap-3f,    0x1.6e2eb2p-1f,     0x1.27bb3p-4f,           0x0p+0f,           0x0p+0f,
    -0x1.d5498ep-4f,   -0x1.8aad9cp-2f,           0x1p-1f,           0x0p+0f,    0x1.010102p-1f,
     0x1.fffffep-2f,   -0x1.d10d74p-2f,    -0x1.77945p-5f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.212600f,         0.715200f,         0.072200f,         0.000000f,         0.000000f,
         -0.114572f,        -0.385428f,         0.500000f,         0.000000f,         0.501961f,
          0.500000f,        -0.454153f,        -0.045847f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_8bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,   -0x1.134d1p-29f,    0x1.93261ap+0f,           0x0p+0f,   -0x1.94bad6p-1f,
            0x1p+0f,   -0x1.7fa3dep-3f,   -0x1.df5bf8p-2f,           0x0p+0f,    0x1.50e7dep-2f,
            0x1p+0f,    0x1.db089ap+0f,  -0x1.1fd88ap-31f,           0x0p+0f,   -0x1.dce582p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -0.000000f,         1.574800f,         0.000000f,        -0.790488f,
          1.000000f,        -0.187324f,        -0.468124f,         0.000000f,         0.329010f,
          1.000000f,         1.855600f,        -0.000000f,         0.000000f,        -0.931439f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_8bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.75ef96p-3f,    0x1.3a7c6ep-1f,    0x1.fbf63ap-5f,           0x0p+0f,    0x1.010102p-4f,
    -0x1.9c3c9ap-4f,   -0x1.5ab29cp-2f,    0x1.c1c1c4p-2f,           0x0p+0f,    0x1.010102p-1f,
      0x1.c1c1cp-2f,   -0x1.98844ap-2f,   -0x1.49ebb2p-5f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.182586f,         0.614231f,         0.062007f,         0.000000f,         0.062745f,
         -0.100644f,        -0.338572f,         0.439216f,         0.000000f,         0.501961f,
          0.439216f,        -0.398942f,        -0.040274f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_8bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2a150cp+0f,  -0x1.532b74p-28f,    0x1.caf116p+0f,           0x0p+0f,   -0x1.f225dep-1f,
     0x1.2a150ap+0f,   -0x1.b4bbaep-3f,   -0x1.10d97ep-1f,           0x0p+0f,     0x1.34b7ep-2f,
     0x1.2a150ap+0f,    0x1.0e632ep+1f,  -0x1.65554ap-26f,           0x0p+0f,   -0x1.2226a6p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.164384f,        -0.000000f,         1.792741f,         0.000000f,        -0.972945f,
          1.164384f,        -0.213249f,        -0.532909f,         0.000000f,         0.301483f,
          1.164384f,         2.112402f,        -0.000000f,         0.000000f,        -1.133402f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_10bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
      0x1.b367ap-3f,    0x1.6e2eb2p-1f,     0x1.27bb3p-4f,           0x0p+0f,           0x0p+0f,
    -0x1.d5498ep-4f,   -0x1.8aad9cp-2f,           0x1p-1f,           0x0p+0f,     0x1.00401p-1f,
     0x1.fffffep-2f,   -0x1.d10d74p-2f,    -0x1.77945p-5f,           0x0p+0f,     0x1.00401p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.212600f,         0.715200f,         0.072200f,         0.000000f,         0.000000f,
         -0.114572f,        -0.385428f,         0.500000f,         0.000000f,         0.500489f,
          0.500000f,        -0.454153f,        -0.045847f,         0.000000f,         0.500489f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_10bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,   -0x1.134d1p-29f,    0x1.93261ap+0f,           0x0p+0f,   -0x1.938afcp-1f,
            0x1p+0f,   -0x1.7fa3dep-3f,   -0x1.df5bf8p-2f,           0x0p+0f,    0x1.4feaeep-2f,
            0x1p+0f,    0x1.db089ap+0f,  -0x1.1fd88ap-31f,           0x0p+0f,   -0x1.db7f7ap-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -0.000000f,         1.574800f,         0.000000f,        -0.788170f,
          1.000000f,        -0.187324f,        -0.468124f,         0.000000f,         0.328045f,
          1.000000f,         1.855600f,        -0.000000f,         0.000000f,        -0.928707f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_10bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.74d6dcp-3f,    0x1.399056p-1f,    0x1.fa78e4p-5f,           0x0p+0f,     0x1.00401p-4f,
    -0x1.9b071ep-4f,   -0x1.59ae54p-2f,    0x1.c0701cp-2f,           0x0p+0f,     0x1.00401p-1f,
     0x1.c0701ap-2f,   -0x1.97519ap-2f,   -0x1.48f404p-5f,           0x0p+0f,     0x1.00401p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.182050f,         0.612429f,         0.061825f,         0.000000f,         0.062561f,
         -0.100349f,        -0.337579f,         0.437928f,         0.000000f,         0.500489f,
          0.437928f,        -0.397772f,        -0.040155f,         0.000000f,         0.500489f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_10bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2af57cp+0f,  -0x1.a10fe6p-28f,    0x1.cc4aa4p+0f,           0x0p+0f,   -0x1.f225dep-1f,
     0x1.2af57ap+0f,   -0x1.b60488p-3f,    -0x1.11a6fp-1f,           0x0p+0f,     0x1.34b7ep-2f,
     0x1.2af57ap+0f,    0x1.0f2ec4p+1f,  -0x1.b49704p-27f,           0x0p+0f,   -0x1.2226a6p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.167808f,        -0.000000f,         1.798014f,         0.000000f,        -0.972945f,
          1.167808f,        -0.213876f,        -0.534477f,         0.000000f,         0.301483f,
          1.167808f,         2.118615f,        -0.000000f,         0.000000f,        -1.133402f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_12bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
      0x1.b367ap-3f,    0x1.6e2eb2p-1f,     0x1.27bb3p-4f,           0x0p+0f,           0x0p+0f,
    -0x1.d5498ep-4f,   -0x1.8aad9cp-2f,           0x1p-1f,           0x0p+0f,    0x1.001002p-1f,
     0x1.fffffep-2f,   -0x1.d10d74p-2f,    -0x1.77945p-5f,           0x0p+0f,    0x1.001002p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.212600f,         0.715200f,         0.072200f,         0.000000f,         0.000000f,
         -0.114572f,        -0.385428f,         0.500000f,         0.000000f,         0.500122f,
          0.500000f,        -0.454153f,        -0.045847f,         0.000000f,         0.500122f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_12bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,   -0x1.134d1p-29f,    0x1.93261ap+0f,           0x0p+0f,    -0x1.933f5p-1f,
            0x1p+0f,   -0x1.7fa3dep-3f,   -0x1.df5bf8p-2f,           0x0p+0f,     0x1.4fabfp-2f,
            0x1p+0f,    0x1.db089ap+0f,  -0x1.1fd88ap-31f,           0x0p+0f,   -0x1.db264ep-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -0.000000f,         1.574800f,         0.000000f,        -0.787592f,
          1.000000f,        -0.187324f,        -0.468124f,         0.000000f,         0.327804f,
          1.000000f,         1.855600f,        -0.000000f,         0.000000f,        -0.928027f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_12bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
      0x1.7490fp-3f,    0x1.395588p-1f,    0x1.fa19e6p-5f,           0x0p+0f,    0x1.001002p-4f,
    -0x1.9aba0ap-4f,    -0x1.596d8p-2f,    0x1.c01c02p-2f,           0x0p+0f,    0x1.001002p-1f,
       0x1.c01cp-2f,   -0x1.970536p-2f,   -0x1.48b652p-5f,           0x0p+0f,    0x1.001002p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.181917f,         0.611981f,         0.061780f,         0.000000f,         0.062515f,
         -0.100275f,        -0.337332f,         0.437607f,         0.000000f,         0.500122f,
          0.437607f,        -0.397481f,        -0.040126f,         0.000000f,         0.500122f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_12bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2b2d96p+0f,   0x1.6e8f86p-29f,    0x1.cca108p+0f,           0x0p+0f,    -0x1.f225ep-1f,
     0x1.2b2d96p+0f,   -0x1.b656bap-3f,   -0x1.11da4cp-1f,           0x0p+0f,    0x1.34b7e2p-2f,
     0x1.2b2d96p+0f,    0x1.0f61aap+1f,    0x1.30aaap-27f,           0x0p+0f,   -0x1.2226a6p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.168664f,         0.000000f,         1.799332f,         0.000000f,        -0.972945f,
          1.168664f,        -0.214033f,        -0.534869f,         0.000000f,         0.301483f,
          1.168664f,         2.120168f,         0.000000f,         0.000000f,        -1.133402f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_16bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
      0x1.b367ap-3f,    0x1.6e2eb2p-1f,     0x1.27bb3p-4f,           0x0p+0f,           0x0p+0f,
    -0x1.d5498ep-4f,   -0x1.8aad9cp-2f,           0x1p-1f,           0x0p+0f,      0x1.0001p-1f,
     0x1.fffffep-2f,   -0x1.d10d74p-2f,    -0x1.77945p-5f,           0x0p+0f,      0x1.0001p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.212600f,         0.715200f,         0.072200f,         0.000000f,         0.000000f,
         -0.114572f,        -0.385428f,         0.500000f,         0.000000f,         0.500008f,
          0.500000f,        -0.454153f,        -0.045847f,         0.000000f,         0.500008f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_16bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,   -0x1.134d1p-29f,    0x1.93261ap+0f,           0x0p+0f,   -0x1.9327acp-1f,
            0x1p+0f,   -0x1.7fa3dep-3f,   -0x1.df5bf8p-2f,           0x0p+0f,    0x1.4f9844p-2f,
            0x1p+0f,    0x1.db089ap+0f,  -0x1.1fd88ap-31f,           0x0p+0f,   -0x1.db0a76p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -0.000000f,         1.574800f,         0.000000f,        -0.787412f,
          1.000000f,        -0.187324f,        -0.468124f,         0.000000f,         0.327729f,
          1.000000f,         1.855600f,        -0.000000f,         0.000000f,        -0.927814f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_16bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.747b1ap-3f,    0x1.39432cp-1f,    0x1.f9fc3ep-5f,           0x0p+0f,      0x1.0001p-4f,
    -0x1.9aa1f8p-4f,   -0x1.595942p-2f,     0x1.c001cp-2f,           0x0p+0f,      0x1.0001p-1f,
     0x1.c001bep-2f,   -0x1.96ed5cp-2f,    -0x1.48a31p-5f,           0x0p+0f,      0x1.0001p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.181875f,         0.611841f,         0.061766f,         0.000000f,         0.062501f,
         -0.100252f,        -0.337255f,         0.437507f,         0.000000f,         0.500008f,
          0.437507f,        -0.397390f,        -0.040117f,         0.000000f,         0.500008f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_16bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
      0x1.2b3f2p+0f,   0x1.db0326p-27f,    0x1.ccbc08p+0f,           0x0p+0f,   -0x1.f225dep-1f,
      0x1.2b3f2p+0f,   -0x1.b6706cp-3f,   -0x1.11ea58p-1f,           0x0p+0f,     0x1.34b7ep-2f,
      0x1.2b3f2p+0f,    0x1.0f7192p+1f,   0x1.cf24bap-26f,           0x0p+0f,   -0x1.2226a6p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.168932f,         0.000000f,         1.799744f,         0.000000f,        -0.972945f,
          1.168932f,        -0.214082f,        -0.534991f,         0.000000f,         0.301483f,
          1.168932f,         2.120653f,         0.000000f,         0.000000f,        -1.133402f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_8bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.0d013ap-2f,    0x1.5b22d2p-1f,    0x1.e5c91ep-5f,           0x0p+0f,           0x0p+0f,
    -0x1.1df65cp-3f,   -0x1.7104d2p-2f,    0x1.fffffep-2f,           0x0p+0f,    0x1.010102p-1f,
            0x1p-1f,   -0x1.d6d212p-2f,   -0x1.496f7ep-5f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.262700f,         0.678000f,         0.059300f,         0.000000f,         0.000000f,
         -0.139630f,        -0.360370f,         0.500000f,         0.000000f,         0.501961f,
          0.500000f,        -0.459786f,        -0.040214f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_8bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,  -0x1.1b1682p-30f,    0x1.797f62p+0f,           0x0p+0f,   -0x1.7afa5ep-1f,
     0x1.fffffep-1f,   -0x1.51013cp-3f,   -0x1.248864p-1f,           0x0p+0f,    0x1.7a42f8p-2f,
            0x1p+0f,     0x1.e1a37p+0f,  -0x1.512cd4p-26f,           0x0p+0f,   -0x1.e386f8p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -0.000000f,         1.474600f,         0.000000f,        -0.740191f,
          1.000000f,        -0.164553f,        -0.571353f,         0.000000f,         0.369396f,
          1.000000f,         1.881400f,        -0.000000f,         0.000000f,        -0.944389f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_8bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.ce0e28p-3f,    0x1.2a20eap-1f,    0x1.a13442p-5f,           0x0p+0f,    0x1.010102p-4f,
    -0x1.f66588p-4f,    -0x1.44286p-2f,    0x1.c1c1c2p-2f,           0x0p+0f,    0x1.010102p-1f,
     0x1.c1c1c4p-2f,   -0x1.9d9566p-2f,   -0x1.2162f2p-5f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.225613f,         0.582282f,         0.050928f,         0.000000f,         0.062745f,
         -0.122655f,        -0.316560f,         0.439216f,         0.000000f,         0.501961f,
          0.439216f,        -0.403890f,        -0.035326f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_8bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2a150ap+0f,   -0x1.22eb3p-27f,    0x1.adbd94p+0f,           0x0p+0f,   -0x1.d4d50cp-1f,
     0x1.2a150ap+0f,   -0x1.7fa4d4p-3f,   -0x1.4d0468p-1f,           0x0p+0f,    0x1.63cc28p-2f,
     0x1.2a150ap+0f,    0x1.122598p+1f,  -0x1.73d38cp-26f,           0x0p+0f,   -0x1.25ecd6p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.164384f,        -0.000000f,         1.678674f,         0.000000f,        -0.915688f,
          1.164384f,        -0.187326f,        -0.650424f,         0.000000f,         0.347458f,
          1.164384f,         2.141772f,        -0.000000f,         0.000000f,        -1.148145f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_10bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.0d013ap-2f,    0x1.5b22d2p-1f,    0x1.e5c91ep-5f,           0x0p+0f,           0x0p+0f,
    -0x1.1df65cp-3f,   -0x1.7104d2p-2f,    0x1.fffffep-2f,           0x0p+0f,     0x1.00401p-1f,
            0x1p-1f,   -0x1.d6d212p-2f,   -0x1.496f7ep-5f,           0x0p+0f,     0x1.00401p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.262700f,         0.678000f,         0.059300f,         0.000000f,         0.000000f,
         -0.139630f,        -0.360370f,         0.500000f,         0.000000f,         0.500489f,
          0.500000f,        -0.459786f,        -0.040214f,         0.000000f,         0.500489f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_10bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,  -0x1.1b1682p-30f,    0x1.797f62p+0f,           0x0p+0f,   -0x1.79dddap-1f,
     0x1.fffffep-1f,   -0x1.51013cp-3f,   -0x1.248864p-1f,           0x0p+0f,    0x1.7926fcp-2f,
            0x1p+0f,     0x1.e1a37p+0f,  -0x1.512cd4p-26f,           0x0p+0f,   -0x1.e21bf6p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -0.000000f,         1.474600f,         0.000000f,        -0.738021f,
          1.000000f,        -0.164553f,        -0.571353f,         0.000000f,         0.368313f,
          1.000000f,         1.881400f,        -0.000000f,         0.000000f,        -0.941620f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_10bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.ccb346p-3f,    0x1.29411ap-1f,    0x1.9ffb0cp-5f,           0x0p+0f,     0x1.00401p-4f,
    -0x1.f4ec5cp-4f,   -0x1.433504p-2f,    0x1.c0701ap-2f,           0x0p+0f,     0x1.00401p-1f,
     0x1.c0701ep-2f,   -0x1.9c5ee8p-2f,    -0x1.2089bp-5f,           0x0p+0f,     0x1.00401p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.224951f,         0.580575f,         0.050779f,         0.000000f,         0.062561f,
         -0.122296f,        -0.315632f,         0.437928f,         0.000000f,         0.500489f,
          0.437928f,        -0.402706f,        -0.035222f,         0.000000f,         0.500489f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_10bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2af57ap+0f,  -0x1.641122p-27f,    0x1.af0126p+0f,           0x0p+0f,   -0x1.d4d50ap-1f,
     0x1.2af57ap+0f,   -0x1.80c5b2p-3f,   -0x1.4dff26p-1f,           0x0p+0f,    0x1.63cc26p-2f,
     0x1.2af57ap+0f,    0x1.12f404p+1f,  -0x1.23e582p-33f,           0x0p+0f,   -0x1.25ecd8p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.167808f,        -0.000000f,         1.683611f,         0.000000f,        -0.915688f,
          1.167808f,        -0.187877f,        -0.652337f,         0.000000f,         0.347458f,
          1.167808f,         2.148072f,        -0.000000f,         0.000000f,        -1.148145f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_12bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.0d013ap-2f,    0x1.5b22d2p-1f,    0x1.e5c91ep-5f,           0x0p+0f,           0x0p+0f,
    -0x1.1df65cp-3f,   -0x1.7104d2p-2f,    0x1.fffffep-2f,           0x0p+0f,    0x1.001002p-1f,
            0x1p-1f,   -0x1.d6d212p-2f,   -0x1.496f7ep-5f,           0x0p+0f,    0x1.001002p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.262700f,         0.678000f,         0.059300f,         0.000000f,         0.000000f,
         -0.139630f,        -0.360370f,         0.500000f,         0.000000f,         0.500122f,
          0.500000f,        -0.459786f,        -0.040214f,         0.000000f,         0.500122f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_12bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,  -0x1.1b1682p-30f,    0x1.797f62p+0f,           0x0p+0f,   -0x1.7996fep-1f,
     0x1.fffffep-1f,   -0x1.51013cp-3f,   -0x1.248864p-1f,           0x0p+0f,    0x1.78e042p-2f,
            0x1p+0f,     0x1.e1a37p+0f,  -0x1.512cd4p-26f,           0x0p+0f,   -0x1.e1c18ep-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -0.000000f,         1.474600f,         0.000000f,        -0.737480f,
          1.000000f,        -0.164553f,        -0.571353f,         0.000000f,         0.368043f,
          1.000000f,         1.881400f,        -0.000000f,         0.000000f,        -0.940930f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_12bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
      0x1.cc5cep-3f,    0x1.29095ap-1f,    0x1.9fad08p-5f,           0x0p+0f,    0x1.001002p-4f,
    -0x1.f48e6ap-4f,   -0x1.42f866p-2f,      0x1.c01cp-2f,           0x0p+0f,    0x1.001002p-1f,
     0x1.c01c04p-2f,   -0x1.9c1192p-2f,   -0x1.205394p-5f,           0x0p+0f,    0x1.001002p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.224787f,         0.580149f,         0.050742f,         0.000000f,         0.062515f,
         -0.122206f,        -0.315401f,         0.437607f,         0.000000f,         0.500122f,
          0.437607f,        -0.402411f,        -0.035196f,         0.000000f,         0.500122f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_12bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2b2d96p+0f,   -0x1.e313dp-28f,    0x1.af520ap+0f,           0x0p+0f,   -0x1.d4d50cp-1f,
     0x1.2b2d96p+0f,   -0x1.810deap-3f,   -0x1.4e3dd6p-1f,           0x0p+0f,    0x1.63cc28p-2f,
     0x1.2b2d96p+0f,    0x1.13279ep+1f,  -0x1.daca5ep-27f,           0x0p+0f,   -0x1.25ecd8p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.168664f,        -0.000000f,         1.684846f,         0.000000f,        -0.915688f,
          1.168664f,        -0.188015f,        -0.652816f,         0.000000f,         0.347458f,
          1.168664f,         2.149647f,        -0.000000f,         0.000000f,        -1.148145f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_16bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.0d013ap-2f,    0x1.5b22d2p-1f,    0x1.e5c91ep-5f,           0x0p+0f,           0x0p+0f,
    -0x1.1df65cp-3f,   -0x1.7104d2p-2f,    0x1.fffffep-2f,           0x0p+0f,      0x1.0001p-1f,
            0x1p-1f,   -0x1.d6d212p-2f,   -0x1.496f7ep-5f,           0x0p+0f,      0x1.0001p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.262700f,         0.678000f,         0.059300f,         0.000000f,         0.000000f,
         -0.139630f,        -0.360370f,         0.500000f,         0.000000f,         0.500008f,
          0.500000f,        -0.459786f,        -0.040214f,         0.000000f,         0.500008f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_16bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,  -0x1.1b1682p-30f,    0x1.797f62p+0f,           0x0p+0f,   -0x1.7980dcp-1f,
     0x1.fffffep-1f,   -0x1.51013cp-3f,   -0x1.248864p-1f,           0x0p+0f,    0x1.78ca2cp-2f,
            0x1p+0f,     0x1.e1a37p+0f,  -0x1.512cd4p-26f,           0x0p+0f,   -0x1.e1a552p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -0.000000f,         1.474600f,         0.000000f,        -0.737311f,
          1.000000f,        -0.164553f,        -0.571353f,         0.000000f,         0.367959f,
          1.000000f,         1.881400f,        -0.000000f,         0.000000f,        -0.940714f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_16bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.cc41e6p-3f,    0x1.28f7f2p-1f,    0x1.9f94acp-5f,           0x0p+0f,      0x1.0001p-4f,
    -0x1.f47116p-4f,   -0x1.42e57ap-2f,     0x1.c001cp-2f,           0x0p+0f,      0x1.0001p-1f,
      0x1.c001cp-2f,   -0x1.9bf96ap-2f,   -0x1.2042aep-5f,           0x0p+0f,      0x1.0001p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.224735f,         0.580017f,         0.050730f,         0.000000f,         0.062501f,
         -0.122178f,        -0.315329f,         0.437507f,         0.000000f,         0.500008f,
          0.437507f,        -0.402319f,        -0.035188f,         0.000000f,         0.500008f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_16bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
      0x1.2b3f2p+0f,   0x1.57718ep-31f,    0x1.af6b54p+0f,           0x0p+0f,   -0x1.d4d50cp-1f,
      0x1.2b3f2p+0f,   -0x1.81247ap-3f,   -0x1.4e516ep-1f,           0x0p+0f,    0x1.63cc28p-2f,
      0x1.2b3f2p+0f,    0x1.1337bep+1f,  -0x1.88ceb8p-26f,           0x0p+0f,   -0x1.25ecd6p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.168932f,         0.000000f,         1.685231f,         0.000000f,        -0.915688f,
          1.168932f,        -0.188058f,        -0.652965f,         0.000000f,         0.347458f,
          1.168932f,         2.150139f,        -0.000000f,         0.000000f,        -1.148145f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_8bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.333334p-2f,    0x1.2e147ap-1f,    0x1.c28f5cp-4f,           0x0p+0f,           0x0p+0f,
    -0x1.592b26p-3f,   -0x1.536a6cp-2f,    0x1.fffffep-2f,           0x0p+0f,    0x1.010102p-1f,
            0x1p-1f,   -0x1.af8af8p-2f,   -0x1.41d41ep-4f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.300000f,         0.590000f,         0.110000f,         0.000000f,         0.000000f,
         -0.168539f,        -0.331461f,         0.500000f,         0.000000f,         0.501961f,
          0.500000f,        -0.421429f,        -0.078571f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_8bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,   0x1.12f3e2p-28f,    0x1.666666p+0f,           0x0p+0f,   -0x1.67ce36p-1f,
            0x1p+0f,   -0x1.53d446p-2f,    -0x1.6c798p-1f,           0x0p+0f,     0x1.0c3e1p-1f,
            0x1p+0f,    0x1.c7ae16p+0f,   0x1.1bcdf2p-31f,           0x0p+0f,    -0x1.c9779p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,         0.000000f,         1.400000f,         0.000000f,        -0.702745f,
          1.000000f,        -0.331864f,        -0.711864f,         0.000000f,         0.523911f,
          1.000000f,         1.780000f,         0.000000f,         0.000000f,        -0.893490f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_8bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.07d4a2p-2f,    0x1.036ef4p-1f,    0x1.82f398p-4f,           0x0p+0f,    0x1.010102p-4f,
    -0x1.2f34f6p-3f,   -0x1.2a2746p-2f,    0x1.c1c1c2p-2f,           0x0p+0f,    0x1.010102p-1f,
     0x1.c1c1c2p-2f,   -0x1.7b14aep-2f,   -0x1.1ab44ep-4f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.257647f,         0.506706f,         0.094471f,         0.000000f,         0.062745f,
         -0.148050f,        -0.291165f,         0.439216f,         0.000000f,         0.501961f,
          0.439216f,        -0.370196f,        -0.069020f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_8bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2a150ap+0f,   -0x1.3a8c8p-29f,        0x1.98p+0f,           0x0p+0f,   -0x1.bf01a4p-1f,
     0x1.2a150ap+0f,   -0x1.82dbeep-2f,   -0x1.9eea4ep-1f,           0x0p+0f,    0x1.0bf578p-1f,
     0x1.2a150ap+0f,    0x1.035f16p+1f,   0x1.a8fbccp-27f,           0x0p+0f,    -0x1.17178p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.164384f,        -0.000000f,         1.593750f,         0.000000f,        -0.873059f,
          1.164384f,        -0.377792f,        -0.810381f,         0.000000f,         0.523357f,
          1.164384f,         2.026339f,         0.000000f,         0.000000f,        -1.090202f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_10bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.333334p-2f,    0x1.2e147ap-1f,    0x1.c28f5cp-4f,           0x0p+0f,           0x0p+0f,
    -0x1.592b26p-3f,   -0x1.536a6cp-2f,    0x1.fffffep-2f,           0x0p+0f,     0x1.00401p-1f,
            0x1p-1f,   -0x1.af8af8p-2f,   -0x1.41d41ep-4f,           0x0p+0f,     0x1.00401p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.300000f,         0.590000f,         0.110000f,         0.000000f,         0.000000f,
         -0.168539f,        -0.331461f,         0.500000f,         0.000000f,         0.500489f,
          0.500000f,        -0.421429f,        -0.078571f,         0.000000f,         0.500489f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_10bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,   0x1.12f3e2p-28f,    0x1.666666p+0f,           0x0p+0f,   -0x1.66c016p-1f,
            0x1p+0f,   -0x1.53d446p-2f,    -0x1.6c798p-1f,           0x0p+0f,    0x1.0b74aep-1f,
            0x1p+0f,    0x1.c7ae16p+0f,   0x1.1bcdf2p-31f,           0x0p+0f,   -0x1.c8201ep-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,         0.000000f,         1.400000f,         0.000000f,        -0.700684f,
          1.000000f,        -0.331864f,        -0.711864f,         0.000000f,         0.522375f,
          1.000000f,         1.780000f,         0.000000f,         0.000000f,        -0.890870f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_10bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.070e92p-2f,     0x1.02ac3p-1f,    0x1.81d118p-4f,           0x0p+0f,     0x1.00401p-4f,
    -0x1.2e5156p-3f,    -0x1.29477p-2f,    0x1.c0701ap-2f,           0x0p+0f,     0x1.00401p-1f,
     0x1.c0701cp-2f,   -0x1.79f816p-2f,   -0x1.19e012p-4f,           0x0p+0f,     0x1.00401p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.256892f,         0.505220f,         0.094194f,         0.000000f,         0.062561f,
         -0.147616f,        -0.290312f,         0.437928f,         0.000000f,         0.500489f,
          0.437928f,        -0.369110f,        -0.068817f,         0.000000f,         0.500489f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_10bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2af57ap+0f,   0x1.36a438p-26f,    0x1.993332p+0f,           0x0p+0f,   -0x1.bf01a2p-1f,
     0x1.2af57ap+0f,   -0x1.83ff38p-2f,   -0x1.a022b8p-1f,           0x0p+0f,    0x1.0bf578p-1f,
     0x1.2af57ap+0f,    0x1.042262p+1f,   0x1.959ceep-27f,           0x0p+0f,    -0x1.17178p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.167808f,         0.000000f,         1.598437f,         0.000000f,        -0.873059f,
          1.167808f,        -0.378903f,        -0.812765f,         0.000000f,         0.523357f,
          1.167808f,         2.032299f,         0.000000f,         0.000000f,        -1.090202f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_12bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.333334p-2f,    0x1.2e147ap-1f,    0x1.c28f5cp-4f,           0x0p+0f,           0x0p+0f,
    -0x1.592b26p-3f,   -0x1.536a6cp-2f,    0x1.fffffep-2f,           0x0p+0f,    0x1.001002p-1f,
            0x1p-1f,   -0x1.af8af8p-2f,   -0x1.41d41ep-4f,           0x0p+0f,    0x1.001002p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.300000f,         0.590000f,         0.110000f,         0.000000f,         0.000000f,
         -0.168539f,        -0.331461f,         0.500000f,         0.000000f,         0.500122f,
          0.500000f,        -0.421429f,        -0.078571f,         0.000000f,         0.500122f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_12bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,   0x1.12f3e2p-28f,    0x1.666666p+0f,           0x0p+0f,    -0x1.667cdp-1f,
            0x1p+0f,   -0x1.53d446p-2f,    -0x1.6c798p-1f,           0x0p+0f,    0x1.0b4286p-1f,
            0x1p+0f,    0x1.c7ae16p+0f,   0x1.1bcdf2p-31f,           0x0p+0f,   -0x1.c7ca94p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,         0.000000f,         1.400000f,         0.000000f,        -0.700171f,
          1.000000f,        -0.331864f,        -0.711864f,         0.000000f,         0.521992f,
          1.000000f,         1.780000f,         0.000000f,         0.000000f,        -0.890217f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_12bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.06dd3cp-2f,    0x1.027bacp-1f,    0x1.8188bcp-4f,           0x0p+0f,    0x1.001002p-4f,
    -0x1.2e18a4p-3f,    -0x1.290fbp-2f,    0x1.c01c02p-2f,           0x0p+0f,    0x1.001002p-1f,
     0x1.c01c02p-2f,   -0x1.79b134p-2f,   -0x1.19ab34p-4f,           0x0p+0f,    0x1.001002p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.256703f,         0.504850f,         0.094125f,         0.000000f,         0.062515f,
         -0.147508f,        -0.290099f,         0.437607f,         0.000000f,         0.500122f,
          0.437607f,        -0.368840f,        -0.068767f,         0.000000f,         0.500122f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_12bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2b2d96p+0f,  -0x1.6ee244p-28f,       0x1.998p+0f,           0x0p+0f,   -0x1.bf01a4p-1f,
     0x1.2b2d98p+0f,   -0x1.844808p-2f,   -0x1.a070d2p-1f,           0x0p+0f,    0x1.0bf578p-1f,
     0x1.2b2d96p+0f,    0x1.045334p+1f,  -0x1.4a556ep-28f,           0x0p+0f,    -0x1.17178p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.168664f,        -0.000000f,         1.599609f,         0.000000f,        -0.873059f,
          1.168664f,        -0.379181f,        -0.813361f,         0.000000f,         0.523357f,
          1.168664f,         2.033789f,        -0.000000f,         0.000000f,        -1.090202f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_16bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.333334p-2f,    0x1.2e147ap-1f,    0x1.c28f5cp-4f,           0x0p+0f,           0x0p+0f,
    -0x1.592b26p-3f,   -0x1.536a6cp-2f,    0x1.fffffep-2f,           0x0p+0f,      0x1.0001p-1f,
            0x1p-1f,   -0x1.af8af8p-2f,   -0x1.41d41ep-4f,           0x0p+0f,      0x1.0001p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.300000f,         0.590000f,         0.110000f,         0.000000f,         0.000000f,
         -0.168539f,        -0.331461f,         0.500000f,         0.000000f,         0.500008f,
          0.500000f,        -0.421429f,        -0.078571f,         0.000000f,         0.500008f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_16bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,   0x1.12f3e2p-28f,    0x1.666666p+0f,           0x0p+0f,   -0x1.6667ccp-1f,
            0x1p+0f,   -0x1.53d446p-2f,    -0x1.6c798p-1f,           0x0p+0f,    0x1.0b32dcp-1f,
            0x1p+0f,    0x1.c7ae16p+0f,   0x1.1bcdf2p-31f,           0x0p+0f,   -0x1.c7afdep-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,         0.000000f,         1.400000f,         0.000000f,        -0.700011f,
          1.000000f,        -0.331864f,        -0.711864f,         0.000000f,         0.521872f,
          1.000000f,         1.780000f,         0.000000f,         0.000000f,        -0.890014f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_16bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.06cdd4p-2f,    0x1.026c86p-1f,    0x1.817226p-4f,           0x0p+0f,      0x1.0001p-4f,
    -0x1.2e06eep-3f,   -0x1.28fe48p-2f,    0x1.c001bep-2f,           0x0p+0f,      0x1.0001p-1f,
      0x1.c001cp-2f,   -0x1.799b12p-2f,   -0x1.199ab4p-4f,           0x0p+0f,      0x1.0001p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.256645f,         0.504734f,         0.094103f,         0.000000f,         0.062501f,
         -0.147474f,        -0.290033f,         0.437507f,         0.000000f,         0.500008f,
          0.437507f,        -0.368756f,        -0.068751f,         0.000000f,         0.500008f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_16bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
      0x1.2b3f2p+0f,  -0x1.a850acp-28f,      0x1.9998p+0f,           0x0p+0f,   -0x1.bf01a2p-1f,
      0x1.2b3f2p+0f,   -0x1.845eccp-2f,   -0x1.a0893ap-1f,           0x0p+0f,    0x1.0bf578p-1f,
     0x1.2b3f22p+0f,    0x1.046276p+1f,  -0x1.ebdb12p-25f,           0x0p+0f,   -0x1.17177ep+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.168932f,        -0.000000f,         1.599976f,         0.000000f,        -0.873059f,
          1.168932f,        -0.379268f,        -0.813547f,         0.000000f,         0.523357f,
          1.168932f,         2.034255f,        -0.000000f,         0.000000f,        -1.090202f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_8bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.b22d0ep-3f,    0x1.66e978p-1f,    0x1.645a1cp-4f,           0x0p+0f,           0x0p+0f,
    -0x1.db8c7cp-4f,   -0x1.891ce2p-2f,           0x1p-1f,           0x0p+0f,    0x1.010102p-1f,
            0x1p-1f,   -0x1.c778dap-2f,   -0x1.c4392ep-5f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.212000f,         0.701000f,         0.087000f,         0.000000f,         0.000000f,
         -0.116101f,        -0.383899f,         0.500000f,         0.000000f,         0.501961f,
          0.500000f,        -0.444797f,        -0.055203f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_8bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,   0x1.d39c56p-29f,    0x1.9374bcp+0f,           0x0p+0f,   -0x1.9509c8p-1f,
            0x1p+0f,   -0x1.d01f2ep-3f,   -0x1.e80f98p-2f,           0x0p+0f,    0x1.697912p-2f,
            0x1p+0f,    0x1.d374bcp+0f,   0x1.ea3024p-29f,           0x0p+0f,   -0x1.d54a08p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,         0.000000f,         1.576000f,         0.000000f,        -0.791090f,
          1.000000f,        -0.226622f,        -0.476622f,         0.000000f,         0.353001f,
          1.000000f,         1.826000f,         0.000000f,         0.000000f,        -0.916580f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_8bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.74e16cp-3f,    0x1.343df8p-1f,    0x1.320b22p-4f,           0x0p+0f,    0x1.010102p-4f,
    -0x1.a1bcaap-4f,   -0x1.595298p-2f,    0x1.c1c1c4p-2f,           0x0p+0f,    0x1.010102p-1f,
     0x1.c1c1c2p-2f,   -0x1.9019d8p-2f,   -0x1.8d3f46p-5f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.182071f,         0.602035f,         0.074718f,         0.000000f,         0.062745f,
         -0.101987f,        -0.337229f,         0.439216f,         0.000000f,         0.501961f,
          0.439216f,        -0.390724f,        -0.048492f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_8bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2a150ap+0f,   -0x1.03331p-27f,    0x1.cb4a9cp+0f,           0x0p+0f,   -0x1.f27fbep-1f,
     0x1.2a150cp+0f,   -0x1.082d2cp-2f,   -0x1.15cd72p-1f,           0x0p+0f,    0x1.50af76p-2f,
     0x1.2a150ap+0f,    0x1.0a1304p+1f,   0x1.2b37d6p-25f,           0x0p+0f,   -0x1.1dd228p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.164384f,        -0.000000f,         1.794107f,         0.000000f,        -0.973631f,
          1.164384f,        -0.257985f,        -0.542583f,         0.000000f,         0.328794f,
          1.164384f,         2.078705f,         0.000000f,         0.000000f,        -1.116488f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_10bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.b22d0ep-3f,    0x1.66e978p-1f,    0x1.645a1cp-4f,           0x0p+0f,           0x0p+0f,
    -0x1.db8c7cp-4f,   -0x1.891ce2p-2f,           0x1p-1f,           0x0p+0f,     0x1.00401p-1f,
            0x1p-1f,   -0x1.c778dap-2f,   -0x1.c4392ep-5f,           0x0p+0f,     0x1.00401p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.212000f,         0.701000f,         0.087000f,         0.000000f,         0.000000f,
         -0.116101f,        -0.383899f,         0.500000f,         0.000000f,         0.500489f,
          0.500000f,        -0.444797f,        -0.055203f,         0.000000f,         0.500489f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_10bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,   0x1.d39c56p-29f,    0x1.9374bcp+0f,           0x0p+0f,   -0x1.93d9b2p-1f,
            0x1p+0f,   -0x1.d01f2ep-3f,   -0x1.e80f98p-2f,           0x0p+0f,    0x1.6869b2p-2f,
            0x1p+0f,    0x1.d374bcp+0f,   0x1.ea3024p-29f,           0x0p+0f,   -0x1.d3e9b6p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,         0.000000f,         1.576000f,         0.000000f,        -0.788770f,
          1.000000f,        -0.226622f,        -0.476622f,         0.000000f,         0.351966f,
          1.000000f,         1.826000f,         0.000000f,         0.000000f,        -0.913892f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_10bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.73c97ep-3f,     0x1.33569p-1f,     0x1.31256p-4f,           0x0p+0f,     0x1.00401p-4f,
    -0x1.a0830cp-4f,   -0x1.584f5ap-2f,    0x1.c0701cp-2f,           0x0p+0f,     0x1.00401p-1f,
     0x1.c0701cp-2f,   -0x1.8eed7ap-2f,   -0x1.8c150cp-5f,           0x0p+0f,     0x1.00401p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.181537f,         0.600270f,         0.074499f,         0.000000f,         0.062561f,
         -0.101687f,        -0.336240f,         0.437928f,         0.000000f,         0.500489f,
          0.437928f,        -0.389578f,        -0.048350f,         0.000000f,         0.500489f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_10bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2af57ap+0f,  -0x1.95ee5cp-28f,    0x1.cca46cp+0f,           0x0p+0f,   -0x1.f27fbcp-1f,
     0x1.2af57ap+0f,   -0x1.08f416p-2f,   -0x1.169e9ep-1f,           0x0p+0f,    0x1.50af76p-2f,
     0x1.2af57cp+0f,    0x1.0adb5ap+1f,  -0x1.2b2ceap-26f,           0x0p+0f,   -0x1.1dd228p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.167808f,        -0.000000f,         1.799384f,         0.000000f,        -0.973631f,
          1.167808f,        -0.258744f,        -0.544179f,         0.000000f,         0.328794f,
          1.167808f,         2.084819f,        -0.000000f,         0.000000f,        -1.116488f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_12bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.b22d0ep-3f,    0x1.66e978p-1f,    0x1.645a1cp-4f,           0x0p+0f,           0x0p+0f,
    -0x1.db8c7cp-4f,   -0x1.891ce2p-2f,           0x1p-1f,           0x0p+0f,    0x1.001002p-1f,
            0x1p-1f,   -0x1.c778dap-2f,   -0x1.c4392ep-5f,           0x0p+0f,    0x1.001002p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.212000f,         0.701000f,         0.087000f,         0.000000f,         0.000000f,
         -0.116101f,        -0.383899f,         0.500000f,         0.000000f,         0.500122f,
          0.500000f,        -0.444797f,        -0.055203f,         0.000000f,         0.500122f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_12bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,   0x1.d39c56p-29f,    0x1.9374bcp+0f,           0x0p+0f,   -0x1.938df6p-1f,
            0x1p+0f,   -0x1.d01f2ep-3f,   -0x1.e80f98p-2f,           0x0p+0f,    0x1.68261cp-2f,
            0x1p+0f,    0x1.d374bcp+0f,   0x1.ea3024p-29f,           0x0p+0f,   -0x1.d391f8p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,         0.000000f,         1.576000f,         0.000000f,        -0.788192f,
          1.000000f,        -0.226622f,        -0.476622f,         0.000000f,         0.351708f,
          1.000000f,         1.826000f,         0.000000f,         0.000000f,        -0.913223f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_12bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.7383c4p-3f,    0x1.331cecp-1f,    0x1.30ec24p-4f,           0x0p+0f,    0x1.001002p-4f,
    -0x1.a034eep-4f,   -0x1.580ec6p-2f,    0x1.c01c02p-2f,           0x0p+0f,    0x1.001002p-1f,
     0x1.c01c02p-2f,   -0x1.8ea2a8p-2f,   -0x1.8bcac4p-5f,           0x0p+0f,    0x1.001002p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.181404f,         0.599830f,         0.074444f,         0.000000f,         0.062515f,
         -0.101613f,        -0.335994f,         0.437607f,         0.000000f,         0.500122f,
          0.437607f,        -0.389292f,        -0.048314f,         0.000000f,         0.500122f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_12bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2b2d96p+0f,   0x1.0361f4p-26f,    0x1.ccfae2p+0f,           0x0p+0f,   -0x1.f27fbep-1f,
     0x1.2b2d98p+0f,    -0x1.0925dp-2f,   -0x1.16d2e8p-1f,           0x0p+0f,    0x1.50af76p-2f,
     0x1.2b2d96p+0f,     0x1.0b0d7p+1f,  -0x1.9d1214p-27f,           0x0p+0f,   -0x1.1dd228p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.168664f,         0.000000f,         1.800703f,         0.000000f,        -0.973631f,
          1.168664f,        -0.258933f,        -0.544578f,         0.000000f,         0.328794f,
          1.168664f,         2.086348f,        -0.000000f,         0.000000f,        -1.116488f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_16bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.b22d0ep-3f,    0x1.66e978p-1f,    0x1.645a1cp-4f,           0x0p+0f,           0x0p+0f,
    -0x1.db8c7cp-4f,   -0x1.891ce2p-2f,           0x1p-1f,           0x0p+0f,      0x1.0001p-1f,
            0x1p-1f,   -0x1.c778dap-2f,   -0x1.c4392ep-5f,           0x0p+0f,      0x1.0001p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.212000f,         0.701000f,         0.087000f,         0.000000f,         0.000000f,
         -0.116101f,        -0.383899f,         0.500000f,         0.000000f,         0.500008f,
          0.500000f,        -0.444797f,        -0.055203f,         0.000000f,         0.500008f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_16bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,   0x1.d39c56p-29f,    0x1.9374bcp+0f,           0x0p+0f,    -0x1.93765p-1f,
            0x1p+0f,   -0x1.d01f2ep-3f,   -0x1.e80f98p-2f,           0x0p+0f,      0x1.6811p-2f,
            0x1p+0f,    0x1.d374bcp+0f,   0x1.ea3024p-29f,           0x0p+0f,    -0x1.d3769p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,         0.000000f,         1.576000f,         0.000000f,        -0.788012f,
          1.000000f,        -0.226622f,        -0.476622f,         0.000000f,         0.351627f,
          1.000000f,         1.826000f,         0.000000f,         0.000000f,        -0.913014f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_16bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.736dfep-3f,    0x1.330aecp-1f,    0x1.30da46p-4f,           0x0p+0f,      0x1.0001p-4f,
    -0x1.a01c8cp-4f,   -0x1.57fa9ep-2f,     0x1.c001cp-2f,           0x0p+0f,      0x1.0001p-1f,
      0x1.c001cp-2f,   -0x1.8e8b4ep-2f,   -0x1.8bb394p-5f,           0x0p+0f,      0x1.0001p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.181362f,         0.599693f,         0.074427f,         0.000000f,         0.062501f,
         -0.101590f,        -0.335917f,         0.437507f,         0.000000f,         0.500008f,
          0.437507f,        -0.389203f,        -0.048303f,         0.000000f,         0.500008f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_16bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2b3f22p+0f,   0x1.73763cp-32f,    0x1.cd15e6p+0f,           0x0p+0f,   -0x1.f27fbcp-1f,
      0x1.2b3f2p+0f,   -0x1.09355ap-2f,    -0x1.16e34p-1f,           0x0p+0f,    0x1.50af76p-2f,
     0x1.2b3f22p+0f,    0x1.0b1d18p+1f,  -0x1.125244p-26f,           0x0p+0f,   -0x1.1dd228p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.168932f,         0.000000f,         1.801115f,         0.000000f,        -0.973631f,
          1.168932f,        -0.258993f,        -0.544703f,         0.000000f,         0.328794f,
          1.168932f,         2.086825f,        -0.000000f,         0.000000f,        -1.116488f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_8bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,          -0x1p-1f,    0x1.f91f2ep-2f,           0x0p+0f,    0x1.010102p-1f,
            0x1p-1f,   -0x1.fbda94p-2f,           0x0p+0f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         1.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,        -0.500000f,         0.493283f,         0.000000f,         0.501961f,
          0.500000f,        -0.495951f,         0.000000f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_8bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.fbda94p-1f,          -0x0p+0f,           0x1p+1f,           0x0p+0f,   -0x1.010102p+0f,
            0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
     0x1.037c66p+0f,    0x1.037c66p+1f,           0x0p+0f,           0x0p+0f,   -0x1.0480e8p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.991902f,        -0.000000f,         2.000000f,         0.000000f,        -1.003922f,
          1.000000f,         0.000000f,         0.000000f,         0.000000f,         0.000000f,
          1.013617f,         2.027234f,         0.000000f,         0.000000f,        -1.017592f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_8bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,    0x1.b7b7b8p-1f,           0x0p+0f,           0x0p+0f,    0x1.010102p-4f,
            0x0p+0f,   -0x1.c1c1c2p-2f,      0x1.bbb7p-2f,           0x0p+0f,    0x1.010102p-1f,
     0x1.c1c1c2p-2f,    -0x1.be1d6p-2f,           0x0p+0f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.858824f,         0.000000f,         0.000000f,         0.062745f,
          0.000000f,        -0.439216f,         0.433315f,         0.000000f,         0.501961f,
          0.439216f,        -0.435659f,         0.000000f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_8bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.27ab18p+0f,          -0x0p+0f,    0x1.236db6p+1f,           0x0p+0f,   -0x1.371f88p+0f,
     0x1.2a150ap+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,   -0x1.2b404cp-4f,
     0x1.2e2422p+0f,    0x1.27659cp+1f,           0x0p+0f,           0x0p+0f,   -0x1.3b8364p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.154954f,        -0.000000f,         2.276786f,         0.000000f,        -1.215325f,
          1.164384f,         0.000000f,         0.000000f,         0.000000f,        -0.073059f,
          1.180239f,         2.307788f,         0.000000f,         0.000000f,        -1.232474f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_10bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,          -0x1p-1f,    0x1.f91f2ep-2f,           0x0p+0f,     0x1.00401p-1f,
            0x1p-1f,   -0x1.fbda94p-2f,           0x0p+0f,           0x0p+0f,     0x1.00401p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         1.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,        -0.500000f,         0.493283f,         0.000000f,         0.500489f,
          0.500000f,        -0.495951f,         0.000000f,         0.000000f,         0.500489f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_10bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.fbda94p-1f,          -0x0p+0f,           0x1p+1f,           0x0p+0f,    -0x1.00401p+0f,
            0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
     0x1.037c66p+0f,    0x1.037c66p+1f,           0x0p+0f,           0x0p+0f,   -0x1.03bd56p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.991902f,        -0.000000f,         2.000000f,         0.000000f,        -1.000978f,
          1.000000f,         0.000000f,         0.000000f,         0.000000f,         0.000000f,
          1.013617f,         2.027234f,         0.000000f,         0.000000f,        -1.014608f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_10bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,    0x1.b66d9cp-1f,           0x0p+0f,           0x0p+0f,     0x1.00401p-4f,
            0x0p+0f,   -0x1.c0701cp-2f,    0x1.ba69e2p-2f,           0x0p+0f,     0x1.00401p-1f,
     0x1.c0701cp-2f,   -0x1.bcce76p-2f,           0x0p+0f,           0x0p+0f,     0x1.00401p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.856305f,         0.000000f,         0.000000f,         0.062561f,
          0.000000f,        -0.437928f,         0.432045f,         0.000000f,         0.500489f,
          0.437928f,        -0.434381f,         0.000000f,         0.000000f,         0.500489f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_10bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2889b6p+0f,          -0x0p+0f,    0x1.244924p+1f,           0x0p+0f,   -0x1.371f88p+0f,
     0x1.2af57ap+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,   -0x1.2b404ap-4f,
     0x1.2f07a2p+0f,    0x1.284408p+1f,           0x0p+0f,           0x0p+0f,   -0x1.3b8364p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.158351f,        -0.000000f,         2.283482f,         0.000000f,        -1.215325f,
          1.167808f,         0.000000f,         0.000000f,         0.000000f,        -0.073059f,
          1.183710f,         2.314576f,         0.000000f,         0.000000f,        -1.232474f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_12bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,          -0x1p-1f,    0x1.f91f2ep-2f,           0x0p+0f,    0x1.001002p-1f,
            0x1p-1f,   -0x1.fbda94p-2f,           0x0p+0f,           0x0p+0f,    0x1.001002p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         1.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,        -0.500000f,         0.493283f,         0.000000f,         0.500122f,
          0.500000f,        -0.495951f,         0.000000f,         0.000000f,         0.500122f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_12bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.fbda94p-1f,          -0x0p+0f,           0x1p+1f,           0x0p+0f,   -0x1.001002p+0f,
            0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
     0x1.037c66p+0f,    0x1.037c66p+1f,           0x0p+0f,           0x0p+0f,    -0x1.038cap+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.991902f,        -0.000000f,         2.000000f,         0.000000f,        -1.000244f,
          1.000000f,         0.000000f,         0.000000f,         0.000000f,         0.000000f,
          1.013617f,         2.027234f,         0.000000f,         0.000000f,        -1.013865f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_12bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,    0x1.b61b62p-1f,           0x0p+0f,           0x0p+0f,    0x1.001002p-4f,
            0x0p+0f,   -0x1.c01c02p-2f,    0x1.ba16eap-2f,           0x0p+0f,    0x1.001002p-1f,
     0x1.c01c02p-2f,   -0x1.bc7b0ap-2f,           0x0p+0f,           0x0p+0f,    0x1.001002p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.855678f,         0.000000f,         0.000000f,         0.062515f,
          0.000000f,        -0.437607f,         0.431728f,         0.000000f,         0.500122f,
          0.437607f,        -0.434063f,         0.000000f,         0.000000f,         0.500122f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_12bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.28c15ep+0f,          -0x0p+0f,       0x1.248p+1f,           0x0p+0f,   -0x1.371f88p+0f,
     0x1.2b2d96p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,   -0x1.2b404cp-4f,
      0x1.2f408p+0f,    0x1.287ba2p+1f,           0x0p+0f,           0x0p+0f,   -0x1.3b8364p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.159201f,        -0.000000f,         2.285156f,         0.000000f,        -1.215325f,
          1.168664f,         0.000000f,         0.000000f,         0.000000f,        -0.073059f,
          1.184578f,         2.316273f,         0.000000f,         0.000000f,        -1.232474f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_16bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,          -0x1p-1f,    0x1.f91f2ep-2f,           0x0p+0f,      0x1.0001p-1f,
            0x1p-1f,   -0x1.fbda94p-2f,           0x0p+0f,           0x0p+0f,      0x1.0001p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         1.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,        -0.500000f,         0.493283f,         0.000000f,         0.500008f,
          0.500000f,        -0.495951f,         0.000000f,         0.000000f,         0.500008f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_16bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.fbda94p-1f,          -0x0p+0f,           0x1p+1f,           0x0p+0f,     -0x1.0001p+0f,
            0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
     0x1.037c66p+0f,    0x1.037c66p+1f,           0x0p+0f,           0x0p+0f,   -0x1.037d6ap+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.991902f,        -0.000000f,         2.000000f,         0.000000f,        -1.000015f,
          1.000000f,         0.000000f,         0.000000f,         0.000000f,         0.000000f,
          1.013617f,         2.027234f,         0.000000f,         0.000000f,        -1.013632f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_16bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,    0x1.b601b6p-1f,           0x0p+0f,           0x0p+0f,      0x1.0001p-4f,
            0x0p+0f,    -0x1.c001cp-2f,    0x1.b9fd02p-2f,           0x0p+0f,      0x1.0001p-1f,
      0x1.c001cp-2f,   -0x1.bc60fep-2f,           0x0p+0f,           0x0p+0f,      0x1.0001p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.855482f,         0.000000f,         0.000000f,         0.062501f,
          0.000000f,        -0.437507f,         0.431629f,         0.000000f,         0.500008f,
          0.437507f,        -0.433964f,         0.000000f,         0.000000f,         0.500008f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_16bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.28d2c2p+0f,          -0x0p+0f,    0x1.249124p+1f,           0x0p+0f,   -0x1.371f88p+0f,
      0x1.2b3f2p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,   -0x1.2b404ap-4f,
     0x1.2f5246p+0f,    0x1.288d04p+1f,           0x0p+0f,           0x0p+0f,   -0x1.3b8364p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.159466f,        -0.000000f,         2.285679f,         0.000000f,        -1.215325f,
          1.168932f,         0.000000f,         0.000000f,         0.000000f,        -0.073059f,
          1.184849f,         2.316803f,         0.000000f,         0.000000f,        -1.232474f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_8bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,
            0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         1.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,         0.000000f,         1.000000f,         0.000000f,         0.000000f,
          1.000000f,         0.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_8bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,
            0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,          -0x0p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.000000f,         1.000000f,         0.000000f,         0.000000f,
          1.000000f,         0.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,         1.000000f,         0.000000f,         0.000000f,        -0.000000f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_8bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,    0x1.b7b7b8p-1f,           0x0p+0f,           0x0p+0f,    0x1.010102p-4f,
            0x0p+0f,           0x0p+0f,    0x1.b7b7b8p-1f,           0x0p+0f,    0x1.010102p-4f,
     0x1.b7b7b8p-1f,           0x0p+0f,           0x0p+0f,           0x0p+0f,    0x1.010102p-4f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.858824f,         0.000000f,         0.000000f,         0.062745f,
          0.000000f,         0.000000f,         0.858824f,         0.000000f,         0.062745f,
          0.858824f,         0.000000f,         0.000000f,         0.000000f,         0.062745f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_8bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x0p+0f,    0x1.2a150ap+0f,           0x0p+0f,   -0x1.2b404cp-4f,
     0x1.2a150ap+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,   -0x1.2b404cp-4f,
            0x0p+0f,    0x1.2a150ap+0f,           0x0p+0f,           0x0p+0f,   -0x1.2b404cp-4f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.000000f,         1.164384f,         0.000000f,        -0.073059f,
          1.164384f,         0.000000f,         0.000000f,         0.000000f,        -0.073059f,
          0.000000f,         1.164384f,         0.000000f,         0.000000f,        -0.073059f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_10bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,
            0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         1.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,         0.000000f,         1.000000f,         0.000000f,         0.000000f,
          1.000000f,         0.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_10bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,
            0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,          -0x0p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.000000f,         1.000000f,         0.000000f,         0.000000f,
          1.000000f,         0.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,         1.000000f,         0.000000f,         0.000000f,        -0.000000f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_10bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,    0x1.b66d9cp-1f,           0x0p+0f,           0x0p+0f,     0x1.00401p-4f,
            0x0p+0f,           0x0p+0f,    0x1.b66d9cp-1f,           0x0p+0f,     0x1.00401p-4f,
     0x1.b66d9cp-1f,           0x0p+0f,           0x0p+0f,           0x0p+0f,     0x1.00401p-4f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.856305f,         0.000000f,         0.000000f,         0.062561f,
          0.000000f,         0.000000f,         0.856305f,         0.000000f,         0.062561f,
          0.856305f,         0.000000f,         0.000000f,         0.000000f,         0.062561f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_10bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x0p+0f,    0x1.2af57ap+0f,           0x0p+0f,   -0x1.2b404ap-4f,
     0x1.2af57ap+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,   -0x1.2b404ap-4f,
            0x0p+0f,    0x1.2af57ap+0f,           0x0p+0f,           0x0p+0f,   -0x1.2b404ap-4f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.000000f,         1.167808f,         0.000000f,        -0.073059f,
          1.167808f,         0.000000f,         0.000000f,         0.000000f,        -0.073059f,
          0.000000f,         1.167808f,         0.000000f,         0.000000f,        -0.073059f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_12bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,
            0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         1.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,         0.000000f,         1.000000f,         0.000000f,         0.000000f,
          1.000000f,         0.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_12bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,
            0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,          -0x0p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.000000f,         1.000000f,         0.000000f,         0.000000f,
          1.000000f,         0.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,         1.000000f,         0.000000f,         0.000000f,        -0.000000f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_12bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,    0x1.b61b62p-1f,           0x0p+0f,           0x0p+0f,    0x1.001002p-4f,
            0x0p+0f,           0x0p+0f,    0x1.b61b62p-1f,           0x0p+0f,    0x1.001002p-4f,
     0x1.b61b62p-1f,           0x0p+0f,           0x0p+0f,           0x0p+0f,    0x1.001002p-4f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.855678f,         0.000000f,         0.000000f,         0.062515f,
          0.000000f,         0.000000f,         0.855678f,         0.000000f,         0.062515f,
          0.855678f,         0.000000f,         0.000000f,         0.000000f,         0.062515f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_12bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x0p+0f,    0x1.2b2d96p+0f,           0x0p+0f,   -0x1.2b404cp-4f,
     0x1.2b2d96p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,   -0x1.2b404cp-4f,
            0x0p+0f,    0x1.2b2d96p+0f,           0x0p+0f,           0x0p+0f,   -0x1.2b404cp-4f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.000000f,         1.168664f,         0.000000f,        -0.073059f,
          1.168664f,         0.000000f,         0.000000f,         0.000000f,        -0.073059f,
          0.000000f,         1.168664f,         0.000000f,         0.000000f,        -0.073059f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_16bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,
            0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         1.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,         0.000000f,         1.000000f,         0.000000f,         0.000000f,
          1.000000f,         0.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_16bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,
            0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,          -0x0p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.000000f,         1.000000f,         0.000000f,         0.000000f,
          1.000000f,         0.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,         1.000000f,         0.000000f,         0.000000f,        -0.000000f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_16bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,    0x1.b601b6p-1f,           0x0p+0f,           0x0p+0f,      0x1.0001p-4f,
            0x0p+0f,           0x0p+0f,    0x1.b601b6p-1f,           0x0p+0f,      0x1.0001p-4f,
     0x1.b601b6p-1f,           0x0p+0f,           0x0p+0f,           0x0p+0f,      0x1.0001p-4f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.855482f,         0.000000f,         0.000000f,         0.062501f,
          0.000000f,         0.000000f,         0.855482f,         0.000000f,         0.062501f,
          0.855482f,         0.000000f,         0.000000f,         0.000000f,         0.062501f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_16bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x0p+0f,     0x1.2b3f2p+0f,           0x0p+0f,   -0x1.2b404ap-4f,
      0x1.2b3f2p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,   -0x1.2b404ap-4f,
            0x0p+0f,     0x1.2b3f2p+0f,           0x0p+0f,           0x0p+0f,   -0x1.2b404ap-4f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.000000f,         1.168932f,         0.000000f,        -0.073059f,
          1.168932f,         0.000000f,         0.000000f,         0.000000f,        -0.073059f,
          0.000000f,         1.168932f,         0.000000f,         0.000000f,        -0.073059f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_8bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p-2f,           0x1p-1f,           0x1p-2f,           0x0p+0f,           0x0p+0f,
           -0x1p-2f,           0x1p-1f,          -0x1p-2f,           0x0p+0f,    0x1.010102p-1f,
            0x1p-1f,           0x0p+0f,          -0x1p-1f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.250000f,         0.500000f,         0.250000f,         0.000000f,         0.000000f,
         -0.250000f,         0.500000f,        -0.250000f,         0.000000f,         0.501961f,
          0.500000f,         0.000000f,        -0.500000f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_8bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,          -0x1p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,
            0x1p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,   -0x1.010102p-1f,
            0x1p+0f,          -0x1p+0f,          -0x1p+0f,           0x0p+0f,    0x1.010102p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -1.000000f,         1.000000f,         0.000000f,         0.000000f,
          1.000000f,         1.000000f,         0.000000f,         0.000000f,        -0.501961f,
          1.000000f,        -1.000000f,        -1.000000f,         0.000000f,         1.003922f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_8bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.b7b7b8p-3f,    0x1.b7b7b8p-2f,    0x1.b7b7b8p-3f,           0x0p+0f,    0x1.010102p-4f,
    -0x1.b7b7b8p-3f,    0x1.b7b7b8p-2f,   -0x1.b7b7b8p-3f,           0x0p+0f,    0x1.010102p-1f,
     0x1.b7b7b8p-2f,           0x0p+0f,   -0x1.b7b7b8p-2f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.214706f,         0.429412f,         0.214706f,         0.000000f,         0.062745f,
         -0.214706f,         0.429412f,        -0.214706f,         0.000000f,         0.501961f,
          0.429412f,         0.000000f,        -0.429412f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_8bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2a150ap+0f,   -0x1.2a150ap+0f,    0x1.2a150ap+0f,           0x0p+0f,   -0x1.2b404cp-4f,
     0x1.2a150ap+0f,    0x1.2a150ap+0f,           0x0p+0f,           0x0p+0f,   -0x1.50a856p-1f,
     0x1.2a150ap+0f,   -0x1.2a150ap+0f,   -0x1.2a150ap+0f,           0x0p+0f,    0x1.188c48p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.164384f,        -1.164384f,         1.164384f,         0.000000f,        -0.073059f,
          1.164384f,         1.164384f,         0.000000f,         0.000000f,        -0.657534f,
          1.164384f,        -1.164384f,        -1.164384f,         0.000000f,         1.095891f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_10bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p-2f,           0x1p-1f,           0x1p-2f,           0x0p+0f,           0x0p+0f,
           -0x1p-2f,           0x1p-1f,          -0x1p-2f,           0x0p+0f,     0x1.00401p-1f,
            0x1p-1f,           0x0p+0f,          -0x1p-1f,           0x0p+0f,     0x1.00401p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.250000f,         0.500000f,         0.250000f,         0.000000f,         0.000000f,
         -0.250000f,         0.500000f,        -0.250000f,         0.000000f,         0.500489f,
          0.500000f,         0.000000f,        -0.500000f,         0.000000f,         0.500489f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_10bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,          -0x1p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,
            0x1p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,    -0x1.00401p-1f,
            0x1p+0f,          -0x1p+0f,          -0x1p+0f,           0x0p+0f,     0x1.00401p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -1.000000f,         1.000000f,         0.000000f,         0.000000f,
          1.000000f,         1.000000f,         0.000000f,         0.000000f,        -0.500489f,
          1.000000f,        -1.000000f,        -1.000000f,         0.000000f,         1.000978f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_10bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.b66d9cp-3f,    0x1.b66d9cp-2f,    0x1.b66d9cp-3f,           0x0p+0f,     0x1.00401p-4f,
    -0x1.b66d9cp-3f,    0x1.b66d9cp-2f,   -0x1.b66d9cp-3f,           0x0p+0f,     0x1.00401p-1f,
     0x1.b66d9cp-2f,           0x0p+0f,   -0x1.b66d9cp-2f,           0x0p+0f,     0x1.00401p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.214076f,         0.428153f,         0.214076f,         0.000000f,         0.062561f,
         -0.214076f,         0.428153f,        -0.214076f,         0.000000f,         0.500489f,
          0.428153f,         0.000000f,        -0.428153f,         0.000000f,         0.500489f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_10bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2af57ap+0f,   -0x1.2af57ap+0f,    0x1.2af57ap+0f,           0x0p+0f,   -0x1.2b404ap-4f,
     0x1.2af57ap+0f,    0x1.2af57ap+0f,           0x0p+0f,           0x0p+0f,   -0x1.50a854p-1f,
     0x1.2af57ap+0f,   -0x1.2af57ap+0f,   -0x1.2af57ap+0f,           0x0p+0f,    0x1.188c46p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.167808f,        -1.167808f,         1.167808f,         0.000000f,        -0.073059f,
          1.167808f,         1.167808f,         0.000000f,         0.000000f,        -0.657534f,
          1.167808f,        -1.167808f,        -1.167808f,         0.000000f,         1.095890f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_12bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p-2f,           0x1p-1f,           0x1p-2f,           0x0p+0f,           0x0p+0f,
           -0x1p-2f,           0x1p-1f,          -0x1p-2f,           0x0p+0f,    0x1.001002p-1f,
            0x1p-1f,           0x0p+0f,          -0x1p-1f,           0x0p+0f,    0x1.001002p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.250000f,         0.500000f,         0.250000f,         0.000000f,         0.000000f,
         -0.250000f,         0.500000f,        -0.250000f,         0.000000f,         0.500122f,
          0.500000f,         0.000000f,        -0.500000f,         0.000000f,         0.500122f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_12bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,          -0x1p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,
            0x1p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,   -0x1.001002p-1f,
            0x1p+0f,          -0x1p+0f,          -0x1p+0f,           0x0p+0f,    0x1.001002p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -1.000000f,         1.000000f,         0.000000f,         0.000000f,
          1.000000f,         1.000000f,         0.000000f,         0.000000f,        -0.500122f,
          1.000000f,        -1.000000f,        -1.000000f,         0.000000f,         1.000244f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_12bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.b61b62p-3f,    0x1.b61b62p-2f,    0x1.b61b62p-3f,           0x0p+0f,    0x1.001002p-4f,
    -0x1.b61b62p-3f,    0x1.b61b62p-2f,   -0x1.b61b62p-3f,           0x0p+0f,    0x1.001002p-1f,
     0x1.b61b62p-2f,           0x0p+0f,   -0x1.b61b62p-2f,           0x0p+0f,    0x1.001002p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.213919f,         0.427839f,         0.213919f,         0.000000f,         0.062515f,
         -0.213919f,         0.427839f,        -0.213919f,         0.000000f,         0.500122f,
          0.427839f,         0.000000f,        -0.427839f,         0.000000f,         0.500122f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_12bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2b2d96p+0f,   -0x1.2b2d96p+0f,    0x1.2b2d96p+0f,           0x0p+0f,   -0x1.2b404cp-4f,
     0x1.2b2d96p+0f,    0x1.2b2d96p+0f,           0x0p+0f,           0x0p+0f,   -0x1.50a856p-1f,
     0x1.2b2d96p+0f,   -0x1.2b2d96p+0f,   -0x1.2b2d96p+0f,           0x0p+0f,    0x1.188c48p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.168664f,        -1.168664f,         1.168664f,         0.000000f,        -0.073059f,
          1.168664f,         1.168664f,         0.000000f,         0.000000f,        -0.657534f,
          1.168664f,        -1.168664f,        -1.168664f,         0.000000f,         1.095891f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_16bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p-2f,           0x1p-1f,           0x1p-2f,           0x0p+0f,           0x0p+0f,
           -0x1p-2f,           0x1p-1f,          -0x1p-2f,           0x0p+0f,      0x1.0001p-1f,
            0x1p-1f,           0x0p+0f,          -0x1p-1f,           0x0p+0f,      0x1.0001p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.250000f,         0.500000f,         0.250000f,         0.000000f,         0.000000f,
         -0.250000f,         0.500000f,        -0.250000f,         0.000000f,         0.500008f,
          0.500000f,         0.000000f,        -0.500000f,         0.000000f,         0.500008f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_16bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,          -0x1p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,
            0x1p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,     -0x1.0001p-1f,
            0x1p+0f,          -0x1p+0f,          -0x1p+0f,           0x0p+0f,      0x1.0001p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -1.000000f,         1.000000f,         0.000000f,         0.000000f,
          1.000000f,         1.000000f,         0.000000f,         0.000000f,        -0.500008f,
          1.000000f,        -1.000000f,        -1.000000f,         0.000000f,         1.000015f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_16bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.b601b6p-3f,    0x1.b601b6p-2f,    0x1.b601b6p-3f,           0x0p+0f,      0x1.0001p-4f,
    -0x1.b601b6p-3f,    0x1.b601b6p-2f,   -0x1.b601b6p-3f,           0x0p+0f,      0x1.0001p-1f,
     0x1.b601b6p-2f,           0x0p+0f,   -0x1.b601b6p-2f,           0x0p+0f,      0x1.0001p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.213870f,         0.427741f,         0.213870f,         0.000000f,         0.062501f,
         -0.213870f,         0.427741f,        -0.213870f,         0.000000f,         0.500008f,
          0.427741f,         0.000000f,        -0.427741f,         0.000000f,         0.500008f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_16bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
      0x1.2b3f2p+0f,    -0x1.2b3f2p+0f,     0x1.2b3f2p+0f,           0x0p+0f,   -0x1.2b404ap-4f,
      0x1.2b3f2p+0f,     0x1.2b3f2p+0f,           0x0p+0f,           0x0p+0f,   -0x1.50a854p-1f,
      0x1.2b3f2p+0f,    -0x1.2b3f2p+0f,    -0x1.2b3f2p+0f,           0x0p+0f,    0x1.188c46p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.168932f,        -1.168932f,         1.168932f,         0.000000f,        -0.073059f,
          1.168932f,         1.168932f,         0.000000f,         0.000000f,        -0.657534f,
          1.168932f,        -1.168932f,        -1.168932f,         0.000000f,         1.095890f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};

static constexpr auto yuv_to_rgb_array = std::to_array<const float *>({
    Rec601_8bit_full_yuv_to_rgb,
    Rec601_8bit_limited_yuv_to_rgb,
    Rec601_10bit_full_yuv_to_rgb,
    Rec601_10bit_limited_yuv_to_rgb,
    Rec601_12bit_full_yuv_to_rgb,
    Rec601_12bit_limited_yuv_to_rgb,
    Rec601_16bit_full_yuv_to_rgb,
    Rec601_16bit_limited_yuv_to_rgb,
    Rec709_8bit_full_yuv_to_rgb,
    Rec709_8bit_limited_yuv_to_rgb,
    Rec709_10bit_full_yuv_to_rgb,
    Rec709_10bit_limited_yuv_to_rgb,
    Rec709_12bit_full_yuv_to_rgb,
    Rec709_12bit_limited_yuv_to_rgb,
    Rec709_16bit_full_yuv_to_rgb,
    Rec709_16bit_limited_yuv_to_rgb,
    BT2020_8bit_full_yuv_to_rgb,
    BT2020_8bit_limited_yuv_to_rgb,
    BT2020_10bit_full_yuv_to_rgb,
    BT2020_10bit_limited_yuv_to_rgb,
    BT2020_12bit_full_yuv_to_rgb,
    BT2020_12bit_limited_yuv_to_rgb,
    BT2020_16bit_full_yuv_to_rgb,
    BT2020_16bit_limited_yuv_to_rgb,
    FCC_8bit_full_yuv_to_rgb,
    FCC_8bit_limited_yuv_to_rgb,
    FCC_10bit_full_yuv_to_rgb,
    FCC_10bit_limited_yuv_to_rgb,
    FCC_12bit_full_yuv_to_rgb,
    FCC_12bit_limited_yuv_to_rgb,
    FCC_16bit_full_yuv_to_rgb,
    FCC_16bit_limited_yuv_to_rgb,
    SMPTE240_8bit_full_yuv_to_rgb,
    SMPTE240_8bit_limited_yuv_to_rgb,
    SMPTE240_10bit_full_yuv_to_rgb,
    SMPTE240_10bit_limited_yuv_to_rgb,
    SMPTE240_12bit_full_yuv_to_rgb,
    SMPTE240_12bit_limited_yuv_to_rgb,
    SMPTE240_16bit_full_yuv_to_rgb,
    SMPTE240_16bit_limited_yuv_to_rgb,
    YDZDX_8bit_full_yuv_to_rgb,
    YDZDX_8bit_limited_yuv_to_rgb,
    YDZDX_10bit_full_yuv_to_rgb,
    YDZDX_10bit_limited_yuv_to_rgb,
    YDZDX_12bit_full_yuv_to_rgb,
    YDZDX_12bit_limited_yuv_to_rgb,
    YDZDX_16bit_full_yuv_to_rgb,
    YDZDX_16bit_limited_yuv_to_rgb,
    GBR_8bit_full_yuv_to_rgb,
    GBR_8bit_limited_yuv_to_rgb,
    GBR_10bit_full_yuv_to_rgb,
    GBR_10bit_limited_yuv_to_rgb,
    GBR_12bit_full_yuv_to_rgb,
    GBR_12bit_limited_yuv_to_rgb,
    GBR_16bit_full_yuv_to_rgb,
    GBR_16bit_limited_yuv_to_rgb,
    YCgCo_8bit_full_yuv_to_rgb,
    YCgCo_8bit_limited_yuv_to_rgb,
    YCgCo_10bit_full_yuv_to_rgb,
    YCgCo_10bit_limited_yuv_to_rgb,
    YCgCo_12bit_full_yuv_to_rgb,
    YCgCo_12bit_limited_yuv_to_rgb,
    YCgCo_16bit_full_yuv_to_rgb,
    YCgCo_16bit_limited_yuv_to_rgb,
});

static constexpr auto rgb_to_yuv_array = std::to_array<const float *>({
    Rec601_8bit_full_rgb_to_yuv,
    Rec601_8bit_limited_rgb_to_yuv,
    Rec601_10bit_full_rgb_to_yuv,
    Rec601_10bit_limited_rgb_to_yuv,
    Rec601_12bit_full_rgb_to_yuv,
    Rec601_12bit_limited_rgb_to_yuv,
    Rec601_16bit_full_rgb_to_yuv,
    Rec601_16bit_limited_rgb_to_yuv,
    Rec709_8bit_full_rgb_to_yuv,
    Rec709_8bit_limited_rgb_to_yuv,
    Rec709_10bit_full_rgb_to_yuv,
    Rec709_10bit_limited_rgb_to_yuv,
    Rec709_12bit_full_rgb_to_yuv,
    Rec709_12bit_limited_rgb_to_yuv,
    Rec709_16bit_full_rgb_to_yuv,
    Rec709_16bit_limited_rgb_to_yuv,
    BT2020_8bit_full_rgb_to_yuv,
    BT2020_8bit_limited_rgb_to_yuv,
    BT2020_10bit_full_rgb_to_yuv,
    BT2020_10bit_limited_rgb_to_yuv,
    BT2020_12bit_full_rgb_to_yuv,
    BT2020_12bit_limited_rgb_to_yuv,
    BT2020_16bit_full_rgb_to_yuv,
    BT2020_16bit_limited_rgb_to_yuv,
    FCC_8bit_full_rgb_to_yuv,
    FCC_8bit_limited_rgb_to_yuv,
    FCC_10bit_full_rgb_to_yuv,
    FCC_10bit_limited_rgb_to_yuv,
    FCC_12bit_full_rgb_to_yuv,
    FCC_12bit_limited_rgb_to_yuv,
    FCC_16bit_full_rgb_to_yuv,
    FCC_16bit_limited_rgb_to_yuv,
    SMPTE240_8bit_full_rgb_to_yuv,
    SMPTE240_8bit_limited_rgb_to_yuv,
    SMPTE240_10bit_full_rgb_to_yuv,
    SMPTE240_10bit_limited_rgb_to_yuv,
    SMPTE240_12bit_full_rgb_to_yuv,
    SMPTE240_12bit_limited_rgb_to_yuv,
    SMPTE240_16bit_full_rgb_to_yuv,
    SMPTE240_16bit_limited_rgb_to_yuv,
    YDZDX_8bit_full_rgb_to_yuv,
    YDZDX_8bit_limited_rgb_to_yuv,
    YDZDX_10bit_full_rgb_to_yuv,
    YDZDX_10bit_limited_rgb_to_yuv,
    YDZDX_12bit_full_rgb_to_yuv,
    YDZDX_12bit_limited_rgb_to_yuv,
    YDZDX_16bit_full_rgb_to_yuv,
    YDZDX_16bit_limited_rgb_to_yuv,
    GBR_8bit_full_rgb_to_yuv,
    GBR_8bit_limited_rgb_to_yuv,
    GBR_10bit_full_rgb_to_yuv,
    GBR_10bit_limited_rgb_to_yuv,
    GBR_12bit_full_rgb_to_yuv,
    GBR_12bit_limited_rgb_to_yuv,
    GBR_16bit_full_rgb_to_yuv,
    GBR_16bit_limited_rgb_to_yuv,
    YCgCo_8bit_full_rgb_to_yuv,
    YCgCo_8bit_limited_rgb_to_yuv,
    YCgCo_10bit_full_rgb_to_yuv,
    YCgCo_10bit_limited_rgb_to_yuv,
    YCgCo_12bit_full_rgb_to_yuv,
    YCgCo_12bit_limited_rgb_to_yuv,
    YCgCo_16bit_full_rgb_to_yuv,
    YCgCo_16bit_limited_rgb_to_yuv,
});

constexpr size_t kSizeOfColorMatrix = 20 * sizeof(float);

void SkColorMatrix_RGB2YUV(SkYUVColorSpace cs, float m[20]) {
    if ((unsigned)cs < (unsigned)kIdentity_SkYUVColorSpace) {
        memcpy(m, rgb_to_yuv_array[(unsigned)cs], kSizeOfColorMatrix);
    } else {
        memset(m, 0, kSizeOfColorMatrix);
        m[0] = m[6] = m[12] = m[18] = 1;
    }
}

void SkColorMatrix_YUV2RGB(SkYUVColorSpace cs, float m[20]) {
    if ((unsigned)cs < (unsigned)kIdentity_SkYUVColorSpace) {
        memcpy(m, yuv_to_rgb_array[(unsigned)cs], kSizeOfColorMatrix);
    } else {
        memset(m, 0, kSizeOfColorMatrix);
        m[0] = m[6] = m[12] = m[18] = 1;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// we just drop the alpha rol/col from the colormatrix
// output is |        tr |
//           |  3x3   tg |
//           |        tb |
//           | 0 0 0  1  |
static void colormatrix_to_matrix44(const float src[20], SkM44* dst) {
    *dst = SkM44(src[ 0], src[ 1], src[ 2], src[ 4],
                 src[ 5], src[ 6], src[ 7], src[ 9],
                 src[10], src[11], src[12], src[14],
                       0,       0,       0,       1);
}

// input: ignore the bottom row
// output: inject identity row/column for alpha
static void matrix44_to_colormatrix(const SkM44& src, float dst[20]) {
    dst[0] = src.rc(0,0);
    dst[1] = src.rc(0,1);
    dst[2] = src.rc(0,2);
    dst[3] = 0;
    dst[4] = src.rc(0,3);    // tx

    dst[5] = src.rc(1,0);
    dst[6] = src.rc(1,1);
    dst[7] = src.rc(1,2);
    dst[8] = 0;
    dst[9] = src.rc(1,3);    // ty

    dst[10] = src.rc(2,0);
    dst[11] = src.rc(2,1);
    dst[12] = src.rc(2,2);
    dst[13] = 0;
    dst[14] = src.rc(2,3);   // tz

    dst[15] = dst[16] = dst[17] = dst[19] = 0;
    dst[18] = 1;
}

static void scale3(float m[], float s) {
    for (int i = 0; i < 3; ++i) {
        m[i] *= s;
    }
}

namespace {
enum class Range { kFull, kLimited };

enum class FormatType {
    kYCbCr,
    kYDZDX,
    kGBR,
    kYCgCo,
};

struct FormatInfo {
    const char* name;
    FormatType type;
    float Kr = 0.0f;
    float Kb = 0.0f;
};

static constexpr auto kFormats = std::to_array<FormatInfo>({
    {"Rec601",   FormatType::kYCbCr, 0.2990f, 0.1140f},
    {"Rec709",   FormatType::kYCbCr, 0.2126f, 0.0722f},
    {"BT2020",   FormatType::kYCbCr, 0.2627f, 0.0593f},
    {"FCC",      FormatType::kYCbCr, 0.3000f, 0.1100f},
    {"SMPTE240", FormatType::kYCbCr, 0.2120f, 0.0870f},
    {"YDZDX",    FormatType::kYDZDX},
    {"GBR",      FormatType::kGBR},
    {"YCgCo",    FormatType::kYCgCo},
});

static constexpr auto kBitDepths = std::to_array<int>({8, 10, 12, 16});

struct RangeInfo {
    Range range;
    const char* name;
};

static constexpr auto kRanges = std::to_array<RangeInfo>({
    {Range::kFull, "full"},
    {Range::kLimited, "limited"},
});
}  // namespace

static void make_rgb_to_yuv_matrix_ycbcr(float mx[20], float Kr, float Kb, int bits, Range range) {
    SkASSERT(bits >= 8);
    const float Kg = 1.0f - Kr - Kb;
    const float Cr = 0.5f / (1.0f - Kb);
    const float Cb = 0.5f / (1.0f - Kr);

    const int shift = bits - 8;

    const float denom = (1 << bits) - 1;
    float scaleY  = 1.0f,
          addY    = 0.0f,
          scaleUV = 1.0f,
          addUV   = (128 << shift) / denom;

    if (range == Range::kLimited) {
        scaleY  = (219 << shift) / denom;
        addY    = ( 16 << shift) / denom;
        scaleUV = (224 << shift) / denom;
    }

    float m[20] = {
          Kr,  Kg,   Kb,  0,  addY,
         -Kr, -Kg, 1-Kb,  0, addUV,
        1-Kr, -Kg,  -Kb,  0, addUV,
           0,   0,    0,  1,     0,
    };
    memcpy(mx, m, sizeof(m));
    scale3(mx +  0,      scaleY );
    scale3(mx +  5, Cr * scaleUV);
    scale3(mx + 10, Cb * scaleUV);
}

static void make_rgb_to_yuv_matrix_ydzdx(float mx[20], int bits, Range range) {
    SkASSERT(bits >= 8);
    const int shift = bits - 8;
    const float denom = (1 << bits) - 1;
    float scaleY  = 1.0f,
          addY    = 0.0f,
          scaleUV = 1.0f,
          addUV   = (128 << shift) / denom;

    if (range == Range::kLimited) {
        scaleY  = (219 << shift) / denom;
        addY    = ( 16 << shift) / denom;
        scaleUV = (224 << shift) / denom;
    }

    // YDZDX applies range correction to YUV values similar to YCbCr.
    float m[20] = {
          0.0f,              1.0f,             0.0f, 0.0f,  addY,  // Y
          0.0f,             -0.5f, 0.986566f / 2.0f, 0.0f, addUV,  // DX or DZ
          0.5f, -0.991902f / 2.0f,             0.0f, 0.0f, addUV,  // DZ or DX
          0.0f,              0.0f,             0.0f, 1.0f,  0.0f,
    };
    memcpy(mx, m, sizeof(m));
    scale3(mx +  0, scaleY );
    scale3(mx +  5, scaleUV);
    scale3(mx + 10, scaleUV);
}

static void make_rgb_to_yuv_matrix_gbr(float mx[20], int bits, Range range) {
    SkASSERT(bits >= 8);
    const int shift = bits - 8;
    const float denom = (1 << bits) - 1;
    float scaleY   = 1.0f,
          addY     = 0.0f;

    if (range == Range::kLimited) {
        scaleY  = (219 << shift) / denom;
        addY    = ( 16 << shift) / denom;
    }

    // GBR applies range correction to RGB values similar to YCgCo.
    float m[20] = {
          0.0f, 1.0f, 0.0f, 0.0f,  addY, // G
          0.0f, 0.0f, 1.0f, 0.0f,  addY, // B
          1.0f, 0.0f, 0.0f, 0.0f,  addY, // R
          0.0f, 0.0f, 0.0f, 1.0f,  0.0f,
    };
    memcpy(mx, m, sizeof(m));
    scale3(mx +  0, scaleY);
    scale3(mx +  5, scaleY);
    scale3(mx + 10, scaleY);
}

static void make_rgb_to_yuv_matrix_ycgco(float mx[20], int bits, Range range) {
    SkASSERT(bits >= 8);
    const int shift = bits - 8;
    const float denom = (1 << bits) - 1;
    float scaleY   = 1.0f,
          addY     = 0.0f,
          chroma05 = (1 << (bits - 1)) / denom;

    if (range == Range::kLimited) {
        scaleY  = (219 << shift) / denom;
        addY    = ( 16 << shift) / denom;
    }

    float m[20] = {
          0.25f,  0.5f,   0.25f,  0.0f,  addY,
         -0.25f,  0.5f,  -0.25f,  0.0f,  chroma05,
           0.5f,  0.0f,  -0.5f,   0.0f,  chroma05,
           0.0f,  0.0f,   0.0f,   1.0f,  0.0f,
    };
    memcpy(mx, m, sizeof(m));
    scale3(mx +  0, scaleY);
    scale3(mx +  5, scaleY);
    scale3(mx + 10, scaleY);
}

static void make_rgb_to_yuv_matrix(float mx[20], const FormatInfo& format, int bits, Range range) {
    switch (format.type) {
        case FormatType::kYCbCr:
            return make_rgb_to_yuv_matrix_ycbcr(mx, format.Kr, format.Kb, bits, range);
        case FormatType::kYDZDX:
            return make_rgb_to_yuv_matrix_ydzdx(mx, bits, range);
        case FormatType::kGBR:
            return make_rgb_to_yuv_matrix_gbr(mx, bits, range);
        case FormatType::kYCgCo:
            return make_rgb_to_yuv_matrix_ycgco(mx, bits, range);
    }
}

static void dump(const float m[20], const char* name, int bits, const char* range_name, bool rgb2yuv) {
    const char* dirname = rgb2yuv ? "rgb_to_yuv" : "yuv_to_rgb";
    SkDebugf("constexpr float %s_%dbit_%s_%s[] = {\n", name, bits, range_name, dirname);
    SkDebugf("#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)\n");
    for (int i = 0; i < 4; ++i) {
        SkDebugf(" ");
        for (int j = 0; j < 5; ++j) {
            const float v = m[i * 5 + j];
            SkDebugf(" %16af,", v);
        }
        SkDebugf("\n");
    }
    SkDebugf("#else\n");
    for (int i = 0; i < 4; ++i) {
        SkDebugf(" ");
        for (int j = 0; j < 5; ++j) {
            SkDebugf(" %16.6ff,", m[i * 5 + j]);
        }
        SkDebugf("\n");
    }
    SkDebugf("#endif\n");
    SkDebugf("};\n");
}

// Used to create the prebuilt tables for each colorspace.
// Don't remove this function, in case we want to recompute those tables in the future.
void SkColorMatrix_DumpYUVMatrixTables() {
    for (const auto& format : kFormats) {
        for (int bits : kBitDepths) {
            for (const auto& r : kRanges) {
                float m[20];
                make_rgb_to_yuv_matrix(m, format, bits, r.range);
                dump(m, format.name, bits, r.name, /*rgb2yuv=*/true);
                SkM44 m44, im44;
                colormatrix_to_matrix44(m, &m44);
                float im[20];
#ifdef SK_DEBUG
                // be sure our conversion between matrix44 and colormatrix is perfect
                matrix44_to_colormatrix(m44, im);
                SkASSERT(memcmp(m, im, sizeof(im)) == 0);
#endif
                SkAssertResult(m44.invert(&im44));
                matrix44_to_colormatrix(im44, im);
                dump(im, format.name, bits, r.name, /*rgb2yuv=*/false);
            }
        }
    }
}
