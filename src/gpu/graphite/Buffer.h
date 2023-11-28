/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Buffer_DEFINED
#define skgpu_graphite_Buffer_DEFINED

#include "include/gpu/GpuTypes.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceTypes.h"

namespace skgpu::graphite {

class Buffer : public Resource {
public:
    size_t size() const { return fSize; }

    // If the buffer is already mapped then pointer is returned. If an asyncMap() was started then
    // it is waited on. Otherwise, a synchronous map is performed.
    void* mapWrite();
    const void* mapRead();
    // Starts a new asynchronous map.
    void asyncMapWrite(GpuFinishedProc = nullptr, GpuFinishedContext = nullptr);
    void asyncMapRead(GpuFinishedProc = nullptr, GpuFinishedContext = nullptr);
    // If the buffer is mapped then unmaps. If an async map is pending then it is cancelled.
    void unmap();

    bool isMapped() const {
        return reinterpret_cast<intptr_t>(fMapPtr) | reinterpret_cast<intptr_t>(fConstMapPtr);
    }

    bool isMappedRead() const {
        return fConstMapPtr;
    }
    bool isMappedWrite() const {
        return fMapPtr;
    }

    // Returns true if mapped or an asyncMap was started and hasn't been completed or canceled.
    virtual bool isUnmappable() const;

    const char* getResourceType() const override { return "Buffer"; }

protected:
    Buffer(const SharedContext* sharedContext,
           size_t size,
           bool commandBufferRefsAsUsageRefs = false)
            : Resource(sharedContext,
                       Ownership::kOwned,
                       skgpu::Budgeted::kYes,
                       size,
                       /*label=*/"Buffer",
                       /*commandBufferRefsAsUsageRefs=*/commandBufferRefsAsUsageRefs)
            , fSize(size) {}

    void* fMapPtr = nullptr;
    const void* fConstMapPtr = nullptr;

private:
    virtual void onMap(bool mapForWrite) = 0;
    virtual void onAsyncMap(GpuFinishedProc, GpuFinishedContext, bool mapForWrite);
    virtual void onUnmap() = 0;

    size_t             fSize;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Buffer_DEFINED

