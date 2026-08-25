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
        uint16_t fTexPage    = 0;       // Atlas page index (0 or 1, or kInvalidTexPage)
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
        ends.clearProxies();
        if (fPages[0].isValid()) {
            ends.addProxy(fPages[0].fTexture);
        }
        if (fPages[1].isValid()) {
            ends.addProxy(fPages[1].fTexture);
        }
    }

    template <uint16_t kTileHeight>
    bool resolveNullCaps(EndCaps<kTileHeight>& ends) {
        int firstNull = ends.firstNullCapIndex();
        if (firstNull == -1) {
            return true;
        }
        int oldSlot = fActiveSlot ^ 1;
        if (fPages[oldSlot].isValid()) {
            fPages[oldSlot].reset();
        }
        if (!this->createPageInSlot(oldSlot, fNullAlphasBytes)) {
            return false;
        }
        TexturePage* newPage = &fPages[oldSlot];
        newPage->fUsedBytes = fNullAlphasBytes;

        std::memcpy(fAlphaBuffer.data() + newPage->fBufferOffset,
                    fAlphaBuffer.data() + fNullAlphasOffset,
                    fNullAlphasBytes);
        fAlphaBuffer.resize(std::max(fPages[0].fBufferOffset + fPages[0].fCapacityBytes,
                                     fPages[1].fBufferOffset + fPages[1].fCapacityBytes));
        fNullAlphasOffset = 0;
        fNullAlphasBytes = 0;
        fActiveSlot = oldSlot;

        auto& caps = ends.caps();
        for (int i = firstNull; i < caps.size(); ++i) {
            caps[i].fTexPage = static_cast<uint16_t>(oldSlot);
        }

        this->populateProxies(ends);
        return true;
    }

private:
    struct TexturePage {
        sk_sp<TextureProxy> fTexture;
        int32_t fRowCount = 0;
        int32_t fCapacityBytes = 0;
        int32_t fUsedBytes = 0;
        int32_t fUploadedRows = 0;
        int32_t fBufferOffset = 0;

        bool isValid() const { return fTexture != nullptr; }
        void reset() {
            fTexture = nullptr;
            fRowCount = 0;
            fCapacityBytes = 0;
            fUsedBytes = 0;
            fUploadedRows = 0;
            fBufferOffset = 0;
        }
    };

    bool createPageInSlot(int slot, int32_t minRequiredBytes = 0);

    Recorder* fRecorder;
    TexturePage fPages[2];
    int fActiveSlot = 0;
    int32_t fNextPageRowCount = SparseStripConfig::kInitialAtlasRows;
    SkTDArray<uint8_t> fAlphaBuffer;
    int32_t fNullAlphasOffset = 0;
    int32_t fNullAlphasBytes = 0;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_AlphaAtlasManager_DEFINED
