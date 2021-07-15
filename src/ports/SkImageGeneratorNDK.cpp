/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/ports/SkImageGeneratorNDK.h"
#include "include/private/SkOnce.h"
#include "src/ports/SkNDKConversions.h"

#include <android/bitmap.h>
#include <android/data_space.h>
#include <android/imagedecoder.h>

#include <dlfcn.h>

namespace {

// Calling NDK decoding APIs directly requires setting __ANDROID_API__ to 30 or
// higher, which would then require a client to use separate builds for devices
// with different __ANDROID_API__ levels. Instead, use dlsym to retrieve the
// desired symbols and gracefully fail if they are missing. Each type is named
// as <actual method name>_type so a macro can be used to declare and retrieve
// the symbol.
using AImageDecoder_createFromBuffer_type = int (*)(const void* buffer, size_t length,
                                                    AImageDecoder** outDecoder);
using AImageDecoder_delete_type = void (*)(AImageDecoder* decoder);
using AImageDecoder_setAndroidBitmapFormat_type = int (*)(AImageDecoder* decoder, int32_t format);
using AImageDecoder_setUnpremultipliedRequired_type = int (*)(AImageDecoder* decoder,
                                                              bool unpremultipliedRequired);
using AImageDecoder_setDataSpace_type = int (*)(AImageDecoder* decoder, int32_t dataspace);
using AImageDecoder_setTargetSize_type = int (*)(AImageDecoder* decoder, int32_t width,
                                                 int32_t height);
using AImageDecoder_computeSampledSize_type = int (*)(const AImageDecoder* decoder, int sampleSize,
                                                      int32_t* width, int32_t* height);
using AImageDecoder_getHeaderInfo_type = const AImageDecoderHeaderInfo* (*) (const AImageDecoder*);
using AImageDecoderHeaderInfo_getMimeType_type = const char* (*)(const AImageDecoderHeaderInfo*);
using AImageDecoderHeaderInfo_getWidth_type = int32_t (*)(const AImageDecoderHeaderInfo*);
using AImageDecoderHeaderInfo_getHeight_type = int32_t (*)(const AImageDecoderHeaderInfo*);
using AImageDecoderHeaderInfo_getAndroidBitmapFormat_type = int32_t (*)
        (const AImageDecoderHeaderInfo*);
using AImageDecoderHeaderInfo_getAlphaFlags_type = int (*)(const AImageDecoderHeaderInfo*);
using AImageDecoderHeaderInfo_getDataSpace_type = int32_t (*)(const AImageDecoderHeaderInfo*);
using AImageDecoder_decodeImage_type = int (*)(AImageDecoder* decoder, void* pixels, size_t stride,
                                               size_t size);

#define DECLARE_FUNC(name) \
    name##_type name;

DECLARE_FUNC(AImageDecoder_createFromBuffer)
DECLARE_FUNC(AImageDecoder_delete)
DECLARE_FUNC(AImageDecoder_setAndroidBitmapFormat)
DECLARE_FUNC(AImageDecoder_setUnpremultipliedRequired)
DECLARE_FUNC(AImageDecoder_setDataSpace)
DECLARE_FUNC(AImageDecoder_setTargetSize)
DECLARE_FUNC(AImageDecoder_computeSampledSize)
DECLARE_FUNC(AImageDecoder_getHeaderInfo)
DECLARE_FUNC(AImageDecoderHeaderInfo_getMimeType)
DECLARE_FUNC(AImageDecoderHeaderInfo_getWidth)
DECLARE_FUNC(AImageDecoderHeaderInfo_getHeight)
DECLARE_FUNC(AImageDecoderHeaderInfo_getAndroidBitmapFormat)
DECLARE_FUNC(AImageDecoderHeaderInfo_getAlphaFlags)
DECLARE_FUNC(AImageDecoderHeaderInfo_getDataSpace)
DECLARE_FUNC(AImageDecoder_decodeImage)
#undef DECLARE_FUNC

struct HandleCloser {
    void operator() (void* handle) const {
        if (handle) dlclose(handle);
    }
};

using Handle = std::unique_ptr<void, HandleCloser>;

bool init() {
    static bool initialized = false;
    static SkOnce initFunctions;
    initFunctions([]{
        const char* libName = "libjnigraphics.so";
        Handle handle_(dlopen(libName, RTLD_NOW | RTLD_LOCAL));
        if (!handle_) {
            SkDebugf("Failed to open %s\n", libName);
            return;
        }

#define INIT_FUNC(name)                                         \
        name = (name##_type) dlsym(handle_.get(), #name);       \
        if (!name) {                                            \
            SkDebugf("Failed to find function %s!\n", #name);   \
            return;                                             \
        }

        INIT_FUNC(AImageDecoder_createFromBuffer)
        INIT_FUNC(AImageDecoder_delete)
        INIT_FUNC(AImageDecoder_setAndroidBitmapFormat)
        INIT_FUNC(AImageDecoder_setUnpremultipliedRequired)
        INIT_FUNC(AImageDecoder_setDataSpace)
        INIT_FUNC(AImageDecoder_setTargetSize)
        INIT_FUNC(AImageDecoder_computeSampledSize)
        INIT_FUNC(AImageDecoder_getHeaderInfo)
        INIT_FUNC(AImageDecoderHeaderInfo_getMimeType)
        INIT_FUNC(AImageDecoderHeaderInfo_getWidth)
        INIT_FUNC(AImageDecoderHeaderInfo_getHeight)
        INIT_FUNC(AImageDecoderHeaderInfo_getAndroidBitmapFormat)
        INIT_FUNC(AImageDecoderHeaderInfo_getAlphaFlags)
        INIT_FUNC(AImageDecoderHeaderInfo_getDataSpace)
        INIT_FUNC(AImageDecoder_decodeImage)
#undef INIT_FUNC

        // Release the handle without calling dlclose so we can continue to call
        // these functions without calling dlopen/dlsym again.
        handle_.release();
        initialized = true;
    });
    return initialized;
}

class ImageGeneratorNDK : public SkImageGenerator {
public:
    static std::unique_ptr<SkImageGenerator> MakeFromEncoded(sk_sp<SkData> data);
    ~ImageGeneratorNDK() override;

protected:
    sk_sp<SkData> onRefEncodedData() override;

    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                     const Options& opts) override;

private:
    sk_sp<SkData>  fData;
    AImageDecoder* fDecoder;
    // Setting the ADataSpace is sticky - it is set for all future decodes
    // until it is set again. But as of R there is no way to reset it to
    // ADATASPACE_UNKNOWN to skip color correction. If the client requests
    // skipping correction after having set it to something else, we need
    // to recreate the AImageDecoder.
    bool           fPreviouslySetADataSpace;

    ImageGeneratorNDK(const SkImageInfo&, sk_sp<SkData>, AImageDecoder*);

    using INHERITED = SkImageGenerator;
};

bool ok(int result) {
    return result == ANDROID_IMAGE_DECODER_SUCCESS;
}

bool set_android_bitmap_format(AImageDecoder* decoder, SkColorType colorType) {
    auto format = SkNDKConversions::toAndroidBitmapFormat(colorType);
    return ok(AImageDecoder_setAndroidBitmapFormat(decoder, format));
}

SkColorType colorType(AImageDecoder* decoder, const AImageDecoderHeaderInfo* headerInfo) {
    // AImageDecoder never defaults to gray, but allows setting it if the image is 8 bit gray.
    if (set_android_bitmap_format(decoder, kGray_8_SkColorType)) {
        return kGray_8_SkColorType;
    }

    auto format = static_cast<AndroidBitmapFormat>(
            AImageDecoderHeaderInfo_getAndroidBitmapFormat(headerInfo));
    return SkNDKConversions::toColorType(format);
}

sk_sp<SkColorSpace> get_default_colorSpace(const AImageDecoderHeaderInfo* headerInfo) {
    auto dataSpace = static_cast<ADataSpace>(AImageDecoderHeaderInfo_getDataSpace(headerInfo));
    if (auto cs = SkNDKConversions::toColorSpace(dataSpace)) {
        return cs;
    }

    return SkColorSpace::MakeSRGB();
}

std::unique_ptr<SkImageGenerator> ImageGeneratorNDK::MakeFromEncoded(sk_sp<SkData> data) {
    if (!init()) return nullptr;

    if (!data) return nullptr;

    AImageDecoder* rawDecoder;
    if (!ok(AImageDecoder_createFromBuffer(data->data(), data->size(), &rawDecoder))) {
        return nullptr;
    }

    const AImageDecoderHeaderInfo* headerInfo = AImageDecoder_getHeaderInfo(rawDecoder);
    int32_t width  = AImageDecoderHeaderInfo_getWidth(headerInfo);
    int32_t height = AImageDecoderHeaderInfo_getHeight(headerInfo);
    SkColorType ct = colorType(rawDecoder, headerInfo);

    // Although the encoded data stores unpremultiplied pixels, AImageDecoder defaults to premul
    // (if the image may have alpha).
    SkAlphaType at = AImageDecoderHeaderInfo_getAlphaFlags(headerInfo)
            == ANDROID_BITMAP_FLAGS_ALPHA_OPAQUE ? kOpaque_SkAlphaType : kPremul_SkAlphaType;
    auto imageInfo = SkImageInfo::Make(width, height, ct, at, get_default_colorSpace(headerInfo));
    return std::unique_ptr<SkImageGenerator>(
            new ImageGeneratorNDK(imageInfo, std::move(data), rawDecoder));
}

ImageGeneratorNDK::ImageGeneratorNDK(const SkImageInfo& info, sk_sp<SkData> data,
                                     AImageDecoder* decoder)
    : INHERITED(info)
    , fData(std::move(data))
    , fDecoder(decoder)
    , fPreviouslySetADataSpace(false)
{
    SkASSERT(fDecoder);
}

ImageGeneratorNDK::~ImageGeneratorNDK() {
    AImageDecoder_delete(fDecoder);
}

bool set_target_size(AImageDecoder* decoder, const SkISize& size, const SkISize targetSize) {
    if (size != targetSize) {
        // AImageDecoder will scale to arbitrary sizes. Only support a size if it's supported by the
        // underlying library.
        const AImageDecoderHeaderInfo* headerInfo = AImageDecoder_getHeaderInfo(decoder);
        const char* mimeType = AImageDecoderHeaderInfo_getMimeType(headerInfo);
        if (0 == strcmp(mimeType, "image/jpeg")) {
            bool supported = false;
            for (int sampleSize : { 2, 4, 8 }) {
                int32_t width;
                int32_t height;
                if (ok(AImageDecoder_computeSampledSize(decoder, sampleSize, &width, &height))
                        && targetSize == SkISize::Make(width, height)) {
                    supported = true;
                    break;
                }
            }
            if (!supported) return false;
        } else if (0 == strcmp(mimeType, "image/webp")) {
            // libwebp supports arbitrary downscaling.
            if (targetSize.width() > size.width() || targetSize.height() > size.height()) {
                return false;
            }
        } else {
            return false;
        }
    }
    return ok(AImageDecoder_setTargetSize(decoder, targetSize.width(), targetSize.height()));
}

bool ImageGeneratorNDK::onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                                    const Options& opts) {
    if (auto* cs = info.colorSpace()) {
        if (!ok(AImageDecoder_setDataSpace(fDecoder, SkNDKConversions::toDataSpace(cs)))) {
            return false;
        }
        fPreviouslySetADataSpace = true;
    } else {
        // If the requested SkColorSpace is null, the client wants the "raw" colors, without color
        // space transformations applied. (This is primarily useful for a client that wants to do
        // their own color transformations.) This is AImageDecoder's default, but if a previous call
        // set an ADataSpace, AImageDecoder is no longer using its default, so we need to set it
        // back.
        if (fPreviouslySetADataSpace) {
            // AImageDecoderHeaderInfo_getDataSpace always returns the same value for the same
            // image, regardless of prior calls to AImageDecoder_setDataSpace. Check if it's
            // ADATASPACE_UNKNOWN, which needs to be handled specially.
            const AImageDecoderHeaderInfo* headerInfo = AImageDecoder_getHeaderInfo(fDecoder);
            const auto defaultDataSpace = AImageDecoderHeaderInfo_getDataSpace(headerInfo);
            if (defaultDataSpace == ADATASPACE_UNKNOWN) {
                // As of R, there's no way to reset AImageDecoder to ADATASPACE_UNKNOWN, so
                // create a new one.
                AImageDecoder* decoder;
                if (!ok(AImageDecoder_createFromBuffer(fData->data(), fData->size(), &decoder))) {
                    return false;
                }
                AImageDecoder_delete(fDecoder);
                fDecoder = decoder;
            } else {
                if (!ok(AImageDecoder_setDataSpace(fDecoder, defaultDataSpace))) {
                    return false;
                }
            }

            // Whether by recreating AImageDecoder or calling AImageDecoder_setDataSpace, the
            // AImageDecoder is back to its default, so if the next call has a null SkColorSpace, it
            // does not need to reset it again.
            fPreviouslySetADataSpace = false;
        }
    }

    if (!set_android_bitmap_format(fDecoder, info.colorType())) {
        return false;
    }

    switch (info.alphaType()) {
        case kUnknown_SkAlphaType:
            return false;
        case kOpaque_SkAlphaType:
            if (this->getInfo().alphaType() != kOpaque_SkAlphaType) {
                return false;
            }
            break;
        case kUnpremul_SkAlphaType:
            if (!ok(AImageDecoder_setUnpremultipliedRequired(fDecoder, true))) {
                return false;
            }
            break;
        case kPremul_SkAlphaType:
            break;
    }

    if (!set_target_size(fDecoder, getInfo().dimensions(), info.dimensions())) {
        return false;
    }

    auto byteSize = info.computeByteSize(rowBytes);
    switch (AImageDecoder_decodeImage(fDecoder, pixels, rowBytes, byteSize)) {
        case ANDROID_IMAGE_DECODER_INCOMPLETE:
            // The image was partially decoded, but the input was truncated. The client may be
            // happy with the partial image.
        case ANDROID_IMAGE_DECODER_ERROR:
            // Similarly, the image was partially decoded, but the input had an error. The client
            // may be happy with the partial image.
        case ANDROID_IMAGE_DECODER_SUCCESS:
            return true;
        default:
            return false;
    }
}

sk_sp<SkData> ImageGeneratorNDK::onRefEncodedData() {
    return fData;
}

} // anonymous namespace

std::unique_ptr<SkImageGenerator> SkImageGeneratorNDK::MakeFromEncodedNDK(sk_sp<SkData> data) {
    return ImageGeneratorNDK::MakeFromEncoded(std::move(data));
}
