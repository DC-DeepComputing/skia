/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngRustEncoderImpl_DEFINED
#define SkPngRustEncoderImpl_DEFINED

#include <memory>

#include "experimental/rust_png/ffi/FFI.rs.h"
#include "include/encode/SkEncoder.h"
#include "third_party/rust/cxx/v1/cxx.h"

struct SkImageInfo;
class SkPixmap;
class SkWStream;

// This class provides the Skia image encoding API (`SkEncoder`) on top of the
// third-party `png` crate (PNG compression and encoding implemented in Rust).
class SkPngRustEncoderImpl final : public SkEncoder {
public:
    static std::unique_ptr<SkEncoder> Make(SkWStream*, const SkPixmap&);

    // `public` to support `std::make_unique<SkPngRustEncoderImpl>(...)`.
    SkPngRustEncoderImpl(SkWStream*, const SkPixmap&, size_t encodedRowSize);

    ~SkPngRustEncoderImpl() override;

protected:
    bool onEncodeRows(int numRows) override;

private:
    // Returns how many bytes per row the encoded PNG will use for the given
    // `srcInfo`.  For example, before encoding `kRGBA_F32_SkColorType` pixels,
    // each component (i.e. red/green/blue/alpha) will first be downsampled into
    // `uint16_t and therefore the encoded row will take `sizeof(uint16_t) * 4 *
    // width` bytes.
    //
    // Returns 0 to indicate an unrecognized/unsupported `srcColor`.
    static size_t getEncodedRowSizeInBytes(const SkImageInfo& srcInfo);
};

#endif  // SkPngRustEncoderImpl_DEFINED
