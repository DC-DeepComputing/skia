/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Buffer_DEFINED
#define skgpu_graphite_Buffer_DEFINED

#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceTypes.h"

namespace skgpu::graphite {

class Buffer : public Resource {
public:
    size_t size() const { return fSize; }

    // TODO(b/262249983): Separate into mapRead(), mapWrite() methods.
    // If the buffer is already mapped then pointer is returned. If an asyncMap() was started then
    // it is waited on. Otherwise, a synchronous map is performed.
    void* map();
    // Starts a new asynchronous map. It can be waited on by calling map() or polled by checking
    // isMapped().
    void asyncMap(GpuFinishedProc = nullptr, GpuFinishedContext = nullptr);
    // If the buffer is mapped then unmaps. If an async map is pending then it is cancelled.
    void unmap();

    bool isMapped() const { return fMapPtr; }

    // Returns true if an asyncMap was started and hasn't been completed or cancelled.
    virtual bool hasPendingAsyncMap() const = 0;

    const char* getResourceType() const override { return "Buffer"; }

protected:
    Buffer(const SharedContext* sharedContext, size_t size)
            : Resource(sharedContext,
                       Ownership::kOwned,
                       skgpu::Budgeted::kYes,
                       size,
                       /*label=*/"Buffer")
            , fSize(size) {}

    void* fMapPtr = nullptr;

    void callAsyncMapCallbacks(CallbackResult);

private:
    virtual void onMap() = 0;
    virtual void onAsyncMap() = 0;
    virtual void onUnmap() = 0;

    size_t             fSize;

    skia_private::TArray<sk_sp<RefCntedCallback>> fAsyncMapCallbacks;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Buffer_DEFINED

