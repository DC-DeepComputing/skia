/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkMipmap.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#endif

#include <cstdint>
#include <cstring>

// kBGR_888x_SkColorType is to kBGRA_8888_SkColorType what kRGB_888x_SkColorType is to
// kRGBA_8888_SkColorType: the pixel layout matches, but the 4th byte is ignored and alpha is
// always treated as 1.

namespace {

constexpr int kW = 16;
constexpr int kH = 16;

// Stores the given colors into memory using the BGR_888x layout, putting a junk value in the
// ignored byte that should never be observed.
void fill_bgrx(SkBitmap* bm, uint8_t junk) {
    for (int y = 0; y < bm->height(); ++y) {
        for (int x = 0; x < bm->width(); ++x) {
            uint8_t* p = static_cast<uint8_t*>(bm->getAddr(x, y));
            p[0] = static_cast<uint8_t>(x * 16);        // B
            p[1] = static_cast<uint8_t>(y * 16);        // G
            p[2] = static_cast<uint8_t>(255 - x * 8);   // R
            p[3] = static_cast<uint8_t>(junk + x + y);  // x (ignored)
        }
    }
}

SkColor expected_color(int x, int y) { return SkColorSetARGB(0xFF, 255 - x * 8, y * 16, x * 16); }

SkBitmap make_bgrx_bitmap(uint8_t junk) {
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::Make(kW, kH, kBGR_888x_SkColorType, kOpaque_SkAlphaType));
    fill_bgrx(&bm, junk);
    bm.setImmutable();
    return bm;
}

// Verifies that `pm` (any color type) contains exactly the colors written by fill_bgrx().
void check_colors(skiatest::Reporter* r, const SkPixmap& pm, const char* label) {
    for (int y = 0; y < pm.height(); ++y) {
        for (int x = 0; x < pm.width(); ++x) {
            SkColor actual = pm.getColor(x, y);
            SkColor expected = expected_color(x, y);
            if (actual != expected) {
                ERRORF(r, "%s: (%d, %d) expected 0x%08x got 0x%08x", label, x, y, expected, actual);
                return;
            }
        }
    }
}

}  // namespace

DEF_TEST(BGR888x_Basics, r) {
    REPORTER_ASSERT(r, SkColorTypeBytesPerPixel(kBGR_888x_SkColorType) == 4);
    REPORTER_ASSERT(r, SkColorTypeIsAlwaysOpaque(kBGR_888x_SkColorType));

    SkAlphaType canonical;
    REPORTER_ASSERT(
            r,
            SkColorTypeValidateAlphaType(kBGR_888x_SkColorType, kPremul_SkAlphaType, &canonical));
    REPORTER_ASSERT(r, canonical == kOpaque_SkAlphaType);
}

DEF_TEST(BGR888x_MemoryLayout, r) {
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::Make(1, 1, kBGR_888x_SkColorType, kOpaque_SkAlphaType));

    // Storing a color writes B, G, R, 0xFF.
    bm.eraseColor(SkColorSetARGB(0xFF, 0x11, 0x22, 0x33));
    const uint8_t* p = static_cast<const uint8_t*>(bm.getPixels());
    REPORTER_ASSERT(r,
                    p[0] == 0x33 && p[1] == 0x22 && p[2] == 0x11 && p[3] == 0xFF,
                    "got %02x %02x %02x %02x",
                    p[0],
                    p[1],
                    p[2],
                    p[3]);

    // Storing a translucent color forces alpha to opaque.
    bm.eraseColor(SkColorSetARGB(0x00, 0x11, 0x22, 0x33));
    REPORTER_ASSERT(r, p[3] == 0xFF);

    // The 4th byte must be ignored when reading.
    uint8_t* wp = static_cast<uint8_t*>(bm.getPixels());
    wp[0] = 0x33;
    wp[1] = 0x22;
    wp[2] = 0x11;
    wp[3] = 0x00;
    REPORTER_ASSERT(r,
                    bm.getColor(0, 0) == SkColorSetARGB(0xFF, 0x11, 0x22, 0x33),
                    "got 0x%08x",
                    bm.getColor(0, 0));
    REPORTER_ASSERT(r, bm.getAlphaf(0, 0) == 1.f);
    SkColor4f c4 = bm.getColor4f(0, 0);
    REPORTER_ASSERT(r, c4 == SkColor4f::FromColor(SkColorSetARGB(0xFF, 0x11, 0x22, 0x33)));
    REPORTER_ASSERT(r, SkBitmap::ComputeIsOpaque(bm));
}

DEF_TEST(BGR888x_ConvertPixels, r) {
    for (uint8_t junk : {0x00, 0x80, 0xFF}) {
        SkBitmap src = make_bgrx_bitmap(junk);
        check_colors(r, src.pixmap(), "src");

        struct {
            SkColorType ct;
            SkAlphaType at;
        } dsts[] = {
                {kRGBA_8888_SkColorType, kPremul_SkAlphaType},
                {kRGBA_8888_SkColorType, kUnpremul_SkAlphaType},
                {kBGRA_8888_SkColorType, kPremul_SkAlphaType},
                {kBGRA_8888_SkColorType, kUnpremul_SkAlphaType},
                {kRGB_888x_SkColorType, kOpaque_SkAlphaType},
                {kBGR_888x_SkColorType, kOpaque_SkAlphaType},
                {kRGBA_F16_SkColorType, kPremul_SkAlphaType},
                {kRGBA_F32_SkColorType, kUnpremul_SkAlphaType},
        };
        for (auto d : dsts) {
            skiatest::ReporterContext ctx(
                    r, SkStringPrintf("junk=%d dst=%s", junk, ToolUtils::colortype_name(d.ct)));
            SkBitmap dst;
            dst.allocPixels(SkImageInfo::Make(kW, kH, d.ct, d.at));
            REPORTER_ASSERT(r, src.readPixels(dst.pixmap()));
            check_colors(r, dst.pixmap(), "readPixels");
            if (SkColorTypeBytesPerPixel(d.ct) == 4 && !SkColorTypeIsAlwaysOpaque(d.ct)) {
                // The junk byte must never leak into an alpha channel.
                for (int y = 0; y < kH; ++y) {
                    for (int x = 0; x < kW; ++x) {
                        REPORTER_ASSERT(r,
                                        static_cast<const uint8_t*>(dst.getAddr(x, y))[3] == 0xFF);
                    }
                }
            }

            // And convert back into BGR_888x.
            SkBitmap roundTrip;
            roundTrip.allocPixels(src.info());
            REPORTER_ASSERT(r, dst.readPixels(roundTrip.pixmap()));
            check_colors(r, roundTrip.pixmap(), "roundTrip");
        }
    }

    // RGB_888x and BGR_888x are R/B swaps of each other.
    SkBitmap src = make_bgrx_bitmap(0x42);
    SkBitmap rgbx;
    rgbx.allocPixels(src.info().makeColorType(kRGB_888x_SkColorType));
    REPORTER_ASSERT(r, src.readPixels(rgbx.pixmap()));
    for (int y = 0; y < kH; ++y) {
        for (int x = 0; x < kW; ++x) {
            const uint8_t* s = static_cast<const uint8_t*>(src.getAddr(x, y));
            const uint8_t* d = static_cast<const uint8_t*>(rgbx.getAddr(x, y));
            REPORTER_ASSERT(r, s[0] == d[2] && s[1] == d[1] && s[2] == d[0]);
        }
    }
}

// Drawing a BGR_888x image must produce opaque results regardless of the value of the ignored
// byte, for all sampling modes (which use different raster pipeline stages).
DEF_TEST(BGR888x_DrawImage, r) {
    SkBitmap src = make_bgrx_bitmap(0x00);  // The ignored byte is fully "transparent".
    sk_sp<SkImage> image = src.asImage();
    REPORTER_ASSERT(r, image->isOpaque());

    const SkSamplingOptions samplings[] = {
            SkSamplingOptions(SkFilterMode::kNearest),
            SkSamplingOptions(SkFilterMode::kLinear),
            SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNearest),
            SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear),
            SkSamplingOptions(SkCubicResampler::Mitchell()),
    };
    for (const SkSamplingOptions& sampling : samplings) {
        for (SkColorType dstCT : {kRGBA_8888_SkColorType,
                                  kBGRA_8888_SkColorType,
                                  kBGR_888x_SkColorType,
                                  kRGB_888x_SkColorType}) {
            auto surface = SkSurfaces::Raster(SkImageInfo::Make(
                    kW,
                    kH,
                    dstCT,
                    SkColorTypeIsAlwaysOpaque(dstCT) ? kOpaque_SkAlphaType : kPremul_SkAlphaType));
            REPORTER_ASSERT(r, surface);
            surface->getCanvas()->clear(SK_ColorTRANSPARENT);
            // Draw at identity so every sampling mode produces exactly the source colors.
            surface->getCanvas()->drawImage(image, 0, 0, sampling);

            SkBitmap result;
            result.allocPixels(
                    SkImageInfo::Make(kW, kH, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType));
            REPORTER_ASSERT(r, surface->readPixels(result.pixmap(), 0, 0));
            for (int y = 0; y < kH; ++y) {
                for (int x = 0; x < kW; ++x) {
                    SkColor c = result.getColor(x, y);
                    if (SkColorGetA(c) != 0xFF) {
                        ERRORF(r,
                               "dst %s: non-opaque pixel 0x%08x at (%d, %d)",
                               ToolUtils::colortype_name(dstCT),
                               c,
                               x,
                               y);
                        break;
                    }
                }
            }
            if (!sampling.useCubic) {
                check_colors(r, result.pixmap(), ToolUtils::colortype_name(dstCT));
            }
        }

        // Drawing scaled down exercises mipmaps (when requested).
        auto surface = SkSurfaces::Raster(
                SkImageInfo::Make(kW / 4, kH / 4, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
        surface->getCanvas()->clear(SK_ColorTRANSPARENT);
        surface->getCanvas()->scale(0.25f, 0.25f);
        surface->getCanvas()->drawImage(image, 0, 0, sampling);
        SkBitmap result;
        result.allocPixels(surface->imageInfo());
        REPORTER_ASSERT(r, surface->readPixels(result.pixmap(), 0, 0));
        for (int y = 0; y < result.height(); ++y) {
            for (int x = 0; x < result.width(); ++x) {
                REPORTER_ASSERT(r, SkColorGetA(result.getColor(x, y)) == 0xFF);
            }
        }
    }
}

// The HQ mipmap downsampler used to refuse to build mipmaps for the 888x and 101010x color types.
DEF_TEST(BGR888x_Mipmaps, r) {
    for (SkColorType ct : {kRGB_888x_SkColorType,
                           kBGR_888x_SkColorType,
                           kRGB_101010x_SkColorType,
                           kBGR_101010x_SkColorType}) {
        skiatest::ReporterContext ctx(r, ToolUtils::colortype_name(ct));
        SkBitmap bm;
        bm.allocPixels(SkImageInfo::Make(kW, kH, ct, kOpaque_SkAlphaType));
        bm.eraseColor(SkColorSetARGB(0xFF, 0x40, 0x80, 0xC0));
        // Junk in the ignored channel should not affect the result.
        if (ct == kRGB_888x_SkColorType || ct == kBGR_888x_SkColorType) {
            for (int y = 0; y < kH; ++y) {
                for (int x = 0; x < kW; ++x) {
                    static_cast<uint8_t*>(bm.getAddr(x, y))[3] = static_cast<uint8_t>(x * y);
                }
            }
        }

        sk_sp<SkMipmap> mm(SkMipmap::Build(bm.pixmap(), nullptr));
        REPORTER_ASSERT(r, mm);
        if (!mm) {
            continue;
        }
        REPORTER_ASSERT(r, mm->countLevels() > 0);
        for (int i = 0; i < mm->countLevels(); ++i) {
            SkMipmap::Level level;
            REPORTER_ASSERT(r, mm->getLevel(i, &level));
            REPORTER_ASSERT(r, level.fPixmap.colorType() == ct);
            SkColor c = level.fPixmap.getColor(0, 0);
            REPORTER_ASSERT(r,
                            c == SkColorSetARGB(0xFF, 0x40, 0x80, 0xC0) ||
                                    // The 10-bit formats round-trip through 10 bits.
                                    (ct != kRGB_888x_SkColorType && ct != kBGR_888x_SkColorType),
                            "level %d got 0x%08x",
                            i,
                            c);
            REPORTER_ASSERT(r, SkColorGetA(c) == 0xFF);
        }
    }
}

// Encoding a BGR_888x image must produce the same result as encoding the equivalent RGB_888x
// image, and must ignore the 4th byte.
DEF_TEST(BGR888x_Encode, r) {
    SkBitmap bgrx = make_bgrx_bitmap(0x00);
    SkBitmap rgbx;
    rgbx.allocPixels(bgrx.info().makeColorType(kRGB_888x_SkColorType));
    REPORTER_ASSERT(r, bgrx.readPixels(rgbx.pixmap()));

    auto encode = [](SkEncodedImageFormat format, const SkPixmap& pm) -> sk_sp<SkData> {
        SkDynamicMemoryWStream stream;
        bool ok = false;
        switch (format) {
            case SkEncodedImageFormat::kPNG:
                ok = SkPngEncoder::Encode(&stream, pm, {});
                break;
            case SkEncodedImageFormat::kJPEG:
                ok = SkJpegEncoder::Encode(&stream, pm, {});
                break;
            case SkEncodedImageFormat::kWEBP: {
                SkWebpEncoder::Options opts;
                opts.fCompression = SkWebpEncoder::Compression::kLossless;
                ok = SkWebpEncoder::Encode(&stream, pm, opts);
                break;
            }
            default:
                break;
        }
        return ok ? stream.detachAsData() : nullptr;
    };

    for (auto format :
         {SkEncodedImageFormat::kPNG, SkEncodedImageFormat::kJPEG, SkEncodedImageFormat::kWEBP}) {
        skiatest::ReporterContext ctx(r, SkStringPrintf("format %d", (int)format));
        sk_sp<SkData> fromBGRX = encode(format, bgrx.pixmap());
        sk_sp<SkData> fromRGBX = encode(format, rgbx.pixmap());
        if (!fromRGBX) {
            // This encoder isn't available in this build.
            continue;
        }
        REPORTER_ASSERT(r, fromBGRX);
        if (!fromBGRX) {
            continue;
        }

        SkBitmap decodedBGRX, decodedRGBX;
        auto decode = [&](sk_sp<SkData> data, SkBitmap* dst) {
            std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(std::move(data));
            if (!codec) {
                return false;
            }
            dst->allocPixels(codec->getInfo()
                                     .makeColorType(kRGBA_8888_SkColorType)
                                     .makeAlphaType(kUnpremul_SkAlphaType));
            return SkCodec::kSuccess == codec->getPixels(dst->pixmap());
        };
        if (!decode(fromBGRX, &decodedBGRX) || !decode(fromRGBX, &decodedRGBX)) {
            // A decoder for this format isn't available in this build.
            continue;
        }
        REPORTER_ASSERT(r, decodedBGRX.dimensions() == decodedRGBX.dimensions());
        for (int y = 0; y < decodedBGRX.height(); ++y) {
            for (int x = 0; x < decodedBGRX.width(); ++x) {
                SkColor a = decodedBGRX.getColor(x, y);
                SkColor b = decodedRGBX.getColor(x, y);
                REPORTER_ASSERT(r, a == b, "(%d, %d) 0x%08x vs 0x%08x", x, y, a, b);
                REPORTER_ASSERT(r, SkColorGetA(a) == 0xFF);
            }
        }
        if (format != SkEncodedImageFormat::kJPEG) {
            // Lossless formats must exactly match the source.
            check_colors(r, decodedBGRX.pixmap(), "decoded");
        }
    }
}

// Not BGR_888x-specific, but SkPixmap::getColor4f() used to swap R and B for RGBA_1010102.
DEF_TEST(Pixmap_getColor4f_1010102, r) {
    for (SkColorType ct : {kRGBA_1010102_SkColorType,
                           kBGRA_1010102_SkColorType,
                           kRGB_101010x_SkColorType,
                           kBGR_101010x_SkColorType}) {
        skiatest::ReporterContext ctx(r, ToolUtils::colortype_name(ct));
        SkBitmap bm;
        bm.allocPixels(SkImageInfo::Make(
                1,
                1,
                ct,
                SkColorTypeIsAlwaysOpaque(ct) ? kOpaque_SkAlphaType : kUnpremul_SkAlphaType));
        bm.eraseColor(SkColors::kRed);
        SkColor4f c = bm.getColor4f(0, 0);
        REPORTER_ASSERT(r, c == SkColors::kRed, "got %f %f %f %f", c.fR, c.fG, c.fB, c.fA);
        REPORTER_ASSERT(r, bm.getColor(0, 0) == SK_ColorRED);
    }
}

namespace {

// Shared logic for the GPU tests: upload a BGR_888x image (with a "transparent" junk byte),
// draw it to an RGBA surface, and make sure that the result is opaque and correct. Also read the
// GPU-backed image back into BGR_888x and RGBA_8888.
void check_gpu_image(skiatest::Reporter* r,
                     sk_sp<SkImage> gpuImage,
                     SkSurface* surface,
                     const char* label) {
    skiatest::ReporterContext ctx(r, label);
    if (!gpuImage) {
        // Not all backends support uploading BGR_888x textures.
        return;
    }
    REPORTER_ASSERT(r,
                    gpuImage->colorType() == kBGR_888x_SkColorType ||
                            // Ganesh may fall back to a different color type for the texture.
                            gpuImage->colorType() == kRGB_888x_SkColorType,
                    "%s",
                    ToolUtils::colortype_name(gpuImage->colorType()));
    REPORTER_ASSERT(r, gpuImage->isOpaque());

    surface->getCanvas()->clear(SK_ColorTRANSPARENT);
    surface->getCanvas()->drawImage(gpuImage, 0, 0);

    SkBitmap result;
    result.allocPixels(SkImageInfo::Make(kW, kH, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType));
    if (surface->readPixels(result.pixmap(), 0, 0)) {
        check_colors(r, result.pixmap(), "draw");
    } else {
        ERRORF(r, "Could not read back surface");
    }
}

}  // namespace

#if defined(SK_GANESH)
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(BGR888x_Ganesh, r, ctxInfo, CtsEnforcement::kNever) {
    GrDirectContext* dContext = ctxInfo.directContext();
    SkBitmap src = make_bgrx_bitmap(0x00);

    auto surface = SkSurfaces::RenderTarget(
            dContext,
            skgpu::Budgeted::kNo,
            SkImageInfo::Make(kW, kH, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    REPORTER_ASSERT(r, surface);
    if (!surface) {
        return;
    }

    bool supported = dContext->colorTypeSupportedAsImage(kBGR_888x_SkColorType);
    sk_sp<SkImage> gpuImage = SkImages::TextureFromImage(dContext, src.asImage());
    REPORTER_ASSERT(r, !supported || gpuImage);
    check_gpu_image(r, gpuImage, surface.get(), "upload");

    if (gpuImage) {
        for (SkColorType ct : {kBGR_888x_SkColorType,
                               kRGBA_8888_SkColorType,
                               kBGRA_8888_SkColorType,
                               kRGB_888x_SkColorType}) {
            SkBitmap readback;
            readback.allocPixels(SkImageInfo::Make(
                    kW,
                    kH,
                    ct,
                    SkColorTypeIsAlwaysOpaque(ct) ? kOpaque_SkAlphaType : kPremul_SkAlphaType));
            if (gpuImage->readPixels(dContext, readback.pixmap(), 0, 0)) {
                check_colors(r, readback.pixmap(), ToolUtils::colortype_name(ct));
            } else {
                ERRORF(r,
                       "Could not read back BGR_888x image into %s",
                       ToolUtils::colortype_name(ct));
            }
        }
    }
}
#endif

#if defined(SK_GRAPHITE)
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(BGR888x_Graphite, r, context, CtsEnforcement::kNever) {
    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();
    SkBitmap src = make_bgrx_bitmap(0x00);

    auto surface = SkSurfaces::RenderTarget(
            recorder.get(), SkImageInfo::Make(kW, kH, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    REPORTER_ASSERT(r, surface);
    if (!surface) {
        return;
    }

    sk_sp<SkImage> gpuImage = SkImages::TextureFromImage(recorder.get(), src.asImage());
    REPORTER_ASSERT(r, gpuImage);
    check_gpu_image(r, gpuImage, surface.get(), "upload");
}
#endif
