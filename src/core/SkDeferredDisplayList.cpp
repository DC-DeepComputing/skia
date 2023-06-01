/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkDeferredDisplayList.h"

#if defined(SK_GANESH)
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrRenderTask.h"

#include <utility>

SkDeferredDisplayList::SkDeferredDisplayList(const SkSurfaceCharacterization& characterization,
                                             sk_sp<GrRenderTargetProxy> targetProxy,
                                             sk_sp<LazyProxyData> lazyProxyData)
        : fCharacterization(characterization)
        , fArenas(true)
        , fTargetProxy(std::move(targetProxy))
        , fLazyProxyData(std::move(lazyProxyData))
{
    SkASSERT(fTargetProxy->isDDLTarget());
}

SkDeferredDisplayList::~SkDeferredDisplayList() {
#if defined(SK_DEBUG)
    for (auto& renderTask : fRenderTasks) {
        SkASSERT(renderTask->unique());
    }
#endif
}

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

#endif // defined(SK_GANESH)
