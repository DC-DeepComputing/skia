/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skiatest_graphite_TestOptions_DEFINED
#define skiatest_graphite_TestOptions_DEFINED

#include "include/gpu/graphite/ContextOptions.h"

namespace skiatest::graphite {

struct TestOptions {
    skgpu::graphite::ContextOptions fContextOptions;
    bool fNeverYieldToWebGPU = false;
};

}  // namespace skiatest::graphite

#endif
