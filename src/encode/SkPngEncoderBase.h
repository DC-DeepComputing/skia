/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngEncoderBase_DEFINED
#define SkPngEncoderBase_DEFINED

#include <cstddef>
#include <cstdint>

#include <optional>
#include <utility>

#include "include/encode/SkEncoder.h"
#include "src/encode/SkImageEncoderFns.h"

struct SkEncodedInfo;
struct SkImageInfo;
class SkPixmap;
template <typename T> class SkSpan;

// This class implements functionality shared between `SkPngEncoderImpl` and
// `SkPngRustEncoderImpl` (the latter is from `experimental/rust_png`).
class SkPngEncoderBase : public SkEncoder {
public:
    // Gets the `SkEncodedInfo` that `srcInfo` should be converted into before
    // encoding and a `transform_scanline_proc` that can transform source rows into
    // ready-to-encode rows.
    //
    // For example, `kRGBA_F32_SkColorType` source will be encoded as
    // `SkEncodedInfo::kRGBA_Color` with 16 `bitsPerComponent`.  Depending on
    // `src`'s alpha type, such transformation can be handled by either
    // `transform_scanline_F32` or `transform_scanline_F32_premul`.
    //
    // Returns `std::nullopt` if `srcInfo` is not supported by the PNG encoder.
    static std::optional<std::pair<SkEncodedInfo, transform_scanline_proc>> getTargetInfo(
            const SkImageInfo& srcInfo);

protected:
    SkPngEncoderBase(const SkEncodedInfo& dstInfo,
                     const SkPixmap& src,
                     transform_scanline_proc transformProc);

    // SkEncoder override:
    bool onEncodeRows(int numRows) final;

    // Called from `onEncodeRows` to encode the given `row` (in `dstInfo` format
    // that was passed to the `SkPngEncoderBase`'s constructor).
    //
    // TODO: This should be `const uint8_t`, but `libpng` takes a non-`const`
    // `png_bytep`.  I guess that if (???) backend becomes not supported, then
    // this TODO can be addressed.
    virtual bool onEncodeRow(SkSpan<uint8_t> row) = 0;

    // Called from `onEncodeRows` to finalize the encoded PNG (e.g. write the
    // `IEND` chunk).
    virtual bool onFinishEncoding() = 0;

private:
    transform_scanline_proc fTransformProc = nullptr;
    size_t fDstRowSize = 0;
    bool fFinishedEncoding = false;
};

#endif  // SkPngEncoderBase_DEFINED
