/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PietRenderer_DEFINED
#define skgpu_graphite_PietRenderer_DEFINED

#include <memory>

namespace skgpu::piet {
class Scene;
}  // namespace skgpu::piet

namespace skgpu::graphite {

class CommandBuffer;
class Gpu;
class Texture;

class PietRenderer {
public:
#ifdef SK_METAL
    static std::unique_ptr<PietRenderer> MakeMetal(const Gpu* gpu);
#endif

    virtual ~PietRenderer() = default;

    virtual void encodeSceneRender(const skgpu::piet::Scene& scene,
                                   const Texture* target,
                                   CommandBuffer*) const = 0;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_PietRenderer_DEFINED
