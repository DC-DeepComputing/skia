/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "include/private/SkAlign.h"
#include "include/private/SkCPUTypes.h"
#include "include/private/SkDebug.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkTo.h"
#include "src/core/SkBlitMask.h"
#include "src/core/SkBlitRow.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkColorData.h"
#include "src/core/SkColorPriv.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkMask.h"
#include "src/core/SkMemset.h"
#include "src/core/SkUtils.h"
#include "src/core/SkVx.h"
#include "src/shaders/SkShaderBase.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>

static inline int scale_255(int value, int scale) {
    SkASSERT((unsigned)value <= 255);
    SkASSERT((unsigned)scale <= 255);
    return (value * scale + 127) / 255;
}

static inline int blend_255(int src, int dst, int scale) {
    SkASSERT((unsigned)src <= 0xFF);
    SkASSERT((unsigned)dst <= 0xFF);
    SkASSERT((unsigned)scale <= 255);
    return (src * scale + dst * (255 - scale) + 127) / 255;
}

static inline SkPMColor blend_lcd32(int srcA, int srcR, int srcG, int srcB,
                                     SkPMColor dst, uint32_t mask) {
    int maskR = (mask >> SK_RGBA_R32_SHIFT) & 0xFF;
    int maskG = (mask >> SK_RGBA_G32_SHIFT) & 0xFF;
    int maskB = (mask >> SK_RGBA_B32_SHIFT) & 0xFF;

    if ((maskR | maskG | maskB) == 0) {
        return dst;
    }

    maskR = scale_255(maskR, srcA);
    maskG = scale_255(maskG, srcA);
    maskB = scale_255(maskB, srcA);

    int dstA = SkGetPackedA32(dst);
    int dstR = SkGetPackedR32(dst);
    int dstG = SkGetPackedG32(dst);
    int dstB = SkGetPackedB32(dst);

    int maskA = srcA < dstA ? std::min(maskR, std::min(maskG, maskB))
                            : std::max(maskR, std::max(maskG, maskB));

    return SkPackARGB32(blend_255(0xFF, dstA, maskA),
                        blend_255(srcR, dstR, maskR),
                        blend_255(srcG, dstG, maskG),
                        blend_255(srcB, dstB, maskB));
}

static inline SkPMColor blend_lcd32_opaque(int srcR, int srcG, int srcB,
                                           SkPMColor dst, uint32_t mask,
                                           SkPMColor opaqueDst) {
    const int maskR = (mask >> SK_RGBA_R32_SHIFT) & 0xFF;
    const int maskG = (mask >> SK_RGBA_G32_SHIFT) & 0xFF;
    const int maskB = (mask >> SK_RGBA_B32_SHIFT) & 0xFF;

    if ((maskR | maskG | maskB) == 0) {
        return dst;
    }

    if (maskR == 255 && maskG == 255 && maskB == 255) {
        return opaqueDst;
    }

    int dstA = SkGetPackedA32(dst);
    int dstR = SkGetPackedR32(dst);
    int dstG = SkGetPackedG32(dst);
    int dstB = SkGetPackedB32(dst);
    int maskA = std::max(maskR, std::max(maskG, maskB));

    return SkPackARGB32(blend_255(0xFF, dstA, maskA),
                        blend_255(srcR, dstR, maskR),
                        blend_255(srcG, dstG, maskG),
                        blend_255(srcB, dstB, maskB));
}

static inline void blit_row_lcd32(SkPMColor dst[], const uint32_t mask[],
                                  SkColor src, int width, SkPMColor) {
    int srcA = SkColorGetA(src);
    int srcR = SkColorGetR(src);
    int srcG = SkColorGetG(src);
    int srcB = SkColorGetB(src);

    for (int i = 0; i < width; i++) {
        dst[i] = blend_lcd32(srcA, srcR, srcG, srcB, dst[i], mask[i]);
    }
}

static inline void blit_row_lcd32_opaque(SkPMColor dst[], const uint32_t mask[],
                                         SkColor src, int width,
                                         SkPMColor opaqueDst) {
    int srcR = SkColorGetR(src);
    int srcG = SkColorGetG(src);
    int srcB = SkColorGetB(src);

    for (int i = 0; i < width; i++) {
        dst[i] = blend_lcd32_opaque(srcR, srcG, srcB, dst[i], mask[i], opaqueDst);
    }
}

static bool blit_color(const SkPixmap& device,
                       const SkMask& mask,
                       const SkIRect& clip,
                       SkColor color) {
    int x = clip.fLeft,
        y = clip.fTop;

    if (device.colorType() == kN32_SkColorType && mask.fFormat == SkMask::kA8_Format) {
        SkOpts::blit_mask_d32_a8(device.writable_addr32(x,y), device.rowBytes(),
                                 (const SkAlpha*)mask.getAddr(x,y), mask.fRowBytes,
                                 color, clip.width(), clip.height());
        return true;
    }

    if (device.colorType() == kN32_SkColorType && mask.fFormat == SkMask::kLCD32_Format) {
        auto dstRow  = device.writable_addr32(x,y);
        auto maskRow = (const uint32_t*)mask.getAddr(x,y);

        auto blit_row = blit_row_lcd32;
        SkPMColor opaqueDst = 0;  // ignored unless opaque

        if (0xff == SkColorGetA(color)) {
            blit_row  = blit_row_lcd32_opaque;
            opaqueDst = SkPreMultiplyColor(color);
        }

        for (int height = clip.height(); height --> 0; ) {
            blit_row(dstRow, maskRow, color, clip.width(), opaqueDst);

            dstRow  = (SkPMColor*)     ((      char*) dstRow + device.rowBytes());
            maskRow = (const uint32_t*)((const char*)maskRow +  mask.fRowBytes);
        }
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

static void SkARGB32_Blit32(const SkPixmap& device, const SkMask& mask,
                            const SkIRect& clip, SkPMColor srcColor) {
    U8CPU alpha = SkGetPackedA32(srcColor);
    unsigned flags = SkBlitRow::kSrcPixelAlpha_Flag32;
    if (alpha != 255) {
        flags |= SkBlitRow::kGlobalAlpha_Flag32;
    }
    SkBlitRow::Proc32 proc = SkBlitRow::Factory32(flags);

    int x = clip.fLeft;
    int y = clip.fTop;
    int width = clip.width();
    int height = clip.height();

    SkPMColor* dstRow = device.writable_addr32(x, y);
    const SkPMColor* srcRow = reinterpret_cast<const SkPMColor*>(mask.getAddr8(x, y));

    do {
        proc(dstRow, srcRow, width, alpha);
        dstRow = (SkPMColor*)((char*)dstRow + device.rowBytes());
        srcRow = (const SkPMColor*)((const char*)srcRow + mask.fRowBytes);
    } while (--height != 0);
}

//////////////////////////////////////////////////////////////////////////////////////

SkARGB32_Blitter::SkARGB32_Blitter(const SkPixmap& device, const SkPaint& paint)
        : SkRasterBlitter(device) {
    SkColor color = paint.getColor();
    fColor = color;
    fSrcA = SkColorGetA(color);
    fPMColor = SkPreMultiplyColor(fColor);
}

#if defined _WIN32  // disable warning : local variable used without having been initialized
#pragma warning ( push )
#pragma warning ( disable : 4701 )
#endif

void SkARGB32_Blitter::blitH(int x, int y, int width) {
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width());

    uint32_t* device = fDevice.writable_addr32(x, y);
    SkBlitRow::Color32(device, width, fPMColor);
}

void SkARGB32_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                 const int16_t runs[]) {
    SkASSERT(fSrcA != 0xFF);  // There is an opaque specialization
    if (fSrcA == 0) {
        return;
    }

    uint32_t* device = fDevice.writable_addr32(x, y);

    for (;;) {
        int count = runs[0];
        SkASSERT(count >= 0);
        if (count <= 0) {
            return;
        }
        SkAlpha aa = antialias[0];
        if (aa) {
            SkPMColor sc = SkAlphaMulQ(fPMColor, SkAlpha255To256(aa));
            SkBlitRow::Color32(device, count, sc);
        }
        runs += count;
        antialias += count;
        device += count;
    }
}

void SkARGB32_Blitter::blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) {
    uint32_t* device = fDevice.writable_addr32(x, y);
    SkDEBUGCODE((void)fDevice.writable_addr32(x + 1, y);)

    device[0] = SkBlendARGB32(fPMColor, device[0], a0);
    device[1] = SkBlendARGB32(fPMColor, device[1], a1);
}

void SkARGB32_Blitter::blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) {
    uint32_t* device = fDevice.writable_addr32(x, y);
    SkDEBUGCODE((void)fDevice.writable_addr32(x, y + 1);)

    device[0] = SkBlendARGB32(fPMColor, device[0], a0);
    device = (uint32_t*)((char*)device + fDevice.rowBytes());
    device[0] = SkBlendARGB32(fPMColor, device[0], a1);
}

//////////////////////////////////////////////////////////////////////////////////////

#define solid_8_pixels(mask, dst, color)    \
    do {                                    \
        if (mask & 0x80) dst[0] = color;    \
        if (mask & 0x40) dst[1] = color;    \
        if (mask & 0x20) dst[2] = color;    \
        if (mask & 0x10) dst[3] = color;    \
        if (mask & 0x08) dst[4] = color;    \
        if (mask & 0x04) dst[5] = color;    \
        if (mask & 0x02) dst[6] = color;    \
        if (mask & 0x01) dst[7] = color;    \
    } while (0)

#define SK_BLITBWMASK_NAME                  SkARGB32_BlitBW
#define SK_BLITBWMASK_ARGS                  , SkPMColor color
#define SK_BLITBWMASK_BLIT8(mask, dst)      solid_8_pixels(mask, dst, color)
#define SK_BLITBWMASK_GETADDR               writable_addr32
#define SK_BLITBWMASK_DEVTYPE               uint32_t
#include "src/core/SkBlitBWMaskTemplate.h"

#define blend_8_pixels(mask, dst, sc, dst_scale)                            \
    do {                                                                    \
        if (mask & 0x80) { dst[0] = sc + SkAlphaMulQ(dst[0], dst_scale); }  \
        if (mask & 0x40) { dst[1] = sc + SkAlphaMulQ(dst[1], dst_scale); }  \
        if (mask & 0x20) { dst[2] = sc + SkAlphaMulQ(dst[2], dst_scale); }  \
        if (mask & 0x10) { dst[3] = sc + SkAlphaMulQ(dst[3], dst_scale); }  \
        if (mask & 0x08) { dst[4] = sc + SkAlphaMulQ(dst[4], dst_scale); }  \
        if (mask & 0x04) { dst[5] = sc + SkAlphaMulQ(dst[5], dst_scale); }  \
        if (mask & 0x02) { dst[6] = sc + SkAlphaMulQ(dst[6], dst_scale); }  \
        if (mask & 0x01) { dst[7] = sc + SkAlphaMulQ(dst[7], dst_scale); }  \
    } while (0)

#define SK_BLITBWMASK_NAME                  SkARGB32_BlendBW
#define SK_BLITBWMASK_ARGS                  , uint32_t sc, unsigned dst_scale
#define SK_BLITBWMASK_BLIT8(mask, dst)      blend_8_pixels(mask, dst, sc, dst_scale)
#define SK_BLITBWMASK_GETADDR               writable_addr32
#define SK_BLITBWMASK_DEVTYPE               uint32_t
#include "src/core/SkBlitBWMaskTemplate.h"

void SkARGB32_Blitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));
    SkASSERT(fSrcA != 0xFF);

    if (fSrcA == 0) {
        return;
    }

    if (blit_color(fDevice, mask, clip, fColor)) {
        return;
    }

    switch (mask.fFormat) {
        case SkMask::kBW_Format:
            SkARGB32_BlendBW(fDevice, mask, clip, fPMColor, SkAlpha255To256(255 - fSrcA));
            break;
        case SkMask::kARGB32_Format:
            SkARGB32_Blit32(fDevice, mask, clip, fPMColor);
            break;
        default:
            SK_ABORT("Mask format not handled.");
    }
}

void SkARGB32_Opaque_Blitter::blitMask(const SkMask& mask,
                                       const SkIRect& clip) {
    SkASSERT(fSrcA == 0xFF);
    SkASSERT(mask.fBounds.contains(clip));

    if (blit_color(fDevice, mask, clip, fColor)) {
        return;
    }

    switch (mask.fFormat) {
        case SkMask::kBW_Format:
            SkARGB32_BlitBW(fDevice, mask, clip, fPMColor);
            break;
        case SkMask::kARGB32_Format:
            SkARGB32_Blit32(fDevice, mask, clip, fPMColor);
            break;
        default:
            SK_ABORT("Mask format not handled.");
    }
}

void SkARGB32_Opaque_Blitter::blitAntiH(int x,
                                        int y,
                                        const SkAlpha antialias[],
                                        const int16_t runs[]) {
    SkASSERT(fSrcA == 0xFF);

    uint32_t* device = fDevice.writable_addr32(x, y);
    for (;;) {
        int count = runs[0];
        SkASSERT(count >= 0);
        if (count <= 0) {
            return;
        }
        SkAlpha aa = antialias[0];
        if (aa == 255) {
            SkOpts::memset32(device, fPMColor, count);
        } else if (aa > 0) {
            SkPMColor sc = SkAlphaMulQ(fPMColor, SkAlpha255To256(aa));
            SkBlitRow::Color32(device, count, sc);
        }
        runs += count;
        antialias += count;
        device += count;
    }
}

void SkARGB32_Opaque_Blitter::blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) {
    uint32_t* device = fDevice.writable_addr32(x, y);
    SkDEBUGCODE((void)fDevice.writable_addr32(x + 1, y);)

    device[0] = SkFastFourByteInterp(fPMColor, device[0], a0);
    device[1] = SkFastFourByteInterp(fPMColor, device[1], a1);
}

void SkARGB32_Opaque_Blitter::blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) {
    uint32_t* device = fDevice.writable_addr32(x, y);
    SkDEBUGCODE((void)fDevice.writable_addr32(x, y + 1);)

    device[0] = SkFastFourByteInterp(fPMColor, device[0], a0);
    device = (uint32_t*)((char*)device + fDevice.rowBytes());
    device[0] = SkFastFourByteInterp(fPMColor, device[0], a1);
}

std::optional<SkBlitter::DirectBlit> SkARGB32_Opaque_Blitter::canDirectBlit() {
    return {{ fDevice, fPMColor }};
}

///////////////////////////////////////////////////////////////////////////////

void SkARGB32_Blitter::blitV(int x, int y, int height, SkAlpha alpha) {
    if (alpha == 0 || fSrcA == 0) {
        return;
    }

    uint32_t* device = fDevice.writable_addr32(x, y);
    SkPMColor color = fPMColor;

    if (alpha != 255) {
        color = SkAlphaMulQ(color, SkAlpha255To256(alpha));
    }

    const unsigned dst_scale = SkAlpha255To256(255 - SkGetPackedA32(color));
    const size_t rowBytes = fDevice.rowBytes();
    while (--height >= 0) {
        device[0] = color + SkAlphaMulQ(device[0], dst_scale);
        device = (uint32_t*)((char*)device + rowBytes);
    }
}

void SkARGB32_Blitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width() && y + height <= fDevice.height());

    if (fSrcA == 0) {
        return;
    }

    uint32_t* device = fDevice.writable_addr32(x, y);
    const size_t rowBytes = fDevice.rowBytes();

    if (SkGetPackedA32(fPMColor) == 0xFF) {
        SkOpts::rect_memset32(device, fPMColor, width, rowBytes, height);
    } else {
        while (height --> 0) {
            SkBlitRow::Color32(device, width, fPMColor);
            device = (uint32_t*)((char*)device + rowBytes);
        }
    }
}

#if defined _WIN32
#pragma warning ( pop )
#endif

///////////////////////////////////////////////////////////////////////

void SkARGB32_Black_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                       const int16_t runs[]) {
    uint32_t*   device = fDevice.writable_addr32(x, y);
    static constexpr SkPMColor kBlack = (SkPMColor)(SK_A32_MASK << SK_A32_SHIFT);

    for (;;) {
        int count = runs[0];
        SkASSERT(count >= 0);
        if (count <= 0) {
            return;
        }
        unsigned aa = antialias[0];
        if (aa) {
            if (aa == 255) {
                SkOpts::memset32(device, kBlack, count);
            } else {
                const SkPMColor src = aa << SK_A32_SHIFT;
                const unsigned dst_scale = SkAlpha255To256(255 - aa);
                int n = count;
                do {
                    --n;
                    device[n] = src + SkAlphaMulQ(device[n], dst_scale);
                } while (n > 0);
            }
        }
        runs += count;
        antialias += count;
        device += count;
    }
}

void SkARGB32_Black_Blitter::blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) {
    uint32_t* device = fDevice.writable_addr32(x, y);
    SkDEBUGCODE((void)fDevice.writable_addr32(x + 1, y);)

    device[0] = (a0 << SK_A32_SHIFT) + SkAlphaMulQ(device[0], 256 - a0);
    device[1] = (a1 << SK_A32_SHIFT) + SkAlphaMulQ(device[1], 256 - a1);
}

void SkARGB32_Black_Blitter::blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) {
    uint32_t* device = fDevice.writable_addr32(x, y);
    SkDEBUGCODE((void)fDevice.writable_addr32(x, y + 1);)

    device[0] = (a0 << SK_A32_SHIFT) + SkAlphaMulQ(device[0], 256 - a0);
    device = (uint32_t*)((char*)device + fDevice.rowBytes());
    device[0] = (a1 << SK_A32_SHIFT) + SkAlphaMulQ(device[0], 256 - a1);
}

///////////////////////////////////////////////////////////////////////////////

SkARGB32_Shader_Blitter::SkARGB32_Shader_Blitter(const SkPixmap& device,
                                                 const SkPaint& paint,
                                                 SkShaderBase::Context* shaderContext)
        : SkShaderBlitter(device, paint, shaderContext) {
    fBuffer = (SkPMColor*)sk_malloc_throw(device.width() * (sizeof(SkPMColor)));

    SkASSERT(paint.isSrcOver());

    int flags = 0;
    if (!(shaderContext->getFlags() & SkShaderBase::kOpaqueAlpha_Flag)) {
        flags |= SkBlitRow::kSrcPixelAlpha_Flag32;
    }
    // we call this on the output from the shader
    fProc32 = SkBlitRow::Factory32(flags);
    // we call this on the output from the shader + alpha from the aa buffer
    fProc32Blend = SkBlitRow::Factory32(flags | SkBlitRow::kGlobalAlpha_Flag32);

    fShadeDirectlyIntoDevice =
            SkToBool(shaderContext->getFlags() & SkShaderBase::kOpaqueAlpha_Flag);
}

SkARGB32_Shader_Blitter::~SkARGB32_Shader_Blitter() {
    sk_free(fBuffer);
}

void SkARGB32_Shader_Blitter::blitH(int x, int y, int width) {
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width());

    uint32_t* device = fDevice.writable_addr32(x, y);

    if (fShadeDirectlyIntoDevice) {
        fShaderContext->shadeSpan(x, y, device, width);
    } else {
        SkPMColor*  span = fBuffer;
        fShaderContext->shadeSpan(x, y, span, width);
        fProc32(device, span, width, 255);
    }
}

void SkARGB32_Shader_Blitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(x >= 0 && y >= 0 &&
             x + width <= fDevice.width() && y + height <= fDevice.height());

    uint32_t*  device = fDevice.writable_addr32(x, y);
    const size_t deviceRB = fDevice.rowBytes();
    auto*      shaderContext = fShaderContext;
    SkPMColor* span = fBuffer;

    if (fShadeDirectlyIntoDevice) {
        do {
            shaderContext->shadeSpan(x, y, device, width);
            y += 1;
            device = (uint32_t*)((char*)device + deviceRB);
        } while (--height > 0);
    } else {
        SkBlitRow::Proc32 proc = fProc32;
        do {
            shaderContext->shadeSpan(x, y, span, width);
            proc(device, span, width, 255);
            y += 1;
            device = (uint32_t*)((char*)device + deviceRB);
        } while (--height > 0);
    }
}

void SkARGB32_Shader_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                        const int16_t runs[]) {
    SkPMColor* span = fBuffer;
    uint32_t*  device = fDevice.writable_addr32(x, y);
    auto*      shaderContext = fShaderContext;

    if (fShadeDirectlyIntoDevice || (shaderContext->getFlags() & SkShaderBase::kOpaqueAlpha_Flag)) {
        for (;;) {
            int count = *runs;
            if (count <= 0) {
                break;
            }
            int aa = *antialias;
            if (aa) {
                if (aa == 255) {
                    // cool, have the shader draw right into the device
                    shaderContext->shadeSpan(x, y, device, count);
                } else {
                    shaderContext->shadeSpan(x, y, span, count);
                    fProc32Blend(device, span, count, aa);
                }
            }
            device += count;
            runs += count;
            antialias += count;
            x += count;
        }
    } else {
        for (;;) {
            int count = *runs;
            if (count <= 0) {
                break;
            }
            int aa = *antialias;
            if (aa) {
                shaderContext->shadeSpan(x, y, span, count);
                if (aa == 255) {
                    fProc32(device, span, count, 255);
                } else {
                    fProc32Blend(device, span, count, aa);
                }
            }
            device += count;
            runs += count;
            antialias += count;
            x += count;
        }
    }
}

using U32  = skvx::Vec< 4, uint32_t>;
using U8x4 = skvx::Vec<16, uint8_t>;
using U8   = skvx::Vec< 4, uint8_t>;

static void drive(SkPMColor* dst, const SkPMColor* src, const uint8_t* cov, int n,
                  U8x4 (*kernel)(U8x4,U8x4,U8x4)) {

    auto apply = [kernel](U32 dst, U32 src, U8 cov) -> U32 {
        U8x4 cov_splat = skvx::shuffle<0,0,0,0, 1,1,1,1, 2,2,2,2, 3,3,3,3>(cov);
        return sk_bit_cast<U32>(kernel(sk_bit_cast<U8x4>(dst),
                                       sk_bit_cast<U8x4>(src),
                                       cov_splat));
    };
    while (n >= 4) {
        apply(U32::Load(dst), U32::Load(src), U8::Load(cov)).store(dst);
        dst += 4;
        src += 4;
        cov += 4;
        n   -= 4;
    }
    while (n --> 0) {
        *dst = apply(U32{*dst}, U32{*src}, U8{*cov})[0];
        dst++;
        src++;
        cov++;
    }
}

static void blend_row_A8(SkPMColor* dst, const void* mask, const SkPMColor* src, int n) {
    auto cov = (const uint8_t*)mask;
    drive(dst, src, cov, n, [](U8x4 d, U8x4 s, U8x4 c) {
        U8x4 s_aa  = skvx::approx_scale(s, c),
             alpha = skvx::shuffle<3,3,3,3, 7,7,7,7, 11,11,11,11, 15,15,15,15>(s_aa);
        return s_aa + skvx::approx_scale(d, 255 - alpha);
    });
}

static void blend_row_A8_opaque(SkPMColor* dst, const void* mask, const SkPMColor* src, int n) {
    auto cov = (const uint8_t*)mask;
    drive(dst, src, cov, n, [](U8x4 d, U8x4 s, U8x4 c) {
        return skvx::div255( skvx::cast<uint16_t>(s) * skvx::cast<uint16_t>(  c  )
                           + skvx::cast<uint16_t>(d) * skvx::cast<uint16_t>(255-c));
    });
}

static void blend_row_lcd32(SkPMColor* dst, const void* vmask, const SkPMColor* src, int n) {
    auto src_alpha_blend = [](int s, int d, int sa, int m) {
        return d + SkAlphaMul(s - SkAlphaMul(sa, d), m);
    };

    auto mask = (const uint32_t*)vmask;
    for (int i = 0; i < n; ++i) {
        uint32_t m = mask[i];
        int maskR = (m >> SK_RGBA_R32_SHIFT) & 0xFF;
        int maskG = (m >> SK_RGBA_G32_SHIFT) & 0xFF;
        int maskB = (m >> SK_RGBA_B32_SHIFT) & 0xFF;
        if ((maskR | maskG | maskB) == 0) {
            continue;
        }

        SkPMColor s = src[i];
        SkPMColor d = dst[i];

        int srcA = SkGetPackedA32(s);
        int srcR = SkGetPackedR32(s);
        int srcG = SkGetPackedG32(s);
        int srcB = SkGetPackedB32(s);

        srcA += srcA >> 7;

        // This LCD blit routine only works if the destination is opaque.
        dst[i] = SkPackARGB32(0xFF,
                              src_alpha_blend(srcR, SkGetPackedR32(d), srcA, maskR),
                              src_alpha_blend(srcG, SkGetPackedG32(d), srcA, maskG),
                              src_alpha_blend(srcB, SkGetPackedB32(d), srcA, maskB));
    }
}

static void blend_row_LCD32_opaque(SkPMColor* dst, const void* vmask, const SkPMColor* src, int n) {
    auto mask = (const uint32_t*)vmask;

    for (int i = 0; i < n; ++i) {
        uint32_t m = mask[i];
        int maskR = (m >> SK_RGBA_R32_SHIFT) & 0xFF;
        int maskG = (m >> SK_RGBA_G32_SHIFT) & 0xFF;
        int maskB = (m >> SK_RGBA_B32_SHIFT) & 0xFF;
        if ((maskR | maskG | maskB) == 0) {
            continue;
        }

        SkPMColor s = src[i];
        SkPMColor d = dst[i];

        int srcR = SkGetPackedR32(s);
        int srcG = SkGetPackedG32(s);
        int srcB = SkGetPackedB32(s);

        // This LCD blit routine only works if the destination is opaque.
        dst[i] = SkPackARGB32(0xFF,
                              blend_255(srcR, SkGetPackedR32(d), maskR),
                              blend_255(srcG, SkGetPackedG32(d), maskG),
                              blend_255(srcB, SkGetPackedB32(d), maskB));
    }
}

void SkARGB32_Shader_Blitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));

    void (*blend_row)(SkPMColor*, const void* mask, const SkPMColor*, int) = nullptr;

    bool opaque = (fShaderContext->getFlags() & SkShaderBase::kOpaqueAlpha_Flag);

    if (mask.fFormat == SkMask::kA8_Format && opaque) {
        blend_row = blend_row_A8_opaque;
    } else if (mask.fFormat == SkMask::kA8_Format) {
        blend_row = blend_row_A8;
    } else if (mask.fFormat == SkMask::kLCD32_Format && opaque) {
        blend_row = blend_row_LCD32_opaque;
    } else if (mask.fFormat == SkMask::kLCD32_Format) {
        blend_row = blend_row_lcd32;
    } else {
        this->SkShaderBlitter::blitMask(mask, clip);
        return;
    }

    const int x = clip.fLeft;
    const int width = clip.width();
    int y = clip.fTop;
    int height = clip.height();

    char* dstRow = (char*)fDevice.writable_addr32(x, y);
    const size_t dstRB = fDevice.rowBytes();
    const uint8_t* maskRow = (const uint8_t*)mask.getAddr(x, y);
    const size_t maskRB = mask.fRowBytes;

    SkPMColor* span = fBuffer;
    SkASSERT(blend_row);
    do {
        fShaderContext->shadeSpan(x, y, span, width);
        blend_row(reinterpret_cast<SkPMColor*>(dstRow), maskRow, span, width);
        dstRow += dstRB;
        maskRow += maskRB;
        y += 1;
    } while (--height > 0);
}

void SkARGB32_Shader_Blitter::blitV(int x, int y, int height, SkAlpha alpha) {
    SkASSERT(x >= 0 && y >= 0 && y + height <= fDevice.height());

    uint32_t* device = fDevice.writable_addr32(x, y);
    const size_t deviceRB = fDevice.rowBytes();

    if (fShadeDirectlyIntoDevice) {
        if (255 == alpha) {
            do {
                fShaderContext->shadeSpan(x, y, device, 1);
                y += 1;
                device = (uint32_t*)((char*)device + deviceRB);
            } while (--height > 0);
        } else {
            do {
                SkPMColor c;
                fShaderContext->shadeSpan(x, y, &c, 1);
                *device = SkFourByteInterp(c, *device, alpha);
                y += 1;
                device = (uint32_t*)((char*)device + deviceRB);
            } while (--height > 0);
        }
    } else {
        SkPMColor* span = fBuffer;
        SkBlitRow::Proc32 proc = (255 == alpha) ? fProc32 : fProc32Blend;
        do {
            fShaderContext->shadeSpan(x, y, span, 1);
            proc(device, span, 1, alpha);
            y += 1;
            device = (uint32_t*)((char*)device + deviceRB);
        } while (--height > 0);
    }
}
