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
#include "src/gpu/graphite/ResourceCache.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "tools/Resources.h"

#if 0
#include "include/core/SkImage.h"
#include "include/gpu/graphite/Recording.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/Resource.h"

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

void check_counts(Recorder* recorder, skiatest::Reporter* reporter,
                  int numCached,
                  int numNonPurgeable,
                  int numPurgeable) {
    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    ResourceCache* resourceCache = resourceProvider->resourceCache();
    ProxyCache* proxyCache = recorder->priv().proxyCache();

    SkDebugf("num %d nonP %d P %d\n",
             proxyCache->numCached(),
             resourceCache->numNonPurgeableResources(),
             resourceCache->numPurgeableResources());

    REPORTER_ASSERT(reporter, proxyCache->numCached() == numCached);
    REPORTER_ASSERT(reporter, resourceCache->numNonPurgeableResources() == numNonPurgeable);
    REPORTER_ASSERT(reporter, resourceCache->numPurgeableResources() == numPurgeable);
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
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ProxyCacheTest, r, context) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ProxyCache* proxyCache = recorder->priv().proxyCache();
    std::unique_ptr<Recording> recording;

    {
        SkBitmap bitmap;
        bool success = GetResourceAsBitmap("images/mandrill_128.png", &bitmap);
        REPORTER_ASSERT(r, success);
        if (!success) {
            return;
        }

        check_counts(recorder.get(), r, /* cached= */ 0, /* nonPurgeable= */ 0, /* purgeable= */ 0);

        sk_sp<TextureProxy> proxy = proxyCache->findOrCreateCachedProxy(recorder.get(), bitmap,
                                                                        Mipmapped::kNo);
        REPORTER_ASSERT(r, proxy);
        if (!proxy) {
            return;
        }
        check_counts(recorder.get(), r, /* cached= */ 1, /* nonPurgeable= */ 1, /* purgeable= */ 0);

        sk_sp<SkImage> img = create_image(recorder.get(), bitmap);

        recording = create_recording(recorder.get(), img.get());

        img.reset();

        check_counts(recorder.get(), r, /* cached= */ 1, /* nonPurgeable= */ 8, /* purgeable= */ 0);

        bitmap.eraseColor(SK_ColorBLACK);

        check_counts(recorder.get(), r, /* cached= */ 1, /* nonPurgeable= */ 8, /* purgeable= */ 0);
    }

    check_counts(recorder.get(), r, /* cached= */ 1, /* nonPurgeable= */ 8, /* purgeable= */ 0);

    recording.reset();

    check_counts(recorder.get(), r, /* cached= */ 1, /* nonPurgeable= */ 8, /* purgeable= */ 0);
}

}  // namespace skgpu::graphite
