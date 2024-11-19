/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/encoder/impl/SkPngRustEncoderImpl.h"

#include <memory>

#include "experimental/rust_png/ffi/FFI.rs.h"
#include "include/core/SkStream.h"
#include "src/encode/SkImageEncoderPriv.h"
#include "third_party/rust/cxx/v1/cxx.h"

#ifdef __clang__
#pragma clang diagnostic error "-Wconversion"
#endif

// static
size_t SkPngRustEncoderImpl::getEncodedRowSizeInBytes(const SkImageInfo& srcInfo) {
    // TODO(https://crbug.com/379312510): Instead of always returning 0
    // (to indicate failure) we should refactor and reuse here how
    // `SkPngEncoderMgr::setHeader` sets `fPngBytesPerPixel`.
    return 0;
}

// static
std::unique_ptr<SkEncoder> SkPngRustEncoderImpl::Make(SkWStream* dst, const SkPixmap& src) {
    if (!SkPixmapIsValid(src)) {
        return nullptr;
    }

    size_t encodedRowSize = SkPngRustEncoderImpl::getEncodedRowSizeInBytes(src.info());
    if (encodedRowSize == 0) {
        return nullptr;
    }

    return std::make_unique<SkPngRustEncoderImpl>(dst, src, encodedRowSize);
}

SkPngRustEncoderImpl::SkPngRustEncoderImpl(SkWStream* dst,
                                           const SkPixmap& src,
                                           size_t encodedRowSize)
        : SkEncoder(src, encodedRowSize) {
    // TODO(https://crbug.com/379312510): Actually call into Rust PNG crate
    // here.
}

SkPngRustEncoderImpl::~SkPngRustEncoderImpl() = default;

bool SkPngRustEncoderImpl::onEncodeRows(int numRows) {
    // TODO(https://crbug.com/379312510): Actually call into Rust PNG crate
    // here.
    return false;
}
