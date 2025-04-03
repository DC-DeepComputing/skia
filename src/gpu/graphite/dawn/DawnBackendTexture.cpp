/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "src/gpu/graphite/BackendTexturePriv.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"
#include "src/gpu/graphite/dawn/DawnTexture.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

#include <cstdint>

namespace skgpu::graphite {

class DawnBackendTextureData final : public BackendTextureData {
public:
    DawnBackendTextureData(WGPUTexture tex, WGPUTextureView rv, WGPUTextureView sv)
            : fTexture(tex), fRenderView(rv), fSampleView(sv) {}

#if defined(SK_DEBUG)
    skgpu::BackendApi type() const override { return skgpu::BackendApi::kDawn; }
#endif

    WGPUTexture texture() const { return fTexture; }
    WGPUTextureView renderView() const { return fRenderView; }
    WGPUTextureView sampleView() const { return fSampleView; }

private:
    WGPUTexture fTexture;
    WGPUTextureView fRenderView;
    WGPUTextureView fSampleView;

    void copyTo(AnyBackendTextureData& dstData) const override {
        // Don't assert that dstData has a Dawn type() because it could be
        // uninitialized and that assert would fail.
        dstData.emplace<DawnBackendTextureData>(fTexture, fRenderView, fSampleView);
    }

    bool equal(const BackendTextureData* that) const override {
        SkASSERT(!that || that->type() == skgpu::BackendApi::kDawn);
        if (auto otherDawn = static_cast<const DawnBackendTextureData*>(that)) {
            return fTexture == otherDawn->fTexture && fRenderView == otherDawn->fRenderView &&
                   fSampleView == otherDawn->fSampleView;
        }
        return false;
    }
};

static const DawnBackendTextureData* get_and_cast_data(const BackendTexture& tex) {
    auto data = BackendTexturePriv::GetData(tex);
    SkASSERT(!data || data->type() == skgpu::BackendApi::kDawn);
    return static_cast<const DawnBackendTextureData*>(data);
}

// When we only have a WGPUTextureView we can't actually take advantage of these TextureUsage bits
// because they require having the WGPUTexture.
static DawnTextureInfo strip_copy_usage(const DawnTextureInfo& info) {
    DawnTextureInfo result = info;
    result.fUsage &= ~(wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc);
    return result;
}

DawnBackendTextureAndView::DawnBackendTextureAndView(BackendTexture backendTexture)
        : fBackendTexture(std::move(backendTexture))
        // wgpu::TextureView ctor will acquire the ref count.
        , fRenderView(BackendTextures::GetDawnRenderViewPtr(fBackendTexture))
        , fSampleView(BackendTextures::GetDawnSampleViewPtr(fBackendTexture)) {}

namespace BackendTextures {
BackendTexture MakeDawn(WGPUTexture texture) {
    return MakeDawn(TextureInfos::MakeDawn(texture), texture, nullptr, nullptr);
}

BackendTexture MakeDawn(const TextureInfo& info,
                        WGPUTexture texture,
                        WGPUTextureView rv,
                        WGPUTextureView sv) {
    return BackendTexturePriv::Make(
            {
                    static_cast<int32_t>(wgpuTextureGetWidth(texture)),
                    static_cast<int32_t>(wgpuTextureGetHeight(texture)),
            },
            info,
            DawnBackendTextureData(texture, rv, sv));
}

DawnBackendTextureAndView MakeDawnAndView(WGPUTexture texture) {
    const auto info = TextureInfos::MakeDawn(texture);

    auto [renderView, sampleView] = DawnTexture::CreateTextureViews(texture, info);
    auto backendTexture = MakeDawn(info, texture, renderView.Get(), sampleView.Get());

    return DawnBackendTextureAndView(std::move(backendTexture));
}

namespace {

BackendTexture MakeDawn(SkISize planeDimensions,
                        const TextureInfo& info,
                        WGPUTexture texture,
                        WGPUTextureView rv,
                        WGPUTextureView sv) {
    const auto& dawnInfo = TextureInfoPriv::Get<DawnTextureInfo>(info);
#if defined(__EMSCRIPTEN__)
    SkASSERT(dawnInfo.fAspect == wgpu::TextureAspect::All);
#else
    SkASSERT(dawnInfo.fAspect == wgpu::TextureAspect::All ||
             dawnInfo.fAspect == wgpu::TextureAspect::Plane0Only ||
             dawnInfo.fAspect == wgpu::TextureAspect::Plane1Only ||
             dawnInfo.fAspect == wgpu::TextureAspect::Plane2Only);
#endif
    return BackendTexturePriv::Make(planeDimensions, info, DawnBackendTextureData(texture, rv, sv));
}

}  // namespace

BackendTexture MakeDawn(SkISize planeDimensions, const DawnTextureInfo& info, WGPUTexture texture) {
    return MakeDawn(planeDimensions, TextureInfos::MakeDawn(info), texture, nullptr, nullptr);
}

DawnBackendTextureAndView MakeDawnAndView(SkISize planeDimensions,
                                          const DawnTextureInfo& dawnInfo,
                                          WGPUTexture texture) {
    const auto info = TextureInfos::MakeDawn(dawnInfo);
    auto [renderView, sampleView] = DawnTexture::CreateTextureViews(texture, info);
    auto backendTexture =
            MakeDawn(planeDimensions, info, texture, renderView.Get(), sampleView.Get());

    return DawnBackendTextureAndView(std::move(backendTexture));
}

BackendTexture MakeDawn(SkISize dimensions,
                        const DawnTextureInfo& info,
                        WGPUTextureView textureView) {
    return BackendTexturePriv::Make(dimensions,
                                    TextureInfos::MakeDawn(strip_copy_usage(info)),
                                    DawnBackendTextureData(nullptr, textureView, textureView));
}

WGPUTexture GetDawnTexturePtr(const BackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != skgpu::BackendApi::kDawn) {
        return nullptr;
    }
    const DawnBackendTextureData* dawnData = get_and_cast_data(tex);
    SkASSERT(dawnData);
    return dawnData->texture();
}

WGPUTextureView GetDawnRenderViewPtr(const BackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != skgpu::BackendApi::kDawn) {
        return nullptr;
    }
    const DawnBackendTextureData* dawnData = get_and_cast_data(tex);
    SkASSERT(dawnData);
    return dawnData->renderView();
}

WGPUTextureView GetDawnSampleViewPtr(const BackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != skgpu::BackendApi::kDawn) {
        return nullptr;
    }
    const DawnBackendTextureData* dawnData = get_and_cast_data(tex);
    SkASSERT(dawnData);
    return dawnData->sampleView();
}

}  // namespace BackendTextures

}  // namespace skgpu::graphite
