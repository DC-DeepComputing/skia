/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ContextOptionsPriv_DEFINED
#define skgpu_graphite_ContextOptionsPriv_DEFINED

#include "include/private/SkMath.h"
#include "src/gpu/graphite/compute/ComputeTuning/ComputeTuning.h"

#include <functional>
#include <optional>

namespace skgpu::graphite {

enum class PathRendererStrategy;

/**
 * Private options that are only meant for testing within Skia's tools.
 */
struct ContextOptionsPriv {

    int  fMaxTextureSizeOverride = SK_MaxS32;

    /**
     * If true, will store a pointer in Recorder that points back to the Context
     * that created it. Used by readPixels() and other methods that normally require a Context.
     */
    bool fStoreContextRefInRecorder = false;

    /**
     * If true, enables layer based draw ordering.
     */
    bool fDrawListLayer = false;

    /**
     * If true, blurs that GaussianBlurStep supports are run with compute instead of the shader
     * blur, on devices that support it.
     */
    bool fUseComputeBlur = false;

    /**
     * If set, this is called with the device's compute tuning (the defaults, with the changes for
     * its GPU architecture) to override it. The tuning is composed into the compute steps' SkSL
     * when the Context is created. A step whose tuning is invalid or exceeds the device's compute
     * limits isn't created, and the reason is logged.
     */
    std::function<void(ComputeTuning*)> fOverrideComputeTuning;

    /**
     * Override Caps' default strategy heuristics to prioritize this one if set *and* is supported.
     */
    std::optional<PathRendererStrategy> fPathRendererStrategy;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ContextOptionsPriv_DEFINED
