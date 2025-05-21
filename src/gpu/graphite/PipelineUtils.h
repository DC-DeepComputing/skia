/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PipelineUtils_DEFINED
#define skgpu_graphite_PipelineUtils_DEFINED

#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTileMode.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkColorData.h"
#include "src/core/SkTHash.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

namespace skgpu::graphite {

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
            UniformWriter writer;
            std::tie(writer, uniformData.fBufferBinding) =
                    fUseStorageBuffers ? bufferMgr->getAlignedSsboWriter(1, uniformDataSize)
                                       : bufferMgr->getUniformWriter(1, uniformDataSize);

            // Early out if buffer mapping failed.
            if (!writer) {
                return {};
            }

            writer.write(uniformData.fCpuData.data(), uniformDataSize);

            if (fUseStorageBuffers) {
                // When using storage buffers, store the SSBO index in the binding's offset field
                // and always use the entire buffer's size in the size field.
                SkASSERT(uniformData.fBufferBinding.fOffset % uniformDataSize == 0);
                uniformData.fBufferBinding.fOffset /= uniformDataSize;
                uniformData.fBufferBinding.fSize = uniformData.fBufferBinding.fBuffer->size();
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

// NOTE: TextureBinding's use as a key type in DenseBiMap relies on the fact that the underlying
// data has been de-duplicated by a PipelineDataCache earlier, so that the bit identity of the data
// blocks (e.g. address+size) is equivalent to the content equality of the texture lists.

// Tracks the combination of textures from the paint and from the RenderStep to describe the full
// binding that needs to be in the command list.
struct TextureBinding {
    TextureDataBlock fPaintTextures;
    TextureDataBlock fStepTextures;

    bool operator==(const TextureBinding& other) const {
        return fPaintTextures == other.fPaintTextures &&
               fStepTextures == other.fStepTextures;
    }
    bool operator!=(const TextureBinding& other) const { return !(*this == other); }

    int numTextures() const {
        return (fPaintTextures ? fPaintTextures.numTextures() : 0) +
               (fStepTextures ? fStepTextures.numTextures() : 0);
    }
};

using TextureBindingCache = DenseBiMap<TextureBinding>;
using GraphicsPipelineCache = DenseBiMap<GraphicsPipelineDesc>;

// Automatically merges and manages texture bindings and uniform bindings sourced from either the
// paint or the RenderStep. Tracks the bound state based on last-provided unique index to write
// Bind commands to a CommandList when necessary.
class TextureBindingTracker {
public:
    TextureBindingCache::Index trackTextures(TextureDataBlock paintTextures,
                                             TextureDataBlock stepTextures) {
        if (!paintTextures && !stepTextures) {
            return TextureBindingCache::kInvalidIndex;
        }
        return fBindingCache.insert({paintTextures, stepTextures});
    }

    bool setCurrentTextureBindings(TextureBindingCache::Index bindingIndex) {
        if (bindingIndex < TextureBindingCache::kInvalidIndex && fLastIndex != bindingIndex) {
            fLastIndex = bindingIndex;
            return true;
        }
        // No binding change
        return false;
    }

    void bindTextures(DrawPassCommands::List* commandList) {
        SkASSERT(fLastIndex < TextureBindingCache::kInvalidIndex);
        const TextureBinding& binding = fBindingCache.lookup(fLastIndex);

        auto [texIndices, samplerIndices] =
                commandList->bindDeferredTexturesAndSamplers(binding.numTextures());

        if (binding.fPaintTextures) {
            for (int i = 0; i < binding.fPaintTextures.numTextures(); ++i) {
                auto [tex, sampler] = binding.fPaintTextures.texture(i);
                *texIndices++     = fProxyCache.insert(tex.get());
                *samplerIndices++ = fSamplerCache.insert(sampler);
            }
        }
        if (binding.fStepTextures) {
            for (int i = 0; i < binding.fStepTextures.numTextures(); ++i) {
                auto [tex, sampler] = binding.fStepTextures.texture(i);
                *texIndices++     = fProxyCache.insert(tex.get());
                *samplerIndices++ = fSamplerCache.insert(sampler);
            }
        }
    }

    skia_private::TArray<sk_sp<TextureProxy>>&& detachTextures() { return fProxyCache.detach(); }
    skia_private::TArray<SamplerDesc>&& detachSamplers() { return fSamplerCache.detach(); }

private:
    struct ProxyRef {
        const TextureProxy* fProxy;
        operator sk_sp<TextureProxy>() const { return sk_ref_sp(fProxy); }
    };
    using TextureProxyCache = DenseBiMap<const TextureProxy*, sk_sp<TextureProxy>, ProxyRef>;
    using SamplerDescCache = DenseBiMap<SamplerDesc>;

    TextureBindingCache fBindingCache;

    TextureProxyCache fProxyCache;
    SamplerDescCache fSamplerCache;

    TextureBindingCache::Index fLastIndex = TextureBindingCache::kInvalidIndex;
};

// Helper to manage packed fields within a uint64_t
template <uint64_t Bits, uint64_t Offset>
struct Bitfield {
    static constexpr uint64_t kMask = ((uint64_t) 1 << Bits) - 1;
    static constexpr uint64_t kOffset = Offset;
    static constexpr uint64_t kBits = Bits;

    static uint32_t get(uint64_t v) { return static_cast<uint32_t>((v >> kOffset) & kMask); }
    static uint64_t set(uint32_t v) { return (v & kMask) << kOffset; }
};

/**
 * Each Draw in a DrawList might be processed by multiple RenderSteps (determined by the Draw's
 * Renderer), which can be sorted independently. Each (step, draw) pair produces its own
 * SortKey.
 *
 * The goal of sorting draws for the DrawPass is to minimize pipeline transitions and dynamic
 * binds within a pipeline, while still respecting the overall painter's order. This decreases
 * the number of low-level draw commands in a command buffer and increases the size of those,
 * allowing the GPU to operate more efficiently and have fewer bubbles within its own
 * instruction stream.
 *
 * The Draw's CompresssedPaintersOrder and DisjointStencilINdex represent the most significant
 * bits of the key, and are shared by all SortKeys produced by the same draw. Next, the pipeline
 * description is encoded in two steps:
 *  1. The index of the RenderStep packed in the high bits to ensure each step for a draw is
 *     ordered correctly.
 *  2. An index into a cache of pipeline descriptions is used to encode the identity of the
 *     pipeline (SortKeys that differ in the bits from #1 necessarily would have different
 *     descriptions, but then the specific ordering of the RenderSteps isn't enforced). Last,
 *     the SortKey encodes an index into the set of uniform bindings accumulated for a DrawPass.
 *     This allows the SortKey to cluster draw steps that have both a compatible pipeline and do
 *     not require rebinding uniform data or other state (e.g. scissor). Since the uniform data
 *     index and the pipeline description index are packed into indices and not actual pointers,
 *     a given SortKey is only valid for the a specific DrawList->DrawPass conversion.
 */
class SortKey {
public:
    SortKey(const DrawList::Draw* draw,
            int renderStep,
            GraphicsPipelineCache::Index pipelineIndex,
            UniformDataCache::Index geomUniformIndex,
            UniformDataCache::Index shadingUniformIndex,
            TextureBindingCache::Index textureBindingIndex)
            : fPipelineKey(
                      ColorDepthOrderField::set(draw->fDrawParams.order().paintOrder().bits()) |
                      StencilIndexField::set(draw->fDrawParams.order().stencilIndex().bits()) |
                      RenderStepField::set(static_cast<uint32_t>(renderStep)) |
                      PipelineField::set(pipelineIndex))
            , fUniformKey(GeometryUniformField::set(geomUniformIndex) |
                          ShadingUniformField::set(shadingUniformIndex) |
                          TextureBindingsField::set(textureBindingIndex))
            , fDraw(draw) {
        SkASSERT(pipelineIndex < GraphicsPipelineCache::kInvalidIndex);
        SkASSERT(renderStep <= draw->fRenderer->numRenderSteps());
    }

    bool operator<(const SortKey& k) const {
        return fPipelineKey < k.fPipelineKey ||
               (fPipelineKey == k.fPipelineKey && fUniformKey < k.fUniformKey);
    }

    const RenderStep& renderStep() const {
        return fDraw->fRenderer->step(RenderStepField::get(fPipelineKey));
    }

    const DrawList::Draw& draw() const { return *fDraw; }

    GraphicsPipelineCache::Index pipelineIndex() const { return PipelineField::get(fPipelineKey); }
    UniformDataCache::Index geometryUniformIndex() const {
        return GeometryUniformField::get(fUniformKey);
    }
    UniformDataCache::Index shadingUniformIndex() const {
        return ShadingUniformField::get(fUniformKey);
    }
    TextureBindingCache::Index textureBindingIndex() const {
        return TextureBindingsField::get(fUniformKey);
    }

private:
    // Fields are ordered from most-significant to least when sorting by 128-bit value.
    // NOTE: We don't use C++ bit fields because field ordering is implementation defined and we
    // need to sort consistently.
    using ColorDepthOrderField = Bitfield<16, 48>;  // sizeof(CompressedPaintersOrder)
    using StencilIndexField = Bitfield<16, 32>;     // sizeof(DisjointStencilIndex)
    using RenderStepField = Bitfield<2, 30>;        // bits >= log2(Renderer::kMaxRenderSteps)
    using PipelineField = Bitfield<30, 0>;          // bits >= log2(max total steps in draw list)
    uint64_t fPipelineKey;

    // The uniform/texture index fields need 1 extra bit to encode "no-data". Values that are
    // greater than or equal to 2^(bits-1) represent "no-data", while values between
    // [0, 2^(bits-1)-1] can access data arrays without extra logic.
    using GeometryUniformField = Bitfield<17, 47>;  // bits >= 1+log2(max total steps)
    using ShadingUniformField = Bitfield<17, 30>;   // bits >= 1+log2(max total steps)
    using TextureBindingsField = Bitfield<30, 0>;   // bits >= 1+log2(max total steps)
    uint64_t fUniformKey;

    // Backpointer to the draw that produced the sort key
    const DrawList::Draw* fDraw;

    static_assert(ColorDepthOrderField::kBits >= sizeof(CompressedPaintersOrder));
    static_assert(StencilIndexField::kBits >= sizeof(DisjointStencilIndex));
    static_assert(RenderStepField::kBits >= SkNextLog2_portable(Renderer::kMaxRenderSteps));
    static_assert(PipelineField::kBits >= SkNextLog2_portable(DrawList::kMaxRenderSteps));
    static_assert(GeometryUniformField::kBits >=
                  1 + SkNextLog2_portable(DrawList::kMaxRenderSteps));
    static_assert(ShadingUniformField::kBits >= 1 + SkNextLog2_portable(DrawList::kMaxRenderSteps));
    static_assert(TextureBindingsField::kBits >=
                  1 + SkNextLog2_portable(DrawList::kMaxRenderSteps));
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_PipelineUtils_DEFINED