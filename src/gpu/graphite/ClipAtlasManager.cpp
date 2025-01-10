/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ClipAtlasManager.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

ClipAtlasManager::ClipAtlasManager(Recorder* recorder) : fRecorder(recorder) {
    static constexpr SkColorType kColorType = kAlpha_8_SkColorType;
    static constexpr int kWidth = 2048;
    static constexpr int kHeight = 2048;

    const Caps* caps = recorder->priv().caps();
    fDrawAtlas = DrawAtlas::Make(kColorType,
                                 SkColorTypeBytesPerPixel(kColorType),
                                 kWidth, kHeight,
                                 /*plotWidth=*/kWidth, /*plotHeight=*/kHeight,
                                 /*generationCounter=*/this,
                                 caps->allowMultipleAtlasTextures() ?
                                         DrawAtlas::AllowMultitexturing::kYes :
                                         DrawAtlas::AllowMultitexturing::kNo,
                                 DrawAtlas::UseStorageTextures::kNo,
                                 /*evictor=*/this,
                                 "ClipAtlas");
    SkASSERT(fDrawAtlas);
    fKeyLists.resize(fDrawAtlas->numPlots() * fDrawAtlas->maxPages());
    for (int i = 0; i < fKeyLists.size(); ++i) {
        fKeyLists[i].reset();
    }
}

namespace {
skgpu::UniqueKey generate_clip_mask_key(uint32_t stackRecordID,
                                        const ClipStack::ElementList* elementList) {
    // TODO
    return {};
}

// TODO: move to shared location?
uint32_t mask_key_list_index(const PlotLocator& locator, const DrawAtlas* drawAtlas) {
    return locator.pageIndex() * drawAtlas->numPlots() + locator.plotIndex();
}
}  // namespace

const TextureProxy* ClipAtlasManager::findOrCreateEntry(uint32_t stackRecordID,
                                                        const ClipStack::ElementList* elementList,
                                                        const Rect& bounds,
                                                        skvx::half2* outPos) {
    skgpu::UniqueKey maskKey = generate_clip_mask_key(stackRecordID, elementList);

    MaskHashArray* cachedArray = fMaskCache.find(maskKey);
    if (cachedArray) {
        for (int i = 0; i < cachedArray->size(); ++i) {
            MaskHashEntry& entry = (*cachedArray)[i];
            // We can reuse a clip mask if has the same key and our bounds is contained in it
            if (entry.fBounds.contains(bounds)) {
                SkIPoint topLeft = entry.fLocator.topLeft();
                // We need to adjust the returned outPos to reflect the subset we're using
                skvx::float2 subsetRelativePos = bounds.topLeft() - entry.fBounds.topLeft();
                *outPos = skvx::half2(topLeft.x() + subsetRelativePos.x(),
                                      topLeft.y() + subsetRelativePos.y());
                fDrawAtlas->setLastUseToken(entry.fLocator,
                                            fRecorder->priv().tokenTracker()->nextFlushToken());
                return fDrawAtlas->getProxies()[entry.fLocator.pageIndex()].get();
            }
        }
    }

    AtlasLocator locator;
    const TextureProxy* proxy = this->addToAtlas(elementList, bounds, outPos, &locator);
    if (!proxy) {
        return nullptr;
    }

    // Add locator and bounds to MaskCache.
    if (cachedArray) {
        cachedArray->push_back({bounds, locator});
    } else {
        MaskHashArray initialArray;
        initialArray.push_back({bounds, locator});
        fMaskCache.set(maskKey, initialArray);
    }
    // Add key to Plot's MaskKeyList.
    uint32_t index = mask_key_list_index(locator.plotLocator(), fDrawAtlas.get());
    MaskKeyEntry* keyEntry = new MaskKeyEntry();
    keyEntry->fKey = maskKey;
    keyEntry->fBounds = bounds;
    fKeyLists[index].addToTail(keyEntry);

    return proxy;
}

const TextureProxy* ClipAtlasManager::addToAtlas(const ClipStack::ElementList*,
                                                 const Rect& bounds,
                                                 skvx::half2* outPos,
                                                 AtlasLocator* locator) {
    return nullptr;
}

bool ClipAtlasManager::recordUploads(DrawContext* dc) {
    return (fDrawAtlas && !fDrawAtlas->recordUploads(dc, fRecorder));
}

void ClipAtlasManager::evict(PlotLocator plotLocator) {
    // Remove all entries for this Plot from the MaskCache
    uint32_t index = mask_key_list_index(plotLocator, fDrawAtlas.get());
    MaskKeyList::Iter iter;
    iter.init(fKeyLists[index], MaskKeyList::Iter::kHead_IterStart);
    MaskKeyEntry* currEntry;
    while ((currEntry = iter.get())) {
        iter.next();
        MaskHashArray* cachedArray = fMaskCache.find(currEntry->fKey);
        // Remove this entry from the hashed array
        for (int i = cachedArray->size()-1; i >=0 ; --i) {
            MaskHashEntry& entry = (*cachedArray)[i];
            if (entry.fBounds == currEntry->fBounds) {
                (*cachedArray).removeShuffle(i);
                break;
            }
        }
        // If we removed the last one, remove the hash entry
        if (cachedArray->size() == 0) {
            fMaskCache.remove(currEntry->fKey);
        }
        fKeyLists[index].remove(currEntry);
        delete currEntry;
    }
}

void ClipAtlasManager::evictAll() {
    fDrawAtlas->evictAllPlots();
    SkASSERT(fMaskCache.empty());
}

void ClipAtlasManager::compact(bool forceCompact) {
    auto tokenTracker = fRecorder->priv().tokenTracker();
    if (fDrawAtlas) {
        fDrawAtlas->compact(tokenTracker->nextFlushToken(), forceCompact);
    }
}

}  // namespace skgpu::graphite
