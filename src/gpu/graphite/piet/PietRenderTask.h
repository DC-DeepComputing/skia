/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PietRenderTask_DEFINED
#define skgpu_graphite_PietRenderTask_DEFINED

#include <vector>

#include "src/gpu/graphite/Task.h"
#include "src/gpu/piet/Scene.h"

namespace skgpu::graphite {

class TextureProxy;

class PietRenderInstance final {
public:
    PietRenderInstance(sk_sp<const skgpu::piet::Scene> scene, sk_sp<TextureProxy> targetProxy)
            : fScene(std::move(scene))
            , fTextureProxy(std::move(targetProxy)) {}

    // Make this type move-only.
    PietRenderInstance(const PietRenderInstance&) = delete;
    PietRenderInstance(PietRenderInstance&&) = default;

    bool prepareResources(ResourceProvider*);

    bool addCommands(ResourceProvider*, CommandBuffer*);

private:
    sk_sp<const skgpu::piet::Scene> fScene;
    sk_sp<TextureProxy> fTextureProxy;
};

class PietRenderTask final : public Task {
public:
    PietRenderTask(std::vector<PietRenderInstance> instances) : fInstances(std::move(instances)) {}

    bool prepareResources(ResourceProvider*) override;

    bool addCommands(ResourceProvider*, CommandBuffer*) override;

private:
    std::vector<PietRenderInstance> fInstances;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_PietRenderTask_DEFINED
