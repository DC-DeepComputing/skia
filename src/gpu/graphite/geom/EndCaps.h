/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_EndCaps_DEFINED
#define skgpu_graphite_geom_EndCaps_DEFINED

#include <array>
#include <cstdint>
#include <type_traits>
#include <utility>
#include "include/private/SkLog.h"
#include "include/private/SkTArray.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"

namespace skgpu::graphite {

// Container holding the "EndCaps" produced by SparseStrips rendering. An EndCap stores the
// antialiased portion of the path rendering, and spans a variable non-zero number of tiles.
//
// Each EndCap's fTexPage indexes this container's per-draw binding table (proxies()), where slots
// [0, kMaxTexturePages) hold the alpha atlas' regular pages and kOverflowSlot holds its overflow
// page. Caps that spilled into the overflow page ("overflow caps") always form a contiguous suffix.
// Until the atlas commits them, their alpha indices refer to the atlas' CPU staging buffer.
class EndCaps {
public:
    struct EndCap {
        EndCap(uint16_t x, uint16_t y, uint16_t width, int32_t alphaIndex, uint16_t texPage)
                : fX(x), fY(y), fWidth(width), fTexPage(texPage), fAlphaIndex(alphaIndex)
                , fPadding(0xffffffff) {}

        uint16_t fX;            // Top left coordinates of the EndCap.
        uint16_t fY;            // ``
        uint16_t fWidth;        // The width of the EndCap in pixels
        uint16_t fTexPage;      // The associated page in the EndCap's backing texture
        int32_t  fAlphaIndex;   // The EndCap's offset into the alpha buffer.
        uint32_t fPadding;      // Pad to vec4 alignment
    };
    static_assert(sizeof(EndCap) == 16);
    static_assert(std::is_trivially_copyable_v<EndCap>);

    using ProxyArray = std::array<sk_sp<TextureProxy>, SparseStripConfig::kNumAlphaBindings>;

    EndCaps() = default;

    void addCap(uint16_t x,
                uint16_t y,
                uint16_t width,
                int32_t alphaIndex,
                uint16_t texPage) {
        fCaps.push_back(EndCap(x, y, width, alphaIndex, texPage));
    }

    // Marks the next added cap as the first overflow cap, if no overflow cap has been added yet.
    void markFirstOverflowCap() {
        if (fFirstOverflowCapIndex == kInvalidIndex) {
            fFirstOverflowCapIndex = static_cast<int32_t>(fCaps.size());
        }
    }

    void setProxy(int slot, sk_sp<TextureProxy> proxy) {
        SkASSERT(slot >= 0 && slot < SparseStripConfig::kNumAlphaBindings);
        fProxies[slot] = std::move(proxy);
    }

    const skia_private::TArray<EndCap>& caps() const { return fCaps; }
    skia_private::TArray<EndCap>& caps() { return fCaps; }
    const ProxyArray& proxies() const { return fProxies; }

    bool empty() const { return fCaps.empty(); }
    size_t size() const { return fCaps.size(); }

    bool hasOverflowCaps() const { return fFirstOverflowCapIndex != kInvalidIndex; }
    int32_t firstOverflowCapIndex() const { return fFirstOverflowCapIndex; }

    // True while overflow caps still reference the atlas' CPU staging buffer rather than its
    // overflow page. Such caps must be committed before the EndCaps can be drawn.
    bool hasPendingOverflow() const { return this->hasOverflowCaps() && !fOverflowCommitted; }
    void markOverflowCommitted() {
        SkASSERT(this->hasOverflowCaps());
        fOverflowCommitted = true;
    }

    void clear() {
        fCaps.clear();
        for (auto& proxy : fProxies) {
            proxy.reset();
        }
        fFirstOverflowCapIndex = kInvalidIndex;
        fOverflowCommitted = false;
    }

private:
    static constexpr int32_t kInvalidIndex = -1;

    skia_private::TArray<EndCap> fCaps;
    ProxyArray fProxies;
    int32_t fFirstOverflowCapIndex = kInvalidIndex;
    bool fOverflowCommitted = false;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_geom_EndCaps_DEFINED
