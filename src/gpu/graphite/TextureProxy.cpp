/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/TextureProxy.h"

#include "include/core/SkRect.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"

#include "src/gpu/graphite/CopyTask.h"

namespace skgpu::graphite {

TextureProxy::TextureProxy(SkISize dimensions, const TextureInfo& info, SkBudgeted budgeted)
        : fDimensions(dimensions), fInfo(info), fBudgeted(budgeted) {
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

sk_sp<Texture> TextureProxy::refTexture() const {
    return fTexture;
}

const Texture* TextureProxy::texture() const {
    return fTexture.get();
}

sk_sp<TextureProxy> TextureProxy::Copy(Recorder* recorder,
                                       sk_sp<TextureProxy> src,
                                       Mipmapped mipmapped,
                                       SkIRect srcRect,
                                       SkBackingFit /* fit */,
                                       SkBudgeted budgeted) {

    // TODO: we could just assert this
    if (!srcRect.intersect(SkIRect::MakeSize(src->dimensions()))) {
        return nullptr;
    }

    const TextureInfo& textureInfo = src->textureInfo();

#if 0
    const Caps* caps = recorder->priv().caps();

    int mipLevelCount = (mipmapped == Mipmapped::kYes)
                        ? SkMipmap::ComputeLevelCount(srcRect.width(), srcRect.height()) + 1
                        : 1;

    auto textureInfo = caps->getDefaultSampledTextureInfo(??,
                                                          mipLevelCount,
                                                          src->textureInfo().isProtected(),
                                                          Renderable::kYes);
#endif
    if (!textureInfo.isValid()) {
        return nullptr;
    }

    sk_sp<TextureProxy> dst(new TextureProxy(srcRect.size(), textureInfo, budgeted));

    sk_sp<CopyTextureToTextureTask> copyTask = CopyTextureToTextureTask::Make(std::move(src),
                                                                              srcRect,
                                                                              dst,
                                                                              {0, 0});
    if (!copyTask) {
        return nullptr;
    }

    recorder->priv().add(std::move(copyTask));
    return dst;
}

#ifdef SK_DEBUG
void TextureProxy::validateTexture(const Texture* texture) {
    SkASSERT(fDimensions == texture->dimensions());
    SkASSERT(fInfo == texture->textureInfo());
}
#endif

} // namespace skgpu::graphite
