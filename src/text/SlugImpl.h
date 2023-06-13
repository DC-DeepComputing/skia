/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef sktext_SlugImpl_DEFINED
#define sktext_SlugImpl_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkCPUTypes.h"
#include "include/private/chromium/Slug.h"
#include "src/core/SkDevice.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkWriteBuffer.h"
#include "src/text/GlyphRun.h"
#include "src/text/gpu/SDFTControl.h"
#include "src/text/gpu/SubRunAllocator.h"
#include "src/text/gpu/SubRunContainer.h"

namespace sktext {

class SlugImpl final : public Slug {
public:
    SlugImpl(SubRunAllocator&& alloc,
             gpu::SubRunContainerOwner subRuns,
             SkRect sourceBounds,
             const SkPaint& paint,
             SkPoint origin);
    ~SlugImpl() override = default;

    static sk_sp<SlugImpl> Make(const SkMatrixProvider& viewMatrix,
                                const sktext::GlyphRunList& glyphRunList,
                                const SkPaint& initialPaint,
                                const SkPaint& drawingPaint,
                                SkStrikeDeviceInfo strikeDeviceInfo,
                                sktext::StrikeForGPUCacheInterface* strikeCache);
    static sk_sp<Slug> MakeFromBuffer(SkReadBuffer& buffer,
                                      const SkStrikeClient* client);
    void doFlatten(SkWriteBuffer& buffer) const override;

// TODO(kjlubick) move these methods to the appropriate GPU backends.
#if defined(SK_GANESH)
    void surfaceDraw(SkCanvas*,
                     const GrClip* clip,
                     const SkMatrixProvider& viewMatrix,
                     const SkPaint& paint,
                     skgpu::ganesh::SurfaceDrawContext* sdc) const;
#endif

#if defined(SK_GRAPHITE)
    void surfaceDraw(SkCanvas*, const SkPaint& paint, skgpu::graphite::Device* device) const;
#endif

    SkRect sourceBounds() const override { return fSourceBounds; }
    SkRect sourceBoundsWithOrigin() const override { return fSourceBounds.makeOffset(fOrigin); }
    const SkPaint& initialPaint() const override { return fInitialPaint; }

    const SkMatrix& initialPositionMatrix() const { return fSubRuns->initialPosition(); }
    SkPoint origin() const { return fOrigin; }

    // Change memory management to handle the data after Slug, but in the same allocation
    // of memory. Only allow placement new.
    void operator delete(void* p) { ::operator delete(p); }
    void* operator new(size_t) { SK_ABORT("All slugs are created by placement new."); }
    void* operator new(size_t, void* p) { return p; }

private:
    // The allocator must come first because it needs to be destroyed last. Other fields of this
    // structure may have pointers into it.
    SubRunAllocator fAlloc;
    gpu::SubRunContainerOwner fSubRuns;
    const SkRect fSourceBounds;
    const SkPaint fInitialPaint;
    const SkPoint fOrigin;
};

}  // namespace sktext

#endif