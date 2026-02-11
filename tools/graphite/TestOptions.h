/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skiatest_graphite_TestOptions_DEFINED
#define skiatest_graphite_TestOptions_DEFINED

#include "include/gpu/graphite/ContextOptions.h"

#if defined(GPU_TEST_UTILS)
#include "src/gpu/graphite/ContextOptionsPriv.h"
#endif

namespace skiatest::graphite {

struct TestOptions {
    TestOptions() {
#if defined(GPU_TEST_UTILS)
    fContextOptions.fOptionsPriv = &fOptionsPriv;
#endif
    }
    TestOptions(const TestOptions&) = default;
    TestOptions(TestOptions&&) = default;
    TestOptions& operator=(const TestOptions&) = default;
    TestOptions& operator=(TestOptions&&) = default;

    bool hasDawnOptions() const {
#if defined(SK_DAWN)
        return fDisableTintSymbolRenaming ||
               fNeverYieldToWebGPU ||
               fUseWGPUTextureView;
#else
        return false;
#endif
    }

    skgpu::graphite::ContextOptions fContextOptions = {};

#if defined(SK_DAWN)
    bool fDisableTintSymbolRenaming = false;
    bool fNeverYieldToWebGPU = false;
    bool fUseWGPUTextureView = false;
#endif

#if defined(GPU_TEST_UTILS)
    skgpu::graphite::ContextOptionsPriv fOptionsPriv;
#endif
};

}  // namespace skiatest::graphite

#endif
