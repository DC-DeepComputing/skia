/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnResourceProvider.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/graphite/dawn/DawnTypes.h"
#include "include/private/base/SkAlign.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/dawn/DawnBuffer.h"
#include "src/gpu/graphite/dawn/DawnCommandBuffer.h"
#include "src/gpu/graphite/dawn/DawnComputePipeline.h"
#include "src/gpu/graphite/dawn/DawnErrorChecker.h"
#include "src/gpu/graphite/dawn/DawnGraphicsPipeline.h"
#include "src/gpu/graphite/dawn/DawnGraphiteTypesPriv.h"
#include "src/gpu/graphite/dawn/DawnSampler.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/gpu/graphite/dawn/DawnTexture.h"
#include "src/sksl/SkSLCompiler.h"

namespace skgpu::graphite {

namespace {

constexpr int kBufferBindingSizeAlignment = 16;
constexpr int kMaxNumberOfCachedBufferBindGroups = 1024;
constexpr int kMaxNumberOfCachedTextureBindGroups = 4096;

wgpu::ShaderModule create_shader_module(const wgpu::Device& device, const char* source) {
#ifdef WGPU_BREAKING_CHANGE_DROP_DESCRIPTOR
    wgpu::ShaderSourceWGSL wgslDesc;
#else
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
#endif
    wgslDesc.code = source;
    wgpu::ShaderModuleDescriptor descriptor;
    descriptor.nextInChain = &wgslDesc;
    return device.CreateShaderModule(&descriptor);
}

wgpu::RenderPipeline create_blit_render_pipeline(const DawnSharedContext* sharedContext,
                                                 const char* label,
                                                 wgpu::ShaderModule vsModule,
                                                 wgpu::ShaderModule fsModule,
                                                 wgpu::TextureFormat renderPassColorFormat,
                                                 wgpu::TextureFormat renderPassDepthStencilFormat,
                                                 int numSamples) {
    wgpu::RenderPipelineDescriptor descriptor;
    descriptor.label = label;
    descriptor.layout = nullptr;

    wgpu::ColorTargetState colorTarget;
    colorTarget.format = renderPassColorFormat;
    colorTarget.blend = nullptr;
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    wgpu::DepthStencilState depthStencil;
    if (renderPassDepthStencilFormat != wgpu::TextureFormat::Undefined) {
        depthStencil.format = renderPassDepthStencilFormat;
        depthStencil.depthWriteEnabled = false;
        depthStencil.depthCompare = wgpu::CompareFunction::Always;

        descriptor.depthStencil = &depthStencil;
    }

    wgpu::FragmentState fragment;
    fragment.module = std::move(fsModule);
    fragment.entryPoint = "main";
    fragment.targetCount = 1;
    fragment.targets = &colorTarget;
    descriptor.fragment = &fragment;

    descriptor.vertex.module = std::move(vsModule);
    descriptor.vertex.entryPoint = "main";
    descriptor.vertex.constantCount = 0;
    descriptor.vertex.constants = nullptr;
    descriptor.vertex.bufferCount = 0;
    descriptor.vertex.buffers = nullptr;

    descriptor.primitive.frontFace = wgpu::FrontFace::CCW;
    descriptor.primitive.cullMode = wgpu::CullMode::None;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleStrip;
    descriptor.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;

    descriptor.multisample.count = numSamples;
    descriptor.multisample.mask = 0xFFFFFFFF;
    descriptor.multisample.alphaToCoverageEnabled = false;

    std::optional<DawnErrorChecker> errorChecker;
    if (sharedContext->dawnCaps()->allowScopedErrorChecks()) {
        errorChecker.emplace(sharedContext);
    }
    auto pipeline = sharedContext->device().CreateRenderPipeline(&descriptor);
    if (errorChecker.has_value() && errorChecker->popErrorScopes() != DawnErrorType::kNoError) {
        return nullptr;
    }

    return pipeline;
}

template <size_t NumEntries>
using BindGroupKey = typename DawnResourceProvider::BindGroupKey<NumEntries>;
using UniformBindGroupKey = BindGroupKey<DawnResourceProvider::kNumUniformEntries>;

UniformBindGroupKey make_ubo_bind_group_key(
        const std::array<std::pair<const DawnBuffer*, uint32_t>,
                         DawnResourceProvider::kNumUniformEntries>& boundBuffersAndSizes) {
    UniformBindGroupKey uniqueKey;
    {
        // Each entry in the bind group needs 2 uint32_t in the key:
        //  - buffer's unique ID: 32 bits.
        //  - buffer's binding size: 32 bits.
        // We need total of 4 entries in the uniform buffer bind group.
        // Unused entries will be assigned zero values.
        UniformBindGroupKey::Builder builder(&uniqueKey);

        for (uint32_t i = 0; i < boundBuffersAndSizes.size(); ++i) {
            const DawnBuffer* boundBuffer = boundBuffersAndSizes[i].first;
            const uint32_t bindingSize = boundBuffersAndSizes[i].second;
            if (boundBuffer) {
                builder[2 * i] = boundBuffer->uniqueID().asUInt();
                builder[2 * i + 1] = bindingSize;
            } else {
                builder[2 * i] = 0;
                builder[2 * i + 1] = 0;
            }
        }

        builder.finish();
    }

    return uniqueKey;
}

BindGroupKey<1> make_texture_bind_group_key(const DawnSampler* sampler,
                                            const DawnTexture* texture) {
    BindGroupKey<1> uniqueKey;
    {
        BindGroupKey<1>::Builder builder(&uniqueKey);

        builder[0] = sampler->uniqueID().asUInt();
        builder[1] = texture->uniqueID().asUInt();

        builder.finish();
    }

    return uniqueKey;
}
}  // namespace

// This represents a single block of instrinsic constants. It has a 'hash' as its unique cache ID.
// It contains a 'BindBufferInfo' for render passes to use. It also tracks its containing
// 'IntrinsicBuffer', and counts the number of active command buffer usages, so that when any
// entry is still in use, its containing buffer mustn't be purged.
class DawnResourceProvider::IntrinsicEntry final : public SkRefCnt {
public:
    static sk_sp<IntrinsicEntry> Make(uint32_t hash,
                                      IntrinsicBuffer* buffer,
                                      BindBufferInfo binding,
                                      IntrinsicConstantsManager* manager) {
        return sk_sp<IntrinsicEntry>(new IntrinsicEntry(hash, buffer, binding, manager));
    }

    ~IntrinsicEntry() override {
        SkASSERT(!hasUsageRef());
    }

    uint32_t hash() const { return fHash; }
    const BindBufferInfo& binding() const { return fBinding; }
    IntrinsicBuffer* buffer() const { return fBuffer; }

    void updateAccessTime() {
        fLastAccess = skgpu::StdSteadyClock::now();
    }
    skgpu::StdSteadyClock::time_point lastAccessTime() const {
        return fLastAccess;
    }

    void refUsage() {
        fUsageRefCnt++;
    }
    void unrefUsage() {
        fUsageRefCnt--;
    }
    bool hasUsageRef() const {
        return fUsageRefCnt > 0;
    }

    // Purge the entry's cache in its manager.
    void purge();

    void trackCommandBuffer(DawnCommandBuffer* cb);

private:
    IntrinsicEntry(uint32_t hash,
                   IntrinsicBuffer* buffer,
                   BindBufferInfo binding,
                   IntrinsicConstantsManager* manager)
            : fHash(hash),
              fBuffer(buffer),
              fBinding(binding),
              fManager(manager),
              fUsageRefCnt(0) {
        fLastAccess = skgpu::StdSteadyClock::now();
    }

    uint32_t fHash;
    // The buffer containing this entry.
    IntrinsicBuffer* const fBuffer;
    BindBufferInfo fBinding;
    IntrinsicConstantsManager* const fManager;

    uint32_t fUsageRefCnt = 0;
    // The last time when any of the entry's command buffers finishes executing.
    skgpu::StdSteadyClock::time_point fLastAccess;
};


// Wraps a Dawn buffer, and contains a list of 'IntrinsicEntry's that are using this buffer.
class DawnResourceProvider::IntrinsicBuffer final {
public:
    static constexpr int kNumSlots = 8;

    static IntrinsicBuffer* Make(sk_sp<DawnBuffer>&& dawnBuffer,
                                 IntrinsicConstantsManager* manager) {
        return new IntrinsicBuffer(std::move(dawnBuffer), manager);
    }

    ~IntrinsicBuffer() = default;

    sk_sp<DawnBuffer> buffer() const { return fDawnBuffer; }

    // Appends an entry using this buffer.
    void addEntry(const sk_sp<IntrinsicEntry>& entry) {
        fEntries.push_back(entry);
    }

    // Returns true if any of its entries is in use.
    bool hasEntryInUse();

    // Returns the last access time of the buffer.
    skgpu::StdSteadyClock::time_point lastAccessTime() const;

    // Purge the buffer's cache in its manager.
    void purge();

private:
    IntrinsicBuffer(sk_sp<DawnBuffer>&& dawnBuffer, IntrinsicConstantsManager* manager)
            : fDawnBuffer(std::move(dawnBuffer)), fManager(manager) {
    }

    using EntryArray = skia_private::STArray<kNumSlots, sk_sp<IntrinsicEntry>>;
    EntryArray fEntries;

    sk_sp<DawnBuffer> fDawnBuffer;
    IntrinsicConstantsManager* const fManager;

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(IntrinsicBuffer);
};

// DawnResourceProvider::IntrinsicConstantsManager
// ----------------------------------------------------------------------------

/**
 * Since Dawn does not currently provide push constants, this helper class manages rotating through
 * buffers and writing each new occurrence of a set of intrinsic uniforms into the current buffer.
 */
class DawnResourceProvider::IntrinsicConstantsManager {
public:
    explicit IntrinsicConstantsManager(DawnResourceProvider* resourceProvider)
            : fResourceProvider(resourceProvider) {}

    ~IntrinsicConstantsManager();

    // Find or create a bind buffer info for the given intrinsic values used in the given command
    // buffer.
    BindBufferInfo add(DawnCommandBuffer* cb, const UniformDataBlock& intrinsicValues);

    void freeGpuResources();
    void purgeResourcesNotUsedSince(StdSteadyClock::time_point purgeTime);

private:
   friend class IntrinsicEntry;
   friend class IntrinsicBuffer;

   // The minimum number of intrinsic buffers to keep around in the cache.
   static constexpr uint32_t kMinNumBuffers = 128;

   // Traverse the intrinsic buffers in LRU order and purge the ones that match the 'pred' until the
   // remaining buffer count is less than 'lowWatermark'.
   template<typename T> void purgeBuffersIf(T pred, uint32_t lowWatermark = kMinNumBuffers);

    // Removes the buffer and returns its dawn buffer.
    void purgeBuffer(IntrinsicBuffer* buffer);

    // Marks the 'entry' as in use by the 'cb'.
    void markEntryAsUsed(DawnCommandBuffer* cb, sk_sp<IntrinsicEntry> entry);

    void invalidateEntry(uint32_t hash);
    void invalidateBuffer(IntrinsicBuffer* buffer);

    DawnResourceProvider* const fResourceProvider;
    // The current buffer being filled up, as well as the how much of it has been written to.
    IntrinsicBuffer* fCurrentBuffer = nullptr;
    int fSlotsUsed = 0; // in multiples of the intrinsic uniform size and UBO binding requirement
    DawnCommandBuffer* fCurrentCommandBuffer = nullptr;

    // All cached intrinsic entries, indexed by their hash.
    skia_private::THashMap<uint32_t, sk_sp<IntrinsicEntry>> fIntrinsicEntries;
    // All cached intrinsic buffers, in LRU order.
    SkTInternalLList<IntrinsicBuffer> fIntrinsicBuffersLRU;
    uint32_t fNumBuffers = 0;

};

DawnResourceProvider::IntrinsicConstantsManager::~IntrinsicConstantsManager() {
    auto alwaysTrue = [](IntrinsicBuffer* buffer) {
        SkASSERT(!buffer->hasEntryInUse());
        return true;
    };
    purgeBuffersIf(alwaysTrue, 0);

    SkASSERT(fIntrinsicBuffersLRU.isEmpty());
    SkASSERT(fIntrinsicEntries.empty());
}

// Find or create a bind buffer info for the given intrinsic values used in the given command
// buffer.
BindBufferInfo DawnResourceProvider::IntrinsicConstantsManager::add(
        DawnCommandBuffer* cb, const UniformDataBlock& intrinsicValues) {
    auto hasher = UniformDataBlock::Hash();
    uint32_t hash = hasher(intrinsicValues);
    sk_sp<IntrinsicEntry>* existing = fIntrinsicEntries.find(hash);
    if (existing) {
        auto entry = *existing;
        markEntryAsUsed(cb, entry);
        return entry->binding();
    }

    // TODO: https://b.corp.google.com/issues/259267703
    // Make updating intrinsic constants faster. Metal has setVertexBytes method to quickly send
    // intrinsic constants to vertex shader without any buffer. But Dawn doesn't have similar
    // capability. So we have to use WriteBuffer(), and this method is not allowed to be called when
    // there is an active render pass.
    SkASSERT(!cb->hasActivePassEncoder());

    const Caps* caps = fResourceProvider->dawnSharedContext()->caps();
    const uint32_t stride =
            SkAlignTo(intrinsicValues.size(), caps->requiredUniformBufferAlignment());
    // In any one of the following cases, we need to create a new buffer:
    //     (1) There is no current buffer.
    //     (2) The current buffer is full.
    //     (3) The current command buffer has changed. In this case, we'd better start a new buffer,
    //         otherwise we could update a buffer which is still in use by the previous command
    //         buffer.
    if (!fCurrentBuffer || fSlotsUsed == IntrinsicBuffer::kNumSlots ||
        cb != fCurrentCommandBuffer) {
        // We can just replace the current buffer; any prior buffer was already tracked in the LRU
        // list and the intrinsic constants were written directly to the Dawn queue.
        DawnResourceProvider* resourceProvider = fResourceProvider;
        auto dawnBuffer =
                resourceProvider->findOrCreateDawnBuffer(stride * IntrinsicBuffer::kNumSlots,
                                                         BufferType::kUniform,
                                                         AccessPattern::kGpuOnly,
                                                         "IntrinsicConstantBuffer");
        fSlotsUsed = 0;

        if (!dawnBuffer) {
            // If we failed to create a GPU buffer to hold the intrinsic uniforms, we will fail the
            // Recording being inserted, so return an empty bind info.
            return {};
        }

        fCurrentBuffer = IntrinsicBuffer::Make(std::move(dawnBuffer), this);
        fIntrinsicBuffersLRU.addToHead(fCurrentBuffer);
        fNumBuffers++;
    }

    SkASSERT(fCurrentBuffer && fSlotsUsed < IntrinsicBuffer::kNumSlots);
    uint32_t offset = (fSlotsUsed++) * stride;
    fResourceProvider->dawnSharedContext()->queue().WriteBuffer(
            fCurrentBuffer->buffer()->dawnBuffer(),
            offset,
            intrinsicValues.data(),
            intrinsicValues.size());

    BindBufferInfo binding{
            fCurrentBuffer->buffer().get(), offset, SkTo<uint32_t>(intrinsicValues.size())};
    sk_sp<IntrinsicEntry> entry = IntrinsicEntry::Make(hash, fCurrentBuffer, binding, this);
    fCurrentBuffer->addEntry(entry);
    fIntrinsicEntries.set(hash, entry);

    markEntryAsUsed(cb, entry);

    return binding;
}

void DawnResourceProvider::IntrinsicConstantsManager::freeGpuResources() {
    purgeResourcesNotUsedSince(skgpu::StdSteadyClock::now());
}

void DawnResourceProvider::IntrinsicConstantsManager::purgeResourcesNotUsedSince(
        StdSteadyClock::time_point purgeTime) {
    // We keep the buffers that are currently in use. We also keep the current buffer as it is
    // likely to be used again soon.
    auto bufferNotUsedSince = [purgeTime, this](IntrinsicBuffer* buffer) {
        return !buffer->hasEntryInUse() && buffer != fCurrentBuffer &&
               buffer->lastAccessTime() < purgeTime;
    };
    purgeBuffersIf(bufferNotUsedSince);
}

template <typename T> void DawnResourceProvider::IntrinsicConstantsManager::purgeBuffersIf(
        T pred, uint32_t lowWatermark) {
    if (fNumBuffers <= lowWatermark) {
        return;
    }

    uint32_t bufferPurged = 0;
    using Iter = SkTInternalLList<IntrinsicBuffer>::Iter;
    Iter iter;
    auto* curr = iter.init(fIntrinsicBuffersLRU, Iter::kTail_IterStart);
    while (curr != nullptr && bufferPurged < fNumBuffers - lowWatermark) {
        auto* prev = iter.prev();
        if (pred(curr)) {
            purgeBuffer(curr);
            bufferPurged++;
        }
        curr = prev;
    }
}

// Removes the buffer and returns its dawn buffer.
void DawnResourceProvider::IntrinsicConstantsManager::purgeBuffer(IntrinsicBuffer* buffer) {
    if (buffer == fCurrentBuffer) {
        fCurrentBuffer = nullptr;
        fSlotsUsed = 0;
    }
    buffer->purge();
    delete buffer;
}

// Marks the 'entry' as in use by the 'cb'.
void DawnResourceProvider::IntrinsicConstantsManager::markEntryAsUsed(
        DawnCommandBuffer* cb, sk_sp<IntrinsicEntry> entry) {
    fCurrentCommandBuffer = cb;
    entry->trackCommandBuffer(cb);

    // Move the containing buffer to the head of the LRU list.
    auto* buffer = entry->buffer();
    if (buffer != fIntrinsicBuffersLRU.head()) {
        fIntrinsicBuffersLRU.remove(buffer);
        fIntrinsicBuffersLRU.addToHead(buffer);
    }
}

void DawnResourceProvider::IntrinsicConstantsManager::invalidateEntry(uint32_t hash) {
    fIntrinsicEntries.remove(hash);
}

void DawnResourceProvider::IntrinsicConstantsManager::invalidateBuffer(IntrinsicBuffer* buffer) {
    fIntrinsicBuffersLRU.remove(buffer);
    fNumBuffers--;
}

// DawnResourceProvider::IntrinsicConstantsManager
// ----------------------------------------------------------------------------


// callback for when a command buffer finishes its execution.
void DawnResourceProviderIntrinsiceEntryFinishedCommandBufferUse(void* context) {
    auto* entry = static_cast<DawnResourceProvider::IntrinsicEntry*>(context);
    entry->unrefUsage();
    // Update the entry's last access time.
    entry->updateAccessTime();
}


// ----------------------------------------------------------------------------
// DawnResourceProvider::IntrinsicEntry

// Tracks the atctive usage of the entry by the command buffer. We mustn't purge an entry while it
// is still in use by any command buffer.
void DawnResourceProvider::IntrinsicEntry::trackCommandBuffer(DawnCommandBuffer* cb) {
    refUsage();
    sk_sp<RefCntedCallback> callback = RefCntedCallback::Make(
        DawnResourceProviderIntrinsiceEntryFinishedCommandBufferUse, this);
    cb->addFinishedProc(std::move(callback));
}

void DawnResourceProvider::IntrinsicEntry::purge() {
    fManager->invalidateEntry(fHash);
}

// DawnResourceProvider::IntrinsicEntry
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// DawnResourceProvider::IntrinsicBuffer

bool DawnResourceProvider::IntrinsicBuffer::hasEntryInUse() {
    for (auto& entry : fEntries) {
        if (entry->hasUsageRef()) {
            return true;
        }
    }
    return false;
}

skgpu::StdSteadyClock::time_point DawnResourceProvider::IntrinsicBuffer::lastAccessTime() const {
    auto lastAccessTime = fEntries.back()->lastAccessTime();
    // The last access time of all its entries.
    for (auto& entry : fEntries) {
        lastAccessTime = std::max(lastAccessTime, entry->lastAccessTime());
    }
    return lastAccessTime;
}

void DawnResourceProvider::IntrinsicBuffer::purge() {
    for (auto& entry : fEntries) {
        entry->purge();
    }

    fManager->invalidateBuffer(this);
}

// DawnResourceProvider::IntrinsicBuffer
// ----------------------------------------------------------------------------


DawnResourceProvider::DawnResourceProvider(SharedContext* sharedContext,
                                           SingleOwner* singleOwner,
                                           uint32_t recorderID,
                                           size_t resourceBudget)
        : ResourceProvider(sharedContext, singleOwner, recorderID, resourceBudget)
        , fUniformBufferBindGroupCache(kMaxNumberOfCachedBufferBindGroups)
        , fSingleTextureSamplerBindGroups(kMaxNumberOfCachedTextureBindGroups) {
    fIntrinsicConstantsManager = std::make_unique<IntrinsicConstantsManager>(this);
}

DawnResourceProvider::~DawnResourceProvider() = default;

wgpu::RenderPipeline DawnResourceProvider::findOrCreateBlitWithDrawPipeline(
        const RenderPassDesc& renderPassDesc) {
    uint32_t renderPassKey =
            this->dawnSharedContext()->dawnCaps()->getRenderPassDescKeyForPipeline(renderPassDesc);
    wgpu::RenderPipeline pipeline = fBlitWithDrawPipelines[renderPassKey];
    if (!pipeline) {
        static constexpr char kVertexShaderText[] = R"(
            var<private> fullscreenTriPositions : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
                vec2(-1.0, -1.0), vec2(-1.0, 3.0), vec2(3.0, -1.0));

            @vertex
            fn main(@builtin(vertex_index) vertexIndex : u32) -> @builtin(position) vec4<f32> {
                return vec4(fullscreenTriPositions[vertexIndex], 1.0, 1.0);
            }
        )";

        static constexpr char kFragmentShaderText[] = R"(
            @group(0) @binding(0) var colorMap: texture_2d<f32>;

            @fragment
            fn main(@builtin(position) fragPosition : vec4<f32>) -> @location(0) vec4<f32> {
                var coords : vec2<i32> = vec2<i32>(i32(fragPosition.x), i32(fragPosition.y));
                return textureLoad(colorMap, coords, 0);
            }
        )";

        auto vsModule = create_shader_module(dawnSharedContext()->device(), kVertexShaderText);
        auto fsModule = create_shader_module(dawnSharedContext()->device(), kFragmentShaderText);

        pipeline = create_blit_render_pipeline(
                dawnSharedContext(),
                /*label=*/"BlitWithDraw",
                std::move(vsModule),
                std::move(fsModule),
                /*renderPassColorFormat=*/
                TextureInfos::GetDawnViewFormat(renderPassDesc.fColorAttachment.fTextureInfo),
                /*renderPassDepthStencilFormat=*/
                renderPassDesc.fDepthStencilAttachment.fTextureInfo.isValid()
                        ? TextureInfos::GetDawnViewFormat(
                                  renderPassDesc.fDepthStencilAttachment.fTextureInfo)
                        : wgpu::TextureFormat::Undefined,
                /*numSamples=*/renderPassDesc.fColorAttachment.fTextureInfo.numSamples());

        if (pipeline) {
            fBlitWithDrawPipelines.set(renderPassKey, pipeline);
        }
    }

    return pipeline;
}

sk_sp<Texture> DawnResourceProvider::onCreateWrappedTexture(const BackendTexture& texture) {
    // Convert to smart pointers. wgpu::Texture* constructor will increment the ref count.
    wgpu::Texture dawnTexture         = BackendTextures::GetDawnTexturePtr(texture);
    wgpu::TextureView dawnTextureView = BackendTextures::GetDawnTextureViewPtr(texture);
    SkASSERT(!dawnTexture || !dawnTextureView);

    if (!dawnTexture && !dawnTextureView) {
        return {};
    }

    if (dawnTexture) {
        return DawnTexture::MakeWrapped(this->dawnSharedContext(),
                                        texture.dimensions(),
                                        texture.info(),
                                        std::move(dawnTexture));
    } else {
        return DawnTexture::MakeWrapped(this->dawnSharedContext(),
                                        texture.dimensions(),
                                        texture.info(),
                                        std::move(dawnTextureView));
    }
}

sk_sp<DawnTexture> DawnResourceProvider::findOrCreateDiscardableMSAALoadTexture(
        SkISize dimensions, const TextureInfo& msaaInfo) {
    SkASSERT(msaaInfo.isValid());

    // Derive the load texture's info from MSAA texture's info.
    DawnTextureInfo dawnMsaaLoadTextureInfo;
    SkAssertResult(TextureInfos::GetDawnTextureInfo(msaaInfo, &dawnMsaaLoadTextureInfo));
    dawnMsaaLoadTextureInfo.fSampleCount = 1;
    dawnMsaaLoadTextureInfo.fUsage |= wgpu::TextureUsage::TextureBinding;

#if !defined(__EMSCRIPTEN__)
    // MSAA texture can be transient attachment (memoryless) but the load texture cannot be.
    // This is because the load texture will need to have its content retained between two passes
    // loading:
    // - first pass: the resolve texture is blitted to the load texture.
    // - 2nd pass: the actual render pass is started and the load texture is blitted to the MSAA
    // texture.
    dawnMsaaLoadTextureInfo.fUsage &= (~wgpu::TextureUsage::TransientAttachment);
#endif

    auto texture = this->findOrCreateDiscardableMSAAAttachment(
            dimensions, TextureInfos::MakeDawn(dawnMsaaLoadTextureInfo));

    return sk_sp<DawnTexture>(static_cast<DawnTexture*>(texture.release()));
}

sk_sp<GraphicsPipeline> DawnResourceProvider::createGraphicsPipeline(
        const RuntimeEffectDictionary* runtimeDict,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc) {
    return DawnGraphicsPipeline::Make(this->dawnSharedContext(),
                                      this,
                                      runtimeDict,
                                      pipelineDesc,
                                      renderPassDesc);
}

sk_sp<ComputePipeline> DawnResourceProvider::createComputePipeline(
        const ComputePipelineDesc& desc) {
    return DawnComputePipeline::Make(this->dawnSharedContext(), desc);
}

sk_sp<Texture> DawnResourceProvider::createTexture(SkISize dimensions,
                                                   const TextureInfo& info,
                                                   skgpu::Budgeted budgeted) {
    return DawnTexture::Make(this->dawnSharedContext(),
                             dimensions,
                             info,
                             budgeted);
}

sk_sp<Buffer> DawnResourceProvider::createBuffer(size_t size,
                                                 BufferType type,
                                                 AccessPattern accessPattern) {
    return DawnBuffer::Make(this->dawnSharedContext(), size, type, accessPattern);
}

sk_sp<Sampler> DawnResourceProvider::createSampler(const SamplerDesc& samplerDesc) {
    return DawnSampler::Make(this->dawnSharedContext(), samplerDesc);
}

BackendTexture DawnResourceProvider::onCreateBackendTexture(SkISize dimensions,
                                                            const TextureInfo& info) {
    wgpu::Texture texture = DawnTexture::MakeDawnTexture(this->dawnSharedContext(),
                                                         dimensions,
                                                         info);
    if (!texture) {
        return {};
    }

    return BackendTextures::MakeDawn(texture.MoveToCHandle());
}

void DawnResourceProvider::onDeleteBackendTexture(const BackendTexture& texture) {
    SkASSERT(texture.isValid());
    SkASSERT(texture.backend() == BackendApi::kDawn);

    // Automatically release the pointers in wgpu::TextureView & wgpu::Texture's dtor.
    // Acquire() won't increment the ref count.
    wgpu::TextureView::Acquire(BackendTextures::GetDawnTextureViewPtr(texture));
    // We need to explicitly call Destroy() here since since that is the recommended way to delete
    // a Dawn texture predictably versus just dropping a ref and relying on garbage collection.
    //
    // Additionally this helps to work around an issue where Skia may have cached a BindGroup that
    // references the underlying texture. Skia currently doesn't destroy BindGroups when its use of
    // the texture goes away, thus a ref to the texture remains on the BindGroup and memory is never
    // cleared up unless we call Destroy() here.
    wgpu::Texture::Acquire(BackendTextures::GetDawnTexturePtr(texture)).Destroy();
}

DawnSharedContext* DawnResourceProvider::dawnSharedContext() const {
    return static_cast<DawnSharedContext*>(fSharedContext);
}

sk_sp<DawnBuffer> DawnResourceProvider::findOrCreateDawnBuffer(size_t size,
                                                               BufferType type,
                                                               AccessPattern accessPattern,
                                                               std::string_view label) {
    sk_sp<Buffer> buffer = this->findOrCreateBuffer(size, type, accessPattern, std::move(label));
    DawnBuffer* ptr = static_cast<DawnBuffer*>(buffer.release());
    return sk_sp<DawnBuffer>(ptr);
}

const wgpu::BindGroupLayout& DawnResourceProvider::getOrCreateUniformBuffersBindGroupLayout() {
    if (fUniformBuffersBindGroupLayout) {
        return fUniformBuffersBindGroupLayout;
    }

    std::array<wgpu::BindGroupLayoutEntry, 4> entries;
    entries[0].binding = DawnGraphicsPipeline::kIntrinsicUniformBufferIndex;
    entries[0].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[0].buffer.type = wgpu::BufferBindingType::Uniform;
    entries[0].buffer.hasDynamicOffset = true;
    entries[0].buffer.minBindingSize = 0;

    entries[1].binding = DawnGraphicsPipeline::kRenderStepUniformBufferIndex;
    entries[1].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[1].buffer.type = fSharedContext->caps()->storageBufferSupport()
                                     ? wgpu::BufferBindingType::ReadOnlyStorage
                                     : wgpu::BufferBindingType::Uniform;
    entries[1].buffer.hasDynamicOffset = true;
    entries[1].buffer.minBindingSize = 0;

    entries[2].binding = DawnGraphicsPipeline::kPaintUniformBufferIndex;
    entries[2].visibility = wgpu::ShaderStage::Fragment;
    entries[2].buffer.type = fSharedContext->caps()->storageBufferSupport()
                                     ? wgpu::BufferBindingType::ReadOnlyStorage
                                     : wgpu::BufferBindingType::Uniform;
    entries[2].buffer.hasDynamicOffset = true;
    entries[2].buffer.minBindingSize = 0;

    // Gradient buffer will only be used when storage buffers are preferred, else large
    // gradients use a texture fallback, set binding type as a uniform when not in use to
    // satisfy any binding type restricions for non-supported ssbo devices.
    entries[3].binding = DawnGraphicsPipeline::kGradientBufferIndex;
    entries[3].visibility = wgpu::ShaderStage::Fragment;
    entries[3].buffer.type = fSharedContext->caps()->storageBufferSupport()
                                     ? wgpu::BufferBindingType::ReadOnlyStorage
                                     : wgpu::BufferBindingType::Uniform;
    entries[3].buffer.hasDynamicOffset = true;
    entries[3].buffer.minBindingSize = 0;

    wgpu::BindGroupLayoutDescriptor groupLayoutDesc;
    if (fSharedContext->caps()->setBackendLabels()) {
        groupLayoutDesc.label = "Uniform buffers bind group layout";
    }

    groupLayoutDesc.entryCount = entries.size();
    groupLayoutDesc.entries = entries.data();
    fUniformBuffersBindGroupLayout =
            this->dawnSharedContext()->device().CreateBindGroupLayout(&groupLayoutDesc);

    return fUniformBuffersBindGroupLayout;
}

const wgpu::BindGroupLayout&
DawnResourceProvider::getOrCreateSingleTextureSamplerBindGroupLayout() {
    if (fSingleTextureSamplerBindGroupLayout) {
        return fSingleTextureSamplerBindGroupLayout;
    }

    std::array<wgpu::BindGroupLayoutEntry, 2> entries;

    entries[0].binding = 0;
    entries[0].visibility = wgpu::ShaderStage::Fragment;
    entries[0].sampler.type = wgpu::SamplerBindingType::Filtering;

    entries[1].binding = 1;
    entries[1].visibility = wgpu::ShaderStage::Fragment;
    entries[1].texture.sampleType = wgpu::TextureSampleType::Float;
    entries[1].texture.viewDimension = wgpu::TextureViewDimension::e2D;
    entries[1].texture.multisampled = false;

    wgpu::BindGroupLayoutDescriptor groupLayoutDesc;
    if (fSharedContext->caps()->setBackendLabels()) {
        groupLayoutDesc.label = "Single texture + sampler bind group layout";
    }

    groupLayoutDesc.entryCount = entries.size();
    groupLayoutDesc.entries = entries.data();
    fSingleTextureSamplerBindGroupLayout =
            this->dawnSharedContext()->device().CreateBindGroupLayout(&groupLayoutDesc);

    return fSingleTextureSamplerBindGroupLayout;
}

const wgpu::Buffer& DawnResourceProvider::getOrCreateNullBuffer() {
    if (!fNullBuffer) {
        wgpu::BufferDescriptor desc;
        if (fSharedContext->caps()->setBackendLabels()) {
            desc.label = "UnusedBufferSlot";
        }
        desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform |
                     wgpu::BufferUsage::Storage;
        desc.size = kBufferBindingSizeAlignment;
        desc.mappedAtCreation = false;

        fNullBuffer = this->dawnSharedContext()->device().CreateBuffer(&desc);
        SkASSERT(fNullBuffer);
    }

    return fNullBuffer;
}

const wgpu::BindGroup& DawnResourceProvider::findOrCreateUniformBuffersBindGroup(
        const std::array<std::pair<const DawnBuffer*, uint32_t>, kNumUniformEntries>&
                boundBuffersAndSizes) {
    auto key = make_ubo_bind_group_key(boundBuffersAndSizes);
    auto* existingBindGroup = fUniformBufferBindGroupCache.find(key);
    if (existingBindGroup) {
        // cache hit.
        return *existingBindGroup;
    }

    // Translate to wgpu::BindGroupDescriptor
    std::array<wgpu::BindGroupEntry, kNumUniformEntries> entries;

    constexpr uint32_t kBindingIndices[] = {
        DawnGraphicsPipeline::kIntrinsicUniformBufferIndex,
        DawnGraphicsPipeline::kRenderStepUniformBufferIndex,
        DawnGraphicsPipeline::kPaintUniformBufferIndex,
        DawnGraphicsPipeline::kGradientBufferIndex,
    };

    for (uint32_t i = 0; i < boundBuffersAndSizes.size(); ++i) {
        const DawnBuffer* boundBuffer = boundBuffersAndSizes[i].first;
        const uint32_t bindingSize = boundBuffersAndSizes[i].second;

        entries[i].binding = kBindingIndices[i];
        entries[i].offset = 0;
        if (boundBuffer) {
            entries[i].buffer = boundBuffer->dawnBuffer();
            entries[i].size = SkAlignTo(bindingSize, kBufferBindingSizeAlignment);
        } else {
            entries[i].buffer = this->getOrCreateNullBuffer();
            entries[i].size = wgpu::kWholeSize;
        }
    }

    wgpu::BindGroupDescriptor desc;
    desc.layout = this->getOrCreateUniformBuffersBindGroupLayout();
    desc.entryCount = entries.size();
    desc.entries = entries.data();

    const auto& device = this->dawnSharedContext()->device();
    auto bindGroup = device.CreateBindGroup(&desc);

    return *fUniformBufferBindGroupCache.insert(key, bindGroup);
}

const wgpu::BindGroup& DawnResourceProvider::findOrCreateSingleTextureSamplerBindGroup(
        const DawnSampler* sampler, const DawnTexture* texture) {
    auto key = make_texture_bind_group_key(sampler, texture);
    auto* existingBindGroup = fSingleTextureSamplerBindGroups.find(key);
    if (existingBindGroup) {
        // cache hit.
        return *existingBindGroup;
    }

    std::array<wgpu::BindGroupEntry, 2> entries;

    entries[0].binding = 0;
    entries[0].sampler = sampler->dawnSampler();
    entries[1].binding = 1;
    entries[1].textureView = texture->sampleTextureView();

    wgpu::BindGroupDescriptor desc;
    desc.layout = getOrCreateSingleTextureSamplerBindGroupLayout();
    desc.entryCount = entries.size();
    desc.entries = entries.data();

    const auto& device = this->dawnSharedContext()->device();
    auto bindGroup = device.CreateBindGroup(&desc);

    return *fSingleTextureSamplerBindGroups.insert(key, bindGroup);
}

void DawnResourceProvider::freeGpuResources() {
    fIntrinsicConstantsManager->freeGpuResources();
    ResourceProvider::freeGpuResources();
}

void DawnResourceProvider::purgeResourcesNotUsedSince(StdSteadyClock::time_point purgeTime) {
    fIntrinsicConstantsManager->purgeResourcesNotUsedSince(purgeTime);
    ResourceProvider::purgeResourcesNotUsedSince(purgeTime);
}

BindBufferInfo DawnResourceProvider::findOrCreateIntrinsicBindBufferInfo(
        DawnCommandBuffer* cb, const UniformDataBlock& intrinsicValues) {
    return fIntrinsicConstantsManager->add(cb, intrinsicValues);
}

} // namespace skgpu::graphite
