/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

class ResourceProvider;

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(GraphiteTextureProxyTest, reporter, context) {
    const Caps* caps = context->priv().caps();
    constexpr SkISize validSize = SkISize::Make(1, 1);
    constexpr SkISize invalidSize = SkISize::MakeEmpty();
    constexpr SkColorType validColorType = kRGBA_8888_SkColorType;
    constexpr SkColorType invalidColorType = kUnknown_SkColorType;

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    const TextureInfo textureInfo = caps->getDefaultSampledTextureInfo(
            validColorType, Mipmapped::kNo, Protected::kNo, Renderable::kNo);
    const BackendTexture backendTexture = recorder->createBackendTexture(validSize, textureInfo);
    sk_sp<Texture> texture = resourceProvider->createWrappedTexture(backendTexture);

    auto nullCallback = [](ResourceProvider*) -> sk_sp<Texture> { return nullptr; };
    auto callback = [texture](ResourceProvider*) -> sk_sp<Texture> { return texture; };

    // Invalid parameters.
    sk_sp<TextureProxy> textureProxy;
    textureProxy = TextureProxy::Make(caps,
                                      invalidSize,
                                      validColorType,
                                      Mipmapped::kNo,
                                      Protected::kNo,
                                      Renderable::kNo,
                                      SkBudgeted::kNo);
    REPORTER_ASSERT(reporter, textureProxy == nullptr);
    textureProxy = TextureProxy::Make(caps,
                                      validSize,
                                      invalidColorType,
                                      Mipmapped::kNo,
                                      Protected::kNo,
                                      Renderable::kNo,
                                      SkBudgeted::kNo);
    REPORTER_ASSERT(reporter, textureProxy == nullptr);

    // Non-lazy TextureProxy, successful instantiation.
    textureProxy = TextureProxy::Make(caps,
                                      validSize,
                                      validColorType,
                                      Mipmapped::kNo,
                                      Protected::kNo,
                                      Renderable::kNo,
                                      SkBudgeted::kNo);
    REPORTER_ASSERT(reporter, !textureProxy->isLazy());
    REPORTER_ASSERT(reporter, !textureProxy->isVolatile());
    REPORTER_ASSERT(reporter, !textureProxy->isInstantiated());
    REPORTER_ASSERT(reporter, textureProxy->dimensions() == validSize);

    bool instantiateSuccess = textureProxy->instantiate(resourceProvider);
    REPORTER_ASSERT(reporter, instantiateSuccess);
    REPORTER_ASSERT(reporter, textureProxy->isInstantiated());
    REPORTER_ASSERT(reporter, textureProxy->dimensions() == validSize);
    const Texture* createdTexture = textureProxy->texture();

    instantiateSuccess = textureProxy->instantiate(resourceProvider);
    REPORTER_ASSERT(reporter, instantiateSuccess);
    REPORTER_ASSERT(reporter, textureProxy->texture() == createdTexture);

    // Lazy, non-volatile TextureProxy, unsuccessful instantiation.
    textureProxy = TextureProxy::MakeLazy(
            validSize, textureInfo, SkBudgeted::kNo, Volatile::kNo, nullCallback);
    REPORTER_ASSERT(reporter, textureProxy->isLazy());
    REPORTER_ASSERT(reporter, !textureProxy->isVolatile());

    instantiateSuccess = textureProxy->lazyInstantiate(resourceProvider);
    REPORTER_ASSERT(reporter, !instantiateSuccess);
    REPORTER_ASSERT(reporter, !textureProxy->isInstantiated());

    // Lazy, non-volatile TextureProxy, successful instantiation.
    textureProxy = TextureProxy::MakeLazy(
            validSize, textureInfo, SkBudgeted::kNo, Volatile::kNo, callback);

    instantiateSuccess = textureProxy->lazyInstantiate(resourceProvider);
    REPORTER_ASSERT(reporter, instantiateSuccess);
    REPORTER_ASSERT(reporter, textureProxy->texture() == texture.get());

    // Lazy, volatile TextureProxy, unsuccessful instantiation.
    textureProxy = TextureProxy::MakeLazy(
            validSize, textureInfo, SkBudgeted::kNo, Volatile::kYes, nullCallback);
    REPORTER_ASSERT(reporter, textureProxy->isLazy());
    REPORTER_ASSERT(reporter, textureProxy->isVolatile());

    instantiateSuccess = textureProxy->lazyInstantiate(resourceProvider);
    REPORTER_ASSERT(reporter, !instantiateSuccess);
    REPORTER_ASSERT(reporter, !textureProxy->isInstantiated());

    // Lazy, volatile TextureProxy, successful instantiation.
    textureProxy = TextureProxy::MakeLazy(
            validSize, textureInfo, SkBudgeted::kNo, Volatile::kYes, callback);

    instantiateSuccess = textureProxy->lazyInstantiate(resourceProvider);
    REPORTER_ASSERT(reporter, instantiateSuccess);
    REPORTER_ASSERT(reporter, textureProxy->texture() == texture.get());

    textureProxy->deinstantiate();
    REPORTER_ASSERT(reporter, !textureProxy->isInstantiated());

    // InstantiateIfNotLazy tests.
    textureProxy = TextureProxy::Make(caps,
                                      validSize,
                                      validColorType,
                                      Mipmapped::kNo,
                                      Protected::kNo,
                                      Renderable::kNo,
                                      SkBudgeted::kNo);
    instantiateSuccess =
            TextureProxy::InstantiateIfNotLazy(resourceProvider, textureProxy.get());
    REPORTER_ASSERT(reporter, instantiateSuccess);

    textureProxy = TextureProxy::MakeLazy(
            validSize, textureInfo, SkBudgeted::kNo, Volatile::kNo, nullCallback);
    instantiateSuccess =
            TextureProxy::InstantiateIfNotLazy(resourceProvider, textureProxy.get());
    REPORTER_ASSERT(reporter, instantiateSuccess);
}

}  // namespace skgpu::graphite
