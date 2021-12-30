/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Resource_DEFINED
#define skgpu_Resource_DEFINED

#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/GraphicsPipelineDesc.h"
#include "experimental/graphite/src/ResourceTypes.h"
#include "include/core/SkSize.h"
#include "src/core/SkLRUCache.h"

namespace skgpu {

class Resource : SkNoncopyable {
public:
    /** Default construct, initializing the reference count to 1.
     */
    Resource() : fRefCnt(1) {}

    /** Destruct, asserting that the reference count is 1.
     */
    virtual ~Resource() {}

#ifdef SK_DEBUG
    /** Return the reference count. Use only for debugging. */
    int32_t getRefCnt() const { return fRefCnt.load(); }
#endif

    /** May return true if the caller is the only owner.
     *  Ensures that all previous owner's actions are complete.
     */
    bool unique() const {
        // The acquire barrier is only really needed if we return true.  It
        // prevents code conditioned on the result of unique() from running
        // until previous owners are all totally done calling unref().
        return 1 == fRefCnt.load(std::memory_order_acquire);
    }

    /** Increment the reference count.
        Must be balanced by a call to unref()
     */
    void ref() const {
        // No barrier required.
        SkDEBUGCODE(int newRefCount = )fRefCnt.fetch_add(+1, std::memory_order_relaxed);
        SkASSERT(newRefCount >= 1);
    }

    /** Decrement the reference count. If the reference count is 1 before the
        decrement, then delete the object. Note that if this is the case, then
        the object needs to have been allocated via new, and not on the stack.
        Any GPU data associated with this resource will be freed before it's deleted.
     */
    void unref() const {
        int oldRefCount = fRefCnt.fetch_add(-1, std::memory_order_acq_rel);
        SkASSERT(newRefCount >= 0);
        if (oldRefCount == 1) {
            // Like unique(), the acquire is only needed on success, to make sure
            // code in internal_dispose() doesn't happen before the decrement.
            this->internal_dispose();
        }
    }

private:
    /** Must be implemented by any subclasses.
     *  Deletes any GPU data associated with this resource
     */
    virtual void freeGPUData() const = 0;

    /**
     *  Called when the ref count goes to 0. Will free GPU resources.
     */
    void internal_dispose() const {
        this->freeGPUData();

#ifdef SK_DEBUG
        SkASSERT(0 == this->getRefCnt());
        fRefCnt.store(1);
#endif
        delete this;
    }

    mutable std::atomic<int32_t> fRefCnt;
};

} // namespace skgpu

#endif // skgpu_Resource_DEFINED

