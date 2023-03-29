
/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/sdl2/WindowContextFactory_sdl2.h"

// #include "/home/ubuntu/skia/third_party/externals/SDL2/include/SDL_surface.h"
#include "include/core/SkSurface.h"
#include "tools/sk_app/RasterWindowContext.h"

using sk_app::DisplayParams;
using sk_app::RasterWindowContext;
using sk_app::window_context_factory::SDL2WindowInfo;

extern SDL_Surface* SDL_CreateRGBSurfaceWithFormatFrom(void *pixels, int width, int height, int depth, int pitch, Uint32 format);

namespace {

class RasterWindowContext_sdl2 : public RasterWindowContext {
public:
    RasterWindowContext_sdl2(const SDL2WindowInfo&, const DisplayParams&);

    sk_sp<SkSurface> getBackbufferSurface() override;
    void swapBuffers() override;
    bool isValid() override { return SkToBool(fWindow); }
    void resize(int w, int h) override;
    void setDisplayParams(const DisplayParams& params) override;

private:
    void setDisplayParamsImpl(const DisplayParams&);  // can be called from constructor.
    sk_sp<SkSurface> fBackbufferSurface;

    SDL_Window* fWindow;

    using INHERITED = RasterWindowContext;
};

RasterWindowContext_sdl2::RasterWindowContext_sdl2(const SDL2WindowInfo& info,
                                                   const DisplayParams& params)
        : INHERITED(params), fWindow(info.window) {
    this->setDisplayParamsImpl(params);
}

void RasterWindowContext_sdl2::setDisplayParams(const DisplayParams& params) {
    this->setDisplayParamsImpl(params);
}

void RasterWindowContext_sdl2::setDisplayParamsImpl(const DisplayParams& params) {
    fDisplayParams = params;
    int w = -1;
    int h = -1;
    SDL_GetWindowSize(fWindow, &w, &h);
    this->resize(w, h);
}

void RasterWindowContext_sdl2::resize(int w, int h) {
    SDL_GetWindowSize(fWindow, &w, &h);
    fBackbufferSurface = SkSurface::MakeRaster(
            SkImageInfo::Make(w, h,
                              fDisplayParams.fColorType,
                              kPremul_SkAlphaType,
                              fDisplayParams.fColorSpace),
            &fDisplayParams.fSurfaceProps);
}

sk_sp<SkSurface> RasterWindowContext_sdl2::getBackbufferSurface() { return fBackbufferSurface; }

static Uint32 colortype_to_format(SkColorType ct) {
    switch (ct) {
        // TODO Support 16 bit per channel somehow?
        case kUnknown_SkColorType:
        case kAlpha_8_SkColorType:
        case kBGRA_1010102_SkColorType:
        case kBGR_101010x_SkColorType:
        case kGray_8_SkColorType:
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType:
        case kRGBA_F32_SkColorType:
        case kR8G8_unorm_SkColorType:
        case kA16_float_SkColorType:
        case kR16G16_float_SkColorType:
        case kA16_unorm_SkColorType:
        case kR16G16_unorm_SkColorType:
        case kR16G16B16A16_unorm_SkColorType:
        case kR8_unorm_SkColorType:
        case kBGR_101010x_XR_SkColorType:
            return SDL_PIXELFORMAT_UNKNOWN;

        // Only kBGRA_8888_SkColorType->SDL_PIXELFORMAT_ARGB8888 has been tested. Likely other
        // mappings are wrong.
        case kRGB_565_SkColorType:
            return SDL_PIXELFORMAT_BGR565;
        case kARGB_4444_SkColorType:
            return SDL_PIXELFORMAT_ARGB4444;
        case kRGBA_8888_SkColorType:
        case kSRGBA_8888_SkColorType:
            return SDL_PIXELFORMAT_RGBA8888;
        case kRGB_888x_SkColorType:
            return SDL_PIXELFORMAT_RGBA8888;
        case kBGRA_8888_SkColorType:
            return SDL_PIXELFORMAT_ARGB8888;
        case kRGBA_1010102_SkColorType:
            return SDL_PIXELFORMAT_ARGB2101010;
        case kRGB_101010x_SkColorType:
            return SDL_PIXELFORMAT_ARGB2101010;
    }
}

void RasterWindowContext_sdl2::swapBuffers() {
    SkPixmap pm;
    if (!fBackbufferSurface->peekPixels(&pm)) {
        return;
    }
    int dstW, dstH;
    SDL_GetWindowSize(fWindow, &dstW, &dstH);
    if (dstW != pm.width() || dstH != pm.height()) {
        return;
    }
    int depth = SkColorTypeBytesPerPixel(pm.colorType());
    Uint32 format = colortype_to_format(pm.colorType());
    if (format == SDL_PIXELFORMAT_UNKNOWN) {
        return;
    }

    SDL_Surface* srcSurface = SDL_CreateRGBSurfaceWithFormatFrom(pm.writable_addr(),
                                                                 pm.width(),
                                                                 pm.height(),
                                                                 depth, pm.rowBytes(),
                                                                 format);
    if (!srcSurface) {
        return;
    }
    SDL_Surface* dstSurface = SDL_GetWindowSurface(fWindow);
    if (!dstSurface) {
        return;
    }
    SDL_Rect rect = {0, 0, dstW, dstH};
    SDL_SetSurfaceBlendMode(dstSurface, SDL_BLENDMODE_NONE);
    SDL_BlitSurface(srcSurface, &rect, dstSurface, &rect);
    SDL_UpdateWindowSurface(fWindow);
}

}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

std::unique_ptr<WindowContext> MakeRasterForSDL2(const SDL2WindowInfo& info,
                                                 const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new RasterWindowContext_sdl2(info, params));
    if (!ctx->isValid()) {
        ctx = nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app
