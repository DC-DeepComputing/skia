/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnSharedContext_DEFINED
#define skgpu_graphite_DawnSharedContext_DEFINED

#include "src/gpu/graphite/SharedContext.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "src/core/SkTHash.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/graphite/ThreadSafeResourceProvider.h"
#include "src/gpu/graphite/dawn/DawnCaps.h"

#include <string>

namespace skgpu::graphite {

struct ContextOptions;
struct DawnBackendContext;

class DawnThreadSafeResourceProvider final : public ThreadSafeResourceProvider {
public:
    DawnThreadSafeResourceProvider(std::unique_ptr<ResourceProvider>);
};

class DawnSharedContext final : public SharedContext {
public:
    static sk_sp<SharedContext> Make(const DawnBackendContext&, const ContextOptions&);
    ~DawnSharedContext() override;

    // Scoped RAII struct that tracks an asynchronous pipeline creation in DawnSharedContext.
    // When constructed via trackPipelineCreationTime, the pipeline label and start timestamp are
    // recorded in DawnSharedContext. Destroying this struct automatically removes the tracking entry
    // from DawnSharedContext's internal hash table.
    struct PipelineCreationTimer {
        PipelineCreationTimer() = default;
        PipelineCreationTimer(const DawnSharedContext* sharedContext, std::string label)
                : fSharedContext(sharedContext), fLabel(std::move(label)) {}
        PipelineCreationTimer(PipelineCreationTimer&& other)
                : fSharedContext(other.fSharedContext), fLabel(std::move(other.fLabel)) {
            other.fSharedContext = nullptr;
        }
        PipelineCreationTimer& operator=(PipelineCreationTimer&& other) {
            if (this != &other) {
                if (fSharedContext && !fLabel.empty()) {
                    SkAutoMutexExclusive lock(fSharedContext->fPendingPipelineAsyncCreationsMutex);
                    fSharedContext->fPendingPipelineAsyncCreations.removeIfExists(fLabel);
                }
                fSharedContext = other.fSharedContext;
                fLabel = std::move(other.fLabel);
                other.fSharedContext = nullptr;
            }
            return *this;
        }
        PipelineCreationTimer(const PipelineCreationTimer&) = delete;
        PipelineCreationTimer& operator=(const PipelineCreationTimer&) = delete;

        ~PipelineCreationTimer();

    private:
        const DawnSharedContext* fSharedContext = nullptr;
        std::string fLabel;
    };

    PipelineCreationTimer trackPipelineCreationTime(const std::string& label) const;
    std::vector<DawnPendingPipelineCreationInfo> getPendingPipelineCreationInfos() const;

    DawnThreadSafeResourceProvider* threadSafeResourceProvider() const;

    std::unique_ptr<ResourceProvider> makeResourceProvider(SingleOwner*,
                                                           uint32_t recorderID,
                                                           size_t resourceBudget) override;

    const DawnCaps* dawnCaps() const { return static_cast<const DawnCaps*>(this->caps()); }
    const wgpu::Device& device() const { return fDevice; }
    const wgpu::Queue& queue() const { return fQueue; }
    const wgpu::Instance& instance() const { return fInstance; }
    const wgpu::ShaderModule& noopFragment() const { return fNoopFragment; }

    bool hasTick() const { return fTick; }

    void tick() const {
        SkASSERT(this->hasTick());
        fTick(fInstance);
    }

    void deviceTick(Context*) override;

    const wgpu::BindGroupLayout& getUniformBuffersBindGroupLayout() const {
        return fUniformBuffersBindGroupLayout;
    }
    const wgpu::BindGroupLayout& getSingleTextureSamplerBindGroupLayout() const {
        return fSingleTextureSamplerBindGroupLayout;
    }

private:
    DawnSharedContext(const DawnBackendContext&,
                      std::unique_ptr<const DawnCaps>,
                      wgpu::ShaderModule noopFragment,
                      SkExecutor*,
                      SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects);

    void createUniformBuffersBindGroupLayout();
    void createSingleTextureSamplerBindGroupLayout();

    sk_sp<GraphicsPipeline> createGraphicsPipeline(const RuntimeEffectDictionary*,
                                                   const UniqueKey&,
                                                   const GraphicsPipelineDesc&,
                                                   const RenderPassDesc&,
                                                   SkEnumBitMask<PipelineCreationFlags>,
                                                   uint32_t compilationID) override;

    wgpu::Instance     fInstance;
    wgpu::Device       fDevice;
    wgpu::Queue        fQueue;
    DawnTickFunction*  fTick;
    // A noop fragment shader, it is used to workaround dawn a validation error(dawn doesn't allow
    // a pipeline with a color attachment but without a fragment shader).
    wgpu::ShaderModule fNoopFragment;

    wgpu::BindGroupLayout fUniformBuffersBindGroupLayout;
    wgpu::BindGroupLayout fSingleTextureSamplerBindGroupLayout;

    mutable SkMutex fPendingPipelineAsyncCreationsMutex;
    mutable skia_private::THashMap<std::string, skgpu::StdSteadyClock::time_point>
            fPendingPipelineAsyncCreations SK_GUARDED_BY(fPendingPipelineAsyncCreationsMutex);
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnSharedContext_DEFINED
