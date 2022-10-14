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

enum SkColorType : int;

namespace skgpu::graphite {

class Caps;
enum class Renderable : bool;
class ResourceProvider;
class Texture;

class TextureProxy : public SkRefCnt {
public:
    TextureProxy(SkISize dimensions,
                 const TextureInfo& info,
                 SkBudgeted budgeted,
                 LateBindable lateBindable);
    TextureProxy(sk_sp<Texture>);

    TextureProxy() = delete;

    ~TextureProxy() override;

    int numSamples() const { return fInfo.numSamples(); }
    Mipmapped mipmapped() const { return Mipmapped(fInfo.numMipLevels() > 1); }

    SkISize dimensions() const { return fDimensions; }
    const TextureInfo& textureInfo() const { return fInfo; }

    bool instantiate(ResourceProvider*);
    void instantiate(sk_sp<Texture> texture);
    void uninstantiate();

    bool isInstantiated() const { return SkToBool(fTexture); }
    LateBindable lateBindable() const { return fLateBindable; }

    sk_sp<Texture> refTexture() const;
    const Texture* texture() const;

    static sk_sp<TextureProxy> Make(const Caps*,
                                    SkISize dimensions,
                                    SkColorType,
                                    Mipmapped,
                                    Protected,
                                    Renderable,
                                    SkBudgeted,
                                    LateBindable);

private:
#ifdef SK_DEBUG
    void validateTexture(const Texture*);
#endif

    SkISize fDimensions;
    TextureInfo fInfo;

    SkBudgeted fBudgeted;

    sk_sp<Texture> fTexture;

    const LateBindable fLateBindable = LateBindable::kNo;
};

// Late-instantiated texture proxy that uninstantiates itself on destruction.
class AutoUninstantiateTextureProxy {
public:
    AutoUninstantiateTextureProxy(TextureProxy*);
    ~AutoUninstantiateTextureProxy();

    void instantiate(sk_sp<Texture>);

private:
    TextureProxy* const fTextureProxy;
};

} // namepsace skgpu::graphite

#endif // skgpu_graphite_TextureProxy_DEFINED
