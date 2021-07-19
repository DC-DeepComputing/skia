/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkFramebuffer_DEFINED
#define GrVkFramebuffer_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/gpu/mtl/GrMtlTypes.h"
#include "src/gpu/GrManagedResource.h"

class GrMtlAttachment;

class GrMtlFramebuffer : public GrManagedResource {
public:
    static sk_sp<const GrMtlFramebuffer> Make(GrMtlAttachment* colorAttachment,
                                              GrMtlAttachment* resolveAttachment,
                                              GrMtlAttachment* stencilAttachment);

#ifdef SK_TRACE_MANAGED_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrMtlFramebuffer: (%d refs)\n", this->getRefCnt());
    }
#endif

    GrMtlAttachment* colorAttachment() { return fColorAttachment.get(); }
    GrMtlAttachment* resolveAttachment() { return fResolveAttachment.get(); }
    GrMtlAttachment* stencilAttachment() { return fStencilAttachment.get(); }

private:
    GrMtlFramebuffer(sk_sp<GrMtlAttachment> colorAttachment,
                     sk_sp<GrMtlAttachment> resolveAttachment,
                     sk_sp<GrMtlAttachment> stencilAttachment);

    ~GrMtlFramebuffer() override {}

    void freeGPUData() const override {}

    sk_sp<GrMtlAttachment> fColorAttachment;
    sk_sp<GrMtlAttachment> fResolveAttachment;
    sk_sp<GrMtlAttachment> fStencilAttachment;
};

#endif
