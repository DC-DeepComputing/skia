/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_CommandBuffer_DEFINED
#define skgpu_CommandBuffer_DEFINED

#include "experimental/graphite/include/private/GraphiteTypesPriv.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"

namespace skgpu {
class Gpu;
class Texture;
class RenderPipeline;

struct AttachmentDesc {
    Texture* fTexture;
    LoadOp fLoadOp;
    StoreOp fStoreOp;
};

struct RenderPassDesc {
    AttachmentDesc fColorAttachment;
    Texture* fResolveTexture;

    // TODO: These will need to be to be equal with some APIs
    // Will need Caps to determine this
    AttachmentDesc fStencilAttachment;
    AttachmentDesc fDepthAttachment;

    // TODO:
    // * bounds (TBD whether exact bounds vs. granular)
    // * input attachments
};

class CommandBuffer : public SkRefCnt {
public:
    ~CommandBuffer() override {}

    bool hasWork() { return fHasWork; }

    virtual void beginRenderPass(const RenderPassDesc&) = 0;
    virtual void endRenderPass() = 0;

    virtual void setRenderPipeline(const RenderPipeline*) = 0;
    virtual void drawPrimitives(PrimitiveType, size_t vertexOffset, unsigned int vertexCount) = 0;

protected:
    CommandBuffer();

    bool fHasWork = false;

    static const int kInitialTrackedResourcesCount = 32;
    SkSTArray<kInitialTrackedResourcesCount, sk_sp<SkRefCnt>> fTrackedResources;

private:
};

} // namespace skgpu

#endif // skgpu_CommandBuffer_DEFINED
