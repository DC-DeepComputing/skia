/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_TextureProxy_DEFINED
#define skgpu_graphite_TextureProxy_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkEnumBitMask.h"

// include src/gpu/graphite/ResourceTypes.h ?

#include <functional>

enum SkColorType : int;

namespace skgpu::graphite {

class Caps;
class Recorder;
class ResourceProvider;
class ScratchResourceManager;
class Texture;
enum class ScratchUsageFlags : uint32_t;

class TextureProxy : public SkRefCnt {
public:
    TextureProxy() = delete;

    ~TextureProxy() override;

    int numSamples() const { return fInfo.numSamples(); }
    Mipmapped mipmapped() const { return fInfo.mipmapped(); }

    SkISize dimensions() const;
    const TextureInfo& textureInfo() const { return fInfo; }

    const char* label() const { return fLabel.c_str(); }
    SkEnumBitMask<ScratchUsageFlags> scratchUsageMask() const { return fScratchUsageMask; }

    bool isLazy() const;
    bool isFullyLazy() const;
    bool isVolatile() const;

    Protected isProtected() const { return fInfo.isProtected(); }

    size_t uninstantiatedGpuMemorySize() const;

    bool instantiate(ResourceProvider*);
    /*
     * We currently only instantiate lazy proxies at insertion-time. Snap-time 'instantiate'
     * calls should be wrapped in 'InstantiateIfNotLazy'.
     *
     * Unlike Ganesh, in Graphite we do not update the proxy's dimensions with the instantiating
     * texture's dimensions. This means that when a fully-lazy proxy is instantiated and
     * deinstantiated, it goes back to being fully-lazy and without dimensions, and can be
     * re-instantiated with a new texture with different dimensions than the first.
     */
    bool lazyInstantiate(ResourceProvider*);
    /*
     * For Lazy proxies this will return true. Otherwise, it will return the result of
     * calling instantiate on the texture proxy.
     *
     * DEPRECATED: Eventually all un-instantiated non-lazy proxies should use the
     *             ScratchResourceManager function instead of the ResourceProvider directly.
     */
    static bool InstantiateIfNotLazy(ResourceProvider*, TextureProxy*);

    /*
     * Instantiate any scratch proxy (not already instantiated and not lazy) by using a texture
     * from the ScratchResourceManager. When possible, this will be a texture that has been returned
     * for reuse by a prior task. Lazy proxies and already instantiated proxies will return true.
     *
     * False is returned if instantiation fails.
     */
    static bool InstantiateIfNotLazy(ScratchResourceManager*, TextureProxy*);

    bool isInstantiated() const { return SkToBool(fTexture); }
    void deinstantiate();
    sk_sp<Texture> refTexture() const;
    const Texture* texture() const;
    Texture* texture() { return fTexture.get(); }

    // TODO update comment
    // Make() and MakeWithCustomLabel() will immediately instantiate non-budgeted proxies. For
    // Graphite internal usage, we
    static sk_sp<TextureProxy> Make(const Caps*,
                                    ResourceProvider*,
                                    SkISize dimensions,
                                    const TextureInfo&,
                                    std::string_view label,
                                    skgpu::Budgeted);

    using LazyInstantiateCallback = std::function<sk_sp<Texture> (ResourceProvider*)>;

    static sk_sp<TextureProxy> MakeLazy(const Caps*,
                                        SkISize dimensions,
                                        const TextureInfo&,
                                        skgpu::Budgeted,
                                        Volatile,
                                        LazyInstantiateCallback&&);
    static sk_sp<TextureProxy> MakeFullyLazy(const TextureInfo&,
                                             skgpu::Budgeted,
                                             Volatile,
                                             LazyInstantiateCallback&&);

    static sk_sp<TextureProxy> Wrap(sk_sp<Texture>);

private:
    TextureProxy(SkISize dimensions,
                 const TextureInfo& info,
                 ScratchUsageFlags usage,
                 skgpu::Budgeted budgeted);
    TextureProxy(SkISize dimensions,
                 const TextureInfo&,
                 skgpu::Budgeted,
                 Volatile,
                 LazyInstantiateCallback&&);
    TextureProxy(sk_sp<Texture>);

#ifdef SK_DEBUG
    void validateTexture(const Texture*);
#endif

    // In the following, 'fVolatile' and 'fLazyInstantiateCallback' can be accessed from
    // multiple threads so need to remain immutable.
    SkISize fDimensions;
    const TextureInfo fInfo;

    // String used to describe the current use of this TextureProxy. It will be set on its
    // Texture object when the proxy gets instantiated. Should be empty for proxies instantiated
    // as scratch textures.
    // TODO: Is this how we get custom scratch labels from the client?
    std::string fLabel;

    // If the proxy represents / is instantiated as a scratch Texture, its label will not be stored
    // as a string but rather as a bitmask of potential usages. These usages map to string labels
    // and are generated upon recording insertion if necessary (i.e. the usages have changed so its
    // label must actually be updated). Should be 0 for texture proxies instantiated as non-scratch.
    // TODO: Fix comment.
    SkEnumBitMask<ScratchUsageFlags> fScratchUsageMask = 0;
    // Track prior scratch usage flags so that we can compare a proxy's current usages against its
    // past ones. This allows us to determine whether or not we actually need to generate a string
    // label and update the resource's label or not.
    SkEnumBitMask<ScratchUsageFlags> fPriorScratchUsageMask = 0;

    skgpu::Budgeted fBudgeted;
    const Volatile fVolatile;

    sk_sp<Texture> fTexture;

    const LazyInstantiateCallback fLazyInstantiateCallback;
};

// Volatile texture proxy that deinstantiates itself on destruction.
class AutoDeinstantiateTextureProxy {
public:
    AutoDeinstantiateTextureProxy(TextureProxy* textureProxy) : fTextureProxy(textureProxy) {}

    ~AutoDeinstantiateTextureProxy() {
        if (fTextureProxy) {
            fTextureProxy->deinstantiate();
        }
    }

private:
    TextureProxy* const fTextureProxy;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_TextureProxy_DEFINED
