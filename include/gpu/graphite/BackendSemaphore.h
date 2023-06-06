/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_BackendSemaphore_DEFINED
#define skgpu_graphite_BackendSemaphore_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/GraphiteTypes.h"

#ifdef SK_METAL
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"
#endif

#ifdef SK_VULKAN
#include "include/private/gpu/vk/SkiaVulkan.h"
#endif

namespace skgpu::graphite {

class SK_API BackendSemaphore {
public:
    BackendSemaphore();
#ifdef SK_METAL
    // It is the creator's responsibility to ref the MTLEvent passed in here, via __bridge_retained.
    // Internally Graphite will wrap this BackendSemaphore and take the ref, via __bridge_transfer.
    BackendSemaphore(MtlHandle mtlEvent, uint64_t value);
#endif

#ifdef SK_VULKAN
    BackendSemaphore(VkSemaphore semaphore);
#endif

    BackendSemaphore(const BackendSemaphore&);

    ~BackendSemaphore();

    BackendSemaphore& operator=(const BackendSemaphore&);

    bool isValid() const { return fIsValid; }
    BackendApi backend() const { return fBackend; }

#ifdef SK_METAL
    MtlSemaphore getMtlSemaphore() const;
    uint64_t getMtlValue() const;
#endif

#ifdef SK_VULKAN
    VkSemaphore getVkSemaphore() const;
#endif

private:
    // TODO: For now, implement as a union until we figure out the plan for this and BackendTexture.
    union {
#ifdef SK_DAWN
        // TODO: WebGPU doesn't seem to have the notion of an Event or Semaphore
#endif
#ifdef SK_METAL
        struct {
            MTLHandle fMtlEvent;    // Expected to be an id<MTLEvent>
            uint64_t fMtlValue;
        };
#endif
#ifdef SK_VULKAN
        VkSemaphore fVkSemaphore;
#endif
        void* fEnsureUnionNonEmpty;
    };
    bool fIsValid = false;
    BackendApi fBackend;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_BackendSemaphore_DEFINED

