/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkOpts_spi.h"
#include "src/opts/SkChecksum_opts.h"

#include <cstdint>

namespace SkOpts {
decltype(hash_fn) hash_fn = SK_OPTS_NS::hash_fn;
}
