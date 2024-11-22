/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/encoder/impl/SkPngRustEncoderImpl.h"

#include <memory>
#include <utility>

#include "experimental/rust_png/ffi/FFI.rs.h"
#include "experimental/rust_png/ffi/UtilsForFFI.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkSafeMath.h"
#include "src/encode/SkImageEncoderPriv.h"
#include "third_party/rust/cxx/v1/cxx.h"

#ifdef __clang__
#pragma clang diagnostic error "-Wconversion"
#endif

namespace {

rust_png::ColorType ToColorType(SkEncodedInfo::Color color) {
    switch (color) {
        case SkEncodedInfo::kRGB_Color:
            return rust_png::ColorType::Rgb;
        case SkEncodedInfo::kRGBA_Color:
            return rust_png::ColorType::Rgba;
        case SkEncodedInfo::kGray_Color:
            return rust_png::ColorType::Grayscale;
        case SkEncodedInfo::kGrayAlpha_Color:
            return rust_png::ColorType::GrayscaleAlpha;
        default:
            SkUNREACHABLE;
    }
}

// This helper class adapts `SkWStream` to expose the API required by Rust FFI
// (i.e. the `WriteTrait` API).
class WriteTraitAdapterForSkWStream final : public rust_png::WriteTrait {
public:
    // SAFETY: The caller needs to guarantee that `stream` will be alive for
    // as long as `WriteTraitAdapterForSkWStream`.
    explicit WriteTraitAdapterForSkWStream(SkWStream* stream) : fStream(stream) {}

    ~WriteTraitAdapterForSkWStream() override = default;

    // Non-copyable and non-movable.
    WriteTraitAdapterForSkWStream(const WriteTraitAdapterForSkWStream&) = delete;
    WriteTraitAdapterForSkWStream& operator=(const WriteTraitAdapterForSkWStream&) = delete;
    WriteTraitAdapterForSkWStream(WriteTraitAdapterForSkWStream&&) = delete;
    WriteTraitAdapterForSkWStream& operator=(WriteTraitAdapterForSkWStream&&) = delete;

    // Implementation of the `std::io::Read::read` method.  See `RustTrait`'s
    // doc comments and
    // https://doc.rust-lang.org/nightly/std/io/trait.Read.html#tymethod.read
    // for guidance on the desired implementation and behavior of this method.
    bool write(rust::Slice<const uint8_t> buffer) override {
        SkSpan<const uint8_t> span = ToSkSpan(buffer);
        return fStream->write(span.data(), span.size());
    }

    void flush() override { fStream->flush(); }

private:
    SkWStream* fStream = nullptr;  // Non-owning pointer.
};

}  // namespace

// static
std::unique_ptr<SkEncoder> SkPngRustEncoderImpl::Make(SkWStream* dst, const SkPixmap& src) {
    if (!SkPixmapIsValid(src)) {
        return nullptr;
    }

    std::optional<std::pair<SkEncodedInfo, transform_scanline_proc>> maybeDstInfo =
            SkPngEncoderBase::getTargetInfo(src.info());
    if (!maybeDstInfo.has_value()) {
        return nullptr;
    }
    const SkEncodedInfo& dstInfo = maybeDstInfo->first;
    transform_scanline_proc transformProc = maybeDstInfo->second;

    SkSafeMath safe;
    uint32_t width = safe.castTo<uint32_t>(dstInfo.width());
    uint32_t height = safe.castTo<uint32_t>(dstInfo.height());
    if (!safe.ok()) {
        return nullptr;
    }

    auto writeTraitAdapter = std::make_unique<WriteTraitAdapterForSkWStream>(dst);
    rust::Box<rust_png::ResultOfWriter> resultOfWriter =
            rust_png::new_writer(std::move(writeTraitAdapter),
                                 width,
                                 height,
                                 ToColorType(dstInfo.color()),
                                 dstInfo.bitsPerComponent());
    if (resultOfWriter->err() != rust_png::EncodingResult::Success) {
        return nullptr;
    }
    rust::Box<rust_png::Writer> writer = resultOfWriter->unwrap();

    return std::make_unique<SkPngRustEncoderImpl>(dstInfo, transformProc, src, std::move(writer));
}

SkPngRustEncoderImpl::SkPngRustEncoderImpl(const SkEncodedInfo& dstInfo,
                                           transform_scanline_proc transformProc,
                                           const SkPixmap& src,
                                           rust::Box<rust_png::Writer> writer)
        : SkPngEncoderBase(dstInfo, src, transformProc), fWriter(std::move(writer)) {}

SkPngRustEncoderImpl::~SkPngRustEncoderImpl() = default;

bool SkPngRustEncoderImpl::onEncodeRow(SkSpan<uint8_t> row) {
    return fWriter->write(rust::Slice<const uint8_t>(row)) == rust_png::EncodingResult::Success;
}

bool SkPngRustEncoderImpl::onFinishEncoding() {
    return fWriter->finish() == rust_png::EncodingResult::Success;
}
