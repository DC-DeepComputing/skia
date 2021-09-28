/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/mtl/GraphiteMtlTestContext.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/mtl/MtlTypes.h"

#ifdef SK_METAL

#import <Metal/Metal.h>

namespace sk_graphite_test::mtl {

GraphiteTestContext* TestContext::Make() {
    sk_cfp<id<MTLDevice>> device;
#ifdef SK_BUILD_FOR_MAC
    sk_cfp<NSArray<id <MTLDevice>>*> availableDevices(MTLCopyAllDevices());
    // Choose the non-integrated CPU if available
    for (id<MTLDevice> dev in availableDevices.get()) {
        if (!dev.isLowPower) {
            device.retain(dev);
            break;
        }
        if (dev.isRemovable) {
            device.retain(dev);
            break;
        }
    }
    if (!device) {
        device.reset(MTLCreateSystemDefaultDevice());
    }
#else
    device.reset(MTLCreateSystemDefaultDevice());
#endif

    skgpu::mtl::BackendContext backendContext = {};

    backendContext.fDevice.retain((skgpu::mtl::Handle)device.get());
    sk_cfp<id<MTLCommandQueue>> queue([*device newCommandQueue]);
    backendContext.fQueue.retain((skgpu::mtl::Handle)queue.get());

    return new TestContext(backendContext);
}

sk_sp<skgpu::Context> TestContext::makeContext() {
    return skgpu::Context::MakeMetal(fMtl);
}

}  // namespace sk_graphite_test::mtl

#endif // SK_METAL
