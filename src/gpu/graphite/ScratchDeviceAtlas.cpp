/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ScratchDeviceAtlas.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/BoundsManager.h"
#include "src/gpu/graphite/task/Task.h"

#include <algorithm>

namespace skgpu::graphite {

namespace {

static constexpr int kDefaultAtlasDim = 2048;
static constexpr int kGridCellSize = 16;
static constexpr int kMaxBruteForceN = 64;
static constexpr int kMaxGridSize = 32;

} // anonymous namespace

ScratchDeviceAtlas::ScratchDeviceAtlas(Recorder* recorder)
        : fRecorder(recorder)
        , fSurfaceProps()
        , fDimensions(SkISize::Make(0, 0))
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
    int maxDim = fDimensions.width() > 0
                         ? fDimensions.width()
                         : std::min(kDefaultAtlasDim, (int)fRecorder->priv().caps()->maxTextureSize());
    return size.width() + 2 * kSlotPadding <= maxDim &&
           size.height() + 2 * kSlotPadding <= maxDim;
}

bool ScratchDeviceAtlas::initAtlas(const SkColorInfo& colorInfo, const SkSurfaceProps& props) {
    if (fAtlasDC) {
        return true;
    }
    if (!fRecorder) {
        return false;
    }

    fColorInfo = colorInfo;
    fSurfaceProps = props;

    const Caps* caps = fRecorder->priv().caps();
    int maxDim = std::min(kDefaultAtlasDim, (int)caps->maxTextureSize());
    fDimensions = SkISize::Make(maxDim, maxDim);

    auto textureInfo = caps->getDefaultSampledTextureInfo(fColorInfo.colorType(),
                                                          Mipmapped::kNo,
                                                          fRecorder->priv().isProtected(),
                                                          Renderable::kYes);
    fAtlasProxy = TextureProxy::Make(caps,
                                     fRecorder->priv().resourceProvider(),
                                     fDimensions,
                                     textureInfo,
                                     skgpu::Budgeted::kYes,
                                     "ScratchDeviceAtlas");
    if (!fAtlasProxy) {
        return false;
    }

    fAtlasDC = DrawContext::Make(caps,
                                 fAtlasProxy,
                                 fDimensions,
                                 fColorInfo,
                                 fSurfaceProps,
                                 /*allowUnpremul=*/false);
    if (!fAtlasDC) {
        fAtlasProxy.reset();
        return false;
    }

    fAtlasDC->clear(SkColors::kTransparent);

    fRectanizer = std::make_unique<skgpu::RectanizerSkyline>(
            fDimensions.width(), fDimensions.height());
    fColorDepthBoundsManager = std::make_unique<HybridBoundsManager>(
            fDimensions, kGridCellSize, kMaxBruteForceN, kMaxGridSize);
    fDisjointStencilSet = std::make_unique<Device::IntersectionTreeSet>();
    fCurrentDepth = DrawOrder::kClearDepth;

    return true;
}

sk_sp<Device> ScratchDeviceAtlas::createDevice(const SkImageInfo& info,
                                               const SkSurfaceProps& props) {
    if (!this->isEligible(info)) {
        return nullptr;
    }

    if (fAtlasDC && fColorInfo != info.colorInfo()) {
        fRecorder->priv().flushScratchDeviceAtlases();
    }

    if (!this->initAtlas(info.colorInfo(), props)) {
        return nullptr;
    }

    SkIPoint16 loc;
    if (!fRectanizer->addPaddedRect(info.width(), info.height(), kSlotPadding, &loc)) {
        // Atlas is full. Flush all atlases in dependency order
        // (fChildAtlas -> fFilterAtlas[0] -> fFilterAtlas[1] -> ...) and start a new page.
        fRecorder->priv().flushScratchDeviceAtlases();
        if (!this->initAtlas(info.colorInfo(), props) ||
            !fRectanizer->addPaddedRect(info.width(), info.height(), kSlotPadding, &loc)) {
            return nullptr;
        }
    }

    SkIPoint atlasOffset = SkIPoint::Make(loc.fX, loc.fY);
    return Device::MakeAtlased(fRecorder,
                               fAtlasDC,
                               atlasOffset,
                               info.dimensions(),
                               info.colorInfo(),
                               props,
                               this);
}

void ScratchDeviceAtlas::flush(Recorder* recorder) {
    if (fAtlasDC && fAtlasDC->modifiesTarget()) {
        recorder->priv().recordAtlasProviderUploads(fAtlasDC.get());
        fAtlasDC->flush(recorder);
        sk_sp<Task> drawTask = fAtlasDC->snapDrawTask();
        if (drawTask) {
            recorder->priv().add(std::move(drawTask));
        }
    }
    if (fColorDepthBoundsManager) {
        fColorDepthBoundsManager->reset();
    }
    if (fDisjointStencilSet) {
        fDisjointStencilSet->reset();
    }
    fCurrentDepth = DrawOrder::kClearDepth;
}

void ScratchDeviceAtlas::reset() {
    fAtlasDC.reset();
    fAtlasProxy.reset();
    fRectanizer.reset();
    fColorDepthBoundsManager.reset();
    fDisjointStencilSet.reset();
    fCurrentDepth = DrawOrder::kClearDepth;
    fDimensions = SkISize::Make(0, 0);
}

TextureProxy* ScratchDeviceAtlas::target() const {
    return fAtlasProxy.get();
}

bool ScratchDeviceAtlas::hasPendingWork() const {
    return fAtlasDC && fAtlasDC->modifiesTarget();
}

} // namespace skgpu::graphite
