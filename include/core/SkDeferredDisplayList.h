/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeferredDisplayList_DEFINED
#define SkDeferredDisplayList_DEFINED

#include "include/core/SkTypes.h"

#if defined(SK_GANESH)
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/base/SkTArray.h"

class GrDirectContext;
class GrRenderTargetProxy;
class GrRenderTask;
class SkDeferredDisplayListPriv;


/*
 * This class contains pre-processed gpu operations that can be replayed into
 * an SkSurface via SkSurface::draw(SkDeferredDisplayList*).
 */
class SkDeferredDisplayList : public SkNVRefCnt<SkDeferredDisplayList> {
public:
    SK_API ~SkDeferredDisplayList();

    SK_API const SkSurfaceCharacterization& characterization() const {
        return fCharacterization;
    }
    /**
     * Iterate through the programs required by the DDL.
     */
    class SK_API ProgramIterator {
    public:
        ProgramIterator(GrDirectContext*, SkDeferredDisplayList*);
        ~ProgramIterator();

        // This returns true if any work was done. Getting a cache hit does not count as work.
        bool compile();
        bool done() const;
        void next();

    private:
        GrDirectContext*                                 fDContext;
        const skia_private::TArray<GrRecordingContext::ProgramData>& fProgramData;
        int                                              fIndex;
    };

    // Provides access to functions that aren't part of the public API.
    SkDeferredDisplayListPriv priv();
    const SkDeferredDisplayListPriv priv() const;  // NOLINT(readability-const-return-type)

private:
    friend class GrDrawingManager; // for access to 'fRenderTasks', 'fLazyProxyData', 'fArenas'
    friend class SkDeferredDisplayListRecorder; // for access to 'fLazyProxyData'
    friend class SkDeferredDisplayListPriv;

    // This object is the source from which the lazy proxy backing the DDL will pull its backing
    // texture when the DDL is replayed. It has to be separately ref counted bc the lazy proxy
    // can outlive the DDL.
    class LazyProxyData : public SkRefCnt {
    public:
        // Upon being replayed - this field will be filled in (by the DrawingManager) with the
        // proxy backing the destination SkSurface. Note that, since there is no good place to
        // clear it, it can become a dangling pointer. Additionally, since the renderTargetProxy
        // doesn't get a ref here, the SkSurface that owns it must remain alive until the DDL
        // is flushed.
        // TODO: the drawing manager could ref the renderTargetProxy for the DDL and then add
        // a renderingTask to unref it after the DDL's ops have been executed.
        GrRenderTargetProxy* fReplayDest = nullptr;
    };

    SK_API SkDeferredDisplayList(const SkSurfaceCharacterization& characterization,
                                 sk_sp<GrRenderTargetProxy> fTargetProxy,
                                 sk_sp<LazyProxyData>);

    const skia_private::TArray<GrRecordingContext::ProgramData>& programData() const {
        return fProgramData;
    }

    const SkSurfaceCharacterization fCharacterization;

    // These are ordered such that the destructor cleans op tasks up first (which may refer back
    // to the arena and memory pool in their destructors).
    GrRecordingContext::OwnedArenas fArenas;
    skia_private::TArray<sk_sp<GrRenderTask>>   fRenderTasks;

    skia_private::TArray<GrRecordingContext::ProgramData> fProgramData;
    sk_sp<GrRenderTargetProxy>      fTargetProxy;
    sk_sp<LazyProxyData>            fLazyProxyData;
};

#endif  // defined(SK_GANESH)

#endif
