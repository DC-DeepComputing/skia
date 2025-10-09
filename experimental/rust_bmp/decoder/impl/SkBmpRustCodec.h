/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBmpRustCodec_DEFINED
#define SkBmpRustCodec_DEFINED

#include <memory>
#include <vector>

#include "experimental/rust_bmp/ffi/FFI.rs.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "third_party/rust/cxx/v1/cxx.h"

struct SkEncodedInfo;
class SkStream;
template <typename T> class SkSpan;

// This class provides the Skia image decoding API (`SkCodec`) on top of:
// * The third-party BMP crate or custom Rust implementation (BMP decompression and decoding implemented in Rust)
// * Skia's `SkSwizzler` and `skcms_Transform` (pixel format and color space transformations implemented in C++).
class SkBmpRustCodec final : public SkCodec {
public:
    static std::unique_ptr<SkBmpRustCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

    // `public` to support `std::make_unique<SkBmpRustCodec>(...)`.
    SkBmpRustCodec(SkEncodedInfo&&, std::unique_ptr<SkStream>, rust::Box<rust_bmp::Decoder>);

    ~SkBmpRustCodec() override;

private:
    struct DecodingState {
        // `fDst` points to the destination buffer for decoded pixels
        SkSpan<uint8_t> fDst;

        // Size of a row (in bytes) in the destination buffer
        size_t fDstRowStride = 0;

        // Size of a row (in bytes) for the current image
        size_t fDstRowSize = 0;

        // Buffer for intermediate pixel data during color transformations
        std::vector<uint8_t> fTempBuffer;

        // Current scanline position for incremental decoding
        int fCurrentScanline = 0;

        // Full decoded image data cached for scanline extraction
        std::vector<uint8_t> fCachedImageData;

        // Destination color type for format conversion
        SkColorType fDstColorType = kUnknown_SkColorType;

        // Source RGB24 buffer from Rust decoder
        std::vector<uint8_t> fRustDecodedData;
    };

    // Helper function for color format conversion
    bool convertRgb24ToRequestedFormat(const uint8_t* srcBgr24,
                                       uint8_t* dst,
                                       uint32_t width,
                                       uint32_t height,
                                       SkColorType dstColorType,
                                       size_t dstRowStride);

    // Helper for validating parameters and setting up decoding state
    Result startDecoding(const SkImageInfo& dstInfo,
                         void* pixels,
                         size_t rowBytes,
                         const Options& options,
                         DecodingState* decodingState);

    // Helper for row-by-row decoding
    Result incrementalDecode(DecodingState& decodingState, int* rowsDecoded);

    // SkCodec overrides:
    Result onGetPixels(const SkImageInfo& dstInfo,
                       void* pixels,
                       size_t rowBytes,
                       const Options& options,
                       int* rowsDecoded) override;

    Result onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                    void* pixels,
                                    size_t rowBytes,
                                    const Options& options) override;

    Result onIncrementalDecode(int* rowsDecoded) override;

    Result onStartScanlineDecode(const SkImageInfo& dstInfo,
                                const Options& options) override;

    int onGetScanlines(void* dst, int countLines, size_t rowBytes) override;

    bool onRewind() override;

    SkEncodedImageFormat onGetEncodedFormat() const override {
        return SkEncodedImageFormat::kBMP;
    }

    // BMP files typically don't support animation, so we use default implementations
    // for frame-related methods that return single-frame behavior

private:
    // The Rust BMP decoder that handles the actual BMP parsing and decoding
    rust::Box<rust_bmp::Decoder> fDecoder;

    // Current decoding state (only valid during incremental decoding)
    std::unique_ptr<DecodingState> fDecodingState;

    using INHERITED = SkCodec;
};

#endif  // SkBmpRustCodec_DEFINED
