/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Context_DEFINED
#define skgpu_Context_DEFINED
#include "include/core/SkRefCnt.h"

struct GrD3DBackendContext;
struct GrGLInterface;
struct GrMtlBackendContext;
struct GrVkBackendContext;

namespace skgpu {

class Gpu;

class Context final : public SkRefCnt {
public:
    ~Context() override;

#ifdef SK_METAL
    static sk_sp<Context> MakeMetal(const GrMtlBackendContext&);
#endif
//#ifdef SK_VULKAN
    static sk_sp<Context> MakeVulkan(const GrVkBackendContext&);
//#endif

protected:
    Context(sk_sp<Gpu>);

private:
    sk_sp<Gpu> fGpu;
};

} // namespace skgpu

#endif // skgpu_Context_DEFINED

