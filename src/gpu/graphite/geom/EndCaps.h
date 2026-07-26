/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_EndCaps_DEFINED
#define skgpu_graphite_geom_EndCaps_DEFINED

#include <cstdint>
#include <utility>
#include <vector>
#include "include/private/SkLog.h"
#include "include/private/SkTArray.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"

namespace skgpu::graphite {

template <uint16_t kTileHeight> class EndCaps {
public:
    struct EndCap {
        EndCap(uint16_t x, uint16_t y, uint16_t width, int32_t alphaIndex)
                : fX(x), fY(y), fWidth(width), fAlphaIndex(alphaIndex) {}

        uint16_t fX;
        uint16_t fY;
        uint16_t fWidth;
        int32_t fAlphaIndex;
    };

    EndCaps() = default;

    void addCap(uint16_t x,
                uint16_t y,
                uint16_t width,
                int32_t alphaIndex) {
        fCaps.push_back(EndCap(x, y, width, alphaIndex));
    }

    void addProxy(sk_sp<TextureProxy> texture) {
        if (fProxies.size() < SparseStripConfig::kMaxTexturePages) {
            fProxies.push_back(texture);
        } else {
            SKIA_LOG_W("Warning, exceeded max textures per endcap renderstep");
        }
    }

    const std::vector<EndCap>& caps() const { return fCaps; }
    const skia_private::TArray<sk_sp<TextureProxy>>& proxies() const { return fProxies; }

    bool empty() const { return fCaps.empty(); }
    size_t size() const { return fCaps.size(); }
    void clear() {
        fCaps.clear();
        fProxies.clear();
    }

private:
    std::vector<EndCap> fCaps;
    skia_private::STArray<SparseStripConfig::kMaxTexturePages, sk_sp<TextureProxy>> fProxies;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_geom_EndCaps_DEFINED
