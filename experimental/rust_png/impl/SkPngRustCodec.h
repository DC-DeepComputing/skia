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
#include "src/codec/SkPngCodecBase.h"
#include "third_party/rust/cxx/v1/cxx.h"

struct SkEncodedInfo;
class SkStream;
class SkFrame;

// This class provides the Skia image decoding API (`SkCodec`) on top of:
// * The third-party `png` crate (PNG decompression and decoding implemented in
//   Rust)
// * Skia's `SkSwizzler` and `skcms_Transform` (pixel format and color space
//   transformations implemented in C++).
class SkPngRustCodec : public SkPngCodecBase {
public:
    static std::unique_ptr<SkPngRustCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

    SkPngRustCodec(SkEncodedInfo&&, std::unique_ptr<SkStream>, rust::Box<rust_png::Reader>);
    ~SkPngRustCodec() override;

private:
    // SkCodec overrides:
    Result onGetPixels(const SkImageInfo& info,
                       void* pixels,
                       size_t rowBytes,
                       const Options&,
                       int* rowsDecoded) override;
    bool onGetFrameInfo(int, FrameInfo*) const override;

    // SkPngCodecBase overrides:
    std::optional<SkSpan<const PaletteColorEntry>> onTryGetPlteChunk() override;
    std::optional<SkSpan<const uint8_t>> onTryGetTrnsChunk() override;

    rust::Box<rust_png::Reader> fReader;

    class PngFrame;
    std::vector<PngFrame> fFrames;

    size_t fNumOfFullyReceivedFrames = 0;
};

#endif  // SkPngRustCodec_DEFINED
