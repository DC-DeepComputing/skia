/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileImageFiltersPriv_DEFINED
#define skgpu_graphite_precompile_PrecompileImageFiltersPriv_DEFINED

#include "src/gpu/graphite/precompile/PaintOptionsPriv.h"

namespace skgpu::graphite {

class KeyContext;
class PipelineDataGatherer;

namespace PrecompileImageFiltersPriv {

    // Used by both the BlurMaskFilter and the BlurImageFilter
    void CreateBlurImageFilterPipelines(const KeyContext&,
                                        PipelineDataGatherer*,
                                        const PaintOptionsPriv::ProcessCombination&);

    // By itself blurring requires 24 combinations:
    //         2 x (hw-tiled and shader-tiled image draws)
    //         12 (for the 6 1D blur effects plus the 6 2D blur effects)
    static constexpr int kExpectedNumBlurPipelines = 24;

} // namespace PrecompileImageFiltersPriv

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PrecompileImageFiltersPriv_DEFINED
