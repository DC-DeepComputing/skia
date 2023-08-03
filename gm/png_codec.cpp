/*
 * Copyright 2023 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/codec/SkPngDecoder.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkAutoMalloc.h"
#include "src/core/SkOpts.h"
#include "src/utils/SkOSPath.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/flags/CommonFlags.h"

#include <memory>
#include <string>
#include <vector>

struct DecodeResult {
    std::unique_ptr<SkCodec> codec;
    std::string errorMsg;
};

static DecodeResult decode(std::string path) {
    sk_sp<SkData> encoded(SkData::MakeFromFileName(path.c_str()));
    if (!encoded) {
        return {.errorMsg = SkStringPrintf("Could not read \"%s\".\n", path.c_str()).c_str()};
    }
    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec = SkPngDecoder::Decode(SkMemoryStream::Make(encoded), &result);
    if (result != SkCodec::Result::kSuccess) {
        return {.errorMsg = SkStringPrintf("Could not create codec for \"%s\": %s.",
                                           path.c_str(),
                                           SkCodec::ResultToString(result))
                                    .c_str()};
    }
    return {.codec = std::move(codec)};
}

// This GM implements the PNG-related behaviors found in DM's CodecSrc class. It takes a single
// image as an argument and applies the same logic as CodecSrc.
//
// See the CodecSrc class here:
// https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.h#158.
class PNGCodecGM : public skiagm::GM {
public:
    // Based on CodecSrc::Mode.
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.h#160
    enum class Mode {
        kCodec,
        kCodecZeroInit,
        kScanline,
    };

    // Based on CodecSrc::DstColorType.
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.h#172
    enum class DstColorType {
        kGetFromCanvas,
        kGrayscale_Always,
        kNonNative8888_Always,
    };

    // Based on DM's CodecSrc::CodecSrc().
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#371
    PNGCodecGM(std::string path, Mode mode, DstColorType dstColorType, SkAlphaType dstAlphaType)
            : skiagm::GM()
            , fPath(path)
            , fMode(mode)
            , fDstColorType(dstColorType)
            , fDstAlphaType(dstAlphaType) {}

    bool isBazelOnly() const override {
        // This GM class overlaps with DM's CodecSrc and related sources.
        return true;
    }

    // Based on DM's push_codec_src() function.
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DM.cpp#555
    SkString getOutputFolder() const override {
        SkString folder;
        switch (fMode) {
            case Mode::kCodec:
                folder.append("codec");
                break;
            case Mode::kCodecZeroInit:
                folder.append("codec_zero_init");
                break;
            case Mode::kScanline:
                folder.append("scanline");
                break;
        }

        switch (fDstColorType) {
            case DstColorType::kGrayscale_Always:
                folder.append("_kGray8");
                break;
            case DstColorType::kNonNative8888_Always:
                folder.append("_kNonNative");
                break;
            default:
                break;
        }

        switch (fDstAlphaType) {
            case kPremul_SkAlphaType:
                folder.append("_premul");
                break;
            case kUnpremul_SkAlphaType:
                folder.append("_unpremul");
                break;
            default:
                break;
        }

        return folder;
    }

protected:
    // Based on CodecSrc::name().
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#828
    SkString onShortName() override {
        SkString name = SkOSPath::Basename(fPath.c_str());
        return name;
    }

    // Based on CodecSrc::size().
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#803
    SkISize onISize() override {
        DecodeResult decodeResult = decode(fPath);
        if (decodeResult.errorMsg != "") {
            return {0, 0};
        }
        return decodeResult.codec->dimensions();
    }

    // Based on CodecSrc::draw().
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#450
    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        DecodeResult decodeResult = decode(fPath);
        if (decodeResult.errorMsg != "") {
            *errorMsg = decodeResult.errorMsg.c_str();
            return DrawResult::kFail;
        }
        std::unique_ptr<SkCodec> codec = std::move(decodeResult.codec);

        SkImageInfo decodeInfo = codec->getInfo();
        if (!getDecodeInfo(&decodeInfo, canvas->imageInfo().colorType())) {
            *errorMsg = "Skipping uninteresting test.";
            return DrawResult::kSkip;
        }

        SkISize size = codec->dimensions();
        decodeInfo = decodeInfo.makeDimensions(size);

        const int bpp = decodeInfo.bytesPerPixel();
        const size_t rowBytes = size.width() * bpp;
        const size_t safeSize = decodeInfo.computeByteSize(rowBytes);
        SkAutoMalloc pixels(safeSize);

        SkCodec::Options options;
        if (Mode::kCodecZeroInit == fMode) {
            memset(pixels.get(), 0, size.height() * rowBytes);
            options.fZeroInitialized = SkCodec::kYes_ZeroInitialized;
        }

        // For codec srcs, we want the "draw" step to be a memcpy.  Any interesting color space or
        // color format conversions should be performed by the codec.  Sometimes the output of the
        // decode will be in an interesting color space.  On our srgb and f16 backends, we need to
        // "pretend" that the color space is standard sRGB to avoid triggering color conversion
        // at draw time.
        SkImageInfo bitmapInfo = decodeInfo.makeColorSpace(SkColorSpace::MakeSRGB());

        if (kRGBA_8888_SkColorType == decodeInfo.colorType() ||
            kBGRA_8888_SkColorType == decodeInfo.colorType()) {
            bitmapInfo = bitmapInfo.makeColorType(kN32_SkColorType);
        }

        switch (fMode) {
            case Mode::kCodecZeroInit:
            case Mode::kCodec: {
                switch (codec->getPixels(decodeInfo, pixels.get(), rowBytes, &options)) {
                    case SkCodec::kSuccess:
                        // We consider these to be valid, since we should still decode what is
                        // available.
                    case SkCodec::kErrorInInput:
                    case SkCodec::kIncompleteInput:
                        break;
                    default:
                        // Everything else is considered a failure.
                        *errorMsg = SkStringPrintf("Couldn't getPixels %s.", fPath.c_str());
                        return DrawResult::kFail;
                }

                drawToCanvas(canvas, bitmapInfo, pixels.get(), rowBytes);
                break;
            }
            case Mode::kScanline: {
                void* dst = pixels.get();
                uint32_t height = decodeInfo.height();
                if (SkCodec::kSuccess ==
                    codec->startIncrementalDecode(decodeInfo, dst, rowBytes, &options)) {
                    int rowsDecoded;
                    auto result = codec->incrementalDecode(&rowsDecoded);
                    if (SkCodec::kIncompleteInput == result || SkCodec::kErrorInInput == result) {
                        codec->fillIncompleteImage(decodeInfo,
                                                   dst,
                                                   rowBytes,
                                                   SkCodec::kNo_ZeroInitialized,
                                                   height,
                                                   rowsDecoded);
                    }
                } else {
                    *errorMsg = "Could not start incremental decode";
                    return DrawResult::kFail;
                }
                drawToCanvas(canvas, bitmapInfo, dst, rowBytes);
                break;
            }
            default:
                SkASSERT(false);
                *errorMsg = "Invalid fMode";
                return DrawResult::kFail;
        }
        return DrawResult::kOk;
    }

private:
    // Based on DM's get_decode_info() function.
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#398
    bool getDecodeInfo(SkImageInfo* decodeInfo, SkColorType canvasColorType) {
        switch (fDstColorType) {
            case DstColorType::kGrayscale_Always:
                if (kRGB_565_SkColorType == canvasColorType) {
                    return false;
                }
                *decodeInfo = decodeInfo->makeColorType(kGray_8_SkColorType);
                break;
            case DstColorType::kNonNative8888_Always:
                if (kRGB_565_SkColorType == canvasColorType ||
                    kRGBA_F16_SkColorType == canvasColorType) {
                    return false;
                }
#ifdef SK_PMCOLOR_IS_RGBA
                *decodeInfo = decodeInfo->makeColorType(kBGRA_8888_SkColorType);
#else
                *decodeInfo = decodeInfo->makeColorType(kRGBA_8888_SkColorType);
#endif
                break;
            default:
                if (kRGB_565_SkColorType == canvasColorType &&
                    kOpaque_SkAlphaType != decodeInfo->alphaType()) {
                    return false;
                }

                *decodeInfo = decodeInfo->makeColorType(canvasColorType);
                break;
        }

        *decodeInfo = decodeInfo->makeAlphaType(fDstAlphaType);
        return true;
    }

    // Based on DM's draw_to_canvas() function.
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#432
    void drawToCanvas(SkCanvas* canvas,
                      const SkImageInfo& info,
                      void* pixels,
                      size_t rowBytes,
                      SkScalar left = 0,
                      SkScalar top = 0) {
        SkBitmap bitmap;
        bitmap.installPixels(info, pixels, rowBytes);
        swapRbIfNecessary(bitmap);
        canvas->drawImage(bitmap.asImage(), left, top);
    }

    // Allows us to test decodes to non-native 8888.
    //
    // Based on DM's swap_rb_if_necessary function.
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#387
    void swapRbIfNecessary(SkBitmap& bitmap) {
        if (DstColorType::kNonNative8888_Always != fDstColorType) {
            return;
        }

        for (int y = 0; y < bitmap.height(); y++) {
            uint32_t* row = (uint32_t*)bitmap.getAddr(0, y);
            SkOpts::RGBA_to_BGRA(row, row, bitmap.width());
        }
    }

    std::string fPath;
    Mode fMode;
    DstColorType fDstColorType;
    SkAlphaType fDstAlphaType;
};

// Populates the vector of GMs with zero or more PNGCodecGM instances for the given image. Returns a
// non-empty, human-friendly error message in the case of errors.
//
// Based on DM's push_codec_srcs() function. It only covers "simple" codecs (lines 740-834).
// https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DM.cpp#740
//
// Specifically, this function does not capture any behaviors found in the following DM classes:
//
//  - AndroidCodecSrc
//  - BRDSrc
//  - ImageGenSrc
//
// We should be able to add those behaviors later if necessary.
static std::string registerGMsForImage(std::string path) {
    DecodeResult decodeResult = decode(path);
    if (decodeResult.errorMsg != "") {
        return decodeResult.errorMsg;
    }

    std::vector<PNGCodecGM::Mode> nativeModes = {
            PNGCodecGM::Mode::kCodec,
            PNGCodecGM::Mode::kCodecZeroInit,
            PNGCodecGM::Mode::kScanline,
    };

    std::vector<PNGCodecGM::DstColorType> colorTypes = {
            PNGCodecGM::DstColorType::kGetFromCanvas,
            PNGCodecGM::DstColorType::kNonNative8888_Always,
    };
    if (decodeResult.codec->getInfo().colorType() == kGray_8_SkColorType) {
        colorTypes.push_back(PNGCodecGM::DstColorType::kGrayscale_Always);
    }

    std::vector<SkAlphaType> alphaModes = {kPremul_SkAlphaType};
    if (decodeResult.codec->getInfo().alphaType() != kOpaque_SkAlphaType) {
        alphaModes.push_back(kUnpremul_SkAlphaType);
    }

    for (PNGCodecGM::Mode mode : nativeModes) {
        for (PNGCodecGM::DstColorType colorType : colorTypes) {
            for (SkAlphaType alphaType : alphaModes) {
                skiagm::Register(new PNGCodecGM(path, mode, colorType, alphaType));
            }
        }
    }

    return "";
}

// This factory registers zero or more PNGCodecGM instances for each image passed via the
// --pngCodecGMImages flag, which can take files and directories. It scans directories
// non-recursively for files with known image extensions, and registers zero or more PNGCodecGM
// instances for each image it finds.
//
// Based on DM's gather_srcs() function.
// https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DM.cpp#953
DEF_GM_REGISTERER_FN([]() -> std::string {
    DECLARE_string(pngCodecGMImages);

    skia_private::TArray<SkString> images;
    if (!CommonFlags::CollectImages(FLAGS_pngCodecGMImages, &images)) {
        return "Failed to collect images.";
    }

    for (const SkString& image : images) {
        if (!(image.endsWith(".png") || image.endsWith(".PNG"))) {
            continue;
        }

        if (std::string errorMsg = registerGMsForImage(image.c_str()); errorMsg != "") {
            return errorMsg;
        }

        break;
    }

    return "";
});
