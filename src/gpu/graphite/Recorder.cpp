/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Recorder.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/ImageProvider.h"
#include "include/gpu/graphite/Recording.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkRuntimeEffectDictionary.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/DrawBufferManager.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/PipelineDataCache.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/TaskGraph.h"
#include "src/gpu/graphite/UploadBufferManager.h"
#include "src/gpu/graphite/text/AtlasManager.h"
#include "src/image/SkImage_Base.h"
#include "src/text/gpu/StrikeCache.h"
#include "src/text/gpu/TextBlobRedrawCoordinator.h"

namespace skgpu::graphite {

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(this->singleOwner())
#define ASSERT_SINGLE_OWNER_PRIV SKGPU_ASSERT_SINGLE_OWNER(fRecorder->singleOwner())

/*
 * The default image provider doesn't perform any conversion so, by default, Graphite won't
 * draw any non-Graphite-backed images.
 */
class DefaultImageProvider final : public ImageProvider {
public:
    static sk_sp<DefaultImageProvider> Make() {
        return sk_ref_sp(new DefaultImageProvider);
    }

    sk_sp<SkImage> findOrCreate(Recorder* recorder,
                                const SkImage* image,
                                SkImage::RequiredImageProperties) override {
        SkASSERT(!as_IB(image)->isGraphiteBacked());

        return nullptr;
    }

private:
    DefaultImageProvider() {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////

RecorderOptions::~RecorderOptions() = default;

///////////////////////////////////////////////////////////////////////////////////////////////////

static int32_t next_id() {
    static std::atomic<int32_t> nextID{1};
    int32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidGenID);
    return id;
}

Recorder::State::State(Recorder* recorder)
        : fDrawBufferManager(recorder->priv().resourceProvider(),
                             recorder->priv().caps()->requiredUniformBufferAlignment())
        , fUploadBufferManager(recorder->priv().resourceProvider())
        , fAtlasManager(recorder)
        , fTextBlobCache(next_id()) {}

void Recorder::State::resetAfterSnap(Recording* recording) {
    // These are cleared regardless of success or failure
    fRuntimeEffectDict.reset();
    fUniformDataCache.reset();
    fTextureDataCache.reset();

    if (recording) {
        fDrawBufferManager.transferToRecording(recording);
        fUploadBufferManager.transferToRecording(recording);
    } else {
        fDrawBufferManager.reset();
        fUploadBufferManager.reset(); // ?
    }

    // Intentionally don't reset the AtlasManager and other text-related
    // state objects so that we can reuse their results across Recordings.
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Recorder::Recorder(sk_sp<SharedContext> sharedContext,
                   const RecorderOptions& options)
        : fSharedContext(std::move(sharedContext))
        , fClientImageProvider(options.fImageProvider ? options.fImageProvider
                                                      : DefaultImageProvider::Make())
        , fGraph(new TaskGraph) {
    // Cannot create the ResourceProvider until we have a valid SingleOwner
    fResourceProvider = fSharedContext->makeResourceProvider(this->singleOwner());
    SkASSERT(fResourceProvider);
    // Must fill in the rest of the Recorder's state after we have its ResourceProvider
    fState = std::make_unique<State>(this);
}

Recorder::~Recorder() {
    ASSERT_SINGLE_OWNER
    for (auto& device : fTrackedDevices) {
        device->abandonRecorder();
    }

    // TODO: needed?
    fState->fStrikeCache.freeAll();
}

std::unique_ptr<TaskGraph> Recorder::snapGraph() {
    std::unique_ptr<TaskGraph> oldGraph = std::move(fGraph);
    fGraph = std::make_unique<TaskGraph>();
    return oldGraph;
}

std::unique_ptr<Recording> Recorder::snap() {
    ASSERT_SINGLE_OWNER
    for (auto& device : fTrackedDevices) {
        device->flushPendingWorkToRecorder();
    }

    std::unique_ptr<TaskGraph> graph = this->snapGraph();
    if (!graph->prepareResources(fResourceProvider.get(),
                                 this->priv().runtimeEffectDictionary())) {
        fState->resetAfterSnap(nullptr);
        return nullptr;
    }

    std::unique_ptr<Recording> recording(new Recording(std::move(graph)));
    fState->resetAfterSnap(recording.get());
    return recording;
}

void Recorder::registerDevice(Device* device) {
    ASSERT_SINGLE_OWNER
    fTrackedDevices.push_back(device);
}

void Recorder::deregisterDevice(const Device* device) {
    ASSERT_SINGLE_OWNER
    for (auto it = fTrackedDevices.begin(); it != fTrackedDevices.end(); it++) {
        if (*it == device) {
            fTrackedDevices.erase(it);
            return;
        }
    }
}

#if GR_TEST_UTILS
bool RecorderPriv::deviceIsRegistered(Device* device) {
    ASSERT_SINGLE_OWNER_PRIV
    for (auto& currentDevice : fRecorder->fTrackedDevices) {
        if (device == currentDevice) {
            return true;
        }
    }
    return false;
}
#endif

void RecorderPriv::add(sk_sp<Task> task) {
    ASSERT_SINGLE_OWNER_PRIV
    fRecorder->fGraph->add(std::move(task));
}

void RecorderPriv::flushTrackedDevices() {
    ASSERT_SINGLE_OWNER_PRIV
    for (Device* device : fRecorder->fTrackedDevices) {
        device->flushPendingWorkToRecorder();
    }
}

} // namespace skgpu::graphite
