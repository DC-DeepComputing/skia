/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/encoder/impl/SkPngRustEncoderImpl.h"

#include <limits>
#include <memory>
#include <utility>

#include "experimental/rust_png/encoder/SkPngRustEncoder.h"
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

rust_png::Compression ToCompression(SkPngRustEncoder::CompressionLevel level) {
    switch (level) {
        case SkPngRustEncoder::CompressionLevel::kFast:
            return rust_png::Compression::Fast;
        case SkPngRustEncoder::CompressionLevel::kAverage:
            return rust_png::Compression::Default;
        case SkPngRustEncoder::CompressionLevel::kSlow:
            return rust_png::Compression::Best;
    }
    SkUNREACHABLE;
}

rust::String getDataTableEntry(const SkDataTable& table, int index, size_t maxLength) {
    SkASSERT((0 <= index) && (index < table.count()));

    size_t size = 0;
    const char* entry = table.atT<char>(index, &size);
    while (size > 0 && entry[size - 1] == '\0') {
        size--;
    }

    if (size > maxLength) {
        size = maxLength;
    }

    return rust::String::lossy(entry, size);
}

rust_png::EncodingResult EncodeComments(rust_png::Writer& writer,
                                        const sk_sp<SkDataTable>& comments) {
    if (comments != nullptr) {
        for (int i = 0; i < comments->count() / 2; ++i) {
            constexpr size_t kMaxKeywordLength = 79;  // Based on the spec.
            rust::String keyword = getDataTableEntry(*comments, 2 * i, kMaxKeywordLength);
            rust::String text =
                    getDataTableEntry(*comments, 2 * i + 1, std::numeric_limits<size_t>::max());
            rust_png::EncodingResult result =
                    writer.write_text_chunk(std::move(keyword), std::move(text));
            if (result != rust_png::EncodingResult::Success) {
                return result;
            }
        }
    }

    return rust_png::EncodingResult::Success;
}

// This helper class adapts `SkWStream` to expose the API required by Rust FFI
// (i.e. the `WriteTrait` API).
class WriteTraitAdapterForSkWStream final : public rust_png::WriteTrait {
public:
    // SAFETY: The caller needs to guarantee that `stream` will be alive for
    // as long as `WriteTraitAdapterForSkWStream`.
    explicit WriteTraitAdapterForSkWStream(SkWStream* stream) : fStream(stream) {
        SkASSERT(fStream);
    }

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
std::unique_ptr<SkEncoder> SkPngRustEncoderImpl::Make(SkWStream* dst,
                                                      const SkPixmap& src,
                                                      const SkPngRustEncoder::Options& options) {
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
                                 dstInfo.bitsPerComponent(),
                                 ToCompression(options.fCompressionLevel));
    if (resultOfWriter->err() != rust_png::EncodingResult::Success) {
        return nullptr;
    }
    rust::Box<rust_png::Writer> writer = resultOfWriter->unwrap();

    if (EncodeComments(*writer, options.fComments) != rust_png::EncodingResult::Success) {
        return nullptr;
    }

    rust::Box<rust_png::ResultOfStreamWriter> resultOfStreamWriter = writer->into_stream_writer();
    if (resultOfStreamWriter->err() != rust_png::EncodingResult::Success) {
        return nullptr;
    }
    rust::Box<rust_png::StreamWriter> stream_writer = resultOfStreamWriter->unwrap();

    return std::make_unique<SkPngRustEncoderImpl>(
            dstInfo, transformProc, src, std::move(stream_writer));
}

SkPngRustEncoderImpl::SkPngRustEncoderImpl(const SkEncodedInfo& dstInfo,
                                           transform_scanline_proc transformProc,
                                           const SkPixmap& src,
                                           rust::Box<rust_png::StreamWriter> stream_writer)
        : SkPngEncoderBase(dstInfo, src, transformProc), fStreamWriter(std::move(stream_writer)) {}

SkPngRustEncoderImpl::~SkPngRustEncoderImpl() = default;

bool SkPngRustEncoderImpl::onEncodeRow(SkSpan<uint8_t> row) {
    return fStreamWriter->write(rust::Slice<const uint8_t>(row)) ==
           rust_png::EncodingResult::Success;
}

bool SkPngRustEncoderImpl::onFinishEncoding() {
    return fStreamWriter->finish() == rust_png::EncodingResult::Success;
}
