/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/flags/CommonFlagsGraphite.h"

#include "include/core/SkExecutor.h"
#include "src/core/SkExecutors.h"
#include "tools/graphite/TestOptions.h"

// Defined in CommonFlagsConfig
DECLARE_int(gpuThreads)
DECLARE_int(internalSamples)

namespace CommonFlags {
#if defined(SK_DAWN)
static DEFINE_bool(disable_tint_symbol_renaming, false, "Disable Tint WGSL symbol renaming when "
                                                        "using Dawn");
static DEFINE_bool(neverYieldToWebGPU, false, "Run Graphite with never-yield context option.");
static DEFINE_bool(useWGPUTextureView, false, "Run Graphite w/ a wrapped WGPU texture view as "
                                              "the destination");
#endif // SK_DAWN

DEFINE_int(internalMSAATileSize, 0, "Run Graphite w/ limited MSAA texture's size");
DEFINE_int(minMSAAPathSize, -1,
           "Graphite uses raster atlas for paths smaller than this, or default value if negative");
DEFINE_bool(useDrawListLayer, false, "Enable experimental layer-based draw ordering.");
DEFINE_string(depthMode, "default", "Set the depth usage mode option: "
              "default, always, never, avoid-internal, when-required");

void SetTestOptions(skiatest::graphite::TestOptions* testOptions) {
#if defined(SK_GRAPHITE_USE_CALLBACK_EXECUTOR)
    using ExecutorType = SkCallbackExecutor;
#else
    using ExecutorType = SkFunctionalExecutor;
#endif

    static std::unique_ptr<ExecutorType> gGpuExecutor;
    if (0 != FLAGS_gpuThreads) {
        gGpuExecutor = SkExecutors<ExecutorType>::MakeMultiListFIFOThreadPool(
            /* numWorkLists= */ 2,
            FLAGS_gpuThreads,
            /* allowBorrowing= */false);
    }

    testOptions->fContextOptions.fExecutor = gGpuExecutor.get();

    if (FLAGS_internalSamples >= 0) {
        testOptions->fContextOptions.fInternalMultisampleCount =
                skgpu::graphite::ToSampleCount(FLAGS_internalSamples);
    }
    if (FLAGS_internalMSAATileSize > 0) {
        testOptions->fContextOptions.fInternalMSAATileSize = {FLAGS_internalMSAATileSize,
                                                              FLAGS_internalMSAATileSize};
    }
    if (FLAGS_minMSAAPathSize >= 0) {
        testOptions->fContextOptions.fMinimumPathSizeForMSAA = FLAGS_minMSAAPathSize;
    }
    if (FLAGS_useDrawListLayer) {
        testOptions->fContextOptions.fUseDrawListLayer = FLAGS_useDrawListLayer;
    }

    if (strcmp(FLAGS_depthMode[0], "default") == 0) {
        testOptions->fContextOptions.fDepthUsageMode = std::nullopt;
    } else if (strcmp(FLAGS_depthMode[0], "always") == 0) {
        testOptions->fContextOptions.fDepthUsageMode = skgpu::graphite::DepthUsageMode::kAlways;
    } else if (strcmp(FLAGS_depthMode[0], "never") == 0) {
        testOptions->fContextOptions.fDepthUsageMode = skgpu::graphite::DepthUsageMode::kNever;
    } else if (strcmp(FLAGS_depthMode[0], "avoid-internal") == 0) {
        testOptions->fContextOptions.fDepthUsageMode =
                skgpu::graphite::DepthUsageMode::kAvoidForInternal;
    } else if (strcmp(FLAGS_depthMode[0], "when-required") == 0) {
        testOptions->fContextOptions.fDepthUsageMode =
                skgpu::graphite::DepthUsageMode::kOnlyWhenRequired;
    } else {
        SkDebugf("Unknown value for --depthMode: %s\n", FLAGS_depthMode[0]);
        exit(-1);
    }

#if defined(SK_DAWN)
    testOptions->fDisableTintSymbolRenaming = FLAGS_disable_tint_symbol_renaming;
    testOptions->fNeverYieldToWebGPU = FLAGS_neverYieldToWebGPU;
    testOptions->fUseWGPUTextureView = FLAGS_useWGPUTextureView;
#endif // SK_DAWN
}

}  // namespace CommonFlags
