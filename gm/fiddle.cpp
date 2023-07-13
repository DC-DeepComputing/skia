/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "skia.h"

#include "include/core/SkVertices.h"

// #include "C:\Program Files\RenderDoc\renderdoc_app.h"
// #include <windows.h>
// #include <cassert>
// RENDERDOC_API_1_1_2 *rdoc_api = nullptr;

static void draw(SkCanvas* canvas);
DEF_SIMPLE_GM(fiddle, canvas, 256, 256) { draw(canvas); }

// Paste your fiddle.skia.org code over this stub.
void draw(SkCanvas* canvas) {
// #if defined(SK_BUILD_FOR_WIN)
//     if(HMODULE mod = GetModuleHandleA("renderdoc.dll"))
//     {
//         pRENDERDOC_GetAPI RENDERDOC_GetAPI =
//             (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
//         int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&rdoc_api);
//         assert(ret == 1);
//     }
// #endif

    // if(rdoc_api) rdoc_api->StartFrameCapture(NULL, NULL);

    // SkPaint p;
    // p.setColor(SK_ColorRED);
    // p.setAntiAlias(true);
    // p.setStyle(SkPaint::kStroke_Style);
    // p.setStrokeWidth(10);

    // canvas->drawLine(20, 20, 100, 100, p);

    SkPoint pts[] = {{0, 0}, {0, 100}, {100, 100}, {100, 0}, {100, 100}, {0, 100}};
    sk_sp<SkVertices> verts =
            SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode, 6, pts, nullptr, nullptr);

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(10);

    canvas->drawVertices(verts, SkBlendMode::kSrc, paint);

    // // stop the capture
    // if(rdoc_api) rdoc_api->EndFrameCapture(NULL, NULL);
}
