/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"

#if defined(SK_CPU_X86) && !defined(SK_ENABLE_OPTIMIZE_SIZE)

#if defined(SK_ENABLE_SKVM)

#define SK_OPTS_TARGET SK_OPTS_TARGET_SKX
#include "src/opts/SkOpts_SetTarget.h"

#define SK_OPTS_NS skx
#include "src/opts/SkVM_opts.h"

#include "src/opts/SkOpts_RestoreTarget.h"

#endif

namespace SkOpts {
    void Init_skx() {
#if defined(SK_ENABLE_SKVM)
        interpret_skvm = SK_OPTS_NS::interpret_skvm;
#endif
    }
}  // namespace SkOpts

#endif // SK_CPU_X86 && !SK_ENABLE_OPTIMIZE_SIZE
