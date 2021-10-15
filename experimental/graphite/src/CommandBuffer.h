/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_CommandBuffer_DEFINED
#define skgpu_CommandBuffer_DEFINED

#include "include/core/SkRefCnt.h"

struct SkIRect;

namespace skgpu {
class Buffer;
class Gpu;
class Texture;

class CommandBuffer : public SkRefCnt {
public:
    ~CommandBuffer() override {}

    bool hasWork() { return fHasWork; }

    virtual void copyTextureToBuffer(sk_sp<Texture>,
                                     SkIRect srcRect,
                                     sk_sp<Buffer>,
                                     size_t bufferOffset,
                                     size_t bufferRowBytes) = 0;

protected:
    CommandBuffer();

    bool fHasWork = false;
    SkDEBUGCODE(bool fInRenderPass = false);

private:
};

} // namespace skgpu

#endif // skgpu_CommandBuffer_DEFINED
