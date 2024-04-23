/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ScratchResourceManager_DEFINED
#define skgpu_graphite_ScratchResourceManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"

namespace skgpu::graphite {

class Resource;
class ResourceProvider;
class Texture;
class TextureInfo;

class ScratchResourceManager {
public:

    // Return a scratch texture with the given size and texture info. The resource manager will
    // remember that there are 'pendingReads' (either draws or copies) that will use the returned
    // texture as a source. Once markResourceRead() has been called that many times with the
    // returned texture, the scratch texture will be considered reusable and is eligible to be
    // returned again from getScratchTexture(). Before that point, the caller has exclusive access
    // to the texture.
    //
    // It is the caller's responsibility to have determined 'pendingReads', order tasks, and call
    // markResourceRead() at the appropriate times.  Executing a task that uses the returned texture
    // after the last call to markResourceRead() can lead to undefined behavior of the texture was
    // then provided to a different caller.
    //
    // First the pool of reusable textures will be queried for an available texture. If there are
    // no available textures that match the dimensions and info, the resource provider will be
    // queried. The resource provider may return a previously created but unused texture, or will
    // create a new texture.
    sk_sp<Texture> getScratchTexture(SkISize, const TextureInfo&, int pendingReads);

    // TODO: Can we fit the DrawBufferManager's vertex buffers and scratch buffers into this model
    // and add a sk_sp<Buffer> getScratchBuffer() function?

    // Mark
    void markResourceRead(const Resource* resource);

    // FIXME do we need push/pop functions to manage the pendingReturns list? I think so
    // because we don't want to mix pending return lists and we also need a function that moves
    // the pending returns into the reuse pool.

    // -> if we don't call markResourceRead() until the actual consuming task, then there isn't a
    // need to have a stack of pending returns because anything that hits 0 can immediately become
    // available. The downside is that DrawTask can't just call markResourceRead and move on, it
    // has to somehow go forward and attach the to-be-marked resources to the RPTask.


    // okay so the pending returns list can be a single list, each time there's a push() to the
    // stack we add a null element. consume() takes all non-null resources from the back up to the
    // most recent null. pop() removes the null element, warns/asserts if there were non-null elements
    // that weren't consumed.

    // drawTask does the push/pop. Renderpasstask and copytexturetox do the consume.
    // but that means for a scratch render to texture + copy, we want a grouping task that handles
    // the push/pop stacking. Not exactly sure how that will look, but maybe it'll be related to
    // the one-time-only task wrapper? Maybe the CopyTexture tasks can have an optional source task
    // that prepares the source texture?

    // we should allow markResourceRead() to be called for non-scratch resources and it's just a
    // no-op. However, when a scratch resource hits 0, we should not remove it from our known map.
    // Just keep its count at 0, so if we get another mark() on it and the count would go negative
    // then we can instead assert.

private:
    // If there are no available resources for reuse, new or cached resources will be fetched from
    // this ResourceProvider.
    ResourceProvider* fProvider;

    // SHould this be where we store the proxy read counts?
    // Can we do this for buffers too or does that require us to use buffer proxies?


    // OR we don't do it terms of proxies at all. When we get a new resource, we supply the pending
    // read count and then whenever we hit a renderpasstask, etc. we invoke a function that says
    // we are done with the resource.
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ResourceReuseManager_DEFINED
