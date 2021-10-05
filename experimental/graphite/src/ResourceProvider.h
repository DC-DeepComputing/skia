/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ResourceProvider_DEFINED
#define skgpu_ResourceProvider_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"

namespace skgpu {

class CommandBuffer;
class Pipeline;
class PipelineDesc;
class Texture;
class TextureInfo;

class ResourceProvider {
public:
    virtual ~ResourceProvider();

    virtual std::unique_ptr<CommandBuffer> createCommandBuffer() { return nullptr; }
    Pipeline* findOrCreatePipeline(const PipelineDesc&);

    sk_sp<Texture> findOrCreateTexture(SkISize, const TextureInfo&);

protected:
    ResourceProvider();

    virtual Pipeline* onCreatePipeline() { return nullptr; }

private:
    virtual sk_sp<Texture> createTexture(SkISize, const TextureInfo&) = 0;
};

} // namespace skgpu

#endif // skgpu_ResourceProvider_DEFINED
