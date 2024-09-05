/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ManagedBackendTexture_Graphite_DEFINED
#define ManagedBackendTexture_Graphite_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkYUVAInfo.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "src/gpu/graphite/RecorderPriv.h"

namespace skgpu {
class RefCntedCallback;
}
namespace skgpu::graphite {
class Recorder;
}
class SkBitmap;
struct SkImageInfo;

using Recorder = skgpu::graphite::Recorder;

namespace sk_gpu_test {

/*
 * Graphite version of ManagedBackendTexture
 */
class ManagedGraphiteTexture : public SkNVRefCnt<ManagedGraphiteTexture> {
public:
    static sk_sp<ManagedGraphiteTexture> MakeUnInit(Recorder*,
                                                    const SkImageInfo&,
                                                    skgpu::Mipmapped,
                                                    skgpu::Renderable,
                                                    skgpu::Protected = skgpu::Protected::kNo);

    static sk_sp<ManagedGraphiteTexture> MakeFromPixmap(Recorder*,
                                                        const SkPixmap&,
                                                        skgpu::Mipmapped,
                                                        skgpu::Renderable,
                                                        skgpu::Protected = skgpu::Protected::kNo);

    static sk_sp<ManagedGraphiteTexture> MakeMipmappedFromPixmaps(
            Recorder*,
            SkSpan<const SkPixmap> levels,
            skgpu::Renderable,
            skgpu::Protected = skgpu::Protected::kNo);

    static sk_sp<ManagedGraphiteTexture> MakeFromCompressedData(
            Recorder*,
            SkISize dimmensions,
            SkTextureCompressionType,
            sk_sp<SkData>,
            skgpu::Mipmapped,
            skgpu::Protected = skgpu::Protected::kNo);

    /** finished and image/surface release procs */
    static void FinishedProc(void* context, skgpu::CallbackResult);
    static void ReleaseProc(void* context);
    static void ImageReleaseProc(void* context);

    ~ManagedGraphiteTexture();

    /**
     * The context to use with the ReleaseProcs. This adds a ref so it *must* be balanced by a call
     * to TextureReleaseProc or ImageReleaseProc.
     */
    void* releaseContext() const;

    sk_sp<skgpu::RefCntedCallback> refCountedCallback() const;

    /**
     * SkImage::MakeGraphiteFromYUVABackendTextures takes a single release proc that is called once
     * for all the textures. This makes a single release context for the group of textures. It's
     * used with the standard ReleaseProc. Like releaseContext(), it must be balanced by a
     * ReleaseProc call for proper ref counting.
     */
    static void* MakeYUVAReleaseContext(const sk_sp<ManagedGraphiteTexture>[SkYUVAInfo::kMaxPlanes]);

    const skgpu::graphite::BackendTexture& texture() { return fTexture; }

private:
    ManagedGraphiteTexture() = default;
    ManagedGraphiteTexture(const ManagedGraphiteTexture&) = delete;
    ManagedGraphiteTexture(ManagedGraphiteTexture&&) = delete;

    skgpu::graphite::Context* fContext;
    skgpu::graphite::BackendTexture fTexture;
};

}  // namespace sk_gpu_test

#endif  // ManagedBackendTexture_Graphite_DEFINED
