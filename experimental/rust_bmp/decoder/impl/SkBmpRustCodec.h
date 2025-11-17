/*
 * Copyright 2025 Google LLC.
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
// * Custom Rust BMP implementation (BMP decompression and decoding implemented in Rust)
// * Skia's `SkSwizzler` and `skcms_Transform` (pixel format and color space transformations implemented in C++).
class SkBmpRustCodec final : public SkCodec {
public:
    static std::unique_ptr<SkBmpRustCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

    ~SkBmpRustCodec() override;

protected:
    SkBmpRustCodec(SkEncodedInfo&&, std::unique_ptr<SkStream>, rust::Box<rust_bmp::Reader>, SkCodec::SkScanlineOrder);

    SkEncodedImageFormat onGetEncodedFormat() const override { return SkEncodedImageFormat::kBMP; }

    bool onRewind() override;

    SkScanlineOrder onGetScanlineOrder() const override { return fRowOrder; }

private:
    struct DecodingState {
        // Buffer for decoded image data from Rust
        std::vector<uint8_t> fRustDecodedData;

        // Whether the image has been fully decoded
        bool fFullyDecoded = false;

        // Current scanline position for incremental decoding
        int fCurrentScanline = 0;

        // Bytes per pixel for the decoded data
        uint32_t fBytesPerPixel = 3;  // Default to 3 for BGR/RGB

        // Whether decoded data is in BGR (true) or RGB (false) format
        bool fIsBgr = true;  // Default to BGR for 24-bit images

        // Whether the input data was incomplete
        bool fDataIsIncomplete = false;
    };

    // Codec implementation
    Result onGetPixels(const SkImageInfo& info,
                       void* dst,
                       size_t dstRowStride,
                       const Options& options,
                       int* rowsDecoded) override;

    // Scanline decoding implementation
    Result onStartScanlineDecode(const SkImageInfo& dstInfo,
                                 const Options& options) override;

    int onGetScanlines(void* dst, int count, size_t rowBytes) override;

    bool onSkipScanlines(int count) override;

    // Helper methods
    Result performFullDecode(const SkImageInfo& dstInfo, void* dst, size_t dstRowStride);
    Result ensureDecodedData();

    // Streaming Rust reader instance
    rust::Box<rust_bmp::Reader> fReader;

    // Decoding state
    std::unique_ptr<DecodingState> fDecodingState;

    // Stream for rewinding
    std::unique_ptr<SkStream> fPrivStream;

    // Scanline order (top-down vs bottom-up)
    const SkScanlineOrder fRowOrder;

private:
    using INHERITED = SkCodec;
};

#endif  // SkBmpRustCodec_DEFINED
