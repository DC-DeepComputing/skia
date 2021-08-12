/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlSemaphore.h"

#include "src/gpu/mtl/GrMtlGpu.h"

sk_sp<GrMtlEvent> GrMtlEvent::Make(GrMtlGpu* gpu) {
    if (@available(macOS 10.14, iOS 12.0, *)) {
        sk_cfp<id<MTLEvent>> event([gpu->device() newEvent]);
        return sk_sp<GrMtlEvent>(new GrMtlEvent(std::move(event)));
    } else {
        return nullptr;
    }
}

sk_sp<GrMtlEvent> GrMtlEvent::MakeWrapped(GrMTLHandle event) {
    // The GrMtlEvent will have strong ownership at this point.
    // The GrMTLHandle will subsequently only have weak ownership.
    // TODO: Is this what we want?
    if (@available(macOS 10.14, iOS 12.0, *)) {
        sk_cfp<id<MTLEvent>> mtlEvent((id<MTLEvent>)event);
        return sk_sp<GrMtlEvent>(new GrMtlEvent(std::move(mtlEvent)));
    } else {
        return nullptr;
    }
}

GrBackendSemaphore GrMtlSemaphore::backendSemaphore() const {
    GrBackendSemaphore backendSemaphore;
    // The GrMtlSemaphore and the GrBackendSemaphore will have strong ownership at this point.
    // Whoever uses the GrBackendSemaphore will subsquently steal this ref (see MakeWrapped, above).
    if (@available(macOS 10.14, iOS 12.0, *)) {
        GrMTLHandle handle = CFRetain((GrMTLHandle)(fEvent->mtlEvent()));
        backendSemaphore.initMetal(handle, fValue);
    }
    return backendSemaphore;
}
