/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlCaps_DEFINED
#define skgpu_MtlCaps_DEFINED

#include "experimental/graphite/src/Caps.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

class Caps final : public skgpu::Caps {
public:
    Caps(const id<MTLDevice>);
    ~Caps() final {}

private:
    void initGPUFamily(const id<MTLDevice>);

    void initGrCaps(const id<MTLDevice>);
    void initShaderCaps();
    void initFormatTable();

    enum class GPUFamily {
        kMac,
        kApple,
    };
    bool isMac() { return fGPUFamily == GPUFamily::kMac; }
    bool isApple() { return fGPUFamily == GPUFamily::kApple; }
    bool getGPUFamily(id<MTLDevice> device, GPUFamily* gpuFamily, int* group);
    bool getGPUFamilyFromFeatureSet(id<MTLDevice> device, GPUFamily* gpuFamily,
                                    int* group);

    GPUFamily fGPUFamily;
    int fFamilyGroup;
};

} // namespace skgpu::mtl

#endif // skgpu_MtlCaps_DEFINED
