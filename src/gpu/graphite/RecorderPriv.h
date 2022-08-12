/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RecorderPriv_DEFINED
#define skgpu_graphite_RecorderPriv_DEFINED

#include "include/gpu/graphite/Recorder.h"

#include "src/core/SkRuntimeEffectDictionary.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/graphite/DrawBufferManager.h"
#include "src/gpu/graphite/PipelineDataCache.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/UploadBufferManager.h"
#include "src/gpu/graphite/text/AtlasManager.h"
#include "src/text/gpu/StrikeCache.h"
#include "src/text/gpu/TextBlobRedrawCoordinator.h"

namespace skgpu::graphite {

struct Recorder::State {
    State(Recorder* recorder);

    void resetAfterSnap(Recording* recording);

    SkRuntimeEffectDictionary fRuntimeEffectDict;

    UniformDataCache fUniformDataCache;
    TextureDataCache fTextureDataCache;

    DrawBufferManager fDrawBufferManager;
    UploadBufferManager fUploadBufferManager;

    AtlasManager fAtlasManager;
    TokenTracker fTokenTracker;
    sktext::gpu::StrikeCache fStrikeCache;
    sktext::gpu::TextBlobRedrawCoordinator fTextBlobCache;
};


class RecorderPriv {
public:
static_assert(sizeof(Recorder::State) < 750);

    void add(sk_sp<Task>);
    void flushTrackedDevices();

    const Caps* caps() const { return fRecorder->fSharedContext->caps(); }

    ResourceProvider* resourceProvider() { return fRecorder->fResourceProvider.get(); }

    const SkRuntimeEffectDictionary* runtimeEffectDictionary() const {
        return &fRecorder->fState->fRuntimeEffectDict;
    }
    SkRuntimeEffectDictionary* runtimeEffectDictionary() {
        return &fRecorder->fState->fRuntimeEffectDict;
    }
    const SkShaderCodeDictionary* shaderCodeDictionary() const {
        return fRecorder->fSharedContext->shaderCodeDictionary();
    }
    SkShaderCodeDictionary* shaderCodeDictionary() {
        return fRecorder->fSharedContext->shaderCodeDictionary();
    }

    UniformDataCache* uniformDataCache() { return &fRecorder->fState->fUniformDataCache; }
    TextureDataCache* textureDataCache() { return &fRecorder->fState->fTextureDataCache; }
    DrawBufferManager* drawBufferManager() { return &fRecorder->fState->fDrawBufferManager; }
    UploadBufferManager* uploadBufferManager() { return &fRecorder->fState->fUploadBufferManager; }

    AtlasManager* atlasManager() { return &fRecorder->fState->fAtlasManager; }
    TokenTracker* tokenTracker() { return &fRecorder->fState->fTokenTracker; }
    sktext::gpu::StrikeCache* strikeCache() { return &fRecorder->fState->fStrikeCache; }
    sktext::gpu::TextBlobRedrawCoordinator* textBlobCache() {
        return &fRecorder->fState->fTextBlobCache;
    }

#if GR_TEST_UTILS
    bool deviceIsRegistered(Device*);
#endif

private:
    explicit RecorderPriv(Recorder* recorder) : fRecorder(recorder) {}
    RecorderPriv& operator=(const RecorderPriv&) = delete;

    // No taking addresses of this type.
    const RecorderPriv* operator&() const = delete;
    RecorderPriv* operator&() = delete;

    Recorder* fRecorder;

    friend class Recorder;  // to construct/copy this type.
};

inline RecorderPriv Recorder::priv() {
    return RecorderPriv(this);
}

inline const RecorderPriv Recorder::priv() const {  // NOLINT(readability-const-return-type)
    return RecorderPriv(const_cast<Recorder*>(this));
}

} // namespace skgpu::graphite

#endif // skgpu_graphite_RecorderPriv_DEFINED
