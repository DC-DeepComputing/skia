/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PathAtlas.h"

#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkLog.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RasterPathUtils.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/Transform.h"

namespace skgpu::graphite {
namespace {

constexpr int kMinAtlasTextureSize = 512;  // the smallest we want the PathAtlas textures to be
                                           // unless the device requires smaller

}  // namespace

PathAtlas::PathAtlas(Recorder* recorder, uint32_t requestedWidth, uint32_t requestedHeight)
        : fRecorder(recorder) {
    const Caps* caps = recorder->priv().caps();
    int maxTextureSize = std::max(caps->maxPathAtlasTextureSize(), kMinAtlasTextureSize);
    maxTextureSize = std::min(maxTextureSize, caps->maxTextureSize());

    fWidth = SkPrevPow2(std::min<uint32_t>(requestedWidth, maxTextureSize));
    fHeight = SkPrevPow2(std::min<uint32_t>(requestedHeight, maxTextureSize));
}

PathAtlas::~PathAtlas() = default;

PathAtlas::AddShapeResult PathAtlas::addShape(const Rect& transformedShapeBounds,
                                              const Shape& shape,
                                              const Transform& localToDevice,
                                              const SkStrokeRec& style) {
    // It is possible for the transformed shape bounds to be fully clipped out while the draw still
    // produces coverage due to an inverse fill. In this case, don't render any mask;
    // CoverageMaskShapeRenderStep will automatically handle the simple fill. We'll handle this
    // by adding an empty mask.
    // TODO: We could have addShape() handle this fully except we need a valid TextureProxy still.
    const bool emptyMask = transformedShapeBounds.isEmptyNegativeOrNaN();

    // Round out the shape bounds to preserve any fractional offset so that it is present in the
    // translation that we use when deriving the atlas-space transform later.
    Rect maskBounds = transformedShapeBounds.makeRoundOut();

    CoverageMaskShape::MaskInfo maskInfo;
    // This size does *not* include any padding that the atlas may place around the mask. This size
    // represents the area the shape can actually modify.
    maskInfo.fMaskSize = emptyMask ? skvx::half2(0) : skvx::cast<uint16_t>(maskBounds.size());
    // We use the origin of the clipped mask bounds relative to the full mask to distinguish
    // between clips of the same size.
    Rect shapeDevBounds = localToDevice.mapRect(shape.bounds());
    skvx::float2 clippedMaskOrigin = maskBounds.topLeft() - shapeDevBounds.topLeft();
    SkIVector transformedMaskOffset = SkIVector::Make(maskBounds.topLeft().x(),
                                                      maskBounds.topLeft().y());
    auto atlasResult = this->onAddShape(shape,
                                        localToDevice,
                                        style,
                                        skvx::cast<uint16_t>(clippedMaskOrigin),
                                        maskInfo.fMaskSize,
                                        transformedMaskOffset);
    if (!atlasResult.has_value()) {
        return {};
    }
    auto& [proxy, pos] = *atlasResult;
    maskInfo.fTextureOrigin = pos;

    return std::make_tuple(
            CoverageMaskShape(shape, std::move(proxy), localToDevice.inverse(), maskInfo),
            SkIPoint{(int)maskBounds.left(), (int)maskBounds.top()},
            fRecorder->priv().rendererProvider()->coverageMask());
}

/////////////////////////////////////////////////////////////////////////////////////////

PathAtlas::DrawAtlasMgr::DrawAtlasMgr(size_t width, size_t height,
                                      size_t plotWidth, size_t plotHeight,
                                      DrawAtlas::UseStorageTextures useStorageTextures,
                                      std::string_view label,
                                      const Caps* caps) {
    static constexpr auto kMaskFormat = MaskFormat::kA8;

    fDrawAtlas = DrawAtlas::Make(kMaskFormat,
                                 width, height,
                                 plotWidth, plotHeight,
                                 caps->allowMultipleAtlasTextures()
                                         ? DrawAtlas::AllowMultitexturing::kYes
                                         : DrawAtlas::AllowMultitexturing::kNo,
                                 useStorageTextures,
                                 /*evictor=*/this,
                                 label);
    SkASSERT(fDrawAtlas);
}

inline PathAtlas::AtlasResult::value_type PathAtlas::DrawAtlasMgr::makeAtlasResult(
        const AtlasEntry& entry) {
    SkIPoint topLeft{entry.fAtlasPosition.fX, entry.fAtlasPosition.fY};
    skvx::half2 pos(topLeft.fX + kEntryPadding, topLeft.fY + kEntryPadding);

    const auto& proxy = fDrawAtlas->getProxies()[entry.fPageIdx];

    return std::make_tuple(proxy, pos);
}

PathAtlas::AtlasResult PathAtlas::DrawAtlasMgr::findOrCreateEntry(Recorder* recorder,
                                                                  const Shape& shape,
                                                                  const Transform& localToDevice,
                                                                  const SkStrokeRec& strokeRec,
                                                                  skvx::half2 maskOrigin,
                                                                  skvx::half2 maskSize,
                                                                  SkIVector transformedMaskOffset) {
    // TODO: pull this out so we don't have to recalculate it for each atlas?
    skgpu::UniqueKey maskKey = GeneratePathMaskKey(shape, localToDevice, strokeRec,
                                                   maskOrigin, maskSize);
    AtlasEntry* cachedEntry = fShapeCache.find(maskKey);
    if (cachedEntry) {
        fDrawAtlas->setLastUseToken(cachedEntry->fRecord,
                                    recorder->priv().tokenTracker()->nextFlushToken());
        return makeAtlasResult(*cachedEntry);
    }

    auto entry = this->internalAddToAtlas(recorder,
                                          shape,
                                          localToDevice,
                                          strokeRec,
                                          maskSize,
                                          transformedMaskOffset);
    if (!entry.has_value()) {
        return {};
    }

    // Add entry to ShapeCache.
    fShapeCache.set(maskKey, *entry);
    // Add to reverse map so we can get look up in fShapeCache on eviction.
    fAtlasRecordMap.set(entry->fRecord, maskKey);

    return makeAtlasResult(*entry);
}

PathAtlas::AtlasResult PathAtlas::DrawAtlasMgr::addToAtlas(Recorder* recorder,
                                                           const Shape& shape,
                                                           const Transform& localToDevice,
                                                           const SkStrokeRec& strokeRec,
                                                           skvx::half2 maskSize,
                                                           SkIVector transformedMaskOffset) {
    auto record = this->internalAddToAtlas(recorder,
                                           shape,
                                           localToDevice,
                                           strokeRec,
                                           maskSize,
                                           transformedMaskOffset);
    if (!record.has_value()) {
        return {};
    }
    return makeAtlasResult(*record);
}

std::optional<PathAtlas::DrawAtlasMgr::AtlasEntry>
PathAtlas::DrawAtlasMgr::internalAddToAtlas(Recorder* recorder,
                                            const Shape& shape,
                                            const Transform& localToDevice,
                                            const SkStrokeRec& strokeRec,
                                            skvx::half2 maskSize,
                                            SkIVector transformedMaskOffset) {
    // Render mask.
    SkIRect iShapeBounds = SkIRect::MakeXYWH(0, 0, maskSize.x(), maskSize.y());
    // Outset to take padding into account
    SkIRect iAtlasBounds = iShapeBounds.makeOutset(kEntryPadding, kEntryPadding);

    DrawAtlas::AddResult addResult;
    if (fDrawAtlas->addRect(recorder, iAtlasBounds.size(), &addResult) !=
        DrawAtlas::ErrorCode::kSucceeded) {
        return {};
    }
    AtlasEntry entry{
            .fRecord        = addResult.fRecord,
            .fAtlasPosition = {SkToS16(addResult.fPosition.fX), SkToS16(addResult.fPosition.fY)},
            .fPageIdx       = SkToU8(addResult.fPageIdx)
    };

    // If the mask is empty, just return.
    // TODO: this may not be needed if we can handle clipped out bounds with inverse fills
    // another way. See PathAtlas::addShape().
    if (!all(maskSize)) {
        // Access the pixmap to ensure the entry is cleared.
        fDrawAtlas->entryPixmap(addResult.fRecord);
        fDrawAtlas->setLastUseToken(entry.fRecord,
                                    recorder->priv().tokenTracker()->nextFlushToken());
        return entry;
    }

    if (!this->onAddToAtlas(shape,
                            localToDevice,
                            strokeRec,
                            iShapeBounds,
                            transformedMaskOffset,
                            entry)) {
        return {};
    }

    fDrawAtlas->setLastUseToken(entry.fRecord, recorder->priv().tokenTracker()->nextFlushToken());
    return entry;
}

bool PathAtlas::DrawAtlasMgr::recordUploads(DrawContext* dc, Recorder* recorder) {
    return fDrawAtlas->recordUploads(dc, recorder);
}

void PathAtlas::DrawAtlasMgr::evict(const DrawAtlas::RecordVisitor& visitor) {
    // Remove all entries for this Plot from the ShapeCache
    visitor.visitRecords([&](DrawAtlas::Record record) {
        if (auto* key = fAtlasRecordMap.find(record)) {
            fShapeCache.remove(*key);
            fAtlasRecordMap.remove(record);
        } else {
            SKIA_LOG_W("Evicted entry not in cache!");
        }
    });
}

void PathAtlas::DrawAtlasMgr::evictAll() {
    fDrawAtlas->evictAllPlots();
    SkASSERT(fShapeCache.empty());
    SkASSERT(fAtlasRecordMap.empty());
}

void PathAtlas::DrawAtlasMgr::compact(Recorder* recorder) {
    fDrawAtlas->compact(recorder->priv().tokenTracker()->nextFlushToken());
}

void PathAtlas::DrawAtlasMgr::freeGpuResources(Recorder* recorder) {
    fDrawAtlas->freeGpuResources(recorder->priv().tokenTracker()->nextFlushToken());
}

}  // namespace skgpu::graphite
