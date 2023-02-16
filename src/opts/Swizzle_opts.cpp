/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// include order matters here. This goes first to set the namespace SK_OPTS_NS that the
// opts implementations will be set in.
#include "src/opts/SkOpts_namespace.h"
// Then this creates the implementations in that namespace using the proper features/settings.
#include "src/opts/Swizzle_opts_impl.h"
// Finally, this declares the function pointers in SkOpts that we can override here.
#include "src/opts/Swizzle_opts.h"

namespace SkOpts {
    DEFINE_DEFAULT(RGBA_to_BGRA);
    DEFINE_DEFAULT(RGBA_to_rgbA);
    DEFINE_DEFAULT(RGBA_to_bgrA);
    DEFINE_DEFAULT(RGB_to_RGB1);
    DEFINE_DEFAULT(RGB_to_BGR1);
    DEFINE_DEFAULT(gray_to_RGB1);
    DEFINE_DEFAULT(grayA_to_RGBA);
    DEFINE_DEFAULT(grayA_to_rgbA);
    DEFINE_DEFAULT(inverted_CMYK_to_RGB1);
    DEFINE_DEFAULT(inverted_CMYK_to_BGR1);
}
