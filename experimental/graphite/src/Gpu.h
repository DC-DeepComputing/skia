/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Gpu_DEFINED
#define skgpu_Gpu_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"

namespace skgpu {

class Caps;
class ResourceProvider;
class CommandBuffer;
class GpuWorkSubmission;

class Gpu : public SkRefCnt {
public:
    ~Gpu() override;

    /**
     * Gets the capabilities of the draw target.
     */
    const Caps* caps() const { return fCaps.get(); }
    sk_sp<const Caps> refCaps() const { return fCaps; }

    ResourceProvider* resourceProvider() const { return fResourceProvider.get(); }

    /**
     * Submit command buffer to GPU and track completion
     */
    enum class SyncToCpu : bool {
        kYes = true,
        kNo = false
    };
    bool submit(sk_sp<CommandBuffer>, SyncToCpu);

protected:
    Gpu(sk_sp<const Caps>);

    std::unique_ptr<ResourceProvider> fResourceProvider;

    SkSTArray<4, std::unique_ptr<GpuWorkSubmission>, true> fActiveGpuWork;

private:
    virtual bool onSubmit(sk_sp<CommandBuffer>, SyncToCpu) = 0;
    void checkForFinishedWork();

    sk_sp<const Caps> fCaps;
};

} // namespace skgpu

#endif // skgpu_Gpu_DEFINED
