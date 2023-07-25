/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
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

// This GM aims to replace DM's CodecSrc. It takes a single image as an argument and applies the
// same logic as CodecSrc.
//
// Class CodecGMRegisterer, defined below, scans the files and directories passed via the
// --codecGMImages flag and dynamically registers zero or more CodecGM instances for each image.
//
// See the CodecSrc class here:
// https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.h#158.
class CodecGM : public skiagm::GM {
public:
    // Based on CodecSrc::Mode.
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.h#160
    enum Mode {
        kCodec_Mode,
        // We choose to test only one mode with zero initialized memory. This will exercise all of
        // the interesting cases in SkSwizzler without doubling the size of our test suite.
        kCodecZeroInit_Mode,
        kScanline_Mode,
        kStripe_Mode,           // Tests the skipping of scanlines.
        kCroppedScanline_Mode,  // Tests (JPEG) cropped scanline optimization.
        kSubset_Mode,           // For codecs that support subsets directly.
        kAnimated_Mode,         // For codecs that support animation.
    };

    // Based on CodecSrc::DstColorType.
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.h#172
    enum DstColorType {
        kGetFromCanvas_DstColorType,
        kGrayscale_Always_DstColorType,
        kNonNative8888_Always_DstColorType,
    };

    // Based on DM's CodecSrc::CodecSrc().
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#371
    CodecGM(std::string path,
            Mode mode,
            DstColorType dstColorType,
            SkAlphaType dstAlphaType,
            float scale)
            : skiagm::GM()
            , fPath(path)
            , fMode(mode)
            , fDstColorType(dstColorType)
            , fDstAlphaType(dstAlphaType)
            , fScale(scale) {}

    bool isBazelOnly() const override {
        // This GM class overlaps with DM's CodecSrc and related sources.
        return true;
    }

    // Based on DM's push_codec_src() function.
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DM.cpp#555
    SkString getOutputFolder() const override {
        SkString folder;
        switch (fMode) {
            case CodecGM::kCodec_Mode:
                folder.append("codec");
                break;
            case CodecGM::kCodecZeroInit_Mode:
                folder.append("codec_zero_init");
                break;
            case CodecGM::kScanline_Mode:
                folder.append("scanline");
                break;
            case CodecGM::kStripe_Mode:
                folder.append("stripe");
                break;
            case CodecGM::kCroppedScanline_Mode:
                folder.append("crop");
                break;
            case CodecGM::kSubset_Mode:
                folder.append("codec_subset");
                break;
            case CodecGM::kAnimated_Mode:
                folder.append("codec_animated");
                break;
        }

        switch (fDstColorType) {
            case CodecGM::kGrayscale_Always_DstColorType:
                folder.append("_kGray8");
                break;
            case CodecGM::kNonNative8888_Always_DstColorType:
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

        if (1.0f != fScale) {
            folder.appendf("_%.3f", fScale);
        }

        return folder;
    }

protected:
    // Based on CodecSrc::name().
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#828
    SkString onShortName() override {
        SkString name = SkOSPath::Basename(fPath.c_str());
        if (fMode == kAnimated_Mode) {
            name.append("_animated");
        }
        if (1.0f == fScale) {
            return name;
        }
        return SkStringPrintf("%s_%.3f", name.c_str(), fScale);
    }

    // Based on CodecSrc::size().
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#803
    SkISize onISize() override {
        sk_sp<SkData> encoded(SkData::MakeFromFileName(fPath.c_str()));
        std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(encoded));
        if (nullptr == codec) {
            return {0, 0};
        }

        if (fMode != kAnimated_Mode) {
            return codec->getScaledDimensions(fScale);
        }

        // We'll draw one of each frame, so make it big enough to hold them all
        // in a grid. The grid will be roughly square, with "factor" frames per
        // row and up to "factor" rows.
        const size_t count = codec->getFrameInfo().size();
        const float root = sqrt((float)count);
        const int factor = sk_float_ceil2int(root);

        auto androidCodec = SkAndroidCodec::MakeFromCodec(std::move(codec));
        auto imageSize = androidCodec->getSampledDimensions(1 / fScale);
        imageSize.fWidth = imageSize.fWidth * factor;
        imageSize.fHeight = imageSize.fHeight * sk_float_ceil2int((float)count / (float)factor);
        return imageSize;
    }

    // Based on CodecSrc::draw().
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#450
    // void onDraw(SkCanvas* canvas) override {
    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        sk_sp<SkData> encoded(SkData::MakeFromFileName(fPath.c_str()));
        if (!encoded) {
            *errorMsg = SkStringPrintf("Could not read %s.", fPath.c_str());
            return DrawResult::kFail;
        }

        std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(encoded));
        if (nullptr == codec) {
            *errorMsg = SkStringPrintf("Could not create codec for %s.", fPath.c_str());
            return DrawResult::kFail;
        }

        SkImageInfo decodeInfo = codec->getInfo();
        if (!get_decode_info(&decodeInfo, canvas->imageInfo().colorType())) {
            *errorMsg = "Skipping uninteresting test.";
            return DrawResult::kSkip;
        }

        // Try to scale the image if it is desired
        SkISize size = codec->getScaledDimensions(fScale);

        std::unique_ptr<SkAndroidCodec> androidCodec;
        if (1.0f != fScale && fMode == kAnimated_Mode) {
            androidCodec = SkAndroidCodec::MakeFromData(encoded);
            size = androidCodec->getSampledDimensions(1 / fScale);
        }

        if (size == decodeInfo.dimensions() && 1.0f != fScale) {
            *errorMsg = "Test without scaling is uninteresting.";
            return DrawResult::kSkip;
        }

        // Visually inspecting very small output images is not necessary.  We will
        // cover these cases in unit testing.
        if ((size.width() <= 10 || size.height() <= 10) && 1.0f != fScale) {
            *errorMsg = "Scaling very small images is uninteresting.";
            return DrawResult::kSkip;
        }
        decodeInfo = decodeInfo.makeDimensions(size);

        const int bpp = decodeInfo.bytesPerPixel();
        const size_t rowBytes = size.width() * bpp;
        const size_t safeSize = decodeInfo.computeByteSize(rowBytes);
        SkAutoMalloc pixels(safeSize);

        SkCodec::Options options;
        if (kCodecZeroInit_Mode == fMode) {
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
            case kAnimated_Mode: {
                SkAndroidCodec::AndroidOptions androidOptions;
                if (fScale != 1.0f) {
                    SkASSERT(androidCodec);
                    androidOptions.fSampleSize = 1 / fScale;
                    auto dims = androidCodec->getSampledDimensions(androidOptions.fSampleSize);
                    decodeInfo = decodeInfo.makeDimensions(dims);
                }

                std::vector<SkCodec::FrameInfo> frameInfos =
                        androidCodec ? androidCodec->codec()->getFrameInfo()
                                     : codec->getFrameInfo();
                if (frameInfos.size() <= 1) {
                    *errorMsg = SkStringPrintf("%s is not an animated image.", fPath.c_str());
                    return DrawResult::kFail;
                }

                // As in CodecSrc::size(), compute a roughly square grid to draw the frames
                // into. "factor" is the number of frames to draw on one row. There will be
                // up to "factor" rows as well.
                const float root = sqrt((float)frameInfos.size());
                const int factor = sk_float_ceil2int(root);

                // Used to cache a frame that future frames will depend on.
                SkAutoMalloc priorFramePixels;
                int cachedFrame = SkCodec::kNoFrame;
                for (int i = 0; static_cast<size_t>(i) < frameInfos.size(); i++) {
                    androidOptions.fFrameIndex = i;
                    // Check for a prior frame
                    const int reqFrame = frameInfos[i].fRequiredFrame;
                    if (reqFrame != SkCodec::kNoFrame && reqFrame == cachedFrame &&
                        priorFramePixels.get()) {
                        // Copy into pixels
                        memcpy(pixels.get(), priorFramePixels.get(), safeSize);
                        androidOptions.fPriorFrame = reqFrame;
                    } else {
                        androidOptions.fPriorFrame = SkCodec::kNoFrame;
                    }
                    SkCodec::Result result =
                            androidCodec
                                    ? androidCodec->getAndroidPixels(
                                              decodeInfo, pixels.get(), rowBytes, &androidOptions)
                                    : codec->getPixels(
                                              decodeInfo, pixels.get(), rowBytes, &androidOptions);
                    if (SkCodec::kInvalidInput == result && i > 0) {
                        // Some of our test images have truncated later frames. Treat that
                        // the same as incomplete.
                        result = SkCodec::kIncompleteInput;
                    }
                    switch (result) {
                        case SkCodec::kSuccess:
                        case SkCodec::kErrorInInput:
                        case SkCodec::kIncompleteInput: {
                            // If the next frame depends on this one, store it in priorFrame.
                            // It is possible that we may discard a frame that future frames depend
                            // on, but the codec will simply redecode the discarded frame. Do this
                            // before calling draw_to_canvas, which premultiplies in place. If we're
                            // decoding to unpremul, we want to pass the unmodified frame to the
                            // codec for decoding the next frame.
                            if (static_cast<size_t>(i + 1) < frameInfos.size() &&
                                frameInfos[i + 1].fRequiredFrame == i) {
                                memcpy(priorFramePixels.reset(safeSize), pixels.get(), safeSize);
                                cachedFrame = i;
                            }

                            SkAutoCanvasRestore acr(canvas, true);
                            const int xTranslate = (i % factor) * decodeInfo.width();
                            const int yTranslate = (i / factor) * decodeInfo.height();
                            canvas->translate(SkIntToScalar(xTranslate), SkIntToScalar(yTranslate));
                            draw_to_canvas(canvas, bitmapInfo, pixels.get(), rowBytes);
                            if (result != SkCodec::kSuccess) {
                                return DrawResult::kOk;
                            }
                            break;
                        }
                        case SkCodec::kInvalidConversion:
                            if (i > 0 && (decodeInfo.colorType() == kRGB_565_SkColorType)) {
                                *errorMsg = SkStringPrintf(
                                        "Cannot decode frame %i to 565 (%s).", i, fPath.c_str());
                                return DrawResult::kSkip;
                            }
                            [[fallthrough]];
                        default:
                            *errorMsg = SkStringPrintf(
                                    "Couldn't getPixels for frame %i in %s.", i, fPath.c_str());
                            return DrawResult::kFail;
                    }
                }
                break;
            }
            case kCodecZeroInit_Mode:
            case kCodec_Mode: {
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

                draw_to_canvas(canvas, bitmapInfo, pixels.get(), rowBytes);
                break;
            }
            case kScanline_Mode: {
                void* dst = pixels.get();
                uint32_t height = decodeInfo.height();
                const bool useIncremental = [this]() {
                    auto exts = {"png", "PNG", "gif", "GIF"};
                    for (auto ext : exts) {
                        if (SkString(fPath).endsWith(ext)) {
                            return true;
                        }
                    }
                    return false;
                }();
                // ico may use the old scanline method or the new one, depending on whether it
                // internally holds a bmp or a png.
                const bool ico = SkString(fPath).endsWith("ico");
                bool useOldScanlineMethod = !useIncremental && !ico;
                if (useIncremental || ico) {
                    if (SkCodec::kSuccess ==
                        codec->startIncrementalDecode(decodeInfo, dst, rowBytes, &options)) {
                        int rowsDecoded;
                        auto result = codec->incrementalDecode(&rowsDecoded);
                        if (SkCodec::kIncompleteInput == result ||
                            SkCodec::kErrorInInput == result) {
                            codec->fillIncompleteImage(decodeInfo,
                                                       dst,
                                                       rowBytes,
                                                       SkCodec::kNo_ZeroInitialized,
                                                       height,
                                                       rowsDecoded);
                        }
                    } else {
                        if (useIncremental) {
                            // Error: These should support incremental decode.
                            *errorMsg = "Could not start incremental decode";
                            return DrawResult::kFail;
                        }
                        // Otherwise, this is an ICO. Since incremental failed, it must contain a
                        // BMP, which should work via startScanlineDecode
                        useOldScanlineMethod = true;
                    }
                }

                if (useOldScanlineMethod) {
                    if (SkCodec::kSuccess != codec->startScanlineDecode(decodeInfo)) {
                        *errorMsg = "Could not start scanline decoder";
                        return DrawResult::kFail;
                    }

                    // We do not need to check the return value.  On an incomplete
                    // image, memory will be filled with a default value.
                    codec->getScanlines(dst, height, rowBytes);
                }

                draw_to_canvas(canvas, bitmapInfo, dst, rowBytes);
                break;
            }
            case kStripe_Mode: {
                const int height = decodeInfo.height();
                // This value is chosen arbitrarily.  We exercise more cases by choosing a value
                // that does not align with image blocks.
                const int stripeHeight = 37;
                const int numStripes = (height + stripeHeight - 1) / stripeHeight;
                void* dst = pixels.get();

                // Decode odd stripes
                if (SkCodec::kSuccess != codec->startScanlineDecode(decodeInfo, &options)) {
                    *errorMsg = "Could not start scanline decoder";
                    return DrawResult::kFail;
                }

                // This mode was designed to test the new skip scanlines API in libjpeg-turbo.
                // Jpegs have kTopDown_SkScanlineOrder, and at this time, it is not interesting
                // to run this test for image types that do not have this scanline ordering.
                // We only run this on Jpeg, which is always kTopDown.
                SkASSERT(SkCodec::kTopDown_SkScanlineOrder == codec->getScanlineOrder());

                for (int i = 0; i < numStripes; i += 2) {
                    // Skip a stripe
                    const int linesToSkip = std::min(stripeHeight, height - i * stripeHeight);
                    codec->skipScanlines(linesToSkip);

                    // Read a stripe
                    const int startY = (i + 1) * stripeHeight;
                    const int linesToRead = std::min(stripeHeight, height - startY);
                    if (linesToRead > 0) {
                        codec->getScanlines(
                                SkTAddOffset<void>(dst, rowBytes * startY), linesToRead, rowBytes);
                    }
                }

                // Decode even stripes
                const SkCodec::Result startResult = codec->startScanlineDecode(decodeInfo);
                if (SkCodec::kSuccess != startResult) {
                    *errorMsg = "Failed to restart scanline decoder with same parameters";
                    return DrawResult::kFail;
                }
                for (int i = 0; i < numStripes; i += 2) {
                    // Read a stripe
                    const int startY = i * stripeHeight;
                    const int linesToRead = std::min(stripeHeight, height - startY);
                    codec->getScanlines(
                            SkTAddOffset<void>(dst, rowBytes * startY), linesToRead, rowBytes);

                    // Skip a stripe
                    const int linesToSkip = std::min(stripeHeight, height - (i + 1) * stripeHeight);
                    if (linesToSkip > 0) {
                        codec->skipScanlines(linesToSkip);
                    }
                }

                draw_to_canvas(canvas, bitmapInfo, dst, rowBytes);
                break;
            }
            case kCroppedScanline_Mode: {
                const int width = decodeInfo.width();
                const int height = decodeInfo.height();
                // This value is chosen because, as we move across the image, it will sometimes
                // align with the jpeg block sizes and it will sometimes not.  This allows us
                // to test interestingly different code paths in the implementation.
                const int tileSize = 36;
                SkIRect subset;
                for (int x = 0; x < width; x += tileSize) {
                    subset = SkIRect::MakeXYWH(x, 0, std::min(tileSize, width - x), height);
                    options.fSubset = &subset;
                    if (SkCodec::kSuccess != codec->startScanlineDecode(decodeInfo, &options)) {
                        *errorMsg = "Could not start scanline decoder.";
                        return DrawResult::kFail;
                    }

                    codec->getScanlines(
                            SkTAddOffset<void>(pixels.get(), x * bpp), height, rowBytes);
                }

                draw_to_canvas(canvas, bitmapInfo, pixels.get(), rowBytes);
                break;
            }
            case kSubset_Mode: {
                // Arbitrarily choose a divisor.
                int divisor = 2;
                // Total width/height of the image.
                const int W = codec->getInfo().width();
                const int H = codec->getInfo().height();
                if (divisor > W || divisor > H) {
                    *errorMsg = SkStringPrintf(
                            "Cannot codec subset: divisor %d is too big "
                            "for %s with dimensions (%d x %d)",
                            divisor,
                            fPath.c_str(),
                            W,
                            H);
                    return DrawResult::kSkip;
                }
                // subset dimensions
                // SkWebpCodec, the only one that supports subsets, requires even top/left
                // boundaries.
                const int w = SkAlign2(W / divisor);
                const int h = SkAlign2(H / divisor);
                SkIRect subset;
                options.fSubset = &subset;
                SkBitmap subsetBm;
                // We will reuse pixel memory from bitmap.
                void* dst = pixels.get();
                // Keep track of left and top (for drawing subsetBm into canvas). We could use
                // fScale * x and fScale * y, but we want integers such that the next subset will
                // start where the last one ended. So we'll add decodeInfo.width() and height().
                int left = 0;
                for (int x = 0; x < W; x += w) {
                    int top = 0;
                    for (int y = 0; y < H; y += h) {
                        // Do not make the subset go off the edge of the image.
                        const int preScaleW = std::min(w, W - x);
                        const int preScaleH = std::min(h, H - y);
                        subset.setXYWH(x, y, preScaleW, preScaleH);
                        // And scale
                        // FIXME: Should we have a version of getScaledDimensions that takes a
                        // subset into account?
                        const int scaledW = std::max(1, SkScalarRoundToInt(preScaleW * fScale));
                        const int scaledH = std::max(1, SkScalarRoundToInt(preScaleH * fScale));
                        decodeInfo = decodeInfo.makeWH(scaledW, scaledH);
                        SkImageInfo subsetBitmapInfo = bitmapInfo.makeWH(scaledW, scaledH);
                        size_t subsetRowBytes = subsetBitmapInfo.minRowBytes();
                        const SkCodec::Result result =
                                codec->getPixels(decodeInfo, dst, subsetRowBytes, &options);
                        switch (result) {
                            case SkCodec::kSuccess:
                            case SkCodec::kErrorInInput:
                            case SkCodec::kIncompleteInput:
                                break;
                            default:
                                *errorMsg = SkStringPrintf(
                                        "subset codec failed to decode (%d, %d, %d, %d) "
                                        "from %s with dimensions (%d x %d)\t error %d",
                                        x,
                                        y,
                                        decodeInfo.width(),
                                        decodeInfo.height(),
                                        fPath.c_str(),
                                        W,
                                        H,
                                        result);
                                return DrawResult::kFail;
                        }
                        draw_to_canvas(canvas,
                                       subsetBitmapInfo,
                                       dst,
                                       subsetRowBytes,
                                       SkIntToScalar(left),
                                       SkIntToScalar(top));

                        // translate by the scaled height.
                        top += decodeInfo.height();
                    }
                    // translate by the scaled width.
                    left += decodeInfo.width();
                }
                return DrawResult::kOk;
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
    bool get_decode_info(SkImageInfo* decodeInfo, SkColorType canvasColorType) {
        switch (fDstColorType) {
            case CodecGM::kGrayscale_Always_DstColorType:
                if (kRGB_565_SkColorType == canvasColorType) {
                    return false;
                }
                *decodeInfo = decodeInfo->makeColorType(kGray_8_SkColorType);
                break;
            case CodecGM::kNonNative8888_Always_DstColorType:
                if (kRGB_565_SkColorType == canvasColorType ||
                    kRGBA_F16_SkColorType == canvasColorType) {
                    return false;
                }
                *decodeInfo = decodeInfo->makeColorType(kBGRA_8888_SkColorType);
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
    void draw_to_canvas(SkCanvas* canvas,
                        const SkImageInfo& info,
                        void* pixels,
                        size_t rowBytes,
                        SkScalar left = 0,
                        SkScalar top = 0) {
        SkBitmap bitmap;
        bitmap.installPixels(info, pixels, rowBytes);
        swap_rb_if_necessary(bitmap);
        canvas->drawImage(bitmap.asImage(), left, top);
    }

    // Allows us to test decodes to non-native 8888.
    //
    // Based on DM's swap_rb_if_necessary function.
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#387
    void swap_rb_if_necessary(SkBitmap& bitmap) {
        if (CodecGM::kNonNative8888_Always_DstColorType != fDstColorType) {
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
    float fScale;
};

// Populates the vector of GMs with zero or more CodecGM instances for the given image. Returns a
// non-empty, human-friendly error message in the case of errors.
//
// Based on DM's push_codec_srcs() function. It only covers "simple" codecs (lines 740-834).
// https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DM.cpp#740
static std::string createGMsForImage(std::vector<std::unique_ptr<skiagm::GM>>& gms,
                                     std::string path) {
    sk_sp<SkData> encoded(SkData::MakeFromFileName(path.c_str()));
    if (!encoded) {
        return SkStringPrintf("Could not read \"%s\".\n", path.c_str()).c_str();
    }
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(encoded);
    if (nullptr == codec) {
        return SkStringPrintf("Could not create codec for \"%s\".", path.c_str()).c_str();
    }

    // Native scaling is only supported by WEBP and JPEG.
    bool supportsNativeScaling = false;

    std::vector<CodecGM::Mode> nativeModes;
    nativeModes.push_back(CodecGM::kCodec_Mode);
    nativeModes.push_back(CodecGM::kCodecZeroInit_Mode);
    switch (codec->getEncodedFormat()) {
        case SkEncodedImageFormat::kJPEG:
            nativeModes.push_back(CodecGM::kScanline_Mode);
            nativeModes.push_back(CodecGM::kStripe_Mode);
            nativeModes.push_back(CodecGM::kCroppedScanline_Mode);
            supportsNativeScaling = true;
            break;
        case SkEncodedImageFormat::kWEBP:
            nativeModes.push_back(CodecGM::kSubset_Mode);
            supportsNativeScaling = true;
            break;
        case SkEncodedImageFormat::kDNG:
            break;
        default:
            nativeModes.push_back(CodecGM::kScanline_Mode);
            break;
    }

    std::vector<CodecGM::DstColorType> colorTypes;
    colorTypes.push_back(CodecGM::kGetFromCanvas_DstColorType);
    colorTypes.push_back(CodecGM::kNonNative8888_Always_DstColorType);
    switch (codec->getInfo().colorType()) {
        case kGray_8_SkColorType:
            colorTypes.push_back(CodecGM::kGrayscale_Always_DstColorType);
            break;
        default:
            break;
    }

    std::vector<SkAlphaType> alphaModes;
    alphaModes.push_back(kPremul_SkAlphaType);
    if (codec->getInfo().alphaType() != kOpaque_SkAlphaType) {
        alphaModes.push_back(kUnpremul_SkAlphaType);
    }

    for (CodecGM::Mode mode : nativeModes) {
        for (CodecGM::DstColorType colorType : colorTypes) {
            for (SkAlphaType alphaType : alphaModes) {
                // Only test kCroppedScanline_Mode when the alpha type is premul. The test is
                // slow and won't be interestingly different with different alpha types.
                if (CodecGM::kCroppedScanline_Mode == mode && kPremul_SkAlphaType != alphaType) {
                    continue;
                }

                gms.push_back(std::make_unique<CodecGM>(path, mode, colorType, alphaType, 1.0f));

                // Skip kNonNative on different native scales. It won't be interestingly
                // different.
                if (supportsNativeScaling &&
                    CodecGM::kNonNative8888_Always_DstColorType == colorType) {
                    // Native Scales
                    // SkJpegCodec natively supports scaling to the following:
                    for (auto scale : {0.125f, 0.25f, 0.375f, 0.5f, 0.625f, 0.750f, 0.875f}) {
                        gms.push_back(
                                std::make_unique<CodecGM>(path, mode, colorType, alphaType, scale));
                    }
                }
            }
        }
    }

    {
        std::vector<SkCodec::FrameInfo> frameInfos = codec->getFrameInfo();
        if (frameInfos.size() > 1) {
            for (auto dstCT : {CodecGM::kNonNative8888_Always_DstColorType,
                               CodecGM::kGetFromCanvas_DstColorType}) {
                for (auto at : {kUnpremul_SkAlphaType, kPremul_SkAlphaType}) {
                    gms.push_back(std::make_unique<CodecGM>(
                            path, CodecGM::kAnimated_Mode, dstCT, at, 1.0f));
                }
            }
            for (float scale : {.5f, .33f}) {
                gms.push_back(std::make_unique<CodecGM>(path,
                                                        CodecGM::kAnimated_Mode,
                                                        CodecGM::kGetFromCanvas_DstColorType,
                                                        kPremul_SkAlphaType,
                                                        scale));
            }
        }
    }

    return "";
}

// This factory registers zero or more CodecGM instances for each image passed via the
// --codecGMImages flag, which can take files and directories. It scans directories non-recursively
// for files with known image extensions, and registers zero or more CodecGM instances for each
// image it finds.
//
// Based on DM's gather_srcs() function.
// https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DM.cpp#953
DEF_MULTIPLE_GMS([] {
    DECLARE_string(codecGMImages);

    skia_private::TArray<SkString> images;
    if (!CommonFlags::CollectImages(FLAGS_codecGMImages, &images)) {
        return skiagm::MultiGMFactoryResult{.errorMsg = "Failed to collect images."};
    }

    std::vector<std::unique_ptr<skiagm::GM>> gms;

    for (const SkString& image : images) {
        std::string errorMsg = createGMsForImage(gms, image.c_str());
        if (errorMsg != "") {
            return skiagm::MultiGMFactoryResult{.errorMsg = errorMsg};
        }
    }

    return skiagm::MultiGMFactoryResult{.gms = std::move(gms)};
});
