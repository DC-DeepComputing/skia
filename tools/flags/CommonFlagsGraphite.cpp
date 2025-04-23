/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/flags/CommonFlagsGraphite.h"
#include "tools/graphite/TestOptions.h"

namespace CommonFlags {
static DEFINE_int(internalSamples, -1,
        "Number of samples for internal draws that use MSAA, or default value if negative.");

#if defined(SK_DAWN)
static DEFINE_bool(disable_tint_symbol_renaming, false, "Disable Tint WGSL symbol renaming when "
                                                        "using Dawn");
static DEFINE_bool(neverYieldToWebGPU, false, "Run Graphite with never-yield context option.");
static DEFINE_bool(useTintIR, false, "Run Graphite with Dawn's use_tint_ir feature");
static DEFINE_bool(useWGPUTextureView, false, "Run Graphite w/ a wrapped WGPU texture view as "
                                              "the destination");
#endif // SK_DAWN

void SetTestOptions(skiatest::graphite::TestOptions* testOptions) {
    if (FLAGS_internalSamples >= 0) {
        testOptions->fContextOptions.fInternalMultisampleCount = FLAGS_internalSamples;
    }
#if defined(SK_DAWN)
    testOptions->fDisableTintSymbolRenaming = FLAGS_disable_tint_symbol_renaming;
    testOptions->fNeverYieldToWebGPU = FLAGS_neverYieldToWebGPU;
    testOptions->fUseTintIR = FLAGS_useTintIR;
    testOptions->fUseWGPUTextureView = FLAGS_useWGPUTextureView;
#endif // SK_DAWN
}

}  // namespace CommonFlags
