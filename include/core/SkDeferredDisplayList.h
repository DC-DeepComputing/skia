/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeferredDisplayList_DEFINED
#define SkDeferredDisplayList_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "include/core/SkTypes.h"

class SkDeferredDisplayListPriv;

<<<<<<< HEAD   (86f93b Enforce IWYU on SkPicture-related files)
#if SK_SUPPORT_GPU
=======
#if defined(SK_GANESH)
>>>>>>> CHANGE (0f7b44 Use SK_GANESH and SK_GRAPHITE everywhere)
#include "include/gpu/GrRecordingContext.h"
#include "include/private/base/SkTArray.h"
#include <map>
class GrRenderTask;
class GrRenderTargetProxy;
#else
using GrRenderTargetProxy = SkRefCnt;
#endif

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

<<<<<<< HEAD   (86f93b Enforce IWYU on SkPicture-related files)
#if SK_SUPPORT_GPU
=======
#if defined(SK_GANESH)
>>>>>>> CHANGE (0f7b44 Use SK_GANESH and SK_GRAPHITE everywhere)
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
        const SkTArray<GrRecordingContext::ProgramData>& fProgramData;
        int                                              fIndex;
    };
#endif

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
<<<<<<< HEAD   (86f93b Enforce IWYU on SkPicture-related files)
#if SK_SUPPORT_GPU
=======
#if defined(SK_GANESH)
>>>>>>> CHANGE (0f7b44 Use SK_GANESH and SK_GRAPHITE everywhere)
    public:
        // Upon being replayed - this field will be filled in (by the DrawingManager) with the
        // proxy backing the destination SkSurface. Note that, since there is no good place to
        // clear it, it can become a dangling pointer. Additionally, since the renderTargetProxy
        // doesn't get a ref here, the SkSurface that owns it must remain alive until the DDL
        // is flushed.
        // TODO: the drawing manager could ref the renderTargetProxy for the DDL and then add
        // a renderingTask to unref it after the DDL's ops have been executed.
        GrRenderTargetProxy* fReplayDest = nullptr;
#endif
    };

    SK_API SkDeferredDisplayList(const SkSurfaceCharacterization& characterization,
                                 sk_sp<GrRenderTargetProxy> fTargetProxy,
                                 sk_sp<LazyProxyData>);

<<<<<<< HEAD   (86f93b Enforce IWYU on SkPicture-related files)
#if SK_SUPPORT_GPU
=======
#if defined(SK_GANESH)
>>>>>>> CHANGE (0f7b44 Use SK_GANESH and SK_GRAPHITE everywhere)
    const SkTArray<GrRecordingContext::ProgramData>& programData() const {
        return fProgramData;
    }
#endif

    const SkSurfaceCharacterization fCharacterization;

<<<<<<< HEAD   (86f93b Enforce IWYU on SkPicture-related files)
#if SK_SUPPORT_GPU
=======
#if defined(SK_GANESH)
>>>>>>> CHANGE (0f7b44 Use SK_GANESH and SK_GRAPHITE everywhere)
    // These are ordered such that the destructor cleans op tasks up first (which may refer back
    // to the arena and memory pool in their destructors).
    GrRecordingContext::OwnedArenas fArenas;
    SkTArray<sk_sp<GrRenderTask>>   fRenderTasks;

    SkTArray<GrRecordingContext::ProgramData> fProgramData;
    sk_sp<GrRenderTargetProxy>      fTargetProxy;
    sk_sp<LazyProxyData>            fLazyProxyData;
#endif
};

#endif
