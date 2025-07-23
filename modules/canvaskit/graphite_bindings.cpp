/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"

#include "modules/canvaskit/WasmCommon.h"

#include <emscripten.h>
#include <emscripten/bind.h>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>

using JSGPUDevice = uintptr_t;

EMSCRIPTEN_BINDINGS(Graphite) {
    constant("graphite", true);

    function("TestGraphite", optional_override([](JSGPUDevice d)->bool {
        constexpr int WIDTH = 200;
        constexpr int HEIGHT = 400;
        auto device = reinterpret_cast<WGPUDevice>(d);
        SkDebugf("device %p\n", device);

        skgpu::graphite::DawnBackendContext ctx;
        ctx.fDevice = wgpu::Device::Acquire(device);
        ctx.fQueue = ctx.fDevice.GetQueue();

        skgpu::graphite::Context* context = skgpu::graphite::ContextFactory::MakeDawn(ctx, {}).release();

        if (!context) {
            return false;
        }
        SkDebugf("Have context\n");

        SkImageInfo imageInfo =
            SkImageInfo::Make(WIDTH, HEIGHT, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

        std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();
        if (!recorder) {
            printf("Could not make recorder\n");
            return 1;
        }
        sk_sp<SkSurface> surface =
                SkSurfaces::RenderTarget(recorder.get(), imageInfo);
        if (!surface) {
            printf("Could not make surface from Dawn Recorder\n");
            return 1;
        }

        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(SK_ColorCYAN);
        SkRRect rrect = SkRRect::MakeRectXY(SkRect::MakeLTRB(10, 20, 50, 70), 10, 10);

        SkPaint paint;
        paint.setColor(SK_ColorYELLOW);
        paint.setAntiAlias(true);

        canvas->drawRRect(rrect, paint);

        printf("ready to snap the GPU calls\n");
        // Now to send the draws to the GPU
        std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
        if (!recording) {
            printf("Could not create a recording\n");
            return 1;
        }
        skgpu::graphite::InsertRecordingInfo info;
        info.fRecording = recording.get();
        if (!context->insertRecording(info)) {
            printf("Context::insertRecording failed\n");
            return 1;
        }

        return true;
    }), allow_raw_pointers());
}

// #ifdef CK_ENABLE_WEBGPU
//         .function("_replaceBackendTexture", optional_override([](SkSurface& self,
//                                                 uint32_t texHandle, uint32_t texFormat,
//                                                 int width, int height) {
//             return ReplaceBackendTexture(self, texHandle, texFormat, width, height);
//          }))
// #endif  // CK_ENABLE_WEBGPU

// #ifdef CK_ENABLE_WEBGPU

// sk_sp<GrDirectContext> MakeGrContext() {
//     GrContextOptions options;
//     wgpu::Device device = wgpu::Device::Acquire(emscripten_webgpu_get_device());
//     return GrDirectContext::MakeDawn(device, options);
// }

// sk_sp<SkSurface> MakeGPUTextureSurface(sk_sp<GrDirectContext> dContext,
//                                        uint32_t textureHandle, uint32_t textureFormat,
//                                        int width, int height, sk_sp<SkColorSpace> colorSpace) {
//     if (!colorSpace) {
//       colorSpace = SkColorSpace::MakeSRGB();
//     }

//     wgpu::TextureFormat format = static_cast<wgpu::TextureFormat>(textureFormat);
//     wgpu::Texture texture(emscripten_webgpu_import_texture(textureHandle));
//     emscripten_webgpu_release_js_handle(textureHandle);

//     // GrDawnRenderTargetInfo currently only supports a 1-mip TextureView.
//     constexpr uint32_t mipLevelCount = 1;
//     constexpr uint32_t sampleCount = 1;

//     GrDawnTextureInfo info;
//     info.fTexture = texture;
//     info.fFormat = format;
//     info.fLevelCount = mipLevelCount;

//     GrBackendTexture target(width, height, info);
//     return SkSurfaces::WrapBackendTexture(
//             dContext.get(),
//             target,
//             kTopLeft_GrSurfaceOrigin,
//             sampleCount,
//             colorSpace->isSRGB() ? kRGBA_8888_SkColorType : kRGBA_F16_SkColorType,
//             colorSpace,
//             nullptr);
// }

// bool ReplaceBackendTexture(SkSurface& surface, uint32_t textureHandle, uint32_t textureFormat,
//                            int width, int height) {
//     wgpu::TextureFormat format = static_cast<wgpu::TextureFormat>(textureFormat);
//     wgpu::Texture texture(emscripten_webgpu_import_texture(textureHandle));
//     emscripten_webgpu_release_js_handle(textureHandle);

//     GrDawnTextureInfo info;
//     info.fTexture = texture;
//     info.fFormat = format;
//     info.fLevelCount = 1;

//     // Use kDiscard_ContentChangeMode to discard the contents of the old backing texture. This not
//     // only avoids an unnecessary blit, we also don't support copying the contents of a swapchain
//     // texture due to the default GPUCanvasConfiguration usage bits we used when configuring the
//     // GPUCanvasContext in JS.
//     //
//     // The default usage bits only contain GPUTextureUsage.RENDER_ATTACHMENT. To support a copy we
//     // would need to also set GPUTextureUsage.TEXTURE_BINDING (to sample it in a shader) or
//     // GPUTextureUsage.COPY_SRC (for a copy command).
//     //
//     // See https://www.w3.org/TR/webgpu/#namespacedef-gputextureusage and
//     // https://www.w3.org/TR/webgpu/#dictdef-gpucanvasconfiguration.
//     GrBackendTexture target(width, height, info);
//     return surface.replaceBackendTexture(target, kTopLeft_GrSurfaceOrigin,
//                                          SkSurface::kDiscard_ContentChangeMode);
// }

// #endif // CK_ENABLE_WEBGPU
