/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/ProxyCache.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "tools/Resources.h"

#if 0
#include "include/core/SkImage.h"
#include "include/gpu/graphite/Recording.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceCache.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/image/SkImage_Base.h"
#endif

namespace skgpu::graphite {

namespace {

sk_sp<SkImage> create_image(Recorder* recorder, const SkBitmap& bitmap) {
    sk_sp<SkImage> img = bitmap.asImage();

    return img->makeTextureImage(recorder, { skgpu::Mipmapped::kNo });
}

// Create a Recording that just contains a draw of the provided image
std::unique_ptr<skgpu::graphite::Recording> create_recording(Recorder* recorder, SkImage* image) {

    SkImageInfo ii = SkImageInfo::Make(image->width(), image->height(),
                                       kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurface::MakeGraphite(recorder, ii);
    if (!surface) {
        return nullptr;
    }

    SkCanvas* canvas = surface->getCanvas();

    canvas->drawImage(image, 0, 0);

    return recorder->snap();
}

} // anonymous namespace

// In this test we:
//    1) manually insert an SkBitmap into the proxy cache
//          ProxyCache: contains proxy    ResourceCache: contains texture    Proxy: 1 ref
//    2) create a Recording that uses that proxy
//          ProxyCache: same              ResourceCache: same                Proxy: 2 refs
//    3) invalidate the proxy cache entry
//          ProxyCache: no proxy          ResourceCache: same                Proxy: 1 ref
//    4) delete the recording
//          ProxyCache: no proxy          ResourceCache: no texture          Proxy: deleted
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ProxyCacheTest, reporter, context) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ProxyCache* proxyCache = recorder->priv().proxyCache();
    std::unique_ptr<Recording> recording;

    {
        SkBitmap bitmap;
        bool success = GetResourceAsBitmap("images/mandrill_128.png", &bitmap);
        REPORTER_ASSERT(reporter, success);
        if (!success) {
            return;
        }

        REPORTER_ASSERT(reporter, proxyCache->numCached() == 0);

        sk_sp<TextureProxy> proxy = proxyCache->findOrCreateCachedProxy(recorder.get(), bitmap,
                                                                        Mipmapped::kNo);
        REPORTER_ASSERT(reporter, proxy);
        if (!proxy) {
            return;
        }
        REPORTER_ASSERT(reporter, proxyCache->numCached() == 1);

        sk_sp<SkImage> img = create_image(recorder.get(), bitmap);

        recording = create_recording(recorder.get(), img.get());

        img.reset();
    }

}

}  // namespace skgpu::graphite
