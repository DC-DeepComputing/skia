/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Context_DEFINED
#define skgpu_graphite_Context_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/private/SingleOwner.h"

#include <memory>

class SkBlenderID;
class SkCombinationBuilder;
class SkRuntimeEffect;

namespace skgpu::graphite {

class BackendTexture;
class Context;
class ContextPriv;
class GlobalCache;
class Gpu;
struct MtlBackendContext;
class QueueManager;
class Recorder;
class Recording;
class ResourceProvider;
class TextureInfo;

/*
 * This class provides a centralized location for clients to perform any caching of images
 * they desire. Whenever a new Graphite-backed SkImage is about to be created from an already
 * existing SkImage, the client image cache will be given the opportunity to return a cached version
 * (via "find") and/or cache the result (via "cache").
 *
 * Note: by default, Graphite will not perform any caching of images
 *
 * Threading concerns:
 *   The same ClientImageCache will be given to each Recorder as it is created. It is up to the
 *   client to handle any required thread synchronization. This is not just limited to acces
 *   to what ever map a derived class may have but also extends to ensuring that an image created
 *   on one Recorder has had its creation work submitted before it is used by any work submitted
 *   by another Recorder.
 *
 * TODO(b/240996632): add documentation re shutdown order.
 * TODO(b/240997067): add unit tests
 */
class ClientImageCache : public SkRefCnt {
public:
    // If the cache already has a Graphite-backed image that has the same contents as 'prior'
    // and meets the 'mipmapped' requirements, then it can be returned to short circuit the
    // Graphite-backed texture creation process.
    virtual sk_sp<SkImage> find(Recorder* recorder,
                                const SkImage* prior,
                                Mipmapped mipmapped) = 0;
    // A Graphite-backed version of 'prior' has been created that fulfills the 'mipmapped'
    // requirements in 'graphiteBacked'. This entry point allows the client to cache the result.
    virtual void cache(Recorder* recorder,
                       const SkImage* prior,
                       Mipmapped mipmapped,
                       sk_sp<SkImage> graphiteBacked) = 0;
};


class Context final {
public:
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;

    ~Context();

#ifdef SK_METAL
    static std::unique_ptr<Context> MakeMetal(const MtlBackendContext&,
                                              const ContextOptions&,
                                              sk_sp<ClientImageCache> = nullptr);
#endif

    BackendApi backend() const { return fBackend; }

    std::unique_ptr<Recorder> makeRecorder();

    void insertRecording(const InsertRecordingInfo&);
    void submit(SyncToCpu = SyncToCpu::kNo);

    /**
     * Checks whether any asynchronous work is complete and if so calls related callbacks.
     */
    void checkAsyncWorkCompletion();

#ifdef SK_ENABLE_PRECOMPILE
    // TODO: add "SkShaderID addUserDefinedShader(sk_sp<SkRuntimeEffect>)" here
    // TODO: add "SkColorFilterID addUserDefinedColorFilter(sk_sp<SkRuntimeEffect>)" here
    SkBlenderID addUserDefinedBlender(sk_sp<SkRuntimeEffect>);

    void precompile(SkCombinationBuilder*);
#endif

    /**
     * Creates a new backend gpu texture matching the dimensinos and TextureInfo. If an invalid
     * TextureInfo or a TextureInfo Skia can't support is passed in, this will return an invalid
     * BackendTexture. Thus the client should check isValid on the returned BackendTexture to know
     * if it succeeded or not.
     *
     * If this does return a valid BackendTexture, the caller is required to use
     * Context::deleteBackendTexture to delete that texture.
     */
    BackendTexture createBackendTexture(SkISize dimensions, const TextureInfo&);

    /**
     * Called to delete the passed in BackendTexture. This should only be called if the
     * BackendTexture was created by calling Context::createBackendTexture. If the BackendTexture is
     * not valid or does not match the BackendApi of the Context then nothing happens.
     *
     * Otherwise this will delete/release the backend object that is wrapped in the BackendTexture.
     * The BackendTexture will be reset to an invalid state and should not be used again.
     */
    void deleteBackendTexture(BackendTexture&);

    // Provides access to functions that aren't part of the public API.
    ContextPriv priv();
    const ContextPriv priv() const;  // NOLINT(readability-const-return-type)

protected:
    Context(sk_sp<Gpu>, std::unique_ptr<QueueManager>, sk_sp<ClientImageCache>, BackendApi);

private:
    friend class ContextPriv;

    SingleOwner* singleOwner() const { return &fSingleOwner; }

    sk_sp<Gpu> fGpu;
    std::unique_ptr<ResourceProvider> fResourceProvider;
    std::unique_ptr<QueueManager> fQueueManager;
    sk_sp<GlobalCache> fGlobalCache;
    sk_sp<ClientImageCache> fClientImageCache;
    BackendApi fBackend;

    // In debug builds we guard against improper thread handling. This guard is passed to the
    // ResourceCache for the Context.
    mutable SingleOwner fSingleOwner;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Context_DEFINED
