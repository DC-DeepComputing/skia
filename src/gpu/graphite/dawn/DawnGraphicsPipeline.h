/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnGraphicsPipeline_DEFINED
#define skgpu_graphite_DawnGraphicsPipeline_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/ports/SkCFObject.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/GraphicsPipeline.h"

#include <array>
#include <optional>
#include <unordered_map>

#include "webgpu/webgpu_cpp.h"

class SkUniform;

namespace skgpu {
struct BlendInfo;
}

namespace skgpu::graphite {

class Attribute;
class Context;
class GraphicsPipelineDesc;
class DawnBuffer;
class DawnResourceProvider;
class DawnSharedContext;
struct DepthStencilSettings;
struct RenderPassDesc;
struct DawnGraphicsPipelineRef;

struct DawnUniformBufferBindGroupEntry {
    bool operator!=(const DawnUniformBufferBindGroupEntry& rhs) const {
        return buffer != rhs.buffer || binding != rhs.binding || size != rhs.size;
    }

    union {
        const DawnBuffer* buffer;
        uint64_t bufferPtr64 = 0;
    };
    uint32_t binding;
    uint32_t size;
};

static_assert(sizeof(DawnUniformBufferBindGroupEntry) == 2 * sizeof(uint64_t),
              "Unexpected DawnUniformBufferBindGroupEntry size");

struct DawnUniformBufferBindGroupDesc {
public:
    DawnUniformBufferBindGroupEntry& pushEntry() { return fEntryStorage.at(fNumEntries++); }
    DawnUniformBufferBindGroupEntry& operator[](size_t idx) {
        SkASSERT(idx < numEntries());
        return fEntryStorage[idx];
    }
    const DawnUniformBufferBindGroupEntry& operator[](size_t idx) const {
        SkASSERT(idx < numEntries());
        return fEntryStorage[idx];
    }
    uint32_t numEntries() const { return fNumEntries; }
    bool operator==(const DawnUniformBufferBindGroupDesc& rhs) const {
        if (fNumEntries != rhs.fNumEntries) return false;
        for (uint32_t i = 0; i < fNumEntries; ++i) {
            if (fEntryStorage[i] != rhs.fEntryStorage[i]) return false;
        }
        return true;
    }

    uint32_t calculateHash() const;

private:
    std::array<DawnUniformBufferBindGroupEntry, 3> fEntryStorage;
    uint32_t fNumEntries = 0;
};

class DawnUniformBufferBindGroupCacheKey {
public:
    DawnUniformBufferBindGroupCacheKey(const DawnUniformBufferBindGroupDesc& desc);
    virtual ~DawnUniformBufferBindGroupCacheKey() = default;

    uint32_t hash() const { return fHashValue; }

    bool operator==(const DawnUniformBufferBindGroupCacheKey& rhs) const {
        return fDesc == rhs.fDesc;
    }

    bool operator==(const DawnUniformBufferBindGroupDesc& rhs) const { return fDesc == rhs; }

protected:
    DawnUniformBufferBindGroupDesc fDesc;
    uint32_t fHashValue;
};

class DawnUniformBufferBindGroupCacheKeepAlive {
public:
    DawnUniformBufferBindGroupCacheKeepAlive(std::weak_ptr<DawnGraphicsPipelineRef> ownerRef,
                                             const DawnUniformBufferBindGroupDesc& desc);
    ~DawnUniformBufferBindGroupCacheKeepAlive();

private:
    std::weak_ptr<DawnGraphicsPipelineRef> fOwnerRef;
    const DawnUniformBufferBindGroupDesc fDesc;
};

}  // namespace skgpu::graphite

namespace std {
template <> struct hash<skgpu::graphite::DawnUniformBufferBindGroupCacheKey> {
    size_t operator()(const skgpu::graphite::DawnUniformBufferBindGroupCacheKey& key) const {
        return key.hash();
    }
};
}  // namespace std

namespace skgpu::graphite {
class DawnGraphicsPipeline final : public GraphicsPipeline {
public:
    inline static constexpr unsigned int kUniformBufferBindGroupIndex = 0;
    inline static constexpr unsigned int kTextureBindGroupIndex = 1;

    inline static constexpr unsigned int kIntrinsicUniformBufferIndex = 0;
    inline static constexpr unsigned int kRenderStepUniformBufferIndex = 1;
    inline static constexpr unsigned int kPaintUniformBufferIndex = 2;
    inline static constexpr unsigned int kNumUniformBuffers = 3;

    inline static constexpr unsigned int kVertexBufferIndex = 0;
    inline static constexpr unsigned int kInstanceBufferIndex = 1;
    inline static constexpr unsigned int kNumVertexBuffers = 2;

    static sk_sp<DawnGraphicsPipeline> Make(const DawnSharedContext*,
                                           std::string label,
                                           wgpu::ShaderModule vsModule,
                                           SkSpan<const SkUniform> uniforms,
                                           size_t numTextures,
                                           SkSpan<const Attribute> vertexAttrs,
                                           SkSpan<const Attribute> instanceAttrs,
                                           PrimitiveType primitiveType,
                                           wgpu::ShaderModule fsModule,
                                           const DepthStencilSettings& depthStencilSettings,
                                           const BlendInfo& blendInfo,
                                           const RenderPassDesc&);

    ~DawnGraphicsPipeline() override {}

    uint32_t stencilReferenceValue() const { return fStencilReferenceValue; }
    PrimitiveType primitiveType() const { return fPrimitiveType; }
    bool hasStepUniforms() const { return fHasStepUniforms; }
    bool hasFragment() const { return fHasFragment; }
    const wgpu::RenderPipeline& dawnRenderPipeline() const;

    const wgpu::BindGroup& getOrCreateBindGroup(
            const DawnUniformBufferBindGroupDesc& uniformBufferDesc) const;
    void eraseBindGroup(const DawnUniformBufferBindGroupDesc& uniformBufferDesc) const;

private:
    DawnGraphicsPipeline(const skgpu::graphite::SharedContext* sharedContext,
                         PrimitiveType primitiveType,
                         uint32_t refValue,
                         bool hasStepUniforms,
                         bool hasFragment)
        : GraphicsPipeline(sharedContext)
        , fPrimitiveType(primitiveType)
        , fStencilReferenceValue(refValue)
        , fHasStepUniforms(hasStepUniforms)
        , fHasFragment(hasFragment) {}

    void freeGpuData() override;

    std::optional<wgpu::RenderPipeline> fRenderPipeline;
    const PrimitiveType fPrimitiveType;
    const uint32_t fStencilReferenceValue;
    const bool fHasStepUniforms;
    const bool fHasFragment;

    // Ref used for weak refererencing this object
    mutable std::shared_ptr<DawnGraphicsPipelineRef> fWeakRefCreator;

    mutable std::unordered_map<DawnUniformBufferBindGroupCacheKey, wgpu::BindGroup>
            fUniformBufferBindGroupCache;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnGraphicsPipeline_DEFINED
