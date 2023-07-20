/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkCpu.h"
#include "src/core/SkOpts.h"

#if defined(SK_CPU_X86) && !defined(SK_ENABLE_OPTIMIZE_SIZE)

#define SK_OPTS_TARGET SK_OPTS_TARGET_SSSE3
#define SK_OPTS_NS ssse3
#include "src/opts/SkOpts_SetTarget.h"
#include "src/opts/SkBitmapProcState_opts.h"
#include "src/opts/SkOpts_RestoreTarget.h"
#undef SK_OPTS_TARGET
#undef SK_OPTS_NS

#define SK_OPTS_TARGET SK_OPTS_TARGET_HSW
#define SK_OPTS_NS hsw
#include "src/opts/SkOpts_SetTarget.h"
#include "src/opts/SkBitmapProcState_opts.h"
#include "src/opts/SkOpts_RestoreTarget.h"
#undef SK_OPTS_TARGET
#undef SK_OPTS_NS

namespace SkOpts {
    void Init_BitmapProcState() {
#if SK_CPU_SSE_LEVEL < SK_CPU_SSE_LEVEL_SSSE3
        if (SkCpu::Supports(SkCpu::SSSE3)) {
            S32_alpha_D32_filter_DX = ssse3::S32_alpha_D32_filter_DX;
        }
#endif

#if SK_CPU_SSE_LEVEL < SK_CPU_SSE_LEVEL_AVX
        if (SkCpu::Supports(SkCpu::HSW)) {
            S32_alpha_D32_filter_DX = hsw::S32_alpha_D32_filter_DX;
        }
#endif
    }

}  // namespace SkOpts

#endif // SK_CPU_X86 && !SK_ENABLE_OPTIMIZE_SIZE
