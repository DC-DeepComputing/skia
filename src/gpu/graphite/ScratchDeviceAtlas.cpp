/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ScratchDeviceAtlas.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/BoundsManager.h"
#include "src/gpu/graphite/task/Task.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace skgpu::graphite {

namespace {

static constexpr int kDefaultAtlasDim = 2048;

} // anonymous namespace

AtlasPage::AtlasPage(sk_sp<TextureProxy> proxy,
                     sk_sp<DrawContext> dc,
                     SkISize maxDimensions)
        : fAtlasProxy(std::move(proxy))
        , fAtlasDC(std::move(dc))
        , fMaxDimensions(maxDimensions)
        , fPlottedDimensions(maxDimensions) {}

AtlasPage::~AtlasPage() = default;

sk_sp<DrawContext> AtlasPage::refDrawContext() const { return fAtlasDC; }

bool AtlasPage::canFit(SkISize slotSize, int padding) const {
    if (fIsPlotted) {
        return false;
    }
    int pw = slotSize.width() + 2 * padding;
    int ph = slotSize.height() + 2 * padding;
    if (pw > fMaxDimensions.width() || ph > fMaxDimensions.height()) {
        return false;
    }
    int64_t maxArea = static_cast<int64_t>(fMaxDimensions.width()) * fMaxDimensions.height();
    int64_t newArea = fTotalPaddedArea + static_cast<int64_t>(pw) * ph;
    if (newArea > maxArea) {
        return false;
    }
    // Fast path: when total padded area is <= 25% of the max page area and each slot is at most
    // kMaxSlotDim (512 <= 2048/4), skyline shelf packing is guaranteed to succeed.
    if (newArea <= maxArea / 4) {
        return true;
    }

    // Verify with offline height-sorted RectanizerSkyline against fMaxDimensions.
    std::vector<SkISize> sizes;
    sizes.reserve(fSlots.size() + 1);
    for (const auto& s : fSlots) {
        sizes.push_back(s.fSize);
    }
    sizes.push_back(slotSize);
    std::stable_sort(sizes.begin(), sizes.end(), [](const SkISize& a, const SkISize& b) {
        if (a.height() != b.height()) {
            return a.height() > b.height();
        }
        return a.width() > b.width();
    });

    skgpu::RectanizerSkyline rectanizer(fMaxDimensions.width(), fMaxDimensions.height());
    for (const auto& sz : sizes) {
        SkIPoint16 loc;
        if (!rectanizer.addPaddedRect(sz.width(), sz.height(), padding, &loc)) {
            return false;
        }
    }
    return true;
}

int AtlasPage::addSlot(SkISize slotSize, int padding) {
    SkASSERT(!fIsPlotted);
    fPadding = padding;
    int slotIdx = static_cast<int>(fSlots.size());
    fSlots.push_back({slotSize, {0, 0}});
    int pw = slotSize.width() + 2 * padding;
    int ph = slotSize.height() + 2 * padding;
    fTotalPaddedArea += static_cast<int64_t>(pw) * ph;
    return slotIdx;
}

void AtlasPage::ensurePlotted() {
    if (fIsPlotted) {
        return;
    }
    fIsPlotted = true;

    if (fSlots.empty()) {
        fPlottedDimensions = fMaxDimensions;
        return;
    }

    std::vector<int> order(fSlots.size());
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(), [this](int a, int b) {
        const SkISize& sa = fSlots[a].fSize;
        const SkISize& sb = fSlots[b].fSize;
        if (sa.height() != sb.height()) {
            return sa.height() > sb.height();
        }
        return sa.width() > sb.width();
    });

    int minW = 1;
    int minH = 1;
    for (const auto& s : fSlots) {
        minW = std::max(minW, s.fSize.width() + 2 * fPadding);
        minH = std::max(minH, s.fSize.height() + 2 * fPadding);
    }

    int estSide = std::max({minW, minH, static_cast<int>(std::ceil(std::sqrt(static_cast<double>(fTotalPaddedArea))))});
    SkISize approxMin = GetApproxSize(SkISize::Make(std::min(minW, fMaxDimensions.width()),
                                                    std::min(minH, fMaxDimensions.height())));
    SkISize candidate = GetApproxSize(SkISize::Make(std::min(estSide, fMaxDimensions.width()),
                                                    std::min(estSide, fMaxDimensions.height())));
    int tryW = std::max(candidate.width(), approxMin.width());
    int tryH = std::max(candidate.height(), approxMin.height());

    auto tryPack = [&](int w, int h, int* outMaxRight, int* outMaxBottom) -> bool {
        skgpu::RectanizerSkyline rectanizer(w, h);
        int maxR = 0;
        int maxB = 0;
        for (int idx : order) {
            SkIPoint16 loc;
            const SkISize& sz = fSlots[idx].fSize;
            if (!rectanizer.addPaddedRect(sz.width(), sz.height(), fPadding, &loc)) {
                return false;
            }
            fSlots[idx].fOffset = SkIPoint::Make(loc.fX, loc.fY);
            maxR = std::max(maxR, loc.fX + sz.width() + fPadding);
            maxB = std::max(maxB, loc.fY + sz.height() + fPadding);
        }
        *outMaxRight = maxR;
        *outMaxBottom = maxB;
        return true;
    };

    int maxRight = 0;
    int maxBottom = 0;
    while (!tryPack(tryW, tryH, &maxRight, &maxBottom)) {
        if (tryW >= fMaxDimensions.width() && tryH >= fMaxDimensions.height()) {
            break;
        }
        if (tryW <= tryH && tryW < fMaxDimensions.width()) {
            tryW = std::min(fMaxDimensions.width(), GetApproxSize(SkISize::Make(tryW + 1, 1)).width());
        } else if (tryH < fMaxDimensions.height()) {
            tryH = std::min(fMaxDimensions.height(), GetApproxSize(SkISize::Make(1, tryH + 1)).height());
        } else {
            tryW = fMaxDimensions.width();
            tryH = fMaxDimensions.height();
        }
    }

    SkISize tightSize = GetApproxSize(SkISize::Make(
            std::clamp(maxRight, 1, fMaxDimensions.width()),
            std::clamp(maxBottom, 1, fMaxDimensions.height())));
    fPlottedDimensions = SkISize::Make(std::min(tightSize.width(), tryW),
                                       std::min(tightSize.height(), tryH));

    if (fAtlasProxy && !fAtlasProxy->isInstantiated()) {
        fAtlasProxy->setDimensions(fPlottedDimensions);
    }
    if (fAtlasDC) {
        fAtlasDC->setDimensions(fPlottedDimensions);
    }
}

ScratchDeviceAtlas::ScratchDeviceAtlas(Recorder* recorder,
                                       const SkColorInfo& colorInfo,
                                       const SkSurfaceProps& props,
                                       Device* parentDevice)
        : fRecorder(recorder)
        , fColorInfo(colorInfo)
        , fSurfaceProps(props)
        , fParentDevice(parentDevice)
        , fCurrentDepth(DrawOrder::kClearDepth) {}

ScratchDeviceAtlas::~ScratchDeviceAtlas() {
    this->reset();
}

bool ScratchDeviceAtlas::isEligible(const SkImageInfo& info) const {
    if (info.width() <= 0 || info.height() <= 0) {
        return false;
    }
    if (info.width() > kMaxSlotDim || info.height() > kMaxSlotDim) {
        return false;
    }
    if (info.colorInfo() != fColorInfo) {
        return false;
    }
    if (info.alphaType() == kUnknown_SkAlphaType ||
        info.alphaType() == kUnpremul_SkAlphaType) {
        return false;
    }
    if (!fRecorder || !fRecorder->priv().caps()->useScratchDeviceAtlas()) {
        return false;
    }
    const Caps* caps = fRecorder->priv().caps();
    TextureInfo textureInfo = caps->getDefaultSampledTextureInfo(info.colorType(),
                                                                 Mipmapped::kNo,
                                                                 fRecorder->priv().isProtected(),
                                                                 Renderable::kYes);
    if (!caps->isRenderable(textureInfo) ||
        !AreColorTypeAndFormatCompatible(info.colorType(),
                                         TextureInfoPriv::ViewFormat(textureInfo))) {
        return false;
    }
    return true;
}

bool ScratchDeviceAtlas::canFit(SkISize size) const {
    if (size.width() <= 0 || size.height() <= 0 ||
        size.width() > kMaxSlotDim || size.height() > kMaxSlotDim || !fRecorder) {
        return false;
    }
    int maxDim = std::min(kDefaultAtlasDim, (int)fRecorder->priv().caps()->maxTextureSize());
    return size.width() + 2 * kSlotPadding <= maxDim &&
           size.height() + 2 * kSlotPadding <= maxDim;
}

bool ScratchDeviceAtlas::initAtlas() {
    if (fCurrentPage) {
        return true;
    }
    if (!fRecorder) {
        return false;
    }

    const Caps* caps = fRecorder->priv().caps();
    int maxDim = std::min(kDefaultAtlasDim, (int)caps->maxTextureSize());
    SkISize maxDimensions = SkISize::Make(maxDim, maxDim);

    auto textureInfo = caps->getDefaultSampledTextureInfo(fColorInfo.colorType(),
                                                          Mipmapped::kNo,
                                                          fRecorder->priv().isProtected(),
                                                          Renderable::kYes);
    sk_sp<TextureProxy> proxy = TextureProxy::Make(caps,
                                                   fRecorder->priv().resourceProvider(),
                                                   maxDimensions,
                                                   textureInfo,
                                                   skgpu::Budgeted::kYes,
                                                   "ScratchDeviceAtlas");
    if (!proxy) {
        return false;
    }

    sk_sp<DrawContext> dc = DrawContext::Make(caps,
                                              proxy,
                                              maxDimensions,
                                              fColorInfo,
                                              fSurfaceProps,
                                              /*allowUnpremul=*/false);
    if (!dc) {
        return false;
    }

    dc->clear(SkColors::kTransparent);

    fCurrentPage = sk_make_sp<AtlasPage>(std::move(proxy), std::move(dc), maxDimensions);
    fCurrentDepth = DrawOrder::kClearDepth;

    return true;
}

sk_sp<Device> ScratchDeviceAtlas::createDevice(const SkImageInfo& info,
                                               const SkSurfaceProps& props) {
    if (!this->isEligible(info)) {
        return nullptr;
    }

    if (!this->initAtlas()) {
        return nullptr;
    }

    if (!fCurrentPage->canFit(info.dimensions(), kSlotPadding)) {
        // Current page is full. Flush all atlases on the parent device in dependency order
        // (fChildAtlas -> fFilterAtlas[0] -> fFilterAtlas[1] -> ...) and start a new page.
        if (fParentDevice) {
            fParentDevice->flushAtlases();
        } else {
            this->flush(fRecorder);
            this->reset();
        }
        if (!this->initAtlas() || !fCurrentPage->canFit(info.dimensions(), kSlotPadding)) {
            return nullptr;
        }
    }

    int slotIndex = fCurrentPage->addSlot(info.dimensions(), kSlotPadding);
    return Device::MakeAtlased(fRecorder,
                               fCurrentPage,
                               slotIndex,
                               info.dimensions(),
                               info.colorInfo(),
                               props,
                               this);
}

void ScratchDeviceAtlas::flush(Recorder* recorder) {
    if (fCurrentPage) {
        DrawContext* dc = fCurrentPage->drawContext();
        if (dc && dc->modifiesTarget()) {
            fCurrentPage->ensurePlotted();
            recorder->priv().recordAtlasProviderUploads(dc);
            dc->flush(recorder);
            sk_sp<Task> drawTask = dc->snapDrawTask();
            if (drawTask) {
                recorder->priv().add(std::move(drawTask));
            }
        }
    }
    fCurrentDepth = DrawOrder::kClearDepth;
}

void ScratchDeviceAtlas::reset() {
    fCurrentPage.reset();
    fCurrentDepth = DrawOrder::kClearDepth;
}

TextureProxy* ScratchDeviceAtlas::target() const {
    return fCurrentPage ? fCurrentPage->target() : nullptr;
}

DrawContext* ScratchDeviceAtlas::drawContext() const {
    return fCurrentPage ? fCurrentPage->drawContext() : nullptr;
}

bool ScratchDeviceAtlas::hasPendingWork() const {
    return fCurrentPage && fCurrentPage->drawContext() &&
           fCurrentPage->drawContext()->modifiesTarget();
}

} // namespace skgpu::graphite
