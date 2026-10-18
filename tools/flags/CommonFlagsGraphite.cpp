/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/flags/CommonFlagsGraphite.h"

#include "include/core/SkExecutor.h"
#include "include/private/SkAssert.h"
#include "src/core/SkExecutors.h"
#include "src/gpu/graphite/compute/ComputeTuning/ComputeTuning.h"
#include "tools/graphite/TestOptions.h"

#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

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
DEFINE_bool(useComputeBlur, false, "Run the blurs that Graphite supports with compute.");
static DEFINE_string(computeTuning, "",
                     "Override the device's compute tuning (see "
                     "src/gpu/graphite/compute/ComputeTuning) with key=value pairs, separated by "
                     "commas or spaces, e.g. blur.tileWidthLog=5,blur.vPerThread=4. The keys that "
                     "aren't given keep the device's tuning. Keys: blur.tileWidthLog, "
                     "blur.tileHeightLog, blur.vPerThread, blur.hPerThread, blur.bucketStep, "
                     "blur.padSums (0 or 1), downsample.workgroupLog, downsample.minSamplesLog, "
                     "downsample.maxClusterLog. A step whose tuning the device can't run isn't "
                     "used, and the reason is logged.");

using skgpu::graphite::ComputeTuning;
using SetTuningFn = void (*)(ComputeTuning*, int);

// The keys of --computeTuning, and the fields of ComputeTuning that they set.
struct TuningKnob {
    const char* fKey;
    SetTuningFn fSet;
};
static constexpr TuningKnob kTuningKnobs[] = {
        {"blur.tileWidthLog",
         [](ComputeTuning* t, int v) { t->fGaussianBlur.fTileWidthLog = v; }},
        {"blur.tileHeightLog",
         [](ComputeTuning* t, int v) { t->fGaussianBlur.fTileHeightLog = v; }},
        {"blur.vPerThread",
         [](ComputeTuning* t, int v) { t->fGaussianBlur.fVerticalSumsPerThread = v; }},
        {"blur.hPerThread",
         [](ComputeTuning* t, int v) { t->fGaussianBlur.fHorizontalSumsPerThread = v; }},
        {"blur.bucketStep",
         [](ComputeTuning* t, int v) { t->fGaussianBlur.fBucketStep = v; }},
        {"blur.padSums",
         [](ComputeTuning* t, int v) { t->fGaussianBlur.fPadSums = v != 0; }},
        {"downsample.workgroupLog",
         [](ComputeTuning* t, int v) { t->fDownsample.fWorkgroupSizeLog = v; }},
        {"downsample.minSamplesLog",
         [](ComputeTuning* t, int v) { t->fDownsample.fMinSamplesPerLaneLog = v; }},
        {"downsample.maxClusterLog",
         [](ComputeTuning* t, int v) { t->fDownsample.fMaxClusterSizeLog = v; }},
};

// Parses --computeTuning into the fields to set and their values, aborting on malformed input.
static std::vector<std::pair<SetTuningFn, int>> parse_compute_tuning() {
    std::string spec;
    for (int i = 0; i < FLAGS_computeTuning.size(); ++i) {
        spec += FLAGS_computeTuning[i];
        spec += ',';
    }
    std::vector<std::pair<SetTuningFn, int>> settings;
    size_t start = 0;
    while (start < spec.size()) {
        const size_t end = spec.find(',', start);
        const std::string pair = spec.substr(start, end - start);
        start = end + 1;
        if (pair.empty()) {
            continue;
        }
        const size_t eq = pair.find('=');
        if (eq == std::string::npos) {
            SK_ABORT("--computeTuning: expected key=value, got '%s'", pair.c_str());
        }
        const std::string key = pair.substr(0, eq);
        const std::string valueString = pair.substr(eq + 1);
        char* valueEnd = nullptr;
        const long value = std::strtol(valueString.c_str(), &valueEnd, 10);
        if (valueString.empty() || *valueEnd != '\0' || value < -1024 || value > 1024) {
            SK_ABORT("--computeTuning: bad value in '%s'", pair.c_str());
        }
        SetTuningFn set = nullptr;
        for (const TuningKnob& knob : kTuningKnobs) {
            if (key == knob.fKey) {
                set = knob.fSet;
                break;
            }
        }
        if (!set) {
            SK_ABORT("--computeTuning: unknown key '%s'", key.c_str());
        }
        settings.emplace_back(set, static_cast<int>(value));
    }
    return settings;
}

void SetTestOptions(skiatest::graphite::TestOptions* testOptions) {
    static std::unique_ptr<SkExecutor> gGpuExecutor;
    if (0 != FLAGS_gpuThreads) {
        gGpuExecutor = SkExecutors::MakeMultiListFIFOThreadPool(/* numWorkLists= */ 2,
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
        testOptions->fOptionsPriv.fDrawListLayer = FLAGS_useDrawListLayer;
    }
    if (FLAGS_useComputeBlur) {
        testOptions->fOptionsPriv.fUseComputeBlur = true;
    }
    if (!FLAGS_computeTuning.isEmpty()) {
        // The settings apply on top of the tuning of the device's architecture.
        testOptions->fOptionsPriv.fOverrideComputeTuning =
                [settings = parse_compute_tuning()](ComputeTuning* tuning) {
                    for (const auto& [set, value] : settings) {
                        set(tuning, value);
                    }
                };
    }

#if defined(SK_DAWN)
    testOptions->fDisableTintSymbolRenaming = FLAGS_disable_tint_symbol_renaming;
    testOptions->fNeverYieldToWebGPU = FLAGS_neverYieldToWebGPU;
    testOptions->fUseWGPUTextureView = FLAGS_useWGPUTextureView;
#endif // SK_DAWN
}

}  // namespace CommonFlags
