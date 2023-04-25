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
#include "src/opts/Hash_opts_impl.h"
// Finally, this declares the function pointers in SkOpts that we can override here.
#include "src/opts/Hash_opts.h"

namespace SkOpts {
    DEFINE_DEFAULT(hash_fn);
}
