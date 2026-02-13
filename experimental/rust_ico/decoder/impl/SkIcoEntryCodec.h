/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkIcoEntryCodec_DEFINED
#define SkIcoEntryCodec_DEFINED

#include "experimental/rust_ico/ffi/FFI.rs.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "third_party/rust/cxx/v1/cxx.h"

#include <memory>
#include <vector>

class SkStream;
struct SkEncodedInfo;
class SkSwizzler;

/**
 * Codec for a single ICO entry (either PNG or BMP-in-ICO).
 *
 * This class wraps the Rust ICO Reader to provide the SkCodec interface
 * for embedded images within an ICO file. It handles:
 * - PNG images (standard PNG format)
 * - BMP images (ICO-specific format without file header, with AND mask)
 *
 * All images are decoded to RGBA format by the Rust decoder.
 */
class SkIcoEntryCodec final : public SkCodec {
public:
    /**
     * Creates a codec for an ICO entry.
     *
     * @param data The raw embedded image data (PNG or BMP-in-ICO format)
     * @param format Whether the data is PNG or BMP
     * @param entrySize Total size from ICO directory (used for AND mask)
     * @param result Output parameter for creation result
     * @return Codec on success, nullptr on failure
     */
    static std::unique_ptr<SkIcoEntryCodec> Make(
            const uint8_t* data,
            size_t dataSize,
            rust_ico::EmbeddedFormat format,
            uint32_t entrySize,
            Result* result);

    ~SkIcoEntryCodec() override;

protected:
    SkEncodedImageFormat onGetEncodedFormat() const override {
        return fFormat == rust_ico::EmbeddedFormat::Png
                ? SkEncodedImageFormat::kPNG
                : SkEncodedImageFormat::kBMP;
    }

    SkScanlineOrder onGetScanlineOrder() const override {
        return kTopDown_SkScanlineOrder;
    }

private:
    SkIcoEntryCodec(SkEncodedInfo&&,
                    rust::Box<rust_ico::Reader> reader,
                    rust_ico::EmbeddedFormat format,
                    size_t srcRowBytes);

    // Full image decode
    Result onGetPixels(const SkImageInfo& dstInfo,
                       void* dst,
                       size_t dstRowBytes,
                       const Options& options,
                       int* rowsDecoded) override;

    // Scanline decode support
    Result onStartScanlineDecode(const SkImageInfo& dstInfo,
                                 const Options& options) override;
    int onGetScanlines(void* dst, int count, size_t rowBytes) override;
    bool onSkipScanlines(int count) override;

    // Incremental decode support
    Result onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                    void* dst,
                                    size_t rowBytes,
                                    const Options& options) override;
    Result onIncrementalDecode(int* rowsDecoded) override;

    // Sampler support
    SkSampler* getSampler(bool createIfNecessary) override;

    // Helper to initialize swizzler
    Result initializeSwizzler(const SkImageInfo& dstInfo, const Options& options);

    rust::Box<rust_ico::Reader> fReader;
    rust_ico::EmbeddedFormat fFormat;
    const size_t fSrcRowBytes;

    // For scanline/incremental decode
    std::unique_ptr<SkSwizzler> fSwizzler;
    std::vector<uint8_t> fSrcBuffer;  // Row buffer for decoding
    int fCurrScanline = 0;

    // For incremental decode
    void* fIncrementalDst = nullptr;
    size_t fIncrementalRowBytes = 0;
};

#endif  // SkIcoEntryCodec_DEFINED
