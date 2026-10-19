/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_GpuAlphaAtlasManager_DEFINED
#define skgpu_graphite_sparse_strips_GpuAlphaAtlasManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/sparse_strips/PreMergeTile.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"

#include <cstdint>
#include <optional>
#include <utility>

namespace skgpu::graphite {

class DrawContext;
class Recorder;

// The GPU coverage counterpart to AlphaAtlasManager, used by MakeStrips::MsaaGpu. It hands out
// alpha atlas space with the same two page, flip-flopping scheme (including the null slot
// overflow), but never holds CPU-side alphas. Instead, MakeStrips only reserves space and stages
// PreMergeTiles (PMTs), and the coverage compute shader writes the alphas directly into the pages,
// which are therefore storage textures.
//
// Since a boundary run's page and offset are only known once the whole run is finalized, its PMTs
// are staged until then, at which point they are committed to the batch of the page they target.
// At flush, each page's batch is consumed by the coverage dispatch for that page.
class GpuAlphaAtlasManager {
public:
    explicit GpuAlphaAtlasManager(Recorder* recorder);
    ~GpuAlphaAtlasManager();

    static constexpr uint16_t kNullSlot = SparseStripConfig::kMaxTexturePages;

    // Appends an uninitialized PMT to the current run.
    PreMergeTile* stagePmt() { return fStagedRun.append(); }

    // The most recently staged PMT of the current run, which must be non-empty.
    PreMergeTile& lastStagedPmt() {
        SkASSERT(!fStagedRun.empty());
        return fStagedRun.back();
    }

    // Reserves `numBytes` of atlas space at the end of the current run, and returns the byte offset
    // of the reservation from the start of the run. Nothing is materialized; the run's page and
    // offset are decided by finalizeRun().
    std::optional<int32_t> reserveAlphaSpace(int32_t numBytes);

    // Finalizes the current boundary run. The run is placed in the active page, or the other page
    // if it does not fit, or the null slot if both pages are in use. Its staged PMTs are then
    // committed to the batch of the chosen slot. Returns a pair of {alphaIndex, texPage} on success,
    // which indicate the slot-local byte offset and slot (0, 1, or kNullSlot) of the run.
    std::optional<std::pair<int32_t, uint16_t>> finalizeRun();

    // Discards the current run's staged PMTs and reservations, e.g. if MakeStrips fails.
    void abandonRun();

    // Called at flush time, before the draws that sample the pages are recorded.
    void recordDispatches(DrawContext* dc);
    void freeGpuResources();
    bool resolveNullCaps(EndCaps* ends);
    void populateProxies(EndCaps* ends) const;

    size_t numPages() const {
        size_t numPages = 0;
        for (int32_t i = 0; i < SparseStripConfig::kMaxTexturePages; ++i) {
            if (fPages[i].isValid()) {
                numPages++;
            }
        }
        return numPages;
    }
    sk_sp<TextureProxy> getPageProxy(size_t index) const {
        return (index < SparseStripConfig::kMaxTexturePages && fPages[index].isValid()) ?
                fPages[index].fTexture : nullptr;
    }
    // The PMTs committed to a slot (including kNullSlot) that have not been dispatched yet.
    SkSpan<const PreMergeTile> getPagePmts(size_t index) const {
        if (index == kNullSlot) {
            return {fNullPmts.data(), static_cast<size_t>(fNullPmts.size())};
        }
        if (index < SparseStripConfig::kMaxTexturePages && fPages[index].isValid()) {
            return {fPages[index].fPmts.data(), static_cast<size_t>(fPages[index].fPmts.size())};
        }
        return {};
    }
    int activeSlot() const { return fActiveSlot; }
    bool hasNullPmts() const { return !fNullPmts.empty(); }

private:
    static constexpr int32_t kInvalidSlot = -1;

    struct TexturePage {
        sk_sp<TextureProxy> fTexture;
        int32_t fRowCount = 0;
        int32_t fCapacityBytes = 0;
        int32_t fUsedBytes = 0;
        // The PMTs committed to this page that have not been dispatched yet.
        SkTDArray<PreMergeTile> fPmts;

        bool isValid() const { return fTexture != nullptr; }
        void reset() {
            fTexture = nullptr;
            fRowCount = 0;
            fCapacityBytes = 0;
            fUsedBytes = 0;
            fPmts.clear();
        }
    };

    bool createPageInSlot(int32_t slot, int32_t minRequiredBytes);
    std::optional<int32_t> placeRun(int32_t runBytes);

    Recorder* fRecorder;
    TexturePage fPages[SparseStripConfig::kMaxTexturePages];

    // The PMTs and reserved bytes of the run being generated, whose alpha indices are relative to
    // the start of the run until it is finalized.
    SkTDArray<PreMergeTile> fStagedRun;
    int32_t fStagedRunBytes;

    // Overflow for when both pages are in use, the counterpart of AlphaAtlasManager's null buffer.
    // fNullRunStarts holds the index of the first PMT of each null run, i.e. null cap, in fNullPmts.
    SkTDArray<PreMergeTile> fNullPmts;
    SkTDArray<int32_t> fNullRunStarts;
    int32_t fNullUsedBytes;

    // The slot that runs are currently placed in, which may be kNullSlot.
    int32_t fActiveSlot;
    // The slot of the last non-null texture page which a run was placed in.
    int32_t fLastGpuSlot;
    // The slot of the last non-null texture page which was retired. Is only populated in null caps
    // scenario, invalid otherwise.
    int32_t fLastRetiredSlot;
    int32_t fNextPageRowCount;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_GpuAlphaAtlasManager_DEFINED
