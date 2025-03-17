/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnLoadResolveWithDrawHelper_DEFINED
#define skgpu_graphite_DawnLoadResolveWithDrawHelper_DEFINED

#include "include/core/SkRect.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

namespace skgpu::graphite {

class DawnResourceProvider;
class DawnTexture;

class DawnLoadResolveWithDrawHelper {
public:
    DawnLoadResolveWithDrawHelper(const DawnTexture* msaaTexture,
                                  const DawnTexture* resolveTexture,
                                  SkIRect bounds);

    DawnLoadResolveWithDrawHelper(const DawnLoadResolveWithDrawHelper& src) = default;
    DawnLoadResolveWithDrawHelper(DawnLoadResolveWithDrawHelper&& src) = default;

    DawnLoadResolveWithDrawHelper& operator=(const DawnLoadResolveWithDrawHelper& src) = default;
    DawnLoadResolveWithDrawHelper& operator=(DawnLoadResolveWithDrawHelper&& src) = default;

    bool loadFromResolveToMSAATexture(DawnResourceProvider* resourceProvider,
                                      const wgpu::CommandEncoder& encoder);
    bool storeFromMSAAToResolveTexture(DawnResourceProvider* resourceProvider,
                                       const wgpu::CommandEncoder& encoder);

private:
    bool doBlitWithDraw(DawnResourceProvider* resourceProvider,
                        const wgpu::CommandEncoder& encoder,
                        const DawnTexture* dstTexture,
                        const DawnTexture* srcTexture,
                        const SkIRect& bound);

    const DawnTexture* fMsaaTexture = nullptr;
    const DawnTexture* fResolveTexture = nullptr;
    SkIRect fBounds;
};

}  // namespace skgpu::graphite

#endif
