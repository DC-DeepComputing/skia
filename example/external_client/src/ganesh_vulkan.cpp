/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/vk/GrVkDirectContext.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/VulkanExtensions.h"

int main(int argc, char** argv) {
    // This will not run (since it's missing all the Vulkan setup code),
    // but it should compile.
    GrVkBackendContext backendContext;

    std::unique_ptr<skgpu::VulkanExtensions> extensions(new skgpu::VulkanExtensions());
    backendContext.fInstance = VK_NULL_HANDLE;
    backendContext.fDevice = VK_NULL_HANDLE;

    sk_sp<GrDirectContext> context = GrDirectContexts::MakeVulkan(backendContext);
    if (!context) {
        // There would need to be vulkan cleanup here.
        return 1;
    }

    SkImageInfo imageInfo =
            SkImageInfo::Make(200, 400, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> surface =
            SkSurfaces::RenderTarget(ctx.get(), skgpu::Budgeted::kYes, imageInfo);
    if (!surface) {
        printf("Could not make surface from Vulkan DirectContext\n");
        return 1;
    }

    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorYELLOW);
    SkRRect rrect = SkRRect::MakeRectXY(SkRect::MakeLTRB(10, 20, 50, 70), 10, 10);

    SkPaint paint;
    paint.setColor(SK_ColorPURPLE);
    paint.setAntiAlias(true);

    canvas->drawRRect(rrect, paint);

    ctx->flush();

    // There would need to be vulkan cleanup here.
    return 0;
}