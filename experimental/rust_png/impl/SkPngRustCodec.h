/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngRustCodec_DEFINED
#define SkPngRustCodec_DEFINED

#include <memory>
#include <vector>

#include "experimental/rust_png/ffi/FFI.rs.h"
#include "src/codec/SkFrameHolder.h"
#include "src/codec/SkPngCodecBase.h"
#include "third_party/rust/cxx/v1/cxx.h"

struct SkEncodedInfo;
class SkFrame;
class SkStream;
template <typename T> class SkSpan;

// This class provides the Skia image decoding API (`SkCodec`) on top of:
// * The third-party `png` crate (PNG decompression and decoding implemented in
//   Rust)
// * Skia's `SkSwizzler` and `skcms_Transform` (pixel format and color space
//   transformations implemented in C++).
class SkPngRustCodec final : public SkPngCodecBase {
public:
    static std::unique_ptr<SkPngRustCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

    // `public` to support `std::make_unique<SkPngRustCodec>(...)`.
    SkPngRustCodec(SkEncodedInfo&&, std::unique_ptr<SkStream>, rust::Box<rust_png::Reader>);

    ~SkPngRustCodec() override;

private:
    struct DecodingState {
        SkSpan<uint8_t> dst;
        size_t dstRowSize;  // in bytes.
        size_t bytesPerPixel;
    };

    // Helper for validating parameters of `onGetPixels` and/or
    // `onStartIncrementalDecode`.  If `kSuccess` is returned then
    // `decodingState` output parameter got populated.
    Result startDecoding(const SkImageInfo& dstInfo,
                         void* pixels,
                         size_t rowBytes,
                         const Options& options,
                         DecodingState* decodingState);

    // Helper for row-by-row decoding which is used from `onGetPixels` and/or
    // `onIncrementalDecode`.
    Result incrementalDecode(DecodingState& decodingState, int* rowsDecoded);

    // SkCodec overrides:
    Result onGetPixels(const SkImageInfo& dstInfo,
                       void* pixels,
                       size_t rowBytes,
                       const Options&,
                       int* rowsDecoded) override;
    Result onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                    void* pixels,
                                    size_t rowBytes,
                                    const Options&) override;
    Result onIncrementalDecode(int* rowsDecoded) override;
    int onGetFrameCount() override;
    bool onGetFrameInfo(int, FrameInfo*) const override;
    int onGetRepetitionCount() override;
    const SkFrameHolder* getFrameHolder() const override;

    // SkPngCodecBase overrides:
    std::optional<SkSpan<const PaletteColorEntry>> onTryGetPlteChunk() override;
    std::optional<SkSpan<const uint8_t>> onTryGetTrnsChunk() override;

    rust::Box<rust_png::Reader> fReader;

    std::optional<DecodingState> fIncrementalDecodingState;

    class FrameHolder final : public SkFrameHolder {
    public:
        FrameHolder(int width, int height);
        ~FrameHolder() override;

        FrameHolder(const FrameHolder&) = delete;
        FrameHolder(FrameHolder&&) = delete;
        FrameHolder& operator=(const FrameHolder&) = delete;
        FrameHolder& operator=(FrameHolder&&) = delete;

        size_t size() const;

        void appendNewFrame(const rust_png::Reader& reader, const SkEncodedInfo& info);

        // Helper for using contents of an `fcTL` chunk to populate a `PngFrame`
        // info.  Internally used from `appendNewFrame`, but exposed publically to
        // support re-populating the frame info after skipping the default, non-animated image
        // (in some PNGs the initial `fcTL` may not appear until later).
        void populateLastFrameInfo(const rust_png::Reader& reader, const SkEncodedInfo& info);

    private:
        const SkFrame* onGetFrame(int i) const override;

        class PngFrame;
        std::vector<PngFrame> fFrames;
    };
    FrameHolder fFrameHolder;

    size_t fNumOfFullyReceivedFrames = 0;
};

#endif  // SkPngRustCodec_DEFINED
