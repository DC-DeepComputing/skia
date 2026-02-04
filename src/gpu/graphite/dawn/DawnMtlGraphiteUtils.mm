/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/mtl/MtlBackendContext.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/dawn/DawnMtlQueueManager.h"
#include "src/gpu/graphite/dawn/DawnMtlSharedContext.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {

std::unique_ptr<Context> MakeMetalDawn(const DawnBackendContext& backendContext,
                                       const ContextOptions& options) {

    id<MTLDevice> metalDevice = (__bridge id<MTLDevice>)backendContext.fDevice.GetMetalDeviceId();
    id<MTLCommandQueue> metalQueue = (__bridge id<MTLCommandQueue>)backendContext.fDevice.GetMetalQueueId();



//    SkDebugf("Device name in MakeMetalDawn: %s, pointer: %p\n", metalDevice.name.UTF8String, metalDevice);
//    SkDebugf("Queue pointer in MakeMetalDawn: %p\n", metalQueue);

    MtlBackendContext mtlBEContext;
    mtlBEContext.fDevice.retain(metalDevice);
    mtlBEContext.fQueue.retain(metalQueue);

    sk_sp<SharedContext> sharedContext = DawnMtlSharedContext::Make(mtlBEContext, backendContext,
                                                                    options);
    if (!sharedContext) {
        return nullptr;
    }

    sk_cfp<id<MTLCommandQueue>> queue =
            sk_ret_cfp((id<MTLCommandQueue>)(mtlBEContext.fQueue.get()));

    auto queueManager =
            std::make_unique<DawnMtlQueueManager>(std::move(queue), sharedContext.get());
    if (!queueManager) {
        return nullptr;
    }

    return ContextCtorAccessor::MakeContext(std::move(sharedContext),
                                            std::move(queueManager),
                                            options);
}

}  // namespace skgpu::graphite
