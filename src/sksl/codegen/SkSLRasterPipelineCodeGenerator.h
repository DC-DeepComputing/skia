/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_RASTERPIPELINEGENERATOR
#define SKSL_RASTERPIPELINEGENERATOR

#include "include/core/SkTypes.h"

struct SkStageRec;

namespace SkSL {

class FunctionDefinition;
struct Program;

class SkRPCallbacks {
public:
    virtual ~SkRPCallbacks() = default;

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

// Convert 'function' to skvm instructions in 'builder', for use by blends, shaders, & color filters
// Assumption is that the arguments to 'function' are in the registers:
//   -- coords in rg for shaders
//   -- color in rgba for color filters
//   -- src, dst in rgba, drgba for blenders
bool AppendStages(const Program& program,
                  const FunctionDefinition& function,
                  const SkStageRec& rec,
                  SkRPCallbacks* callbacks);

}  // namespace SkSL

#endif
