/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlAttachment_DEFINED
#define GrMtlAttachment_DEFINED

#include "include/gpu/mtl/GrMtlTypes.h"
#include "src/gpu/GrAttachment.h"
#include "src/gpu/GrManagedResource.h"

#import <Metal/Metal.h>

class GrMtlGpu;

class GrMtlAttachment : public GrAttachment {
private:
    class Resource;

public:
    static sk_sp<GrMtlAttachment> MakeStencil(GrMtlGpu* gpu,
                                              SkISize dimensions,
                                              int sampleCnt,
                                              MTLPixelFormat format);

    static sk_sp<GrMtlAttachment> MakeMSAA(GrMtlGpu* gpu,
                                           SkISize dimensions,
                                           int sampleCnt,
                                           MTLPixelFormat format);

    static sk_sp<GrMtlAttachment> MakeTexture(GrMtlGpu* gpu,
                                              SkISize dimensions,
                                              MTLPixelFormat format,
                                              uint32_t mipLevels,
                                              GrRenderable renderable,
                                              int numSamples,
                                              SkBudgeted budgeted);

    static sk_sp<GrMtlAttachment> MakeWrapped(GrMtlGpu* gpu,
                                              SkISize dimensions,
                                              id<MTLTexture>,
                                              UsageFlags attachmentUsages,
                                              GrWrapCacheable);

    ~GrMtlAttachment() override;

    GrBackendFormat backendFormat() const override {
        return GrBackendFormat::MakeMtl(this->mtlTexture().pixelFormat);
    }

    MTLPixelFormat mtlFormat() const { return this->mtlTexture().pixelFormat; }

    id<MTLTexture> mtlTexture() const { return fResource->mtlTexture(); }

    const Resource* resource() const {
        SkASSERT(fResource);
        return fResource.get();
    }

protected:
    void onRelease() override;
    void onAbandon() override;

private:
    static sk_sp<GrMtlAttachment> Make(GrMtlGpu* gpu,
                                       SkISize dimensions,
                                       UsageFlags attachmentUsages,
                                       int sampleCnt,
                                       MTLPixelFormat format,
                                       uint32_t mipLevels,
                                       int mtlTextureUsage,
                                       int mtlStorageMode,
                                       SkBudgeted);

    GrMtlAttachment(GrMtlGpu* gpu,
                    SkISize dimensions,
                    UsageFlags supportedUsages,
                    id<MTLTexture> texture,
                    SkBudgeted);

    GrMtlAttachment(GrMtlGpu* gpu,
                    SkISize dimensions,
                    UsageFlags supportedUsages,
                    id<MTLTexture> texture,
                    GrWrapCacheable);

    class Resource : public GrTextureResource {
    public:
        Resource(id<MTLTexture> texture)
            : fTexture(texture) {}

        ~Resource() override {}

        id<MTLTexture> mtlTexture() { return fTexture; }

#ifdef SK_TRACE_MANAGED_RESOURCES
        /** output a human-readable dump of this resource's information
         */
        void dumpInfo() const override {
            SkDebugf("GrMtlTexture: %p (%ld refs)\n", fTexture,
                     CFGetRetainCount((CFTypeRef)fTexture));
        }
#endif

#ifdef SK_DEBUG
        const GrManagedResource* asVkImageResource() const override { return this; }
#endif

    private:
        void freeGPUData() const override {
            fTexture = nil;
        }

        mutable id<MTLTexture> fTexture;

        using INHERITED = GrTextureResource;
    };

    sk_sp<Resource> fResource;

    GrMtlGpu* getMtlGpu() const;

};

#endif
