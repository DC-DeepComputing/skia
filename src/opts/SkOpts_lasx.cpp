/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"

#define SK_OPTS_NS lasx
#include "src/opts/SkBitmapProcState_opts.h"
#include "src/opts/SkBlitRow_opts.h"
#include "src/opts/SkChecksum_opts.h"
#include "src/opts/SkSwizzler_opts.h"

namespace SkOpts {
    void Init_lasx() {
        hash_fn = lasx::hash_fn;
        blit_row_s32a_opaque = lasx::blit_row_s32a_opaque;
        RGBA_to_BGRA          = lasx::RGBA_to_BGRA;
        RGBA_to_rgbA          = lasx::RGBA_to_rgbA;
        RGBA_to_bgrA          = lasx::RGBA_to_bgrA;
        RGB_to_RGB1           = lasx::RGB_to_RGB1;
        RGB_to_BGR1           = lasx::RGB_to_BGR1;
        gray_to_RGB1          = lasx::gray_to_RGB1;
        grayA_to_RGBA         = lasx::grayA_to_RGBA;
        grayA_to_rgbA         = lasx::grayA_to_rgbA;
        inverted_CMYK_to_RGB1 = lasx::inverted_CMYK_to_RGB1;
        inverted_CMYK_to_BGR1 = lasx::inverted_CMYK_to_BGR1;

        S32_alpha_D32_filter_DX  = lasx::S32_alpha_D32_filter_DX;
    }
}  // namespace SkOpts
