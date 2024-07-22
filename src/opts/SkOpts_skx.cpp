/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

#define SK_OPTS_NS skx
#include "src/opts/SkRasterPipeline_opts.h"

namespace SkOpts {
    void Init_skx() {
        raster_pipeline_lowp_stride  = SK_OPTS_NS::raster_pipeline_lowp_stride();
        raster_pipeline_highp_stride = SK_OPTS_NS::raster_pipeline_highp_stride();

        start_pipeline_highp = SK_OPTS_NS::start_pipeline;
        start_pipeline_lowp = SK_OPTS_NS::lowp::start_pipeline;
    }
}  // namespace SkOpts

#endif // SK_ENABLE_OPTIMIZE_SIZE
