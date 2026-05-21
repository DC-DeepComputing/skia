/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkM44.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "tests/Test.h"
#include "tools/gpu/ContextType.h"

static void only_allow_small(GrContextOptions* options) {
    options->fGpuPathRenderers = GpuPathRenderers::kSmall;
}

DEF_GANESH_TEST_FOR_CONTEXTS(SmallPathRenderer,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             only_allow_small,
                             CtsEnforcement::kNever) {
    auto ctx = ctxInfo.directContext();
    auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(
                ctx, GrColorType::kRGBA_8888, nullptr, SkBackingFit::kExact,
                {100, 100}, SkSurfaceProps(), /*label=*/{});
    if (!sdc) {
        return;
    }

    sdc->clear(SK_PMColor4fBLACK);

    GrStyle style(SkStrokeRec::kFill_InitStyle);
    const SkPath smolpath = SkPath::Oval({1.57842e-41f, 0, 2.29589e-41f, 1.4013e-45f});
    const SkMatrix m = SkMatrix::MakeAll(
        -0, 70368341861093281422320336896.f, -590295810358705651712.f,
         2, 17469300226849243136.f, 0,
         868052350533632.f, 4220791729285326409039872.f, 3572.759766f
    );
    sdc->drawPath(nullptr, GrPaint(), GrAA::kYes, m, smolpath, style);

    ctx->flushAndSubmit();
}
