/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ResourceProvider_DEFINED
#define skgpu_ResourceProvider_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu {

class CommandBuffer;

class ResourceProvider {
public:
    virtual ~ResourceProvider();

    sk_sp<CommandBuffer> createCommandBuffer();

protected:
    ResourceProvider();

    virtual sk_sp<CommandBuffer> onCreateCommandBuffer() { return nullptr; }

private:
};

} // namespace skgpu

#endif // skgpu_Gpu_DEFINED
