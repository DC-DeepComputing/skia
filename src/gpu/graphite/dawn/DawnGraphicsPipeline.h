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
#include "src/core/SkLRUCache.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/dawn/DawnAsyncWait.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

class SkUniform;

namespace SkSL {
    class Compiler;
}
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
class RuntimeEffectDictionary;

class DawnGraphicsPipeline final : public GraphicsPipeline {
public:
    inline static constexpr unsigned int kUniformBufferBindGroupIndex = 0;
    inline static constexpr unsigned int kTextureBindGroupIndex = 1;
    inline static constexpr unsigned int kBindGroupCount = 2;

    inline static constexpr unsigned int kIntrinsicUniformBufferIndex = 0;
    inline static constexpr unsigned int kRenderStepUniformBufferIndex = 1;
    inline static constexpr unsigned int kPaintUniformBufferIndex = 2;
    inline static constexpr unsigned int kNumUniformBuffers = 3;

    inline static constexpr unsigned int kVertexBufferIndex = 0;
    inline static constexpr unsigned int kInstanceBufferIndex = 1;
    inline static constexpr unsigned int kNumVertexBuffers = 2;

    struct BufferBindGroupEntry {
        bool operator!=(const BufferBindGroupEntry& rhs) const {
            return buffer != rhs.buffer || bindingIndex != rhs.bindingIndex;
        }
        const DawnBuffer* buffer;
        uint32_t bindingIndex;
    };

    struct BufferBindGroupDesc {
        bool operator==(const BufferBindGroupDesc& rhs) const;

        std::array<BufferBindGroupEntry, 3> fEntries;
        uint32_t fNumEntries = 0;
    };

    static sk_sp<DawnGraphicsPipeline> Make(const DawnSharedContext* sharedContext,
                                            SkSL::Compiler* compiler,
                                            const RuntimeEffectDictionary* runtimeDict,
                                            const GraphicsPipelineDesc& pipelineDesc,
                                            const RenderPassDesc& renderPassDesc);

    ~DawnGraphicsPipeline() override {}

    uint32_t stencilReferenceValue() const { return fStencilReferenceValue; }
    PrimitiveType primitiveType() const { return fPrimitiveType; }
    int stepUniformsTotalBytes() const { return fRenderStepUniformsTotalBytes; }
    int paintUniformsTotalBytes() const { return fPaintUniformsTotalBytes; }
    const wgpu::RenderPipeline& dawnRenderPipeline() const;

    using BindGroupLayouts = std::array<wgpu::BindGroupLayout, kBindGroupCount>;
    const BindGroupLayouts& dawnGroupLayouts() const { return fGroupLayouts; }

    wgpu::BindGroup getOrCreateUboBindGroup(const BufferBindGroupDesc& bindGroupDesc) const;

private:
    using AsyncPipelineCreation = DawnAsyncResult<wgpu::RenderPipeline>;

    DawnGraphicsPipeline(const skgpu::graphite::SharedContext* sharedContext,
                         PipelineInfo* pipelineInfo,
                         std::unique_ptr<AsyncPipelineCreation> pipelineCreationInfo,
                         BindGroupLayouts groupLayouts,
                         PrimitiveType primitiveType,
                         uint32_t refValue,
                         int stepUniformsTotalBytes,
                         int paintUniformsTotalBytes);

    void freeGpuData() override;

    std::unique_ptr<AsyncPipelineCreation> fAsyncPipelineCreation;
    BindGroupLayouts fGroupLayouts;
    const PrimitiveType fPrimitiveType;
    const uint32_t fStencilReferenceValue;
    const int fRenderStepUniformsTotalBytes;
    const int fPaintUniformsTotalBytes;

    struct UniqueKeyHash {
        uint32_t operator()(const skgpu::UniqueKey& key) const { return key.hash(); }
    };

    using BufferBindGroupCache = SkLRUCache<UniqueKey, wgpu::BindGroup, UniqueKeyHash>;
    std::unique_ptr<BufferBindGroupCache> fUniformBufferBindGroupCache;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnGraphicsPipeline_DEFINED
