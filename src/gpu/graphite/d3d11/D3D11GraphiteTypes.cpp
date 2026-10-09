/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/d3d11/D3D11GraphiteTypes.h"

#include "include/core/SkString.h"
#include "src/gpu/graphite/BackendTexturePriv.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/d3d11/D3D11Caps.h"

namespace skgpu::graphite {

TextureFormat D3D11TextureInfo::viewFormat() const {
    return D3D11FormatToTextureFormat(this->getViewFormat());
}

SkString D3D11TextureInfo::toBackendString() const {
    return SkStringPrintf("dxgiFormat=%u,bindFlags=0x%08X,usage=%u",
                          static_cast<unsigned int>(fFormat),
                          fBindFlags,
                          static_cast<unsigned int>(fUsage));
}

bool D3D11TextureInfo::isCompatible(const TextureInfo& that, bool requireExact) const {
    const auto& dt = TextureInfoPriv::Get<D3D11TextureInfo>(that);
    return this->getViewFormat() == dt.getViewFormat() &&
           (fBindFlags & dt.fBindFlags) == fBindFlags;
}

namespace TextureInfos {

TextureInfo MakeD3D11(const D3D11TextureInfo& d3d11Info) {
    return TextureInfoPriv::Make(d3d11Info);
}

bool GetD3D11TextureInfo(const TextureInfo& info, D3D11TextureInfo* out) {
    return TextureInfoPriv::Copy(info, out);
}

}  // namespace TextureInfos

class D3D11BackendTextureData final : public BackendTextureData {
public:
    D3D11BackendTextureData(Microsoft::WRL::ComPtr<ID3D11Texture2D> tex)
            : fTexture(std::move(tex)) {}

#if defined(SK_DEBUG)
    skgpu::BackendApi type() const override { return skgpu::BackendApi::kD3D11; }
#endif

    ID3D11Texture2D* texture() const { return fTexture.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D> fTexture;

    void copyTo(AnyBackendTextureData& dstData) const override {
        dstData.emplace<D3D11BackendTextureData>(fTexture);
    }

    bool equal(const BackendTextureData* that) const override {
        SkASSERT(!that || that->type() == skgpu::BackendApi::kD3D11);
        if (auto other = static_cast<const D3D11BackendTextureData*>(that)) {
            return fTexture == other->fTexture;
        }
        return false;
    }
};

namespace BackendTextures {

BackendTexture MakeD3D11(SkISize dimensions,
                         const D3D11TextureInfo& info,
                         ID3D11Texture2D* texture) {
    return BackendTexturePriv::Make(dimensions,
                                    TextureInfos::MakeD3D11(info),
                                    D3D11BackendTextureData(texture));
}

ID3D11Texture2D* GetD3D11Texture2D(const BackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != skgpu::BackendApi::kD3D11) {
        return nullptr;
    }
    const auto* data =
            static_cast<const D3D11BackendTextureData*>(BackendTexturePriv::GetData(tex));
    return data ? data->texture() : nullptr;
}

}  // namespace BackendTextures

}  // namespace skgpu::graphite
