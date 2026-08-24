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
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace skgpu::graphite {

class DrawContext;
class Recorder;

class AlphaAtlasManager {
public:
    struct AlphaAllocation {
        uint8_t* fWritePtr   = nullptr;
        int32_t  fAlphaIndex = 0;       // Page-local byte offset
        uint16_t fTexPage    = 0;       // Draw-local index into ends.proxies() (0 or 1)
        bool     fSuccess    = false;   // True if allocation succeeded within draw's max pages
    };

    AlphaAtlasManager(Recorder* recorder);

    AlphaAllocation requestAlphaSpace(skia_private::TArray<sk_sp<TextureProxy>>& proxies,
                                      int32_t numBytes);

    template <uint16_t kTileHeight>
    AlphaAllocation requestAlphaSpace(EndCaps<kTileHeight>& ends, int32_t numBytes) {
        return this->requestAlphaSpace(ends.proxyArray(), numBytes);
    }

    void recordUploads(DrawContext* dc);
    void freeGpuResources();

    size_t numPages() const { return fPages.size(); }
    int32_t activePageRowCount() const {
        return (fActivePageIndex >= 0 && fActivePageIndex < static_cast<int32_t>(fPages.size()))
                       ? fPages[fActivePageIndex].fRowCount
                       : 0;
    }
    sk_sp<TextureProxy> getPageProxy(size_t index) const {
        return (index < fPages.size()) ? fPages[index].fTexture : nullptr;
    }

private:
    struct TexturePage {
        sk_sp<TextureProxy> fTexture;
        int32_t fRowCount = SparseStripConfig::kInitialAtlasRows;
        int32_t fCapacityBytes = SparseStripConfig::kAtlasWidthBytes;
        int32_t fUsedBytes = 0;
        int32_t fUploadedRows = 0;
        SkTDArray<uint8_t> fAlphaBuffer;
    };

    bool createNewPage(int32_t minRequiredBytes = 0);

    Recorder* fRecorder;
    std::vector<TexturePage> fPages;
    int32_t fActivePageIndex = -1;
    int32_t fNextPageRowCount = SparseStripConfig::kInitialAtlasRows;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_AlphaAtlasManager_DEFINED
