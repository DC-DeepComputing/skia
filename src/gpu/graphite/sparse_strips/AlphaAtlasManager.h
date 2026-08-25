/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_AlphaAtlasManager_DEFINED
#define skgpu_graphite_AlphaAtlasManager_DEFINED

#include "include/core/SkColorType.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <vector>

namespace skgpu::graphite {

class DrawContext;
class Recorder;
template <uint16_t kTileHeight> class EndCaps;

class AlphaAtlasManager {
public:
    struct AlphaAllocation {
        uint8_t* fWritePtr   = nullptr;
        int32_t  fAlphaIndex = 0;       // Page-local byte offset
        uint16_t fTexPage    = 0;       // Atlas page index (0 or 1)
    };

    AlphaAtlasManager(Recorder* recorder);

    std::optional<AlphaAllocation> requestAlphaSpace(int32_t numBytes);

    void recordUploads(DrawContext* dc);
    void freeGpuResources();

    size_t numPages() const {
        return (fPages[0].isValid() ? 1 : 0) + (fPages[1].isValid() ? 1 : 0);
    }
    int32_t activePageRowCount() const {
        return fPages[fActiveSlot].isValid() ? fPages[fActiveSlot].fRowCount : 0;
    }
    sk_sp<TextureProxy> getPageProxy(size_t index) const {
        return (index < 2 && fPages[index].isValid()) ? fPages[index].fTexture : nullptr;
    }
    int activeSlot() const { return fActiveSlot; }

    template <uint16_t kTileHeight>
    void populateProxies(EndCaps<kTileHeight>& ends) const {
        if (fPages[0].isValid()) {
            ends.addProxy(fPages[0].fTexture);
        }
        if (fPages[1].isValid()) {
            ends.addProxy(fPages[1].fTexture);
        }
    }

private:
    struct TexturePage {
        sk_sp<TextureProxy> fTexture;
        int32_t fRowCount = 0;
        int32_t fCapacityBytes = 0;
        int32_t fUsedBytes = 0;
        int32_t fUploadedRows = 0;
        SkTDArray<uint8_t> fAlphaBuffer;

        bool isValid() const { return fTexture != nullptr; }
        void reset() {
            fTexture = nullptr;
            fRowCount = 0;
            fCapacityBytes = 0;
            fUsedBytes = 0;
            fUploadedRows = 0;
            fAlphaBuffer.clear();
        }
    };

    bool createPageInSlot(int slot, int32_t minRequiredBytes = 0);

    Recorder* fRecorder;
    TexturePage fPages[2];
    int fActiveSlot = 0;
    int32_t fNextPageRowCount = SparseStripConfig::kInitialAtlasRows;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_AlphaAtlasManager_DEFINED
