/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnLoadResolveWithDrawHelper_DEFINED
#define skgpu_graphite_DawnLoadResolveWithDrawHelper_DEFINED

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

namespace skgpu::graphite {

class DawnResourceProvider;
class DawnTexture;

class DawnLoadResolveWithDrawHelper {
public:
    DawnLoadResolveWithDrawHelper(DawnResourceProvider* resourceProvider,
                                  const wgpu::CommandEncoder& encoder,
                                  const DawnTexture* msaaTexture,
                                  const DawnTexture* resolveTexture,
                                  uint16_t resolveOffsetX,
                                  uint16_t resolveOffsetY,
                                  uint16_t resolveWidth,
                                  uint16_t resolveHeight);

    DawnLoadResolveWithDrawHelper(const DawnLoadResolveWithDrawHelper& src) = default;
    DawnLoadResolveWithDrawHelper(DawnLoadResolveWithDrawHelper&& src) = default;

    DawnLoadResolveWithDrawHelper& operator=(const DawnLoadResolveWithDrawHelper& src) = default;
    DawnLoadResolveWithDrawHelper& operator=(DawnLoadResolveWithDrawHelper&& src) = default;

    bool loadFromResolveToMSAATexture();
    bool storeFromMSAAToResolveTexture();

private:
    bool doBlitWithDraw(const DawnTexture* dstTexture,
                        const DawnTexture* srcTexture,
                        uint16_t x,
                        uint16_t y,
                        uint16_t width,
                        uint16_t height);

    DawnResourceProvider* fResourceProvider = nullptr;
    wgpu::CommandEncoder fEncoder;
    const DawnTexture* fMsaaTexture = nullptr;
    const DawnTexture* fResolveTexture = nullptr;
    uint16_t fResolveOffsetX = 0;
    uint16_t fResolveOffsetY = 0;
    uint16_t fResolveWidth = 0;
    uint16_t fResolveHeight = 0;
};

}  // namespace skgpu::graphite

#endif
