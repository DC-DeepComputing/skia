/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"

#if defined(__linux__)
#include "include/gpu/gl/glx/GrGLMakeGLXInterface.h"
#endif

#include <cstdio>

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <name.png>\n", argv[0]);
        return 1;
    }

    GrContextOptions opts;
    opts.fSuppressPrints = true;
#if defined(__linux__)
    sk_sp<const GrGLInterface> interface = GrGLInterfaces::MakeGLX();
#endif
    sk_sp<GrDirectContext> ctx = GrDirectContexts::MakeGL(interface, opts);

    if (!ctx) {
        printf("Could not make GL interface\n");
        return 1;
    }
    printf("Context made, now to make the surface\n");

    SkImageInfo imageInfo =
            SkImageInfo::Make(200, 400, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> surface =
            SkSurfaces::RenderTarget(ctx.get(), skgpu::Budgeted::kYes, imageInfo);
    if (!surface) {
        printf("Could not make surface from GL DirectContext\n");
        return 1;
    }

    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorRED);
    SkRRect rrect = SkRRect::MakeRectXY(SkRect::MakeLTRB(10, 20, 50, 70), 10, 10);

    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    paint.setAntiAlias(true);

    canvas->drawRRect(rrect, paint);

    ctx->flush();

    return 0;
}
