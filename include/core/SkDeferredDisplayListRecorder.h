/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeferredDisplayListRecorder_DEFINED
#define SkDeferredDisplayListRecorder_DEFINED

#include "include/core/SkTypes.h"

#if defined(SK_GANESH)
#include "include/core/SkDeferredDisplayList.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "include/gpu/GrTypes.h"

class GrBackendFormat;
class GrRecordingContext;
class GrRenderTargetProxy;
class GrYUVABackendTextureInfo;
class SkCanvas;
class SkColorSpace;
class SkImage;
class SkPromiseImageTexture;
class SkSurface;
enum SkAlphaType : int;
enum SkColorType : int;


/*
 * This class is intended to be used as:
 *   Get an SkSurfaceCharacterization representing the intended gpu-backed destination SkSurface
 *   Create one of these (an SkDeferredDisplayListRecorder) on the stack
 *   Get the canvas and render into it
 *   Snap off and hold on to an SkDeferredDisplayList
 *   Once your app actually needs the pixels, call SkSurface::draw(SkDeferredDisplayList*)
 *
 * This class never accesses the GPU but performs all the cpu work it can. It
 * is thread-safe (i.e., one can break a scene into tiles and perform their cpu-side
 * work in parallel ahead of time).
 */
class SK_API SkDeferredDisplayListRecorder {
public:
    SkDeferredDisplayListRecorder(const SkSurfaceCharacterization&);
    ~SkDeferredDisplayListRecorder();

    const SkSurfaceCharacterization& characterization() const {
        return fCharacterization;
    }

    // The backing canvas will become invalid (and this entry point will return
    // null) once 'detach' is called.
    // Note: ownership of the SkCanvas is not transferred via this call.
    SkCanvas* getCanvas();

    sk_sp<SkDeferredDisplayList> detach();

    using PromiseImageTextureContext = void*;
    using PromiseImageTextureFulfillProc =
            sk_sp<SkPromiseImageTexture> (*)(PromiseImageTextureContext);
    using PromiseImageTextureReleaseProc = void (*)(PromiseImageTextureContext);

#ifndef SK_MAKE_PROMISE_TEXTURE_DISABLE_LEGACY_API
    /** Deprecated: Use SkImages::PromiseTextureFrom instead. */
    sk_sp<SkImage> makePromiseTexture(const GrBackendFormat& backendFormat,
                                      int width,
                                      int height,
                                      GrMipmapped mipmapped,
                                      GrSurfaceOrigin origin,
                                      SkColorType colorType,
                                      SkAlphaType alphaType,
                                      sk_sp<SkColorSpace> colorSpace,
                                      PromiseImageTextureFulfillProc textureFulfillProc,
                                      PromiseImageTextureReleaseProc textureReleaseProc,
                                      PromiseImageTextureContext textureContext);

    /** Deprecated: Use SkImages::PromiseTextureFromYUVA instead. */
    sk_sp<SkImage> makeYUVAPromiseTexture(const GrYUVABackendTextureInfo& yuvaBackendTextureInfo,
                                          sk_sp<SkColorSpace> imageColorSpace,
                                          PromiseImageTextureFulfillProc textureFulfillProc,
                                          PromiseImageTextureReleaseProc textureReleaseProc,
                                          PromiseImageTextureContext textureContexts[]);
#endif // SK_MAKE_PROMISE_TEXTURE_DISABLE_LEGACY_API


private:
    SkDeferredDisplayListRecorder(const SkDeferredDisplayListRecorder&) = delete;
    SkDeferredDisplayListRecorder& operator=(const SkDeferredDisplayListRecorder&) = delete;

    bool init();

    const SkSurfaceCharacterization             fCharacterization;
    sk_sp<GrRecordingContext>                   fContext;
    sk_sp<GrRenderTargetProxy>                  fTargetProxy;
    sk_sp<SkDeferredDisplayList::LazyProxyData> fLazyProxyData;
    sk_sp<SkSurface>                            fSurface;
};

#endif // defined(SK_GANESH)

#endif
