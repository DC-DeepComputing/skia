/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/ClipAtlasManager.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSize.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkLog.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ProxyCache.h"
#include "src/gpu/graphite/RasterPathUtils.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/Shape.h"

namespace skgpu::graphite {

static constexpr int kClipAtlasWidth = 2048;
static constexpr int kClipAtlasHeight = 2048;

ClipAtlasManager::ClipAtlasManager(Recorder* recorder)
        : fRecorder(recorder)
        , fPathKeyAtlasMgr(kClipAtlasWidth, kClipAtlasHeight,
                           /*plotWidth=*/kClipAtlasWidth/2, /*plotHeight=*/kClipAtlasHeight/2,
                           DrawAtlas::UseStorageTextures::kNo,
                           "PathKeyClipAtlas", recorder->priv().caps())
        // Examining the results from the top 20 or so webpages, the SaveRecord keyed clips
        // tend to be considerably smaller and rarer, so we use a smaller atlas here.
        , fSaveRecordKeyAtlasMgr(kClipAtlasWidth/2, kClipAtlasHeight/2,
                                 /*plotWidth=*/kClipAtlasWidth/2, /*plotHeight=*/kClipAtlasHeight/2,
                                 DrawAtlas::UseStorageTextures::kNo,
                                 "SaveRecordKeyClipAtlas", recorder->priv().caps()) {}

namespace {
// Needed to ensure that we have surrounding context, e.g. for inverse clips this would be solid.
constexpr int kEntryPadding = 1;

// If the first element to draw is an intersect, we clear to 0 and will draw it directly with
// coverage 1 (subsequent intersect elements will be inverse-filled and draw 0 outside).
// If the first element to draw is a difference, we clear to 1, and in all cases we draw the
// difference element directly with coverage 0.
SkAlpha initial_alpha_for_elements(const ClipStack::ElementList& elements) {
    SkASSERT(!elements.empty());
    return elements[0]->fOp == SkClipOp::kIntersect ? 0x00 : 0xFF;
}

void render_elements(RasterMaskHelper* helper, const ClipStack::ElementList& elements) {
    SkASSERT(!elements.empty());
    bool isFirst = true;
    for (const auto& ePtr : elements) {
        const auto& e = *ePtr;
        uint8_t alpha;
        bool invert;
        if (e.fOp == SkClipOp::kIntersect) {
            // Intersect modifies pixels outside of its geometry. If this is the first element,
            // we can draw directly with coverage 1 since we cleared to 0. Otherwise we draw the
            // inverse-filled shape with 0 coverage to erase everything outside the element.
            if (isFirst) {
                alpha = 0xFF;
                invert = false;
            } else {
                alpha = 0x00;
                invert = true;
            }
        } else {
            // For difference ops, can always just subtract the shape directly by drawing 0 coverage
            SkASSERT(e.fOp == SkClipOp::kDifference);
            alpha = 0x00;
            invert = false;
        }

        // Draw the shape; based on how we've initialized the buffer and chosen alpha+invert,
        // every element is drawn with the kReplace_Op
        if (invert != e.fShape.inverted()) {
            Shape inverted(e.fShape);
            inverted.setInverted(invert);
            helper->drawClip(inverted, e.fLocalToDevice, alpha);
        } else {
            helper->drawClip(e.fShape, e.fLocalToDevice, alpha);
        }
        isFirst = false;
    }
}

} // anonymous namespace

ClipAtlasManager::Result
ClipAtlasManager::findOrCreateEntry(uint32_t stackRecordID,
                                    const ClipStack::ElementList* elementList,
                                    SkIRect maskDeviceBounds) {
    // For the ClipAtlas cache, we don't include the bounds in the key
    skgpu::UniqueKey maskKey;
    bool usesPathKey;
    // The keyBounds are the maskDeviceBounds relative to the full transformed mask. We use this
    // to ensure we capture the situation where the maskDeviceBounds are equal in two cases but
    // actually enclose different regions of the full mask due to a difference in integer
    // translation (which is not captured in the key) in the element transforms.
    SkIRect keyBounds;
    maskKey = GenerateClipMaskKey(stackRecordID, elementList, maskDeviceBounds,
                                  /*includeBounds=*/false, &keyBounds, &usesPathKey);

    sk_sp<TextureProxy> proxy;
    SkIPoint position;

    if (usesPathKey) {
        std::tie(proxy, position) = fPathKeyAtlasMgr.findOrCreateEntry(fRecorder,
                                                                       maskKey,
                                                                       elementList,
                                                                       maskDeviceBounds,
                                                                       keyBounds);
    } else {
        std::tie(proxy, position) = fSaveRecordKeyAtlasMgr.findOrCreateEntry(fRecorder,
                                                                             maskKey,
                                                                             elementList,
                                                                             maskDeviceBounds,
                                                                             keyBounds);
    }
    if (proxy) {
        return {std::move(proxy), position};
    }

    // We need to include the bounds in the key when using the ProxyCache
    maskKey = GenerateClipMaskKey(stackRecordID, elementList, maskDeviceBounds,
                                  /*includeBounds=*/true, &keyBounds, &usesPathKey);

    const struct ClipDrawContext {
        const ClipStack::ElementList* fElementList;
        SkIRect fMaskDeviceBounds;
    } context = {elementList, maskDeviceBounds};
    proxy = fRecorder->priv().proxyCache()->findOrCreateCachedProxy(
            fRecorder, maskKey, &context,
            [](const void* ctx) {
                const ClipDrawContext* cdc = static_cast<const ClipDrawContext*>(ctx);
                auto translate =
                        -cdc->fMaskDeviceBounds.topLeft() + SkIVector{kEntryPadding, kEntryPadding};
                auto [bm, helper] =
                        RasterMaskHelper::Allocate(cdc->fMaskDeviceBounds.size(),
                                                   translate,
                                                   kEntryPadding,
                                                   initial_alpha_for_elements(*cdc->fElementList));

                render_elements(&helper, *cdc->fElementList);
                bm.setImmutable();
                return bm;
            });
    position = {kEntryPadding, kEntryPadding};

    return {proxy, position};
}

bool ClipAtlasManager::recordUploads(DrawContext* dc) {
    return fPathKeyAtlasMgr.recordUploads(dc, fRecorder) ||
           fSaveRecordKeyAtlasMgr.recordUploads(dc, fRecorder);
}

void ClipAtlasManager::compact() {
    fPathKeyAtlasMgr.compact(fRecorder);
    fSaveRecordKeyAtlasMgr.compact(fRecorder);
}

void ClipAtlasManager::freeGpuResources() {
    fPathKeyAtlasMgr.freeGpuResources(fRecorder);
    fSaveRecordKeyAtlasMgr.freeGpuResources(fRecorder);
}

void ClipAtlasManager::evictAtlases() {
    fPathKeyAtlasMgr.evictAll();
    fSaveRecordKeyAtlasMgr.evictAll();
}

//////////////////////////////////////////////////////////////////////////////////////////////

ClipAtlasManager::DrawAtlasMgr::DrawAtlasMgr(size_t width, size_t height,
                                             size_t plotWidth, size_t plotHeight,
                                             DrawAtlas::UseStorageTextures useStorageTextures,
                                             std::string_view label, const Caps* caps) {
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

ClipAtlasManager::Result ClipAtlasManager::DrawAtlasMgr::findOrCreateEntry(
        Recorder* recorder,
        const UniqueKey& maskKey,
        const ClipStack::ElementList* elementList,
        SkIRect maskDeviceBounds,
        SkIRect keyBounds) {
    MaskHashEntry* entry = fMaskCache.find(maskKey);
    while (entry) {
        // If this entry contains the subset of the clip we require, use it
        SkIRect maskRelativeBounds = entry->fMaskRelativeBounds;
        SkASSERT(maskDeviceBounds.isEmpty() == keyBounds.isEmpty());
        if (maskRelativeBounds.contains(keyBounds)) {
            // Adjust the atlas position to account for the bounds we require.
            SkIVector offset = keyBounds.topLeft() - maskRelativeBounds.topLeft();
            auto atlasPos = SkIPoint{entry->fAtlasPos.fX, entry->fAtlasPos.fY} + offset;
            fDrawAtlas->setLastUseToken(entry->fRecord,
                                        recorder->priv().tokenTracker()->nextFlushToken());
            auto proxy = fDrawAtlas->getProxy(entry->fPageIdx);
            return {proxy, atlasPos};
        }
        entry = entry->fNext;
    }

    auto result = this->addToAtlas(recorder, elementList, maskDeviceBounds);
    if (!result.has_value()) {
        return {};
    }

    const auto& [record, pageIdx, pos] = *result;

    // Look up again (in case this entry got purged during addToAtlas())
    MaskHashEntry* entryList = fMaskCache.find(maskKey);

    SkIPoint16 pos16{SkToS16(pos.fX), SkToS16(pos.fY)};
    // Add locator and bounds to MaskCache.
    MaskHashEntry e{nullptr, record, keyBounds, pos16, SkToU8(pageIdx)};
    if (entryList) {
        // Add new list entry to the end. This will sort them from smallest bounds to largest,
        // so that when we search above we'll pick the one with the smallest bounds that contains
        // the clip.
        MaskHashEntry* currEntry = entryList;
        while (currEntry->fNext) {
            currEntry = currEntry->fNext;
        }
        SkASSERT(currEntry);
        SkASSERT(currEntry->fNext == nullptr); // Should be at the end
        currEntry->fNext = new MaskHashEntry{e};
    } else {
        fMaskCache.set(maskKey, e);
    }

    fAtlasRecordMap.set(record, maskKey);

    return {fDrawAtlas->getProxy(pageIdx), pos};
}

std::optional<DrawAtlas::AddResult>
ClipAtlasManager::DrawAtlasMgr::addToAtlas(Recorder* recorder,
                                           const ClipStack::ElementList* elementsForMask,
                                           SkIRect maskDeviceBounds) {
    // Render mask.
    SkISize maskSize = maskDeviceBounds.size();
    if (maskSize.isEmpty()) {
        return {};
    }
    // Expand to include padding as well (so we clear correctly for inverse clip)
    maskSize.fWidth += 2*kEntryPadding;
    maskSize.fHeight += 2*kEntryPadding;

    // Request space in DrawAtlas, including padding
    DrawAtlas::AddResult addResult;
    DrawAtlas::ErrorCode ec = fDrawAtlas->addRect(recorder, maskSize, &addResult);
    if (ec != DrawAtlas::ErrorCode::kSucceeded) {
        return {};
    }
    const auto& record = addResult.fRecord;

    // Rasterize path to the record's pixmap with an inset.
    std::optional<SkColor> clearColor;
    if (SkAlpha alpha =  initial_alpha_for_elements(*elementsForMask); alpha != 0) {
        clearColor = alpha << 24;
    }
    auto pixmap = fDrawAtlas->entryPixmap(record, 0, clearColor);
    SkASSERT(pixmap);

    auto translate = -maskDeviceBounds.topLeft() + SkIVector{kEntryPadding, kEntryPadding};
    RasterMaskHelper helper(*pixmap, translate);
    render_elements(&helper, *elementsForMask);

    addResult.fPosition += SkIVector{kEntryPadding, kEntryPadding};

    fDrawAtlas->setLastUseToken(record, recorder->priv().tokenTracker()->nextFlushToken());

    return addResult;
}

bool ClipAtlasManager::DrawAtlasMgr::recordUploads(DrawContext* dc, Recorder* recorder) {
    return (fDrawAtlas && !fDrawAtlas->recordUploads(dc, recorder));
}

void ClipAtlasManager::DrawAtlasMgr::evict(const DrawAtlas::RecordVisitor& visitor) {
    visitor.visitRecords([&](DrawAtlas::Record record) {
        auto* maskKey = fAtlasRecordMap.find(record);
        SkASSERT(maskKey);
        auto* curr = fMaskCache.find(*maskKey);
        SkASSERT(curr);
        MaskHashEntry* prev = nullptr;
        while (true) {
            if (curr->fRecord == record) {
                if (prev) {
                    prev->fNext = curr->fNext;
                    delete curr;
                } else if (auto* next = curr->fNext) {
                    // curr is not on the heap, it is stored in fMaskCache.
                    // Let it take over next's role and then delete next.
                    *curr = *next;
                    delete next;
                } else {
                    // The list in fMaskCache would now be empty, so remove it.
                    fMaskCache.remove(*maskKey);
                }
                break;
            }
            prev = curr;
            curr = curr->fNext;
            if (!curr) {
                SKIA_LOG_W("Could not find AtlasRecord in mask cache");
                break;
            }
        }
        fAtlasRecordMap.remove(record);
    });
}

void ClipAtlasManager::DrawAtlasMgr::evictAll() {
    fDrawAtlas->evictAllPlots();
    SkASSERT(fMaskCache.empty());
    SkASSERT(fAtlasRecordMap.empty());
}

void ClipAtlasManager::DrawAtlasMgr::compact(Recorder* recorder) {
    auto tokenTracker = recorder->priv().tokenTracker();
    if (fDrawAtlas) {
        fDrawAtlas->compact(tokenTracker->nextFlushToken());
    }
}

void ClipAtlasManager::DrawAtlasMgr::freeGpuResources(Recorder* recorder) {
    auto tokenTracker = recorder->priv().tokenTracker();
    if (fDrawAtlas) {
        fDrawAtlas->freeGpuResources(tokenTracker->nextFlushToken());
    }
}

}  // namespace skgpu::graphite
