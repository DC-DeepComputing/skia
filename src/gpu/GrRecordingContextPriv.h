/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRecordingContextPriv_DEFINED
#define GrRecordingContextPriv_DEFINED

#include "include/core/SkPaint.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/BaseDevice.h"
#include "src/gpu/GrImageContextPriv.h"
#include "src/gpu/text/GrSDFTControl.h"

class SkDeferredDisplayList;

/** Class that exposes methods on GrRecordingContext that are only intended for use internal to
    Skia. This class is purely a privileged window into GrRecordingContext. It should never have
    additional data members or virtual methods. */
class GrRecordingContextPriv : public GrImageContextPriv {
public:
    GrRecordingContext* context() { return static_cast<GrRecordingContext*>(fContext); }
    const GrRecordingContext* context() const {
        return static_cast<const GrRecordingContext*>(fContext);
    }

    GrProxyProvider* proxyProvider() { return this->context()->proxyProvider(); }
    const GrProxyProvider* proxyProvider() const { return this->context()->proxyProvider(); }

    GrDrawingManager* drawingManager() { return this->context()->drawingManager(); }

    SkArenaAlloc* recordTimeAllocator() { return this->context()->arenas().recordTimeAllocator(); }
    GrSubRunAllocator* recordTimeSubRunAllocator() {
        return this->context()->arenas().recordTimeSubRunAllocator();
    }
    GrRecordingContext::Arenas arenas() { return this->context()->arenas(); }

    GrRecordingContext::OwnedArenas&& detachArenas() { return this->context()->detachArenas(); }

    void recordProgramInfo(const GrProgramInfo* programInfo) {
        this->context()->recordProgramInfo(programInfo);
    }

    void detachProgramData(SkTArray<GrRecordingContext::ProgramData>* dst) {
        this->context()->detachProgramData(dst);
    }

    GrTextBlobCache* getTextBlobCache() { return this->context()->getTextBlobCache(); }

    GrThreadSafeCache* threadSafeCache() { return this->context()->threadSafeCache(); }

    void moveRenderTasksToDDL(SkDeferredDisplayList*);

    /**
     * Registers an object for flush-related callbacks. (See GrOnFlushCallbackObject.)
     *
     * NOTE: the drawing manager tracks this object as a raw pointer; it is up to the caller to
     * ensure its lifetime is tied to that of the context.
     */
    void addOnFlushCallbackObject(GrOnFlushCallbackObject*);

    GrAuditTrail* auditTrail() { return this->context()->fAuditTrail.get(); }

#if GR_TEST_UTILS
    // Used by tests that intentionally exercise codepaths that print warning messages, in order to
    // not confuse users with output that looks like a testing failure.
    class AutoSuppressWarningMessages {
    public:
        AutoSuppressWarningMessages(GrRecordingContext* context) : fContext(context) {
            ++fContext->fSuppressWarningMessages;
        }
        ~AutoSuppressWarningMessages() {
            --fContext->fSuppressWarningMessages;
        }
    private:
        GrRecordingContext* fContext;
    };
    void incrSuppressWarningMessages() { ++this->context()->fSuppressWarningMessages; }
    void decrSuppressWarningMessages() { --this->context()->fSuppressWarningMessages; }
#endif

    void printWarningMessage(const char* msg) const {
#if GR_TEST_UTILS
        if (this->context()->fSuppressWarningMessages > 0) {
            return;
        }
#endif
        SkDebugf("%s", msg);
    }

    GrRecordingContext::Stats* stats() {
        return &this->context()->fStats;
    }

#if GR_GPU_STATS && GR_TEST_UTILS
    using DMSAAStats = GrRecordingContext::DMSAAStats;
    DMSAAStats& dmsaaStats() { return this->context()->fDMSAAStats; }
#endif

    GrSDFTControl getSDFTControl(bool useSDFTForSmallText) const;

    /**
     * Create a GrRecordingContext without a resource cache
     */
    static sk_sp<GrRecordingContext> MakeDDL(sk_sp<GrContextThreadSafeProxy>);

    sk_sp<skgpu::BaseDevice> createDevice(GrColorType,
                                          sk_sp<GrSurfaceProxy>,
                                          sk_sp<SkColorSpace>,
                                          GrSurfaceOrigin,
                                          const SkSurfaceProps&,
                                          skgpu::BaseDevice::InitContents);
    sk_sp<skgpu::BaseDevice> createDevice(SkBudgeted,
                                          const SkImageInfo&,
                                          SkBackingFit,
                                          int sampleCount,
                                          GrMipmapped,
                                          GrProtected,
                                          GrSurfaceOrigin,
                                          const SkSurfaceProps&,
                                          skgpu::BaseDevice::InitContents);

protected:
    explicit GrRecordingContextPriv(GrRecordingContext* rContext) : GrImageContextPriv(rContext) {}

private:
    GrRecordingContextPriv(const GrRecordingContextPriv&) = delete;
    GrRecordingContextPriv& operator=(const GrRecordingContextPriv&) = delete;

    // No taking addresses of this type.
    const GrRecordingContextPriv* operator&() const;
    GrRecordingContextPriv* operator&();

    friend class GrRecordingContext; // to construct/copy this type.

    using INHERITED = GrImageContextPriv;
};

inline GrRecordingContextPriv GrRecordingContext::priv() { return GrRecordingContextPriv(this); }

inline const GrRecordingContextPriv GrRecordingContext::priv () const {  // NOLINT(readability-const-return-type)
    return GrRecordingContextPriv(const_cast<GrRecordingContext*>(this));
}

#endif
