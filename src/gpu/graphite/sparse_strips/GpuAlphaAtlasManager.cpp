/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/sparse_strips/GpuAlphaAtlasManager.h"

#include "include/core/SkSize.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/SkLog.h"
#include "include/private/SkTo.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ComputeProvider.h"
#include "src/gpu/graphite/ComputeTypes.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsCoverageStep.h"
#include "src/gpu/graphite/task/ComputeTask.h"

#include <algorithm>
#include <memory>
#include <tuple>

namespace skgpu::graphite {

namespace {

// Appends `run` to `dst`, offsetting the alpha index of each of its end tiles by `alphaDelta`.
void append_run(SkSpan<const PreMergeTile> run, int32_t alphaDelta, SkTDArray<PreMergeTile>* dst) {
    PreMergeTile* out = dst->append(SkToInt(run.size()));
    for (const PreMergeTile& pmt : run) {
        *out = pmt;
        if (pmt.isEndTile()) {
            out->fAlphaIndex =
                    static_cast<uint32_t>(static_cast<int32_t>(pmt.fAlphaIndex) + alphaDelta);
        }
        ++out;
    }
}

// Returns the dispatch that rasterizes a page's batch of PreMergeTiles into the page.
std::unique_ptr<DispatchGroup> make_coverage_dispatch(Recorder* recorder,
                                                      const SparseStripsCoverageStep* step,
                                                      BindBufferInfo maskLUT,
                                                      SkSpan<const PreMergeTile> pmts,
                                                      sk_sp<TextureProxy> page) {
    using Step = SparseStripsCoverageStep;
    const uint32_t tileCount = CountTiles(pmts);
    SkASSERT(tileCount > 0);
    const WorkgroupSize dispatchSize = Step::DispatchSize(tileCount);
    const uint32_t dispatchedTiles = Step::DispatchedTiles(dispatchSize);

    DrawBufferManager* bufferManager = recorder->priv().drawBufferManager();
    DrawBufferManager::MappedAllocationInfo tiles =
            bufferManager->getMappedStorageBuffer(dispatchedTiles, sizeof(TileWork));
    DrawBufferManager::MappedAllocationInfo pmtBuffer =
            bufferManager->getMappedStorageBuffer(pmts.size(), sizeof(PreMergeTile));
    BufferWriter& tileWriter = std::get<BufferWriter>(tiles);
    BufferWriter& pmtWriter = std::get<BufferWriter>(pmtBuffer);
    if (!tileWriter || !pmtWriter || !maskLUT) {
        return nullptr;
    }
    // Every workgroup reads 8 TileWorks, so the last one is padded with empty ones.
    ForEachTileWork(pmts, [&tileWriter](const TileWork& tile) { tileWriter.write(tile); });
    tileWriter.zeroBytes((dispatchedTiles - tileCount) * sizeof(TileWork));
    pmtWriter.write(pmts.data(), pmts.size_bytes());

    DispatchGroup::Builder builder(recorder);
    builder.assignSharedBuffer(std::get<BindBufferInfo>(tiles), Step::kTilesSlot);
    builder.assignSharedBuffer(std::get<BindBufferInfo>(pmtBuffer), Step::kPmtsSlot);
    builder.assignSharedBuffer(maskLUT, Step::kMaskLUTSlot);
    builder.assignSharedTexture(std::move(page), Step::kAtlasSlot);
    if (!builder.appendStep(step, dispatchSize)) {
        return nullptr;
    }
    return builder.finalize();
}

}  // namespace

GpuAlphaAtlasManager::GpuAlphaAtlasManager(Recorder* recorder)
        : fRecorder(recorder)
        , fStagedRunBytes(0)
        , fNullUsedBytes(0)
        , fActiveSlot(0)
        , fLastGpuSlot(0)
        , fLastRetiredSlot(kInvalidSlot)
        , fNextPageRowCount(SparseStripConfig::kInitialAtlasRows) {}

GpuAlphaAtlasManager::~GpuAlphaAtlasManager() = default;

bool GpuAlphaAtlasManager::createPageInSlot(int32_t slot, int32_t minRequiredBytes) {
    SkASSERT(slot >= 0 && slot < SparseStripConfig::kMaxTexturePages);
    SkASSERT(!fPages[slot].isValid());

    int32_t rows = fNextPageRowCount;
    if (minRequiredBytes > 0) {
        int32_t neededRows = (minRequiredBytes + SparseStripConfig::kAtlasWidthBytes - 1) /
                             SparseStripConfig::kAtlasWidthBytes;
        if (neededRows > SparseStripConfig::kMaxAtlasRows) {
            return false;
        }
        rows = std::max(rows, neededRows);
    }
    rows = std::min(rows, SparseStripConfig::kMaxAtlasRows);

    // The coverage compute shader writes the alphas directly into the page, so in addition to being
    // sampled by EndCapRenderStep, it must be bindable as a storage texture.
    const Caps* caps = fRecorder->priv().caps();
    TextureInfo info = caps->getDefaultStorageTextureInfo(SparseStripConfig::kColorType);

    sk_sp<TextureProxy> proxy =
            TextureProxy::Make(caps,
                               fRecorder->priv().resourceProvider(),
                               SkISize::Make(SparseStripConfig::kAtlasWidth, rows),
                               info,
                               Budgeted::kYes,
                               "GpuAlphaAtlas");
    if (!proxy) {
        return false;
    }

    TexturePage& page = fPages[slot];
    page.fTexture = std::move(proxy);
    page.fRowCount = rows;
    page.fCapacityBytes = rows * SparseStripConfig::kAtlasWidthBytes;
    page.fUsedBytes = 0;
    page.fPmts.clear();

    // Double row count for the next created page, clamped to max atlas rows
    fNextPageRowCount = std::min(SparseStripConfig::kMaxAtlasRows, rows * 2);
    return true;
}

std::optional<int32_t> GpuAlphaAtlasManager::reserveAlphaSpace(int32_t numBytes) {
    // A run is addressed by the single alphaIndex of its EndCap, so it must fit within one page.
    if (numBytes <= 0 || numBytes > SparseStripConfig::kMaxAtlasBytes - fStagedRunBytes) {
        return std::nullopt;
    }
    int32_t runOffset = fStagedRunBytes;
    fStagedRunBytes += numBytes;
    return runOffset;
}

// Unlike AlphaAtlasManager, which must append alphas to a page as they are rasterized, and then
// relocate runs that straddle its capacity, the size of the run is already known here. So, the
// run is placed in the active page if it fits, else in the other page if it is free, else in the
// null slot, where all subsequent runs are placed until resolveNullCaps() migrates them to a page.
std::optional<int32_t> GpuAlphaAtlasManager::placeRun(int32_t runBytes) {
    if (fActiveSlot == kNullSlot) {
        return kNullSlot;
    }

    if (!fPages[fActiveSlot].isValid()) {
        if (!this->createPageInSlot(fActiveSlot, runBytes)) {
            return std::nullopt;
        }
        return fActiveSlot;
    }

    const TexturePage& activePage = fPages[fActiveSlot];
    if (activePage.fUsedBytes + runBytes <= activePage.fCapacityBytes) {
        return fActiveSlot;
    }

    int32_t nextSlot = fActiveSlot ^ 1;
    if (fPages[nextSlot].isValid()) {
        // Both pages are in use, transition to the null slot.
        fActiveSlot = kNullSlot;
        return kNullSlot;
    }
    if (!this->createPageInSlot(nextSlot, runBytes)) {
        return std::nullopt;
    }
    fActiveSlot = nextSlot;
    return fActiveSlot;
}

std::optional<std::pair<int32_t, uint16_t>> GpuAlphaAtlasManager::finalizeRun() {
    SkASSERT(!fStagedRun.empty() && fStagedRunBytes > 0);

    std::optional<int32_t> slot = this->placeRun(fStagedRunBytes);
    if (!slot) {
        this->abandonRun();
        return std::nullopt;
    }

    int32_t alphaIndex;
    if (*slot == kNullSlot) {
        alphaIndex = fNullUsedBytes;
        fNullRunStarts.push_back(fNullPmts.size());
        append_run(SkSpan(fStagedRun.data(), fStagedRun.size()), alphaIndex, &fNullPmts);
        fNullUsedBytes += fStagedRunBytes;
    } else {
        TexturePage& page = fPages[*slot];
        alphaIndex = page.fUsedBytes;
        append_run(SkSpan(fStagedRun.data(), fStagedRun.size()), alphaIndex, &page.fPmts);
        page.fUsedBytes += fStagedRunBytes;
        fLastGpuSlot = *slot;
    }

    fStagedRun.clear();
    fStagedRunBytes = 0;
    return std::make_pair(alphaIndex, static_cast<uint16_t>(*slot));
}

void GpuAlphaAtlasManager::abandonRun() {
    fStagedRun.clear();
    fStagedRunBytes = 0;
}

void GpuAlphaAtlasManager::recordDispatches(DrawContext* dc) {
    const ComputeProvider* computeProvider = fRecorder->priv().computeProvider();
    // The GPU atlas only exists for PathRendererStrategy::kGPUSparseStripsMSAA8, which always has
    // the coverage step.
    const SparseStripsCoverageStep* step = computeProvider->sparseStripsCoverageStep();
    SkASSERT(step);

    ComputeTask::DispatchGroupList dispatches;
    for (TexturePage& page : fPages) {
        if (!page.isValid() || page.fPmts.empty()) {
            continue;
        }
        std::unique_ptr<DispatchGroup> dispatch =
                make_coverage_dispatch(fRecorder,
                                       step,
                                       computeProvider->sparseStripsMaskLUT(),
                                       SkSpan(page.fPmts.data(), page.fPmts.size()),
                                       page.fTexture);
        if (dispatch) {
            dispatches.push_back(std::move(dispatch));
        } else {
            SKIA_LOG_E("Failed to record the sparse strips coverage dispatch.");
        }
        page.fPmts.clear();
    }
    if (!dispatches.empty()) {
        // Added to the DrawContext now, so the dispatches run before the draws of this flush, which
        // sample the pages.
        dc->recordDependency(ComputeTask::Make(std::move(dispatches)));
    }

    // At flush time, retire the older GPU slot if both were valid.
    if (fPages[0].isValid() && fPages[1].isValid()) {
        // If we're on the null slot, the active slot is instead the last gpu slot.
        int32_t activeGpuSlot =
                (fActiveSlot < SparseStripConfig::kMaxTexturePages) ? fActiveSlot : fLastGpuSlot;
        int32_t oldSlot = activeGpuSlot ^ 1;
        SkASSERT(fPages[oldSlot].isValid());
        fPages[oldSlot].reset();
        fLastRetiredSlot = oldSlot;
    }
}

bool GpuAlphaAtlasManager::resolveNullCaps(EndCaps* ends) {
    SkASSERT(ends && ends->hasNullCaps() && !fNullPmts.empty());
    int32_t firstNull = ends->firstNullCapIndex();
    auto& caps = ends->caps();
    SkASSERT(firstNull >= 0 && firstNull < caps.size());

    // The null caps of `ends` are the most recently finalized null runs.
    int32_t numNullCaps = caps.size() - firstNull;
    SkASSERT(numNullCaps <= fNullRunStarts.size());
    int32_t firstNullRun = fNullRunStarts.size() - numNullCaps;

    // Use the slot that was freed at flush time
    if (fLastRetiredSlot == kInvalidSlot) {
        return false;
    }
    SkASSERT(!fPages[fLastRetiredSlot].isValid());
    int32_t currentSlot = fLastRetiredSlot;

    // Allocate currentSlot up to max capacity
    int32_t nullBytes = fNullUsedBytes - caps[firstNull].fAlphaIndex;
    if (!this->createPageInSlot(currentSlot,
                                std::min(nullBytes, SparseStripConfig::kMaxAtlasBytes))) {
        return false;
    }
    fLastRetiredSlot = kInvalidSlot;

    // Move the PMTs of each null cap into the new texture pages.
    int32_t nextNull = firstNull;
    int32_t roundStartCap = firstNull;
    for (; nextNull < caps.size(); ++nextNull) {
        int32_t alphaIndex = caps[nextNull].fAlphaIndex;
        int32_t capBytes = caps[nextNull].fWidth * SparseStripConfig::kTileHeight;
        SkASSERT(capBytes > 0);

        // If the run exceeds the current slot, try to instantiate the other page
        if (fPages[currentSlot].fUsedBytes + capBytes > fPages[currentSlot].fCapacityBytes) {
            int32_t otherSlot = currentSlot ^ 1;
            if (!fPages[otherSlot].isValid() || fPages[otherSlot].fUsedBytes == 0) {
                if (fPages[otherSlot].isValid()) {
                    fPages[otherSlot].reset();
                }
                int32_t remainingBytes = fNullUsedBytes - alphaIndex;
                if (!this->createPageInSlot(
                            otherSlot,
                            std::min(remainingBytes, SparseStripConfig::kMaxAtlasBytes))) {
                    break;
                }
                if (fPages[otherSlot].fUsedBytes + capBytes > fPages[otherSlot].fCapacityBytes) {
                    fPages[otherSlot].reset();
                    break;
                }
                currentSlot = otherSlot;
            } else {
                // Both slots have been filled for this round
                break;
            }
        }

        int32_t run = firstNullRun + (nextNull - firstNull);
        int32_t pmtStart = fNullRunStarts[run];
        int32_t pmtEnd =
                (run + 1 < fNullRunStarts.size()) ? fNullRunStarts[run + 1] : fNullPmts.size();
        TexturePage& page = fPages[currentSlot];
        append_run(SkSpan(fNullPmts.data() + pmtStart, pmtEnd - pmtStart),
                   page.fUsedBytes - alphaIndex,
                   &page.fPmts);

        caps[nextNull].fTexPage = static_cast<uint16_t>(currentSlot);
        caps[nextNull].fAlphaIndex = page.fUsedBytes;
        page.fUsedBytes += capBytes;
    }

    if (nextNull == roundStartCap) {
        // Made zero progress
        return false;
    }

    ends->setDrawStartIndex(roundStartCap);
    if (nextNull == caps.size()) {
        // We're done, no more rounds of flushing are necessary.
        ends->clearNullCaps();
        fNullPmts.clear();
        fNullRunStarts.clear();
        fNullUsedBytes = 0;
    } else {
        ends->setFirstNullCapIndex(nextNull);
    }

    this->populateProxies(ends);

    fActiveSlot = currentSlot;
    fLastGpuSlot = currentSlot;
    return true;
}

void GpuAlphaAtlasManager::populateProxies(EndCaps* ends) const {
    if (ends->empty()) {
        return;
    }
    int32_t startSlot = 0;
    while (startSlot < SparseStripConfig::kMaxTexturePages && !fPages[startSlot].isValid()) {
        startSlot++;
    }
    SkASSERT(startSlot < SparseStripConfig::kMaxTexturePages);

    sk_sp<TextureProxy> lastValid = fPages[startSlot].fTexture;
    for (int32_t i = 0; i < SparseStripConfig::kMaxTexturePages; ++i) {
        int32_t slot = (startSlot + i) % SparseStripConfig::kMaxTexturePages;
        if (fPages[slot].isValid()) {
            lastValid = fPages[slot].fTexture;
        }
        ends->setProxy(slot, lastValid);
    }
}

void GpuAlphaAtlasManager::freeGpuResources() {
    for (int32_t i = 0; i < SparseStripConfig::kMaxTexturePages; ++i) {
        fPages[i].reset();
    }
    this->abandonRun();
    fNullPmts.clear();
    fNullRunStarts.clear();
    fNullUsedBytes = 0;
    fActiveSlot = 0;
    fLastGpuSlot = 0;
    fLastRetiredSlot = kInvalidSlot;
    fNextPageRowCount = SparseStripConfig::kInitialAtlasRows;
}

}  // namespace skgpu::graphite
