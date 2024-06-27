/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/DrawPass.h"

#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkAlign.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/CopyTask.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/PipelineDataCache.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/geom/BoundsManager.h"

#include "src/base/SkMathPriv.h"
#include "src/base/SkTBlockList.h"

#include <algorithm>
#include <unordered_map>

using namespace skia_private;

namespace skgpu::graphite {

namespace {

// Helper to manage packed fields within a uint64_t
template <uint64_t Bits, uint64_t Offset>
struct Bitfield {
    static constexpr uint64_t kMask = ((uint64_t) 1 << Bits) - 1;
    static constexpr uint64_t kOffset = Offset;
    static constexpr uint64_t kBits = Bits;

    static uint32_t get(uint64_t v) { return static_cast<uint32_t>((v >> kOffset) & kMask); }
    static uint64_t set(uint32_t v) { return (v & kMask) << kOffset; }
};

// This class maps objects to a dense index which can then be used to look them up later
template <typename T, typename V = T, typename C = V>
class DenseBiMap {
public:
    using Index = uint32_t;

    // See note below in GeometryUniformField. This value can be round-tripped within the SortKey
    // packing for all fields but will not be produced when recording actual draw data.
    static constexpr Index kInvalidIndex{1 << SkNextLog2_portable(DrawList::kMaxRenderSteps)};

    bool empty() const { return fIndexToData.empty(); }
    size_t size() const { return fIndexToData.size(); }

    Index insert(const T& data) {
        Index* index = fDataToIndex.find(data);
        if (!index) {
            SkASSERT(SkToU32(fIndexToData.size()) < kInvalidIndex - 1);
            index = fDataToIndex.set(data, (Index) fIndexToData.size());
            fIndexToData.push_back(C{data});
        }
        return *index;
    }

    const V& lookup(Index index) {
        SkASSERT(index < kInvalidIndex);
        return fIndexToData[index];
    }

    SkSpan<V> data() { return {fIndexToData.data(), fIndexToData.size()}; }

    TArray<V>&& detach() { return std::move(fIndexToData); }

private:
    THashMap<T, Index> fDataToIndex;
    TArray<V> fIndexToData;
};


struct Binding {
    const UniformDataBlock* fPaintUniforms;
    const UniformDataBlock* fRenderStepUniforms;
    const TextureDataBlock* fPaintTextures;
    const TextureDataBlock* fRenderStepTextures;
};
using BindingCache = DenseBiMap<Binding>;
using GraphicsPipelineCache = DenseBiMap<GraphicsPipelineDesc>;

enum BindingChange : int {
    kNone               = 0b000,
    kPaintUniforms      = 0b001,
    kRenderStepUniforms = 0b010,
    kTextures           = 0b100,
};
SK_MAKE_BITMASK_OPS(BindingChange)

class UniformBindingTracker {
public:
    UniformBindingTracker(bool useStorageBuffers) : fUseStorageBuffers(useStorageBuffers) {}

    const BindUniformBufferInfo& bindingInfo() const { return fLastBindingInfo; }
    int ssboIndex() const { return fLastSSBOIndex; }

    bool setCurrentBinding(DrawBufferManager* bufferMgr, const UniformDataBlock* data) {
        BindBufferInfo bufferInfo = this->getGPUData(bufferMgr, data);
        if (fUseStorageBuffers) {
            if (fLastBindingInfo) {
                // The values can be indexed from the from the original binding, so calculate that
                // index and save for later inclusion in the vertex data, but do not mark a binding.
                SkASSERT(bufferInfo.fBuffer == fLastBindingInfo.fBuffer);
                int byteOffset = bufferInfo.fOffset - fLastBindingInfo.fOffset;
                SkASSERT(byteOffset % data->requiredAlignment() == 0);
                fLastSSBOIndex = byteOffset / data->requiredAlignment();
                return false;
            } else {
                // This is the start of a new SSBO array so there is a binding change
                fLastBindingInfo = bufferInfo;
                fLastSSBOIndex = 0;
                return true;
            }
        } else if (bufferInfo != fLastBindingInfo) {
            fLastBindingInfo = bufferInfo;
            return true;
        }
    }

    void bind(DrawPassCommands::List* commandList, UniformSlot slot) {
        SkASSERT(!fDeferredBindingSize);
        if (fUseStorageBuffers) {
            fDeferredBindingSize = commandList->bindDeferredUniformBuffer(fLastBindingInfo, slot);
        } else {
            commandList->bindUniformBuffer({{fLastBindingInfo.fBuffer,
                                             fLastBindingInfo.fOffset},
                                            fLastBindingSize},
                                           slot);
        }
    }

private:
    using UniformBindingCache = THashMap<const UniformDataBlock*, BindBufferInfo>;

    const BindBufferInfo& getGPUData(DrawBufferManager* bufferMgr, const UniformDataBlock* cpuData) {
        BindBufferInfo* binding = fCache.find(cpuData);
        if (binding) {
            return *binding; // The data has already been written to the GPU so reuse that binding
        }

        // Must write new data to the GPU
        UniformWriter writer;
        BindBufferInfo bufferInfo;
        size_t udbSize = data->size();
        if (fUseStorageBuffers) {
            // For storage buffers, we bind one byte array for all data until we've hit the max
            // binding size. At the point we reset and start a new array. The SSBO is heterogenous
            // so we align each data block to its required alignment and calculate its "index" based
            // on the stride that the pipeline thinks the data type is. Data blocks for different
            // pipelines will not be accessed and this allows us to skip clustering data by pipeline
            std::tie(writer, bufferInfo) = bufferMgr->getSsboWriter(udbSize,
                                                                    cpuData->requiredAlignment());

            // FIXME: we need to check this *before* we request the writer because the first
            // instance is going to need to be at a max alignment for binding.
            // FIXME: but we are also interleaving paint and renderstep uniforms here into the
            // same buffer. would it be better to let the tracker checkout entire buffers from
            // the buffer manager and manage its own sub-allocations?
            if (bufferInfo.fBuffer != fLastBindingInfo.fBuffer ||
                bufferInfo.fOffset - fLastBindingInfo.fOffset > caps->maxBindingSize()) {
                // Must reset tracking since we hit the end of a buffer or the binding size allowed
                fLastBindingInfo = {}; // signals reset to setCurrentBinding()
                fCache.reset();
                if (fDeferredBindingSize) {
                    *fDeferredBindingSize = fLastBindingSize;
                }
                fLastBindingSize = udbSize;
            } else {
                // Appending to the existing storage buffer binding
                fLastBindingSize = udbSize + bufferInfo.fOffset - fLastBindingInfo.fOffset;
            }
        } else {
            // For UBOs we write each data block on its own, automatically aligned to the uniform
            // binding alignment by the DrawBufferManager. There is no need to worry about max
            // binding size either since we are only binding data for one set of data.
            SkASSERT(data->requiredAlignment() <= bufferMgr->uniformAlignment());
            std::tie(writer, bufferInfo) = bufferMgr->getUniformWriter(udbSize);
            fLastBindingSize = udbSize;
        }

        writer.write(data->data(), udbSize);
        return *fCache.set(data, bufferInfo);
    }

    UniformBindingCache fCache;

    BindBufferInfo fLastBindingInfo = {};
    int fLastSSBOIndex = {};
    uint32_t fLastBindingSize = 0;

    uint32_t* fDeferredBindingSize = nullptr;

    const bool fUseStorageBuffers;
};

class BindingTracker {
public:
    BindingTracker(bool useStorageBuffers)
            : fPaintUniformBinding(useStorageBuffers)
            , fRenderStepUniformBinding(useStorageBuffers) {}

    BindingCache::Index track(const UniformDataBlock* paintUniforms,
                              const UniformDataBlock* renderStepUniforms,
                              const TextureDataBlock* paintTextures,
                              const TextureDataBlock* renderStepTextures) {
        if (!paintUniforms && !renderStepUniforms && !paintTextures && !renderStepUniforms) {
            return BindingCache::kInvalidIndex;
        }

        return fBindingCache.insert(Binding{paintUniforms, renderStepUniforms,
                                            paintTextures, renderStepTextures});
    }

    bool setCurrentBinding(DrawBufferManager* bufferMgr, BindingCache::Index index) {
        SkASSERT(fBindingCache == BindingChange::kNone);
        if (index >= BindingCache::kInvalidIndex || fLastBinding == index) {
            return false;
        }

        const Binding& binding = fBindingCache.lookup(index);
        if (binding.fPaintTextures != fLastPaintTextures ||
            binding.fRenderStepTextures != fLastRenderStepTextures) {
            // Both texture sets are bound together, so a change in either requires re-binding
            fBindingChange |= BindingChange::kTextures;
            fLastPaintTextures = binding.fPaintTextures;
            fLastRenderStepTextures = binding.fRenderStepTextures;
        }

        if (fPaintUniformBinding.setCurrentBinding(bufferMgr, binding.fPaintUniforms)) {
            fBindingChange |= BindingChange::kPaintUniforms;
        }
        if (fRenderStepUniformBinding.setCurrentBinding(bufferMgr, binding.fRenderStepUniforms)) {
            fBindingChange |= BindingChange::kRenderStepUniforms;
        }

        fLastBinding = index;
        return fBindingChange != BindingChange::kNone;
    }

    void bind(DrawPassCommands::List* commandList) {
        SkASSERT(fLastBinding < BindingCache::kInvalidIndex);
        SkASSERT(fBindingChange != BindingChange::kNone);

        if (fBindingChange & BindingChange::kPaintUniforms) {
            fPaintUniformBinding.bind(commandList, UniformSlot::kPaint);
        }
        if (fBindingChange & BindingChange::kRenderStepUniforms) {
            fRenderStepUniformBinding.bind(commandList, UniformSlot::kRenderStep);
        }

        if (fBindingChange & BindingChange::kTextures) {
            // The two texture sets are merged into a single binding
            int numTextures =
                    (fLastPaintTextures      ? fLastPaintTextures->numTextures()      : 0) +
                    (fLastRenderStepTextures ? fLastRenderStepTextures->numTextures() : 0);
            auto [texIndices, samplerIndices] =
                    commandList->bindDeferredTexturesAndSamplers(numTextures);

            if (fLastPaintTextures) {
                for (int i = 0; i < fLastPaintTextures->numTextures(); ++i) {
                    auto [tex, sampler] = fLastPaintTextures->texture(i);
                    *texIndices++       = fProxyCache.insert(tex.get());
                    *samplerIndices++   = fSamplerCache.insert(sampler);
                }
            }
            if (fLastRenderStepTextures) {
                for (int i = 0; i < fLastRenderStepTextures->numTextures(); ++i) {
                    auto [tex, sampler] = fLastRenderStepTextures->texture(i);
                    *texIndices++       = fProxyCache.insert(tex.get());
                    *samplerIndices++   = fSamplerCache.insert(sampler);
                }
            }
        }

        fBindingChange = BindingChange::kNone;
    }

    TArray<sk_sp<TextureProxy>>&& detachTextures() { return fProxyCache.detach(); }
    TArray<SamplerDesc>&& detachSamplers() { return fSamplerCache.detach(); }

    skvx::ushort2 ssboIndices() const {
        return skvx::ushort2(SkTo<uint16_t>(fRenderStepUniformBinding.ssboIndex()),
                             SkTo<uint16_t>(fPaintUniformBinding.ssboIndex()));
    }

private:
    struct ProxyRef {
        const TextureProxy* fProxy;
        operator sk_sp<TextureProxy>() const { return sk_ref_sp(fProxy); }
    };
    using TextureProxyCache = DenseBiMap<const TextureProxy*, sk_sp<TextureProxy>, ProxyRef>;
    using SamplerDescCache  = DenseBiMap<SamplerDesc>;

    // Collected CPU data mapped to index for state sorting
    BindingCache fBindingCache;

    // Collected GPU data
    TextureProxyCache fProxyCache;
    SamplerDescCache fSamplerCache;

    UniformBindingTracker fPaintUniformBinding;
    UniformBindingTracker fRenderStepUniformBinding;

    // State tracking between draws and calls to setCurrentBinding() and bind()
    BindingCache::Index fLastBinding = BindingCache::kInvalidIndex;
    SkEnumBitMask<BindingChange> fBindingChange = BindingChange::kNone;

    const TextureDataBlock* fLastPaintTextures = nullptr;
    const TextureDataBlock* fLastRenderStepTextures = nullptr;
};

} // namespace

///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Each Draw in a DrawList might be processed by multiple RenderSteps (determined by the Draw's
 * Renderer), which can be sorted independently. Each (step, draw) pair produces its own SortKey.
 *
 * The goal of sorting draws for the DrawPass is to minimize pipeline transitions and dynamic binds
 * within a pipeline, while still respecting the overall painter's order. This decreases the number
 * of low-level draw commands in a command buffer and increases the size of those, allowing the GPU
 * to operate more efficiently and have fewer bubbles within its own instruction stream.
 *
 * The Draw's CompresssedPaintersOrder and DisjointStencilINdex represent the most significant bits
 * of the key, and are shared by all SortKeys produced by the same draw. Next, the pipeline
 * description is encoded in two steps:
 *  1. The index of the RenderStep packed in the high bits to ensure each step for a draw is
 *     ordered correctly.
 *  2. An index into a cache of pipeline descriptions is used to encode the identity of the
 *     pipeline (SortKeys that differ in the bits from #1 necessarily would have different
 *     descriptions, but then the specific ordering of the RenderSteps isn't enforced).
 * Last, the SortKey encodes an index into the set of uniform bindings accumulated for a DrawPass.
 * This allows the SortKey to cluster draw steps that have both a compatible pipeline and do not
 * require rebinding uniform data or other state (e.g. scissor). Since the uniform data index and
 * the pipeline description index are packed into indices and not actual pointers, a given SortKey
 * is only valid for the a specific DrawList->DrawPass conversion.
 */
class DrawPass::SortKey {
public:
    SortKey(const DrawList::Draw* draw,
            int renderStep,
            GraphicsPipelineCache::Index pipelineIndex,
            BindingCache::Index bindingIndex)
        : fPipelineKey(ColorDepthOrderField::set(draw->fDrawParams.order().paintOrder().bits()) |
                       StencilIndexField::set(draw->fDrawParams.order().stencilIndex().bits())  |
                       RenderStepField::set(static_cast<uint32_t>(renderStep))                  |
                       PipelineField::set(pipelineIndex)                                        |
                       BindingField::set(bindingIndex))
        , fDraw(draw) {
        SkASSERT(pipelineIndex < GraphicsPipelineCache::kInvalidIndex);
        SkASSERT(renderStep <= draw->fRenderer->numRenderSteps());
    }

    bool operator<(const SortKey& k) const { return fKey < k.fKey; }

    const DrawList::Draw& draw() const { return *fDraw; }

    const RenderStep& renderStep() const {
        return fDraw->fRenderer->step(RenderStepField::get(fKey));
    }

    GraphicsPipelineCache::Index pipelineIndex() const { return PipelineField::get(fKey); }
    BindingCache::Index bindingIndex() const { return BindingField::get(fKey); }

private:
    // Fields are ordered from most-significant to least when sorting by 64-bit value.
    // NOTE: We don't use C++ bit fields because field ordering is implementation defined and we
    // need to sort consistently.
    using ColorDepthOrderField = Bitfield<16, 48>; // sizeof(CompressedPaintersOrder)
    using StencilIndexField    = Bitfield<16, 32>; // sizeof(DisjointStencilIndex)
    using RenderStepField      = Bitfield<2,  30>; // bits >= log2(Renderer::kMaxRenderSteps)
    using PipelineField        = Bitfield<15, 15>; // bits >= log2(DrawList::kMaxRenderSteps)
    using BindingField         = Bitfield<15, 0 >; // bits >= log2(DrawList::kMaxRenderSteps)+null
    uint64_t fKey;

    // Backpointer to the draw that produced the sort key
    const DrawList::Draw* fDraw;

    static_assert(ColorDepthOrderField::kBits >= sizeof(CompressedPaintersOrder));
    static_assert(StencilIndexField::kBits    >= sizeof(DisjointStencilIndex));
    static_assert(RenderStepField::kBits      >= SkNextLog2_portable(Renderer::kMaxRenderSteps));
    static_assert(PipelineField::kBits        >= SkNextLog2_portable(DrawList::kMaxRenderSteps));
    static_assert(BindingField::kBits         >= 1+SkNextLog2_portable(DrawList::kMaxRenderSteps));
};

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<TextureProxy> add_copy_target_task(Recorder* recorder,
                                         sk_sp<TextureProxy> target,
                                         const SkImageInfo& targetInfo,
                                         const SkIPoint& targetOffset) {
    SkASSERT(recorder->priv().caps()->isTexturable(target->textureInfo()));
    SkIRect dstSrcRect = SkIRect::MakePtSize(targetOffset, targetInfo.dimensions());
    sk_sp<TextureProxy> copy = TextureProxy::Make(recorder->priv().caps(),
                                                  targetInfo.dimensions(),
                                                  target->textureInfo(),
                                                  skgpu::Budgeted::kYes);
    if (!copy) {
        SKGPU_LOG_W("Failed to create destination copy texture for dst read.");
        return nullptr;
    }

    sk_sp<CopyTextureToTextureTask> copyTask = CopyTextureToTextureTask::Make(
            std::move(target), dstSrcRect, copy, /*dstPoint=*/{0, 0});
    if (!copyTask) {
        SKGPU_LOG_W("Failed to create destination copy task for dst read.");
        return nullptr;
    }

    recorder->priv().add(std::move(copyTask));
    return copy;
}

DrawPass::DrawPass(sk_sp<TextureProxy> target,
                   std::pair<LoadOp, StoreOp> ops,
                   std::array<float, 4> clearColor)
        : fTarget(std::move(target))
        , fBounds(SkIRect::MakeEmpty())
        , fOps(ops)
        , fClearColor(clearColor) {}

DrawPass::~DrawPass() = default;

std::unique_ptr<DrawPass> DrawPass::Make(Recorder* recorder,
                                         std::unique_ptr<DrawList> draws,
                                         sk_sp<TextureProxy> target,
                                         const SkImageInfo& targetInfo,
                                         std::pair<LoadOp, StoreOp> ops,
                                         std::array<float, 4> clearColor) {
    // NOTE: This assert is here to ensure SortKey is as tightly packed as possible. Any change to
    // its size should be done with care and good reason. The performance of sorting the keys is
    // heavily tied to the total size.
    //
    // At 24 bytes (current), sorting is about 30% slower than if SortKey could be packed into just
    // 16 bytes. There are several ways this could be done if necessary:
    //  - Restricting the max draw count to 16k (14-bits) and only using a single index to refer to
    //    the uniform data => 8 bytes of key, 8 bytes of pointer.
    //  - Restrict the max draw count to 32k (15-bits), use a single uniform index, and steal the
    //    4 low bits from the Draw* pointer since it's 16 byte aligned.
    //  - Compact the Draw* to an index into the original collection, although that has extra
    //    indirection and does not work as well with SkTBlockList.
    // In pseudo tests, manipulating the pointer or having to mask out indices was about 15% slower
    // than an 8 byte key and unmodified pointer.
    static_assert(sizeof(DrawPass::SortKey) ==
                  SkAlignTo(16 + sizeof(void*), alignof(DrawPass::SortKey)));

    TRACE_EVENT1("skia.gpu", TRACE_FUNC, "draw count", draws->fDraws.count());

    // The DrawList is converted directly into the DrawPass' data structures, but once the DrawPass
    // is returned from Make(), it is considered immutable.
    std::unique_ptr<DrawPass> drawPass(new DrawPass(target, ops, clearColor));

    Rect passBounds = Rect::InfiniteInverted();

    // We don't expect the uniforms from the renderSteps to reappear multiple times across a
    // recorder's lifetime so we only de-dupe them w/in a given DrawPass.
    UniformDataCache geometryUniformDataCache;
    TextureDataCache* textureDataCache = recorder->priv().textureDataCache();
    DrawBufferManager* bufferMgr = recorder->priv().drawBufferManager();

    GraphicsPipelineCache pipelineCache;

    // Geometry uniforms are currently always UBO-backed.
    const bool useStorageBuffers = recorder->priv().caps()->storageBufferPreferred();
    const ResourceBindingRequirements& bindingReqs =
            recorder->priv().caps()->resourceBindingRequirements();

    BindingTracker bindingTracker{useStorageBuffers};

    ShaderCodeDictionary* dict = recorder->priv().shaderCodeDictionary();
    PaintParamsKeyBuilder builder(dict);

    PipelineDataGatherer gatherer(useStorageBuffers ? bindingReqs.fStorageBufferLayout
                                                    : bindingReqs.fUniformBufferLayout);

    // Copy of destination, if needed.
    sk_sp<TextureProxy> dst;
    SkIPoint dstOffset;
    if (!draws->dstCopyBounds().isEmptyNegativeOrNaN()) {
        TRACE_EVENT_INSTANT0("skia.gpu", "DrawPass requires dst copy", TRACE_EVENT_SCOPE_THREAD);

        SkIRect dstCopyPixelBounds = draws->dstCopyBounds().makeRoundOut().asSkIRect();
        dstOffset = dstCopyPixelBounds.topLeft();
        dst = add_copy_target_task(
                recorder, target, targetInfo.makeDimensions(dstCopyPixelBounds.size()), dstOffset);
        if (!dst) {
            SKGPU_LOG_W("Failed to copy destination for reading. Dropping draw pass!");
            return nullptr;
        }
    }

    std::vector<SortKey> keys;
    keys.reserve(draws->renderStepCount());

    for (const DrawList::Draw& draw : draws->fDraws.items()) {
        // If we have two different descriptors, such that the uniforms from the PaintParams can be
        // bound independently of those used by the rest of the RenderStep, then we can upload now
        // and remember the location for re-use on any RenderStep that does shading.
        UniquePaintParamsID shaderID;
        const UniformDataBlock* shadingUniforms = nullptr;
        const TextureDataBlock* paintTextures = nullptr;
        if (draw.fPaintParams.has_value()) {
            sk_sp<TextureProxy> curDst =
                    draw.fPaintParams->dstReadRequirement() == DstReadRequirement::kTextureCopy
                            ? dst
                            : nullptr;
            std::tie(shaderID, shadingUniforms, paintTextures) =
                    ExtractPaintData(recorder,
                                     &gatherer,
                                     &builder,
                                     draw.fDrawParams.transform(),
                                     draw.fPaintParams.value(),
                                     curDst,
                                     dstOffset,
                                     targetInfo.colorInfo());
        } // else depth-only

        for (int stepIndex = 0; stepIndex < draw.fRenderer->numRenderSteps(); ++stepIndex) {
            const RenderStep* const step = draw.fRenderer->steps()[stepIndex];
            const bool performsShading = draw.fPaintParams.has_value() && step->performsShading();

            GraphicsPipelineCache::Index pipelineIndex = pipelineCache.insert(
                    {step, performsShading ? shaderID : UniquePaintParamsID::InvalidID()});
            auto [stepUniforms, stepTextures] = ExtractRenderStepData(&geometryUniformDataCache,
                                                                      textureDataCache,
                                                                      &gatherer,
                                                                      step,
                                                                      draw.fDrawParams);

            BindingCache::Index bindingIndex = bindingTracker.track(
                    performsShading ? shadingUniforms : nullptr,
                    geometryUniforms,
                    performsShading ? paintTextures : nullptr,
                    stepTextures);

            keys.push_back({&draw, stepIndex, pipelineIndex, bindingIndex});
        }

        passBounds.join(draw.fDrawParams.clip().drawBounds());
        drawPass->fDepthStencilFlags |= draw.fRenderer->depthStencilFlags();
        drawPass->fRequiresMSAA |= draw.fRenderer->requiresMSAA();
    }

    // TODO: Explore sorting algorithms; in all likelihood this will be mostly sorted already, so
    // algorithms that approach O(n) in that condition may be favorable. Alternatively, could
    // explore radix sort that is always O(n). Brief testing suggested std::sort was faster than
    // std::stable_sort and SkTQSort on my [ml]'s Windows desktop. Also worth considering in-place
    // vs. algorithms that require an extra O(n) storage.
    // TODO: It's not strictly necessary, but would a stable sort be useful or just end up hiding
    // bugs in the DrawOrder determination code?
    std::sort(keys.begin(), keys.end());

    // Used to record vertex/instance data, buffer binds, and draw calls
    DrawWriter drawWriter(&drawPass->fCommandList, bufferMgr);
    GraphicsPipelineCache::Index lastPipeline = GraphicsPipelineCache::kInvalidIndex;
    SkIRect lastScissor = SkIRect::MakeSize(targetInfo.dimensions());

    SkASSERT(drawPass->fTarget->isFullyLazy() ||
             SkIRect::MakeSize(drawPass->fTarget->dimensions()).contains(lastScissor));
    drawPass->fCommandList.setScissor(lastScissor);

    for (const SortKey& key : keys) {
        const DrawList::Draw& draw = key.draw();
        const RenderStep& renderStep = key.renderStep();

        const bool pipelineChange = key.pipelineIndex() != lastPipeline;
        const bool bindingChange  =
                bindingTracker.setCurrentBindings(bufferMgr, key.bindingIndex());
        const SkIRect* newScissor = draw.fDrawParams.clip().scissor() != lastScissor ?
                &draw.fDrawParams.clip().scissor() : nullptr;

        // Update DrawWriter *before* we actually change any state so that accumulated draws from
        // the previous state use the proper state.
        if (pipelineChange) {
            drawWriter.newPipelineState(renderStep.primitiveType(),
                                        renderStep.vertexStride(),
                                        renderStep.instanceStride());
        } else if (bindingChange || SkToBool(newScissor)) {
            drawWriter.newDynamicState();
        }

        // Make state changes before accumulating new draw data
        if (pipelineChange) {
            drawPass->fCommandList.bindGraphicsPipeline(key.pipelineIndex());
            lastPipeline = key.pipelineIndex();
        }
        if (bindingChange) {
            bindingTracker.bind(&drawPass->fCommandList);
        }
        if (newScissor) {
            drawPass->fCommandList.setScissor(*newScissor);
            lastScissor = *newScissor;
        }

        renderStep.writeVertices(&drawWriter, draw.fDrawParams, bindingTracker.ssboIndices());
    }
    // Finish recording draw calls for any collected data at the end of the loop
    drawWriter.flush();

    drawPass->fBounds = passBounds.roundOut().asSkIRect();

    drawPass->fPipelineDescs   = pipelineCache.detach();
    drawPass->fSamplerDescs    = textureBindingTracker.detachSamplers();
    drawPass->fSampledTextures = textureBindingTracker.detachTextures();

    TRACE_COUNTER1("skia.gpu", "# pipelines", drawPass->fPipelineDescs.size());
    TRACE_COUNTER1("skia.gpu", "# textures", drawPass->fSampledTextures.size());
    TRACE_COUNTER1("skia.gpu", "# commands", drawPass->fCommandList.count());

    return drawPass;
}

bool DrawPass::prepareResources(ResourceProvider* resourceProvider,
                                const RuntimeEffectDictionary* runtimeDict,
                                const RenderPassDesc& renderPassDesc) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    fFullPipelines.reserve(fFullPipelines.size() + fPipelineDescs.size());
    for (const GraphicsPipelineDesc& pipelineDesc : fPipelineDescs) {
        auto pipeline = resourceProvider->findOrCreateGraphicsPipeline(runtimeDict,
                                                                       pipelineDesc,
                                                                       renderPassDesc);
        if (!pipeline) {
            SKGPU_LOG_W("Failed to create GraphicsPipeline for draw in RenderPass. Dropping pass!");
            return false;
        }
        fFullPipelines.push_back(std::move(pipeline));
    }
    // The DrawPass may be long lived on a Recording and we no longer need the GraphicPipelineDescs
    // once we've created pipelines, so we drop the storage for them here.
    fPipelineDescs.clear();

    for (int i = 0; i < fSampledTextures.size(); ++i) {
        // TODO: We need to remove this check once we are creating valid SkImages from things like
        // snapshot, save layers, etc. Right now we only support SkImages directly made for graphite
        // and all others have a TextureProxy with an invalid TextureInfo.
        if (!fSampledTextures[i]->textureInfo().isValid()) {
            SKGPU_LOG_W("Failed to validate sampled texture. Will not create renderpass!");
            return false;
        }
        if (!TextureProxy::InstantiateIfNotLazy(resourceProvider, fSampledTextures[i].get())) {
            SKGPU_LOG_W("Failed to instantiate sampled texture. Will not create renderpass!");
            return false;
        }
    }

    fSamplers.reserve(fSamplers.size() + fSamplerDescs.size());
    for (int i = 0; i < fSamplerDescs.size(); ++i) {
        sk_sp<Sampler> sampler = resourceProvider->findOrCreateCompatibleSampler(
                fSamplerDescs[i].samplingOptions(),
                fSamplerDescs[i].tileModeX(),
                fSamplerDescs[i].tileModeY());
        if (!sampler) {
            SKGPU_LOG_W("Failed to create sampler. Will not create renderpass!");
            return false;
        }
        fSamplers.push_back(std::move(sampler));
    }
    // The DrawPass may be long lived on a Recording and we no longer need the SamplerDescs
    // once we've created Samplers, so we drop the storage for them here.
    fSamplerDescs.clear();

    return true;
}

void DrawPass::addResourceRefs(CommandBuffer* commandBuffer) const {
    for (int i = 0; i < fFullPipelines.size(); ++i) {
        commandBuffer->trackResource(fFullPipelines[i]);
    }
    for (int i = 0; i < fSampledTextures.size(); ++i) {
        commandBuffer->trackCommandBufferResource(fSampledTextures[i]->refTexture());
    }
    for (int i = 0; i < fSamplers.size(); ++i) {
        commandBuffer->trackResource(fSamplers[i]);
    }
}

const Texture* DrawPass::getTexture(size_t index) const {
    SkASSERT(index < SkToSizeT(fSampledTextures.size()));
    SkASSERT(fSampledTextures[index]);
    SkASSERT(fSampledTextures[index]->texture());
    return fSampledTextures[index]->texture();
}
const Sampler* DrawPass::getSampler(size_t index) const {
    SkASSERT(index < SkToSizeT(fSamplers.size()));
    SkASSERT(fSamplers[index]);
    return fSamplers[index].get();
}

} // namespace skgpu::graphite
