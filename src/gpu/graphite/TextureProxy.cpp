/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/TextureProxy.h"

#include "src/core/SkMipmap.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"

namespace skgpu::graphite {

TextureProxy::TextureProxy(SkISize dimensions,
                           const TextureInfo& info,
                           SkBudgeted budgeted,
                           LateBindable lateBindable)
        : fDimensions(dimensions), fInfo(info), fBudgeted(budgeted), fLateBindable(lateBindable) {
    // TODO: Enable this assert once we are correctly handling the creation of all graphite
    // SkImages. Right now things like makeImageSnapshot create an invalid proxy with an invalid
    // TextureInfo.
    // SkASSERT(fInfo.isValid());
}

TextureProxy::TextureProxy(sk_sp<Texture> texture)
        : fDimensions(texture->dimensions())
        , fInfo(texture->textureInfo())
        , fBudgeted(texture->budgeted())
        , fTexture(std::move(texture)) {
    SkASSERT(fInfo.isValid());
}

TextureProxy::~TextureProxy() {}

bool TextureProxy::instantiate(ResourceProvider* resourceProvider) {
    if (fTexture) {
        return true;
    }
    fTexture = resourceProvider->findOrCreateScratchTexture(fDimensions, fInfo, fBudgeted);
    if (!fTexture) {
        return false;
    }
    SkDEBUGCODE(this->validateTexture(fTexture.get()));
    return true;
}

void TextureProxy::instantiate(sk_sp<Texture> texture) {
    SkASSERT(fLateBindable == LateBindable::kYes && !this->isInstantiated());
    fTexture = std::move(texture);
    fDimensions = fTexture->dimensions();
    SkDEBUGCODE(this->validateTexture(fTexture.get()));
}

void TextureProxy::uninstantiate() {
    SkASSERT(fLateBindable == LateBindable::kYes && this->isInstantiated());
    fTexture = nullptr;
}

sk_sp<Texture> TextureProxy::refTexture() const {
    return fTexture;
}

const Texture* TextureProxy::texture() const {
    return fTexture.get();
}

sk_sp<TextureProxy> TextureProxy::Make(const Caps* caps,
                                       SkISize dimensions,
                                       SkColorType colorType,
                                       Mipmapped mipmapped,
                                       Protected isProtected,
                                       Renderable renderable,
                                       SkBudgeted budgeted,
                                       LateBindable lateBindable) {
    if (dimensions.width() < 1 || dimensions.height() < 1) {
        return nullptr;
    }

    int mipLevelCount = (mipmapped == Mipmapped::kYes)
                        ? SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1
                        : 1;

    TextureInfo textureInfo = caps->getDefaultSampledTextureInfo(colorType,
                                                                 mipLevelCount,
                                                                 isProtected,
                                                                 renderable);
    if (!textureInfo.isValid()) {
        return nullptr;
    }

    return sk_make_sp<TextureProxy>(dimensions, textureInfo, budgeted, lateBindable);
}

#ifdef SK_DEBUG
void TextureProxy::validateTexture(const Texture* texture) {
    SkASSERT(fDimensions == texture->dimensions());
    SkASSERT(fInfo == texture->textureInfo());
}
#endif

AutoUninstantiateTextureProxy::AutoUninstantiateTextureProxy(TextureProxy* textureProxy)
        : fTextureProxy(textureProxy) {}

AutoUninstantiateTextureProxy::~AutoUninstantiateTextureProxy() {
    if (fTextureProxy) {
        fTextureProxy->uninstantiate();
    }
}

void AutoUninstantiateTextureProxy::instantiate(sk_sp<Texture> texture) {
    fTextureProxy->instantiate(std::move(texture));
}

} // namespace skgpu::graphite
