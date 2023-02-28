/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkDeferredDisplayList.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "src/base/SkArenaAlloc.h"

#include <utility>

class SkSurfaceCharacterization;

<<<<<<< HEAD   (86f93b Enforce IWYU on SkPicture-related files)
#if SK_SUPPORT_GPU
=======
#if defined(SK_GANESH)
>>>>>>> CHANGE (0f7b44 Use SK_GANESH and SK_GRAPHITE everywhere)
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrRenderTask.h"
#endif

SkDeferredDisplayList::SkDeferredDisplayList(const SkSurfaceCharacterization& characterization,
                                             sk_sp<GrRenderTargetProxy> targetProxy,
                                             sk_sp<LazyProxyData> lazyProxyData)
        : fCharacterization(characterization)
<<<<<<< HEAD   (86f93b Enforce IWYU on SkPicture-related files)
#if SK_SUPPORT_GPU
=======
#if defined(SK_GANESH)
>>>>>>> CHANGE (0f7b44 Use SK_GANESH and SK_GRAPHITE everywhere)
        , fArenas(true)
        , fTargetProxy(std::move(targetProxy))
        , fLazyProxyData(std::move(lazyProxyData))
#endif
{
<<<<<<< HEAD   (86f93b Enforce IWYU on SkPicture-related files)
#if SK_SUPPORT_GPU
=======
#if defined(SK_GANESH)
>>>>>>> CHANGE (0f7b44 Use SK_GANESH and SK_GRAPHITE everywhere)
    SkASSERT(fTargetProxy->isDDLTarget());
#endif
}

SkDeferredDisplayList::~SkDeferredDisplayList() {
<<<<<<< HEAD   (86f93b Enforce IWYU on SkPicture-related files)
#if SK_SUPPORT_GPU && defined(SK_DEBUG)
=======
#if defined(SK_GANESH) && defined(SK_DEBUG)
>>>>>>> CHANGE (0f7b44 Use SK_GANESH and SK_GRAPHITE everywhere)
    for (auto& renderTask : fRenderTasks) {
        SkASSERT(renderTask->unique());
    }
#endif
}

//-------------------------------------------------------------------------------------------------
<<<<<<< HEAD   (86f93b Enforce IWYU on SkPicture-related files)
#if SK_SUPPORT_GPU
=======
#if defined(SK_GANESH)
>>>>>>> CHANGE (0f7b44 Use SK_GANESH and SK_GRAPHITE everywhere)

SkDeferredDisplayList::ProgramIterator::ProgramIterator(GrDirectContext* dContext,
                                                        SkDeferredDisplayList* ddl)
    : fDContext(dContext)
    , fProgramData(ddl->programData())
    , fIndex(0) {
}

SkDeferredDisplayList::ProgramIterator::~ProgramIterator() {}

bool SkDeferredDisplayList::ProgramIterator::compile() {
    if (!fDContext || fIndex < 0 || fIndex >= (int) fProgramData.size()) {
        return false;
    }

    return fDContext->priv().compile(fProgramData[fIndex].desc(), fProgramData[fIndex].info());
}

bool SkDeferredDisplayList::ProgramIterator::done() const {
    return fIndex >= (int) fProgramData.size();
}

void SkDeferredDisplayList::ProgramIterator::next() {
    ++fIndex;
}

#endif
