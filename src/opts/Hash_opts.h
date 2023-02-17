/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Hash_opts_DEFINED
#define Hash_opts_DEFINED

#include <cstdint>
#include <cstddef>

namespace SkOpts {
    // The fastest high quality 32-bit hash we can provide on this platform.
    extern uint32_t (*hash_fn)(const void* data, size_t bytes, uint32_t seed);
}

#endif
