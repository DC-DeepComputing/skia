/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Buffer_DEFINED
#define Buffer_DEFINED

#include "include/gpu/graphite/GraphiteTypes.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include <cstddef>

#define GRAPHITE_DEFAULT_BUFFER_ALIGNMENT 16

namespace skgpu {
enum class Protected : bool;
}

namespace skgpu::graphite {
class SharedContext;

class SkGraphiteBufferHelper {
public:
    Buffer* bufferPtr = nullptr;
    bool hasBuffer() const { return bufferPtr != nullptr; }
};

class Buffer : public Resource {
public:
    Buffer(size_t size);

    size_t size() const { return fSize; }

    // Public virtual methods instead of non-virtual wrapper + private onMap/onUnmap
    virtual void* map();
    virtual void asyncMap(GpuFinishedProc = nullptr, GpuFinishedContext = nullptr);
    virtual void unmap();

    bool isMapped() const { return fMapPtr; }

    // Returns true if mapped or an asyncMap was started and hasn't been completed or canceled.
    virtual bool isUnmappable() const;

    virtual const char* getResourceType() const override { return "Buffer"; }

    virtual Protected isProtected() const { return fIsProtected; }

    virtual void onMap() = 0;
    virtual void onAsyncMap(GpuFinishedProc, GpuFinishedContext);
    virtual void onUnmap() = 0;

protected:
    Buffer(const SharedContext* sharedContext,
           size_t size,
           Protected isProtected,
           std::string_view label,
           bool reusableRequiresPurgeable = false,
           bool requiresPrepareForReturnToCache = false) :
            Resource(sharedContext,
                     Ownership::kOwned,
                     size,
                     label,
                     reusableRequiresPurgeable,
                     requiresPrepareForReturnToCache),
            fSize(size),
            fIsProtected(isProtected) {}

    void* fMapPtr = nullptr;

private:
    size_t    fSize;
    Protected fIsProtected;
};

} // namespace skgpu::graphite

#endif // Buffer_DEFINED
