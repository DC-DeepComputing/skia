/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUnifiedContext_SharedContext_DEFINED
#define SkUnifiedContext_SharedContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/private/SingleOwner.h"
#include "src/capture/SkCaptureManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/PipelineManager.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/ThreadSafeResourceProvider.h"

#include <memory>

namespace SkUnifiedContext {

class SharedContext : public SkRefCnt {
public:
    ~SharedContext() override {}

    const Caps* caps() const { return fCaps.get(); }

    ResourceProvider* resourceProvider() const { return fResourceProvider.get(); }

    GlobalCache* globalCache() const { return fGlobalCache.get(); }

    ShaderCodeDictionary* shaderCodeDictionary() const { return fShaderCodeDictionary.get(); }

    PipelineManager* pipelineManager() const { return fPipelineManager.get(); }

    SkCaptureManager* captureManager() const { return fCaptureManager.get(); }

protected:
    SharedContext(std::unique_ptr<const Caps> caps,
                  std::unique_ptr<ResourceProvider> resourceProvider,
                  std::unique_ptr<GlobalCache> globalCache,
                  std::unique_ptr<ShaderCodeDictionary> shaderCodeDictionary,
                  std::unique_ptr<PipelineManager> pipelineManager,
                  std::unique_ptr<SkCaptureManager> captureManager)
            : fCaps(std::move(caps))
            , fResourceProvider(std::move(resourceProvider))
            , fGlobalCache(std::move(globalCache))
            , fShaderCodeDictionary(std::move(shaderCodeDictionary))
            , fPipelineManager(std::move(pipelineManager))
            , fCaptureManager(std::move(captureManager)) {}

private:
    std::unique_ptr<const Caps> fCaps;
    std::unique_ptr<ResourceProvider> fResourceProvider;
    std::unique_ptr<GlobalCache> fGlobalCache;
    std::unique_ptr<ShaderCodeDictionary> fShaderCodeDictionary;
    std::unique_ptr<PipelineManager> fPipelineManager;
    std::unique_ptr<SkCaptureManager> fCaptureManager;
};

}  // namespace SkUnifiedContext

#endif  // SkUnifiedContext_SharedContext_DEFINED
