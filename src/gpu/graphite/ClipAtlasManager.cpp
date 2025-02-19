/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ClipAtlasManager.h"

#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkFixed.h"
#include "src/base/SkFloatBits.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/RasterPathUtils.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

ClipAtlasManager::ClipAtlasManager(Recorder* recorder) : fRecorder(recorder) {
    static constexpr SkColorType kColorType = kAlpha_8_SkColorType;
    static constexpr int kWidth = 4096;
    static constexpr int kHeight = 4096;

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
// TODO: is this necessary for clips?
constexpr int kEntryPadding = 1;
}  // namespace

const TextureProxy* ClipAtlasManager::findOrCreateEntry(uint32_t stackRecordID,
                                                        const ClipStack::ElementList* elementList,
                                                        SkIRect iBounds,
                                                        SkIPoint* outPos) {
    skgpu::UniqueKey maskKey = GenerateClipMaskKey(stackRecordID, elementList);

    SkDebugf("%p Looking for key %d and bounds [%d %d %d %d]\n",
             this, maskKey.hash(),
             iBounds.fLeft, iBounds.fTop, iBounds.fRight, iBounds.fBottom);

    MaskHashEntry* entryList = fMaskCache.find(maskKey);
    if (entryList) {
        SkDebugf("%p Found hashtable entry\n", this);
        MaskHashEntry* entry = entryList;
        while (entry) {
            if (entry->fBounds.contains(iBounds)) {
                const SkIRect& bounds = entry->fBounds;
                SkDebugf("%p Found containing bounds: [%d %d %d %d]\n", this,
                         bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);
                SkIPoint topLeft = entry->fLocator.topLeft();
                // We need to adjust the returned outPos to reflect the subset we're using
                SkIPoint subsetRelativePos = iBounds.topLeft() - entry->fBounds.topLeft();
                *outPos = SkIPoint::Make(topLeft.x() + kEntryPadding + subsetRelativePos.x(),
                                         topLeft.y() + kEntryPadding + subsetRelativePos.y());
                fDrawAtlas->setLastUseToken(entry->fLocator,
                                            fRecorder->priv().tokenTracker()->nextFlushToken());
                return fDrawAtlas->getProxies()[entry->fLocator.pageIndex()].get();
            }
            entry = entry->fNext;
        }
    }

    AtlasLocator locator;
    const TextureProxy* proxy = this->addToAtlas(elementList, iBounds, outPos, &locator);
    if (!proxy) {
        return nullptr;
    }

    uint32_t index = fDrawAtlas->getListIndex(locator.plotLocator());
    // Add locator and bounds to MaskCache.
    if (entryList) {
        SkDebugf("%p Adding new hashlist entry: key %d plot %d bounds [%d %d %d %d]\n", this,
                 maskKey.hash(), index, iBounds.fLeft, iBounds.fTop, iBounds.fRight, iBounds.fBottom);
        MaskHashEntry* newEntry = new MaskHashEntry{iBounds, locator, nullptr};
        newEntry->fNext = entryList->fNext;
        entryList->fNext = newEntry;
    } else {
        SkDebugf("%p Adding new hashtable entry: key %d plot %d bounds [%d %d %d %d]\n", this,
                 maskKey.hash(), index, iBounds.fLeft, iBounds.fTop, iBounds.fRight, iBounds.fBottom);
        MaskHashEntry newEntry{iBounds, locator, nullptr};
        MaskHashEntry* pEntry = fMaskCache.set(maskKey, newEntry);
        // Validate that we actually added an entry
        if (!pEntry) {
            SkDebugf("%p **** ClipAtlasERROR ****\n", this);
            SkDebugf("%p Didn't actually add an entry!\n", this);
        } else {
            if (pEntry->fBounds != iBounds ||
                pEntry->fLocator.plotLocator() != locator.plotLocator()) {
                SkDebugf("%p **** ClipAtlasERROR ****\n", this);
                const SkIRect& entryBounds = pEntry->fBounds;
                index = fDrawAtlas->getListIndex(pEntry->fLocator.plotLocator());
                SkDebugf("%p Entry contents are wrong! plot %d bounds [%d %d %d %d]\n", this,
                         index, entryBounds.fLeft, entryBounds.fTop,
                         entryBounds.fRight, entryBounds.fBottom);
            }
        }
    }

    // Add key to Plot's MaskKeyList.
    SkDebugf("%p Adding new plotlist entry: key %d plot %d bounds [%d %d %d %d]\n", this,
             maskKey.hash(), index, iBounds.fLeft, iBounds.fTop, iBounds.fRight, iBounds.fBottom);
    MaskKeyEntry* keyEntry = new MaskKeyEntry{maskKey, iBounds};
    fKeyLists[index].addToTail(keyEntry);

    return proxy;
}

// Copied and modified from Ganesh ClipStack
void draw_to_sw_mask(RasterMaskHelper* helper,
                     const ClipStack::Element& e,
                     bool clearMask,
                     const SkIRect& resultBounds) {
    // If the first element to draw is an intersect, we clear to 0 and will draw it directly with
    // coverage 1 (subsequent intersect elements will be inverse-filled and draw 0 outside).
    // If the first element to draw is a difference, we clear to 1, and in all cases we draw the
    // difference element directly with coverage 0.
    if (clearMask) {
        helper->clear(e.fOp == SkClipOp::kIntersect ? 0x00 : 0xFF, resultBounds);
    }

    uint8_t alpha;
    bool invert;
    if (e.fOp == SkClipOp::kIntersect) {
        // Intersect modifies pixels outside of its geometry. If this isn't the first op, we
        // draw the inverse-filled shape with 0 coverage to erase everything outside the element
        // But if we are the first element, we can draw directly with coverage 1 since we
        // cleared to 0.
        if (clearMask) {
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
        helper->drawClip(inverted, e.fLocalToDevice, alpha, resultBounds);
    } else {
        helper->drawClip(e.fShape, e.fLocalToDevice, alpha, resultBounds);
    }
}

const TextureProxy* ClipAtlasManager::addToAtlas(const ClipStack::ElementList* elementsForMask,
                                                 SkIRect iBounds,
                                                 SkIPoint* outPos,
                                                 AtlasLocator* locator) {
    // Render mask.
    SkISize maskSize = iBounds.size();
    if (maskSize.isEmpty()) {
        return nullptr;
    }

    // Bounds relative to the AtlasLocator
    // Expanded to include padding as well (so we clear correctly for inverse clip)
    SkIRect iShapeBounds = SkIRect::MakeXYWH(0, 0,
                                             maskSize.width() + 2*kEntryPadding,
                                             maskSize.height() + 2*kEntryPadding);

    // Request space in DrawAtlas, including padding
    DrawAtlas::ErrorCode errorCode = fDrawAtlas->addRect(fRecorder,
                                                         iShapeBounds.width(),
                                                         iShapeBounds.height(),
                                                         locator);
    if (errorCode != DrawAtlas::ErrorCode::kSucceeded) {
        return nullptr;
    }
    SkIPoint topLeft = locator->topLeft();
    *outPos = SkIPoint::Make(topLeft.x() + kEntryPadding, topLeft.y() + kEntryPadding);

    // Rasterize path to backing pixmap.
    // This pixmap will be the size of the Plot that contains the given rect, not the entire atlas,
    // and hence the position we render at will be relative to that Plot.
    // The value of outPos is relative to the entire texture, to be used for texture coords.
    SkAutoPixmapStorage dst;
    SkIPoint renderPos = fDrawAtlas->prepForRender(*locator, &dst);

    // The shape bounds are expanded by kEntryPadding so we need to take that into account here.
    SkIVector transformedMaskOffset = {iBounds.left() - kEntryPadding,
                                       iBounds.top() - kEntryPadding};
    RasterMaskHelper helper(&dst);
    if (!helper.init(fDrawAtlas->plotSize(), transformedMaskOffset)) {
        return nullptr;
    }

    // Offset bounds to plot location for draw
    iShapeBounds.offset(renderPos.x(), renderPos.y());

    SkASSERT(!elementsForMask->empty());
    for (int i = 0; i < elementsForMask->size(); ++i) {
        draw_to_sw_mask(&helper, *(*elementsForMask)[i], i == 0, iShapeBounds);
    }

    fDrawAtlas->setLastUseToken(*locator,
                                fRecorder->priv().tokenTracker()->nextFlushToken());

    return fDrawAtlas->getProxies()[locator->pageIndex()].get();
}

bool ClipAtlasManager::recordUploads(DrawContext* dc) {
    return (fDrawAtlas && !fDrawAtlas->recordUploads(dc, fRecorder));
}

void ClipAtlasManager::evict(PlotLocator plotLocator) {
    SkDebugf("%p Evicting...\n", this);
    // Remove all entries for this Plot from the MaskCache
    uint32_t index = fDrawAtlas->getListIndex(plotLocator);
    MaskKeyList::Iter iter;
    iter.init(fKeyLists[index], MaskKeyList::Iter::kHead_IterStart);
    MaskKeyEntry* currKeyEntry;
    while ((currKeyEntry = iter.get())) {
        iter.next();
        const SkIRect& bounds = currKeyEntry->fBounds;
        SkDebugf("%p Preparing to remove plotlist entry: key %d plot %d bounds [%d %d %d %d]\n", this,
                 currKeyEntry->fKey.hash(), index, bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);
        MaskHashEntry* currHashEntry = fMaskCache.find(currKeyEntry->fKey);
        MaskHashEntry* prevHashEntry = nullptr;
        if (!currHashEntry) {
            SkDebugf("%p ****** ClipAtlasERROR ******\n", this);
            SkDebugf("%p Found no hash entry!\n", this);
        }
        while (currHashEntry) {
            const SkIRect& hashBounds = currHashEntry->fBounds;
            uint32_t hashIndex = fDrawAtlas->getListIndex(currHashEntry->fLocator.plotLocator());
            SkDebugf("%p Checking against hashlist entry: plot %d bounds [%d %d %d %d]\n", this,
                     hashIndex, hashBounds.fLeft, hashBounds.fTop, hashBounds.fRight, hashBounds.fBottom);

            if (currHashEntry->fBounds == currKeyEntry->fBounds) {
                SkDebugf("%p Found match\n", this);
                // Validate that this entry is for this plot
                uint32_t entryIndex = fDrawAtlas->getListIndex(currHashEntry->fLocator.plotLocator());
                if (entryIndex != index) {
                    SkDebugf("%p ****** ClipAtlasERROR ******\n", this);
                    SkDebugf("%p Hash entry has wrong index! plot %d\n", this, entryIndex);
                }
                if (prevHashEntry) {
                    prevHashEntry->fNext = currHashEntry->fNext;
                    delete currHashEntry;
                } else if (currHashEntry->fNext) {
                    MaskHashEntry* next = currHashEntry->fNext;
                    currHashEntry->fBounds = next->fBounds;
                    currHashEntry->fLocator = next->fLocator;
                    currHashEntry->fNext = next->fNext;
                    delete next;
                } else {
                    SkDebugf("%p Validate removal\n", this);
                    // Validate that this hash entry is in no other list
                    for (int i = 0; i < fKeyLists.size(); ++i) {
                        if ((uint32_t)i != index) {
                            MaskKeyList::Iter validateIter;
                            validateIter.init(fKeyLists[i], MaskKeyList::Iter::kHead_IterStart);
                            MaskKeyEntry* currValidateEntry;
                            while ((currValidateEntry = validateIter.get())) {
                                validateIter.next();
                                if (currValidateEntry->fKey == currKeyEntry->fKey) {
                                    SkDebugf("%p ****** ClipAtlasERROR ******\n", this);
                                    SkDebugf("%p Extra plotlist entry: plot %d\n", this, i);
                                }
//                                SkASSERT_RELEASE(currValidateEntry->fKey != currKeyEntry->fKey);
                            }
                        }
                    }
                    SkDebugf("%p Removing last entry...\n", this);
                    // Remove
                    fMaskCache.remove(currKeyEntry->fKey);
                    SkDebugf("%p ... done.\n", this);
                }
                break;
            }
            prevHashEntry = currHashEntry;
            currHashEntry = currHashEntry->fNext;
        }
        SkDebugf("%p Removing plotlist entry: key %d plot %d bounds [%d %d %d %d]\n", this,
                 currKeyEntry->fKey.hash(), index, bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);
        fKeyLists[index].remove(currKeyEntry);
        delete currKeyEntry;
    }
}

void ClipAtlasManager::evictAtlases() {
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
