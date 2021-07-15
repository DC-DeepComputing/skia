/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkTFitsIn.h"
#include "include/private/SkTo.h"
#include "include/private/SkOnce.h"
#include "src/images/SkImageEncoderPriv.h"
#include "src/ports/SkNDKConversions.h"

#include <dlfcn.h>

namespace {
// Calling AndroidBitmap_compress directly requires setting __ANDROID_API__ to
// 30 or higher, which would then require a client to use separate builds for
// devices with different __ANDROID_API__ levels. Instead, use dlsym to retrieve
// the desired symbol and gracefully fail if it is missing.

// AndroidBitmap_CompressWriteFunc cannot be used directly here, since we are
// not defining __ANDROID_API__. Redeclare it with the same signature.
typedef bool (*CompressWriteFunc)(void* userContext, const void* data, size_t size);

using AndroidBitmap_compress_type = int(*) (const AndroidBitmapInfo* info, int32_t dataspace,
                                            const void* pixels, int32_t format, int32_t quality,
                                            void* userContext, CompressWriteFunc fn);
// A function pointer to AndroidBitmap_compress. It would be clearer to call this
// "AndroidBitmap_compress", but that would be ambiguous with the actual function declared in
// <android/bitmap.h>. It could be called from within this anonymous namespace, but not by
// SkEncodeImageWithNDK, which must be callable from elsewhere.
AndroidBitmap_compress_type ndk_encode;

struct HandleCloser {
    void operator() (void* handle) const {
        if (handle) dlclose(handle);
    }
};

using Handle = std::unique_ptr<void, HandleCloser>;

bool init() {
    static bool initialized = false;
    static SkOnce initFunction;
    initFunction([]{
        const char* libName = "libjnigraphics.so";
        Handle handle_ (dlopen(libName, RTLD_NOW | RTLD_LOCAL));
        if (!handle_) {
            SkDebugf("Failed to open %s\n", libName);
            return;
        }
        ndk_encode = (AndroidBitmap_compress_type)dlsym(handle_.get(), "AndroidBitmap_compress");
        if (!ndk_encode) {
            SkDebugf("Failed to find AndroidBitmap_compress!\n");
            return;
        }

        // Release the handle without calling dlclose so we can continue to call
        // ndk_encode without calling dlopen/dlsym again.
        handle_.release();
        initialized = true;
    });
    return initialized;
}
} // Anonymous namespace

bool SkEncodeImageWithNDK(SkWStream* stream, const SkPixmap& pmap, SkEncodedImageFormat format,
                          int quality) {
    if (!init()) {
        return false;
    }

    // If any of these values is invalid (e.g. set to zero), the info will be rejected by
    // AndroidBitmap_compress.
    AndroidBitmapInfo info {
        .width  = SkTFitsIn<uint32_t>(pmap.width())    ? SkToU32(pmap.width())    : 0,
        .height = SkTFitsIn<uint32_t>(pmap.height())   ? SkToU32(pmap.height())   : 0,
        .stride = SkTFitsIn<uint32_t>(pmap.rowBytes()) ? SkToU32(pmap.rowBytes()) : 0,
        .format = SkNDKConversions::toAndroidBitmapFormat(pmap.colorType())
    };

    switch (pmap.alphaType()) {
        case kPremul_SkAlphaType:
            info.flags = ANDROID_BITMAP_FLAGS_ALPHA_PREMUL;
            break;
        case kOpaque_SkAlphaType:
            info.flags = ANDROID_BITMAP_FLAGS_ALPHA_OPAQUE;
            break;
        case kUnpremul_SkAlphaType:
            info.flags = ANDROID_BITMAP_FLAGS_ALPHA_UNPREMUL;
            break;
        default:
            return false;
    }

    AndroidBitmapCompressFormat androidFormat;
    switch (format) {
        case SkEncodedImageFormat::kJPEG:
            androidFormat = ANDROID_BITMAP_COMPRESS_FORMAT_JPEG;
            break;
        case SkEncodedImageFormat::kPNG:
            androidFormat = ANDROID_BITMAP_COMPRESS_FORMAT_PNG;
            break;
        case SkEncodedImageFormat::kWEBP:
            if (quality == 100) {
                // Mimic the behavior of SkImageEncoder.cpp. In LOSSLESS mode, libwebp
                // interprets quality as the amount of effort (time) to spend making
                // the encoded image smaller, while the visual quality remains constant.
                // This value of 75 (on a scale of 0 - 100, where 100 spends the most
                // time for the smallest encoding) matches WebPConfigInit.
                androidFormat = ANDROID_BITMAP_COMPRESS_FORMAT_WEBP_LOSSLESS;
                quality = 75;
            } else {
                androidFormat = ANDROID_BITMAP_COMPRESS_FORMAT_WEBP_LOSSY;
            }
            break;
        default:
            return false;
    }

    auto write_to_stream = [](void* userContext, const void* data, size_t size) {
        return reinterpret_cast<SkWStream*>(userContext)->write(data, size);
    };

    return ANDROID_BITMAP_RESULT_SUCCESS == ndk_encode(&info,
            SkNDKConversions::toDataSpace(pmap.colorSpace()), pmap.addr(), androidFormat, quality,
            reinterpret_cast<void*>(stream), write_to_stream);
}
