/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "src/sksl/codegen/SkSLRasterPipelineCodeGenerator.h"

namespace SkSL {

bool AppendStages(const SkSL::Program& program,
                  const FunctionDefinition& function,
                  SkRasterPipeline* pipeline,
                  SkArenaAlloc* alloc,
                  SkRasterPipelineCallbacks* callbacks) {
    // TODO(skia:13676): use the input program.
    // For now, ignore the program. Load magenta (1 1 0 1) into the first four slots.
    RP::Builder builder;
    builder.immediate_f(1.0f);
    builder.store_unmasked(0);
    builder.store_unmasked(1);
    builder.store_unmasked(3);
    builder.immediate_f(0.0f);
    builder.store_unmasked(2);

    // Load the first four slots into RGBA.
    builder.load_src(RP::SlotRange{0, 4});

    // Emit these stages into the raster pipeline.
    RP::Program rasterProg = builder.finish();
    rasterProg.appendStages(pipeline, alloc);

    return true;
}

}  // namespace SkSL
