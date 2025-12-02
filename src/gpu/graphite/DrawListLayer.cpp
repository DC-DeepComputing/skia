/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/DrawListLayer.h"

#include "include/core/SkTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/geom/Geometry.h"

namespace skgpu::graphite {

namespace {

// Writes uniform data either to uniform buffers or to shared storage buffers, and tracks when
// bindings need to change between draws.
class UniformTracker {
public:
    UniformTracker(bool useStorageBuffers) : fUseStorageBuffers(useStorageBuffers) {}

    bool writeUniforms(UniformDataCache& uniformCache,
                       DrawBufferManager* bufferMgr,
                       UniformDataCache::Index index) {
        if (index >= UniformDataCache::kInvalidIndex) {
            return false;
        }

        if (index == fLastIndex) {
            return false;
        }
        fLastIndex = index;

        UniformDataCache::Entry& uniformData = uniformCache.lookup(index);
        const size_t uniformDataSize = uniformData.fCpuData.size();

        // Upload the uniform data if we haven't already.
        // Alternatively, re-upload the uniform data to avoid a rebind if we're using storage
        // buffers. This will result in more data uploaded, but the tradeoff seems worthwhile.
        if (!uniformData.fBufferBinding.fBuffer ||
            (fUseStorageBuffers && uniformData.fBufferBinding.fBuffer != fLastBinding.fBuffer)) {
            BufferWriter writer;
            std::tie(writer, uniformData.fBufferBinding) =
                    fCurrentBuffer.getMappedSubrange(1, uniformDataSize);
            if (!writer) {
                // Allocate a new buffer
                std::tie(writer, uniformData.fBufferBinding, fCurrentBuffer) =
                        fUseStorageBuffers ? bufferMgr->getMappedStorageBuffer(1, uniformDataSize)
                                           : bufferMgr->getMappedUniformBuffer(1, uniformDataSize);
                if (!writer) {
                    return {}; // Allocation failed so early out
                }
            }

            writer.write(uniformData.fCpuData.data(), uniformDataSize);

            if (fUseStorageBuffers) {
                // When using storage buffers, store the SSBO index in the binding's offset field
                // and always use the entire buffer's size in the size field.
                SkASSERT(uniformData.fBufferBinding.fOffset % uniformDataSize == 0);
                uniformData.fBufferBinding.fOffset /= uniformDataSize;
                uniformData.fBufferBinding.fSize = uniformData.fBufferBinding.fBuffer->size();
            } else {
                // Every new set of uniform data has to be bound, this ensures its aligned correctly
                fCurrentBuffer.resetForNewBinding();
            }
        }

        const bool needsRebind =
                uniformData.fBufferBinding.fBuffer != fLastBinding.fBuffer ||
                (!fUseStorageBuffers && uniformData.fBufferBinding.fOffset != fLastBinding.fOffset);

        fLastBinding = uniformData.fBufferBinding;

        return needsRebind;
    }

    void bindUniforms(UniformSlot slot, DrawPassCommands::List* commandList) {
        BindBufferInfo binding = fLastBinding;
        if (fUseStorageBuffers) {
            // Track the SSBO index in fLastBinding, but set offset = 0 in the actual used binding.
            binding.fOffset = 0;
        }
        commandList->bindUniformBuffer(binding, slot);
    }

    uint32_t ssboIndex() const {
        // The SSBO index for the last-bound storage buffer is stored in the binding's offset field.
        return fLastBinding.fOffset;
    }

private:
    // The GPU buffer data is being written into; for SSBOs, Graphite will only record a bind
    // command when this changes. Sub-allocations will be aligned such that they can be randomly
    // accessed even if the data is heterogenous. UBOs will always have to issue binding commands
    // when a draw needs to use a different set of uniform values.
    BufferSubAllocator fCurrentBuffer;

    // Internally track the last binding returned, so that we know whether new uploads or rebindings
    // are necessary. If we're using SSBOs, this is treated specially -- the fOffset field holds the
    // index in the storage buffer of the last-written uniforms, and the offsets used for actual
    // bindings are always zero.
    BindBufferInfo fLastBinding;

    // This keeps track of the last index used for writing uniforms from a provided uniform cache.
    // If a provided index matches the last index, the uniforms are assumed to already be written
    // and no additional uploading is performed. This assumes a UniformTracker will always be
    // provided with the same uniform cache.
    UniformDataCache::Index fLastIndex = UniformDataCache::kInvalidIndex;

    const bool fUseStorageBuffers;
};

// Tracks when to issue BindTexturesAndSamplers commands to a command list and converts
// TextureDataBlocks to that representation as needed.
class TextureTracker {
public:
    TextureTracker(TextureDataCache* textureCache)
            : fTextureCache(textureCache) {}

    bool setCurrentTextureBindings(TextureDataCache::Index bindingIndex) {
        if (bindingIndex < TextureDataCache::kInvalidIndex && fLastIndex != bindingIndex) {
            fLastIndex = bindingIndex;
            return true;
        }
        // No binding change
        return false;
    }

    void bindTextures(DrawPassCommands::List* commandList) {
        SkASSERT(fLastIndex < TextureDataCache::kInvalidIndex);
        TextureDataBlock binding = fTextureCache->lookup(fLastIndex);

        auto [textures, samplers] =
                commandList->bindDeferredTexturesAndSamplers(binding.numTextures());

        for (int i = 0; i < binding.numTextures(); ++i) {
            auto [t, s] = binding.texture(i);
            textures[i] = t.get();
            samplers[i] = s;
        }
    }

private:
    TextureDataCache::Index fLastIndex = TextureDataCache::kInvalidIndex;

    TextureDataCache* const fTextureCache;
};

} // anonymous namespace

void DrawListLayer::reset(LoadOp loadOp, SkColor4f color) {
    fLoadOp = loadOp;
    fClearColor = color.premul().array();

    fStorage.reset(); // Should clear everything?
    fLayers.reset();
    fTransforms.reset();

    fRenderStepCount = 0;
    fRequiresMSAA = false;
    fDepthStencilFlags = DepthStencilFlags::kNone;
    SkDEBUGCODE(fCoverageMaskShapeDrawCount = 0);

    fDstReadBounds = Rect::InfiniteInverted();
    fPassBounds = Rect::InfiniteInverted();

    fUniformDataCache.reset();
    fTextureDataCache.reset();
    fPipelineCache.reset();
}

const Transform& DrawListLayer::deduplicateTransform(const Transform& localToDevice) {
    // TODO: This is a pretty simple deduplication strategy and doesn't take advantage of the stack
    // knowledge that Device has.
    if (fTransforms.empty() || fTransforms.back() != localToDevice) {
        fTransforms.push_back(localToDevice);
    }
    return fTransforms.back();
}

void DrawListLayer::Layer::add(SkArenaAllocWithReset* alloc,
                               BindingList* match,
                               const LayerKey& key,
                               const RenderStep* step,
                               const DrawParams& params,
                               bool dependsOnDst) {
    RecordedDraw* draw = alloc->make<RecordedDraw>(step, params);
    if (match) {
        match->fDraws.addToTail(draw);
        match->fBounds.join(params.drawBounds());
    } else {
        BindingList* list = alloc->make<BindingList>();
        list->fKey = key;
        list->fBounds = params.drawBounds();
        list->fDraws.addToHead(draw);
        fBindings.addToTail(list);
    }
}

std::pair<BoundsTest, DrawListLayer::BindingList*>
DrawListLayer::Layer::test(const Rect& drawBounds, const LayerKey& key, bool disjointStencil) {

    if (!disjointStencil && fBindings.head() == fBindings.tail() && fBindings.head()->fKey == key) {
        return {BoundsTest::kCompatibleOverlap, fBindings.head()};
    }

    BindingList* foundMatch = nullptr;

    for (BindingList* list : fBindings) {
        if (list->fKey == key) {
            foundMatch = list;
            if (!disjointStencil) continue;
        }

        if (list->fBounds.intersects(drawBounds)) {
            for (const RecordedDraw* d : list->fDraws) {
                if (d->fDrawParams.drawBounds().intersects(drawBounds)) {
                    return {BoundsTest::kIncompatibleOverlap, foundMatch};
                }
            }
        }
    }

    return {foundMatch ? BoundsTest::kCompatibleOverlap : BoundsTest::kDisjoint, foundMatch};
}

// TODO test to see if worth
bool DrawListLayer::Layer::canExtractBindingList(const BindingList* candidate) {
    // We can extract 'candidate' if it is geometrically disjoint from all other binding lists
    // currently in this layer. If it overlaps with another list, we cannot change their Z-order
    // relative to each other, so we can't extract it.

    if (fBindings.head() == fBindings.tail()) {
        return true;
    }

    for (const BindingList* other : fBindings) {
        if (other == candidate) {
            continue;
        }
        // If the candidate overlaps with any "bystander" list, we are stuck.
        // We cannot extract it without breaking painter's order.
        if (other->fBounds.intersects(candidate->fBounds)) {
            return false;
        }
    }
    return true;
}

void DrawListLayer::recordStep(const RenderStep* step,
                               GraphicsPipelineCache::Index pipelineIndex,
                               UniformDataCache::Index uniformIndex,
                               TextureDataCache::Index textureIndex,
                               const DrawParams& drawParams,
                               SkEnumBitMask<DstUsage> dstUsage) {

    LayerKey key = {pipelineIndex, uniformIndex, textureIndex};

    SkTInternalLList<Layer>::Iter iter;
    Layer* current = iter.init(fLayers, SkTInternalLList<Layer>::Iter::kTail_IterStart);
    Layer* targetLayer = nullptr;
    BindingList* targetMatch = nullptr;

    bool dependsOnDst = (dstUsage & DstUsage::kDependsOnDst) != DstUsage::kNone;
    bool disjointStencil = (step->depthStencilFlags() & DepthStencilFlags::kStencil) !=
                           DepthStencilFlags::kNone;
    if (!dependsOnDst && !disjointStencil) {
        targetLayer = fLayers.head();
        current = nullptr;

        if (targetLayer) {
            auto result = targetLayer->test(drawParams.drawBounds(), key, disjointStencil);
            targetMatch = result.second;
        }
    }

    int limit = kMaxSearchLimit;
    BindingList* forwardMerge = nullptr;
    while (current && limit > 0) {
        auto [testResult, match] = current->test(drawParams.drawBounds(), key, disjointStencil);

        if (testResult == BoundsTest::kIncompatibleOverlap) {
            // If we need to read the dst, we cannot go earlier than this layer.
            if (dependsOnDst) {
                // Do we have a layer that can accept this draw or do we need to add a new one?
                if (current->fNext) {
                    // There is a layer above the conflict, place it there.
                    targetLayer = current->fNext;

                    auto result = targetLayer->test(drawParams.drawBounds(), key, disjointStencil);
                    targetMatch = result.second;
                    break;
                } else {
                    // We are at the end (Tail), so we need a new layer on top.

                    // FORWARD MERGE:
                    // Check if we can pull the conflicting layer's matching binding list
                    // *forward* into our new layer. This allows the new draw to batch
                    // with the old draws, despite the new layer creation.

                    // Old, might be better
                    // if (!disjointStencil && match) {
                    //     // We only move it if it isn't the only list in that layer
                    //     // (heuristically keeps layers stable).
                    //     if (current->fBindings.head() != match ||
                    //         current->fBindings.tail() != match) {
                    //         forwardMerge = match;
                    //     }
                    // }

                    if (match && !disjointStencil) {
                        if (current->canExtractBindingList(match)) {
                            forwardMerge = match;
                        }
                    }
                    break;
                }
            }
            // If !dependsOnDst, we just keep searching backwards.
        } else {
            targetLayer = current;
            targetMatch = match;

            if (testResult == BoundsTest::kCompatibleOverlap) {
                break;
            }
        }

        current = iter.prev();
        limit--;
    }

    if (!targetLayer) {
        targetLayer = fStorage.make<Layer>();
        fLayers.addToTail(targetLayer);

        if (forwardMerge) {
            SkASSERT(current);
            current->fBindings.remove(forwardMerge);
            targetLayer->fBindings.addToHead(forwardMerge);
            targetMatch = forwardMerge;
        }
    } else {
        SkASSERT(!forwardMerge);
    }

    targetLayer->add(&fStorage, targetMatch, key, step, drawParams, dependsOnDst);
}

void DrawListLayer::recordDraw(const Renderer* renderer,
                          const Transform& localToDevice,
                          const Geometry& geometry,
                          const Clip& clip,
                          DrawOrder ordering,
                          UniquePaintParamsID paintID,
                          SkEnumBitMask<DstUsage> dstUsage,
                          BarrierType barrierBeforeDraws,
                          PipelineDataGatherer* gatherer,
                          const StrokeStyle* stroke) {
    SkASSERT(localToDevice.valid());
    SkASSERT(!geometry.isEmpty() && !clip.drawBounds().isEmptyNegativeOrNaN());
    SkASSERT(!(renderer->depthStencilFlags() & DepthStencilFlags::kStencil) ||
             ordering.stencilIndex() != DrawOrder::kUnassigned);

    DrawParams drawParams(this->deduplicateTransform(localToDevice),
                          geometry, clip, ordering, stroke);

    fRenderStepCount += renderer->numRenderSteps();
    // Create a sort key for every render step in this draw
    for (int stepIndex = 0; stepIndex < renderer->numRenderSteps(); ++stepIndex) {
        const RenderStep* const step = renderer->steps()[stepIndex];
        gatherer->markOffsetAndAlign(step->performsShading(), step->uniformAlignment());

        GraphicsPipelineCache::Index pipelineIndex = fPipelineCache.insert(
                { step->renderStepID(), step->performsShading() ?
                                        paintID : UniquePaintParamsID::Invalid()});

        step->writeUniformsAndTextures(drawParams, gatherer);

        auto [combinedUniforms, combinedTextures] =
                gatherer->endCombinedData(step->performsShading());

        UniformDataCache::Index uniformIndex = combinedUniforms ?
                fUniformDataCache.insert(combinedUniforms) : UniformDataCache::kInvalidIndex;
        TextureDataCache::Index textureBindingIndex = combinedTextures ?
                fTextureDataCache.insert(combinedTextures) : TextureDataCache::kInvalidIndex;

        this->recordStep(step, pipelineIndex, uniformIndex, textureBindingIndex, drawParams,
                         dstUsage);
        gatherer->rewindForRenderStep();
    }

//     fPassBounds.join(clip.drawBounds());
//     fRequiresMSAA |= renderer->requiresMSAA();
//     fDepthStencilFlags |= renderer->depthStencilFlags();
//     if (dstUsage & DstUsage::kDstReadRequired) {
//         // For paints that read from the dst, update the bounds. It may later be determined that the
//         // DstReadStrategy does not require them, but they are inexpensive to track.
//         fDstReadBounds.join(clip.drawBounds());
//     }

// #if defined(SK_DEBUG)
//     if (geometry.isCoverageMaskShape()) {
//         fCoverageMaskShapeDrawCount++;
//     }
// #endif
}

} // namespace skgpu::graphite
