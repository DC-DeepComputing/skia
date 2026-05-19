/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/chromium/GrDeferredDisplayList.h"
#include "include/private/chromium/GrDeferredDisplayListRecorder.h"
#include "include/private/chromium/GrSurfaceCharacterization.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkRandom.h"
#include "src/core/SkColorData.h"
#include "src/core/SkMessageBus.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrAppliedClip.h"
#include "src/gpu/ganesh/GrBuffer.h"
#include "src/gpu/ganesh/GrCanvas.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrDefaultGeoProcFactory.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrProcessorSet.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrResourceCache.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/GrThreadSafeCache.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/image/GrMippedBitmap.h"
#include "src/gpu/ganesh/ops/GrDrawOp.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#include "tests/ComparePixels.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/ganesh/ProxyUtils.h"


#include "src/gpu/ResourceKey.h"

#include <thread>

namespace {

skgpu::UniqueKey create_key(int keyValue) {
    static skgpu::UniqueKey::Domain d = skgpu::UniqueKey::GenerateDomain();

    skgpu::UniqueKey key;

    skgpu::UniqueKey::Builder builder(&key, d, 1, nullptr);
    builder[0] = keyValue;
    builder.finish();

    return key;
}

sk_sp<SkImage> create_keyed_image(GrRecordingContext* rContext, int keyValue) {
    SkImageInfo ii = SkImageInfo::MakeN32Premul(16, 16);
    sk_sp<SkSurface> surf = SkSurfaces::RenderTarget(rContext, skgpu::Budgeted::kYes, ii);

    surf->getCanvas()->clear(SK_ColorWHITE);
    surf->getCanvas()->drawCircle(8, 8, 8, SkPaint());

    sk_sp<SkImage> img = surf->makeImageSnapshot();

    GrTextureProxy* proxy = sk_gpu_test::GetTextureImageProxy(img.get(),
                                                              surf->recordingContext());

    skgpu::UniqueKey key = create_key(keyValue);

    rContext->priv().proxyProvider()->assignUniqueKeyToProxy(key, proxy);

    return img;
}

void ddl_deleter(skiatest::Reporter* reporter,
                 sk_sp<GrUniquelyKeyedProxyRegistry> registry,
                 sk_sp<GrDeferredDisplayList> ddl,
                 int keyValue) {
    const auto kSleepDuration = std::chrono::milliseconds(1);

    std::this_thread::sleep_for(kSleepDuration);

    skgpu::UniqueKey key = create_key(keyValue);

    REPORTER_ASSERT(reporter, registry->find(key));
    ddl.reset();
    REPORTER_ASSERT(reporter, !registry->find(key));
}

} // anonymous namespace

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrUniqueProxyTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    GrDirectContext* dContext = ctxInfo.directContext();

    GrSurfaceCharacterization characterization;

    // Grab a characterization
    {
        SkImageInfo ii = SkImageInfo::MakeN32Premul(16, 16);
        sk_sp<SkSurface> surf = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, ii);
        SkAssertResult(surf);

        SkAssertResult(surf->characterize(&characterization));
    }

    // Create a recorder
    std::unique_ptr<GrDeferredDisplayListRecorder> recorder;
    recorder = std::make_unique<GrDeferredDisplayListRecorder>(characterization);

    GrProxyProvider* proxyProvider = recorder->getCanvas()->recordingContext()->priv().proxyProvider();
    sk_sp<GrUniquelyKeyedProxyRegistry> registry = proxyProvider->uniquelyKeyedProxyRegistry();

    std::vector<std::thread> threads;

    // Create DDLs and immediately hand them off to a thread for deletion. Each DDL will have
    // a single uniquely keyed proxy.
    for (int i = 0; i < 10; i++) {
        SkCanvas* canvas = recorder->getCanvas();
        canvas->clear(SkColors::kRed);

        sk_sp<SkImage> img = create_keyed_image(canvas->recordingContext(), i);

        canvas->drawImage(img.get(), 0, 0);

        sk_sp<GrDeferredDisplayList> ddl = recorder->detach();

        threads.push_back(std::thread(ddl_deleter, reporter, registry, std::move(ddl), i));
    }

    // Delete the recorder - hopefully before all the DDLs are deleted
    recorder.reset();

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // All the texture proxies should be deleted and have deregistered their keys
    REPORTER_ASSERT(reporter, registry->count() == 0);
}
