/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_RASTERPIPELINECODEGENERATOR
#define SKSL_RASTERPIPELINECODEGENERATOR

#include "include/core/SkTypes.h"

struct SkStageRec;

namespace SkSL {

class FunctionDefinition;
struct Program;

class SkRasterPipelineCallbacks {
public:
    virtual ~SkRasterPipelineCallbacks() = default;

    // rg (coords) -> rgba
    virtual bool sampleShader(int index) = 0;

    // rgba -> rgba
    virtual bool sampleColorFilter(int index) = 0;

    // rgba, drgba -> rgba
    virtual bool sampleBlender(int index) = 0;

    // rgba -> rgba
    virtual bool toLinearSrgb() = 0;
    virtual bool fromLinearSrgb() = 0;
};

// Convert 'function' to Raster Pipeline stages, for use by blends, shaders, and color filters.
// The arguments to the function function are passed in registers:
//   -- coords in rg for shaders
//   -- color in rgba for color filters
//   -- src, dst in rgba, drgba for blenders
bool AppendStages(const Program& program,
                  const FunctionDefinition& function,
                  const SkStageRec& rec,
                  SkRasterPipelineCallbacks* callbacks);

}  // namespace SkSL

#endif
