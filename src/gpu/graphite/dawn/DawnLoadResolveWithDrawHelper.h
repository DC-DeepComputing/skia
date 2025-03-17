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
                            uint16_t msaaOffsetX,
                            uint16_t msaaOffsetY,
                            const DawnTexture* resolveTexture,
                            uint16_t resolveOffsetX,
                            uint16_t resolveOffsetY,
                            uint16_t resolveWidth,
                            uint16_t resolveHeight);

    bool loadFromResolveToMSAATexture();
    bool storeFromMSAAToResolveTexture();

private:
    bool doBlitWithDraw(const DawnTexture* dstTexture,
                        uint16_t dstX,
                        uint16_t dstY,
                        const DawnTexture* srcTexture,
                        uint16_t srcX,
                        uint16_t srcY,
                        uint16_t width,
                        uint16_t height);

    DawnResourceProvider* fResourceProvider;
    wgpu::CommandEncoder fEncoder;
    const DawnTexture* fMsaaTexture;
    uint16_t fMsaaOffsetX;
    uint16_t fMsaaOffsetY;
    const DawnTexture* fResolveTexture;
    uint16_t fResolveOffsetX;
    uint16_t fResolveOffsetY;
    uint16_t fResolveWidth;
    uint16_t fResolveHeight;
};

}  // namespace skgpu::graphite

#endif
